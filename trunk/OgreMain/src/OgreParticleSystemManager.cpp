/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#include "OgreParticleSystemManager.h"
#include "OgreParticleEmitterFactory.h"
#include "OgreParticleAffectorFactory.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreString.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreBillboardParticleRenderer.h"
#include "OgreStringConverter.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    // Shortcut to set up billboard particle renderer
    BillboardParticleRendererFactory* mBillboardRendererFactory = 0;
    //-----------------------------------------------------------------------
    template<> ParticleSystemManager* Singleton<ParticleSystemManager>::ms_Singleton = 0;
    ParticleSystemManager* ParticleSystemManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    ParticleSystemManager& ParticleSystemManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //-----------------------------------------------------------------------
    ParticleSystemManager::ParticleSystemManager()
    {
		mTimeFactor = 1;
        mScriptPatterns.push_back("*.particle");
        ResourceGroupManager::getSingleton()._registerScriptLoader(this);
		mFactory = new ParticleSystemFactory();
    }
    //-----------------------------------------------------------------------
    ParticleSystemManager::~ParticleSystemManager()
    {
        // Destroy all templates
        ParticleTemplateMap::iterator t;
        for (t = mSystemTemplates.begin(); t != mSystemTemplates.end(); ++t)
        {
            delete t->second;
        }
        mSystemTemplates.clear();
        // Destroy all systems 
        ParticleSystemMap::iterator i;
        for (i = mSystems.begin(); i != mSystems.end(); ++i)
        {
            delete i->second;
        }
        mSystems.clear();
        ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
        // delete billboard factory
        if (mBillboardRendererFactory)
            delete mBillboardRendererFactory;
		// delete particle system factory
		delete mFactory;

    }
    //-----------------------------------------------------------------------
    const StringVector& ParticleSystemManager::getScriptPatterns(void) const
    {
        return mScriptPatterns;
    }
    //-----------------------------------------------------------------------
    Real ParticleSystemManager::getLoadingOrder(void) const
    {
        /// Load late
        return 1000.0f;
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseScript(DataStreamPtr& stream, const String& groupName)
    {
        String line;
        ParticleSystem* pSys;
        std::vector<String> vecparams;

        pSys = 0;

        while(!stream->eof())
        {
            line = stream->getLine();
            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (pSys == 0)
                {
                    // No current system
                    // So first valid data should be a system name
                    pSys = createTemplate(line, groupName);
					pSys->_notifyOrigin(stream->getName());
                    // Skip to and over next {
                    skipToNextOpenBrace(stream);
                }
                else
                {
                    // Already in a system
                    if (line == "}")
                    {
                        // Finished system
                        pSys = 0;
                    }
                    else if (line.substr(0,7) == "emitter")
                    {
                        // new emitter
                        // Get typename
                        vecparams = StringUtil::split(line, "\t ");
                        if (vecparams.size() < 2)
                        {
                            // Oops, bad emitter
                            LogManager::getSingleton().logMessage("Bad particle system emitter line: '"
                                + line + "' in " + pSys->getName());
                            skipToNextCloseBrace(stream);

                        }
                        skipToNextOpenBrace(stream);
                        parseNewEmitter(vecparams[1], stream, pSys);

                    }
                    else if (line.substr(0,8) == "affector")
                    {
                        // new affector
                        // Get typename
                        vecparams = StringUtil::split(line, "\t ");
                        if (vecparams.size() < 2)
                        {
                            // Oops, bad emitter
                            LogManager::getSingleton().logMessage("Bad particle system affector line: '"
                                + line + "' in " + pSys->getName());
                            skipToNextCloseBrace(stream);

                        }
                        skipToNextOpenBrace(stream);
                        parseNewAffector(vecparams[1],stream, pSys);
                    }
                    else
                    {
                        // Attribute
                        parseAttrib(line, pSys);
                    }

                }

            }


        }


    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::addEmitterFactory(ParticleEmitterFactory* factory)
    {
        String name = factory->getName();
        mEmitterFactories[name] = factory;
        LogManager::getSingleton().logMessage("Particle Emitter Type '" + name + "' registered");
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::addAffectorFactory(ParticleAffectorFactory* factory)
    {
        String name = factory->getName();
        mAffectorFactories[name] = factory;
        LogManager::getSingleton().logMessage("Particle Affector Type '" + name + "' registered");
    }
	//-----------------------------------------------------------------------
	void ParticleSystemManager::addRendererFactory(ParticleSystemRendererFactory* factory)
	{
        String name = factory->getType();
        mRendererFactories[name] = factory;
        LogManager::getSingleton().logMessage("Particle Renderer Type '" + name + "' registered");
	}
	//-----------------------------------------------------------------------
    void ParticleSystemManager::addTemplate(const String& name, ParticleSystem* sysTemplate)
    {
        mSystemTemplates[name] = sysTemplate;
    }
    //-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::createTemplate(const String& name, 
        const String& resourceGroup)
    {
        ParticleSystem* tpl = new ParticleSystem(name, resourceGroup);
        addTemplate(name, tpl);
        return tpl;

    }
    //-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::getTemplate(const String& name)
    {
        ParticleTemplateMap::iterator i = mSystemTemplates.find(name);
        if (i != mSystemTemplates.end())
        {
            return i->second;
        }
        else
        {
            return 0;
        }
    }
    //-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::createSystem(const String& name, size_t quota, 
        const String& resourceGroup)
    {
		return createSystemImpl(name, quota, resourceGroup, true);
    }
    //-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::createSystem(const String& name, 
		const String& templateName)

	{
		return createSystemImpl(name, templateName, true);
	}
	//-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::createSystemImpl(const String& name, size_t quota, 
        const String& resourceGroup, bool notifySM)
    {
		// check name
		if (mSystems.find(name) != mSystems.end())
		{
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
				"ParticleSystem with name '" + name + "' already exists.", 
				"ParticleSystemManager::createSystemImpl");
		}
        ParticleSystem* sys = new ParticleSystem(name, resourceGroup);
        sys->setParticleQuota(quota);
        mSystems.insert( ParticleSystemMap::value_type( name, sys ) );
		if (notifySM)
		{
			// notify current scene manager, if any
			SceneManager* sm = Root::getSingleton()._getCurrentSceneManager();
			if (sm)
				sm->injectMovableObject(sys);
		}
        return sys;
    }
    //-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::createSystemImpl(const String& name, 
		const String& templateName, bool notifySM)
    {
		// check name
		if (mSystems.find(name) != mSystems.end())
		{
			OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM, 
				"ParticleSystem with name '" + name + "' already exists.", 
				"ParticleSystemManager::createSystem");
		}
        // Look up template
        ParticleSystem* pTemplate = getTemplate(templateName);
        if (!pTemplate)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find required template '" + templateName + "'", "ParticleSystemManager::createSystem");
        }

        ParticleSystem* sys = createSystem(name, pTemplate->getParticleQuota(), 
            pTemplate->getResourceGroupName());
        // Copy template settings
        *sys = *pTemplate;
		if (notifySM)
		{
			// notify current scene manager
			SceneManager* sm = Root::getSingleton()._getCurrentSceneManager();
			if (sm)
				sm->injectMovableObject(sys);
		}
        return sys;
        
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::destroySystemImpl(const String& name, 
		bool notifySceneMgr)
	{
		if (notifySceneMgr)
		{
			// notify scene manager
			SceneManager* sm = Root::getSingleton()._getCurrentSceneManager();
			if (sm)
				sm->extractMovableObject(name, 
					ParticleSystemFactory::FACTORY_TYPE_NAME);
		}
        ParticleSystemMap::iterator i = mSystems.find(name);
        if (i != mSystems.end())
        {
            delete i->second;
            mSystems.erase(i);
        }
	}
    //-----------------------------------------------------------------------
    void ParticleSystemManager::destroySystemImpl(ParticleSystem* sys, 
		bool notifySceneMgr)
	{
		if (notifySceneMgr)
		{
			// notify scene manager
			SceneManager* sm = Root::getSingleton()._getCurrentSceneManager();
			if (sm)
				sm->extractMovableObject(sys);
		}
		
        ParticleSystemMap::iterator i;
        for (i = mSystems.begin(); i != mSystems.end(); ++i)
        {
            if (i->second == sys)
            {
                delete i->second;
                mSystems.erase(i);
                break;
            }
        }
	}
    //-----------------------------------------------------------------------
    void ParticleSystemManager::destroySystem(const String& name)
    {
		destroySystemImpl(name, true);
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::destroySystem(ParticleSystem* sys)
    {
		destroySystemImpl(sys, true);
    }

    //-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::getSystem(const String& name)
    {
        ParticleSystemMap::iterator i = mSystems.find(name);
        if (i != mSystems.end())
        {
            return i->second;
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot find particle system '" + name + "'",
                "ParticleSystemManager::getSystem");
        }
    }

    //-----------------------------------------------------------------------
    ParticleEmitter* ParticleSystemManager::_createEmitter(
        const String& emitterType, ParticleSystem* psys)
    {
        // Locate emitter type
        ParticleEmitterFactoryMap::iterator pFact = mEmitterFactories.find(emitterType);

        if (pFact == mEmitterFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find requested emitter type.", 
                "ParticleSystemManager::_createEmitter");
        }

        return pFact->second->createEmitter(psys);
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::_destroyEmitter(ParticleEmitter* emitter)
    {
        // Destroy using the factory which created it
        ParticleEmitterFactoryMap::iterator pFact = mEmitterFactories.find(emitter->getType());

        if (pFact == mEmitterFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find emitter factory to destroy emitter.", 
                "ParticleSystemManager::_destroyEmitter");
        }

        pFact->second->destroyEmitter(emitter);
    }
    //-----------------------------------------------------------------------
    ParticleAffector* ParticleSystemManager::_createAffector(
        const String& affectorType, ParticleSystem* psys)
    {
        // Locate affector type
        ParticleAffectorFactoryMap::iterator pFact = mAffectorFactories.find(affectorType);

        if (pFact == mAffectorFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find requested affector type.", 
                "ParticleSystemManager::_createAffector");
        }

        return pFact->second->createAffector(psys);

    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::_destroyAffector(ParticleAffector* affector)
    {
        // Destroy using the factory which created it
        ParticleAffectorFactoryMap::iterator pFact = mAffectorFactories.find(affector->getType());

        if (pFact == mAffectorFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find affector factory to destroy affector.", 
                "ParticleSystemManager::_destroyAffector");
        }

        pFact->second->destroyAffector(affector);
    }
    //-----------------------------------------------------------------------
    ParticleSystemRenderer* ParticleSystemManager::_createRenderer(const String& rendererType)
	{
        // Locate affector type
        ParticleSystemRendererFactoryMap::iterator pFact = mRendererFactories.find(rendererType);

        if (pFact == mRendererFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find requested renderer type.", 
                "ParticleSystemManager::_createRenderer");
        }

        return pFact->second->createInstance(rendererType);
	}
	//-----------------------------------------------------------------------
    void ParticleSystemManager::_destroyRenderer(ParticleSystemRenderer* renderer)
	{
        // Destroy using the factory which created it
        ParticleSystemRendererFactoryMap::iterator pFact = mRendererFactories.find(renderer->getType());

        if (pFact == mRendererFactories.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot find renderer factory to destroy renderer.", 
                "ParticleSystemManager::_destroyRenderer");
        }

        pFact->second->destroyInstance(renderer);
	}
    //-----------------------------------------------------------------------
    bool ParticleSystemManager::frameStarted(const FrameEvent &evt)
    {
		// Apply time factor
		Real timeSinceLastFrame = mTimeFactor * evt.timeSinceLastFrame;

        // update systems
        // TODO: only do this for visible systems
        ParticleSystemMap::iterator i;
        for (i = mSystems.begin(); i != mSystems.end(); ++i)
        {
            i->second->_update(timeSinceLastFrame);
        }

        return true;
    }
    //-----------------------------------------------------------------------
    bool ParticleSystemManager::frameEnded(const FrameEvent &evt)
    {
        return true;
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::_initialise(void)
    {
        // Register self as a frame listener
        Root::getSingleton().addFrameListener(this);

        // Create Billboard renderer factory
        mBillboardRendererFactory = new BillboardParticleRendererFactory();
        addRendererFactory(mBillboardRendererFactory);

    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseNewEmitter(const String& type, DataStreamPtr& stream, ParticleSystem* sys)
    {
        // Create new emitter
        ParticleEmitter* pEmit = sys->addEmitter(type);
        // Parse emitter details
        String line;

        while(!stream->eof())
        {
            line = stream->getLine();
            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (line == "}")
                {
                    // Finished emitter
                    break;
                }
                else
                {
                    // Attribute
					StringUtil::toLowerCase(line);
                    parseEmitterAttrib(line, pEmit);
                }
            }
        }


        
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseNewAffector(const String& type, DataStreamPtr& stream, ParticleSystem* sys)
    {
        // Create new affector
        ParticleAffector* pAff = sys->addAffector(type);
        // Parse affector details
        String line;

        while(!stream->eof())
        {
            line = stream->getLine();
            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (line == "}")
                {
                    // Finished affector
                    break;
                }
                else
                {
                    // Attribute
					StringUtil::toLowerCase(line);
                    parseAffectorAttrib(line, pAff);
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseAttrib(const String& line, ParticleSystem* sys)
    {
        std::vector<String> vecparams;

        // Split params on space
        vecparams = StringUtil::split(line, "\t ", 1);

        // Look up first param (command setting)
        if (!sys->setParameter(vecparams[0], vecparams[1]))
        {
            // Attribute not supported by particle system, try the renderer
            ParticleSystemRenderer* renderer = sys->getRenderer();
            if (renderer)
            {
                if (!renderer->setParameter(vecparams[0], vecparams[1]))
                {
                    LogManager::getSingleton().logMessage("Bad particle system attribute line: '"
                        + line + "' in " + sys->getName() + " (tried renderer)");
                }
            }
            else
            {
                // BAD command. BAD!
                LogManager::getSingleton().logMessage("Bad particle system attribute line: '"
                    + line + "' in " + sys->getName() + " (no renderer)");
            }
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseEmitterAttrib(const String& line, ParticleEmitter* emit)
    {
        std::vector<String> vecparams;

        // Split params on first space
        vecparams = StringUtil::split(line, "\t ", 1);

        // Look up first param (command setting)
        if (!emit->setParameter(vecparams[0], vecparams[1]))
        {
            // BAD command. BAD!
            LogManager::getSingleton().logMessage("Bad particle emitter attribute line: '"
                + line + "' for emitter " + emit->getType());
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseAffectorAttrib(const String& line, ParticleAffector* aff)
    {
        std::vector<String> vecparams;

        // Split params on space
        vecparams = StringUtil::split(line, "\t ", 1);

        // Look up first param (command setting)
        if (!aff->setParameter(vecparams[0], vecparams[1]))
        {
            // BAD command. BAD!
            LogManager::getSingleton().logMessage("Bad particle affector attribute line: '"
                + line + "' for affector " + aff->getType());
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::skipToNextCloseBrace(DataStreamPtr& stream)
    {
        String line = "";
        while (!stream->eof() && line != "}")
        {
            line = stream->getLine();
        }

    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::skipToNextOpenBrace(DataStreamPtr& stream)
    {
        String line = "";
        while (!stream->eof() && line != "{")
        {
            line = stream->getLine();
        }

    }
	//-----------------------------------------------------------------------
	Real ParticleSystemManager::getTimeFactor(void) const {
		return mTimeFactor;
	}
	//-----------------------------------------------------------------------
	void ParticleSystemManager::setTimeFactor(Real tf) {
		if(tf >= 0) mTimeFactor = tf;
	}
	//-----------------------------------------------------------------------
	ParticleSystemManager::ParticleAffectorFactoryIterator 
	ParticleSystemManager::getAffectorFactoryIterator(void)
	{
		return ParticleAffectorFactoryIterator(
			mAffectorFactories.begin(), mAffectorFactories.end());
	}
	//-----------------------------------------------------------------------
	ParticleSystemManager::ParticleEmitterFactoryIterator 
	ParticleSystemManager::getEmitterFactoryIterator(void)
	{
		return ParticleEmitterFactoryIterator(
			mEmitterFactories.begin(), mEmitterFactories.end());
	}
	//-----------------------------------------------------------------------
	ParticleSystemManager::ParticleRendererFactoryIterator 
	ParticleSystemManager::getRendererFactoryIterator(void)
	{
		return ParticleRendererFactoryIterator(
			mRendererFactories.begin(), mRendererFactories.end());
	}
	//-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
	String ParticleSystemFactory::FACTORY_TYPE_NAME = "ParticleSystem";
    //-----------------------------------------------------------------------
	MovableObject* ParticleSystemFactory::createInstanceImpl( const String& name, 
			const NameValuePairList* params)
	{
		if (params != 0)
		{
			NameValuePairList::const_iterator ni = params->find("templateName");
			if (ni != params->end())
			{
				String templateName = ni->second;
				// create using manager, but don't notify SM (we've come from there!)
				return ParticleSystemManager::getSingleton().createSystemImpl(
						name, templateName, false);
			}
		}
		// Not template based, look for quota & resource name
		size_t quota = 500;
		String resourceGroup = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
		if (params != 0)
		{
			NameValuePairList::const_iterator ni = params->find("quota");
			if (ni != params->end())
			{
				quota = StringConverter::parseUnsignedInt(ni->second);
			}
			ni = params->find("resourceGroup");
			if (ni != params->end())
			{
				resourceGroup = ni->second;
			}
		}
		// create using manager, but don't notify SM (we've come from there!)
		return ParticleSystemManager::getSingleton().createSystemImpl(
				name, quota, resourceGroup, false);
				

	}
    //-----------------------------------------------------------------------
	const String& ParticleSystemFactory::getType(void) const
	{
		return FACTORY_TYPE_NAME;
	}
    //-----------------------------------------------------------------------
	void ParticleSystemFactory::destroyInstance( MovableObject* obj) 
	{
		// use manager, but don't notify SM (we've come from there!)
		ParticleSystemManager::getSingleton().destroySystemImpl(
			obj->getName(), false);

	}
    //-----------------------------------------------------------------------
}

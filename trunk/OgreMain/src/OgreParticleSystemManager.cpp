/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
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
#include "OgreSDDataChunk.h"


namespace Ogre {
    ParticleSystemManager* Singleton<ParticleSystemManager>::ms_Singleton = 0;
    //-----------------------------------------------------------------------
    ParticleSystemManager::ParticleSystemManager()
    {
		mTimeFactor = 1;
    }
    //-----------------------------------------------------------------------
    ParticleSystemManager::~ParticleSystemManager()
    {
        // Templates will be destroyed by by-value STL container
        mSystemTemplates.clear();
        // Destroy all systems 
        ParticleSystemMap::iterator i;
        for (i = mSystems.begin(); i != mSystems.end(); ++i)
        {
            delete i->second;
        }
        mSystems.clear();
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseScript(DataChunk& chunk)
    {
        String line;
        ParticleSystem* pSys;
        std::vector<String> vecparams;

        pSys = 0;

        while(!chunk.isEOF())
        {
            line = chunk.getLine();
            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (pSys == 0)
                {
                    // No current system
                    // So first valid data should be a system name
                    pSys = createTemplate(line);
                    // Skip to and over next {
                    skipToNextOpenBrace(chunk);
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
                        vecparams = line.split("\t ");
                        if (vecparams.size() < 2)
                        {
                            // Oops, bad emitter
                            LogManager::getSingleton().logMessage("Bad particle system emitter line: '"
                                + line + "' in " + pSys->getName());
                            skipToNextCloseBrace(chunk);

                        }
                        skipToNextOpenBrace(chunk);
                        parseNewEmitter(vecparams[1], chunk, pSys);

                    }
                    else if (line.substr(0,8) == "affector")
                    {
                        // new affector
                        // Get typename
                        vecparams = line.split("\t ");
                        if (vecparams.size() < 2)
                        {
                            // Oops, bad emitter
                            LogManager::getSingleton().logMessage("Bad particle system affector line: '"
                                + line + "' in " + pSys->getName());
                            skipToNextCloseBrace(chunk);

                        }
                        skipToNextOpenBrace(chunk);
                        parseNewAffector(vecparams[1],chunk, pSys);
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
    void ParticleSystemManager::parseAllSources(const String& extension)
    {
        std::set<String> particleFiles;

        particleFiles = ResourceManager::_getAllCommonNamesLike("./", extension);

        // Iterate through returned files
        std::set<String>::iterator i;
        for (i = particleFiles.begin(); i != particleFiles.end(); ++i)
        {
            SDDataChunk chunk;
            LogManager::getSingleton().logMessage("Parsing particle script " + *i);
            ResourceManager::_findCommonResourceData(*i, chunk);
            parseScript(chunk);
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
    void ParticleSystemManager::addTemplate(const String& name, const ParticleSystem& sysTemplate)
    {
        mSystemTemplates[name] = sysTemplate;
    }
    //-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::createTemplate(const String& name)
    {
        addTemplate(name, ParticleSystem(name));
        return getTemplate(name);

    }
    //-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::getTemplate(const String& name)
    {
        ParticleTemplateMap::iterator i = mSystemTemplates.find(name);
        if (i != mSystemTemplates.end())
        {
            return &(i->second);
        }
        else
        {
            return 0;
        }
    }
    //-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::createSystem(const String& name, unsigned int quota)
    {
        ParticleSystem* sys = new ParticleSystem(name);
        sys->setParticleQuota(quota);
        mSystems.insert( ParticleSystemMap::value_type( name, sys ) );
        return sys;
    }
    //-----------------------------------------------------------------------
    ParticleSystem* ParticleSystemManager::createSystem(const String& name, const String& templateName)
    {
        // Look up template
        ParticleSystem* pTemplate = getTemplate(templateName);
        if (!pTemplate)
        {
            Except(Exception::ERR_INVALIDPARAMS, "Cannot find required template'" + templateName + "'", "ParticleSystemManager::createSystem");
        }

        ParticleSystem* sys = createSystem(name, pTemplate->getParticleQuota());
        // Copy template settings
        *sys = *pTemplate;
        return sys;
        
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::destroySystem(const String& name)
    {
        ParticleSystemMap::iterator i = mSystems.find(name);
        if (i != mSystems.end())
        {
            delete i->second;
            mSystems.erase(i);
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::destroySystem(ParticleSystem* sys)
    {
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
    ParticleSystem* ParticleSystemManager::getSystem(const String& name)
    {
        ParticleSystemMap::iterator i = mSystems.find(name);
        if (i != mSystems.end())
        {
            return i->second;
        }
        else
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot find particle system '" + name + "'",
                "ParticleSystemManager::getSystem");
        }
    }

    //-----------------------------------------------------------------------
    ParticleEmitter* ParticleSystemManager::_createEmitter(const String& emitterType)
    {
        // Locate emitter type
        ParticleEmitterFactoryMap::iterator pFact = mEmitterFactories.find(emitterType);

        if (pFact == mEmitterFactories.end())
        {
            Except(Exception::ERR_INVALIDPARAMS, "Cannot find requested emitter type.", 
                "ParticleSystemManager::_createEmitter");
        }

        return pFact->second->createEmitter();
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::_destroyEmitter(ParticleEmitter* emitter)
    {
        // Destroy using the factory which created it
        ParticleEmitterFactoryMap::iterator pFact = mEmitterFactories.find(emitter->getType());

        if (pFact == mEmitterFactories.end())
        {
            Except(Exception::ERR_INVALIDPARAMS, "Cannot find emitter factory to destroy emitter.", 
                "ParticleSystemManager::_destroyEmitter");
        }

        pFact->second->destroyEmitter(emitter);
    }
    //-----------------------------------------------------------------------
    ParticleAffector* ParticleSystemManager::_createAffector(const String& affectorType)
    {
        // Locate affector type
        ParticleAffectorFactoryMap::iterator pFact = mAffectorFactories.find(affectorType);

        if (pFact == mAffectorFactories.end())
        {
            Except(Exception::ERR_INVALIDPARAMS, "Cannot find requested affector type.", 
                "ParticleSystemManager::_createAffector");
        }

        return pFact->second->createAffector();

    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::_destroyAffector(ParticleAffector* affector)
    {
        // Destroy using the factory which created it
        ParticleAffectorFactoryMap::iterator pFact = mAffectorFactories.find(affector->getType());

        if (pFact == mAffectorFactories.end())
        {
            Except(Exception::ERR_INVALIDPARAMS, "Cannot find affector factory to destroy affector.", 
                "ParticleSystemManager::_destroyAffector");
        }

        pFact->second->destroyAffector(affector);
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

        // Parse all scripts
        parseAllSources();
    }
    //-----------------------------------------------------------------------
    ParticleSystemManager& ParticleSystemManager::getSingleton(void)
    {
        return Singleton<ParticleSystemManager>::getSingleton();
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseNewEmitter(const String& type, DataChunk& chunk, ParticleSystem* sys)
    {
        // Create new emitter
        ParticleEmitter* pEmit = sys->addEmitter(type);
        // Parse emitter details
        String line;

        while(!chunk.isEOF())
        {
            line = chunk.getLine();
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
                    parseEmitterAttrib(line.toLowerCase(), pEmit);
                }
            }
        }


        
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseNewAffector(const String& type, DataChunk& chunk, ParticleSystem* sys)
    {
        // Create new affector
        ParticleAffector* pAff = sys->addAffector(type);
        // Parse affector details
        String line;

        while(!chunk.isEOF())
        {
            line = chunk.getLine();
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
                    parseAffectorAttrib(line.toLowerCase(), pAff);
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseAttrib(const String& line, ParticleSystem* sys)
    {
        std::vector<String> vecparams;

        // Split params on space
        vecparams = line.split("\t ", 1);

        // Look up first param (command setting)
        if (!sys->setParameter(vecparams[0], vecparams[1]))
        {
            // BAD command. BAD!
            LogManager::getSingleton().logMessage("Bad particle system attribute line: '"
                + line + "' in " + sys->getName());
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::parseEmitterAttrib(const String& line, ParticleEmitter* emit)
    {
        std::vector<String> vecparams;

        // Split params on first space
        vecparams = line.split("\t ", 1);

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
        vecparams = line.split("\t ", 1);

        // Look up first param (command setting)
        if (!aff->setParameter(vecparams[0], vecparams[1]))
        {
            // BAD command. BAD!
            LogManager::getSingleton().logMessage("Bad particle affector attribute line: '"
                + line + "' for affector " + aff->getType());
        }
    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::skipToNextCloseBrace(DataChunk& chunk)
    {
        String line = "";
        while (!chunk.isEOF() && line != "}")
        {
            line = chunk.getLine();
        }

    }
    //-----------------------------------------------------------------------
    void ParticleSystemManager::skipToNextOpenBrace(DataChunk& chunk)
    {
        String line = "";
        while (!chunk.isEOF() && line != "{")
        {
            line = chunk.getLine();
        }

    }
	//-----------------------------------------------------------------------
	Real ParticleSystemManager::getTimeFactor(void) {
		return mTimeFactor;
	}
	//-----------------------------------------------------------------------
	void ParticleSystemManager::setTimeFactor(Real tf) {
		if(tf >= 0) mTimeFactor = tf;
	}
}

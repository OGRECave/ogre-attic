/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#include "OgreOverlayManager.h"
#include "OgreStringVector.h"
#include "OgreGuiManager.h"
#include "OgreOverlayContainer.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreSceneManagerEnumerator.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreEntity.h"
#include "OgrePositionTarget.h"
#include "OgreException.h"
#include "OgreViewport.h"

namespace Ogre {

    //---------------------------------------------------------------------
    template<> OverlayManager *Singleton<OverlayManager>::ms_Singleton = 0;
    OverlayManager* OverlayManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    OverlayManager& OverlayManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //---------------------------------------------------------------------
    OverlayManager::OverlayManager() 
      : mLastViewportWidth(0), 
        mLastViewportHeight(0), 
        mViewportDimensionsChanged(false)
    {

        // Scripting is supported by this manager
        mScriptPatterns.push_back("*.overlay");
		ResourceGroupManager::getSingleton()._registerScriptLoader(this);

    }
    //---------------------------------------------------------------------
    OverlayManager::~OverlayManager()
    {
        destroyAll();

        // Unregister with resource group manager
		ResourceGroupManager::getSingleton()._unregisterScriptLoader(this);
    }
    //---------------------------------------------------------------------
    const StringVector& OverlayManager::getScriptPatterns(void) const
    {
        return mScriptPatterns;
    }
    //---------------------------------------------------------------------
    Real OverlayManager::getLoadingOrder(void) const
    {
        // Load late
        return 1100.0f;
    }
    //---------------------------------------------------------------------
    Overlay* OverlayManager::create(const String& name)
    {
        Overlay* ret = new Overlay(name);
        OverlayMap::iterator i = mOverlayMap.find(name);
        if (i == mOverlayMap.end())
        {
            mOverlayMap[name] = ret;
        }
        else
        {
            Except(Exception::ERR_DUPLICATE_ITEM, 
                "Overlay with name '" + name + "' a;ready exists!",
                "OverlayManager::create");
        }

        return ret;

    }
    //---------------------------------------------------------------------
    Overlay* OverlayManager::getByName(const String& name)
    {
        OverlayMap::iterator i = mOverlayMap.find(name);
        if (i == mOverlayMap.end())
        {
            return 0;
        }
        else
        {
            return i->second;
        }

    }
    //---------------------------------------------------------------------
    void OverlayManager::destroy(const String& name)
    {
        OverlayMap::iterator i = mOverlayMap.find(name);
        if (i == mOverlayMap.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
                "Overlay with name '" + name + "' not found.",
                "OverlayManager::destroy");
        }
        else
        {
            delete i->second;
            mOverlayMap.erase(i);
        }
    }
    //---------------------------------------------------------------------
    void OverlayManager::destroy(Overlay* overlay)
    {
        for (OverlayMap::iterator i = mOverlayMap.begin();
            i != mOverlayMap.end(); ++i)
        {
            if (i->second == overlay)
            {
                delete i->second;
                mOverlayMap.erase(i);
                return;
            }
        }

        Except(Exception::ERR_ITEM_NOT_FOUND, 
            "Overlay not found.",
            "OverlayManager::destroy");
    }
    //---------------------------------------------------------------------
    void OverlayManager::destroyAll(void)
    {
        for (OverlayMap::iterator i = mOverlayMap.begin();
            i != mOverlayMap.end(); ++i)
        {
            delete i->second;
        }
        mOverlayMap.clear();
    }
    //---------------------------------------------------------------------
    OverlayManager::OverlayMapIterator OverlayManager::getOverlayIterator(void)
    {
        return OverlayMapIterator(mOverlayMap.begin(), mOverlayMap.end());
    }
    //---------------------------------------------------------------------
    void OverlayManager::parseScript(DataStreamPtr& stream, const String& groupName)
    {
	    String line;
	    Overlay* pOverlay = 0;
		bool skipLine;

	    while(!stream->eof())
	    {
			bool isTemplate = false;
			skipLine = false;
		    line = stream->getLine();
		    // Ignore comments & blanks
		    if (!(line.length() == 0 || line.substr(0,2) == "//"))
		    {
				if (line.substr(0,8) == "#include")
				{
                    std::vector<String> params = StringUtil::split(line, "\t\n ()<>");
                    DataStreamPtr includeStream = 
                        ResourceGroupManager::getSingleton()._findResource(
                            params[1], groupName);
					parseScript(includeStream, groupName);
					continue;
				}
			    if (!pOverlay)
			    {
				    // No current overlay

					// check to see if there is a template
					if (line.substr(0,8) == "template")
					{
						isTemplate = true;

					}
					else
					{
			
						// So first valid data should be overlay name
						pOverlay = create(line);
						// Skip to and over next {
						skipToNextOpenBrace(stream);
						skipLine = true;
					}
			    }
			    if ((!!pOverlay && !skipLine) || isTemplate)
			    {
				    // Already in overlay
                    std::vector<String> params = StringUtil::split(line, "\t\n ()");


					uint skipParam = 0;
				    if (line == "}")
				    {
					    // Finished overlay
					    pOverlay = 0;
						isTemplate = false;
				    }
				    else if (parseChildren(stream,line, pOverlay, isTemplate, NULL))
						
				    {

				    }
				    else if (params[0+skipParam] == "entity")
				    {
					    // new 3D element
                        if (params.size() != (3+skipParam))
                        {
		                    LogManager::getSingleton().logMessage( 
			                    "Bad entity line: '"
			                    + line + "' in " + pOverlay->getName() + 
			                    ", expecting 'entity meshName(entityName)'");
                                skipToNextCloseBrace(stream);
                        }
                        else
                        {
                            skipToNextOpenBrace(stream);
					        parseNewMesh(stream, params[1+skipParam], params[2+skipParam], pOverlay);
                        }

				    }
				    else
				    {
					    // Attribute
						if (!isTemplate)
						{
							parseAttrib(line, pOverlay);
						}
				    }

			    }

		    }


	    }

    }
    //---------------------------------------------------------------------
    void OverlayManager::_queueOverlaysForRendering(Camera* cam, 
        RenderQueue* pQueue, Viewport* vp)
    {
        // Flag for update pixel-based GUIElements if viewport has changed dimensions
        if (mLastViewportWidth != vp->getActualWidth() || 
            mLastViewportHeight != vp->getActualHeight())
        {
            mViewportDimensionsChanged = true;
            mLastViewportWidth = vp->getActualWidth();
            mLastViewportHeight = vp->getActualHeight();

        }
        else
        {
            mViewportDimensionsChanged = false;
        }

        OverlayMap::iterator i, iend;
        iend = mOverlayMap.end();
        for (i = mOverlayMap.begin(); i != iend; ++i)
        {
            Overlay* o = i->second;
            o->_findVisibleObjects(cam, pQueue);
        }
    }
    //---------------------------------------------------------------------
    void OverlayManager::parseNewElement( DataStreamPtr& stream, String& elemType, String& elemName, 
            bool isContainer, Overlay* pOverlay, bool isTemplate, String templateName, OverlayContainer* container)
    {
        String line;

		OverlayElement* newElement = NULL;
		newElement = 
				GuiManager::getSingleton().createOverlayElementFromTemplate(templateName, elemType, elemName, isTemplate);

			// do not add a template to an overlay

		// add new element to parent
		if (container)
		{
			// Attach to container
			container->addChild(newElement);
		}
		// do not add a template to the overlay. For templates overlay = 0
		else if (!!pOverlay)	
		{
			pOverlay->add2D((OverlayContainer*)newElement);
		}

        while(!stream->eof())
        {
            line = stream->getLine();
            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (line == "}")
                {
                    // Finished element
                    break;
                }
                else
                {
                    if (isContainer && parseChildren(stream,line, pOverlay, isTemplate, static_cast<OverlayContainer*>(newElement)))
                    {
					    // nested children... don't reparse it
                    }
                    else
                    {
                        // Attribute
                        parseElementAttrib(line, pOverlay, newElement);
                    }
                }
            }
        }
    }

    //---------------------------------------------------------------------
    bool OverlayManager::parseChildren( DataStreamPtr& stream, const String& line,
            Overlay* pOverlay, bool isTemplate, OverlayContainer* parent)
	{
		bool ret = false;
		std::vector<String> params;
		uint skipParam =0;
		params = StringUtil::split(line, "\t\n ()");

		if (isTemplate)
		{
			if (params[0] == "template")
			{
				skipParam++;		// the first param = 'template' on a new child element
			}
		}
						
		// top level component cannot be an element, it must be a container unless it is a template
		if (params[0+skipParam] == "container" || (params[0+skipParam] == "element" && (isTemplate || parent != NULL)) )
		{
			String templateName = "";
			ret = true;
			// nested container/element
			if (params.size() > 3+skipParam)
			{
				if (params.size() != 5+skipParam)
				{
					LogManager::getSingleton().logMessage( 
						"Bad element/container line: '"
						+ line + "' in " + parent->getTypeName()+ " " + parent->getName() +
						", expecting ':' templateName");
					skipToNextCloseBrace(stream);
					// barf 
					return ret;
				}
				if (params[3+skipParam] != ":")
				{
					LogManager::getSingleton().logMessage( 
						"Bad element/container line: '"
						+ line + "' in " + parent->getTypeName()+ " " + parent->getName() +
						", expecting ':' for element inheritance");
					skipToNextCloseBrace(stream);
					// barf 
					return ret;
				}

				templateName = params[4+skipParam];
			}

			else if (params.size() != 3+skipParam)
			{
				LogManager::getSingleton().logMessage( 
					"Bad element/container line: '"
						+ line + "' in " + parent->getTypeName()+ " " + parent->getName() +
					", expecting 'element type(name)'");
				skipToNextCloseBrace(stream);
				// barf 
				return ret;
			}
       
			skipToNextOpenBrace(stream);
			parseNewElement(stream, params[1+skipParam], params[2+skipParam], true, pOverlay, isTemplate, templateName, (OverlayContainer*)parent);

		}


		return ret;
	}

    //---------------------------------------------------------------------
    void OverlayManager::parseAttrib( const String& line, Overlay* pOverlay)
    {
        std::vector<String> vecparams;

        // Split params on first space
        vecparams = StringUtil::split(line, "\t ", 1);

        // Look up first param (command setting)
		StringUtil::toLowerCase(vecparams[0]);
        if (vecparams[0] == "zorder")
        {
            pOverlay->setZOrder(StringConverter::parseUnsignedInt(vecparams[1]));
        }
        else
        {
            LogManager::getSingleton().logMessage("Bad overlay attribute line: '"
                + line + "' for overlay " + pOverlay->getName());
        }
    }
    //---------------------------------------------------------------------
    void OverlayManager::parseElementAttrib( const String& line, Overlay* pOverlay, OverlayElement* pElement )
    {
        std::vector<String> vecparams;

        // Split params on first space
        vecparams = StringUtil::split(line, "\t ", 1);

        // Look up first param (command setting)
		StringUtil::toLowerCase(vecparams[0]);
        if (!pElement->setParameter(vecparams[0], vecparams[1]))
        {
            // BAD command. BAD!
            LogManager::getSingleton().logMessage("Bad element attribute line: '"
                + line + "' for element " + pElement->getName() + " in overlay " + 
                (!pOverlay ? "" : pOverlay->getName().c_str() ));
        }
    }
    //-----------------------------------------------------------------------
    void OverlayManager::skipToNextCloseBrace(DataStreamPtr& stream)
    {
        String line = "";
        while (!stream->eof() && line != "}")
        {
            line = stream->getLine();
        }

    }
    //-----------------------------------------------------------------------
    void OverlayManager::skipToNextOpenBrace(DataStreamPtr& stream)
    {
        String line = "";
        while (!stream->eof() && line != "{")
        {
            line = stream->getLine();
        }

    }
    //-----------------------------------------------------------------------
    void OverlayManager::parseNewMesh(DataStreamPtr& stream, String& meshName, String& entityName, 
        Overlay* pOverlay)
    {
        String line;
        StringVector params;

        // NB at this stage any scene manager will do, it's just for allocation not rendering
        SceneManager* sm = SceneManagerEnumerator::getSingleton().getSceneManager(ST_GENERIC);

        // Create entity
        Entity* ent = sm->createEntity(entityName, meshName);
        // Add a new entity via a node
        SceneNode* node = sm->createSceneNode(entityName + "_autoNode");

        node->attachObject(ent);


        // parse extra info
        while(!stream->eof())
        {
            line = stream->getLine();
            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (line == "}")
                {
                    // Finished 
                    break;
                }
                else
                {
                    if (line.substr(0, 8) == "position")
                    {
                        params = StringUtil::split(line, " \t");
                        if (params.size() != 4)
                        {
                            LogManager::getSingleton().logMessage("Bad position attribute line: '"
                                + line + "' for entity " + entityName + " in overlay " + 
                                pOverlay->getName());
                            break;
                        }
                        node->translate(StringConverter::parseReal(params[1]),
                                        StringConverter::parseReal(params[2]), 
                                        StringConverter::parseReal(params[3]));
                    }
                    else if (line.substr(0, 8) == "rotation")
                    {
                        params = StringUtil::split(line, " \t");
                        if (params.size() != 5)
                        {
                            LogManager::getSingleton().logMessage("Bad rotation attribute line: '"
                                + line + "' for entity " + entityName + " in overlay " + 
                                pOverlay->getName());
                            break;
                        }
                        // in file it is angle axis_x axis_y axis_z
                        Vector3 axis(StringConverter::parseReal(params[2]),
                                    StringConverter::parseReal(params[3]),
                                    StringConverter::parseReal(params[4]));
                        node->rotate(axis, StringConverter::parseAngle(params[1]));
                    }
                }
            }
        }



        // Attach node to overlay
        pOverlay->add3D(node);
        
    }
    //---------------------------------------------------------------------
    bool OverlayManager::hasViewportChanged(void) const
    {
        return mViewportDimensionsChanged;
    }
    //---------------------------------------------------------------------
    int OverlayManager::getViewportHeight(void) const
    {
        return mLastViewportHeight;
    }
    //---------------------------------------------------------------------
    int OverlayManager::getViewportWidth(void) const
    {
        return mLastViewportWidth;
    }
    //---------------------------------------------------------------------
    Real OverlayManager::getViewportAspectRatio(void) const
    {
        return (Real)mLastViewportHeight / (Real)mLastViewportWidth;
    }
    //---------------------------------------------------------------------
}


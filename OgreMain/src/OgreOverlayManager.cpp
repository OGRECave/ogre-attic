/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2002 The OGRE Team
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
#include "OgreOverlay.h"
#include "OgreGuiManager.h"
#include "OgreOverlayContainer.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreSceneManagerEnumerator.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreEntity.h"
#include "OgrePositionTarget.h"
#include "OgreEventProcessor.h"
#include "OgreException.h"
#include "OgreViewport.h"
#include "OgreSDDataChunk.h"

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
    OverlayManager::OverlayManager() :
        mEventDispatcher(this), mCursorGuiInitialised(false), 
        mLastViewportWidth(0), mLastViewportHeight(0), 
        mViewportDimensionsChanged(false)
    {
		mCursorGuiRegistered = 0;
		mCursorLevelOverlay = 0;
    }
    //---------------------------------------------------------------------
    OverlayManager::~OverlayManager()
    {
    }
    //---------------------------------------------------------------------
    void OverlayManager::parseOverlayFile(DataChunk& chunk)
    {
	    String line;
	    Overlay* pOverlay;
		bool skipLine;

	    pOverlay = 0;

	    while(!chunk.isEOF())
	    {
			bool isTemplate = false;
			skipLine = false;
		    line = chunk.getLine();
		    // Ignore comments & blanks
		    if (!(line.length() == 0 || line.substr(0,2) == "//"))
		    {
				if (line.substr(0,8) == "#include")
				{
                    std::vector<String> params = StringUtil::split(line, "\t\n ()<>");
					loadAndParseOverlayFile(params[1]);
					continue;
				}
			    if (pOverlay == 0)
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
						pOverlay = (Overlay*)create(line);
						// Skip to and over next {
						skipToNextOpenBrace(chunk);
						skipLine = true;
					}
			    }
			    if ((pOverlay && !skipLine) || isTemplate)
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
				    else if (parseChildren(chunk,line, pOverlay, isTemplate, NULL))
						
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
                                skipToNextCloseBrace(chunk);
                        }
                        else
                        {
                            skipToNextOpenBrace(chunk);
					        parseNewMesh(chunk, params[1+skipParam], params[2+skipParam], pOverlay);
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
    void OverlayManager::parseAllSources(const String& extension)
    {
        StringVector overlayFiles;

        std::vector<ArchiveEx*>::iterator i = mVFS.begin();

        // Specific archives
        for (; i != mVFS.end(); ++i)
        {
            overlayFiles = (*i)->getAllNamesLike( "./", extension);
            for (StringVector::iterator si = overlayFiles.begin(); si!=overlayFiles.end(); ++si)
            {
                parseOverlayFile(*i,si[0]);
            }

        }
        // search common archives
        for (i = mCommonVFS.begin(); i != mCommonVFS.end(); ++i)
        {
            overlayFiles = (*i)->getAllNamesLike( "./", extension);
            for (StringVector::iterator si = overlayFiles.begin(); si!=overlayFiles.end(); ++si)
            {
                parseOverlayFile(*i,si[0]);
            }
        }
    }
    //---------------------------------------------------------------------
    void OverlayManager::loadAndParseOverlayFile(const String& filename)
    {
		bool isLoaded = false;
        for (StringVector::iterator i = mLoadedOverlays.begin(); i != mLoadedOverlays.end(); ++i)
        {
			if (*i == filename)
			{
				LogManager::getSingleton().logMessage( 
					"Skipping loading overlay include: '"
					+ filename+ " as it is already loaded.");
				isLoaded = true;
				break;

			}
        }
		if (!isLoaded)
		{

			std::vector<ArchiveEx*>::iterator i = mVFS.begin();

			// Specific archives
			for (; i != mVFS.end(); ++i)
			{
				if ((*i)->fileTest(filename))
				{
					parseOverlayFile(*i,filename);
				}

			}
			// search common archives
			for (i = mCommonVFS.begin(); i != mCommonVFS.end(); ++i)
			{
				if ((*i)->fileTest(filename))
				{
					parseOverlayFile(*i,filename);
				}
			}
		}
    }
    //---------------------------------------------------------------------
    void OverlayManager::parseOverlayFile(ArchiveEx* pArchiveEx, const String& name)
	{
        DataChunk* pChunk;
        SDDataChunk dat; 
		pChunk = &dat;
        pArchiveEx->fileRead(name, &pChunk );
        parseOverlayFile(dat);
		mLoadedOverlays.push_back(name);
	}


    //---------------------------------------------------------------------
    Resource* OverlayManager::create( const String& name)
    {
        Overlay* s = new Overlay(name);
        load(s,1);
        return s;
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

        ResourceMap::iterator i, iend;
        iend = mResources.end();
        for (i = mResources.begin(); i != iend; ++i)
        {
            Overlay* o = (Overlay*)i->second;
            o->_findVisibleObjects(cam, pQueue);
        }
    }
    //---------------------------------------------------------------------
    void OverlayManager::parseNewElement( DataChunk& chunk, String& elemType, String& elemName, 
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
		else if (pOverlay)	
		{
			pOverlay->add2D((OverlayContainer*)newElement);
		}

        while(!chunk.isEOF())
        {
            line = chunk.getLine();
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
                    if (isContainer && parseChildren(chunk,line, pOverlay, isTemplate, static_cast<OverlayContainer*>(newElement)))
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
    bool OverlayManager::parseChildren( DataChunk& chunk, const String& line,
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
					skipToNextCloseBrace(chunk);
					// barf 
					return ret;
				}
				if (params[3+skipParam] != ":")
				{
					LogManager::getSingleton().logMessage( 
						"Bad element/container line: '"
						+ line + "' in " + parent->getTypeName()+ " " + parent->getName() +
						", expecting ':' for element inheritance");
					skipToNextCloseBrace(chunk);
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
				skipToNextCloseBrace(chunk);
				// barf 
				return ret;
			}
       
			skipToNextOpenBrace(chunk);
			parseNewElement(chunk, params[1+skipParam], params[2+skipParam], true, pOverlay, isTemplate, templateName, (OverlayContainer*)parent);

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
                (pOverlay ? pOverlay->getName().c_str() : ""));
        }
    }
    //-----------------------------------------------------------------------
    void OverlayManager::skipToNextCloseBrace(DataChunk& chunk)
    {
        String line = "";
        while (!chunk.isEOF() && line != "}")
        {
            line = chunk.getLine();
        }

    }
    //-----------------------------------------------------------------------
    void OverlayManager::skipToNextOpenBrace(DataChunk& chunk)
    {
        String line = "";
        while (!chunk.isEOF() && line != "{")
        {
            line = chunk.getLine();
        }

    }
    //-----------------------------------------------------------------------
    void OverlayManager::parseNewMesh(DataChunk& chunk, String& meshName, String& entityName, 
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
        while(!chunk.isEOF())
        {
            line = chunk.getLine();
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

	//-----------------------------------------------------------------------------

	PositionTarget* OverlayManager::getPositionTargetAt(Real x, Real y)
	{
		PositionTarget* ret = NULL;
		int currZ = -1;
        ResourceMap::iterator i, iend;
        iend = mResources.end();
        for (i = mResources.begin(); i != iend; ++i)
        {
            Overlay* o = (Overlay*)i->second;
			int z = o->getZOrder();
			if (z > currZ && o->isVisible())
			{
				PositionTarget* elementFound = static_cast<MouseTarget*>(o->findElementAt(x,y));	// OverlayElements are MouseTargets and MouseMotionTargets,
																									// you need to choose one to static cast
				if (elementFound)
				{
					currZ = z;
					ret = elementFound;
				}
			}
        }

		return ret;
	}

	//-----------------------------------------------------------------------------
    void OverlayManager::processEvent(InputEvent* e)
    {
        MouseMotionListenerList::iterator i, iEnd;

        mEventDispatcher.dispatchEvent(e);

            // process for cursor listeners
        switch (e->getID())
        {
        case MouseEvent::ME_MOUSE_MOVED :
            iEnd = mMouseMotionListenerList.end();
            for (i = mMouseMotionListenerList.begin(); i != iEnd; ++i)
                (*i)->mouseMoved(static_cast<MouseEvent*>(e));
            break;

        case MouseEvent::ME_MOUSE_DRAGGED :
            iEnd = mMouseMotionListenerList.end();
            for (i = mMouseMotionListenerList.begin(); i != iEnd; ++i)
                (*i)->mouseDragged(static_cast<MouseEvent*>(e));
            break;
        }
    }

	//-----------------------------------------------------------------------------
	void OverlayManager::setDefaultCursorGui(OverlayContainer* cursor, MouseMotionListener* cursorListener)
	{
		mCursorGuiRegistered = cursor;
		//mCursorListener = cursorListener;
        mCursorGuiInitialised = false;
       
        //if (mCursorListener != 0)
        //    addMouseMotionListener(mCursorListener);
	}

	//-----------------------------------------------------------------------------
	void OverlayManager::setCursorGui(OverlayContainer* cursor)
	{
            // remove old cursor and listener, if any
        if (mCursorGuiRegistered != 0)
            mCursorGuiRegistered->hide();
        //if (mCursorListener != 0)
        //    removeMouseMotionListener(mCursorListener);

		mCursorGuiRegistered  = cursor;
		//mCursorListener       = cursorListener;
        mCursorGuiInitialised = true;

            // add new cursor, if any
        if (mCursorGuiRegistered != 0)
            mCursorGuiRegistered->show();            
        //if (mCursorListener != 0)
        //    addMouseMotionListener(mCursorListener);
	}

	//-----------------------------------------------------------------------------
	OverlayContainer* OverlayManager::getCursorGui()
	{
        if(!mCursorGuiInitialised)
        {
            mCursorGuiRegistered->initialise();
            mCursorGuiInitialised = true;
        }
		return mCursorGuiRegistered;
	}

    //-----------------------------------------------------------------------------
    void OverlayManager::addMouseMotionListener(MouseMotionListener* l)
    {
        mMouseMotionListenerList.push_back(l);
    }

    //-----------------------------------------------------------------------------
    void OverlayManager::removeMouseMotionListener(MouseMotionListener* l)
    {
        MouseMotionListenerList::iterator i, iEnd;

        iEnd = mMouseMotionListenerList.end();
        for (i = mMouseMotionListenerList.begin(); i != iEnd; ++i)
            if (*i == l)
            {
                mMouseMotionListenerList.erase(i);
                break;
            }
    }

	//-----------------------------------------------------------------------------
	void OverlayManager::createCursorOverlay()
	{
		mCursorLevelOverlay = static_cast<Overlay* > (create("CursorLevelOverlay"));
		mCursorLevelOverlay->setZOrder(600);
		mCursorLevelOverlay->show();
		EventProcessor::getSingleton().addEventTarget(this);

		// register the new cursor and display it
		if (mCursorGuiRegistered/* && mCursorListener*/)	
		{
			mCursorLevelOverlay->add2D(mCursorGuiRegistered);
            mCursorGuiRegistered->show();
		}
	}

}


/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/

#include "OgreOverlayManager.h"
#include "OgreStringVector.h"
#include "OgreOverlay.h"
#include "OgreGuiManager.h"
#include "OgreGuiContainer.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"

namespace Ogre {

    //---------------------------------------------------------------------
    template<> OverlayManager *Singleton<OverlayManager>::ms_Singleton = 0;
    //---------------------------------------------------------------------
    OverlayManager::OverlayManager()
    {
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

	    pOverlay = 0;

	    while(!chunk.isEOF())
	    {
		    line = chunk.getLine();
		    // Ignore comments & blanks
		    if (!(line.length() == 0 || line.substr(0,2) == "//"))
		    {
			    if (pOverlay == 0)
			    {
				    // No current overlay
				    // So first valid data should be overlay name
				    pOverlay = (Overlay*)create(line);
				    // Skip to and over next {
				    skipToNextOpenBrace(chunk);
			    }
			    else
			    {
				    // Already in overlay
				    if (line == "}")
				    {
					    // Finished overlay
					    pOverlay = 0;
				    }
				    else if (line.substr(0,9) == "container")
				    {
					    // new 2D element
                        std::vector<String> params = line.split("\t\n ()");
                        if (params.size() != 3)
                        {
		                    LogManager::getSingleton().logMessage( 
			                    "Bad container line: '"
			                    + line + "' in " + pOverlay->getName() + 
			                    ", expecting 'container type(name)'");
                                skipToNextCloseBrace(chunk);
                        }
                        else
                        {
                            skipToNextOpenBrace(chunk);
					        parseNewElement(chunk, params[1], params[2], true, pOverlay);
                        }

				    }
				    else
				    {
					    // Attribute
					    parseAttrib(line, pOverlay);
				    }

			    }

		    }


	    }

    }
    //---------------------------------------------------------------------
    void OverlayManager::parseAllSources(const String& extension)
    {
        StringVector overlayFiles;
        DataChunk* pChunk;

        std::vector<ArchiveEx*>::iterator i = mVFS.begin();

        // Specific archives
        for (; i != mVFS.end(); ++i)
        {
            overlayFiles = (*i)->getAllNamesLike( "./", extension);
            for (StringVector::iterator si = overlayFiles.begin(); si!=overlayFiles.end(); ++si)
            {
                DataChunk dat; pChunk = &dat;
                (*i)->fileRead(si[0], &pChunk );
                parseOverlayFile(dat);
            }

        }
        // search common archives
        for (i = mCommonVFS.begin(); i != mCommonVFS.end(); ++i)
        {
            overlayFiles = (*i)->getAllNamesLike( "./", extension);
            for (StringVector::iterator si = overlayFiles.begin(); si!=overlayFiles.end(); ++si)
            {
                DataChunk dat; pChunk = &dat;
                (*i)->fileRead(si[0], &pChunk );
                parseOverlayFile(dat);
            }
        }
    }
    //---------------------------------------------------------------------
    Resource* OverlayManager::create( const String& name)
    {
        Overlay* s = new Overlay(name);
        load(s,1);
        return s;
    }
    //---------------------------------------------------------------------
    void OverlayManager::_queueOverlaysForRendering(Camera* cam, RenderQueue* pQueue)
    {
        ResourceMap::iterator i, iend;
        iend = mResources.end();
        for (i = mResources.begin(); i != iend; ++i)
        {
            Overlay* o = (Overlay*)i->second;
            o->_findVisibleObjects(cam, pQueue);
        }
    }
    //---------------------------------------------------------------------
    OverlayManager& OverlayManager::getSingleton(void)
    {
        return Singleton<OverlayManager>::getSingleton();
    }
    //---------------------------------------------------------------------
    void OverlayManager::parseNewElement( DataChunk& chunk, String& elemType, String& elemName, 
            bool isContainer, Overlay* pOverlay, GuiContainer* container)
    {
        String line;
        std::vector<String> params;
        GuiElement* newElement = 
            GuiManager::getSingleton().createGuiElement(elemType, elemName);

        // add new element to parent
        if (container)
        {
            // Attach to container
            if (isContainer)
            {
                container->addChild((GuiContainer*)newElement);
            }
            else
            {
                container->addChild((GuiContainer*)newElement);
            }
        }
        else
        {
            pOverlay->add2D((GuiContainer*)newElement);
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
                    if (line.substr(0,9) == "container" && isContainer)
                    {
					    // nested container
                        params = line.split("\t\n ()");
                        if (params.size() != 3)
                        {
		                    LogManager::getSingleton().logMessage( 
			                    "Bad container line: '"
			                    + line + "' in " + elemType + " " + elemName +
                                ", expecting 'container type(name)'");
                            skipToNextCloseBrace(chunk);
                            return;
                        }
                       
                        skipToNextOpenBrace(chunk);
					    parseNewElement(chunk, params[1], params[2], true, pOverlay, (GuiContainer*)newElement);

                    }
                    else if (line.substr(0,7) == "element" && isContainer)
                    {
					    // nested element
                        params = line.split("\t\n ()");
                        if (params.size() != 3)
                        {
		                    LogManager::getSingleton().logMessage( 
			                    "Bad element line: '"
			                    + line + "' in " + elemType + " " + elemName +
                                ", expecting 'container type(name)'");
                            skipToNextCloseBrace(chunk);
                            return;
                        }
                       
                        skipToNextOpenBrace(chunk);
					    parseNewElement(chunk, params[1], params[2], false, pOverlay, (GuiContainer*)newElement);
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
    void OverlayManager::parseAttrib( const String& line, Overlay* pOverlay)
    {
        std::vector<String> vecparams;

        // Split params on first space
        vecparams = line.split("\t ", 1);

        // Look up first param (command setting)
        if (vecparams[0].toLowerCase() == "zorder")
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
    void OverlayManager::parseElementAttrib( const String& line, Overlay* pOverlay, GuiElement* pElement )
    {
        std::vector<String> vecparams;

        // Split params on first space
        vecparams = line.split("\t ", 1);

        // Look up first param (command setting)
        if (!pElement->setParameter(vecparams[0].toLowerCase(), vecparams[1]))
        {
            // BAD command. BAD!
            LogManager::getSingleton().logMessage("Bad element attribute line: '"
                + line + "' for element " + pElement->getName() + " in overlay " + 
                pOverlay->getName());
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





}


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

#include "OgreString.h"
#include "OgreGuiManager.h"
#include "OgreGuiElementFactories.h"
#include "OgreException.h"
#include "OgreStringVector.h"
#include "OgreRoot.h"
#include "OgreCursorGuiElement.h"
#include "OgreOverlayManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    OverlayElementFactory* pCursorFactory = NULL;
    OverlayElementFactory* pPanelFactory = NULL;
    OverlayElementFactory* pBorderPanelFactory = NULL;
    OverlayElementFactory* pTextAreaFactory = NULL;
    OverlayElementFactory* pTextBoxFactory = NULL;
    OverlayElementFactory* pButtonFactory = NULL;
    OverlayElementFactory* pBorderButtonFactory = NULL;
    OverlayElementFactory* pListFactory = NULL;
    OverlayElementFactory* pScrollBarFactory = NULL;
    OverlayElementFactory* pPopupMenuFactory = NULL;
    OverlayElementFactory* pTTYFactory = NULL;
  	CursorGuiElement*  pCursorGui = NULL;
    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    extern "C" void dllStartPlugin(void)
    {
        SET_TERM_HANDLER;
        
        pCursorFactory = new CursorGuiElementFactory();
        GuiManager::getSingleton().addOverlayElementFactory(pCursorFactory);

        pPanelFactory = new PanelOverlayElementFactory();
        GuiManager::getSingleton().addOverlayElementFactory(pPanelFactory);

        pBorderPanelFactory = new BorderPanelOverlayElementFactory();
        GuiManager::getSingleton().addOverlayElementFactory(pBorderPanelFactory);
        
        pTextAreaFactory = new TextAreaOverlayElementFactory();
        GuiManager::getSingleton().addOverlayElementFactory(pTextAreaFactory);

        pTextBoxFactory = new TextBoxGuiElementFactory();
        GuiManager::getSingleton().addOverlayElementFactory(pTextBoxFactory);

		pButtonFactory = new ButtonGuiElementFactory();
        GuiManager::getSingleton().addOverlayElementFactory(pButtonFactory);

		pBorderButtonFactory = new BorderButtonGuiElementFactory();
        GuiManager::getSingleton().addOverlayElementFactory(pBorderButtonFactory);

		pListFactory = new ListGuiElementFactory();
        GuiManager::getSingleton().addOverlayElementFactory(pListFactory);

		pScrollBarFactory = new ScrollBarGuiElementFactory();
        GuiManager::getSingleton().addOverlayElementFactory(pScrollBarFactory);

		pPopupMenuFactory = new PopupMenuGuiElementFactory();
        GuiManager::getSingleton().addOverlayElementFactory(pPopupMenuFactory);
        
        pTTYFactory = new TTYGuiElementFactory();
        GuiManager::getSingleton().addOverlayElementFactory(pTTYFactory);

            // create default cursor
		pCursorGui = new CursorGuiElement("Cursor default");
		OverlayManager::getSingleton().setDefaultCursorGui(pCursorGui, pCursorGui);
    } 

    //-----------------------------------------------------------------------
    extern "C" void dllStopPlugin(void)
    {
        delete pPanelFactory;
        delete pBorderPanelFactory;
        delete pTextAreaFactory;
        delete pTextBoxFactory;
		delete pButtonFactory;
		delete pScrollBarFactory;
        delete pPopupMenuFactory;
		delete pBorderButtonFactory;
		delete pListFactory;
		delete pCursorGui;
        delete pTTYFactory;
            delete pCursorFactory; 

    }

}


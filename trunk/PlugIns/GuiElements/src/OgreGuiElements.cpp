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
    GuiElementFactory* pCursorFactory = NULL;
    GuiElementFactory* pPanelFactory = NULL;
    GuiElementFactory* pBorderPanelFactory = NULL;
    GuiElementFactory* pTextAreaFactory = NULL;
    GuiElementFactory* pTextBoxFactory = NULL;
    GuiElementFactory* pButtonFactory = NULL;
    GuiElementFactory* pBorderButtonFactory = NULL;
    GuiElementFactory* pListFactory = NULL;
    GuiElementFactory* pScrollBarFactory = NULL;
    GuiElementFactory* pPopupMenuFactory = NULL;
    GuiElementFactory* pTTYFactory = NULL;
  	CursorGuiElement*  pCursorGui = NULL;
    //-----------------------------------------------------------------------

    //-----------------------------------------------------------------------
    extern "C" void dllStartPlugin(void)
    {
        SET_TERM_HANDLER;
        
        pCursorFactory = new CursorGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pCursorFactory);

        pPanelFactory = new PanelGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pPanelFactory);

        pBorderPanelFactory = new BorderPanelGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pBorderPanelFactory);
        
        pTextAreaFactory = new TextAreaGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pTextAreaFactory);

        pTextBoxFactory = new TextBoxGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pTextBoxFactory);

		pButtonFactory = new ButtonGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pButtonFactory);

		pBorderButtonFactory = new BorderButtonGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pBorderButtonFactory);

		pListFactory = new ListGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pListFactory);

		pScrollBarFactory = new ScrollBarGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pScrollBarFactory);

		pPopupMenuFactory = new PopupMenuGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pPopupMenuFactory);
        
        pTTYFactory = new TTYGuiElementFactory();
        GuiManager::getSingleton().addGuiElementFactory(pTTYFactory);

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


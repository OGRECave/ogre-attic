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
#include "OgreGuiElementCommands.h"
#include "OgreGuiElement.h"
#include "OgreStringConverter.h"


namespace Ogre {

    namespace GuiElementCommands {

        //-----------------------------------------------------------------------
        String CmdLeft::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<GuiElement*>(target)->getLeft() );
        }
        void CmdLeft::doSet(void* target, const String& val)
        {
            static_cast<GuiElement*>(target)->setLeft(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdTop::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<GuiElement*>(target)->getTop() );
        }
        void CmdTop::doSet(void* target, const String& val)
        {
            static_cast<GuiElement*>(target)->setTop(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdWidth::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<GuiElement*>(target)->getWidth() );
        }
        void CmdWidth::doSet(void* target, const String& val)
        {
            static_cast<GuiElement*>(target)->setWidth(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdHeight::doGet(void* target)
        {
            return StringConverter::toString(
                static_cast<GuiElement*>(target)->getHeight() );
        }
        void CmdHeight::doSet(void* target, const String& val)
        {
            static_cast<GuiElement*>(target)->setHeight(StringConverter::parseReal(val));
        }
        //-----------------------------------------------------------------------
        String CmdMaterial::doGet(void* target)
        {
            return static_cast<GuiElement*>(target)->getMaterialName();
        }
        void CmdMaterial::doSet(void* target, const String& val)
        {
			if (val != "")
			{
				static_cast<GuiElement*>(target)->setMaterialName(val);
			}
        }
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        String CmdCaption::doGet(void* target)
        {
            return static_cast<GuiElement*>(target)->getCaption();
        }
        void CmdCaption::doSet(void* target, const String& val)
        {
            static_cast<GuiElement*>(target)->setCaption(val);
        }
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        String CmdMetricsMode::doGet(void* target)
        {
            GuiMetricsMode gmm = 
                static_cast<GuiElement*>(target)->getMetricsMode();
            if (gmm == GMM_PIXELS)
            {
                return "pixels";
            }
            else
            {
                return "relative";
            }
        }
        void CmdMetricsMode::doSet(void* target, const String& val)
        {
            if (val == "pixels")
            {
                static_cast<GuiElement*>(target)->setMetricsMode(GMM_PIXELS);
            }
            else
            {
                static_cast<GuiElement*>(target)->setMetricsMode(GMM_RELATIVE);
            }
        }
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        String CmdHorizontalAlign::doGet(void* target)
        {
            GuiHorizontalAlignment gha = 
                static_cast<GuiElement*>(target)->getHorizontalAlignment();
            switch(gha)
            {
            case GHA_LEFT:
                return "left";
            case GHA_RIGHT:
                return "right";
            case GHA_CENTER:
                return "center";
            }
            // To keep compiler happy
            return "center";
        }
        void CmdHorizontalAlign::doSet(void* target, const String& val)
        {
            if (val == "left")
            {
                static_cast<GuiElement*>(target)->setHorizontalAlignment(GHA_LEFT);
            }
            else if (val == "right")
            {
                static_cast<GuiElement*>(target)->setHorizontalAlignment(GHA_RIGHT);
            }
            else
            {
                static_cast<GuiElement*>(target)->setHorizontalAlignment(GHA_CENTER);
            }
        }
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        String CmdVerticalAlign::doGet(void* target)
        {
            GuiVerticalAlignment gva = 
                static_cast<GuiElement*>(target)->getVerticalAlignment();
            switch(gva)
            {
            case GVA_TOP:
                return "top";
            case GVA_BOTTOM:
                return "bottom";
            case GVA_CENTER:
                return "center";
            }
            // To keep compiler happy
            return "center";
        }
        void CmdVerticalAlign::doSet(void* target, const String& val)
        {
            if (val == "top")
            {
                static_cast<GuiElement*>(target)->setVerticalAlignment(GVA_TOP);
            }
            else if (val == "bottom")
            {
                static_cast<GuiElement*>(target)->setVerticalAlignment(GVA_BOTTOM);
            }
            else
            {
                static_cast<GuiElement*>(target)->setVerticalAlignment(GVA_CENTER);
            }
        }
        //-----------------------------------------------------------------------
    }
}


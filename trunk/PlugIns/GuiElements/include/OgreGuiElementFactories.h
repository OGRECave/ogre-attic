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

#ifndef __GuiElementFactories_H__
#define __GuiElementFactories_H__

#include "OgreGuiElementPrerequisites.h"
#include "OgreGuiElementFactory.h"
#include "OgreCursorGuiElement.h"
#include "OgrePanelGuiElement.h"
#include "OgreBorderPanelGuiElement.h"
#include "OgreButtonGuiElement.h"
#include "OgreBorderButtonGuiElement.h"
#include "OgreListGuiElement.h"
#include "OgrePopupMenuGuiElement.h"
#include "OgreTextAreaGuiElement.h"
#include "OgreTextBoxGuiElement.h"
#include "OgreTTYGuiElement.h"


// This file includes definitions for all the GuiElement factories provided with OGRE
// These classes are exported incase anyone wants to extend them

namespace Ogre {

    /** Factory for creating PanelGuiElement instances. */
    class _OgreGuiElementExport CursorGuiElementFactory: public GuiElementFactory
    {
    public:
        /** See GuiElementFactory */
        GuiElement* createGuiElement(const String& instanceName)
        {
            return new CursorGuiElement(instanceName);
        }
        /** See GuiElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "Cursor";
            return name;
        }
    };

    /** Factory for creating PanelGuiElement instances. */
    class _OgreGuiElementExport PanelGuiElementFactory: public GuiElementFactory
    {
    public:
        /** See GuiElementFactory */
        GuiElement* createGuiElement(const String& instanceName)
        {
            return new PanelGuiElement(instanceName);
        }
        /** See GuiElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "Panel";
            return name;
        }
    };

    /** Factory for creating BorderPanelGuiElement instances. */
    class _OgreGuiElementExport BorderPanelGuiElementFactory: public GuiElementFactory
    {
    public:
        /** See GuiElementFactory */
        GuiElement* createGuiElement(const String& instanceName)
        {
            return new BorderPanelGuiElement(instanceName);
        }
        /** See GuiElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "BorderPanel";
            return name;
        }
    };

    /** Factory for creating TextAreaGuiElement instances. */
    class _OgreGuiElementExport TextAreaGuiElementFactory: public GuiElementFactory
    {
    public:
        /** See GuiElementFactory */
        GuiElement* createGuiElement(const String& instanceName)
        {
            return new TextAreaGuiElement(instanceName);
        }
        /** See GuiElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "TextArea";
            return name;
        }
    };

    /** Factory for creating TextBoxGuiElement instances. */
    class _OgreGuiElementExport TextBoxGuiElementFactory: public GuiElementFactory
    {
    public:
        /** See GuiElementFactory */
        GuiElement* createGuiElement(const String& instanceName)
        {
            return new TextBoxGuiElement(instanceName);
        }
        /** See GuiElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "TextBox";
            return name;
        }
    };


    /** Factory for creating ButtonGuiElement instances. */
    class _OgreGuiElementExport ButtonGuiElementFactory: public GuiElementFactory
    {
    public:
        /** See GuiElementFactory */
        GuiElement* createGuiElement(const String& instanceName)
        {
            return new ButtonGuiElement(instanceName);
        }
        /** See GuiElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "Button";
            return name;
        }
    };

    /** Factory for creating ButtonGuiElement instances. */
    class _OgreGuiElementExport BorderButtonGuiElementFactory: public GuiElementFactory
    {
    public:
        /** See GuiElementFactory */
        GuiElement* createGuiElement(const String& instanceName)
        {
            return new BorderButtonGuiElement(instanceName);
        }
        /** See GuiElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "BorderButton";
            return name;
        }
    };

    /** Factory for creating ButtonGuiElement instances. */
    class _OgreGuiElementExport ListGuiElementFactory: public GuiElementFactory
    {
    public:
        /** See GuiElementFactory */
        GuiElement* createGuiElement(const String& instanceName)
        {
            return new ListGuiElement(instanceName);
        }
        /** See GuiElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "List";
            return name;
        }
    };
 

    /** Factory for creating ButtonGuiElement instances. */
    class _OgreGuiElementExport ScrollBarGuiElementFactory: public GuiElementFactory
    {
    public:
        /** See GuiElementFactory */
        GuiElement* createGuiElement(const String& instanceName)
        {
            return new ScrollBarGuiElement(instanceName);
        }
        /** See GuiElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "ScrollBar";
            return name;
        }
    };
    /** Factory for creating ButtonGuiElement instances. */
    class _OgreGuiElementExport PopupMenuGuiElementFactory: public GuiElementFactory
    {
    public:
        /** See GuiElementFactory */
        GuiElement* createGuiElement(const String& instanceName)
        {
            return new PopupMenuGuiElement(instanceName);
        }
        /** See GuiElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "PopupMenu";
            return name;
        }
    };
    /** Factory for creating TTYGuiElement instances. */
    class _OgreGuiElementExport TTYGuiElementFactory: public GuiElementFactory
    {
    public:
        /** See GuiElementFactory */
        GuiElement* createGuiElement(const String& instanceName)
        {
            return new TTYGuiElement(instanceName);
        }
        /** See GuiElementFactory */
        const String& getTypeName(void) const
        {
            static String name = "TTY";
            return name;
        }
    };

}


#endif


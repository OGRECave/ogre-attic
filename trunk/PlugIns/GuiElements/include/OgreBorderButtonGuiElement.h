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
#ifndef __BorderButtonGuiElement_H__
#define __BorderButtonGuiElement_H__

#include "OgreGuiElementPrerequisites.h"
#include "OgreBorderPanelGuiElement.h"
#include "OgreGuiPressable.h"

namespace Ogre {

    
	/**
	 * The listener interface for receiving "interesting" mouse events 
	 * (press, release, click, enter, and exit) on a component.
	 * (To track mouse moves and mouse drags, use the MouseMotionListener.)
	 * <P>
	 * The class that is interested in processing a mouse event
	 * either implements this interface (and all the methods it
	 * contains) or extends the abstract <code>MouseAdapter</code> class
	 * (overriding only the methods of interest).
	 * <P>
	 * The listener object created from that class is then registered with a
	 * component using the component's <code>addMouseListener</code> 
	 * method. A mouse event is generated when the mouse is pressed, released
	 * clicked (pressed and released). A mouse event is also generated when
	 * the mouse cursor enters or leaves a component. When a mouse event
	 * occurs the relevant method in the listener object is invoked, and 
	 * the <code>MouseEvent</code> is passed to it.
	 *
	 */
	class _OgreGuiElementExport BorderButtonGuiElement : public BorderPanelGuiElement, public GuiPressable
    {
    protected:
		String mBorderDownMaterialName;
		String mBorderUpMaterialName;
		String mBorderHiliteDownMaterialName;
		String mBorderHiliteUpMaterialName;
		String mBorderDisabledMaterialName;

        static String msTypeName;
		GuiElement* mInsideObject;
		
		void changeChild(GuiElement* e, Real add);

	public :
		const String& getTypeName(void) const;
	    void addBaseParameters(void);

		BorderButtonGuiElement(const String& name);
		void updateMaterials(bool init = false);
		void processEvent(InputEvent* e) ;

        /** Sets the name of the material to use for the borders. */
        void setBorderDownMaterialName(const String& name);
        /** Gets the name of the material to use for the borders. */
        const String& getBorderDownMaterialName(void) const;

        /** Sets the name of the material to use for the borders. */
        void setBorderUpMaterialName(const String& name);
        /** Gets the name of the material to use for the borders. */
        const String& getBorderUpMaterialName(void) const;

        /** Sets the name of the material to use for the hilited borders. */
        void setBorderHiliteDownMaterialName(const String& name);
        /** Gets the name of the material to use for the hilited borders. */
        const String& getBorderHiliteDownMaterialName(void) const;

        /** Sets the name of the material to use for the hilited borders. */
        void setBorderHiliteUpMaterialName(const String& name);
        /** Gets the name of the material to use for the hilited borders. */
        const String& getBorderHiliteUpMaterialName(void) const;

        /** Sets the name of the material to use for the disabled borders. */
        void setBorderDisabledMaterialName(const String& name);
        /** Gets the name of the material to use for the disabled borders. */
        const String& getBorderDisabledMaterialName(void) const;

	    void setButtonCaption(const String& templateName, const String& name);
	    String getButtonCaption() const;

        /** Command object for specifying the Material for the border (see ParamCommand).*/
        class CmdBorderDownMaterial : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for specifying the Material for the border (see ParamCommand).*/
        class CmdBorderUpMaterial : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for specifying the Material for the border (see ParamCommand).*/
        class CmdBorderHiliteDownMaterial : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for specifying the Material for the border (see ParamCommand).*/
        class CmdBorderHiliteUpMaterial : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for specifying the Material for the border (see ParamCommand).*/
        class CmdBorderDisabledMaterial : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for specifying the Material for the border (see ParamCommand).*/
        class CmdBorderButtonCaption : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        static CmdBorderDownMaterial msCmdBorderDownMaterial;
        static CmdBorderUpMaterial msCmdBorderUpMaterial;
        static CmdBorderHiliteDownMaterial msCmdBorderHiliteDownMaterial;
        static CmdBorderHiliteUpMaterial msCmdBorderHiliteUpMaterial;
        static CmdBorderDisabledMaterial msCmdBorderDisabledMaterial;
        static CmdBorderButtonCaption msCmdBorderButtonCaption;
    };
}


#endif  // __BorderButtonGuiElement_H__

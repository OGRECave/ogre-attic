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
#ifndef __ButtonGuiElement_H__
#define __ButtonGuiElement_H__

#include "OgreGuiElementPrerequisites.h"
#include "OgrePanelGuiElement.h"
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
	class _OgreGuiElementExport ButtonGuiElement : public PanelGuiElement, public GuiPressable
    {
    protected:
		bool mButtonDown;

		String mDownMaterialName;
		String mUpMaterialName;
		String mHiliteDownMaterialName;
		String mHiliteUpMaterialName;
		String mDisabledMaterialName;
		
        ColourValue mCaptionColour;
        ColourValue mCaptionDisabledColour;
		bool mSetCaptionColor;
		bool mSetCaptionDisabledColor;

        static String msTypeName;
		GuiElement* mInsideObject;
		
		void changeChild(GuiElement* e, Real add);

	public :
		const String& getTypeName(void) const;
	    void addBaseParameters(void);

		ButtonGuiElement(const String& name);
        ~ButtonGuiElement() {}

		void processEvent(InputEvent* e);

		void updateMaterials(bool init = false);


        /** Sets the name of the material to use for the button. */
        void setDownMaterialName(const String& name);
        /** Gets the name of the material to use for the button. */
        const String& getDownMaterialName(void) const;

        /** Sets the name of the material to use for the button. */
        void setUpMaterialName(const String& name);
        /** Gets the name of the material to use for the button. */
        const String& getUpMaterialName(void) const;

        /** Sets the name of the material to use for the hilited button. */
        void setHiliteDownMaterialName(const String& name);
        /** Gets the name of the material to use for the hilited button. */
        const String& getHiliteDownMaterialName(void) const;

        /** Sets the name of the material to use for the hilited button. */
        void setHiliteUpMaterialName(const String& name);
        /** Gets the name of the material to use for the hilited button. */
        const String& getHiliteUpMaterialName(void) const;

        /** Sets the name of the material to use for the disabled button. */
        void setDisabledMaterialName(const String& name);
        /** Gets the name of the material to use for the disabled button. */
        const String& getDisabledMaterialName(void) const;

        /** Sets the caption colour to use for the disabled button. */
        void setCaptionColour(const ColourValue& col);
        /** Gets the caption colour to use for the disabled button. */
        ColourValue getCaptionColour(void) const;

        /** Sets the caption colour to use for the disabled button. */
        void setCaptionDisabledColour(const ColourValue& col);
        /** Gets the caption colour to use for the disabled button. */
        ColourValue getCaptionDisabledColour(void) const;

	    void setButtonCaption(const String& templateName, const String& name);
	    String getButtonCaption() const;

        /** Command object for specifying the Material for the button (see ParamCommand).*/
        class CmdButtonDownMaterial : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for specifying the Material for the button (see ParamCommand).*/
        class CmdButtonUpMaterial : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for specifying the Material for the button (see ParamCommand).*/
        class CmdButtonHiliteDownMaterial : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for specifying the Material for the button (see ParamCommand).*/
        class CmdButtonHiliteUpMaterial : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for specifying the Material for the button (see ParamCommand).*/
        class CmdButtonDisabledMaterial : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying the Caption color for the button (see ParamCommand).*/
        class CmdCaptionColour : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying the disabled Caption color for the button (see ParamCommand).*/
        class CmdCaptionDisabledColour : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for specifying the Material for the button (see ParamCommand).*/
        class CmdButtonCaption : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        static CmdButtonDownMaterial msCmdButtonDownMaterial;
        static CmdButtonUpMaterial msCmdButtonUpMaterial;
        static CmdButtonHiliteDownMaterial msCmdButtonHiliteDownMaterial;
        static CmdButtonHiliteUpMaterial msCmdButtonHiliteUpMaterial;
        static CmdButtonDisabledMaterial msCmdButtonDisabledMaterial;
        static CmdCaptionColour msCmdCaptionColour;
        static CmdCaptionDisabledColour msCmdCaptionDisabledColour;
        static CmdButtonCaption msCmdButtonCaption;
    };
}


#endif  // __ButtonGuiElement_H__

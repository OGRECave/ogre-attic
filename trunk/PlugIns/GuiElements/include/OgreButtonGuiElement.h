/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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
#ifndef __ButtonGuiElement_H__
#define __ButtonGuiElement_H__

#include "OgreGuiElementPrerequisites.h"
#include "OgreActionTarget.h"
#include "OgreBorderPanelGuiElement.h"

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
	class _OgreGuiElementExport ButtonGuiElement : public BorderPanelGuiElement, public ActionTarget
    {
    protected:
		String mActionCommand;
		bool mPressed;
		void fireActionPerformed();
		String mBorderDownMaterialName,mBorderUpMaterialName;
		void changeChild(GuiElement* e, Real add);
        static String msTypeName;
		GuiElement* mInsideObject;
	public :
	    void addBaseParameters(void);

		ButtonGuiElement(const String& name);
		void processEvent(InputEvent* e) ;

		void setPressed(bool b, bool init = false);
		String getActionCommand();
		bool isPressed();
		GuiElement* findElementAt(Real x, Real y);


        /** Sets the name of the material to use for the borders. */
        void setBorderDownMaterialName(const String& name);
        /** Gets the name of the material to use for the borders. */
        const String& getBorderDownMaterialName(void);

        /** Sets the name of the material to use for the borders. */
        void setBorderUpMaterialName(const String& name);
        /** Gets the name of the material to use for the borders. */
        const String& getBorderUpMaterialName(void);

	    void setButtonCaption(const String& templateName, const String& name);
	    String getButtonCaption();

        /** Command object for specifying the Material for the border (see ParamCommand).*/
        class CmdBorderDownMaterial : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };

        /** Command object for specifying the Material for the border (see ParamCommand).*/
        class CmdBorderUpMaterial : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying the Material for the border (see ParamCommand).*/
        class CmdButtonCaption : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        static CmdBorderDownMaterial msCmdBorderDownMaterial;
        static CmdBorderUpMaterial msCmdBorderUpMaterial;
        static CmdButtonCaption msCmdButtonCaption;

    };

}


#endif  // __ButtonGuiElement_H__

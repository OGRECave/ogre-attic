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
#ifndef __GuiControl_H__
#define __GuiControl_H__

#include "OgrePrerequisites.h"
#include "OgreActionTarget.h"
#include "OgreString.h"

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
	class _OgreExport GuiControl : public ActionTarget
    {
    protected:
		String mActionCommand;
		bool mPressed;
		bool mMouseWithin;
		bool mButtonDown;
		bool mEnabled;
		
		void fireActionPerformed();

	public :
		GuiControl(const String& name);
        virtual ~GuiControl() { }

		void processEvent(InputEvent* e) ;

		const String& getActionCommand() const;
		void setActionCommand(const String& action);

		bool isPressed() const;
		virtual void setPressed(bool b);

		bool isEnabled() const;
		virtual void setEnabled(bool b);

		bool isMouseWithin() const;
		virtual void setMouseWithin(bool b);
    };
}


#endif  // __GuiControl_H__

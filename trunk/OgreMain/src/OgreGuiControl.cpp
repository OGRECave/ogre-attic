/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#include "OgreGuiControl.h"	
#include "OgreActionEvent.h"
#include "OgreMouseEvent.h"

namespace Ogre {

    //-----------------------------------------------------------------------
	GuiControl::GuiControl(const String& name) :
		ActionTarget()
	{
		mPressed = false;
		mMouseWithin = false;
		mEnabled = true;

		mActionCommand = name;
	}

    //-----------------------------------------------------------------------
	void GuiControl::processEvent(InputEvent* e) 
	{
		if (!mEnabled || e->isConsumed())
			return;

		switch(e->getID()) 
		{
		case ActionEvent::AE_ACTION_PERFORMED:
			processActionEvent(static_cast<ActionEvent*>(e));
			break;

		case MouseEvent::ME_MOUSE_PRESSED:
			setPressed(true);
			break;

		case MouseEvent::ME_MOUSE_RELEASED:
			setPressed(false);
			break;

		case MouseEvent::ME_MOUSE_ENTERED:
			setMouseWithin(true);
			break;

		case MouseEvent::ME_MOUSE_EXITED:
			setMouseWithin(false);
			break;

		case MouseEvent::ME_MOUSE_CLICKED:
			fireActionPerformed();
			break;

		default:
			break;
		}
	}

    //-----------------------------------------------------------------------
	const String& GuiControl::getActionCommand() const
	{ 
		return mActionCommand;
	}

    //-----------------------------------------------------------------------
	void GuiControl::setActionCommand(const String& action)
	{ 
		mActionCommand = action;
	}

    //-----------------------------------------------------------------------
	bool GuiControl::isPressed() const
	{ 
		return mPressed;
	}

    //-----------------------------------------------------------------------
	void GuiControl::setPressed(bool b) 
	{
		mPressed = b;
	}

    //-----------------------------------------------------------------------
	bool GuiControl::isEnabled() const
	{ 
		return mEnabled;
	}

    //-----------------------------------------------------------------------
	void GuiControl::setEnabled(bool b) 
	{
		if (b)
		{
			mEnabled = true;
		}
		else
		{
			mPressed = false;
			mMouseWithin = false;
			mButtonDown = false;
			mEnabled = false;
		}
	}

    //-----------------------------------------------------------------------
	bool GuiControl::isMouseWithin() const
	{ 
		return mMouseWithin;
	}

    //-----------------------------------------------------------------------
	void GuiControl::setMouseWithin(bool b)
	{
		mMouseWithin = b;
	}

    //-----------------------------------------------------------------------
	void GuiControl::fireActionPerformed()
	{
		ActionEvent* ae = new ActionEvent(this, ActionEvent::AE_ACTION_PERFORMED, 0, 0, getActionCommand());
		processEvent(ae);
		delete ae;
	}
}


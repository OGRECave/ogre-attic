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

#include "OgreInput.h"
#include "OgreCursor.h"
#include "OgreEventQueue.h"


namespace Ogre {

    //-----------------------------------------------------------------------
    InputReader::InputReader()
    {
		mCursor = 0;
		mEventQueue = 0;
		mUseBuffered = false;
    }

    //-----------------------------------------------------------------------
    void InputReader::useBufferedInput(EventQueue* pEventQueue) 
    {
		mEventQueue = pEventQueue;

		if (mCursor)
			delete mCursor;

		mCursor = new Cursor();
		mUseBuffered = true;
    }

    //-----------------------------------------------------------------------
    InputReader::~InputReader()
    {
		if (mCursor)
		{
			delete mCursor;
		}
    }

    //-----------------------------------------------------------------------
	void InputReader::triggerMouseButton(int nMouseCode, bool mousePressed)
	{
		if (mousePressed)
		{
			mModifiers |= nMouseCode;
			createMouseEvent(MouseEvent::ME_MOUSE_PRESSED, nMouseCode);

		}
		else
		{	// button up... trigger MouseReleased, and MouseClicked
			mModifiers &= !nMouseCode;
			createMouseEvent(MouseEvent::ME_MOUSE_RELEASED, nMouseCode);
			//createMouseEvent(MouseEvent::ME_MOUSE_CLICKED, nMouseCode);	JCA - moved to EventDispatcher
		}
	}

    //-----------------------------------------------------------------------
	void InputReader::createMouseEvent(int id, int button)
	{
		MouseEvent* me = new MouseEvent(NULL, id, button, 0, // hack fix time
			mModifiers, mCursor->getX(), mCursor->getY(), mCursor->getZ(),0);	// hack fix click count
		mCursor->processEvent(me);
		mEventQueue->push(me);

	}
 
    //-----------------------------------------------------------------------
	void InputReader::mouseMoved()
	{

		if (mModifiers & InputEvent::BUTTON_ANY_MASK)	// don't need to know which button. you can get that from the modifiers
		{
			createMouseEvent(MouseEvent::ME_MOUSE_DRAGGED, 0);
		}
		else
		{
			createMouseEvent(MouseEvent::ME_MOUSE_MOVED, 0);
		}

	}
    //-----------------------------------------------------------------------
	void InputReader::addCursorMoveListener(MouseMotionListener* c)
	{
		mCursor->addMouseMotionListener(c);
	}
    //-----------------------------------------------------------------------
	void InputReader::removeCursorMoveListener(MouseMotionListener* c)
	{
		mCursor->removeMouseMotionListener(c);
	}
    //-----------------------------------------------------------------------

}

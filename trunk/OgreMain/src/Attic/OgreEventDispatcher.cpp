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
#include "OgreStableHeaders.h"

#include "OgreMouseEvent.h"
#include "OgreEventDispatcher.h"
#include "OgreGuiContainer.h"
#include "OgreTargetManager.h"
#include "OgreEventProcessor.h"
#include "OgrePositionTarget.h"
#include "OgreKeyEvent.h"

namespace Ogre {

    EventDispatcher::EventDispatcher(TargetManager* pTargetManager, EventProcessor* pEventProcessor) :
	mTargetManager (pTargetManager),	// abstract this out TODO
	mEventProcessor (pEventProcessor)
    {
		mFocus = 0;
		mMousePositionTarget = 0;
		mKeyCursorOn = 0;
		mEventMask = 0;
		mTargetLastEntered = 0;
		mDragging = false;


    }
    //---------------------------------------------------------------------
    EventDispatcher::~EventDispatcher()
    {

    }

    //---------------------------------------------------------------------
	bool EventDispatcher::dispatchEvent(InputEvent* e) 
	{
		bool ret = false;
		if (e->isEventBetween(MouseEvent::ME_FIRST_EVENT, MouseEvent::ME_LAST_EVENT))	// i am open to suggestions for a better way to do this
																						// maybe a method e->isEvent(InputEvent::MouseEvent) ??
		{
			MouseEvent* me = static_cast<MouseEvent*> (e);
			ret = processMouseEvent(me);
		}
		else if (e->isEventBetween(KeyEvent::KE_FIRST_EVENT, KeyEvent::KE_LAST_EVENT))
		{
			KeyEvent* ke = static_cast<KeyEvent*> (e);
			ret = processKeyEvent(ke);

		}
		// todo do focus and key event handling
		return ret;
	}


	bool EventDispatcher::processKeyEvent(KeyEvent* e) 
	{
		if (mKeyCursorOn)
		{
			mKeyCursorOn->processEvent(e);
		}
		return e->isConsumed();

	}
	
    //---------------------------------------------------------------------

	bool EventDispatcher::processMouseEvent(MouseEvent* e) 
	{
		int id = e->getID();
		PositionTarget* targetOver;

		switch (id) 
		{		
		case MouseEvent::ME_MOUSE_PRESSED:
			mDragging = true;
			if (mMousePositionTarget && mMousePositionTarget->isKeyEnabled())
			{
				mKeyCursorOn = mMousePositionTarget;
			}
			else
			{
				mKeyCursorOn = NULL;
			}
			break;

		case MouseEvent::ME_MOUSE_RELEASED:
			mDragging = false;
			targetOver = mTargetManager->getPositionTargetAt(e->getX(), e->getY());

			if (targetOver && targetOver != mMousePositionTarget)
			{
				
                if (mMousePositionTarget)
					mMousePositionTarget->processEvent(e); 
				retargetMouseEvent(targetOver, MouseEvent::ME_MOUSE_ENTERED, e);
				setMouseTarget(targetOver, e);

				e->consume();
				return e->isConsumed();
			}
			else if (targetOver && targetOver == mMousePositionTarget)
			{
				mMousePositionTarget->processEvent(e);
				id = MouseEvent::ME_MOUSE_CLICKED;
			}
			break;

		case MouseEvent::ME_MOUSE_ENTERED:
			// If there is a click and drag, only give enter and exit events to the item originally clicked in
			//
			// This makes it tough to implement drag and drop, but works for everything else. Drag and
			//	drop requires more extensive changes like testing if the original item can be dropped on the
			//	item the mouse is currently over. So this seems like a reasonable limitation for now.
			if (mDragging && mTargetManager->getPositionTargetAt(e->getX(), e->getY()) != mMousePositionTarget)
			{
				e->consume();

				return e->isConsumed();
			}
			break;

		case MouseEvent::ME_MOUSE_EXITED:
			if (!mDragging) 
			{
				setMouseTarget(NULL, e);
			}
			else if (mTargetManager->getPositionTargetAt(e->getX(), e->getY()) != mMousePositionTarget)
			{
				e->consume();

				return e->isConsumed();
			}
			break;

		case MouseEvent::ME_MOUSE_MOVED:
		case MouseEvent::ME_MOUSE_DRAGGED:
			targetOver = mTargetManager->getPositionTargetAt(e->getX(), e->getY());
			trackMouseEnterExit(targetOver, e);

			if (!mDragging) 
			{
				setMouseTarget(targetOver, e);
			}
			break;

			break;
		
		case MouseEvent::ME_MOUSE_CLICKED:
			break;

		}
		retargetMouseEvent(mMousePositionTarget, id, e);

		return e->isConsumed();
	}
    //---------------------------------------------------------------------

	void EventDispatcher::retargetMouseEvent(PositionTarget* target, int id, MouseEvent* e) 
	{

		if (target == NULL)
		{
			return;

		}
		/*Real x = e->getX(), y = e->getY();
		PositionTarget* positionTarget;

		for(positionTarget = target;
			positionTarget != NULL ;
			positionTarget = positionTarget->getPositionTargetParent()) 
		{
			x -= positionTarget->getLeft();
			y -= positionTarget->getTop();
		}*/
		MouseEvent* retargeted = new MouseEvent(target,
											   id, 
											   e->getButtonID(),
											   e->getWhen(), 
											   e->getModifiers(),
											   e->getX(), 
											   e->getY(), 
											   e->getZ(),
											   e->getClickCount());

		target->processEvent(retargeted);
		
		e->consume();
		
		delete retargeted;
	}
    //---------------------------------------------------------------------
	void EventDispatcher::setMouseTarget(PositionTarget* target, MouseEvent* e) 
	{
		if (target != mMousePositionTarget) 
		{
			mMousePositionTarget = target;
		}
	}

    //---------------------------------------------------------------------
	void EventDispatcher::trackMouseEnterExit(PositionTarget* targetOver, MouseEvent* e) 
	{
		int	id = e->getID();

		if (mTargetLastEntered == targetOver) 
		{
			return;
		}

		// If there is a click and drag, only give enter and exit events to the item originally clicked in
		//
		// This makes it tough to implement drag and drop, but works for everything else. Drag and
		//	drop requires more extensive changes like testing if the original item can be dropped on the
		//	item the mouse is currently over. So this seems like a reasonable limitation for now.

		if (mTargetLastEntered && (!mDragging || mTargetLastEntered == mMousePositionTarget))
		{
			retargetMouseEvent(mTargetLastEntered, MouseEvent::ME_MOUSE_EXITED, e);
			if (id == MouseEvent::ME_MOUSE_EXITED) 
			{
				// consume native exit event if we generate one
				e->consume();
			}

			mTargetLastEntered = NULL;
		}

		if (targetOver && (!mDragging || targetOver == mMousePositionTarget))
		{
			retargetMouseEvent(targetOver, MouseEvent::ME_MOUSE_ENTERED, e);
			if (id == MouseEvent::ME_MOUSE_ENTERED) 
			{
				// consume native enter event if we generate one
				e->consume();
			}

			mTargetLastEntered = targetOver;
		}
	}
}




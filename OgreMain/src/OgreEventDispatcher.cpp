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

#include "OgreMouseEvent.h"
#include "OgreEventDispatcher.h"
#include "OgreGuiContainer.h"
#include "OgreTargetManager.h"
#include "OgreEventProcessor.h"
#include "OgrePositionTarget.h"

namespace Ogre {

    EventDispatcher::EventDispatcher(TargetManager* pTargetManager, EventProcessor* pEventProcessor) :
	mTargetManager (pTargetManager),	// abstract this out TODO
	mEventProcessor (pEventProcessor)
    {
		mFocus = 0;
		mMousePositionTarget = 0;
		mCursorOn = 0;
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
		// todo do focus and key event handling
		return ret;
	}

    //---------------------------------------------------------------------

	bool EventDispatcher::processMouseEvent(MouseEvent* e) 
	{
		int id = e->getID();
		PositionTarget* targetOver;	// if it = NULL, then use the default mouse listener

		targetOver = mTargetManager->getPositionTargetAt(e->getX(), e->getY());
		trackMouseEnterExit(targetOver, e);
		setMouseTarget(targetOver,e);

		if (mMousePositionTarget != NULL) 
		{
			// we are currently forwarding to some PositionTarget, check
			// to see if we should continue to forward.
			switch(id) 
			{
			case MouseEvent::ME_MOUSE_DRAGGED:
				if(mDragging) 
				{
					retargetMouseEvent(mMousePositionTarget, id, e);
				}
				break;
			
			case MouseEvent::ME_MOUSE_PRESSED:
				mDragging = true;
				retargetMouseEvent(mMousePositionTarget, id, e);
				break;

			case MouseEvent::ME_MOUSE_RELEASED:
			{
				PositionTarget* releasedTarget = mMousePositionTarget;
				mDragging = false;
				retargetMouseEvent(mMousePositionTarget, id, e);
				setMouseTarget(targetOver, e);

				break;
			}
			
			case MouseEvent::ME_MOUSE_CLICKED:
				// click event should not be redirected since PositionTarget has moved or hidden
				retargetMouseEvent(mMousePositionTarget, id, e);
				break;

			case MouseEvent::ME_MOUSE_ENTERED:
				break;

			case MouseEvent::ME_MOUSE_EXITED:
				if (!mDragging) 
				{
					setMouseTarget(NULL, e);
				}
				break;

			case MouseEvent::ME_MOUSE_MOVED:
				retargetMouseEvent(mMousePositionTarget, id, e);
				break;

			}
			e->consume();
		}

	return e->isConsumed();
	}
    //---------------------------------------------------------------------

	void EventDispatcher::retargetMouseEvent(PositionTarget* target, int id, MouseEvent* e) 
	{

		if (target == NULL)
		{
			return;

		}
		int x = e->getX(), y = e->getY();
		PositionTarget* positionTarget;

		for(positionTarget = target;
			positionTarget != NULL ;
			positionTarget = positionTarget->getPositionTargetParent()) 
		{
			x -= positionTarget->getLeft();
			y -= positionTarget->getTop();
		}
		MouseEvent* retargeted = new MouseEvent(target,
											   id, 
											   e->getButtonID(),
											   e->getWhen(), 
											   e->getModifiers(),
											   x, 
											   y, 
											   e->getZ(),
											   e->getClickCount());

		target->processEvent(retargeted);
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
		PositionTarget*	targetEnter = NULL;
		int	id = e->getID();

		targetEnter = targetOver;

		if (mTargetLastEntered == targetEnter) 
		{
			return;
		}

		retargetMouseEvent(mTargetLastEntered, MouseEvent::ME_MOUSE_EXITED, e);
		if (id == MouseEvent::ME_MOUSE_EXITED) 
		{
			// consume native exit event if we generate one
			e->consume();
		}

		retargetMouseEvent(targetEnter, MouseEvent::ME_MOUSE_ENTERED, e);
		if (id == MouseEvent::ME_MOUSE_ENTERED) 
		{
			// consume native enter event if we generate one
			e->consume();
		}

		mTargetLastEntered = targetEnter;
	}



}




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

#include "OgreEventProcessor.h"
#include "OgreOverlayManager.h"
#include "OgreEventQueue.h"
#include "OgreRoot.h"
#include "OgreMouseEvent.h"
#include "OgreKeyEvent.h"
#include "OgreActionEvent.h"
#include "OgreInput.h"
#include "OgreCursor.h"


namespace Ogre {
    //-----------------------------------------------------------------------
    template<> EventProcessor* Singleton<EventProcessor>::ms_Singleton = 0;
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    EventProcessor& EventProcessor::getSingleton(void)
    {
        return Singleton<EventProcessor>::getSingleton();
    }

//-----------------------------------------------------------------------------
    EventProcessor::EventProcessor() :
		MouseTarget(),
		MouseMotionTarget()
    {
		mEventQueue = 0;
		mInputDevice = 0;
    }

//-----------------------------------------------------------------------------
    EventProcessor::~EventProcessor()
    {

		cleanup();
    }

//-----------------------------------------------------------------------------
	void EventProcessor::cleanup()
	{
		if (mEventQueue)
			delete mEventQueue;

        for(DispatcherList::iterator i = mDispatcherList.begin(); i != mDispatcherList.end(); ++i )                 
        {
			delete *i;
        }
		mDispatcherList.clear();

	}

//-----------------------------------------------------------------------------
	void EventProcessor::stopProcessingEvents()
	{

		mEventQueue->activateEventQueue(false);
		Root::getSingleton().removeFrameListener(this);

	}

//-----------------------------------------------------------------------------
	void EventProcessor::initialise(RenderWindow* ren)
	{
		cleanup();


		mEventQueue = new EventQueue();

		mInputDevice = PlatformManager::getSingleton().createInputReader();
		mInputDevice->useBufferedInput(mEventQueue);
		mInputDevice->initialise(ren,true, true, false);	

	}
//-----------------------------------------------------------------------------

	void EventProcessor::addTargetManager(TargetManager* targetManager)
	{
		EventDispatcher* pDispatcher = new EventDispatcher(targetManager, this);	
		mDispatcherList.push_back(pDispatcher);
	}


//-----------------------------------------------------------------------------
	void EventProcessor::startProcessingEvents()
	{
		Root::getSingleton().addFrameListener(this);
		mEventQueue->activateEventQueue(true);
	}


//-----------------------------------------------------------------------------
	bool EventProcessor::frameStarted(const FrameEvent& evt)
	{
		mInputDevice->capture();
		while (mEventQueue->getSize() > 0)
		{
			InputEvent* e = mEventQueue->pop();
			bool bIsConsumed = false;
			for(DispatcherList::iterator i = mDispatcherList.begin(); i != mDispatcherList.end(); ++i )                 
			{
				bIsConsumed |= (*i)->dispatchEvent(e);
			}
			if (!bIsConsumed)
			{
				processEvent(e);		// nothing used it, so use default processing 

			}

			delete e;		// created from dispatch;

		}
		return true;
	}

//-----------------------------------------------------------------------------
	void EventProcessor::processEvent(InputEvent* e)
	{
		switch(e->getID()) 
		{
		case MouseEvent::ME_MOUSE_PRESSED:
		case MouseEvent::ME_MOUSE_RELEASED:
		case MouseEvent::ME_MOUSE_CLICKED:
		case MouseEvent::ME_MOUSE_ENTERED:
		case MouseEvent::ME_MOUSE_EXITED:
			processMouseEvent(static_cast<MouseEvent*>(e));
			break;
		case MouseEvent::ME_MOUSE_MOVED:
		case MouseEvent::ME_MOUSE_DRAGGED:
			processMouseMotionEvent(static_cast<MouseEvent*>(e));
			break;
		case KeyEvent::KE_KEY_PRESSED:
		case KeyEvent::KE_KEY_RELEASED:
		case KeyEvent::KE_KEY_CLICKED:
			processKeyEvent(static_cast<KeyEvent*>(e));
			break;
		}
	}

//-----------------------------------------------------------------------------
	void EventProcessor::addCursorMoveListener(MouseMotionListener* c)
	{
		mInputDevice->addCursorMoveListener(c);
	}
//-----------------------------------------------------------------------------
	void EventProcessor::removeCursorMoveListener(MouseMotionListener* c)
	{
		mInputDevice->removeCursorMoveListener(c);
	}

//-----------------------------------------------------------------------------
	Real EventProcessor::getLeft() const
	{
		return 0;
	}

//-----------------------------------------------------------------------------
	Real EventProcessor::getTop() const
	{
		return 0;
	}
	
//-----------------------------------------------------------------------------
	PositionTarget* EventProcessor::getPositionTargetParent() 
	{
		return NULL;
	}
//-----------------------------------------------------------------------------

}


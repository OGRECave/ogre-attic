/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General  License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General  License for more details.

You should have received a copy of the GNU Lesser General  License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

/***************************************************************************
OgreEventMulticaster.h  -  
	  This class implements efficient and thread-safe multi-cast event 
	  dispatching.  
	  It will manage an immutable structure consisting of a binary chain of 
	  event listeners and will dispatch events to those listeners.  Because
	  the structure is immutable, it is safe to use this API to add/remove
	  listeners during the process of an event dispatch operation.
	 
	  An example of how this class could be used to implement a new
	  component which fires "action" events:
	 
	   // member variable
      ActionListener* mActionListener;
	 
	  void addActionListener(ActionListener* l) 
	  {
		 mActionListener = EventMulticaster::add(mActionListener, l);
	  }
	  void removeActionListener(ActionListener* l) 
	  {
	   	 mActionListener = EventMulticaster::remove(mActionListener, l);
	  }
	  void processEvent(InputEvent* e) 
	  {
	      // when event occurs which is an action
	      if (mActionListener != NULL) 
		  {
	          mActionListener->actionPerformed(static_cast<ActionEvent*>(e));
	      }         
	  }

-------------------
begin                : Nov 19 2002
copyright            : (C) 2002 by Kenny Sabir
email                : kenny@sparksuit.com
***************************************************************************/

#ifndef __EVENT_MULTICASTER_H__
#define __EVENT_MULTICASTER_H__

#include "OgrePrerequisites.h"
#include "OgreEventListeners.h"


namespace Ogre {

	/** This class implements efficient and thread-safe multi-cast event 
	  dispatching.
	@remarks
	  It manages an immutable structure consisting of a binary chain of 
	  event listeners and will dispatch events to those listeners.  Because
	  the structure is immutable, it is safe to use this API to add/remove
	  listeners during the process of an event dispatch operation.
	*/
    class _OgreExport EventMulticaster :  public MouseListener, public ActionListener
    {
    public:



		/**
		 * Creates an event multicaster instance which chains listener-a
		 * with listener-b. Input parameters <code>a</code> and <code>b</code> 
		 * should not be <code>null</code>, though implementations may vary in 
		 * choosing whether or not to throw <code>NullPointerException</code> 
		 * in that case.
		 * @param a listener-a
		 * @param b listener-b
		 */ 
		EventMulticaster(EventListener* a, EventListener* b);

		/**
		 * Handles the actionPerformed event by invoking the
		 * actionPerformed methods on listener-a and listener-b.
		 * @param e the action event
		 */
		 void listSelected(ListSelectionEvent* e);


		/**
		 * Handles the actionPerformed event by invoking the
		 * actionPerformed methods on listener-a and listener-b.
		 * @param e the action event
		 */
		 void actionPerformed(ActionEvent* e);

		/**
		 * Handles the scrollPerformed event by invoking the
		 * scrollPerformed methods on listener-a and listener-b.
		 * @param e the action event
		 */
		 void scrollPerformed(ScrollEvent* e);

		/**
		 * Adds action-listener-a with action-listener-b and
		 * returns the resulting multicast listener.
		 * @param a action-listener-a
		 * @param b action-listener-b
		 */
		 static KeyListener* add(KeyListener* a, KeyListener* b);
		 static MouseListener* add(MouseListener* a, MouseListener* b);
		 static ActionListener* add(ActionListener* a, ActionListener* b);
		 static MouseMotionListener* add(MouseMotionListener* a, MouseMotionListener* b) ;
		 static ListSelectionListener* add(ListSelectionListener* a, ListSelectionListener* b) ;
		 static ScrollListener* add(ScrollListener* a, ScrollListener* b) ;


		 /**
		 * Adds mouse-listener-a with mouse-listener-b and
		 * returns the resulting multicast listener.
		 * @param a mouse-listener-a
		 * @param b mouse-listener-b
		 */

//		 static TextListener* add(TextListener* a, TextListener* b);



		/**
		 * Handles the keyPressed event by invoking the
		 * keyPressed methods on listener-a and listener-b.
		 * @param e the key event
		 */
		 void keyPressed(KeyEvent* e);

		 /**
		 * Handles the keyReleased event by invoking the
		 * keyReleased methods on listener-a and listener-b.
		 * @param e the key event
		 */
		 void keyReleased(KeyEvent* e);

		 /**
		 * Handles the keyTyped event by invoking the
		 * keyTyped methods on listener-a and listener-b.
		 * @param e the key event
		 */
		 void keyClicked(KeyEvent* e);

		 /**
		 * Handles the mouseClicked event by invoking the
		 * mouseClicked methods on listener-a and listener-b.
		 * @param e the mouse event
		 */
		 void mouseClicked(MouseEvent* e);

		 /**
		 * Handles the mouseEntered event by invoking the
		 * mouseEntered methods on listener-a and listener-b.
		 * @param e the mouse event
		 */
		 void mouseEntered(MouseEvent* e);

		 /**
		 * Handles the mouseExited event by invoking the
		 * mouseExited methods on listener-a and listener-b.
		 * @param e the mouse event
		 */
		 void mouseExited(MouseEvent* e);

		 /**
		 * Handles the mouseMoved event by invoking the
		 * mouseMoved methods on listener-a and listener-b.
		 * @param e the mouse event
		 */
		 void mouseMoved(MouseEvent* e);

		 /**
		 * Handles the mousePressed event by invoking the
		 * mousePressed methods on listener-a and listener-b.
		 * @param e the mouse event
		 */
		 void mousePressed(MouseEvent* e);

		 /**
		 * Handles the mouseReleased event by invoking the
		 * mouseReleased methods on listener-a and listener-b.
		 * @param e the mouse event
		 */
		 void mouseReleased(MouseEvent* e);

		 /**
		 * Removes the old action-listener from action-listener-l and
		 * returns the resulting multicast listener.
		 * @param l action-listener-l
		 * @param oldl the action-listener being removed
		 */
		 static ActionListener* remove(ActionListener* l, ActionListener* oldl);

		 /**
		 * Removes the old scroll-listener from scroll-listener-l and
		 * returns the resulting multicast listener.
		 * @param l scroll-listener-l
		 * @param oldl the scroll-listener being removed
		 */
		 static ScrollListener* remove(ScrollListener* l, ScrollListener* oldl);

		 /**
		 * Removes the old listSlection-listener from action-listener-l and
		 * returns the resulting multicast listener.
		 * @param l action-listener-l
		 * @param oldl the action-listener being removed
		 */
		 static ListSelectionListener* remove(ListSelectionListener* l, ListSelectionListener* oldl);

		 /**
		 * Removes the old key-listener from key-listener-l and
		 * returns the resulting multicast listener.
		 * @param l key-listener-l
		 * @param oldl the key-listener being removed
		 */
		 static KeyListener* remove(KeyListener* l, KeyListener* oldl);

		 /**
		 * Removes the old mouse-listener from mouse-listener-l and
		 * returns the resulting multicast listener.
		 * @param l mouse-listener-l
		 * @param oldl the mouse-listener being removed
		 */
		 static MouseMotionListener* remove(MouseMotionListener* l, MouseMotionListener* oldl);

		 /**
		 * Removes the old mouse-listener from mouse-listener-l and
		 * returns the resulting multicast listener.
		 * @param l mouse-listener-l
		 * @param oldl the mouse-listener being removed
		 */
		 static MouseListener* remove(MouseListener* l, MouseListener* oldl);

//		 static TextListener* remove(TextListener* l, TextListener* oldl);

		/**
		 * Removes a listener from this multicaster and returns the
		 * resulting multicast listener.
		 * @param oldl the listener to be removed
		 */
		 EventListener* remove(EventListener* oldl, bool& deleteSelf) ;

		virtual bool isMulticaster() const;


    protected:
		EventListener* mA;
		EventListener* mB;

		/** 
		 * Returns the resulting multicast listener after removing the
		 * old listener from listener-l.
		 * If listener-l equals the old listener OR listener-l is null, 
		 * returns null.
		 * Else if listener-l is an instance of EventMulticaster, 
		 * then it removes the old listener from it.
		 * Else, returns listener l.
		 * @param l the listener being removed from
		 * @param oldl the listener being removed
		 */
		 static EventListener* removeInternal(EventListener* l, EventListener* oldl);

		/** 
		 * Returns the resulting multicast listener from adding listener-a
		 * and listener-b together.  
		 * If listener-a is null, it returns listener-b;  
		 * If listener-b is null, it returns listener-a
		 * If neither are null, then it creates and returns
		 * a new EventMulticaster instance which chains a with b.
		 * @param a event listener-a
		 * @param b event listener-b
		 */

		 static EventListener* addInternal(EventListener* a, EventListener* b) ;

		/**
		 * Converts a EventMulticaster to the base parent class EventListener.
		 * Since multicaster has many parent listeners (Action,Mouse etc), it has multiple 
		 * EventListeners, so choose 1 (Action arbitrary) to reach the EventListener
		 */
		 static EventListener* convertMultiToListener(EventMulticaster* m);

		/**
		 * Converts a EventListener to the class EventMulticaster .
		 * Since multicaster has many parent listeners (Action,Mouse etc), it has multiple 
		 * EventListeners, so choose 1 (Action arbitrary) to reach the EventListener.
		 * This method throws an exception if the listener isn't a multicaster.
		 */
		 static EventMulticaster* convertListenerToMulti(EventListener* l);

    };



}


#endif //__EVENT_MULTICASTER_H__


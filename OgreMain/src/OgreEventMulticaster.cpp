/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General License for more details.

You should have received a copy of the GNU Lesser General License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#include "OgreException.h"
#include "OgreEventMulticaster.h"


namespace Ogre {

	EventMulticaster::EventMulticaster(EventListener* a, EventListener* b) 
	 {
		mA = a; 
		mB = b;
	}
	MouseListener* EventMulticaster::add(MouseListener* a, MouseListener* b) 
	{
		return static_cast<MouseListener*>(addInternal(a, b));
	}
	KeyListener* EventMulticaster::add(KeyListener* a, KeyListener* b) 
	{
		return static_cast<KeyListener*>(addInternal(a, b));
	}
	MouseMotionListener* EventMulticaster::add(MouseMotionListener* a, MouseMotionListener* b) 
	{
		return static_cast<MouseMotionListener*>(addInternal(a, b));
	}

	ActionListener* EventMulticaster::add(ActionListener* a, ActionListener* b) 
	{
		return static_cast<ActionListener*>(addInternal(a, b));
	}

	ScrollListener* EventMulticaster::add(ScrollListener* a, ScrollListener* b) 
	{
		return static_cast<ScrollListener*>(addInternal(a, b));
	}

	ListSelectionListener* EventMulticaster::add(ListSelectionListener* a, ListSelectionListener* b) 
	{
		return static_cast<ListSelectionListener*>(addInternal(a, b));
	}

	EventListener* EventMulticaster::addInternal(EventListener* a, EventListener* b) 
	{
		if (a == NULL)  return b;
		if (b == NULL)  return a;
		return convertMultiToListener(new EventMulticaster(a, b));
	}

	void EventMulticaster::listSelected(ListSelectionEvent* e) 
	{
		(static_cast<ListSelectionListener*>(mA))->listSelected(e);
		(static_cast<ListSelectionListener*>(mB))->listSelected(e);
	}

	void EventMulticaster::actionPerformed(ActionEvent* e) 
	{
		(static_cast<ActionListener*>(mA))->actionPerformed(e);
		(static_cast<ActionListener*>(mB))->actionPerformed(e);
	}

	void EventMulticaster::scrollPerformed(ScrollEvent* e) 
	{
		(static_cast<ScrollListener*>(mA))->scrollPerformed(e);
		(static_cast<ScrollListener*>(mB))->scrollPerformed(e);
	}

	void EventMulticaster::mouseClicked(MouseEvent* e) 
	{
		(static_cast<MouseListener*>(mA))->mouseClicked(e);
		(static_cast<MouseListener*>(mB))->mouseClicked(e);
	}

	void EventMulticaster::keyClicked(KeyEvent* e) 
	{
		(static_cast<KeyListener*>(mA))->keyClicked(e);
		(static_cast<KeyListener*>(mB))->keyClicked(e);
	}

	void EventMulticaster::mouseEntered(MouseEvent* e) 
	{
		(static_cast<MouseListener*>(mA))->mouseEntered(e);
		(static_cast<MouseListener*>(mB))->mouseEntered(e);
	}

	void EventMulticaster::mouseExited(MouseEvent* e) 
	{
		(static_cast<MouseListener*>(mA))->mouseExited(e);
		(static_cast<MouseListener*>(mB))->mouseExited(e);
	}

	void EventMulticaster::mousePressed(MouseEvent* e) 
	{
		(static_cast<MouseListener*>(mA))->mousePressed(e);
		(static_cast<MouseListener*>(mB))->mousePressed(e);
	}

	void EventMulticaster::mouseReleased(MouseEvent* e) 
	{
		(static_cast<MouseListener*>(mA))->mouseReleased(e);
		(static_cast<MouseListener*>(mB))->mouseReleased(e);
	}
	void EventMulticaster::keyPressed(KeyEvent* e) 
	{
		(static_cast<KeyListener*>(mA))->keyPressed(e);
		(static_cast<KeyListener*>(mB))->keyPressed(e);
	}

	void EventMulticaster::keyReleased(KeyEvent* e) 
	{
		(static_cast<KeyListener*>(mA))->keyReleased(e);
		(static_cast<KeyListener*>(mB))->keyReleased(e);
	}

	ListSelectionListener* EventMulticaster::remove(ListSelectionListener* l, ListSelectionListener* oldl) 
	{
		return static_cast<ListSelectionListener*>( removeInternal(l, oldl));
	}


	ActionListener* EventMulticaster::remove(ActionListener* l, ActionListener* oldl) 
	{
	return static_cast<ActionListener*>( removeInternal(l, oldl));
	}

	ScrollListener* EventMulticaster::remove(ScrollListener* l, ScrollListener* oldl) 
	{
	return static_cast<ScrollListener*>( removeInternal(l, oldl));
	}

	MouseListener* EventMulticaster::remove(MouseListener* l, MouseListener* oldl) 
	{
	return static_cast<MouseListener*> (removeInternal(l, oldl));
	}
	KeyListener* EventMulticaster::remove(KeyListener* l, KeyListener* oldl) 
	{
	return static_cast<KeyListener*> (removeInternal(l, oldl));
	}

	MouseMotionListener* EventMulticaster::remove(MouseMotionListener* l, MouseMotionListener* oldl) 
	{
	return static_cast<MouseMotionListener*> (removeInternal(l, oldl));
	}

	EventListener* EventMulticaster::remove(EventListener* oldl, bool& deleteSelf) 
	 {
		 if (oldl == mA) 
		 {
			 delete mA;
			 deleteSelf = true;	// the multicaster is now not needed, so delete this at a higher level
			 return mB;
		 }
		 if (oldl == mB)  
		 {
			 delete mB;
			 deleteSelf = true; // the multicaster is now not needed, so delete this at a higher level
			 return mA;
		 }
		EventListener* a2 = removeInternal(mA, oldl);
		EventListener* b2 = removeInternal(mB, oldl);
		if (a2 == mA && b2 == mB) 
		{
			return convertMultiToListener(this);	// it's not here
		}
		return addInternal(a2, b2); // the oldl has been removed so join the rest and return it.
	}

	EventListener* EventMulticaster::removeInternal(EventListener* l, EventListener* oldl) 
	 {
		if (l == oldl || l == NULL) 
		{
			return NULL;
		}
		else if (l->isMulticaster()) 
		{
			bool deleteSelf = false;
			EventListener* newl = convertListenerToMulti(l)->remove(oldl, deleteSelf);

			if (deleteSelf)
			{
				delete l;
			}


			return newl;
		}
		else 
		{
			return l;		// it's not here
		}
	}

	bool EventMulticaster::isMulticaster() const
	{
		return true; 
	}


	EventListener* EventMulticaster::convertMultiToListener(EventMulticaster* m)
	{
			// messy, but C++ uses 2 copies of Base class for multiple inheritance with a common parent,
			// so ActionListener is used to resolve ambiguity.
			// Since Multicaster has many listeners parents, it chooses just one (actionListener arbitary) 
			// to get to the EventListener parent.
			//http://citeseer.nj.nec.com/cache/papers/cs/15957/
			//http:zSzzSzwww.cs.colorado.eduzSz~diwanzSz5535-00zSzmi.pdf/stroustrup99multiple.pdf
		return static_cast <ActionListener*> (m);
	}

	EventMulticaster* EventMulticaster::convertListenerToMulti(EventListener* l)
	{
			// messy, but C++ uses 2 copies of Base class for multiple inheritance with a common parent,
			// so ActionListener is used to resolve ambiguity.
			// Since Multicaster has many listeners parents, it chooses just one (actionListener arbitary) 
			// to get to the EventListener parent.
			//http://citeseer.nj.nec.com/cache/papers/cs/15957/
			//http:zSzzSzwww.cs.colorado.eduzSz~diwanzSz5535-00zSzmi.pdf/stroustrup99multiple.pdf
		if (!l->isMulticaster())
		{
            Except(Exception::ERR_INVALIDPARAMS, "Illegal convertion, listener is not a multicaster"
               , "EventMulticaster::convertListenerToMulti");

		}
		return (static_cast <EventMulticaster*>(static_cast <ActionListener*> (l)));
	}
}


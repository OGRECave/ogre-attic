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
OgreListSelectionEvent.h  -  
	 This event is created whenever a list item is selected.
-------------------
begin                : Dec 03 2002
copyright            : (C) 2002 by Kenny Sabir
email                : kenny@sparksuit.com
***************************************************************************/

#ifndef __ListSelectionEvent_H__
#define __ListSelectionEvent_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreInputEvent.h"

namespace Ogre {

    class _OgreExport ListSelectionEvent : public InputEvent
    {
    protected:
	
	public:
	enum
	{
	/**
	 * The first number in the range of ids used for ListSelection events.
	 */
		LSE_LIST_SELECTION_FIRST		= 1501,

	/**
	 * The last number in the range of ids used for ListSelection events.
	 */
		LSE_LIST_SELECTION_LAST		        = 1501
	};

	/**
	 * This event id indicates that a meaningful ListSelection occured.
	 */
	enum
	{
		LSE_LIST_SELECTED	= LSE_LIST_SELECTION_FIRST 
	};


		/**
		 * Constructs a ListSelectionEvent object with the specified source GuiElement,
		 * type, modifiers, coordinates, and click count.
		 *
		 * @param source       the GuiElement that originated the event
		 * @param id           the integer that identifies the event
		 * @param when         a long int that gives the time the event occurred
		 * @param modifiers    the modifier keys down during event
		 *                     (shift, ctrl, alt, meta)
		 * @param listSelectionCommand   - Gives specific details of the event
		 */
		 ListSelectionEvent(GuiElement* source, int id, Real when, int modifiers);

		/**
		 * Returns a parameter string identifying this ListSelection event.
		 * This method is useful for event-logging and for debugging.
		 * 
		 * @return a string identifying the event and its associated command 
		 */
		String paramString() const;

    };


}


#endif 



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

#include "OgreListSelectionTarget.h"
#include "OgreEventListeners.h"
#include "OgreListSelectionEvent.h"
#include "OgreEventMulticaster.h"	



namespace Ogre {

	ListSelectionTarget::ListSelectionTarget() 
	{
		mListSelectionListener = 0;
	}

    //-----------------------------------------------------------------------

	void ListSelectionTarget::processListSelectionEvent(ListSelectionEvent* e) 
	{
		ListSelectionListener* listener = mListSelectionListener;
		if (listener != NULL) 
		{
			listener->listSelected(e);
		}
	}

	void ListSelectionTarget::addListSelectionListener(ListSelectionListener* l) 
	{
		if (l == NULL) 
		{
			return;
		}
		mListSelectionListener = EventMulticaster::add(mListSelectionListener,l);
	}

	void ListSelectionTarget::removeListSelectionListener(ListSelectionListener* l) 
	{
		if (l == NULL) 
		{
			return;
		}
		mListSelectionListener = EventMulticaster::remove(mListSelectionListener,l);
	}
}


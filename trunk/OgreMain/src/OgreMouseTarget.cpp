
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

#include "OgreMouseEvent.h"
#include "OgreEventListeners.h"
#include "OgreMouseTarget.h"	// HACK move higher
#include "OgreEventMulticaster.h"	



namespace Ogre {


	MouseTarget::MouseTarget() 
	{
		mMouseListener = 0;
		mMouseWithin = false;

	}
    //-----------------------------------------------------------------------

	void MouseTarget::processMouseEvent(MouseEvent* e) 
	{
		MouseListener* listener = mMouseListener;
		if (listener != NULL) 
		{
			int id = e->getID();
			switch(id) 
			{
			case MouseEvent::ME_MOUSE_PRESSED:
				listener->mousePressed(e);
				break;
			case MouseEvent::ME_MOUSE_RELEASED:
				listener->mouseReleased(e);
				break;
			case MouseEvent::ME_MOUSE_CLICKED:
				listener->mouseClicked(e);
				break;
			case MouseEvent::ME_MOUSE_EXITED:
				mMouseWithin = false;
				listener->mouseExited(e);
				break;
			case MouseEvent::ME_MOUSE_ENTERED:
				mMouseWithin = true;
				listener->mouseEntered(e);
				break;
			case MouseEvent::ME_MOUSE_DRAGENTERED:
				mMouseWithin = true;
				listener->mouseDragEntered(e);
				break;
			case MouseEvent::ME_MOUSE_DRAGEXITED:
				mMouseWithin = false;
				listener->mouseDragExited(e);
				break;
			case MouseEvent::ME_MOUSE_DRAGDROPPED:
				listener->mouseDragDropped(e);
				break;
			}
		}
	}

	void MouseTarget::addMouseListener(MouseListener* l) 
	{
		if (l == NULL) 
		{
			return;
		}
		mMouseListener = EventMulticaster::add(mMouseListener,l);
	}

	void MouseTarget::removeMouseListener(MouseListener* l) 
	{
		if (l == NULL) 
		{
			return;
		}
		mMouseListener = EventMulticaster::remove(mMouseListener,l);
	}

    //-----------------------------------------------------------------------
	bool MouseTarget::isMouseWithin() const
	{ 
		return mMouseWithin;
	}
}


/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

/***************************************************************************
OgreScrollTarget.h  -  
	Handles the adding and removing of ScrollListeners.
	This is implemented through the use of a binary multicaster.
	See EventMulticaster.

	GuiElements (or other components) that process the Scroll should subclass this class 
	and call processScrollEvent when that event is identified.

-------------------
begin                : Nov 19 2002
copyright            : (C) 2002 by Kenny Sabir
email                : kenny@sparksuit.com
***************************************************************************/
#ifndef __ScrollTarget_H__
#define __ScrollTarget_H__

#include "OgrePrerequisites.h"
#include "OgreEventTarget.h"

namespace Ogre {

    class _OgreExport ScrollTarget : public EventTarget
    {
    protected:
		ScrollListener* mScrollListener;

    public:
		ScrollTarget();
	void processScrollEvent(ScrollEvent* e) ;
	void addScrollListener(ScrollListener* l) ;
	void removeScrollListener(ScrollListener* l) ;
    };



}


#endif //__MouseTarget_H__

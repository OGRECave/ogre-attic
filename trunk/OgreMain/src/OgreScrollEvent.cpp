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
#include "OgreStableHeaders.h"

#include "OgreString.h"
#include "OgreScrollEvent.h"
#include "OgreScrollTarget.h"
#include "OgreStringConverter.h"

namespace Ogre {
 

    //-----------------------------------------------------------------------
	 ScrollEvent::ScrollEvent(ScrollTarget* source, int id, Real when, int modifiers,
		size_t topVisible, size_t range, size_t total) :
     InputEvent(source, id, when, modifiers),
		mTopVisible(topVisible),
		mRange(range), 
		mTotal(total)
	{
	}

    //-----------------------------------------------------------------------
	size_t ScrollEvent::getTopVisible() const
	{
		return mTopVisible;
	}

    //-----------------------------------------------------------------------
	size_t ScrollEvent::getRange() const
	{
		return mRange;
	}


    //-----------------------------------------------------------------------
	size_t ScrollEvent::getTotal() const
	{
		return mTotal;
	}

    //-----------------------------------------------------------------------
	String ScrollEvent::paramString() const {
		String typeStr;
		switch(mId) {
		  case SE_SCROLL_PERFORMED:
			  typeStr = "SCROLL_PERFORMED";
			  break;
		  default:
			  typeStr = "unknown type";
		}
		return typeStr + 
            ",top=" + StringConverter::toString(static_cast<unsigned int>(mTopVisible)) +
            ",range=" + StringConverter::toString(static_cast<unsigned int>(mRange)) + 
            ",total=" + StringConverter::toString(static_cast<unsigned int>(mTotal));
	}
    //-----------------------------------------------------------------------


}


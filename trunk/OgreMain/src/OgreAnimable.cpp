/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2005 The OGRE Team
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
#include "OgreAnimable.h"

namespace Ogre {
	//--------------------------------------------------------------------------
	AnimableObject::AnimableDictionaryMap AnimableObject::msAnimableDictionary;
	//--------------------------------------------------------------------------
	void AnimableValue::resetToBaseValue(void)
	{
		switch(mType)
		{
		case INT:
			setValue(mBaseValueInt);
			break;
		case REAL:
			setValue(mBaseValueReal[0]);
			break;
		case VECTOR2:
			setValue(Vector2(mBaseValueReal));
			break;
		case VECTOR3:
			setValue(Vector3(mBaseValueReal));
			break;
		case VECTOR4:
			setValue(Vector4(mBaseValueReal));
			break;
		case QUATERNION:
			setValue(Quaternion(mBaseValueReal));
			break;
		case COLOUR:
			setValue(ColourValue(mBaseValueReal[0], mBaseValueReal[1], 
				mBaseValueReal[2], mBaseValueReal[3]));
			break;
		}
	}

}

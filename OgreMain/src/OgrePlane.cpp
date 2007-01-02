/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgrePlane.h"
#include "OgreMatrix3.h"

namespace Ogre {
	//-----------------------------------------------------------------------
	Plane::Plane ()
	{
		normal = Vector3::ZERO;
		d = 0.0;
	}
	//-----------------------------------------------------------------------
	Plane::Plane (const Plane& rhs)
	{
		normal = rhs.normal;
		d = rhs.d;
	}
	//-----------------------------------------------------------------------
	Plane::Plane (const Vector3& rkNormal, Real fConstant)
	{
		normal = rkNormal;
		d = -fConstant;
	}
	//-----------------------------------------------------------------------
	Plane::Plane (const Vector3& rkNormal, const Vector3& rkPoint)
	{
		redefine(rkNormal, rkPoint);
	}
	//-----------------------------------------------------------------------
	Plane::Plane (const Vector3& rkPoint0, const Vector3& rkPoint1,
		const Vector3& rkPoint2)
	{
		redefine(rkPoint0, rkPoint1, rkPoint2);
	}
	//-----------------------------------------------------------------------
	Real Plane::getDistance (const Vector3& rkPoint) const
	{
		return normal.dotProduct(rkPoint) + d;
	}
	//-----------------------------------------------------------------------
	Plane::Side Plane::getSide (const Vector3& rkPoint) const
	{
		Real fDistance = getDistance(rkPoint);

		if ( fDistance < 0.0 )
			return Plane::NEGATIVE_SIDE;

		if ( fDistance > 0.0 )
			return Plane::POSITIVE_SIDE;

		return Plane::NO_SIDE;
	}


	//-----------------------------------------------------------------------
	Plane::Side Plane::getSide (const AxisAlignedBox& box) const
	{
		if (box.isNull()) 
			return NO_SIDE;
		if (box.isInfinite())
			return BOTH_SIDE;

		const Vector3& min = box.getMinimum();
		const Vector3& max = box.getMaximum();
		Plane::Side firstSide = getSide(min); // 0 min min min

		Vector3 currentCorner = min; 
		Plane::Side currentSide;

		currentCorner.z = max.z; // 6 min min max
		currentSide = getSide(currentCorner);
		if(currentSide != firstSide)
			return BOTH_SIDE;

		currentCorner.y = max.y; // 5 min max max
		currentSide = getSide(currentCorner);
		if(currentSide != firstSide)
			return BOTH_SIDE;

		currentCorner.z = min.z; // 1 min max min
		currentSide = getSide(currentCorner);
		if(currentSide != firstSide)
			return BOTH_SIDE;

		currentCorner.x = max.x; // 2 max max min
		currentSide = getSide(currentCorner);
		if(currentSide != firstSide)
			return BOTH_SIDE;

		currentCorner.z = max.z; // 4 max max max
		currentSide = getSide(currentCorner);
		if(currentSide != firstSide)
			return BOTH_SIDE;        

		currentCorner.y = min.y; // 7 max min max
		currentSide = getSide(currentCorner);
		if(currentSide != firstSide)
			return BOTH_SIDE;

		currentCorner.z = min.z; // 3 max min min
		currentSide = getSide(currentCorner);
		if(currentSide != firstSide)
			return BOTH_SIDE;

		return firstSide;
	}

	//-----------------------------------------------------------------------
	void Plane::redefine(const Vector3& rkPoint0, const Vector3& rkPoint1,
		const Vector3& rkPoint2)
	{
		Vector3 kEdge1 = rkPoint1 - rkPoint0;
		Vector3 kEdge2 = rkPoint2 - rkPoint0;
		normal = kEdge1.crossProduct(kEdge2);
		normal.normalise();
		d = -normal.dotProduct(rkPoint0);
	}
	//-----------------------------------------------------------------------
	void Plane::redefine(const Vector3& rkNormal, const Vector3& rkPoint)
	{
		normal = rkNormal;
		d = -rkNormal.dotProduct(rkPoint);
	}
	//-----------------------------------------------------------------------
	Vector3 Plane::projectVector(const Vector3& p)
	{
		// We know plane normal is unit length, so use simple method
		Matrix3 xform;
		xform[0][0] = normal.x * normal.x - 1.0f;
		xform[0][1] = normal.x * normal.y;
		xform[0][2] = normal.x * normal.z;
		xform[1][0] = normal.y * normal.x;
		xform[1][1] = normal.y * normal.y - 1.0f;
		xform[1][2] = normal.y * normal.z;
		xform[2][0] = normal.z * normal.x;
		xform[2][1] = normal.z * normal.y;
		xform[2][2] = normal.z * normal.z - 1.0f;
		return xform * p;

	}
	//-----------------------------------------------------------------------
	std::ostream& operator<< (std::ostream& o, Plane& p)
	{
		o << "Plane(normal=" << p.normal << ", d=" << p.d << ")";
		return o;
	}
} // namespace Ogre

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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
#include "OgreMovablePlane.h"
#include "OgreNode.h"

namespace Ogre {

    String MovablePlane::msMovableType = "MovablePlane";
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    MovablePlane::MovablePlane(const String& name) : Plane(),
        mName(name),
        mLastTranlate(Vector3::ZERO), 
        mLastRotate(Quaternion::IDENTITY),
        mDirty(true)
    {
    }
    //-----------------------------------------------------------------------
    MovablePlane::MovablePlane (const Plane& rhs) : Plane(rhs), 
        mLastTranlate(Vector3::ZERO), mLastRotate(Quaternion::IDENTITY), 
        mDirty(true)
    {
    }
    //-----------------------------------------------------------------------
    MovablePlane::MovablePlane (const Vector3& rkNormal, Real fConstant)
        : Plane (rkNormal, fConstant), mLastTranlate(Vector3::ZERO), 
        mLastRotate(Quaternion::IDENTITY), mDirty(true)
    {
    }
    //-----------------------------------------------------------------------
    MovablePlane::MovablePlane (const Vector3& rkNormal, const Vector3& rkPoint)
        : Plane(rkNormal, rkPoint), mLastTranlate(Vector3::ZERO), 
        mLastRotate(Quaternion::IDENTITY), mDirty(true)
    {
    }
    //-----------------------------------------------------------------------
    MovablePlane::MovablePlane (const Vector3& rkPoint0, const Vector3& rkPoint1,
        const Vector3& rkPoint2)
        : Plane(rkPoint0, rkPoint1, rkPoint2), mLastTranlate(Vector3::ZERO), 
        mLastRotate(Quaternion::IDENTITY), mDirty(true)
    {
    }
    //-----------------------------------------------------------------------
    const Plane& MovablePlane::_getDerivedPlane(void) const
    {
        if (mParentNode)
        {
            if (mDirty ||
                !(mParentNode->_getDerivedOrientation() == mLastRotate &&
                mParentNode->_getDerivedPosition() == mLastTranlate))
            {
                mLastRotate = mParentNode->_getDerivedOrientation();
                mLastTranlate = mParentNode->_getDerivedPosition();
                // Rotate normal
                mDerivedPlane.normal = mLastRotate * normal;
                // Dot the local normal with the transformed normal to 
                // adjust d
                mDerivedPlane.d = normal.dotProduct(mDerivedPlane.normal) * d;
                // Add on the effect of the translation (project onto new normal)
                mDerivedPlane.d -= mDerivedPlane.normal.dotProduct(mLastTranlate);

                mDirty = false;

            }
        }
        else
        {
            return *this;
        }

        return mDerivedPlane;
    }
    //-----------------------------------------------------------------------
    const String& MovablePlane::getName(void) const
    {
        return mName;
    }
    //-----------------------------------------------------------------------
    const String& MovablePlane::getMovableType(void) const
    {
        return msMovableType;
    }
}

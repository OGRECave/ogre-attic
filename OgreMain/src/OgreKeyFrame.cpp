/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/

#include "OgreKeyFrame.h"

namespace Ogre
{
    //---------------------------------------------------------------------
    KeyFrame::KeyFrame(Real time) : mTime(time)
    {
        mTranslate = Vector3::ZERO;
        mScale.x = mScale.y = mScale.z = 1.0;
        mRotate = Quaternion::IDENTITY;
    }
    //---------------------------------------------------------------------
    KeyFrame::KeyFrame() : mTime(0.0)
    {
        mTranslate = Vector3::ZERO;
        mScale.x = mScale.y = mScale.z = 1.0;
        mRotate = Quaternion::IDENTITY;
    }
    //---------------------------------------------------------------------
    Real KeyFrame::getTime(void) const
    {
        return mTime;
    }
    //---------------------------------------------------------------------
    void KeyFrame::setTranslate(const Vector3& trans)
    {
        mTranslate = trans;
    }
    //---------------------------------------------------------------------
    Vector3 KeyFrame::getTranslate(void) const
    {
        return mTranslate;
    }
    //---------------------------------------------------------------------
    void KeyFrame::setScale(const Vector3& scale)
    {
        mScale = scale;
    }
    //---------------------------------------------------------------------
    Vector3 KeyFrame::getScale(void) const
    {
        return mScale;
    }
    //---------------------------------------------------------------------
    void KeyFrame::setRotation(const Quaternion& rot)
    {
        mRotate = rot;
    }
    //---------------------------------------------------------------------
    Quaternion KeyFrame::getRotation(void) const
    {
        return mRotate;
    }

}


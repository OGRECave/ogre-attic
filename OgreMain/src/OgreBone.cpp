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

#include "OgreBone.h"
#include "OgreSkeleton.h"


namespace Ogre {

    //---------------------------------------------------------------------
    Bone::Bone(unsigned short handle, Skeleton* creator) 
        : mCreator(creator), mHandle(handle)
    {
    }
    //---------------------------------------------------------------------
    Bone::~Bone()
    {
    }
    //---------------------------------------------------------------------
    Bone* Bone::createChild(const Vector3& translate, const Quaternion& rotate)
    {
        // Provided as a passthrough to superclass just for ease
        return static_cast<Bone*>(Node::createChild(translate, rotate));
    }
    //---------------------------------------------------------------------
    Bone* Bone::createChild(unsigned short handle, const Vector3& translate, 
        const Quaternion& rotate)
    {
        Bone* retBone = mCreator->createBone(handle);
        retBone->translate(translate);
        retBone->rotate(rotate);
        this->addChild(retBone);
        return retBone;
    }
    //---------------------------------------------------------------------
    Bone* Bone::getChild(unsigned short index)
    {
        // Provided as a passthrough to superclass just for ease
        return static_cast<Bone*>(Node::getChild(index));
    }
    //---------------------------------------------------------------------
    Bone* Bone::removeChild(unsigned short index)
    {
        // Provided as a passthrough to superclass just for ease
        return static_cast<Bone*>(Node::removeChild(index));
    }
    //---------------------------------------------------------------------
    Node* Bone::createChildImpl(void)
    {
        return mCreator->createBone();
    }
    //---------------------------------------------------------------------
    void Bone::setBindingPose(void)
    {
        // Bake the current status into the binding pose
        mBindOrientation = mOrientation;
        mBindPosition = mPosition;

        // Save derived, used for mesh transform later (assumes _update() has been called by Skeleton)
        mBindDerivedOrientation = mDerivedOrientation;
        mBindDerivedPosition = mDerivedPosition;
    }
    //---------------------------------------------------------------------
    void Bone::reset(void)
    {
        // Restore from binding pose
        mOrientation = mBindOrientation;
        mPosition = mBindPosition;
    }




}


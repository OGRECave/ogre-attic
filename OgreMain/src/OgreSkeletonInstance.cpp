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
#include "OgreSkeletonInstance.h"
#include "OgreBone.h"
#include "OgreTagPoint.h"


namespace Ogre {
    //-------------------------------------------------------------------------
    SkeletonInstance::SkeletonInstance(Skeleton* masterCopy) 
        : Skeleton(""), mSkeleton(masterCopy)
    {
        mNextTagPointAutoHandle = 0;
    }
    //-------------------------------------------------------------------------
    SkeletonInstance::~SkeletonInstance()
    {
    }
    //-------------------------------------------------------------------------
    unsigned short SkeletonInstance::getNumAnimations(void) const
    {
        return mSkeleton->getNumAnimations();
    }
    //-------------------------------------------------------------------------
    Animation* SkeletonInstance::getAnimation(unsigned short index) const
    {
        return mSkeleton->getAnimation(index);
    }
    //-------------------------------------------------------------------------
    Animation* SkeletonInstance::createAnimation(const String& name, Real length)
    {
        return mSkeleton->createAnimation(name, length);
    }
    //-------------------------------------------------------------------------
    Animation* SkeletonInstance::getAnimation(const String& name) const
    {
        return mSkeleton->getAnimation(name);
    }
    //-------------------------------------------------------------------------
    void SkeletonInstance::removeAnimation(const String& name)
    {
        mSkeleton->removeAnimation(name);
    }
    //-------------------------------------------------------------------------
    void SkeletonInstance::cloneBoneAndChildren(Bone* source, Bone* parent)
    {
        Bone* newBone;
        if (source->getName() == "")
        {
            newBone = createBone(source->getHandle());
        }
        else
        {
            newBone = createBone(source->getName(), source->getHandle());
        }
        if (parent == NULL)
        {
            mRootBone = newBone;
        }
        else
        {
            parent->addChild(newBone);
        }
        newBone->setOrientation(source->getOrientation());
        newBone->setPosition(source->getPosition());
        newBone->setScale(source->getScale());

        // Process children
        Node::ChildNodeIterator it = source->getChildIterator();
        while (it.hasMoreElements())
        {
            cloneBoneAndChildren(static_cast<Bone*>(it.getNext()), newBone);
        }
    }
    //-------------------------------------------------------------------------
    void SkeletonInstance::load(void)
    {
        mNextAutoHandle = mSkeleton->mNextAutoHandle;
        mNextTagPointAutoHandle = 0;
        // construct self from master
        mBlendState = mSkeleton->mBlendState;
        // Copy bones
        cloneBoneAndChildren(mSkeleton->mRootBone, 0);
        mRootBone->_update(true, false);
        setBindingPose();
        mIsLoaded = true;
    }
    //-------------------------------------------------------------------------
    void SkeletonInstance::unload(void)
    {
        Skeleton::unload();

        // destroy TagPoints
        TagPointList::iterator itp;
        for (itp = mTagPointList.begin(); itp != mTagPointList.end(); ++itp)
        {
            delete itp->second;
        }
        mTagPointList.clear();
    }

    //-------------------------------------------------------------------------
    TagPoint* SkeletonInstance::createTagPointOnBone(Bone* bone,
        const Quaternion &offsetOrientation, 
        const Vector3 &offsetPosition)
    {
        TagPoint* ret = new TagPoint(mNextTagPointAutoHandle++, this);
        mTagPointList[mNextTagPointAutoHandle] = ret;

        ret->translate(offsetPosition);
        ret->rotate(offsetOrientation);
        ret->setBindingPose();
        bone->addChild(ret);

        return ret;
    }

}


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
#include "OgreSkeleton.h"
#include "OgreBone.h"
#include "OgreAnimation.h"
#include "OgreAnimationState.h"
#include "OgreException.h"


namespace Ogre {

    //---------------------------------------------------------------------
    Skeleton::Skeleton(String name) 
    {
        mName = name;
        // Start auto handles high to avoid problems with combinations
        mNextAutoHandle = 32768;

        // Create root bone, handle 0
        mRootBone = createBone(0);

    }
    //---------------------------------------------------------------------
    Skeleton::~Skeleton()
    {
        unload();
        // delete root bone
        delete mRootBone;
    }
    //---------------------------------------------------------------------
    void Skeleton::load(void)
    {
        //TODO
    }
    //---------------------------------------------------------------------
    void Skeleton::unload(void)
    {
        //TODO

        // destroy bones, except root
        BoneList::iterator i;
        for (i = mBoneList.begin(); i != mBoneList.end(); ++i)
        {
            if (i->second != mRootBone) // If not root
                delete i->second;
        }
        mBoneList.clear();

        // Destroy animations
        AnimationList::iterator ai;
        for (ai = mAnimationsList.begin(); ai != mAnimationsList.end(); ++i)
        {
            delete ai->second;
        }
        mAnimationsList.clear();

    }
    //---------------------------------------------------------------------
    Bone* Skeleton::createBone(void)
    {
        // use autohandle
        return createBone(mNextAutoHandle++);
    }
    //---------------------------------------------------------------------
    Bone* Skeleton::createBone(unsigned short handle)
    {
        Bone* ret = new Bone(handle, this);
        mBoneList[handle] = ret;
        return ret;

    }
    //---------------------------------------------------------------------
    Bone* Skeleton::getRootBone(void)
    {
        return mRootBone;

    }
    //---------------------------------------------------------------------
    void Skeleton::setAnimationState(const AnimationStateSet& animSet)
    {
        // TODO
        /* 
        Algorithm:
          1. Check if animation state is any different from last, if not do nothing
          2. Reset all bone positions
          3. Iterate per AnimationState, if enabled get Animation and call Animation::apply
        */

        if (mLastAnimationState.size() == animSet.size())
        {
            // Same size, may be able to skip update
            bool different = false;
            AnimationStateSet::iterator i;
            AnimationStateSet::const_iterator j;
            i = mLastAnimationState.begin();
            j = animSet.begin();
            for (; i != mLastAnimationState.end(); ++i, ++j)
            {
                if (i->second != j->second)
                {
                    different = true;
                    break;
                }
            }
            // Check any differences?
            if (!different)
            {
                // No, no need to update
                return;
            }
        }

        // Ok, we've established the animation state is different

        // Reset bones
        reset();

        // Per animation state
        AnimationStateSet::const_iterator istate;
        for (istate = animSet.begin(); istate != animSet.end(); ++istate)
        {
            // Apply if enabled
            const AnimationState& animState = istate->second;
            if (animState.getEnabled())
            {
                Animation* anim = getAnimation(animState.getAnimationName());
                anim->apply(animState.getTimePosition(), animState.getWeight());
            }
        }

    }
    //---------------------------------------------------------------------
    void Skeleton::setBindingPose(void)
    {
        BoneList::iterator i;
        for (i = mBoneList.begin(); i != mBoneList.end(); ++i)
        {
            i->second->setBindingPose();
        }
    }
    //---------------------------------------------------------------------
    void Skeleton::reset(void)
    {
        BoneList::iterator i;
        for (i = mBoneList.begin(); i != mBoneList.end(); ++i)
        {
            i->second->reset();
        }
    }
    //---------------------------------------------------------------------
    Animation* Skeleton::createAnimation(const String& name, Real length)
    {
        Animation* ret = new Animation(name, length);

        // Add to list
        mAnimationsList[name] = ret;

        // Also add to state
        mLastAnimationState[name] = AnimationState(name, 0, length);

        return ret;

    }
    //---------------------------------------------------------------------
    Animation* Skeleton::getAnimation(const String& name)
    {
        AnimationList::iterator i = mAnimationsList.find(name);

        if (i == mAnimationsList.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "No animation entry found named " + name, 
            "Skeleton::getAnimation");
        }

        return i->second;
    }
    //---------------------------------------------------------------------
    void Skeleton::removeAnimation(const String& name)
    {
        AnimationList::iterator i = mAnimationsList.find(name);

        if (i == mAnimationsList.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "No animation entry found named " + name, 
            "Skeleton::getAnimation");
        }

        delete i->second;

        mAnimationsList.erase(i);

    }
    //---------------------------------------------------------------------
    const AnimationStateSet& Skeleton::getAnimationState(void)
    {
        return mLastAnimationState;
    }


}

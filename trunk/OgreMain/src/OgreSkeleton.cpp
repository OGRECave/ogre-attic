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
#include "OgreLogManager.h"
#include "OgreSkeletonManager.h"
#include "OgreSkeletonSerializer.h"


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
        // Load from specified 'name'
        if (mIsLoaded)
        {
            unload();
            mIsLoaded = false;
        }

        SkeletonSerializer serializer;
        char msg[100];
        sprintf(msg, "Skeleton: Loading %s .", mName.c_str());
        LogManager::getSingleton().logMessage(msg);

        DataChunk chunk;
        SkeletonManager::getSingleton()._findResourceData(mName, chunk);

        // Determine file type
        std::vector<String> extVec = mName.split(".");

        String& ext = extVec[extVec.size() - 1];
        ext.toLowerCase();

        if (ext == "skeleton")
        {
            serializer.importSkeleton(chunk, this);
        }
        else
        {
            // Unsupported format
            chunk.clear();
            Except(999, "Unsupported skeleton file format.",
                "Skeleton::load");
        }

        chunk.clear();

    }
    //---------------------------------------------------------------------
    void Skeleton::unload(void)
    {
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
        if (mBoneList.size() == OGRE_MAX_NUM_BONES)
        {
            Except(Exception::ERR_INVALIDPARAMS, "Exceeded the maximum number of bones per skeleton.",
                "Skeleton::createBone");
        }
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
        // Update the derived transforms
        mRootBone->_update();


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
    //-----------------------------------------------------------------------
    void Skeleton::_initAnimationState(AnimationStateSet* animSet)
    {
        animSet->clear();
           
        AnimationList::iterator i;
        for (i = mAnimationsList.begin(); i != mAnimationsList.end(); ++i)
        {
            Animation* anim = i->second;
            // Create animation at time index 0, default params mean this has weight 1 and is disabled
            String animName = anim->getName();
            (*animSet)[animName] = AnimationState(animName, 0.0, anim->getLength());
        }
    }
    //-----------------------------------------------------------------------
    unsigned short Skeleton::getNumBones(void)
    {
        return (unsigned short)mBoneList.size();
    }
    //-----------------------------------------------------------------------
    void Skeleton::_getBoneMatrices(Matrix4* pMatrices)
    {
        // Update derived transforms
        mRootBone->_update();

        /* 
            Calculating the bone matrices
            -----------------------------
            Now that we have the derived orientations & positions in the Bone nodes, we have
            to compute the Matrix4 to apply to the vertices of a mesh.
            Because any modification of a vertex has to be relative to the bone, we must first
            reverse transform by the Bone's original derived position/orientation, then transform
            by the new derived position / orientation.
        */

        BoneList::iterator i, boneend;
        boneend = mBoneList.end();
        
        
        for(i = mBoneList.begin();i != boneend; ++i)
        {
            Bone* pBone = i->second;
            *pMatrices = pBone->_getFullTransform() *  pBone->_getBindingPoseInverseTransform();
            pMatrices++;
        }

    }



}

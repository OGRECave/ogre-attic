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
// Just for logging
#include "OgreAnimationTrack.h"
#include "OgreKeyFrame.h"


namespace Ogre {

    //---------------------------------------------------------------------
    Skeleton::Skeleton(String name) 
    {
        mName = name;

        // Start next handle
        mNextAutoHandle = 0;

        // Indicate root has not been derived yet
        mRootBone = 0;

    }
    //---------------------------------------------------------------------
    Skeleton::~Skeleton()
    {
        unload();
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

#if OGRE_DEBUG_MODE
        _dumpContents(mName + ".log");
#endif

    }
    //---------------------------------------------------------------------
    void Skeleton::unload(void)
    {
        // destroy bones
        BoneList::iterator i;
        for (i = mBoneList.begin(); i != mBoneList.end(); ++i)
        {
            delete i->second;
        }
        mBoneList.clear();

        // Destroy animations
        AnimationList::iterator ai;
        for (ai = mAnimationsList.begin(); ai != mAnimationsList.end(); ++ai)
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
    Bone* Skeleton::createBone(const String& name)
    {
        if (mBoneList.size() == OGRE_MAX_NUM_BONES)
        {
            Except(Exception::ERR_INVALIDPARAMS, "Exceeded the maximum number of bones per skeleton.",
                "Skeleton::createBone");
        }
        Bone* ret = new Bone(name, mNextAutoHandle++, this);
        mBoneList[ret->getHandle()] = ret;
        mBoneListByName[name] = ret;
        return ret;
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
        mBoneListByName[ret->getName()] = ret;
        return ret;

    }
    //---------------------------------------------------------------------
    Bone* Skeleton::getRootBone(void) const
    {
        if (mRootBone == 0)
        {
            deriveRootBone();
        }

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
        getRootBone()->_update();


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
    Animation* Skeleton::getAnimation(const String& name) const
    {
        AnimationList::const_iterator i = mAnimationsList.find(name);

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
    const AnimationStateSet& Skeleton::getAnimationState(void) const
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
    unsigned short Skeleton::getNumBones(void) const
    {
        return (unsigned short)mBoneList.size();
    }
    //-----------------------------------------------------------------------
    void Skeleton::_getBoneMatrices(Matrix4* pMatrices)
    {
        // Update derived transforms
        getRootBone()->_update();

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
    //---------------------------------------------------------------------
    unsigned short Skeleton::getNumAnimations(void) const
    {
        return (unsigned short)mAnimationsList.size();
    }
    //---------------------------------------------------------------------
    Animation* Skeleton::getAnimation(unsigned short index) const
    {
		// If you hit this assert, then the index is out of bounds.
        assert( index < mAnimationsList.size() );

        AnimationList::const_iterator i = mAnimationsList.begin();

        while (index--)
            ++i;

        return i->second;
    }
    //---------------------------------------------------------------------
    Bone* Skeleton::getBone(unsigned short handle) const
    {
        BoneList::const_iterator i = mBoneList.find(handle);

        if (i == mBoneList.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Bone with specified handle not found.", 
                "Skeleton::getBone");
        }

        return i->second;

    }
    //---------------------------------------------------------------------
    Bone* Skeleton::getBone(const String& name) const
    {
        BoneListByName::const_iterator i = mBoneListByName.find(name);

        if (i == mBoneListByName.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Bone named '" + name + "' not found.", 
                "Skeleton::getBone");
        }

        return i->second;

    }
    //---------------------------------------------------------------------
    void Skeleton::deriveRootBone(void) const
    {
        // Start at the first bone and work up
        if (mBoneList.empty())
        {
            Except(Exception::ERR_INVALIDPARAMS, "Cannot derive root bone as this "
                "skeleton has no bones!", "Skeleton::deriveRootBone");
        }

        Bone* currentBone;
        BoneList::const_iterator i = mBoneList.begin();

        currentBone = i->second;
        while (currentBone->getParent() != 0)
        {
            // Keep going up the tree
            currentBone = (Bone*)currentBone->getParent();
        }

        // No more parents, this must be the root
        mRootBone = currentBone;
    }
    //---------------------------------------------------------------------
    void Skeleton::_dumpContents(const String& filename)
    {
        std::ofstream of;

        Quaternion q;
        Real angle;
        Vector3 axis;
        of.open(filename);

        of << "-= Debug output of skeleton " << mName << " =-" << std::endl << std::endl;
        of << "== Bones ==" << std::endl;
        of << "Number of bones: " << mBoneList.size() << std::endl;
        
        BoneList::iterator bi;
        for (bi = mBoneList.begin(); bi != mBoneList.end(); ++bi)
        {
            Bone* bone = bi->second;

            of << "-- Bone " << bone->getHandle() << " --" << std::endl;
            of << "Position: " << bone->getPosition();
            q = bone->getOrientation();
            of << "Rotation: " << q;
            q.ToAngleAxis(angle, axis);
            of << " = " << angle << " radians around axis " << axis << std::endl << std::endl;
        }

        of << "== Animations ==" << std::endl;
        of << "Number of animations: " << mAnimationsList.size() << std::endl;

        AnimationList::iterator ai;
        for (ai = mAnimationsList.begin(); ai != mAnimationsList.end(); ++ai)
        {
            Animation* anim = ai->second;

            of << "-- Animation '" << anim->getName() << "' (length " << anim->getLength() << ") --" << std::endl;
            of << "Number of tracks: " << anim->getNumTracks() << std::endl;

            int ti;
            for (ti = 0; ti < anim->getNumTracks(); ++ti)
            {
                AnimationTrack* track = anim->getTrack(ti);
                of << "  -- AnimationTrack " << ti << " --" << std::endl;
                of << "  Affects bone: " << ((Bone*)track->getAssociatedNode())->getHandle() << std::endl;
                of << "  Number of keyframes: " << track->getNumKeyFrames() << std::endl;

                int ki;
                
                for (ki = 0; ki < track->getNumKeyFrames(); ++ki)
                {
                    KeyFrame* key = track->getKeyFrame(ki);
                    of << "    -- KeyFrame " << ki << " --" << std::endl;
                    of << "    Time index: " << key->getTime(); 
                    of << "    Translation: " << key->getTranslate() << std::endl;
                    q = key->getRotation();
                    of << "    Rotation: " << q;
                    q.ToAngleAxis(angle, axis);
                    of << " = " << angle << " radians around axis " << axis << std::endl;
                }

            }



        }

    }

}


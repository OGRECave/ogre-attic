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

#ifndef __Skeleton_H__
#define __Skeleton_H__

#include "OgrePrerequisites.h"
#include "OgreResource.h"
#include "OgreAnimationState.h"

namespace Ogre {

#define OGRE_MAX_NUM_BONES 256

    /** A collection of Bone objects used to animate a skinned mesh.
    @remarks
        Skeletal animation works by having a collection of 'bones' which are 
        actually just joints with a position and orientation, arranged in a tree structure.
        For example, the wrist joint is a child of the elbow joint, which in turn is a
        child of the shoulder joint. Rotating the shoulder automatically moves the elbow
        and wrist as well due to this hierarchy.
    @par
        So how does this animate a mesh? Well every vertex in a mesh is assigned to one or more
        bones which affects it's position when the bone is moved. If a vertex is assigned to 
        more than one bone, then weights must be assigned to determine how much each bone affects
        the vertex (actually a weight of 1.0 is used for single bone assignments). 
        Weighted vertex assignments are especially useful around the joints themselves
        to avoid 'pinching' of the mesh in this region. 
    @par
        Therefore by moving the skeleton using preset animations, we can animate the mesh. The
        advantage of using skeletal animation is that you store less animation data, especially
        as vertex counts increase. In addition, you are able to blend multiple animations together
        (e.g. walking and looking around, running and shooting) and provide smooth transitions
        between animations without incurring as much of an overhead as would be involved if you
        did this on the core vertex data.
    @par
        Skeleton definitions are loaded from datafiles, namely the .skeleton file format. They
        are loaded on demand, especially when referenced by a Mesh.
    */
    class _OgreExport Skeleton : public Resource
    {
    public:
        /** Constructor, don't call directly, use SkeletonManager.
        @remarks
            On creation, a Skeleton has a single Bone, namely the 'root bone' which 
            is the joint from which all other bones start (a common choice is the
            pelvis, but it can be any bone).
        */
        Skeleton(String name);
        ~Skeleton();

        /** Generic load - called by SkeletonManager.
        */
        virtual void load(void);

        /** Generic unload - called by SkeletonManager.
        */
        virtual void unload(void);

        /** Creates a brand new Bone owned by this Skeleton. 
        @remarks
            This method creates an unattached new Bone for this skeleton. You must
            attach it to another Bone in the skeleton for it to be any use. For this
            reason you will likely be better off creating child bones using the
            Bone::createChild method instead. You can start with the root bone (which 
            is always created for you) by calling the getRootBone method.
        @par
            Note that this method automatically generates a handle for the bone, which you
            can retrieve using Bone::getHandle. If you wish the new Bone to have a specific
            handle, use the alternate form of this method which takes a handle as a parameter,
            although you should note the restrictions.
        */
        Bone* createBone(void);

        /** Creates a brand new Bone owned by this Skeleton. 
        @remarks
            This method creates an unattached new Bone for this skeleton and assigns it
            the specified handle. You must attach it to another Bone in the skeleton 
            for it to be any use. For this reason you will likely be better off creating 
            child bones using the Bone::createChild method instead. You can start with 
            the root bone (which is always created for you) by calling the getRootBone method.
        @param handle The handle to give to this new bone - must be unique within this skeleton. Note
            that the root bone of the skeleton (created for you when the Skeleton is created) always
            has handle 0, so don't try to use this one.
            You should also ensure that all bone handles are eventually contiguous. For this reason
            it is advised that you use the simpler createBone method which automatically assigns a
            handle.
        */
        Bone* createBone(unsigned short handle);

        /** Returns the number of bones in this skeleton. */
        unsigned short getNumBones(void) const;

        /** Gets the root bone of the skeleton. */
        Bone* getRootBone(void) const;

        /** Gets a bone by it's handle. */
        Bone* getBone(unsigned short handle) const;

        /** Sets the current position / orientation to be the 'binding pose' ie the layout in which 
            bones were originally bound to a mesh.
        */
        void setBindingPose(void);

        /** Resets the position and orientation of all bones in this skeleton to their original binding position.
        @remarks
            A skeleton is bound to a mesh in a binding pose. Bone positions are then modified from this
            position during animation. This method returns all the bones to their original position and
            orientation.
        */
        void reset(void);

        /** Creates a new Animation object for animating this skeleton. 
        @param name The name of this animation
        @param length The length of the animation in seconds
        */
        Animation* createAnimation(const String& name, Real length);

        /** Returns the named Animation object. */
        Animation* getAnimation(const String& name) const;

        /** Removes an Animation from this skeleton. */
        void removeAnimation(const String& name);

        /** Changes the state of the skeleton to reflect the application of the passed in collection of animations.
        @remarks
            Animating a skeleton involves both interpolating between keyframes of a specific animation,
            and blending between the animations themselves. Calling this method sets the state of
            the skeleton so that it reflects the combination of all the passed in animations, at the
            time index specified for each, using the weights specified. Note that the weights between 
            animations do not have to sum to 1.0, because some animations may affect only subsets
            of the skeleton. If the weights exceed 1.0 for the same area of the skeleton, the 
            movement will just be exaggerated.
            @param 
        */
        void setAnimationState(const AnimationStateSet& animSet);

        /** Gets the last animation state of this skeleton. */
        const AnimationStateSet& getAnimationState(void) const;
        

        /** Initialise an animation set suitable for use with this mesh. 
        @remarks
            Only recommended for use inside the engine, not by applications.
        */
        void _initAnimationState(AnimationStateSet* animSet);

        /** Populates the passed in array with the bone matrices based on the current position.
        @remarks
            Internal use only. The array pointed to by the passed in pointer must
            be at least as large as the number of bones.
            Assumes animation has already been updated.
        */
        void _getBoneMatrices(Matrix4* pMatrices);

        /** Gets the number of animations on this skeleton. */
        unsigned short getNumAnimations(void) const;

        /** Gets a single animation by index. */
        Animation* getAnimation(unsigned short index) const;



    protected:
        /// Storage of bones, lookup by bone handle
        typedef std::map<unsigned short, Bone*> BoneList;
        BoneList mBoneList;
        /// Pointer to root bone (all others follow)
        Bone *mRootBone;
        /// Automatic handles
        unsigned short mNextAutoHandle;

        /// Storage of animations, lookup by name
        typedef std::map<String, Animation*> AnimationList;
        AnimationList mAnimationsList;


        /// Saved version of last animation
        AnimationStateSet mLastAnimationState;


    };

}


#endif


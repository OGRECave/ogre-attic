/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
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

#ifndef __Skeleton_H__
#define __Skeleton_H__

#include "OgrePrerequisites.h"
#include "OgreResource.h"
#include "OgreAnimationState.h"
#include "OgreQuaternion.h"
#include "OgreVector3.h"

namespace Ogre {

    /**  */
    enum SkeletonAnimationBlendMode {
        /// Animations are applied by calculating a weighted average of all animations
	    ANIMBLEND_AVERAGE,
        /// Animations are applied by calculating a weighted cumulative total
	    ANIMBLEND_CUMULATIVE
    };

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
        friend class SkeletonInstance;
    public:
        /** Constructor, don't call directly, use SkeletonManager.
        @remarks
            On creation, a Skeleton has a no bones, you should create them and link
            them together appropriately. Unless you state otherwise by attaching it to 
            a higher bone, the first bone you create is deemed to be the root bone.
        */
        Skeleton(const String& name);
        virtual ~Skeleton();

        /** Generic load - called by SkeletonManager.
        */
        virtual void load(void);

        /** Generic unload - called by SkeletonManager.
        */
        virtual void unload(void);

        /** Creates a brand new Bone owned by this Skeleton. 
        @remarks
            This method creates an unattached new Bone for this skeleton. Unless this is to
            be the root bone (there must only be one of these), you must
            attach it to another Bone in the skeleton using addChild for it to be any use. 
            For this reason you will likely be better off creating child bones using the
            Bone::createChild method instead, once you have created the root bone. 
        @par
            Note that this method automatically generates a handle for the bone, which you
            can retrieve using Bone::getHandle. If you wish the new Bone to have a specific
            handle, use the alternate form of this method which takes a handle as a parameter,
            although you should note the restrictions.
        */
        virtual Bone* createBone(void);

        /** Creates a brand new Bone owned by this Skeleton. 
        @remarks
            This method creates an unattached new Bone for this skeleton and assigns it a 
            specific handle. Unless this is to
            be the root bone (there must only be one of these), you must
            attach it to another Bone in the skeleton using addChild for it to be any use. 
            For this reason you will likely be better off creating child bones using the
            Bone::createChild method instead, once you have created the root bone. 
        @param handle The handle to give to this new bone - must be unique within this skeleton. 
            You should also ensure that all bone handles are eventually contiguous (this is to simplify
            their compilation into an indexed array of transformation matrices). For this reason
            it is advised that you use the simpler createBone method which automatically assigns a
            sequential handle starting from 0.
        */
        virtual Bone* createBone(unsigned short handle);

        /** Creates a brand new Bone owned by this Skeleton. 
        @remarks
            This method creates an unattached new Bone for this skeleton and assigns it a 
            specific name. Unless this is to
            be the root bone (there must only be one of these), you must
            attach it to another Bone in the skeleton using addChild for it to be any use. 
            For this reason you will likely be better off creating child bones using the
            Bone::createChild method instead, once you have created the root bone. 
        @param name The name to give to this new bone - must be unique within this skeleton. 
            Note that the way OGRE looks up bones is via a numeric handle, so if you name a
            Bone this way it will be given an automatic sequential handle. The name is just
            for your convenience, although it is recommended that you only use the handle to 
            retrieve the bone in performance-critical code.
        */
        virtual Bone* createBone(const String& name);

        /** Creates a brand new Bone owned by this Skeleton. 
        @remarks
            This method creates an unattached new Bone for this skeleton and assigns it a 
            specific name and handle. Unless this is to
            be the root bone (there must only be one of these), you must
            attach it to another Bone in the skeleton using addChild for it to be any use. 
            For this reason you will likely be better off creating child bones using the
            Bone::createChild method instead, once you have created the root bone. 
        @param name The name to give to this new bone - must be unique within this skeleton. 
        @param handle The handle to give to this new bone - must be unique within this skeleton. 
        */
        virtual Bone* createBone(const String& name, unsigned short handle);

        /** Returns the number of bones in this skeleton. */
        virtual unsigned short getNumBones(void) const;

        /** Gets the root bone of the skeleton: deprecated in favour of getRootBoneIterator. 
        @remarks
            The system derives the root bone the first time you ask for it. The root bone is the
            only bone in the skeleton which has no parent. The system locates it by taking the
            first bone in the list and going up the bone tree until there are no more parents,
            and saves this top bone as the root. If you are building the skeleton manually using
            createBone then you must ensure there is only one bone which is not a child of 
            another bone, otherwise your skeleton will not work properly. If you use createBone
            only once, and then use Bone::createChild from then on, then inherently the first
            bone you create will by default be the root.
        */
        virtual Bone* getRootBone(void) const;

        typedef std::vector<Bone*> BoneList;
        typedef VectorIterator<BoneList> BoneIterator;
        /// Get an iterator over the root bones in the skeleton, ie those with no parents
        virtual BoneIterator getRootBoneIterator(void);
        /// Get an iterator over all the bones in the skeleton
        virtual BoneIterator getBoneIterator(void);

        /** Gets a bone by it's handle. */
        virtual Bone* getBone(unsigned short handle) const;

        /** Gets a bone by it's name. */
        virtual Bone* getBone(const String& name) const;

        /** Sets the current position / orientation to be the 'binding pose' ie the layout in which 
            bones were originally bound to a mesh.
        */
        virtual void setBindingPose(void);

        /** Resets the position and orientation of all bones in this skeleton to their original binding position.
        @remarks
            A skeleton is bound to a mesh in a binding pose. Bone positions are then modified from this
            position during animation. This method returns all the bones to their original position and
            orientation.
        @param resetManualBones If set to true, causes the state of manual bones to be reset
            too, which is normally not done to allow the manual state to persist even 
            when keyframe animation is applied.
        */
        virtual void reset(bool resetManualBones = false);

        /** Creates a new Animation object for animating this skeleton. 
        @param name The name of this animation
        @param length The length of the animation in seconds
        */
        virtual Animation* createAnimation(const String& name, Real length);

        /** Returns the named Animation object. */
        virtual Animation* getAnimation(const String& name) const;

        /** Removes an Animation from this skeleton. */
        virtual void removeAnimation(const String& name);

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
        virtual void setAnimationState(const AnimationStateSet& animSet);

        /** Gets the last animation state of this skeleton. */
        virtual const AnimationStateSet& getAnimationState(void) const;
        

        /** Initialise an animation set suitable for use with this mesh. 
        @remarks
            Only recommended for use inside the engine, not by applications.
        */
        virtual void _initAnimationState(AnimationStateSet* animSet);

        /** Populates the passed in array with the bone matrices based on the current position.
        @remarks
            Internal use only. The array pointed to by the passed in pointer must
            be at least as large as the number of bones.
            Assumes animation has already been updated.
        */
        virtual void _getBoneMatrices(Matrix4* pMatrices);

        /** Gets the number of animations on this skeleton. */
        virtual unsigned short getNumAnimations(void) const;

        /** Gets a single animation by index. */
        virtual Animation* getAnimation(unsigned short index) const;


		/** Gets the animation blending mode which this skeleton will use. */
        virtual SkeletonAnimationBlendMode getBlendMode();
        /** Sets the animation blending mode this skeleton will use. */
		virtual void setBlendMode(SkeletonAnimationBlendMode state);

        /// Updates all the derived transforms in the skeleton
        virtual void _updateTransforms(void);


    protected:
		SkeletonAnimationBlendMode mBlendState;
        /// Storage of bones, indexed by bone handle
        BoneList mBoneList;
        /// Lookup by bone name
        typedef std::map<String, Bone*> BoneListByName;
        BoneListByName mBoneListByName;


        /// Pointer to root bones (can now have multiple roots)
        mutable BoneList mRootBones;
        /// Bone automatic handles
        unsigned short mNextAutoHandle;


        /// Storage of animations, lookup by name
        typedef std::map<String, Animation*> AnimationList;
        AnimationList mAnimationsList;


        /// Saved version of last animation
        AnimationStateSet mLastAnimationState;

        /** Internal method which parses the bones to derive the root bone. 
        @remarks
            Must be const because called in getRootBone but mRootBone is mutable
            since lazy-updated.
        */
        void deriveRootBone(void) const;

        /// Debugging method
        void _dumpContents(const String& filename);


    };

}


#endif


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

namespace Ogre {

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
            handle, use the alternate form of this method which takes a handle as a parameter.
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
        */
        Bone* createBone(unsigned short handle);

        /** Gets the root bone of the skeleton. */
        Bone* getRootBone(void);


    protected:
        /// Storage of bones, lookup by bone handle
        typedef std::map<unsigned short, Bone*> BoneList;
        BoneList mBoneList;
        /// Pointer to root bone (all others follow)
        Bone *mRootBone;
        /// Automatic handles
        unsigned short mNextAutoHandle;


    };

}


#endif


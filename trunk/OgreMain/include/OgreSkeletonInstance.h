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

#ifndef __SkeletonInstance_H__
#define __SkeletonInstance_H__

#include "OgrePrerequisites.h"
#include "OgreSkeleton.h"

namespace Ogre {

    /** A SkeletonInstance is a single instance of a Skeleton used by a world object.
    @remarks
        The difference between a Skeleton and a SkeletonInstance is that the
        Skeleton is the 'master' version much like Mesh is a 'master' version of
        Entity. Many SkeletonInstance objects can be based on a single Skeleton, 
        and are copies of it when created. Any changes made to this are not
        reflected in the master copy. The exception is animations; these are
        shared on the Skeleton itself and may not be modified here.
    */
    class _OgreExport SkeletonInstance : public Skeleton
    {
    public:
        /** Constructor, don't call directly, this will be created automatically
        when you create an Entity based on a skeletally animated Mesh.
        */
        SkeletonInstance(Skeleton* masterCopy);
        ~SkeletonInstance();

        /** Gets the number of animations on this skeleton. */
        unsigned short getNumAnimations(void) const;

        /** Gets a single animation by index. */
        Animation* getAnimation(unsigned short index) const;

        /** Creates a new Animation object for animating this skeleton. 
        @remarks
            This method updates the reference skeleton, not just this instance!
        @param name The name of this animation
        @param length The length of the animation in seconds
        */
        Animation* createAnimation(const String& name, Real length);

        /** Returns the named Animation object. */
        Animation* getAnimation(const String& name) const;

        /** Removes an Animation from this skeleton. 
        @remarks
            This method updates the reference skeleton, not just this instance!
        */
        void removeAnimation(const String& name);

        /** Overridden from Skeleton
        */
        void load(void);
        /** Overridden from Skeleton
        */
        void unload(void);

        /** Creates a TagPoint ready to be attached to a bone */
        TagPoint* createTagPointOnBone(Bone* bone, 
            const Quaternion &offsetOrientation = Quaternion::IDENTITY, 
            const Vector3	  &offsetPosition    = Vector3::UNIT_SCALE);


    protected:
        /// Pointer back to master Skeleton
        Skeleton* mSkeleton;
        /// Storage of tagPoints, lookup by handle
        typedef std::map<unsigned short, TagPoint*> TagPointList;
        TagPointList mTagPointList;
        /// TagPoint automatic handles
        unsigned short mNextTagPointAutoHandle;

        void cloneBoneAndChildren(Bone* source, Bone* parent);

    };

}


#endif


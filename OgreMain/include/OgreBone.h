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

#ifndef __Bone_H__
#define __Bone_H__

#include "OgrePrerequisites.h"
#include "OgreNode.h"


namespace Ogre 
{
    /** A bone in a skeleton.
    @remarks
        See Skeleton for more information about the principles behind skeletal animation.
        This class is a node in the joint hierarchy. Mesh vertices also have assignments
        to bones to define how they move in relation to the skeleton.
    */
    class _OgreExport Bone : public Node
    {
    public:
        /** Constructor, not to be used directly (use Bone::createChild or Skeleton::createBone) */
        Bone(unsigned short handle, Skeleton* creator);
        /** Constructor, not to be used directly (use Bone::createChild or Skeleton::createBone) */
        Bone(const String& name, unsigned short handle, Skeleton* creator);
        ~Bone();

        /** Creates a new Bone as a child of this bone.
        @remarks
            This method creates a new bone which will inherit the transforms of this
            bone. Note that the numeric handle of the new bone will be generated - 
            if you need to specify it then you should use the alternate form of this
            method which takes a handle as a parameter.
            @param
                translate Initial translation offset of child relative to parent
            @param
                rotate Initial rotation relative to parent
        */
        Bone* createChild(const Vector3& translate = Vector3::ZERO, const Quaternion& rotate = Quaternion::IDENTITY);

        /** Creates a new Bone as a child of this bone.
        @remarks
            This method creates a new bone which will inherit the transforms of this
            bone, with the handle specified.
            @param 
                handle The numeric handle to give the new bone; must be unique within the Skeleton.
            @param
                translate Initial translation offset of child relative to parent
            @param
                rotate Initial rotation relative to parent
        */
        Bone* createChild(unsigned short handle, 
            const Vector3& translate = Vector3::ZERO, const Quaternion& rotate = Quaternion::IDENTITY);
        /** Creates a new Bone as a child of this bone.
        @remarks
            This method creates a new bone which will inherit the transforms of this
            bone, and also gives it the name specified. This bone can then be looked up in 
            the Skeleton using it's name if required.
            @param 
                handle The numeric handle to give the new bone; must be unique within the Skeleton.
            @param
                translate Initial translation offset of child relative to parent
            @param
                rotate Initial rotation relative to parent
        */
        Bone* createChild(const String& name , 
            const Vector3& translate = Vector3::ZERO, const Quaternion& rotate = Quaternion::IDENTITY);

        /** Gets a pointer to a child node.*/
        Bone* getChild(unsigned short index) const;

        /** Drops the specified child from this node. Does not delete the node, just detaches it from
            this parent, potentially to be reattached elsewhere.
        */
        Bone* removeChild(unsigned short index);

        /** Gets the numeric handle for this bone (unique within the skeleton). */
        unsigned short getHandle(void) const;

        /** Sets the current position / orientation to be the 'binding pose' ie the layout in which 
            bones were originally bound to a mesh.
        */
        void setBindingPose(void);

        /** Resets the position and orientation of this Bone to the original binding position.
        @remarks
            Bones are bound to the mesh in a binding pose. They are then modified from this
            position during animation. This method returns the bone to it's original position and
            orientation.
        */
        void reset(void);

        
        /** Gets the inverse transform which takes bone space to origin from the binding pose. 
        @remarks
            Internal use only.
        */
        Matrix4 _getBindingPoseInverseTransform(void);

    protected:
        /// The numeric handle of this bone
        unsigned short mHandle;

        /** See Node. */
        Node* createChildImpl(void);
        /** See Node. */
        Node* createChildImpl(const String& name);

        /// Pointer back to creator, for child creation (central memory allocation)
        Skeleton* mCreator;

        /// Original orientation in binding pose
        Quaternion mBindOrientation;
        /// Original position in binding pose
        Vector3 mBindPosition;

        /// The inversed derived transform of the bone in the binding pose
        Matrix4 mBindDerivedInverseTransform;

    };


}

#endif

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 The OGRE Team
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
#ifndef __SkeletonFileFormat_H__
#define __SkeletonFileFormat_H__

#include "OgrePrerequisites.h"

namespace Ogre {

/** Definition of the OGRE .skeleton file format 

    .skeleton files are binary files (for read efficiency at runtime) and are arranged into chunks 
    of data, very like 3D Studio's format.
    A chunk always consists of:
        unsigned short CHUNK_ID        : one of the following chunk ids identifying the chunk
        unsigned long  LENGTH          : length of the chunk in bytes, including this header
        void*          DATA            : the data, which may contain other sub-chunks (various data types)
    
    A .skeleton file contains both the definition of the Skeleton object and the animations it contains. It
    contains only a single skeleton but can contain multiple animations.


*/
    enum SkeletonChunkID {
        SKELETON_HEADER            = 0x1000,
            // char* version           : Version number check
        SKELETON_BONE              = 0x2000,
        // Repeating section defining each bone in the system. 
        // Bones are assigned indexes automatically based on their order of declaration
        // starting with 0.

            // unsigned short handle            : handle of the bone, should be contiguous & start at 0
            // Vector3 position                 : position of this bone relative to parent 
            // Quaternion orientation           : orientation of this bone relative to parent 

        SKELETON_BONE_PARENT       = 0x3000,
        // Record of the parent of a single bone, used to build the node tree
        // Repeating section, listed in Bone Index order, one per Bone

            // unsigned short handle             : child bone
            // unsigned short parentHandle   : parent bone

        SKELETON_ANIMATION         = 0x4000,
        // A single animation for this skeleton

            // char* name                       : Name of the animation
            // Real length                      : Length of the animation in seconds

            SKELETON_ANIMATION_TRACK = 0x4100,
            // A single animation track (relates to a single bone)
            // Repeating section (within SKELETON_ANIMATION)
                
                // unsigned short boneIndex     : Index of bone to apply to

                SKELETON_ANIMATION_TRACK_KEYFRAME = 0x4110,
                // A single keyframe within the track
                // Repeating section

                    // Real time                    : The time position (seconds)
                    // Quaternion rotate            : Rotation to apply at this keyframe
                    // Vector3 translate            : Translation to apply at this keyframe
    };

} // namespace


#endif

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

#include "OgreSkeletonFileFormat.h"
#include "OgreSkeletonSerializer.h"
#include "OgreSkeleton.h"
#include "OgreAnimation.h"
#include "OgreBone.h"




namespace Ogre {
    /// Chunk overhead = ID + size
    const unsigned long CHUNK_OVERHEAD_SIZE = sizeof(unsigned short) + sizeof(unsigned long);
    //---------------------------------------------------------------------
    SkeletonSerializer::SkeletonSerializer()
    {
    }
    //---------------------------------------------------------------------
    SkeletonSerializer::~SkeletonSerializer()
    {
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::exportSkeleton(const Skeleton* pSkeleton, const String& filename)
    {
        mpfFile = fopen(filename, "wb");

        writeFileHeader();

        // Write main skeleton data
        writeSkeleton(pSkeleton);

        // Write all animations
        unsigned short numAnims = pSkeleton->getNumAnimations();
        for (unsigned short i = 0; i < numAnims; ++i)
        {
            Animation* pAnim = pSkeleton->getAnimation(i);
            writeAnimation(pAnim);
        }
        fclose(mpfFile);

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::importSkeleton(DataChunk& chunk, Skeleton* pDest)
    {
        //TODO
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeSkeleton(const Skeleton* pSkel)
    {
        // Write each bone
        unsigned short numBones = pSkel->getNumBones();
        for (unsigned short i = 0; i < numBones; ++i)
        {
            Bone* pBone = pSkel->getBone(i);
            writeBone(pBone);
        }
        // Write parents
        for (unsigned short i = 0; i < numBones; ++i)
        {
            Bone* pBone = pSkel->getBone(i);
            unsigned short handle = pBone->getHandle();
            if (handle != 0) // root bone
            {
                Bone* pParent = (Bone*)pBone->getParent();
                writeBoneParent(handle, pParent->getHandle());
            }
        }
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeBone(const Bone* pBone)
    {
        writeChunkHeader(SKELETON_BONE, calcBoneSize(pBone));

        unsigned short handle = pBone->getHandle();
        writeShorts(&handle, 1);
        writeObject(pBone->getPosition());
        writeObject(pBone->getOrientation());
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeBoneParent(unsigned short boneId, unsigned short parentId)
    {
        writeChunkHeader(SKELETON_BONE_PARENT, calcBoneParentSize());

        writeShorts(&boneId, 1);
        writeShorts(&parentId, 1);

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeAnimation(const Animation* anim)
    {
        // TODO
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeAnimationTrack(const AnimationTrack* track)
    {
        // TODO
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeKeyFrame(const KeyFrame* key)
    {
        // TODO
    }
    //---------------------------------------------------------------------
    unsigned long SkeletonSerializer::calcBoneSize(const Bone* pBone)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // handle
        size += sizeof(unsigned short);

        // position
        size += sizeof(Real) * 3;

        // orientation
        size += sizeof(Real) * 4;

        return size;

    }
    //---------------------------------------------------------------------
    unsigned long SkeletonSerializer::calcBoneParentSize(void)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // handle
        size += sizeof(unsigned short);

        // parent handle
        size += sizeof(unsigned short);

        return size;

    }
    //---------------------------------------------------------------------
    unsigned long SkeletonSerializer::calcAnimationSize(const Animation* pAnim)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // TODO

        return size;

    }
    //---------------------------------------------------------------------
    unsigned long SkeletonSerializer::calcAnimationTrackSize(const AnimationTrack* pTrack)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // TODO

        return size;
    }
    //---------------------------------------------------------------------
    unsigned long SkeletonSerializer::calcKeyFrameSize(const KeyFrame* pKey)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // TODO

        return size;
    }
    //---------------------------------------------------------------------



}



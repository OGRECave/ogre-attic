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
#include "OgreStableHeaders.h"

#include "OgreSkeletonFileFormat.h"
#include "OgreSkeletonSerializer.h"
#include "OgreSkeleton.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreKeyFrame.h"
#include "OgreBone.h"
#include "OgreString.h"
#include "OgreDataChunk.h"
#include "OgreLogManager.h"




namespace Ogre {
    /// Chunk overhead = ID + size
    const unsigned long CHUNK_OVERHEAD_SIZE = sizeof(unsigned short) + sizeof(unsigned long);
    //---------------------------------------------------------------------
    SkeletonSerializer::SkeletonSerializer()
    {
        // Version number
        // NB changed to include bone names in 1.1
        mVersion = "[Serializer_v1.10]";
    }
    //---------------------------------------------------------------------
    SkeletonSerializer::~SkeletonSerializer()
    {
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::exportSkeleton(const Skeleton* pSkeleton, const String& filename)
    {
        String msg;
        mpfFile = fopen(filename, "wb");

        writeFileHeader();

        // Write main skeleton data
        LogManager::getSingleton().logMessage("Exporting bones..");
        writeSkeleton(pSkeleton);
        LogManager::getSingleton().logMessage("Bones exported.");

        // Write all animations
        unsigned short numAnims = pSkeleton->getNumAnimations();
        msg = "Exporting animations, count=";
        msg << numAnims;
        LogManager::getSingleton().logMessage(msg);
        for (unsigned short i = 0; i < numAnims; ++i)
        {
            Animation* pAnim = pSkeleton->getAnimation(i);
            msg = "Exporting animation: ";
            msg << pAnim->getName();
            LogManager::getSingleton().logMessage(msg);
            writeAnimation(pAnim);
            LogManager::getSingleton().logMessage("Animation exported.");

        }
        fclose(mpfFile);

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::importSkeleton(DataChunk& chunk, Skeleton* pDest)
    {
        mpSkeleton = pDest;

        // Check header
        readFileHeader(chunk);

        unsigned short chunkID;
        while(!chunk.isEOF())
        {
            chunkID = readChunk(chunk);
            switch (chunkID)
            {
            case SKELETON_BONE:
                readBone(chunk);
                break;
            case SKELETON_BONE_PARENT:
                readBoneParent(chunk);
                break;
            case SKELETON_ANIMATION:
                readAnimation(chunk);
            }
        }

        // Assume bones are stored in the binding pose
        mpSkeleton->setBindingPose();


    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeSkeleton(const Skeleton* pSkel)
    {
        // Write each bone
        unsigned short numBones = pSkel->getNumBones();
        unsigned short i;
        for (i = 0; i < numBones; ++i)
        {
            Bone* pBone = pSkel->getBone(i);
            writeBone(pBone);
        }
        // Write parents
        for (i = 0; i < numBones; ++i)
        {
            Bone* pBone = pSkel->getBone(i);
            unsigned short handle = pBone->getHandle();
            Bone* pParent = (Bone*)pBone->getParent(); 
            if (pParent != NULL) 
            {
                writeBoneParent(handle, pParent->getHandle());             
            }
        }
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeBone(const Bone* pBone)
    {
        writeChunkHeader(SKELETON_BONE, calcBoneSize(pBone));

        unsigned short handle = pBone->getHandle();
        // char* name
        writeString(pBone->getName());
        // unsigned short handle            : handle of the bone, should be contiguous & start at 0
        writeShorts(&handle, 1);
        // Vector3 position                 : position of this bone relative to parent 
        writeObject(pBone->getPosition());
        // Quaternion orientation           : orientation of this bone relative to parent 
        writeObject(pBone->getOrientation());
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeBoneParent(unsigned short boneId, unsigned short parentId)
    {
        writeChunkHeader(SKELETON_BONE_PARENT, calcBoneParentSize());

        // unsigned short handle             : child bone
        writeShorts(&boneId, 1);
        // unsigned short parentHandle   : parent bone
        writeShorts(&parentId, 1);

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeAnimation(const Animation* anim)
    {
        writeChunkHeader(SKELETON_ANIMATION, calcAnimationSize(anim));

        // char* name                       : Name of the animation
        writeString(anim->getName());
        // Real length                      : Length of the animation in seconds
        Real len = anim->getLength();
        writeReals(&len, 1);

        // Write all tracks
        for (unsigned short i = 0; i < anim->getNumTracks(); ++i)
        {
            writeAnimationTrack(anim->getTrack(i));
        }

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeAnimationTrack(const AnimationTrack* track)
    {
        writeChunkHeader(SKELETON_ANIMATION_TRACK, calcAnimationTrackSize(track));

        // unsigned short boneIndex     : Index of bone to apply to
        Bone* bone = (Bone*)track->getAssociatedNode();
        unsigned short boneid = bone->getHandle();
        writeShorts(&boneid, 1);

        // Write all keyframes
        for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
        {
            writeKeyFrame(track->getKeyFrame(i));
        }

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeKeyFrame(const KeyFrame* key)
    {

        writeChunkHeader(SKELETON_ANIMATION_TRACK_KEYFRAME, calcKeyFrameSize(key));

        // Real time                    : The time position (seconds)
        Real time = key->getTime();
        writeReals(&time, 1);
        // Quaternion rotate            : Rotation to apply at this keyframe
        writeObject(key->getRotation());
        // Vector3 translate            : Translation to apply at this keyframe
        writeObject(key->getTranslate());
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

        // Name, including terminator
        size += (unsigned long)pAnim->getName().length() + 1;
        // length
        size += sizeof(Real);

        // Nested animation tracks
        for (unsigned short i = 0; i < pAnim->getNumTracks(); ++i)
        {
            size += calcAnimationTrackSize(pAnim->getTrack(i));
        }


        return size;

    }
    //---------------------------------------------------------------------
    unsigned long SkeletonSerializer::calcAnimationTrackSize(const AnimationTrack* pTrack)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // unsigned short boneIndex     : Index of bone to apply to
        size += sizeof(unsigned short);

        // Nested keyframes
        for (unsigned short i = 0; i < pTrack->getNumKeyFrames(); ++i)
        {
            size += calcKeyFrameSize(pTrack->getKeyFrame(i));
        }


        return size;
    }
    //---------------------------------------------------------------------
    unsigned long SkeletonSerializer::calcKeyFrameSize(const KeyFrame* pKey)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // Real time                    : The time position (seconds)
        size += sizeof(Real);
        // Quaternion rotate            : Rotation to apply at this keyframe
        size += sizeof(Real) * 4;
        // Vector3 translate            : Translation to apply at this keyframe
        size += sizeof(Real) * 3;

        return size;
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readBone(DataChunk &chunk)
    {
        // char* name
        String name = readString(chunk);
        // unsigned short handle            : handle of the bone, should be contiguous & start at 0
        unsigned short handle;
        readShorts(chunk, &handle, 1);

        // Create new bone
        Bone* pBone = mpSkeleton->createBone(name, handle);

        // Vector3 position                 : position of this bone relative to parent 
        Vector3 pos;
        readObject(chunk, &pos);
        pBone->setPosition(pos);
        // Quaternion orientation           : orientation of this bone relative to parent 
        Quaternion q;
        readObject(chunk, &q);
        pBone->setOrientation(q);
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readBoneParent(DataChunk &chunk)
    {
        // All bones have been created by this point
        Bone *child, *parent;
        unsigned short childHandle, parentHandle;

        // unsigned short handle             : child bone
        readShorts(chunk, &childHandle, 1);
        // unsigned short parentHandle   : parent bone
        readShorts(chunk, &parentHandle, 1);

        // Find bones
        parent = mpSkeleton->getBone(parentHandle);
        child = mpSkeleton->getBone(childHandle);

        // attach
        parent->addChild(child);

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readAnimation(DataChunk &chunk)
    {
        // char* name                       : Name of the animation
        String name;
        name = readString(chunk);
        // Real length                      : Length of the animation in seconds
        Real len;
        readReals(chunk, &len, 1);

        Animation *pAnim = mpSkeleton->createAnimation(name, len);

        // Read all tracks
        if (!chunk.isEOF())
        {
            unsigned short chunkID = readChunk(chunk);
            while(chunkID == SKELETON_ANIMATION_TRACK && !chunk.isEOF())
            {
                readAnimationTrack(chunk, pAnim);

                if (!chunk.isEOF())
                {
                    // Get next chunk
                    chunkID = readChunk(chunk);
                }
            }
            if (!chunk.isEOF())
            {
                // Backpedal back to start of this chunk if we've found a non-track
                chunk.skip(-(long)CHUNK_OVERHEAD_SIZE);
            }

        }



    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readAnimationTrack(DataChunk &chunk, Animation* anim)
    {
        // unsigned short boneIndex     : Index of bone to apply to
        unsigned short boneHandle;
        readShorts(chunk, &boneHandle, 1);

        // Find bone
        Bone *targetBone = mpSkeleton->getBone(boneHandle);

        // Create track
        AnimationTrack* pTrack = anim->createTrack(boneHandle, targetBone);

        // Keep looking for nested keyframes
        if (!chunk.isEOF())
        {
            unsigned short chunkID = readChunk(chunk);
            while(chunkID == SKELETON_ANIMATION_TRACK_KEYFRAME && !chunk.isEOF())
            {
                readKeyFrame(chunk, pTrack);

                if (!chunk.isEOF())
                {
                    // Get next chunk
                    chunkID = readChunk(chunk);
                }
            }
            if (!chunk.isEOF())
            {
                // Backpedal back to start of this chunk if we've found a non-keyframe
                chunk.skip(-(long)CHUNK_OVERHEAD_SIZE);
            }

        }


    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readKeyFrame(DataChunk &chunk, AnimationTrack* track)
    {
        // Real time                    : The time position (seconds)
        Real time;
        readReals(chunk, &time, 1);

        KeyFrame *kf = track->createKeyFrame(time);

        // Quaternion rotate            : Rotation to apply at this keyframe
        Quaternion rot;
        readObject(chunk, &rot);
        kf->setRotation(rot);
        // Vector3 translate            : Translation to apply at this keyframe
        Vector3 trans;
        readObject(chunk, &trans);
        kf->setTranslate(trans);
    }
    //---------------------------------------------------------------------



}



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
#include "OgreDataStream.h"
#include "OgreLogManager.h"




namespace Ogre {
    /// stream overhead = ID + size
    const unsigned long stream_OVERHEAD_SIZE = sizeof(unsigned short) + sizeof(unsigned long);
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
    void SkeletonSerializer::exportSkeleton(const SkeletonPtr& pSkeleton, const String& filename)
    {
        String msg;
        mpfFile = fopen(filename.c_str(), "wb");

        writeFileHeader();

        // Write main skeleton data
        LogManager::getSingleton().logMessage("Exporting bones..");
        writeSkeleton(pSkeleton);
        LogManager::getSingleton().logMessage("Bones exported.");

        // Write all animations
        unsigned short numAnims = pSkeleton->getNumAnimations();
        msg = "Exporting animations, count=";
		StringUtil::StrStreamType num;
		num << numAnims;
        msg += num.str();
        LogManager::getSingleton().logMessage(msg);
        for (unsigned short i = 0; i < numAnims; ++i)
        {
            Animation* pAnim = pSkeleton->getAnimation(i);
            msg = "Exporting animation: " + pAnim->getName();
            LogManager::getSingleton().logMessage(msg);
            writeAnimation(pSkeleton, pAnim);
            LogManager::getSingleton().logMessage("Animation exported.");

        }
        fclose(mpfFile);

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::importSkeleton(DataStream& stream, SkeletonPtr& pSkel)
    {

        // Check header
        readFileHeader(stream);

        unsigned short streamID;
        while(!stream.eof())
        {
            streamID = readChunk(stream);
            switch (streamID)
            {
            case SKELETON_BONE:
                readBone(stream, pSkel);
                break;
            case SKELETON_BONE_PARENT:
                readBoneParent(stream, pSkel);
                break;
            case SKELETON_ANIMATION:
                readAnimation(stream, pSkel);
            }
        }

        // Assume bones are stored in the binding pose
        pSkel->setBindingPose();


    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeSkeleton(const SkeletonPtr& pSkel)
    {
        // Write each bone
        unsigned short numBones = pSkel->getNumBones();
        unsigned short i;
        for (i = 0; i < numBones; ++i)
        {
            Bone* pBone = pSkel->getBone(i);
            writeBone(pSkel, pBone);
        }
        // Write parents
        for (i = 0; i < numBones; ++i)
        {
            Bone* pBone = pSkel->getBone(i);
            unsigned short handle = pBone->getHandle();
            Bone* pParent = (Bone*)pBone->getParent(); 
            if (pParent != NULL) 
            {
                writeBoneParent(pSkel, handle, pParent->getHandle());             
            }
        }
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeBone(const SkeletonPtr& pSkel, const Bone* pBone)
    {
        writeChunkHeader(SKELETON_BONE, calcBoneSize(pSkel, pBone));

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
    void SkeletonSerializer::writeBoneParent(const SkeletonPtr& pSkel, 
        unsigned short boneId, unsigned short parentId)
    {
        writeChunkHeader(SKELETON_BONE_PARENT, calcBoneParentSize(pSkel));

        // unsigned short handle             : child bone
        writeShorts(&boneId, 1);
        // unsigned short parentHandle   : parent bone
        writeShorts(&parentId, 1);

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeAnimation(const SkeletonPtr& pSkel, 
        const Animation* anim)
    {
        writeChunkHeader(SKELETON_ANIMATION, calcAnimationSize(pSkel, anim));

        // char* name                       : Name of the animation
        writeString(anim->getName());
        // Real length                      : Length of the animation in seconds
        Real len = anim->getLength();
        writeReals(&len, 1);

        // Write all tracks
        Animation::TrackIterator trackIt = anim->getTrackIterator();
        while(trackIt.hasMoreElements())
        {
            writeAnimationTrack(pSkel, trackIt.getNext());
        }

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeAnimationTrack(const SkeletonPtr& pSkel, 
        const AnimationTrack* track)
    {
        writeChunkHeader(SKELETON_ANIMATION_TRACK, calcAnimationTrackSize(pSkel, track));

        // unsigned short boneIndex     : Index of bone to apply to
        Bone* bone = (Bone*)track->getAssociatedNode();
        unsigned short boneid = bone->getHandle();
        writeShorts(&boneid, 1);

        // Write all keyframes
        for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
        {
            writeKeyFrame(pSkel, track->getKeyFrame(i));
        }

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::writeKeyFrame(const SkeletonPtr& pSkel, 
        const KeyFrame* key)
    {

        writeChunkHeader(SKELETON_ANIMATION_TRACK_KEYFRAME, 
            calcKeyFrameSize(pSkel, key));

        // Real time                    : The time position (seconds)
        Real time = key->getTime();
        writeReals(&time, 1);
        // Quaternion rotate            : Rotation to apply at this keyframe
        writeObject(key->getRotation());
        // Vector3 translate            : Translation to apply at this keyframe
        writeObject(key->getTranslate());
        // Vector3 scale                : Scale to apply at this keyframe
        writeObject(key->getScale());
    }
    //---------------------------------------------------------------------
    unsigned long SkeletonSerializer::calcBoneSize(const SkeletonPtr& pSkel, 
        const Bone* pBone)
    {
        unsigned long size = stream_OVERHEAD_SIZE;

        // handle
        size += sizeof(unsigned short);

        // position
        size += sizeof(Real) * 3;

        // orientation
        size += sizeof(Real) * 4;

        return size;

    }
    //---------------------------------------------------------------------
    unsigned long SkeletonSerializer::calcBoneParentSize(const SkeletonPtr& pSkel)
    {
        unsigned long size = stream_OVERHEAD_SIZE;

        // handle
        size += sizeof(unsigned short);

        // parent handle
        size += sizeof(unsigned short);

        return size;

    }
    //---------------------------------------------------------------------
    unsigned long SkeletonSerializer::calcAnimationSize(const SkeletonPtr& pSkel, 
        const Animation* pAnim)
    {
        unsigned long size = stream_OVERHEAD_SIZE;

        // Name, including terminator
        size += (unsigned long)pAnim->getName().length() + 1;
        // length
        size += sizeof(Real);

        // Nested animation tracks
        for (unsigned short i = 0; i < pAnim->getNumTracks(); ++i)
        {
            size += calcAnimationTrackSize(pSkel, pAnim->getTrack(i));
        }


        return size;

    }
    //---------------------------------------------------------------------
    unsigned long SkeletonSerializer::calcAnimationTrackSize(const SkeletonPtr& pSkel, 
        const AnimationTrack* pTrack)
    {
        unsigned long size = stream_OVERHEAD_SIZE;

        // unsigned short boneIndex     : Index of bone to apply to
        size += sizeof(unsigned short);

        // Nested keyframes
        for (unsigned short i = 0; i < pTrack->getNumKeyFrames(); ++i)
        {
            size += calcKeyFrameSize(pSkel, pTrack->getKeyFrame(i));
        }


        return size;
    }
    //---------------------------------------------------------------------
    unsigned long SkeletonSerializer::calcKeyFrameSize(const SkeletonPtr& pSkel, 
        const KeyFrame* pKey)
    {
        unsigned long size = stream_OVERHEAD_SIZE;

        // Real time                    : The time position (seconds)
        size += sizeof(Real);
        // Quaternion rotate            : Rotation to apply at this keyframe
        size += sizeof(Real) * 4;
        // Vector3 translate            : Translation to apply at this keyframe
        size += sizeof(Real) * 3;
        // Vector3 scale                : Scale to apply at this keyframe
        size += sizeof(Real) * 3;

        return size;
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readBone(DataStream &stream, SkeletonPtr& pSkel)
    {
        // char* name
        String name = readString(stream);
        // unsigned short handle            : handle of the bone, should be contiguous & start at 0
        unsigned short handle;
        readShorts(stream, &handle, 1);

        // Create new bone
        Bone* pBone = pSkel->createBone(name, handle);

        // Vector3 position                 : position of this bone relative to parent 
        Vector3 pos;
        readObject(stream, &pos);
        pBone->setPosition(pos);
        // Quaternion orientation           : orientation of this bone relative to parent 
        Quaternion q;
        readObject(stream, &q);
        pBone->setOrientation(q);
    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readBoneParent(DataStream &stream, SkeletonPtr& pSkel)
    {
        // All bones have been created by this point
        Bone *child, *parent;
        unsigned short childHandle, parentHandle;

        // unsigned short handle             : child bone
        readShorts(stream, &childHandle, 1);
        // unsigned short parentHandle   : parent bone
        readShorts(stream, &parentHandle, 1);

        // Find bones
        parent = pSkel->getBone(parentHandle);
        child = pSkel->getBone(childHandle);

        // attach
        parent->addChild(child);

    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readAnimation(DataStream &stream, SkeletonPtr& pSkel)
    {
        // char* name                       : Name of the animation
        String name;
        name = readString(stream);
        // Real length                      : Length of the animation in seconds
        Real len;
        readReals(stream, &len, 1);

        Animation *pAnim = pSkel->createAnimation(name, len);

        // Read all tracks
        if (!stream.eof())
        {
            unsigned short streamID = readChunk(stream);
            while(streamID == SKELETON_ANIMATION_TRACK && !stream.eof())
            {
                readAnimationTrack(stream, pAnim, pSkel);

                if (!stream.eof())
                {
                    // Get next stream
                    streamID = readChunk(stream);
                }
            }
            if (!stream.eof())
            {
                // Backpedal back to start of this stream if we've found a non-track
                stream.skip(-(long)stream_OVERHEAD_SIZE);
            }

        }



    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readAnimationTrack(DataStream &stream, Animation* anim, 
        SkeletonPtr& pSkel)
    {
        // unsigned short boneIndex     : Index of bone to apply to
        unsigned short boneHandle;
        readShorts(stream, &boneHandle, 1);

        // Find bone
        Bone *targetBone = pSkel->getBone(boneHandle);

        // Create track
        AnimationTrack* pTrack = anim->createTrack(boneHandle, targetBone);

        // Keep looking for nested keyframes
        if (!stream.eof())
        {
            unsigned short streamID = readChunk(stream);
            while(streamID == SKELETON_ANIMATION_TRACK_KEYFRAME && !stream.eof())
            {
                readKeyFrame(stream, pTrack, pSkel);

                if (!stream.eof())
                {
                    // Get next stream
                    streamID = readChunk(stream);
                }
            }
            if (!stream.eof())
            {
                // Backpedal back to start of this stream if we've found a non-keyframe
                stream.skip(-(long)stream_OVERHEAD_SIZE);
            }

        }


    }
    //---------------------------------------------------------------------
    void SkeletonSerializer::readKeyFrame(DataStream &stream, AnimationTrack* track, 
        SkeletonPtr& pSkel)
    {
        // Real time                    : The time position (seconds)
        Real time;
        readReals(stream, &time, 1);

        KeyFrame *kf = track->createKeyFrame(time);

        // Quaternion rotate            : Rotation to apply at this keyframe
        Quaternion rot;
        readObject(stream, &rot);
        kf->setRotation(rot);
        // Vector3 translate            : Translation to apply at this keyframe
        Vector3 trans;
        readObject(stream, &trans);
        kf->setTranslate(trans);
        // Do we have scale?
        if (mCurrentstreamLen == calcKeyFrameSize(pSkel, kf))
        {
            Vector3 scale;
            readObject(stream, &scale);
            kf->setScale(scale);
        }
    }
    //---------------------------------------------------------------------



}



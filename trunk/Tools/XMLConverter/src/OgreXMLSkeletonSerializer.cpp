/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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

#include "OgreXMLSkeletonSerializer.h"
#include "OgreSkeleton.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreKeyFrame.h"
#include "OgreBone.h"
#include "OgreString.h"
#include "OgreDataChunk.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"


namespace Ogre {

    
    //---------------------------------------------------------------------
    XMLSkeletonSerializer::XMLSkeletonSerializer()
    {
    }
    //---------------------------------------------------------------------
    XMLSkeletonSerializer::~XMLSkeletonSerializer()
    {
    }
    //---------------------------------------------------------------------
    Skeleton* XMLSkeletonSerializer::importSkeleton(const String& filename, 
        Skeleton* pSkeleton)
    {
        // TODO
        return NULL;
    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::exportSkeleton(const Skeleton* pSkeleton, 
        const String& filename)
    {
        LogManager::getSingleton().logMessage("XMLSkeletonSerializer writing "
            " skeleton data to " + filename + "...");

        mXMLDoc = new TiXmlDocument();
        mXMLDoc->InsertEndChild(TiXmlElement("skeleton"));
        TiXmlElement* rootNode = mXMLDoc->RootElement();

        LogManager::getSingleton().logMessage("Populating DOM...");


        // Write main skeleton data
        LogManager::getSingleton().logMessage("Exporting bones..");
        writeSkeleton(pSkeleton);
        LogManager::getSingleton().logMessage("Bones exported.");

        // Write all animations
        unsigned short numAnims = pSkeleton->getNumAnimations();
        String msg = "Exporting animations, count=";
        msg << numAnims;
        LogManager::getSingleton().logMessage(msg);

        TiXmlElement* animsNode = 
            rootNode->InsertEndChild(TiXmlElement("animations"))->ToElement();
        
        for (unsigned short i = 0; i < numAnims; ++i)
        {
            Animation* pAnim = pSkeleton->getAnimation(i);
            msg = "Exporting animation: ";
            msg << pAnim->getName();
            LogManager::getSingleton().logMessage(msg);
            writeAnimation(animsNode, pAnim);
            LogManager::getSingleton().logMessage("Animation exported.");

        }

        LogManager::getSingleton().logMessage("DOM populated, writing XML file..");

        // Write out to a file
        mXMLDoc->SaveFile(filename);

    
        delete mXMLDoc;

        LogManager::getSingleton().logMessage("XMLSkeletonSerializer export successful.");

    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeSkeleton(const Skeleton* pSkel)
    {
        TiXmlElement* rootNode = mXMLDoc->RootElement();

        TiXmlElement* bonesElem = 
            rootNode->InsertEndChild(TiXmlElement("bones"))->ToElement();

        unsigned short numBones = pSkel->getNumBones();
        unsigned short i;
        for (i = 0; i < numBones; ++i)
        {
            Bone* pBone = pSkel->getBone(i);
            writeBone(bonesElem, pBone);
        }

        // Write parents
        TiXmlElement* hierElem = 
            rootNode->InsertEndChild(TiXmlElement("bonehierarchy"))->ToElement();
        for (i = 0; i < numBones; ++i)
        {
            Bone* pBone = pSkel->getBone(i);
            unsigned short handle = pBone->getHandle();
            if (handle != 0) // root bone
            {
                Bone* pParent = (Bone*)pBone->getParent();
                writeBoneParent(hierElem, handle, pParent->getHandle());
            }
        }


    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeBone(TiXmlElement* bonesElement, const Bone* pBone)
    {
        TiXmlElement* boneElem = 
            bonesElement->InsertEndChild(TiXmlElement("bone"))->ToElement();

        
        // Bone name & handle
        boneElem->SetAttribute("id", 
            StringConverter::toString(pBone->getHandle()));
        boneElem->SetAttribute("name", pBone->getName());

        // Position
        TiXmlElement* subNode = 
            boneElem->InsertEndChild(TiXmlElement("position"))->ToElement();
        Vector3 pos = pBone->getPosition();
        subNode->SetAttribute("x", StringConverter::toString(pos.x));
        subNode->SetAttribute("y", StringConverter::toString(pos.y));
        subNode->SetAttribute("z", StringConverter::toString(pos.z));
        
        // Orientation 
        subNode = 
            boneElem->InsertEndChild(TiXmlElement("rotation"))->ToElement();
        // Show Quaternion as angle / axis
        Real angle;
        Vector3 axis;
        pBone->getOrientation().ToAngleAxis(angle, axis);
        TiXmlElement* axisNode = 
            subNode->InsertEndChild(TiXmlElement("axis"))->ToElement();
        subNode->SetAttribute("angle", StringConverter::toString(angle));
        axisNode->SetAttribute("x", StringConverter::toString(axis.x));
        axisNode->SetAttribute("y", StringConverter::toString(axis.y));
        axisNode->SetAttribute("z", StringConverter::toString(axis.z));



    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeBoneParent(TiXmlElement* boneHierarchyNode, 
        unsigned short boneId, unsigned short parentId)
    {
        TiXmlElement* boneParentNode = 
            boneHierarchyNode->InsertEndChild(TiXmlElement("boneparent"))->ToElement();

        boneParentNode->SetAttribute("boneid", StringConverter::toString(boneId));
        boneParentNode->SetAttribute("parentid", StringConverter::toString(parentId));
    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeAnimation(TiXmlElement* animsNode, 
        const Animation* anim)
    {
        TiXmlElement* animNode = 
            animsNode->InsertEndChild(TiXmlElement("animation"))->ToElement();

        animNode->SetAttribute("name", anim->getName());
        animNode->SetAttribute("length", StringConverter::toString(anim->getLength()));

        // Write all tracks
        TiXmlElement* tracksNode = 
            animNode->InsertEndChild(TiXmlElement("tracks"))->ToElement();
        for (unsigned short i = 0; i < anim->getNumTracks(); ++i)
        {
            writeAnimationTrack(tracksNode, anim->getTrack(i));
        }

    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeAnimationTrack(TiXmlElement* tracksNode, 
        const AnimationTrack* track)
    {
        TiXmlElement* trackNode = 
            tracksNode->InsertEndChild(TiXmlElement("track"))->ToElement();
        
        
        // unsigned short boneIndex     : Index of bone to apply to
        Bone* bone = (Bone*)track->getAssociatedNode();
        unsigned short boneid = bone->getHandle();
        trackNode->SetAttribute("boneid", StringConverter::toString(boneid));

        // Write all keyframes
        TiXmlElement* keysNode = 
            trackNode->InsertEndChild(TiXmlElement("keyframes"))->ToElement();
        for (unsigned short i = 0; i < track->getNumKeyFrames(); ++i)
        {
            writeKeyFrame(keysNode, track->getKeyFrame(i));
        }
    }
    //---------------------------------------------------------------------
    void XMLSkeletonSerializer::writeKeyFrame(TiXmlElement* keysNode, const KeyFrame* key)
    {
        TiXmlElement* keyNode = 
            keysNode->InsertEndChild(TiXmlElement("keyframe"))->ToElement();

        keyNode->SetAttribute("time", StringConverter::toString(key->getTime()));

        TiXmlElement* transNode = 
            keyNode->InsertEndChild(TiXmlElement("translate"))->ToElement();
        Vector3 trans = key->getTranslate();
        transNode->SetAttribute("x", StringConverter::toString(trans.x));
        transNode->SetAttribute("y", StringConverter::toString(trans.y));
        transNode->SetAttribute("z", StringConverter::toString(trans.z));

        TiXmlElement* rotNode = 
            keyNode->InsertEndChild(TiXmlElement("rotate"))->ToElement();
        // Show Quaternion as angle / axis
        Real angle;
        Vector3 axis;
        key->getRotation().ToAngleAxis(angle, axis);
        TiXmlElement* axisNode = 
            rotNode->InsertEndChild(TiXmlElement("axis"))->ToElement();
        rotNode->SetAttribute("angle", StringConverter::toString(angle));
        axisNode->SetAttribute("x", StringConverter::toString(axis.x));
        axisNode->SetAttribute("y", StringConverter::toString(axis.y));
        axisNode->SetAttribute("z", StringConverter::toString(axis.z));

        

    }
    //---------------------------------------------------------------------
}



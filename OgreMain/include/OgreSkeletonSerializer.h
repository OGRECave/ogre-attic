/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2002 The OGRE Team
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

#ifndef __SkeletonSerializer_H__
#define __SkeletonSerializer_H__

#include "OgrePrerequisites.h"
#include "OgreSkeleton.h"
#include "OgreSerializer.h"

namespace Ogre {

    /** Class for serialising skeleton data to/from an OGRE .skeleton file.
    @remarks
        This class allows exporters to write OGRE .skeleton files easily, and allows the
        OGRE engine to import .skeleton files into instatiated OGRE Skeleton objects.
        Note that a .skeleton file includes not only the Skeleton, but also definitions of
        any Animations it uses.
    @par
        To export a Skeleton:<OL>
        <LI>Create a Skeleton object and populate it using it's methods.</LI>
        <LI>Call the exportSkeleton method</LI>
        </OL>
    */
    class _OgreExport SkeletonSerializer : public Serializer
    {
    public:
        SkeletonSerializer();
        virtual ~SkeletonSerializer();


        /** Exports a skeleton to the file specified. 
        @remarks
            This method takes an externally created Skeleton object, and exports both it
            and animations it uses to a .skeleton file.
        @param pSkeleton Weak reference to the Skeleton to export
        @param filename The destination filename
        */
        void exportSkeleton(const Skeleton* pSkeleton, const String& filename);

        /** Imports Skeleton and animation data from a .skeleton file DataStream.
        @remarks
            This method imports data from a DataStream opened from a .skeleton file and places it's
            contents into the Skeleton object which is passed in. 
        @param stream The DataStream holding the .skeleton data. Must be initialised (pos at the start of the buffer).
        @param pDest Weak reference to the Skeleton object which will receive the data. Should be blank already.
        */
        void importSkeleton(DataStreamPtr& stream, Skeleton* pDest);

        // TODO: provide Cal3D importer?

    private:
        // Internal export methods
        void writeSkeleton(const Skeleton* pSkel);
        void writeBone(const Skeleton* pSkel, const Bone* pBone);
        void writeBoneParent(const Skeleton* pSkel, unsigned short boneId, unsigned short parentId);
        void writeAnimation(const Skeleton* pSkel, const Animation* anim);
        void writeAnimationTrack(const Skeleton* pSkel, const AnimationTrack* track);
        void writeKeyFrame(const Skeleton* pSkel, const KeyFrame* key);

        // Internal import methods
        void readBone(DataStreamPtr& stream, Skeleton* pSkel);
        void readBoneParent(DataStreamPtr& stream, Skeleton* pSkel);
        void readAnimation(DataStreamPtr& stream, Skeleton* pSkel);
        void readAnimationTrack(DataStreamPtr& stream, Animation* anim, Skeleton* pSkel);
        void readKeyFrame(DataStreamPtr& stream, AnimationTrack* track, Skeleton* pSkel);

        unsigned long calcBoneSize(const Skeleton* pSkel, const Bone* pBone);
        unsigned long calcBoneParentSize(const Skeleton* pSkel);
        unsigned long calcAnimationSize(const Skeleton* pSkel, const Animation* pAnim);
        unsigned long calcAnimationTrackSize(const Skeleton* pSkel, const AnimationTrack* pTrack);
        unsigned long calcKeyFrameSize(const Skeleton* pSkel, const KeyFrame* pKey);




    };

}


#endif

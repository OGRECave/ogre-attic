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

#ifndef __SkeletonSerializer_H__
#define __SkeletonSerializer_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreSkeleton.h"

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
    class _OgreExport SkeletonSerializer
    {
    public:
        SkeletonSerializer();
        virtual ~SkeletonSerializer();


        /** Exports a skeleton to the file specified. 
        @remarks
            This method takes an externally created Skeleton object, and exports both it
            and animations it uses to a .skeleton file.
        @param pSkeleton Pointer to the Skeleton to export
        @param filename The destination filename
        */
        void exportSkeleton(const Skeleton* pSkeleton, const String& filename);

        /** Imports Skeleton and animation data from a .skeleton file DataChunk.
        @remarks
            This method imports data from a DataChunk opened from a .skeleton file and places it's
            contents into the Skeleton object which is passed in. 
        @param chunk The DataChunk holding the .skeleton data. Must be initialised (pos at the start of the buffer).
        @param pDest Pointer to the Skeleton object which will receive the data. Should be blank already.
        */
        void importSkeleton(DataChunk& chunk, Skeleton* pDest);

        // TODO: provide Cal3D importer?

    private:
        Skeleton* mpSkeleton;

        // Internal methods


    };

}


#endif

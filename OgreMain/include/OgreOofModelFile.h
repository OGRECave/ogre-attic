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
#ifndef __Oof_model_H
#define __Oof_model_H

#include "OgrePrerequisites.h"

#include "OgreColourValue.h"
#include "OgreMaterial.h"
#include "OgreGeometryData.h"

namespace Ogre {

    /** Class to load an Ogre Object File (oof).
        @remarks
            <b>DEPRECATED in favour of MeshSerializer</b>
        @par
            This class loads the .oof files created by 3ds2oof (see
            readme.txt in the 3ds2oof folder for more info).
            For now these files use only a subset of the vertex formats
            that OGRE can handle, assuming only 1 set of 2D (u,v) texture
            coordinates (if present). The .oof format and this loader
            will develop to add more features in due course.
    */
    class OofModelFile
    {
    public:
        /** Indicates if this class should deallocate the memory it
            allocates for data when it is destroyed or more data is loaded.
            @remarks
                This parameter defaults to true (prevents memory leaks), however
                if another class wants to use the memory allocated by this object
                (without copying it out) it can set this member to false to prevent
                the memory being destroyed. However the caller must then ensure this
                memory is deallocated at the appropriate time, meaning ALL memory
                allocated by this object including geometry for each material.
        */
        bool autoDeallocateMemory;
        /// Filename of latest model loaded.
        String filename;

        /** Geometry shared among sub-parts of the model.
            The geometry of the model is split into chunks using
            the same material - these can either share this central
            geometry data or use separate buffers. See the readme.txt
            for the 3ds2oof utility for more info.
        */
        GeometryData sharedGeometry;

        /** Struct containing model material data. */
        struct MaterialData {
            /// The actual material used.
            Material material;
            /** If true, the faces for this material use shared vertex data.
                Otherwise, vertex data sed only by this material is contained
                in the GeometryData inside this structure.
            */
            bool useSharedVertices;
            /** This structure contains the dedicated geometry data for
                the faces using this material, if shared vertex data is not used.
            */
            GeometryData materialGeometry;

            /// The number of faces using this material.
            unsigned short numFaces;
            /// Pointer to a list of indices into the vertex data which form the faces using this material.
            unsigned short* pIndexes;
        };

        /** List of materials used by this model.
            @warning
                These are NOT automatically added to the SceneManager.
                Call registerMaterials() to perform this task.
        */
        std::vector<MaterialData> materials;


        OofModelFile();
        ~OofModelFile();

        /** Loads the named oof file.
        */
        void load(String filename);
        /** Loads oof data from a chunk of memory, which has been read from a file or archive.
        */
        void load(DataChunk& chunk);

        /** Destroys all the data held in the object and frees up the memory.
        */
        void freeMemory(void);
    };
}


#endif

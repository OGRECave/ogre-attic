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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/
#ifndef __OOF_H__
#define __OOF_H__

#include "OgrePrerequisites.h"

namespace Ogre {

/** Definition of the Ogre Object Format (.oof) file format.
    @remarks
        <b>This format is now DEPRECATED.</b>The .mesh format replaces it.
    @par
        OOF files are arranged into chunks of data, very like 3D Studio's format.
        A chunk always consists of:
            unsigned short CHUNK_ID        : one of the following chunk ids identifying the chunk
            unsigned short DATA_LENGTH    : length of the contained data in bytes
            void*           DATA            : the data, which may contain other sub-chunks (various data types)
*/
    enum OofChunkIDs {
        OOF_HEADER                = 0x1000,
            // ENTIRE HEADER ALWAYS UNCOMPRESSED
            // bool compression
            // unsigned short numMaterials
            // unsigned short numObjects
        OOF_MATERIAL            = 0x2000,
            // char* name (\n terminated)
            // AMBIENT
            // float r, g, b
            // DIFFUSE
            // float r, g, b
            // SPECULAR
            // float r, g, b
            // SHININESS
            // float val;
            OOF_TEXTURE_LAYER    = 0x2200, // optional, repeat per layer
                // char* name (\n terminated)
                // TODO - scale, offset, effects
        OOF_OBJECT                = 0x3000,
            // char* name (\n terminated)
            // unsigned short numSharedVertices
            OOF_VSHAREDPOSITIONS        = 0x3100, // present only if numSharedVertices > 0
                // float* (x,y,z) * numVertices
            OOF_VSHAREDNORMALS        = 0x3200, // optional, present only if numSharedVertices > 0
                // float* (x,y,z) * numVertices
            OOF_VSHAREDTEXCOORDS        = 0x3300, // optional, present only if numSharedVertices > 0
                // float* (u,v) * numVertices
            OOF_VSHAREDCOLOURS        = 0x3400, // optional, present only if numSharedVertices > 0
                // float* (r,g,b) * numVertices
            OOF_MATERIAL_GROUP    = 0x3500, // optional, present only if numSharedVertices > 0
                // unsigned short materialIndex
                // unsigned short numFaces
                // unsigned short* faceVertexIndices ((v1, v2, v3) * numFaces)
                // unsigned short numDedicatedVertices
                OOF_VPOSITIONS        = 0x3510, //present only if numDedicatedVertices > 0
                    // float* (x,y,z) * numVertices
                OOF_VNORMALS        = 0x3520, // optional, present only if numDedicatedVertices > 0
                    // float* (x,y,z) * numVertices
                OOF_VTEXCOORDS        = 0x3530, // optional, present only if numDedicatedVertices > 0
                    // float* (u,v) * numVertices
                OOF_VCOLOURS        = 0x3540, // optional, present only if numDedicatedVertices > 0
                    // float* (r,g,b) * numVertices
    };

} // namespace


#endif

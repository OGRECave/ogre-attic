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
#ifndef __MeshFileFormat_H__
#define __MeshFileFormat_H__

#include "OgrePrerequisites.h"

namespace Ogre {

/** Definition of the OGRE .mesh file format 

    .mesh files are binary files (for read efficiency at runtime) and are arranged into chunks 
    of data, very like 3D Studio's format.
    A chunk always consists of:
        unsigned short CHUNK_ID        : one of the following chunk ids identifying the chunk
        unsigned long  DATA_LENGTH     : length of the contained data in bytes
        void*          DATA            : the data, which may contain other sub-chunks (various data types)
    
    A .mesh file can contain both the definition of the Mesh itself, and optionally the definitions
    of the materials is uses (although these can be omitted, if so the Mesh assumes that at runtime the
    Materials referred to by name in the Mesh are loaded/created from another source)

    A .mesh file only contains a single mesh, which can itself have multiple submeshes.

*/
    enum MeshChunkIDs {
        M_HEADER                = 0x1000,
            // char*          version           : Version number check
            // unsigned short numMaterials      : Number of materials 
        M_MATERIAL            = 0x2000,
            // char* name 
            // AMBIENT
            // float r, g, b
            // DIFFUSE
            // float r, g, b
            // SPECULAR
            // float r, g, b
            // SHININESS
            // float val;
            OOF_TEXTURE_LAYER    = 0x2200, // optional, repeat per layer
                // char* name 
                // TODO - scale, offset, effects
        M_MESH                = 0x3000,
            // unsigned short numSharedVertices
            M_SHAREDPOSITIONS        = 0x3100, // present only if numSharedVertices > 0
                // float* (x,y,z) * numVertices
            M_SHAREDNORMALS        = 0x3200, // optional, present only if numSharedVertices > 0
                // float* (x,y,z) * numVertices
            M_SHAREDTEXCOORDS        = 0x3300, // optional, present only if numSharedVertices > 0
                // float* (u,v) * numVertices
            M_SHAREDCOLOURS        = 0x3400, // optional, present only if numSharedVertices > 0
                // float* (r,g,b) * numVertices
        M_SUBMESH = 0x4000, 
            // char* materialName
            // unsigned short numFaces
            // unsigned short* faceVertexIndices ((v1, v2, v3) * numFaces)
            // unsigned short numOwnVertices
            OOF_VPOSITIONS        = 0x4100, //present only if numOwnVertices > 0
                // float* (x,y,z) * numVertices
            OOF_VNORMALS        = 0x4200, // optional, present only if numOwnVertices > 0
                // float* (x,y,z) * numVertices
            OOF_VTEXCOORDS        = 0x4300, // optional, present only if numOwnVertices > 0
                // float* (u,v) * numVertices
            OOF_VCOLOURS        = 0x4400, // optional, present only if numOwnVertices > 0
                // float* (r,g,b) * numVertices
    };

} // namespace


#endif

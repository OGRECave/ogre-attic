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
#ifndef __MeshFileFormat_H__
#define __MeshFileFormat_H__

#include "OgrePrerequisites.h"

namespace Ogre {

/** Definition of the OGRE .mesh file format 

    .mesh files are binary files (for read efficiency at runtime) and are arranged into chunks 
    of data, very like 3D Studio's format.
    A chunk always consists of:
        unsigned short CHUNK_ID        : one of the following chunk ids identifying the chunk
        unsigned long  LENGTH          : length of the chunk in bytes, including this header
        void*          DATA            : the data, which may contain other sub-chunks (various data types)
    
    A .mesh file can contain both the definition of the Mesh itself, and optionally the definitions
    of the materials is uses (although these can be omitted, if so the Mesh assumes that at runtime the
    Materials referred to by name in the Mesh are loaded/created from another source)

    A .mesh file only contains a single mesh, which can itself have multiple submeshes.

*/
    enum MeshChunkID {
        M_HEADER                = 0x1000,
            // char*          version           : Version number check
        M_MATERIAL            = 0x2000,
            // char* name 
            // AMBIENT
            // Real r, g, b
            // DIFFUSE
            // Real r, g, b
            // SPECULAR
            // Real r, g, b
            // SHININESS
            // Real val;
            M_TEXTURE_LAYER    = 0x2200, // optional, repeat per layer
                // char* name 
                // TODO - scale, offset, effects
        M_MESH                = 0x3000,
            // M_GEOMETRY chunk
            M_SUBMESH             = 0x4000, 
                // char* materialName
                // bool useSharedVertices
                // unsigned int numFaces
                // unsigned int* faceVertexIndices ((v1, v2, v3) * numFaces)
                // M_GEOMETRY chunk (Optional: present only if useSharedVertices = false)
                M_SUBMESH_BONE_ASSIGNMENT = 0x4100,
                    // Optional bone weights (repeating section)
                    // unsigned int vertexIndex;
                    // unsigned short boneIndex;
                    // Real weight;
            M_GEOMETRY          = 0x5000, // NB this chunk is embedded within M_MESH and M_SUBMESH
                // unsigned int numVertices
                // Real* pVertices (x, y, z order x numVertices)
                M_GEOMETRY_NORMALS = 0x5100,    //(Optional)
                    // Real* pNormals (x, y, z order x numVertices)
                M_GEOMETRY_COLOURS = 0x5200,    //(Optional)
                    // unsigned long* pColours (RGBA 8888 format x numVertices)
                M_GEOMETRY_TEXCOORDS = 0x5300,    //(Optional, REPEATABLE, each one adds an extra set)
                    // unsigned short dimensions    (1 for 1D, 2 for 2D, 3 for 3D)
                    // Real* pTexCoords  (u [v] [w] order, dimensions x numVertices)
            M_MESH_SKELETON_LINK = 0x6000,
                // Optional link to skeleton
                // char* skeletonName           : name of .skeleton to use
            M_MESH_BONE_ASSIGNMENT = 0x7000,
                // Optional bone weights (repeating section)
                // unsigned int vertexIndex;
                // unsigned short boneIndex;
                // Real weight;
            M_MESH_LOD = 0x8000,
                // Optional LOD information
                // unsigned short numLevels;
                // bool manual;  (true for manual alternate meshes, false for generated)
                M_MESH_LOD_USAGE = 0x8100,
                // Repeating section, ordered in increasing depth
				// NB LOD 0 (full detail from 0 depth) is omitted
                // Real fromSquaredDepth;
                    M_MESH_LOD_MANUAL = 0x8110,
                    // Required if M_MESH_LOD section manual = true
                    // String manualMeshName;
                    M_MESH_LOD_GENERATED = 0x8120
                    // Required if M_MESH_LOD section manual = false
					// Repeating section (1 per submesh)
                    // unsigned int numFaces;
                    // unsigned int* faceIndexes;  ((v1, v2, v3) * numFaces)
                    
                    


                


    };

	/* Version 1.0 of the .mesh fornmat (deprecated)
    enum MeshChunkID {
        M_HEADER                = 0x1000,
            // char*          version           : Version number check
        M_MATERIAL            = 0x2000,
            // char* name 
            // AMBIENT
            // Real r, g, b
            // DIFFUSE
            // Real r, g, b
            // SPECULAR
            // Real r, g, b
            // SHININESS
            // Real val;
            M_TEXTURE_LAYER    = 0x2200, // optional, repeat per layer
                // char* name 
                // TODO - scale, offset, effects
        M_MESH                = 0x3000,
            // M_GEOMETRY chunk
            M_SUBMESH             = 0x4000, 
                // char* materialName
                // bool useSharedVertices
                // unsigned short numFaces
                // unsigned short* faceVertexIndices ((v1, v2, v3) * numFaces)
                // M_GEOMETRY chunk (Optional: present only if useSharedVertices = false)
                M_SUBMESH_BONE_ASSIGNMENT = 0x4100,
                    // Optional bone weights (repeating section)
                    // unsigned short vertexIndex;
                    // unsigned short boneIndex;
                    // Real weight;
            M_GEOMETRY          = 0x5000, // NB this chunk is embedded within M_MESH and M_SUBMESH
                // unsigned short numVertices
                // Real* pVertices (x, y, z order x numVertices)
                M_GEOMETRY_NORMALS = 0x5100,    //(Optional)
                    // Real* pNormals (x, y, z order x numVertices)
                M_GEOMETRY_COLOURS = 0x5200,    //(Optional)
                    // unsigned long* pColours (RGBA 8888 format x numVertices)
                M_GEOMETRY_TEXCOORDS = 0x5300,    //(Optional, REPEATABLE, each one adds an extra set)
                    // unsigned short dimensions    (1 for 1D, 2 for 2D, 3 for 3D)
                    // Real* pTexCoords  (u [v] [w] order, dimensions x numVertices)
            M_MESH_SKELETON_LINK = 0x6000,
                // Optional link to skeleton
                // char* skeletonName           : name of .skeleton to use
            M_MESH_BONE_ASSIGNMENT = 0x7000,
                // Optional bone weights (repeating section)
                // unsigned short vertexIndex;
                // unsigned short boneIndex;
                // Real weight;
            M_MESH_LOD = 0x8000,
                // Optional LOD information
                // unsigned short numLevels;
                // bool manual;  (true for manual alternate meshes, false for generated)
                M_MESH_LOD_USAGE = 0x8100,
                // Repeating section, ordered in increasing depth
				// NB LOD 0 (full detail from 0 depth) is omitted
                // Real fromSquaredDepth;
                    M_MESH_LOD_MANUAL = 0x8110,
                    // Required if M_MESH_LOD section manual = true
                    // String manualMeshName;
                    M_MESH_LOD_GENERATED = 0x8120
                    // Required if M_MESH_LOD section manual = false
					// Repeating section (1 per submesh)
                    // unsigned short numFaces;
                    // unsigned short* faceIndexes;  ((v1, v2, v3) * numFaces)
                    
                    


                


    };
*/
} // namespace


#endif

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

#include "OgreMeshSerializer.h"
#include "OgreMeshFileFormat.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreException.h"
#include "OgreOofModelFile.h"
#include "OgreMaterialManager.h"
#include "OgreLogManager.h"
#include "OgreSkeleton.h"


namespace Ogre {

    /// Chunk overhead = ID + size
    const unsigned long CHUNK_OVERHEAD_SIZE = sizeof(unsigned short) + sizeof(unsigned long);
    //---------------------------------------------------------------------
    MeshSerializer::MeshSerializer()
    {
        mpMesh = 0;

        // Version number
        mVersion = "[MeshSerializer_v1.00]";
    }
    //---------------------------------------------------------------------
    MeshSerializer::~MeshSerializer()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::exportMesh(const Mesh* pMesh, const String& filename, bool includeMaterials)
    {
        LogManager::getSingleton().logMessage("MeshSerializer writing mesh data to " + filename + "...");

        MaterialManager& matMgr = MaterialManager::getSingleton();
        mpfFile = fopen(filename, "wb");

        writeFileHeader();
        LogManager::getSingleton().logMessage("File header written.");

        // Write materials if required
        if (includeMaterials)
        {
            LogManager::getSingleton().logMessage("Writing Materials to file...");
            for (int i = 0; i < pMesh->getNumSubMeshes(); ++i)
            {
                SubMesh* sm = pMesh->getSubMesh(i);
                Material* pMat = (Material*)matMgr.getByName(sm->getMaterialName());
                if (pMat)
                {
                    LogManager::getSingleton().logMessage("Exporting Material '" + pMat->getName() + "'...");
                    writeMaterial(pMat);
                    LogManager::getSingleton().logMessage("Material '" + pMat->getName() + "' exported.");
                }
            }
        }

        LogManager::getSingleton().logMessage("Writing mesh data...");
        writeMesh(pMesh);
        LogManager::getSingleton().logMessage("Mesh data exported.");

        fclose(mpfFile);
        LogManager::getSingleton().logMessage("MeshSerializer export successful.");
    }
    //---------------------------------------------------------------------
    void MeshSerializer::importMesh(DataChunk& chunk, Mesh* pDest)
    {
        mpMesh = pDest;

        // Check header
        readFileHeader(chunk);

        unsigned short chunkID;
        while(!chunk.isEOF())
        {
            chunkID = readChunk(chunk);
            switch (chunkID)
            {
            case M_MATERIAL:
                readMaterial(chunk);
                break;
            case M_MESH:
                readMesh(chunk);
                break;
            }
        }
    }
    //---------------------------------------------------------------------
    void MeshSerializer::importLegacyOof(DataChunk& chunk, Mesh* pDest)
    {
        // Load from OOF (Ogre Object File)
        OofModelFile oofModel;
        MaterialManager& matMgr = MaterialManager::getSingleton();

        oofModel.load(chunk);

        // Set memory deallocation off (allows us to use pointers)
        oofModel.autoDeallocateMemory = false;
        // Copy root-level geometry, including pointers
        // We've told the OofModel not to deallocate
        pDest->sharedGeometry = oofModel.sharedGeometry;
        pDest->sharedGeometry.numBlendWeightsPerVertex = 0; // oof does not support skeletons

        // Create sub-meshes from the loaded model
        for (unsigned int meshNo = 0; meshNo < oofModel.materials.size(); ++meshNo)
        {
            SubMesh* sub = pDest->createSubMesh();
            // Copy submesh geometry if present
            sub->useSharedVertices = oofModel.materials[meshNo].useSharedVertices;
            if (!sub->useSharedVertices)
            {
                sub->geometry = oofModel.materials[meshNo].materialGeometry;
                sub->geometry.numBlendWeightsPerVertex = 0; // oof does not support skeletons
            }

            // Always create materials from oof
            try 
            {
                matMgr.add(oofModel.materials[meshNo].material);
            }
            catch (Exception& e)
            {
                if(e.getNumber() == Exception::ERR_DUPLICATE_ITEM)
                {
                    // Material already exists
                    char msg[256];
                    sprintf(msg, "Material '%s' in model '%s' has been ignored "
                        "because a material with the same name has already "
                        "been registered.", 
                        oofModel.materials[meshNo].material.getName().c_str(),
                        pDest->getName().c_str());
                    LogManager::getSingleton().logMessage(msg);
                }
                else
                {
                    throw;
                }
            }

            sub->setMaterialName(oofModel.materials[meshNo].material.getName());

            sub->numFaces = oofModel.materials[meshNo].numFaces;
            sub->faceVertexIndices = oofModel.materials[meshNo].pIndexes;



        }
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeMaterial(const Material* m)
    {
        // Header
        writeChunkHeader(M_MATERIAL, calcMaterialSize(m));

        // Name
        writeString(m->getName());

        // Ambient
        const ColourValue& ambient = m->getAmbient();
        writeData(&ambient.r, sizeof(Real), 1);
        writeData(&ambient.g, sizeof(Real), 1);
        writeData(&ambient.b, sizeof(Real), 1);

        // Diffuse
        const ColourValue& diffuse = m->getDiffuse();
        writeData(&diffuse.r, sizeof(Real), 1);
        writeData(&diffuse.g, sizeof(Real), 1);
        writeData(&diffuse.b, sizeof(Real), 1);

        // Specular
        const ColourValue& specular = m->getSpecular();
        writeData(&specular.r, sizeof(Real), 1);
        writeData(&specular.g, sizeof(Real), 1);
        writeData(&specular.b, sizeof(Real), 1);

        // Shininess
        Real val = m->getShininess();
        writeData(&val, sizeof(Real), 1);

        // Nested texture layers
        for (int i = 0; i < m->getNumTextureLayers(); ++i)
        {
            writeTextureLayer(m->getTextureLayer(i));
        }

    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeTextureLayer(const Material::TextureLayer* pTex)
    {
        // Header
        writeChunkHeader(M_TEXTURE_LAYER, calcTextureLayerSize(pTex));

        // Name
        writeString(pTex->getTextureName());

    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeMesh(const Mesh* pMesh)
    {
        // Header
        writeChunkHeader(M_MESH, calcMeshSize(pMesh));

        // Write geometry
        writeGeometry(&pMesh->sharedGeometry);

        // Write Submeshes
        for (int i = 0; i < pMesh->getNumSubMeshes(); ++i)
        {
            LogManager::getSingleton().logMessage("Writing submesh...");
            writeSubMesh(pMesh->getSubMesh(i));
            LogManager::getSingleton().logMessage("Submesh exported.");
        }

        // Write skeleton info if required
        if (pMesh->hasSkeleton())
        {
            LogManager::getSingleton().logMessage("Exporting skeleton link...");
            // Write skeleton link
            writeSkeletonLink(pMesh->getSkeleton());
            LogManager::getSingleton().logMessage("Skeleton link exported.");

            // Write bone assignments
            if (!pMesh->mBoneAssignments.empty())
            {
                LogManager::getSingleton().logMessage("Exporting shared geometry bone assignments...");

                Mesh::VertexBoneAssignmentList::const_iterator vi;
                for (vi = pMesh->mBoneAssignments.begin(); 
                vi != pMesh->mBoneAssignments.end(); ++vi)
                {
                    writeMeshBoneAssignment(&(vi->second));
                }

                LogManager::getSingleton().logMessage("Shared geometry bone assignments exported.");
            }
        }


    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeSubMesh(const SubMesh* s)
    {
        // Header
        writeChunkHeader(M_SUBMESH, calcSubMeshSize(s));

        // char* materialName
        writeString(s->getMaterialName());

        // bool useSharedVertices
        writeData(&s->useSharedVertices, sizeof(bool), 1);

        // unsigned short numFaces
        writeData(&s->numFaces, sizeof(unsigned short), 1);

        // unsigned short* faceVertexIndices ((v1, v2, v3) * numFaces)
        writeShorts(s->faceVertexIndices, s->numFaces * 3);

        // M_GEOMETRY chunk (Optional: present only if useSharedVertices = false)
        if (!s->useSharedVertices)
        {
            writeGeometry(&s->geometry);
        }

        // Bone assignments
        if (!s->mBoneAssignments.empty())
        {
            LogManager::getSingleton().logMessage("Exporting dedicated geometry bone assignments...");

            SubMesh::VertexBoneAssignmentList::const_iterator vi;
            for (vi = s->mBoneAssignments.begin(); 
            vi != s->mBoneAssignments.end(); ++vi)
            {
                writeSubMeshBoneAssignment(&(vi->second));
            }

            LogManager::getSingleton().logMessage("Dedicated geometry bone assignments exported.");
        }


    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeGeometry(const GeometryData* pGeom)
    {
        // Header
        writeChunkHeader(M_GEOMETRY, calcGeometrySize(pGeom));

        // unsigned short numVertices
        writeData(&pGeom->numVertices, sizeof(unsigned short), 1);

        // Real* pVertices (x, y, z order x numVertices)
        writeReals(pGeom->pVertices, pGeom->numVertices * 3);

        if (pGeom->hasNormals)
        {
            writeChunkHeader(M_GEOMETRY_NORMALS, sizeof(Real) * pGeom->numVertices * 3);

            // Real* pNormals (x, y, z order x numVertices)
            writeReals(pGeom->pNormals, pGeom->numVertices * 3);
        }

        if (pGeom->hasColours)
        {
            writeChunkHeader(M_GEOMETRY_COLOURS, sizeof(unsigned long) * pGeom->numVertices);
            // unsigned long* pColours (RGBA 8888 format x numVertices)
            writeLongs((unsigned long*)pGeom->pColours, pGeom->numVertices);
        }

        for (int t = 0; t < pGeom->numTexCoords; ++t)
        {
            writeChunkHeader(M_GEOMETRY_TEXCOORDS, 
                sizeof(Real) * pGeom->numVertices * pGeom->numTexCoordDimensions[t]);

            // unsigned short dimensions    (1 for 1D, 2 for 2D, 3 for 3D)
            writeData(&pGeom->numTexCoordDimensions[t], sizeof(unsigned short), 1);
            // Real* pTexCoords  (u [v] [w] order, dimensions x numVertices)
            writeReals(pGeom->pTexCoords[t], pGeom->numVertices * pGeom->numTexCoordDimensions[t]);
        }


    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializer::calcMaterialSize(const Material* pMat)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // Name, including terminator
        size += (unsigned long)pMat->getName().length() + 1;
        // Ambient 
        size += sizeof(Real) * 3;
        // Diffuse
        size += sizeof(Real) * 3;
        // Specular
        size += sizeof(Real) * 3;
        // Shininess
        size += sizeof(Real);

        // Nested texture layers
        for (int i = 0; i < pMat->getNumTextureLayers(); ++i)
        {
            size += calcTextureLayerSize(pMat->getTextureLayer(i));
        }

        return size;
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializer::calcTextureLayerSize(const Material::TextureLayer* pTex)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // Name, including terminator
        size += (unsigned long)pTex->getTextureName().length() + 1;

        return size;
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializer::calcMeshSize(const Mesh* pMesh)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // Num shared vertices
        size += sizeof(unsigned short);

        // Geometry
        if (pMesh->sharedGeometry.numVertices > 0)
        {
            size += calcGeometrySize(&(pMesh->sharedGeometry));
        }

        // Submeshes
        for (int i = 0; i < pMesh->getNumSubMeshes(); ++i)
        {
            size += calcSubMeshSize(pMesh->getSubMesh(i));
        }

        // Skeleton link
        if (pMesh->hasSkeleton())
        {
            size += calcSkeletonLinkSize(pMesh->getSkeleton());
        }

        return size;


    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializer::calcSubMeshSize(const SubMesh* pSub)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // Material name
        size += (unsigned long)pSub->getMaterialName().length() + 1;

        // bool useSharedVertices
        size += sizeof(bool);
        // unsigned short numFaces
        size += sizeof(unsigned short);
        // unsigned short* faceVertexIndices ((v1, v2, v3) * numFaces)
        size += sizeof(unsigned short) * pSub->numFaces * 3;

        // Geometry
        if (!pSub->useSharedVertices)
        {
            size += calcGeometrySize(&pSub->geometry);
        }

        return size;
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializer::calcGeometrySize(const GeometryData* pGeom)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // Num vertices
        size += sizeof(unsigned short);
        // Vertex data
        size += sizeof(Real) * pGeom->numVertices * 3;

        if (pGeom->hasNormals)
        {
            size += CHUNK_OVERHEAD_SIZE; // subheader
            size += sizeof(Real) * pGeom->numVertices * 3; //data
        }

        for (int i = 0; i < pGeom->numTexCoords; ++i)
        {
            size += CHUNK_OVERHEAD_SIZE; // subheader
            size += sizeof(unsigned short); // dimensions
            size += sizeof(Real) * pGeom->numVertices * pGeom->numTexCoordDimensions[i]; //data
        }

        if (pGeom->hasColours) 
        {
            size += CHUNK_OVERHEAD_SIZE; // subheader
            size += sizeof(unsigned long) * pGeom->numVertices; //data
        }

        return size;
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readMaterial(DataChunk& chunk)
    {
        ColourValue col;
        Real rVal;

        // char* name 
        String name = readString(chunk);

        // Create a new material
        Material* pMat;
        try 
        {
            pMat = (Material*)MaterialManager::getSingleton().create(name);
        }
        catch (Exception& e)
        {
            if(e.getNumber() == Exception::ERR_DUPLICATE_ITEM)
            {
                // Material already exists
                char msg[256];
                sprintf(msg, "Material '%s' in model '%s' has been ignored "
                    "because a material with the same name has already "
                    "been registered.", name.c_str(),
                    mpMesh->getName().c_str());
                LogManager::getSingleton().logMessage(msg);
                // Skip the rest of this material
                chunk.skip(mCurrentChunkLen - name.length() - 1 - CHUNK_OVERHEAD_SIZE);
                return;

            }
            else
            {
                throw;
            }
        }

        // AMBIENT
        // Real r, g, b
        chunk.read(&col.r, sizeof(Real));
        chunk.read(&col.g, sizeof(Real));
        chunk.read(&col.b, sizeof(Real));
        pMat->setAmbient(col);

        // DIFFUSE
        // Real r, g, b
        chunk.read(&col.r, sizeof(Real));
        chunk.read(&col.g, sizeof(Real));
        chunk.read(&col.b, sizeof(Real));
        pMat->setDiffuse(col);

        // SPECULAR
        // Real r, g, b
        chunk.read(&col.r, sizeof(Real));
        chunk.read(&col.g, sizeof(Real));
        chunk.read(&col.b, sizeof(Real));
        pMat->setSpecular(col);

        // SHININESS
        // Real val;
        chunk.read(&rVal, sizeof(Real));
        pMat->setShininess(rVal);

        // Read any texture layers
        if (!chunk.isEOF())
        {
            unsigned short chunkID = readChunk(chunk);
            while(chunkID == M_TEXTURE_LAYER && !chunk.isEOF())
            {
                readTextureLayer(chunk, pMat);
                if (!chunk.isEOF())
                {
                    chunkID = readChunk(chunk);
                }
            }
            // Get next chunk
            if (!chunk.isEOF())
            {
                // Backpedal back to start of non-texture layer chunk
                chunk.skip(-(long)CHUNK_OVERHEAD_SIZE);
            }
        }

    }
    //---------------------------------------------------------------------
    void MeshSerializer::readTextureLayer(DataChunk& chunk, Material* pMat)
    {
        // Just name for now
        String name = readString(chunk);

        pMat->addTextureLayer(name);
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readMesh(DataChunk& chunk)
    {
        unsigned short chunkID;
        // M_GEOMETRY chunk
        chunkID = readChunk(chunk);
        if (chunkID != M_GEOMETRY)
        {
            Except(Exception::ERR_INTERNAL_ERROR, "Missing geometry data in mesh file", 
                "MeshSerializer::readMesh");
        }
        readGeometry(chunk, &mpMesh->sharedGeometry);

        // Find all subchunks 
        if (!chunk.isEOF())
        {
            chunkID = readChunk(chunk);
            while(!chunk.isEOF() &&
                (chunkID == M_SUBMESH ||
                 chunkID == M_MESH_SKELETON_LINK ||
                 chunkID == M_MESH_BONE_ASSIGNMENT))
            {
                switch(chunkID)
                {
                case M_SUBMESH:
                    readSubMesh(chunk);
                    break;
                case M_MESH_SKELETON_LINK:
                    readSkeletonLink(chunk);
                    break;
                case M_MESH_BONE_ASSIGNMENT:
                    readMeshBoneAssignment(chunk);
                    break;
                }

                if (!chunk.isEOF())
                {
                    chunkID = readChunk(chunk);
                }

            }
            if (!chunk.isEOF())
            {
                // Backpedal back to start of chunk
                chunk.skip(-(long)CHUNK_OVERHEAD_SIZE);
            }
        }

    }
    //---------------------------------------------------------------------
    void MeshSerializer::readSubMesh(DataChunk& chunk)
    {
        unsigned short chunkID;

        SubMesh* sm = mpMesh->createSubMesh();
        // char* materialName
        String materialName = readString(chunk);
        sm->setMaterialName(materialName);

        // bool useSharedVertices
        chunk.read(&sm->useSharedVertices, sizeof(bool));

        // unsigned short numFaces
        chunk.read(&sm->numFaces, sizeof(unsigned short));

        // unsigned short* faceVertexIndices ((v1, v2, v3) * numFaces)
        sm->faceVertexIndices = new unsigned short[sm->numFaces * 3];
        readShorts(chunk, sm->faceVertexIndices, sm->numFaces * 3);

        // M_GEOMETRY chunk (Optional: present only if useSharedVertices = false)
        if (!sm->useSharedVertices)
        {
            chunkID = readChunk(chunk);
            if (chunkID != M_GEOMETRY)
            {
                Except(Exception::ERR_INTERNAL_ERROR, "Missing geometry data in mesh file", 
                    "MeshSerializer::readSubMesh");
            }
            readGeometry(chunk, &sm->geometry);
        }


        // Find all bone assignments (if present) 
        if (!chunk.isEOF())
        {
            chunkID = readChunk(chunk);
            while(!chunk.isEOF() &&
                (chunkID == M_SUBMESH_BONE_ASSIGNMENT))
            {
                switch(chunkID)
                {
                case M_SUBMESH_BONE_ASSIGNMENT:
                    readSubMeshBoneAssignment(chunk, sm);
                    break;
                }

                if (!chunk.isEOF())
                {
                    chunkID = readChunk(chunk);
                }

            }
            if (!chunk.isEOF())
            {
                // Backpedal back to start of chunk
                chunk.skip(-(long)CHUNK_OVERHEAD_SIZE);
            }
        }

    }
    //---------------------------------------------------------------------
    void MeshSerializer::readGeometry(DataChunk& chunk, GeometryData* dest)
    {
        unsigned short texCoordSet = 0;

        // unsigned short numVertices
        chunk.read(&dest->numVertices, sizeof(unsigned short));

        // Real* pVertices (x, y, z order x numVertices)
        dest->pVertices = new Real[dest->numVertices * 3];
        dest->vertexStride = 0;
        readReals(chunk, dest->pVertices, dest->numVertices * 3);

        // Find optional geometry chunks
        if (!chunk.isEOF())
        {
            unsigned short chunkID = readChunk(chunk);
            while(!chunk.isEOF() && 
                (chunkID == M_GEOMETRY_NORMALS || 
                 chunkID == M_GEOMETRY_COLOURS ||
                 chunkID == M_GEOMETRY_TEXCOORDS ))
            {
                switch (chunkID)
                {
                case M_GEOMETRY_NORMALS:
                    // Real* pNormals (x, y, z order x numVertices)
                    dest->hasNormals = true;
                    dest->normalStride = 0;
                    dest->pNormals = new Real[dest->numVertices * 3];
                    readReals(chunk, dest->pNormals, dest->numVertices * 3);
                    break;
                case M_GEOMETRY_COLOURS:
                    // unsigned long* pColours (RGBA 8888 format x numVertices)
                    dest->hasColours = true;
                    dest->colourStride = 0;
                    dest->pColours = new unsigned long[dest->numVertices];
                    readLongs(chunk, dest->pColours, dest->numVertices);
                    break;
                case M_GEOMETRY_TEXCOORDS:
                    // unsigned short dimensions    (1 for 1D, 2 for 2D, 3 for 3D)
                    chunk.read(&dest->numTexCoordDimensions[texCoordSet], sizeof(unsigned short));
                    // Real* pTexCoords  (u [v] [w] order, dimensions x numVertices)
                    dest->pTexCoords[texCoordSet] = 
                        new Real[dest->numVertices * dest->numTexCoordDimensions[texCoordSet]];
                    dest->texCoordStride[texCoordSet] = 0;
                    readReals(chunk, dest->pTexCoords[texCoordSet],
                        dest->numVertices * dest->numTexCoordDimensions[texCoordSet]);
                    ++texCoordSet;
                    break;
                }
                // Get next chunk
                if (!chunk.isEOF())
                {
                    chunkID = readChunk(chunk);
                }
            }
            if (!chunk.isEOF())
            {
                // Backpedal back to start of non-submesh chunk
                chunk.skip(-(long)CHUNK_OVERHEAD_SIZE);
            }
            // Store number of texture coordinate sets found
            dest->numTexCoords = texCoordSet;
        }
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeSkeletonLink(const Skeleton* pSkel)
    {
        writeChunkHeader(M_MESH_SKELETON_LINK, calcSkeletonLinkSize(pSkel));

        writeString(pSkel->getName());

    }
    //---------------------------------------------------------------------
    void MeshSerializer::readSkeletonLink(DataChunk &chunk)
    {
        String skelName = readString(chunk);
        mpMesh->setSkeletonName(skelName);
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializer::calcSkeletonLinkSize(const Skeleton* pSkel)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        size += (unsigned long)pSkel->getName().length() + 1;

        return size;

    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeMeshBoneAssignment(const VertexBoneAssignment* assign)
    {
        writeChunkHeader(M_MESH_BONE_ASSIGNMENT, calcBoneAssignmentSize());

        // unsigned short vertexIndex;
        writeShorts(&(assign->vertexIndex), 1);
        // unsigned short boneIndex;
        writeShorts(&(assign->boneIndex), 1);
        // Real weight;
        writeReals(&(assign->weight), 1);
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeSubMeshBoneAssignment(const VertexBoneAssignment* assign)
    {
        writeChunkHeader(M_SUBMESH_BONE_ASSIGNMENT, calcBoneAssignmentSize());

        // unsigned short vertexIndex;
        writeShorts(&(assign->vertexIndex), 1);
        // unsigned short boneIndex;
        writeShorts(&(assign->boneIndex), 1);
        // Real weight;
        writeReals(&(assign->weight), 1);
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readMeshBoneAssignment(DataChunk& chunk)
    {
        VertexBoneAssignment assign;

        // unsigned short vertexIndex;
        readShorts(chunk, &(assign.vertexIndex),1);
        // unsigned short boneIndex;
        readShorts(chunk, &(assign.boneIndex),1);
        // Real weight;
        readReals(chunk, &(assign.weight), 1);

        mpMesh->addBoneAssignment(assign);

    }
    //---------------------------------------------------------------------
    void MeshSerializer::readSubMeshBoneAssignment(DataChunk& chunk, SubMesh* sub)
    {
        VertexBoneAssignment assign;

        // unsigned short vertexIndex;
        readShorts(chunk, &(assign.vertexIndex),1);
        // unsigned short boneIndex;
        readShorts(chunk, &(assign.boneIndex),1);
        // Real weight;
        readReals(chunk, &(assign.weight), 1);

        sub->addBoneAssignment(assign);

    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializer::calcBoneAssignmentSize(void)
    {
        unsigned long size;

        size = CHUNK_OVERHEAD_SIZE;

        // Vert index
        size += sizeof(unsigned short);
        // Bone index
        size += sizeof(unsigned short);
        // weight
        size += sizeof(Real);

        return size;
    }



}


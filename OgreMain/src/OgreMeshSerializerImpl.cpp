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

#include "OgreMeshSerializerImpl.h"
#include "OgreMeshFileFormat.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreException.h"
#include "OgreMaterialManager.h"
#include "OgreLogManager.h"
#include "OgreSkeleton.h"
#include "OgreHardwareBufferManager.h"


namespace Ogre {

    /// Chunk overhead = ID + size
    const unsigned long CHUNK_OVERHEAD_SIZE = sizeof(unsigned short) + sizeof(unsigned long);
    //---------------------------------------------------------------------
    MeshSerializerImpl::MeshSerializerImpl()
    {
        mpMesh = 0;

        // Version number
        mVersion = "[MeshSerializer_v1.10]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl::~MeshSerializerImpl()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::exportMesh(const Mesh* pMesh, const String& filename, bool includeMaterials)
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
    void MeshSerializerImpl::importMesh(DataChunk& chunk, Mesh* pDest)
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
    void MeshSerializerImpl::writeMaterial(const Material* m)
    {
        // Header
        writeChunkHeader(M_MATERIAL, calcMaterialSize(m));

        // Name
        writeString(m->getName());

        // Ambient
        const ColourValue& ambient = m->getAmbient();
        writeReals(&ambient.r, 1);
        writeReals(&ambient.g, 1);
        writeReals(&ambient.b, 1);

        // Diffuse
        const ColourValue& diffuse = m->getDiffuse();
        writeReals(&diffuse.r, 1);
        writeReals(&diffuse.g, 1);
        writeReals(&diffuse.b, 1);

        // Specular
        const ColourValue& specular = m->getSpecular();
        writeReals(&specular.r, 1);
        writeReals(&specular.g, 1);
        writeReals(&specular.b, 1);

        // Shininess
        Real val = m->getShininess();
        writeReals(&val, 1);

        // Nested texture layers
        for (int i = 0; i < m->getNumTextureLayers(); ++i)
        {
            writeTextureLayer(m->getTextureLayer(i));
        }

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeTextureLayer(const Material::TextureLayer* pTex)
    {
        // Header
        writeChunkHeader(M_TEXTURE_LAYER, calcTextureLayerSize(pTex));

        // Name
        writeString(pTex->getTextureName());

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeMesh(const Mesh* pMesh)
    {
        // Header
        writeChunkHeader(M_MESH, calcMeshSize(pMesh));

        // Write geometry
        writeGeometry(pMesh->sharedVertexData);

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
            writeSkeletonLink(pMesh->getSkeletonName());
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

        // Write LOD data if any
        if (pMesh->getNumLodLevels() > 1)
        {
            LogManager::getSingleton().logMessage("Exporting LOD information....");
            writeLodInfo(pMesh);
            LogManager::getSingleton().logMessage("LOD information exported.");
            
        }


    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSubMesh(const SubMesh* s)
    {
        // Header
        writeChunkHeader(M_SUBMESH, calcSubMeshSize(s));

        // char* materialName
        writeString(s->getMaterialName());

        // bool useSharedVertices
        writeBools(&s->useSharedVertices, 1);

		// unsigned int indexCount
        writeInts(&s->indexData->indexCount, 1);

        // bool indexes32Bit
        bool idx32bit = (s->indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT);
        writeBools(&idx32bit, 1);

        // unsigned short* faceVertexIndices ((indexCount)
        HardwareIndexBufferSharedPtr ibuf = s->indexData->indexBuffer;
        void* pIdx = ibuf->lock(0, ibuf->getSizeInBytes(), HardwareBuffer::HBL_READ_ONLY);
        if (idx32bit)
        {
            unsigned int* pIdx32 = static_cast<unsigned int*>(pIdx);
            writeInts(pIdx32, s->indexData->indexCount);
        }
        else
        {
            unsigned short* pIdx16 = static_cast<unsigned short*>(pIdx);
            writeShorts(pIdx16, s->indexData->indexCount);
        }
        ibuf->unlock();

        // M_GEOMETRY chunk (Optional: present only if useSharedVertices = false)
        if (!s->useSharedVertices)
        {
            writeGeometry(s->vertexData);
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
    void MeshSerializerImpl::writeGeometry(const VertexData* pGeom)
    {
		/* TODO
        // Header
        writeChunkHeader(M_GEOMETRY, calcGeometrySize(pGeom));

        // unsigned short numVertices
        writeShorts(&pGeom->numVertices, 1);

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
            writeShorts(&pGeom->numTexCoordDimensions[t], 1);
            // Real* pTexCoords  (u [v] [w] order, dimensions x numVertices)
            writeReals(pGeom->pTexCoords[t], pGeom->numVertices * pGeom->numTexCoordDimensions[t]);
        }
		*/


    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcMaterialSize(const Material* pMat)
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
    unsigned long MeshSerializerImpl::calcTextureLayerSize(const Material::TextureLayer* pTex)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // Name, including terminator
        size += (unsigned long)pTex->getTextureName().length() + 1;

        return size;
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcMeshSize(const Mesh* pMesh)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;
		/* TODO

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
            size += calcSkeletonLinkSize(pMesh->getSkeletonName());
        }

		*/
        return size;

    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcSubMeshSize(const SubMesh* pSub)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;
		/* TODO

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

		*/
        return size;
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcGeometrySize(const GeometryData* pGeom)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;
		/* TODO

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

		*/
        return size;
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readMaterial(DataChunk& chunk)
    {
        ColourValue col;
        Real rVal;

        // char* name 
        String name = readString(chunk);

        // Create a new material
        Material* pMat;
        try 
        {
            pMat = (Material*)MaterialManager::getSingleton().createDeferred(name);
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
        readReals(chunk, &col.r, 1);
        readReals(chunk, &col.g, 1);
        readReals(chunk, &col.b, 1);
        pMat->setAmbient(col);

        // DIFFUSE
        // Real r, g, b
        readReals(chunk, &col.r, 1);
        readReals(chunk, &col.g, 1);
        readReals(chunk, &col.b, 1);
        pMat->setDiffuse(col);

        // SPECULAR
        // Real r, g, b
        readReals(chunk, &col.r, 1);
        readReals(chunk, &col.g, 1);
        readReals(chunk, &col.b, 1);
        pMat->setSpecular(col);

        // SHININESS
        // Real val;
        readReals(chunk, &rVal, 1);
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
    void MeshSerializerImpl::readTextureLayer(DataChunk& chunk, Material* pMat)
    {
        // Just name for now
        String name = readString(chunk);

        pMat->addTextureLayer(name);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readMesh(DataChunk& chunk)
    {
        unsigned short chunkID;
        // M_GEOMETRY chunk
        chunkID = readChunk(chunk);
        if (chunkID == M_GEOMETRY)
        {
			mpMesh->sharedVertexData = new VertexData();
			readGeometry(chunk, mpMesh->sharedVertexData);
        }

        // Find all subchunks 
        if (!chunk.isEOF())
        {
            chunkID = readChunk(chunk);
            while(!chunk.isEOF() &&
                (chunkID == M_SUBMESH ||
                 chunkID == M_MESH_SKELETON_LINK ||
                 chunkID == M_MESH_BONE_ASSIGNMENT ||
				 chunkID == M_MESH_LOD))
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
                case M_MESH_LOD:
					readMeshLodInfo(chunk);
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
    void MeshSerializerImpl::readSubMesh(DataChunk& chunk)
    {
        unsigned short chunkID;

        SubMesh* sm = mpMesh->createSubMesh();
        // char* materialName
        String materialName = readString(chunk);
        sm->setMaterialName(materialName);

        // bool useSharedVertices
        readBools(chunk,&sm->useSharedVertices, 1);

        // unsigned int indexCount
        sm->indexData->indexStart = 0;
        readInts(chunk, &sm->indexData->indexCount, 1);

        HardwareIndexBufferSharedPtr ibuf;
        // bool indexes32Bit
        bool idx32bit;
        readBools(chunk, &idx32bit, 1);
        if (idx32bit)
        {
            ibuf = HardwareBufferManager::getSingleton().
                createIndexBuffer(
                    HardwareIndexBuffer::IT_32BIT, 
                    sm->indexData->indexCount, 
                    HardwareBuffer::HBU_STATIC);
            // unsigned int* faceVertexIndices 
            unsigned int* pIdx = static_cast<unsigned int*>(
                ibuf->lock(0, ibuf->getSizeInBytes(), HardwareBuffer::HBL_DISCARD)
                );
            readInts(chunk, pIdx, sm->indexData->indexCount);
            ibuf->unlock();

        }
        else // 16-bit
        {
            ibuf = HardwareBufferManager::getSingleton().
                createIndexBuffer(
                    HardwareIndexBuffer::IT_16BIT, 
                    sm->indexData->indexCount, 
                    HardwareBuffer::HBU_STATIC);
            // unsigned short* faceVertexIndices 
            unsigned short* pIdx = static_cast<unsigned short*>(
                ibuf->lock(0, ibuf->getSizeInBytes(), HardwareBuffer::HBL_DISCARD)
                );
            readShorts(chunk, pIdx, sm->indexData->indexCount);
            ibuf->unlock();
        }

        // M_GEOMETRY chunk (Optional: present only if useSharedVertices = false)
        if (!sm->useSharedVertices)
        {
            chunkID = readChunk(chunk);
            if (chunkID != M_GEOMETRY)
            {
                Except(Exception::ERR_INTERNAL_ERROR, "Missing geometry data in mesh file", 
                    "MeshSerializerImpl::readSubMesh");
            }
            sm->vertexData = new VertexData();
            readGeometry(chunk, sm->vertexData);
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
    void MeshSerializerImpl::readGeometry(DataChunk& chunk, VertexData* dest)
    {
        unsigned short texCoordSet = 0;
        HardwareVertexBufferSharedPtr vbuf;
        unsigned short bindIdx = 0;
        Real *pReal = 0;
        RGBA* pRGBA = 0;

        dest->vertexStart = 0;

        // unsigned int numVertices
        readInts(chunk, &dest->vertexCount, 1);

        // Vertex buffers
        // TODO: consider redesigning this so vertex buffers can be combined

        // Real* pVertices (x, y, z order x numVertices)
        dest->vertexDeclaration->addElement(bindIdx, 0, VET_FLOAT3, VES_POSITION);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            HardwareBuffer::HBU_STATIC);
        pReal = static_cast<Real*>(
            vbuf->lock(0, vbuf->getSizeInBytes(), HardwareBuffer::HBL_DISCARD));
        readReals(chunk, pReal, dest->vertexCount * 3);
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
        ++bindIdx;

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
                    dest->vertexDeclaration->addElement(bindIdx, 0, VET_FLOAT3, VES_NORMAL);
                    vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
                        dest->vertexDeclaration->getVertexSize(bindIdx),
                        dest->vertexCount,
                        HardwareBuffer::HBU_STATIC);
                    pReal = static_cast<Real*>(
                        vbuf->lock(0, vbuf->getSizeInBytes(), HardwareBuffer::HBL_DISCARD));
                    readReals(chunk, pReal, dest->vertexCount * 3);
                    vbuf->unlock();
                    dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
                    ++bindIdx;
                    break;
                case M_GEOMETRY_COLOURS:
                    // unsigned long* pColours (RGBA 8888 format x numVertices)
                    dest->vertexDeclaration->addElement(bindIdx, 0, VET_COLOUR, VES_DIFFUSE);
                    vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
                        dest->vertexDeclaration->getVertexSize(bindIdx),
                        dest->vertexCount,
                        HardwareBuffer::HBU_STATIC);
                    pRGBA = static_cast<RGBA*>(
                        vbuf->lock(0, vbuf->getSizeInBytes(), HardwareBuffer::HBL_DISCARD));
                    readLongs(chunk, pRGBA, dest->vertexCount);
                    vbuf->unlock();
                    dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
                    ++bindIdx;
                    break;
                case M_GEOMETRY_TEXCOORDS:
                    // unsigned short dimensions    (1 for 1D, 2 for 2D, 3 for 3D)
                    unsigned short dim;
                    readShorts(chunk, &dim, 1);
                    // Real* pTexCoords  (u [v] [w] order, dimensions x numVertices)
                    dest->vertexDeclaration->addElement(
                        bindIdx, 
                        0, 
                        VertexElement::multiplyTypeCount(VET_FLOAT1, dim), 
                        VES_TEXTURE_COORDINATES,
                        texCoordSet);
                    vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
                        dest->vertexDeclaration->getVertexSize(bindIdx),
                        dest->vertexCount,
                        HardwareBuffer::HBU_STATIC);
                    pReal = static_cast<Real*>(
                        vbuf->lock(0, vbuf->getSizeInBytes(), HardwareBuffer::HBL_DISCARD));
                    readReals(chunk, pReal, dest->vertexCount * dim);
                    vbuf->unlock();
                    dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
                    ++texCoordSet;
                    ++bindIdx;
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
        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSkeletonLink(const String& skelName)
    {
        writeChunkHeader(M_MESH_SKELETON_LINK, calcSkeletonLinkSize(skelName));

        writeString(skelName);

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readSkeletonLink(DataChunk &chunk)
    {
        String skelName = readString(chunk);
        mpMesh->setSkeletonName(skelName);
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcSkeletonLinkSize(const String& skelName)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        size += (unsigned long)skelName.length() + 1;

        return size;

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeMeshBoneAssignment(const VertexBoneAssignment* assign)
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
    void MeshSerializerImpl::writeSubMeshBoneAssignment(const VertexBoneAssignment* assign)
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
    void MeshSerializerImpl::readMeshBoneAssignment(DataChunk& chunk)
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
    void MeshSerializerImpl::readSubMeshBoneAssignment(DataChunk& chunk, SubMesh* sub)
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
    unsigned long MeshSerializerImpl::calcBoneAssignmentSize(void)
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
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeLodInfo(const Mesh* pMesh)
    {
        unsigned short numLods = pMesh->getNumLodLevels();
        bool manual = pMesh->isLodManual();
        writeLodSummary(numLods, manual);

		// Loop from LOD 1 (not 0, this is full detail)
        for (unsigned short i = 1; i < numLods; ++i)
        {
			const Mesh::MeshLodUsage& usage = pMesh->getLodLevel(i);
			if (manual)
			{
				writeLodUsageManual(usage);
			}
			else
			{
				writeLodUsageGenerated(pMesh, usage, i);
			}
            
        }
        

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeLodSummary(unsigned short numLevels, bool manual)
    {
        // Header
        unsigned long size = CHUNK_OVERHEAD_SIZE;
        // unsigned short numLevels;
        size += sizeof(unsigned short);
        // bool manual;  (true for manual alternate meshes, false for generated)
        size += sizeof(bool);
        writeChunkHeader(M_MESH_LOD, size);

        // Details
        // unsigned short numLevels;
        writeShorts(&numLevels, 1);
        // bool manual;  (true for manual alternate meshes, false for generated)
        writeBools(&manual, 1);

        
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeLodUsageManual(const Mesh::MeshLodUsage& usage)
    {
        // Header
        unsigned long size = CHUNK_OVERHEAD_SIZE;
        unsigned long manualSize = CHUNK_OVERHEAD_SIZE;
        // Real fromDepthSquared;
        size += sizeof(Real);
        // Manual part size

        // String manualMeshName;
        manualSize += static_cast<unsigned long>(usage.manualName.length() + 1);

        size += manualSize;

        writeChunkHeader(M_MESH_LOD_USAGE, size);
        writeReals(&(usage.fromDepthSquared), 1);

        writeChunkHeader(M_MESH_LOD_MANUAL, manualSize);
        writeString(usage.manualName);
        

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeLodUsageGenerated(const Mesh* pMesh, const Mesh::MeshLodUsage& usage,
		unsigned short lodNum)
    {
		/* TODO
		// Usage Header
        unsigned long size = CHUNK_OVERHEAD_SIZE;
		unsigned short subidx;

        // Real fromDepthSquared;
        size += sizeof(Real);

        // Calc generated SubMesh sections size
		for(subidx = 0; subidx < pMesh->getNumSubMeshes(); ++subidx)
		{
			// header
			size += CHUNK_OVERHEAD_SIZE;
			// unsigned short numFaces;
			size += sizeof(unsigned short);
			SubMesh* sm = pMesh->getSubMesh(subidx);
			// unsigned short* faceIndexes;  ((v1, v2, v3) * numFaces)
			size += sizeof(unsigned short) * 
				sm->mLodFaceList[lodNum - 1]->indexCount;

		}

        writeChunkHeader(M_MESH_LOD_USAGE, size);
        writeReals(&(usage.fromDepthSquared), 1);

		// Now write sections
        // Calc generated SubMesh sections size
		for(subidx = 0; subidx < pMesh->getNumSubMeshes(); ++subidx)
		{
			size = CHUNK_OVERHEAD_SIZE;
			// unsigned short numFaces;
			size += sizeof(unsigned short);
			SubMesh* sm = pMesh->getSubMesh(subidx);
			// unsigned short* faceIndexes;  ((v1, v2, v3) * numFaces)
			size = sizeof(unsigned short) * 
				sm->mLodFaceList[lodNum - 1]->indexCount;

			writeChunkHeader(M_MESH_LOD_GENERATED, size);
			unsigned short numFaces = sm->mLodFaceList[lodNum - 1]->indexCount / 3;
			writeShorts(&numFaces, 1);
			writeShorts(sm->mLodFaceList[lodNum - 1]->indexCount, 
				sm->mLodFaceList[lodNum - 1]->indexCount);
		}
		*/

    }
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readMeshLodInfo(DataChunk& chunk)
	{
		unsigned short chunkID, i;

        // unsigned short numLevels;
		readShorts(chunk, &(mpMesh->mNumLods), 1);
        // bool manual;  (true for manual alternate meshes, false for generated)
		readBools(chunk, &(mpMesh->mIsLodManual), 1);

		// Preallocate submesh lod face data if not manual
		if (!mpMesh->mIsLodManual)
		{
			unsigned short numsubs = mpMesh->getNumSubMeshes();
			for (i = 0; i < numsubs; ++i)
			{
				SubMesh* sm = mpMesh->getSubMesh(i);
				sm->mLodFaceList.resize(mpMesh->mNumLods-1);
			}
		}

		// Loop from 1 rather than 0 (full detail index is not in file)
		for (i = 1; i < mpMesh->mNumLods; ++i)
		{
			chunkID = readChunk(chunk);
			if (chunkID != M_MESH_LOD_USAGE)
			{
				Except(Exception::ERR_ITEM_NOT_FOUND, 
					"Missing M_MESH_LOD_USAGE chunk in " + mpMesh->getName(), 
					"MeshSerializerImpl::readMeshLodInfo");
			}
			// Read depth
			Mesh::MeshLodUsage usage;
			readReals(chunk, &(usage.fromDepthSquared), 1);

			if (mpMesh->isLodManual())
			{
				readMeshLodUsageManual(chunk, i, usage);
			}
			else //(!mpMesh->isLodManual)
			{
				readMeshLodUsageGenerated(chunk, i, usage);
			}

			// Save usage
			mpMesh->mMeshLodUsageList.push_back(usage);
		}


	}
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readMeshLodUsageManual(DataChunk& chunk, 
		unsigned short lodNum, Mesh::MeshLodUsage& usage)
	{
		unsigned long chunkID;
		// Read detail chunk
		chunkID = readChunk(chunk);
		if (chunkID != M_MESH_LOD_MANUAL)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Missing M_MESH_LOD_MANUAL chunk in " + mpMesh->getName(),
				"MeshSerializerImpl::readMeshLodUsageManual");
		}

		usage.manualName = readString(chunk);
		usage.manualMesh = NULL; // will trigger load later
	}
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readMeshLodUsageGenerated(DataChunk& chunk, 
		unsigned short lodNum, Mesh::MeshLodUsage& usage)
	{
		/* TODO
		usage.manualName = "";
		usage.manualMesh = 0;

		// Get one set of detail per SubMesh
		unsigned short numSubs, i;
		unsigned long chunkID;
		numSubs = mpMesh->getNumSubMeshes();
		for (i = 0; i < numSubs; ++i)
		{
			chunkID = readChunk(chunk);
			if (chunkID != M_MESH_LOD_GENERATED)
			{
				Except(Exception::ERR_ITEM_NOT_FOUND, 
					"Missing M_MESH_LOD_GENERATED chunk in " + mpMesh->getName(),
					"MeshSerializerImpl::readMeshLodUsageGenerated");
			}

			SubMesh* sm = mpMesh->getSubMesh(i);
			// lodNum - 1 because SubMesh doesn't store full detail LOD
			ProgressiveMesh::LODFaceData& data = sm->mLodFaceList[lodNum - 1];
            unsigned short numFaces;
			readShorts(chunk, &numFaces, 1);
			data.numIndexes = numFaces * 3;
            // unsigned short* faceIndexes;  ((v1, v2, v3) * numFaces)
			data.pIndexes = new unsigned short[data.numIndexes];
			readShorts(chunk, data.pIndexes, data.numIndexes);

		}
		*/
	}
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1::MeshSerializerImpl_v1()
    {
        // Version number
        mVersion = "[MeshSerializer_v1.00]";

    }



}


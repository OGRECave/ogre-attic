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

#include "OgreMeshSerializerImpl.h"
#include "OgreMeshFileFormat.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreException.h"
#include "OgreMaterialManager.h"
#include "OgreLogManager.h"
#include "OgreSkeleton.h"
#include "OgreHardwareBufferManager.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"

namespace Ogre {

    /// Chunk overhead = ID + size
    const unsigned long CHUNK_OVERHEAD_SIZE = sizeof(unsigned short) + sizeof(unsigned long);
    //---------------------------------------------------------------------
    MeshSerializerImpl::MeshSerializerImpl()
    {
        mpMesh = 0;

        // Version number
        mVersion = "[MeshSerializer_v1.30]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl::~MeshSerializerImpl()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::exportMesh(const Mesh* pMesh, const String& filename)
    {
        LogManager::getSingleton().logMessage("MeshSerializer writing mesh data to " + filename + "...");

        // Check that the mesh has it's bounds set
        if (pMesh->getBounds().isNull() || pMesh->getBoundingSphereRadius() == 0.0f)
        {
            Except(Exception::ERR_INVALIDPARAMS, "The Mesh you have supplied does not have its"
                " bounds completely defined. Define them first before exporting.", 
                "MeshSerializerImpl::exportMesh");
        }
        mpfFile = fopen(filename.c_str(), "wb");

        writeFileHeader();
        LogManager::getSingleton().logMessage("File header written.");


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
            case M_MESH:
                readMesh(chunk);
                break;
			}

        }
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeMesh(const Mesh* pMesh)
    {
        // Header
        writeChunkHeader(M_MESH, calcMeshSize(pMesh));

		// bool skeletallyAnimated
		bool skelAnim = pMesh->hasSkeleton();
		writeBools(&skelAnim, 1);

        // Write shared geometry
        if (pMesh->sharedVertexData)
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
        // Write bounds information
        LogManager::getSingleton().logMessage("Exporting bounds information....");
        writeBoundsInfo(pMesh);
        LogManager::getSingleton().logMessage("Bounds information exported.");

		// Write submesh name table
		LogManager::getSingleton().logMessage("Exporting submesh name table...");
		writeSubMeshNameTable(pMesh);
		LogManager::getSingleton().logMessage("Submesh name table exported.");
		
		// Write edge lists
		if (pMesh->isEdgeListBuilt())
		{
			LogManager::getSingleton().logMessage("Exporting edge lists...");
			writeEdgeList(pMesh);
			LogManager::getSingleton().logMessage("Edge lists exported");
		}

    }
    //---------------------------------------------------------------------
	// Added by DrEvil
	void MeshSerializerImpl::writeSubMeshNameTable(const Mesh* pMesh)
	{
		// Header
		writeChunkHeader(M_SUBMESH_NAME_TABLE, calcSubMeshNameTableSize(pMesh));

		// Loop through and save out the index and names.
		Mesh::SubMeshNameMap::const_iterator it = pMesh->mSubMeshNameMap.begin();

		while(it != pMesh->mSubMeshNameMap.end())
		{
			// Header
			writeChunkHeader(M_SUBMESH_NAME_TABLE_ELEMENT, CHUNK_OVERHEAD_SIZE + 
				sizeof(unsigned short) + (unsigned long)it->first.length() + 1);

			// write the index
			writeShorts(&it->second, 1);
			// name
	        writeString(it->first);

			++it;
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
        void* pIdx = ibuf->lock(HardwareBuffer::HBL_READ_ONLY);
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
        
        // Operation type
        writeSubMeshOperation(s);

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
    void MeshSerializerImpl::writeSubMeshOperation(const SubMesh* sm)
    {
        // Header
        writeChunkHeader(M_SUBMESH_OPERATION, calcSubMeshOperationSize(sm));

        // unsigned short operationType
        unsigned short opType = static_cast<unsigned short>(sm->operationType);
        writeShorts(&opType, 1);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeGeometry(const VertexData* vertexData)
    {
		// calc size
        const VertexDeclaration::VertexElementList& elemList = 
            vertexData->vertexDeclaration->getElements();
        const VertexBufferBinding::VertexBufferBindingMap& bindings = 
            vertexData->vertexBufferBinding->getBindings();
        VertexBufferBinding::VertexBufferBindingMap::const_iterator vbi, vbiend;

		size_t size = CHUNK_OVERHEAD_SIZE + sizeof(unsigned int) + // base
			(CHUNK_OVERHEAD_SIZE + elemList.size() * (CHUNK_OVERHEAD_SIZE + sizeof(unsigned short) * 5)); // elements
        vbiend = bindings.end();
		for (vbi = bindings.begin(); vbi != vbiend; ++vbi)
		{
			const HardwareVertexBufferSharedPtr& vbuf = vbi->second;
			size += (CHUNK_OVERHEAD_SIZE * 2) + (sizeof(unsigned short) * 2) + vbuf->getSizeInBytes();
		}

		// Header
        writeChunkHeader(M_GEOMETRY, size);

        // unsigned int numVertices
        writeInts(&vertexData->vertexCount, 1);

		// Vertex declaration
		size = CHUNK_OVERHEAD_SIZE + elemList.size() * (CHUNK_OVERHEAD_SIZE + sizeof(unsigned short) * 5);
		writeChunkHeader(M_GEOMETRY_VERTEX_DECLARATION, size);
		
        VertexDeclaration::VertexElementList::const_iterator vei, veiend;
		veiend = elemList.end();
		unsigned short tmp;
		size = CHUNK_OVERHEAD_SIZE + sizeof(unsigned short) * 5;
		for (vei = elemList.begin(); vei != veiend; ++vei)
		{
			const VertexElement& elem = *vei;
			writeChunkHeader(M_GEOMETRY_VERTEX_ELEMENT, size);
			// unsigned short source;  	// buffer bind source
			tmp = elem.getSource();
			writeShorts(&tmp, 1);
			// unsigned short type;    	// VertexElementType
			tmp = static_cast<unsigned short>(elem.getType());
			writeShorts(&tmp, 1);
			// unsigned short semantic; // VertexElementSemantic
			tmp = static_cast<unsigned short>(elem.getSemantic());
			writeShorts(&tmp, 1);
			// unsigned short offset;	// start offset in buffer in bytes
			tmp = static_cast<unsigned short>(elem.getOffset());
			writeShorts(&tmp, 1);
			// unsigned short index;	// index of the semantic (for colours and texture coords)
			tmp = elem.getIndex();
			writeShorts(&tmp, 1);

		}

		// Buffers and bindings
		vbiend = bindings.end();
		for (vbi = bindings.begin(); vbi != vbiend; ++vbi)
		{
			const HardwareVertexBufferSharedPtr& vbuf = vbi->second;
			size = (CHUNK_OVERHEAD_SIZE * 2) + (sizeof(unsigned short) * 2) + vbuf->getSizeInBytes();
			writeChunkHeader(M_GEOMETRY_VERTEX_BUFFER,  size);
			// unsigned short bindIndex;	// Index to bind this buffer to
			tmp = vbi->first;
			writeShorts(&tmp, 1);
			// unsigned short vertexSize;	// Per-vertex size, must agree with declaration at this index
			tmp = (unsigned short)vbuf->getVertexSize();
			writeShorts(&tmp, 1);
			
			// Data
			size = CHUNK_OVERHEAD_SIZE + vbuf->getSizeInBytes();
			writeChunkHeader(M_GEOMETRY_VERTEX_BUFFER_DATA, size);
			void* pBuf = vbuf->lock(HardwareBuffer::HBL_READ_ONLY);
#		if OGRE_ENDIAN == ENDIAN_BIG
			// endian conversion for OSX
			// Copy data
			unsigned char* tempData = new unsigned char[vbuf->getSizeInBytes()];
			memcpy(tempData, pBuf, vbuf->getSizeInBytes());
			flipToLittleEndian(
				tempData, 
				vertexData->vertexCount,
				vbuf->getVertexSize(),
				vertexData->vertexDeclaration->findElementsBySource(vbi->first));
			writeData(tempData, vbuf->getVertexSize(), vertexData->vertexCount);
			delete [] tempData;
#		else
			writeData(pBuf, vbuf->getVertexSize(), vertexData->vertexCount);
#		endif
		}


    }
    //---------------------------------------------------------------------
	unsigned long MeshSerializerImpl::calcSubMeshNameTableSize(const Mesh *pMesh)
	{
		size_t size = CHUNK_OVERHEAD_SIZE;
		// Figure out the size of the Name table.
		// Iterate through the subMeshList & add up the size of the indexes and names.
		Mesh::SubMeshNameMap::const_iterator it = pMesh->mSubMeshNameMap.begin();		
		while(it != pMesh->mSubMeshNameMap.end())
		{
			// size of the index
			size += sizeof(unsigned short);
			// name
			size += (unsigned long)it->first.length() + 1;

			++it;
		}        		

		// size of the sub-mesh name table.
		return (unsigned long)size;
	}
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcMeshSize(const Mesh* pMesh)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // Num shared vertices
        size += sizeof(unsigned int);

        // Geometry
        if (pMesh->sharedVertexData && pMesh->sharedVertexData->vertexCount > 0)
        {
            size += calcGeometrySize(pMesh->sharedVertexData);
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

		// Submesh name table
		size += calcSubMeshNameTableSize(pMesh);

		// Edge list
		if (pMesh->isEdgeListBuilt())
		{
			size += calcEdgeListSize(pMesh);
		}
		
		return size;

    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcSubMeshSize(const SubMesh* pSub)
    {
        size_t size = CHUNK_OVERHEAD_SIZE;

        // Material name
        size += (unsigned long)pSub->getMaterialName().length() + 1;

        // bool useSharedVertices
        size += sizeof(bool);
        // unsigned int indexCount
        size += sizeof(unsigned int);
        // bool indexes32bit
        size += sizeof(bool);
        // unsigned int* faceVertexIndices 
        size += sizeof(unsigned int) * pSub->indexData->indexCount;

        // Geometry
        if (!pSub->useSharedVertices)
        {
            size += calcGeometrySize(pSub->vertexData);
        }

        return static_cast<unsigned long>(size);
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcSubMeshOperationSize(const SubMesh* pSub)
    {
        return CHUNK_OVERHEAD_SIZE + sizeof(unsigned short);
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcGeometrySize(const VertexData* vertexData)
    {
        size_t size = CHUNK_OVERHEAD_SIZE;

        // Num vertices
        size += sizeof(unsigned int);

        const VertexDeclaration::VertexElementList& elems = 
            vertexData->vertexDeclaration->getElements();

        VertexDeclaration::VertexElementList::const_iterator i, iend;
        iend = elems.end();
        for (i = elems.begin(); i != iend; ++i)
        {
            const VertexElement& elem = *i;
            // Vertex element
            size += VertexElement::getTypeSize(elem.getType()) * vertexData->vertexCount;
        }
        return static_cast<unsigned long>(size);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readGeometry(DataChunk& chunk, VertexData* dest)
    {

        dest->vertexStart = 0;

        // unsigned int numVertices
        readInts(chunk, &dest->vertexCount, 1);

        // Find optional geometry chunks
        if (!chunk.isEOF())
        {
            unsigned short chunkID = readChunk(chunk);
            while(!chunk.isEOF() && 
                (chunkID == M_GEOMETRY_VERTEX_DECLARATION || 
                 chunkID == M_GEOMETRY_VERTEX_BUFFER ))
            {
                switch (chunkID)
                {
                case M_GEOMETRY_VERTEX_DECLARATION:
                    readGeometryVertexDeclaration(chunk, dest);
                    break;
                case M_GEOMETRY_VERTEX_BUFFER:
                    readGeometryVertexBuffer(chunk, dest);
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
    void MeshSerializerImpl::readGeometryVertexDeclaration(DataChunk& chunk, VertexData* dest)
    {
        // Find optional geometry chunks
        if (!chunk.isEOF())
        {
            unsigned short chunkID = readChunk(chunk);
            while(!chunk.isEOF() && 
                (chunkID == M_GEOMETRY_VERTEX_ELEMENT ))
            {
                switch (chunkID)
                {
                case M_GEOMETRY_VERTEX_ELEMENT:
                    readGeometryVertexElement(chunk, dest);
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
    void MeshSerializerImpl::readGeometryVertexElement(DataChunk& chunk, VertexData* dest)
    {
		unsigned short source, offset, index, tmp;
		VertexElementType vType;
		VertexElementSemantic vSemantic;
		// unsigned short source;  	// buffer bind source
		readShorts(chunk, &source, 1);
		// unsigned short type;    	// VertexElementType
		readShorts(chunk, &tmp, 1);
		vType = static_cast<VertexElementType>(tmp);
		// unsigned short semantic; // VertexElementSemantic
		readShorts(chunk, &tmp, 1);
		vSemantic = static_cast<VertexElementSemantic>(tmp);
		// unsigned short offset;	// start offset in buffer in bytes
		readShorts(chunk, &offset, 1);
		// unsigned short index;	// index of the semantic
		readShorts(chunk, &index, 1);

		dest->vertexDeclaration->addElement(source, offset, vType, vSemantic, index);

	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readGeometryVertexBuffer(DataChunk& chunk, VertexData* dest)
    {
		unsigned short bindIndex, vertexSize;
		// unsigned short bindIndex;	// Index to bind this buffer to
		readShorts(chunk, &bindIndex, 1);
		// unsigned short vertexSize;	// Per-vertex size, must agree with declaration at this index
		readShorts(chunk, &vertexSize, 1);

		// Check for vertex data header
		unsigned short headerID;
		headerID = readChunk(chunk);
		if (headerID != M_GEOMETRY_VERTEX_BUFFER_DATA)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, "Can't find vertex buffer data area",
            	"MeshSerializerImpl::readGeometryVertexBuffer");
		}
		// Check that vertex size agrees
		if (dest->vertexDeclaration->getVertexSize(bindIndex) != vertexSize)
		{
			Except(Exception::ERR_INTERNAL_ERROR, "Buffer vertex size does not agree with vertex declaration",
            	"MeshSerializerImpl::readGeometryVertexBuffer");
		}
		
		// Create / populate vertex buffer
		HardwareVertexBufferSharedPtr vbuf;
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            vertexSize,
            dest->vertexCount,
            mpMesh->mVertexBufferUsage, 
			mpMesh->mVertexBufferShadowBuffer);
        void* pBuf = vbuf->lock(HardwareBuffer::HBL_DISCARD);
        chunk.read(pBuf, dest->vertexCount * vertexSize);

		// endian conversion for OSX
		flipFromLittleEndian(
			pBuf, 
			dest->vertexCount, 
			vertexSize,
			dest->vertexDeclaration->findElementsBySource(bindIndex));
        vbuf->unlock();
		
		// Set binding
        dest->vertexBufferBinding->setBinding(bindIndex, vbuf);

	}
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readSubMeshNameTable(DataChunk& chunk)
	{
		// The map for
		std::map<unsigned short, String> subMeshNames;
		unsigned short chunkID, subMeshIndex;

		// Need something to store the index, and the objects name
		// This table is a method that imported meshes can retain their naming
		// so that the names established in the modelling software can be used
		// to get the sub-meshes by name. The exporter must support exporting
		// the optional chunk M_SUBMESH_NAME_TABLE.

        // Read in all the sub-chunks. Each sub-chunk should contain an index and Ogre::String for the name.
		if (!chunk.isEOF())
		{
			chunkID = readChunk(chunk);
			while(!chunk.isEOF() && (chunkID == M_SUBMESH_NAME_TABLE_ELEMENT ))
			{
				// Read in the index of the submesh.
				readShorts(chunk, &subMeshIndex, 1);
				// Read in the String and map it to its index.
				subMeshNames[subMeshIndex] = readString(chunk);					

				// If we're not end of file get the next chunk ID
				if (!chunk.isEOF())
					chunkID = readChunk(chunk);
			}
			if (!chunk.isEOF())
			{
				// Backpedal back to start of chunk
				chunk.skip(-(long)CHUNK_OVERHEAD_SIZE);
			}
		}

		// Set all the submeshes names
		// ?

		// Loop through and save out the index and names.
		std::map<unsigned short, String>::const_iterator it = subMeshNames.begin();

		while(it != subMeshNames.end())
		{			
			// Name this submesh to the stored name.
			mpMesh->nameSubMesh(it->second, it->first);
			++it;
		}



	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readMesh(DataChunk& chunk)
    {
        unsigned short chunkID;

        // Never automatically build edge lists for this version
        // expect them in the file or not at all
        mpMesh->mAutoBuildEdgeLists = false;

		// bool skeletallyAnimated
		readBools(chunk, &mIsSkeletallyAnimated, 1);

        // Find all subchunks 
        if (!chunk.isEOF())
        {
            chunkID = readChunk(chunk);
            while(!chunk.isEOF() &&
                (chunkID == M_GEOMETRY ||
				 chunkID == M_SUBMESH ||
                 chunkID == M_MESH_SKELETON_LINK ||
                 chunkID == M_MESH_BONE_ASSIGNMENT ||
				 chunkID == M_MESH_LOD ||
                 chunkID == M_MESH_BOUNDS ||
				 chunkID == M_SUBMESH_NAME_TABLE ||
				 chunkID == M_EDGE_LISTS))
            {
                switch(chunkID)
                {
				case M_GEOMETRY:
					mpMesh->sharedVertexData = new VertexData();
					try {
						readGeometry(chunk, mpMesh->sharedVertexData);
					}
					catch (Exception& e)
					{
						if (e.getNumber() == Exception::ERR_ITEM_NOT_FOUND)
						{
							// duff geometry data entry with 0 vertices
							delete mpMesh->sharedVertexData;
							mpMesh->sharedVertexData = 0;
							// Skip this chunk (pointer will have been returned to just after header)
							chunk.skip(mCurrentChunkLen - CHUNK_OVERHEAD_SIZE);
						}
						else
						{
							throw;
						}
					}
					break;
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
                case M_MESH_BOUNDS:
                    readBoundsInfo(chunk);
                    break;
				case M_SUBMESH_NAME_TABLE:
    	            readSubMeshNameTable(chunk);
					break;
                case M_EDGE_LISTS:
                    readEdgeList(chunk);
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
                    mpMesh->mIndexBufferUsage,
					mpMesh->mIndexBufferShadowBuffer);
            // unsigned int* faceVertexIndices 
            unsigned int* pIdx = static_cast<unsigned int*>(
                ibuf->lock(HardwareBuffer::HBL_DISCARD)
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
                    mpMesh->mIndexBufferUsage,
					mpMesh->mIndexBufferShadowBuffer);
            // unsigned short* faceVertexIndices 
            unsigned short* pIdx = static_cast<unsigned short*>(
                ibuf->lock(HardwareBuffer::HBL_DISCARD)
                );
            readShorts(chunk, pIdx, sm->indexData->indexCount);
            ibuf->unlock();
        }
        sm->indexData->indexBuffer = ibuf;

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
                (chunkID == M_SUBMESH_BONE_ASSIGNMENT ||
                 chunkID == M_SUBMESH_OPERATION))
            {
                switch(chunkID)
                {
                case M_SUBMESH_OPERATION:
                    readSubMeshOperation(chunk, sm);
                    break;
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
    void MeshSerializerImpl::readSubMeshOperation(DataChunk& chunk, SubMesh* sm)
    {
        // unsigned short operationType
        unsigned short opType;
        readShorts(chunk, &opType, 1);
        sm->operationType = static_cast<RenderOperation::OperationType>(opType);
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
    void MeshSerializerImpl::readTextureLayer(DataChunk& chunk, MaterialPtr& pMat)
    {
        // Material definition section phased out of 1.1
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

        // unsigned int vertexIndex;
        writeInts(&(assign->vertexIndex), 1);
        // unsigned short boneIndex;
        writeShorts(&(assign->boneIndex), 1);
        // Real weight;
        writeReals(&(assign->weight), 1);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeSubMeshBoneAssignment(const VertexBoneAssignment* assign)
    {
        writeChunkHeader(M_SUBMESH_BONE_ASSIGNMENT, calcBoneAssignmentSize());

        // unsigned int vertexIndex;
        writeInts(&(assign->vertexIndex), 1);
        // unsigned short boneIndex;
        writeShorts(&(assign->boneIndex), 1);
        // Real weight;
        writeReals(&(assign->weight), 1);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readMeshBoneAssignment(DataChunk& chunk)
    {
        VertexBoneAssignment assign;

        // unsigned int vertexIndex;
        readInts(chunk, &(assign.vertexIndex),1);
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

        // unsigned int vertexIndex;
        readInts(chunk, &(assign.vertexIndex),1);
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
        size += sizeof(unsigned int);
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
			// unsigned int numFaces;
			size += sizeof(unsigned int);
			SubMesh* sm = pMesh->getSubMesh(subidx);
            const IndexData* indexData = sm->mLodFaceList[lodNum - 1];

            // bool indexes32Bit
			size += sizeof(bool);
			// unsigned short*/int* faceIndexes;  
            if (indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT)
            {
			    size += static_cast<unsigned long>(
                    sizeof(unsigned int) * indexData->indexCount);
            }
            else
            {
			    size += static_cast<unsigned long>(
                    sizeof(unsigned short) * indexData->indexCount);
            }

		}

        writeChunkHeader(M_MESH_LOD_USAGE, size);
        writeReals(&(usage.fromDepthSquared), 1);

		// Now write sections
        // Calc generated SubMesh sections size
		for(subidx = 0; subidx < pMesh->getNumSubMeshes(); ++subidx)
		{
			size = CHUNK_OVERHEAD_SIZE;
			// unsigned int numFaces;
			size += sizeof(unsigned int);
			SubMesh* sm = pMesh->getSubMesh(subidx);
            const IndexData* indexData = sm->mLodFaceList[lodNum - 1];
            // bool indexes32Bit
			size += sizeof(bool);
			// unsigned short*/int* faceIndexes;  
            if (indexData->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT)
            {
			    size += static_cast<unsigned long>(
                    sizeof(unsigned int) * indexData->indexCount);
            }
            else
            {
			    size += static_cast<unsigned long>(
                    sizeof(unsigned short) * indexData->indexCount);
            }

			writeChunkHeader(M_MESH_LOD_GENERATED, size);
			unsigned int idxCount = static_cast<unsigned int>(indexData->indexCount);
			writeInts(&idxCount, 1);
            // Lock index buffer to write
            HardwareIndexBufferSharedPtr ibuf = indexData->indexBuffer;
			// bool indexes32bit
			bool idx32 = (ibuf->getType() == HardwareIndexBuffer::IT_32BIT);
			writeBools(&idx32, 1);
            if (idx32)
            {
                unsigned int* pIdx = static_cast<unsigned int*>(
                    ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
			    writeInts(pIdx, indexData->indexCount);
                ibuf->unlock();
            }
            else
            {
                unsigned short* pIdx = static_cast<unsigned short*>(
                    ibuf->lock(HardwareBuffer::HBL_READ_ONLY));
			    writeShorts(pIdx, indexData->indexCount);
                ibuf->unlock();
            }
		}
	

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::writeBoundsInfo(const Mesh* pMesh)
    {
		// Usage Header
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        size += sizeof(Real) * 7;
        writeChunkHeader(M_MESH_BOUNDS, size);

        // Real minx, miny, minz
        const Vector3& min = pMesh->mAABB.getMinimum();
        const Vector3& max = pMesh->mAABB.getMaximum();
        writeReals(&min.x, 1);
        writeReals(&min.y, 1);
        writeReals(&min.z, 1);
        // Real maxx, maxy, maxz
        writeReals(&max.x, 1);
        writeReals(&max.y, 1);
        writeReals(&max.z, 1);
        // Real radius
        writeReals(&pMesh->mBoundRadius, 1);

    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::readBoundsInfo(DataChunk& chunk)
    {
        Vector3 min, max;
        // Real minx, miny, minz
        readReals(chunk, &min.x, 1);
        readReals(chunk, &min.y, 1);
        readReals(chunk, &min.z, 1);
        // Real maxx, maxy, maxz
        readReals(chunk, &max.x, 1);
        readReals(chunk, &max.y, 1);
        readReals(chunk, &max.z, 1);
        AxisAlignedBox box(min, max);
        mpMesh->_setBounds(box, true);
        // Real radius
        Real radius;
        readReals(chunk, &radius, 1);
        mpMesh->_setBoundingSphereRadius(radius);



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
            usage.edgeData = NULL;

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
            sm->mLodFaceList[lodNum - 1] = new IndexData();
			IndexData* indexData = sm->mLodFaceList[lodNum - 1];
            // unsigned int numIndexes
            unsigned int numIndexes;
			readInts(chunk, &numIndexes, 1);
            indexData->indexCount = static_cast<size_t>(numIndexes);
            // bool indexes32Bit
            bool idx32Bit;
            readBools(chunk, &idx32Bit, 1);
            // unsigned short*/int* faceIndexes;  ((v1, v2, v3) * numFaces)
            if (idx32Bit)
            {
                indexData->indexBuffer = HardwareBufferManager::getSingleton().
                    createIndexBuffer(HardwareIndexBuffer::IT_32BIT, indexData->indexCount,
                    mpMesh->mIndexBufferUsage, mpMesh->mIndexBufferShadowBuffer);
                unsigned int* pIdx = static_cast<unsigned int*>(
                    indexData->indexBuffer->lock(
                        0, 
                        indexData->indexBuffer->getSizeInBytes(), 
                        HardwareBuffer::HBL_DISCARD) );

			    readInts(chunk, pIdx, indexData->indexCount);
                indexData->indexBuffer->unlock();

            }
            else
            {
                indexData->indexBuffer = HardwareBufferManager::getSingleton().
                    createIndexBuffer(HardwareIndexBuffer::IT_16BIT, indexData->indexCount,
                    mpMesh->mIndexBufferUsage, mpMesh->mIndexBufferShadowBuffer);
                unsigned short* pIdx = static_cast<unsigned short*>(
                    indexData->indexBuffer->lock(
                        0, 
                        indexData->indexBuffer->getSizeInBytes(), 
                        HardwareBuffer::HBL_DISCARD) );
			    readShorts(chunk, pIdx, indexData->indexCount);
                indexData->indexBuffer->unlock();

            }

		}
	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::flipFromLittleEndian(void* pData, size_t vertexCount, 
        size_t vertexSize, const VertexDeclaration::VertexElementList& elems)
	{
#	if OGRE_ENDIAN == ENDIAN_BIG
        flipEndian(pData, vertexCount, vertexSize, elems);
#	endif	
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl::flipToLittleEndian(void* pData, size_t vertexCount, 
			size_t vertexSize, const VertexDeclaration::VertexElementList& elems)
	{
#	if OGRE_ENDIAN == ENDIAN_BIG
        flipEndian(pData, vertexCount, vertexSize, elems);
#	endif	
	}
    //---------------------------------------------------------------------
    void MeshSerializerImpl::flipEndian(void* pData, size_t vertexCount, 
        size_t vertexSize, const VertexDeclaration::VertexElementList& elems)
	{
		void *pBase = pData;
		for (size_t v = 0; v < vertexCount; ++v)
		{
			VertexDeclaration::VertexElementList::const_iterator ei, eiend;
			eiend = elems.end();
			for (ei = elems.begin(); ei != eiend; ++ei)
			{
				void *pElem;
				// re-base pointer to the element
				(*ei).baseVertexPointerToElement(pBase, &pElem);
				// Flip the endian based on the type
				size_t typeSize = 0;
				switch (VertexElement::getBaseType((*ei).getType()))
				{
					case VET_FLOAT1:
						typeSize = sizeof(Real);
						break;
					case VET_SHORT1:
						typeSize = sizeof(short);
						break;
					case VET_COLOUR:
						typeSize = sizeof(RGBA);
						break;
					case VET_UBYTE4:
						typeSize = 0; // NO FLIPPING
						break;
				};
                Serializer::flipEndian(pElem, typeSize, 
					VertexElement::getTypeCount((*ei).getType()));
				
			}

			pBase = static_cast<void*>(
				static_cast<unsigned char*>(pBase) + vertexSize);
			
		}
	}
    //---------------------------------------------------------------------
	unsigned long MeshSerializerImpl::calcEdgeListSize(const Mesh *pMesh)
	{
        size_t size = CHUNK_OVERHEAD_SIZE;

        for (ushort i = 0; i < pMesh->getNumLodLevels(); ++i)
        {
            
            const EdgeData* edgeData = pMesh->getEdgeList(i);
            bool isManual = pMesh->isLodManual() && (i > 0);

            size += calcEdgeListLodSize(edgeData, isManual);

        }

        return size;
	}
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcEdgeListLodSize(const EdgeData* edgeData, bool isManual)
    {
        size_t size = CHUNK_OVERHEAD_SIZE;

        // unsigned short lodIndex
        size += sizeof(unsigned short);

        // bool isManual			// If manual, no edge data here, loaded from manual mesh
        size += sizeof(bool);
        if (!isManual)
        {
            // unsigned long numTriangles
            size += sizeof(unsigned long);
            // unsigned long numEdgeGroups
            size += sizeof(unsigned long);
            // Triangle* triangleList
            size_t triSize = 0;
            // unsigned long indexSet
            // unsigned long vertexSet
            // unsigned long vertIndex[3]
            // unsigned long sharedVertIndex[3] 
            // Real normal[4] 
            triSize += sizeof(unsigned long) * 8 
                    + sizeof(Real) * 4;

            size += triSize * edgeData->triangles.size();
            // Write the groups
            for (EdgeData::EdgeGroupList::const_iterator gi = edgeData->edgeGroups.begin();
                gi != edgeData->edgeGroups.end(); ++gi)
            {
                const EdgeData::EdgeGroup& edgeGroup = *gi;
                size += calcEdgeGroupSize(edgeGroup);
            }

        }

        return size;
    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializerImpl::calcEdgeGroupSize(const EdgeData::EdgeGroup& group)
    {
        size_t size = CHUNK_OVERHEAD_SIZE;

        // unsigned long vertexSet
        size += sizeof(unsigned long);
        // unsigned long numEdges
        size += sizeof(unsigned long);
        // Edge* edgeList
        size_t edgeSize = 0;
        // unsigned long  triIndex[2]
        // unsigned long  vertIndex[2]
        // unsigned long  sharedVertIndex[2]
        // bool degenerate
        edgeSize += sizeof(unsigned long) * 6 + sizeof(bool);
        size += edgeSize * group.edges.size();

        return size;
    }
    //---------------------------------------------------------------------
	void MeshSerializerImpl::writeEdgeList(const Mesh* pMesh)
	{
        writeChunkHeader(M_EDGE_LISTS, calcEdgeListSize(pMesh));

        for (ushort i = 0; i < pMesh->getNumLodLevels(); ++i)
        {
            const EdgeData* edgeData = pMesh->getEdgeList(i);
            bool isManual = pMesh->isLodManual() && (i > 0);
            writeChunkHeader(M_EDGE_LIST_LOD, calcEdgeListLodSize(edgeData, isManual));

            // unsigned short lodIndex
            writeShorts(&i, 1);

            // bool isManual			// If manual, no edge data here, loaded from manual mesh
            writeBools(&isManual, 1);
            if (!isManual)
            {
                // unsigned long  numTriangles
                unsigned long count = static_cast<unsigned long>(edgeData->triangles.size());
                writeLongs(&count, 1);
                // unsigned long numEdgeGroups
                count = static_cast<unsigned long>(edgeData->edgeGroups.size());
                writeLongs(&count, 1);
                // Triangle* triangleList
                // Iterate rather than writing en-masse to allow endian conversion
                for (EdgeData::TriangleList::const_iterator t = edgeData->triangles.begin();
                    t != edgeData->triangles.end(); ++t)
                {
                    const EdgeData::Triangle& tri = *t;
                    // unsigned long indexSet; 
                    unsigned long tmp[3];
                    tmp[0] = tri.indexSet;
                    writeLongs(tmp, 1);
                    // unsigned long vertexSet;
                    tmp[0] = tri.vertexSet;
                    writeLongs(tmp, 1);
                    // unsigned long vertIndex[3];
                    tmp[0] = tri.vertIndex[0];
                    tmp[1] = tri.vertIndex[1];
                    tmp[2] = tri.vertIndex[2];
                    writeLongs(tmp, 3);
                    // unsigned long sharedVertIndex[3]; 
                    tmp[0] = tri.sharedVertIndex[0];
                    tmp[1] = tri.sharedVertIndex[1];
                    tmp[2] = tri.sharedVertIndex[2];
                    writeLongs(tmp, 3);
                    // Real normal[4];   
                    writeReals(&(tri.normal.x), 4);

                }
                // Write the groups
                for (EdgeData::EdgeGroupList::const_iterator gi = edgeData->edgeGroups.begin();
                    gi != edgeData->edgeGroups.end(); ++gi)
                {
                    const EdgeData::EdgeGroup& edgeGroup = *gi;
                    writeChunkHeader(M_EDGE_GROUP, calcEdgeGroupSize(edgeGroup));
                    // unsigned long vertexSet
                    unsigned long vertexSet = static_cast<unsigned long>(edgeGroup.vertexSet);
                    writeLongs(&vertexSet, 1);
                    // unsigned long numEdges
                    count = static_cast<unsigned long>(edgeGroup.edges.size());
                    writeLongs(&count, 1);
                    // Edge* edgeList
                    // Iterate rather than writing en-masse to allow endian conversion
                    for (EdgeData::EdgeList::const_iterator ei = edgeGroup.edges.begin();
                        ei != edgeGroup.edges.end(); ++ei)
                    {
                        const EdgeData::Edge& edge = *ei;
                        unsigned long tmp[2];
                        // unsigned long  triIndex[2]
                        tmp[0] = edge.triIndex[0];
                        tmp[1] = edge.triIndex[1];
                        writeLongs(tmp, 2);
                        // unsigned long  vertIndex[2]
                        tmp[0] = edge.vertIndex[0];
                        tmp[1] = edge.vertIndex[1];
                        writeLongs(tmp, 2);
                        // unsigned long  sharedVertIndex[2]
                        tmp[0] = edge.sharedVertIndex[0];
                        tmp[1] = edge.sharedVertIndex[1];
                        writeLongs(tmp, 2);
                        // bool degenerate
                        writeBools(&(edge.degenerate), 1);
                    }

                }

            }

        }
	}
    //---------------------------------------------------------------------
	void MeshSerializerImpl::readEdgeList(DataChunk& chunk)
	{
        unsigned short chunkID;

        if (!chunk.isEOF())
        {
            chunkID = readChunk(chunk);
            while(!chunk.isEOF() &&
                chunkID == M_EDGE_LIST_LOD)
            {
                // Process single LOD

                // unsigned short lodIndex
                unsigned short lodIndex;
                readShorts(chunk, &lodIndex, 1);

                // bool isManual			// If manual, no edge data here, loaded from manual mesh
                bool isManual;
                readBools(chunk, &isManual, 1);
                // Only load in non-manual levels; others will be connected up by Mesh on demand
                if (!isManual)
                {
                    Mesh::MeshLodUsage& usage = const_cast<Mesh::MeshLodUsage&>(mpMesh->getLodLevel(lodIndex));

                    usage.edgeData = new EdgeData();
                    // unsigned long numTriangles
                    unsigned long numTriangles;
                    readLongs(chunk, &numTriangles, 1);
                    // Allocate correct amount of memory
                    usage.edgeData->triangles.resize(numTriangles);
                    // unsigned long numEdgeGroups
                    unsigned long numEdgeGroups;
                    readLongs(chunk, &numEdgeGroups, 1);
                    // Allocate correct amount of memory
                    usage.edgeData->edgeGroups.resize(numEdgeGroups);
                    // Triangle* triangleList
                    unsigned long tmp[3];
                    for (size_t t = 0; t < numTriangles; ++t)
                    {
                        EdgeData::Triangle& tri = usage.edgeData->triangles[t];
                        // unsigned long indexSet
                        readLongs(chunk, tmp, 1);
                        tri.indexSet = tmp[0];
                        // unsigned long vertexSet
                        readLongs(chunk, tmp, 1);
                        tri.vertexSet = tmp[0];
                        // unsigned long vertIndex[3]
                        readLongs(chunk, tmp, 3);
                        tri.vertIndex[0] = tmp[0];
                        tri.vertIndex[1] = tmp[1];
                        tri.vertIndex[2] = tmp[2];
                        // unsigned long sharedVertIndex[3] 
                        readLongs(chunk, tmp, 3);
                        tri.sharedVertIndex[0] = tmp[0];
                        tri.sharedVertIndex[1] = tmp[1];
                        tri.sharedVertIndex[2] = tmp[2];
                        // Real normal[4] 
                        readReals(chunk, &(tri.normal.x), 4);

                    }

                    for (unsigned long eg = 0; eg < numEdgeGroups; ++eg)
                    {
                        chunkID = readChunk(chunk);
                        if (chunkID != M_EDGE_GROUP)
                        {
                            Except(Exception::ERR_INTERNAL_ERROR, 
                                "Missing M_EDGE_GROUP chunk", 
                                "MeshSerializerImpl::readEdgeList");
                        }
                        EdgeData::EdgeGroup& edgeGroup = usage.edgeData->edgeGroups[eg];

                        // unsigned long vertexSet
                        readLongs(chunk, tmp, 1);
                        edgeGroup.vertexSet = tmp[0];
                        // unsigned long numEdges
                        unsigned long numEdges;
                        readLongs(chunk, &numEdges, 1);
                        edgeGroup.edges.resize(numEdges);
                        // Edge* edgeList
                        for (unsigned long e = 0; e < numEdges; ++e)
                        {
                            EdgeData::Edge& edge = edgeGroup.edges[e];
                            // unsigned long  triIndex[2]
                            readLongs(chunk, tmp, 2);
                            edge.triIndex[0] = tmp[0];
                            edge.triIndex[1] = tmp[1];
                            // unsigned long  vertIndex[2]
                            readLongs(chunk, tmp, 2);
                            edge.vertIndex[0] = tmp[0];
                            edge.vertIndex[1] = tmp[1];
                            // unsigned long  sharedVertIndex[2]
                            readLongs(chunk, tmp, 2);
                            edge.sharedVertIndex[0] = tmp[0];
                            edge.sharedVertIndex[1] = tmp[1];
                            // bool degenerate
                            readBools(chunk, &(edge.degenerate), 1);
                        }
                        // Populate edgeGroup.vertexData pointers
                        // If there is shared vertex data, vertexSet 0 is that, 
                        // otherwise 0 is first dedicated
                        if (mpMesh->sharedVertexData)
                        {
                            if (edgeGroup.vertexSet == 0)
                            {
                                edgeGroup.vertexData = mpMesh->sharedVertexData;
                            }
                            else
                            {
                                edgeGroup.vertexData = mpMesh->getSubMesh(
                                    edgeGroup.vertexSet-1)->vertexData;
                            }
                        }
                        else
                        {
                            edgeGroup.vertexData = mpMesh->getSubMesh(
                                edgeGroup.vertexSet)->vertexData;
                        }
                    }
                    
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



        mpMesh->mEdgeListsBuilt = true;
	}
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_2::MeshSerializerImpl_v1_2()
    {
        // Version number
        mVersion = "[MeshSerializer_v1.20]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_2::~MeshSerializerImpl_v1_2()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readMesh(DataChunk& chunk)
    {
        MeshSerializerImpl::readMesh(chunk);
        // Always automatically build edge lists for this version
        mpMesh->mAutoBuildEdgeLists = true;
        
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometry(DataChunk& chunk, VertexData* dest)
    {
        unsigned short texCoordSet = 0;
        
        unsigned short bindIdx = 0;

        dest->vertexStart = 0;

        // unsigned int numVertices
        readInts(chunk, &dest->vertexCount, 1);

        // Vertex buffers

        readGeometryPositions(bindIdx, chunk, dest);
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
                    readGeometryNormals(bindIdx++, chunk, dest);
                    break;
                case M_GEOMETRY_COLOURS:
                    readGeometryColours(bindIdx++, chunk, dest);
                    break;
                case M_GEOMETRY_TEXCOORDS:
                    readGeometryTexCoords(bindIdx++, chunk, dest, texCoordSet++);
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
    void MeshSerializerImpl_v1_2::readGeometryPositions(unsigned short bindIdx, 
        DataChunk& chunk, VertexData* dest)
    {
        Real *pReal = 0;
        HardwareVertexBufferSharedPtr vbuf;
        // Real* pVertices (x, y, z order x numVertices)
        dest->vertexDeclaration->addElement(bindIdx, 0, VET_FLOAT3, VES_POSITION);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            mpMesh->mVertexBufferUsage, 
			mpMesh->mIndexBufferShadowBuffer);
        pReal = static_cast<Real*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readReals(chunk, pReal, dest->vertexCount * 3);
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometryNormals(unsigned short bindIdx, 
        DataChunk& chunk, VertexData* dest)
    {
        Real *pReal = 0;
        HardwareVertexBufferSharedPtr vbuf;
        // Real* pNormals (x, y, z order x numVertices)
        dest->vertexDeclaration->addElement(bindIdx, 0, VET_FLOAT3, VES_NORMAL);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            mpMesh->mVertexBufferUsage,
			mpMesh->mVertexBufferShadowBuffer);
        pReal = static_cast<Real*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readReals(chunk, pReal, dest->vertexCount * 3);
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometryColours(unsigned short bindIdx, 
        DataChunk& chunk, VertexData* dest)
    {
        RGBA* pRGBA = 0;
        HardwareVertexBufferSharedPtr vbuf;
        // unsigned long* pColours (RGBA 8888 format x numVertices)
        dest->vertexDeclaration->addElement(bindIdx, 0, VET_COLOUR, VES_DIFFUSE);
        vbuf = HardwareBufferManager::getSingleton().createVertexBuffer(
            dest->vertexDeclaration->getVertexSize(bindIdx),
            dest->vertexCount,
            mpMesh->mVertexBufferUsage,
			mpMesh->mVertexBufferShadowBuffer);
        pRGBA = static_cast<RGBA*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readLongs(chunk, pRGBA, dest->vertexCount);
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_2::readGeometryTexCoords(unsigned short bindIdx, 
        DataChunk& chunk, VertexData* dest, unsigned short texCoordSet)
    {
        Real *pReal = 0;
        HardwareVertexBufferSharedPtr vbuf;
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
            mpMesh->mVertexBufferUsage,
			mpMesh->mVertexBufferShadowBuffer);
        pReal = static_cast<Real*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readReals(chunk, pReal, dest->vertexCount * dim);
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_1::MeshSerializerImpl_v1_1()
    {
        // Version number
        mVersion = "[MeshSerializer_v1.10]";
    }
    //---------------------------------------------------------------------
    MeshSerializerImpl_v1_1::~MeshSerializerImpl_v1_1()
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializerImpl_v1_1::readGeometryTexCoords(unsigned short bindIdx, 
        DataChunk& chunk, VertexData* dest, unsigned short texCoordSet)
    {
        Real *pReal = 0;
        HardwareVertexBufferSharedPtr vbuf;
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
            mpMesh->getVertexBufferUsage(),
			mpMesh->isVertexBufferShadowed());
        pReal = static_cast<Real*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD));
        readReals(chunk, pReal, dest->vertexCount * dim);

        // Adjust individual v values to (1 - v)
        if (dim == 2)
        {
            for (size_t i = 0; i < dest->vertexCount; ++i)
            {
                ++pReal; // skip u
                *pReal = 1.0 - *pReal; // v = 1 - v
                ++pReal;
            }
            
        }
        vbuf->unlock();
        dest->vertexBufferBinding->setBinding(bindIdx, vbuf);
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------




}


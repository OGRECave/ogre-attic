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

#ifndef __MeshSerializerImpl_H__
#define __MeshSerializerImpl_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreSerializer.h"
#include "OgreMaterial.h"
#include "OgreMesh.h"
#include "OgreEdgeListBuilder.h"

namespace Ogre {


    /** Internal implementation of Mesh reading / writing for the latest version of the
    .mesh format.
    @remarks
    In order to maintain compatibility with older versions of the .mesh format, there
    will be alternative subclasses of this class to load older versions, whilst this class
    will remain to load the latest version.
    */
    class MeshSerializerImpl : public Serializer
    {
    public:
        MeshSerializerImpl();
        virtual ~MeshSerializerImpl();
        /** Exports a mesh to the file specified. 
        @remarks
        This method takes an externally created Mesh object, and exports both it
        and optionally the Materials it uses to a .mesh file.
        @param pMesh Pointer to the Mesh to export
        @param filename The destination filename
        */
        void exportMesh(const MeshPtr& pMesh, const String& filename);

        /** Imports Mesh and (optionally) Material data from a .mesh file DataStream.
        @remarks
        This method imports data from a DataStream opened from a .mesh file and places it's
        contents into the Mesh object which is passed in. 
        @param stream The DataStream holding the .mesh data. Must be initialised (pos at the start of the buffer).
        @param pDest Pointer to the Mesh object which will receive the data. Should be blank already.
        */
        void importMesh(DataStream& stream, MeshPtr& pDest);

    protected:

        bool mIsSkeletallyAnimated;

        // Internal methods
        virtual void writeSubMeshNameTable(const MeshPtr& pMesh);
        virtual void writeMesh(const MeshPtr& pMesh);
        virtual void writeSubMesh(const SubMesh* s);
        virtual void writeSubMeshOperation(const SubMesh* s);
        virtual void writeGeometry(const VertexData* pGeom);
        virtual void writeSkeletonLink(const String& skelName);
        virtual void writeMeshBoneAssignment(const VertexBoneAssignment* assign);
        virtual void writeSubMeshBoneAssignment(const VertexBoneAssignment* assign);
        virtual void writeLodInfo(const MeshPtr& pMesh);
        virtual void writeLodSummary(unsigned short numLevels, bool manual);
        virtual void writeLodUsageManual(const Mesh::MeshLodUsage& usage);
        virtual void writeLodUsageGenerated(const MeshPtr& pMesh, const Mesh::MeshLodUsage& usage, unsigned short lodNum);
        virtual void writeBoundsInfo(const MeshPtr& pMesh);
        virtual void writeEdgeList(const MeshPtr& pMesh);

        virtual unsigned long calcMeshSize(const MeshPtr& pMesh);
        virtual unsigned long calcSubMeshSize(const SubMesh* pSub);
        virtual unsigned long calcGeometrySize(const VertexData* pGeom);
        virtual unsigned long calcSkeletonLinkSize(const String& skelName);
        virtual unsigned long calcBoneAssignmentSize(void);
        virtual unsigned long calcSubMeshOperationSize(const SubMesh* pSub);
        virtual unsigned long calcSubMeshNameTableSize(const MeshPtr& pMesh);
        virtual unsigned long calcEdgeListSize(const MeshPtr& pMesh);
        virtual unsigned long calcEdgeListLodSize(const EdgeData* data, bool isManual);
        virtual unsigned long calcEdgeGroupSize(const EdgeData::EdgeGroup& group);

        virtual void readTextureLayer(DataStream& stream, MeshPtr& pMesh, MaterialPtr& pMat);
        virtual void readSubMeshNameTable(DataStream& stream, MeshPtr& pMesh);
        virtual void readMesh(DataStream& stream, MeshPtr& pMesh);
        virtual void readSubMesh(DataStream& stream, MeshPtr& pMesh);
        virtual void readSubMeshOperation(DataStream& stream, MeshPtr& pMesh, SubMesh* sub);
        virtual void readGeometry(DataStream& stream, MeshPtr& pMesh, VertexData* dest);
        virtual void readGeometryVertexDeclaration(DataStream& stream, MeshPtr& pMesh, VertexData* dest);
        virtual void readGeometryVertexElement(DataStream& stream, MeshPtr& pMesh, VertexData* dest);
        virtual void readGeometryVertexBuffer(DataStream& stream, MeshPtr& pMesh, VertexData* dest);

        virtual void readSkeletonLink(DataStream &stream, MeshPtr& pMesh);
        virtual void readMeshBoneAssignment(DataStream& stream, MeshPtr& pMesh);
        virtual void readSubMeshBoneAssignment(DataStream& stream, MeshPtr& pMesh, 
            SubMesh* sub);
        virtual void readMeshLodInfo(DataStream& stream, MeshPtr& pMesh);
        virtual void readMeshLodUsageManual(DataStream& stream, MeshPtr& pMesh, 
            unsigned short lodNum, Mesh::MeshLodUsage& usage);
        virtual void readMeshLodUsageGenerated(DataStream& stream, MeshPtr& pMesh, 
            unsigned short lodNum, Mesh::MeshLodUsage& usage);
        virtual void readBoundsInfo(DataStream& stream, MeshPtr& pMesh);
        virtual void readEdgeList(DataStream& stream, MeshPtr& pMesh);



        /// Flip an entire vertex buffer from little endian
        virtual void flipFromLittleEndian(void* pData, size_t vertexCount, size_t vertexSize, const VertexDeclaration::VertexElementList& elems);
        /// Flip an entire vertex buffer to little endian
        virtual void flipToLittleEndian(void* pData, size_t vertexCount, size_t vertexSize, const VertexDeclaration::VertexElementList& elems);
        /// Flip the endianness of an entire vertex buffer, passed in as a 
        /// pointer to locked or temporary memory 
        virtual void flipEndian(void* pData, size_t vertexCount, size_t vertexSize, const VertexDeclaration::VertexElementList& elems);



    };

    /** Class for providing backwards-compatibility for loading version 1.2 of the .mesh format. */
    class MeshSerializerImpl_v1_2 : public MeshSerializerImpl
    {
    public:
        MeshSerializerImpl_v1_2();
        ~MeshSerializerImpl_v1_2();
    protected:
        virtual void readMesh(DataStream& stream, MeshPtr& pMesh);
        virtual void readGeometry(DataStream& stream, MeshPtr& pMesh, VertexData* dest);
        virtual void readGeometryPositions(unsigned short bindIdx, DataStream& stream, 
            MeshPtr& pMesh, VertexData* dest);
        virtual void readGeometryNormals(unsigned short bindIdx, DataStream& stream, 
            MeshPtr& pMesh, VertexData* dest);
        virtual void readGeometryColours(unsigned short bindIdx, DataStream& stream, 
            MeshPtr& pMesh, VertexData* dest);
        virtual void readGeometryTexCoords(unsigned short bindIdx, DataStream& stream, 
            MeshPtr& pMesh, VertexData* dest, unsigned short set);
    };

    /** Class for providing backwards-compatibility for loading version 1.1 of the .mesh format. */
    class MeshSerializerImpl_v1_1 : public MeshSerializerImpl_v1_2
    {
    public:
        MeshSerializerImpl_v1_1();
        ~MeshSerializerImpl_v1_1();
    protected:
        void readGeometryTexCoords(unsigned short bindIdx, DataStream& stream, 
            MeshPtr& pMesh, VertexData* dest, unsigned short set);
    };


}

#endif

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

#ifndef __MeshSerializerImpl_H__
#define __MeshSerializerImpl_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"
#include "OgreSerializer.h"
#include "OgreMaterial.h"
#include "OgreMesh.h"

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
        void exportMesh(const Mesh* pMesh, const String& filename);

        /** Imports Mesh and (optionally) Material data from a .mesh file DataChunk.
        @remarks
            This method imports data from a DataChunk opened from a .mesh file and places it's
            contents into the Mesh object which is passed in. 
        @param chunk The DataChunk holding the .mesh data. Must be initialised (pos at the start of the buffer).
        @param pDest Pointer to the Mesh object which will receive the data. Should be blank already.
        */
        void importMesh(DataChunk& chunk, Mesh* pDest);

    protected:

        typedef std::map<String, Material*> MaterialMap;
        MaterialMap mMaterialList;
        Mesh* mpMesh;
        bool mIsSkeletallyAnimated;

        // Internal methods
        virtual void writeMaterial(const Material* m);
        virtual void writeTextureLayer(const Material::TextureLayer* pTex);
        virtual void writeMesh(const Mesh* pMesh);
        virtual void writeSubMesh(const SubMesh* s);
        virtual void writeGeometry(const VertexData* pGeom);
        virtual void writeSkeletonLink(const String& skelName);
        virtual void writeMeshBoneAssignment(const VertexBoneAssignment* assign);
        virtual void writeSubMeshBoneAssignment(const VertexBoneAssignment* assign);
        virtual void writeLodInfo(const Mesh* pMesh);
        virtual void writeLodSummary(unsigned short numLevels, bool manual);
        virtual void writeLodUsageManual(const Mesh::MeshLodUsage& usage);
        virtual void writeLodUsageGenerated(const Mesh* pMesh, const Mesh::MeshLodUsage& usage, unsigned short lodNum);
        virtual void writeBoundsInfo(const Mesh* pMesh);

        virtual unsigned long calcMaterialSize(const Material* pMat);
        virtual unsigned long calcTextureLayerSize(const Material::TextureLayer* pTex);
        virtual unsigned long calcMeshSize(const Mesh* pMesh);
        virtual unsigned long calcSubMeshSize(const SubMesh* pSub);
        virtual unsigned long calcGeometrySize(const VertexData* pGeom);
        virtual unsigned long calcSkeletonLinkSize(const String& skelName);
        virtual unsigned long calcBoneAssignmentSize(void);

        virtual void readMaterial(DataChunk& chunk);
        virtual void readTextureLayer(DataChunk& chunk, Material* pMat);
        virtual void readMesh(DataChunk& chunk);
        virtual void readSubMesh(DataChunk& chunk);
        virtual void readGeometry(DataChunk& chunk, VertexData* dest);
        virtual void readSkeletonLink(DataChunk &chunk);
        virtual void readMeshBoneAssignment(DataChunk& chunk);
        virtual void readSubMeshBoneAssignment(DataChunk& chunk, SubMesh* sub);
		virtual void readMeshLodInfo(DataChunk& chunk);
		virtual void readMeshLodUsageManual(DataChunk& chunk, unsigned short lodNum, Mesh::MeshLodUsage& usage);
		virtual void readMeshLodUsageGenerated(DataChunk& chunk, unsigned short lodNum, Mesh::MeshLodUsage& usage);
        virtual void readBoundsInfo(DataChunk& chunk);


    };

    /** Class for providing backwards-compatibility for loading version 1.0 of the .mesh format. */
    class MeshSerializerImpl_v1 : public MeshSerializerImpl
    {
    protected:
        bool mFirstGeometry;
    public:
        MeshSerializerImpl_v1();
        void readMaterial(DataChunk& chunk);
        void readTextureLayer(DataChunk& chunk, Material* pMat);
        void readMesh(DataChunk& chunk);
        void readSubMesh(DataChunk& chunk);
        void readGeometry(DataChunk& chunk, VertexData* dest);
        void readMeshBoneAssignment(DataChunk& chunk);
        void readSubMeshBoneAssignment(DataChunk& chunk, SubMesh* sub);
		void readMeshLodUsageGenerated(DataChunk& chunk, unsigned short lodNum, Mesh::MeshLodUsage& usage);

    };

}

#endif

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

#ifndef __MeshSerializer_H__
#define __MeshSerializer_H__

#include "OgrePrerequisites.h"
#include "OgreSerializer.h"
#include "OgreMaterial.h"
#include "OgreString.h"
#include "OgreMesh.h"

namespace Ogre {

    /** Class for serialising mesh data to/from an OGRE .mesh file.
    @remarks
        This class allows exporters to write OGRE .mesh files easily, and allows the
        OGRE engine to import .mesh files into instatiated OGRE Meshes.
        Note that a .mesh file can include not only the Mesh, but also definitions of
        any Materials it uses (although this is optional, the .mesh can rely on the
        Material being loaded from another source, especially useful if you want to
        take advantage of OGRE's advanced Material properties which may not be available
        in your modeller).
    @par
        To export a Mesh:<OL>
        <LI>Use the MaterialManager methods to create any dependent Material objects, if you want
            to export them with the Mesh.</LI>
        <LI>Create a Mesh object and populate it using it's methods.</LI>
        <LI>Call the exportMesh method</LI>
        </OL>
    @par
        It's important to realise that this exporter uses OGRE terminology. In this context,
        'Mesh' means a top-level mesh structure which can actually contain many SubMeshes, each
        of which has only one Material. Modelling packages may refer to these differently, for
        example in Milkshape, it says 'Model' instead of 'Mesh' and 'Mesh' instead of 'SubMesh', 
        but the theory is the same.
    */
    class _OgreExport MeshSerializer : public Serializer
    {
    public:
        MeshSerializer();
        virtual ~MeshSerializer();


        /** Exports a mesh to the file specified. 
        @remarks
            This method takes an externally created Mesh object, and exports both it
            and optionally the Materials it uses to a .mesh file.
        @param pMesh Pointer to the Mesh to export
        @param filename The destination filename
        @param includeMaterials If true, Material data is also exported into the file.
        */
        void exportMesh(const Mesh* pMesh, const String& filename, bool includeMaterials = false);

        /** Imports Mesh and (optionally) Material data from a .mesh file DataChunk.
        @remarks
            This method imports data from a DataChunk opened from a .mesh file and places it's
            contents into the Mesh object which is passed in. 
        @param chunk The DataChunk holding the .mesh data. Must be initialised (pos at the start of the buffer).
        @param pDest Pointer to the Mesh object which will receive the data. Should be blank already.
        */
        void importMesh(DataChunk& chunk, Mesh* pDest);


    private:
        typedef std::map<String, Material*> MaterialMap;
        MaterialMap mMaterialList;
        Mesh* mpMesh;

        // Internal methods
        void writeMaterial(const Material* m);
        void writeTextureLayer(const Material::TextureLayer* pTex);
        void writeMesh(const Mesh* pMesh);
        void writeSubMesh(const SubMesh* s);
        void writeGeometry(const VertexData* pGeom);
        void writeSkeletonLink(const String& skelName);
        void writeMeshBoneAssignment(const VertexBoneAssignment* assign);
        void writeSubMeshBoneAssignment(const VertexBoneAssignment* assign);
        void writeLodInfo(const Mesh* pMesh);
        void writeLodSummary(unsigned short numLevels, bool manual);
        void writeLodUsageManual(const Mesh::MeshLodUsage& usage);
        void writeLodUsageGenerated(const Mesh* pMesh, const Mesh::MeshLodUsage& usage, unsigned short lodNum);

        unsigned long calcMaterialSize(const Material* pMat);
        unsigned long calcTextureLayerSize(const Material::TextureLayer* pTex);
        unsigned long calcMeshSize(const Mesh* pMesh);
        unsigned long calcSubMeshSize(const SubMesh* pSub);
        unsigned long calcGeometrySize(const GeometryData* pGeom);
        unsigned long calcSkeletonLinkSize(const String& skelName);
        unsigned long calcBoneAssignmentSize(void);

        void readMaterial(DataChunk& chunk);
        void readTextureLayer(DataChunk& chunk, Material* pMat);
        void readMesh(DataChunk& chunk);
        void readSubMesh(DataChunk& chunk);
        void readGeometry(DataChunk& chunk, VertexData* dest);
        void readSkeletonLink(DataChunk &chunk);
        void readMeshBoneAssignment(DataChunk& chunk);
        void readSubMeshBoneAssignment(DataChunk& chunk, SubMesh* sub);
		void readMeshLodInfo(DataChunk& chunk);
		void readMeshLodUsageManual(DataChunk& chunk, unsigned short lodNum, Mesh::MeshLodUsage& usage);
		void readMeshLodUsageGenerated(DataChunk& chunk, unsigned short lodNum, Mesh::MeshLodUsage& usage);




    };

}


#endif

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

#ifndef __MeshSerializer_H__
#define __MeshSerializer_H__

#include "OgrePrerequisites.h"
#include "OgreMaterial.h"
#include "OgreString.h"

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
        <LI>call createMesh and populate the returned Mesh object</LI>
        <LI>optionally call createMaterial 1 or more times to create the material definitions</LI>
        <LI>call export</LI>
        </OL>
    @par
        It's important to realise that this exporter uses OGRE terminology. In this context,
        'Mesh' means a top-level mesh structure which can actually contain many SubMeshes, each
        of which has only one Material. Modelling packages may refer to these differently, for
        example in Milkshape, it says 'Model' instead of 'Mesh' and 'Mesh' instead of 'SubMesh', 
        but the theory is the same.
    */
    class _OgreExport MeshSerializer
    {
    public:
        MeshSerializer();
        virtual ~MeshSerializer();

        /** Tells this class whether it should automatically free the Mesh memory it creates.
        @remarks
            This parameter determines whether the internal Mesh object that this class holds should 
            be deleted by this class. The default is 'true' which is
            probably what you want for exporters. However when importing you may wish to just
            transfer the Mesh object over to another class rather than destroy this one
            and create a copy. So internally OGRE uses this to minimise memory thrashing.
        @param shouldDelete If true, the internal Mesh will be deleted by this class. If false, 
            it expects other classes to take responsibility.
        */
        void setAutoDeleteMeshes(bool shouldDelete);

        /** Creates a new Mesh ready to be exported.
        @remarks
            Exporters can call this to create a new Mesh ready to be exported.
            It can then call all the usual Mesh methods like createSubMesh etc to build up the
            required data. Once happy with the Mesh, and after creating any Materials you
            may want to include in the .mesh file, call exportToFile.
        @par If a Mesh had previously been created (and probably exported) it will be deleted if the
            setAutoDeleteMeshes option is true (the default). Materials will also be deleted.
        */
        Mesh* createMesh(void);

        /** Creates a new material as used by the mesh you are exporting. 
        @remarks
            Once created, you can modify the properties of this Material exactly as
            per the standard Material obejcts in OGRE.
        */
        Material* createMaterial(const String& name);


        /** Exports the stored mesh data to the file specified. 
        @remarks
            This method exports the Mesh and Material data held in this class to the specified
            file.
        @param filename The destination filename
        */
        void export(const String& filename);

        /** Exports an externally stored mesh to the file specified. 
        @remarks
            This method takes an externally created Mesh object, and exports both it
            and optionally the Materials it uses to a .mesh file.
        @param pMesh Pointer to the Mesh to export
        @param filename The destination filename
        @param includeMaterials If true, Material data is also exported into the file.
        */
        void exportOther(const Mesh* pMesh, const String& filename, bool includeMaterials = false);

        /** Imports Mesh and (optionally) Material data from a DataChunk.
        @remarks
            After calling this method, you can call getMesh and getMaterial to retrieve the
            data imported from the chunk. 
        */
        void import(DataChunk& chunk);

        /** Returns a pointer to an imported Mesh.
        @remarks
            If you intend to use this Mesh object as-is and not copy it, ensure you call
            setAutoDeleteMeshes(false) otherwise the Mesh will be freed later on.
        */
        Mesh* getMesh(void);

        /** Gets an imported Material by index. 
        @remarks
            You should copy this Material and not assume the object will remain in existence.
        */
        Material* getMaterial(size_t index);

        /** Gets an imported Material by it's name. 
        @remarks
            You should copy this Material and not assume the object will remain in existence.
        */
        Material* getMaterial(const String& name);

        /** Returns the number of imported Material objects. */
        size_t getNumMaterials(void);


    private:
        bool mShouldDelete;
        typedef std::map<String, Material*> MaterialMap;
        MaterialMap mMaterialList;
        Mesh* mpMesh;
        FILE* mpfFile;
        String mVersion;

        // Internal methods
        void freeMemory(void);

        void writeFileHeader(unsigned short numMaterials);
        void writeChunkHeader(unsigned short id, unsigned long size);
        void writeMaterial(const Material* m);
        void writeTextureLayer(const Material::TextureLayer* pTex);
        void writeMesh(const Mesh* pMesh);
        void writeSubMesh(const SubMesh* s);
        void writeGeometry(const GeometryData* pGeom);
        void writeReals(const Real* pReal, unsigned short count);
        void writeShorts(const unsigned short* pShort, unsigned short count);
        void writeLongs(const unsigned long* pLong, unsigned short count); 

        unsigned long calcMaterialSize(const Material* pMat);
        unsigned long calcTextureLayerSize(const Material::TextureLayer* pTex);
        unsigned long calcMeshSize(const Mesh* pMesh);
        unsigned long calcSubMeshSize(const SubMesh* pSub);
        unsigned long calcGeometrySize(const GeometryData* pGeom);

        void writeData(const void* buf, size_t size, size_t count);
        void writeString(const String& string);

        void readFileHeader(DataChunk& chunk);
        void readChunk(DataChunk& chunk);
        void readMaterial(DataChunk& chunk);
        void readTextureLayer(DataChunk& chunk, Material* pMat);
        void readMesh(DataChunk& chunk);
        void readSubMesh(DataChunk& chunk);
        void readGeometry(DataChunk& chunk, GeometryData* dest);
        void readReals(DataChunk& chunk, Real* pDest, unsigned short count);
        void readShorts(DataChunk& chunk, unsigned short* pDest, unsigned short count);
        void readLongs(DataChunk& chunk, unsigned long* pDest, unsigned short count); 

        void readData(DataChunk& chunk, void* buf, size_t size, size_t count);
        String readString(DataChunk& chunk);

    };

}


#endif

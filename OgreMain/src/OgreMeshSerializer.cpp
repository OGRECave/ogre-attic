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


namespace Ogre {

    /// Chunk overhead = ID + size
    const unsigned long CHUNK_OVERHEAD_SIZE = sizeof(unsigned short) + sizeof(unsigned long);
    //---------------------------------------------------------------------
    MeshSerializer::MeshSerializer()
    {
        mpMesh = 0;
        mShouldDelete = true;

        // Version number
        mVersion = "[MeshSerializer v1.00]";
    }
    //---------------------------------------------------------------------
    MeshSerializer::~MeshSerializer()
    {
        freeMemory();
    }
    //---------------------------------------------------------------------
    void MeshSerializer::freeMemory(void)
    {
        if (mpMesh && mShouldDelete)
        {
            delete mpMesh;
            mpMesh = 0;
        }

        // Init materials
        MaterialMap::iterator i;
        for (i = mMaterialList.begin(); i != mMaterialList.end(); ++i)
        {
            delete i->second;
        }
        mMaterialList.clear();
    }
    //---------------------------------------------------------------------
    void MeshSerializer::setAutoDeleteMeshes(bool shouldDelete)
    {
        mShouldDelete = shouldDelete;
    }
    //---------------------------------------------------------------------
    Mesh* MeshSerializer::createMesh(void)
    {
        freeMemory();
        mpMesh = new Mesh("exporter");
        return mpMesh;
    }
    //---------------------------------------------------------------------
    Material* MeshSerializer::createMaterial(const String& name)
    {
        Material* pM = new Material(name);
        mMaterialList[name] = pM;
        return pM;
    }
    //---------------------------------------------------------------------
    void MeshSerializer::export(const String& name)
    {
        mpfFile = fopen(name, "wb");
        
        writeFileHeader((unsigned short)mMaterialList.size());

        // Write materials if present
        MaterialMap::iterator i;
        for (i = mMaterialList.begin(); i != mMaterialList.end(); ++i)
        {
            writeMaterial(i->second);
        }

        writeMesh(mpMesh);

        fclose(mpfFile);
    }
    //---------------------------------------------------------------------
    void MeshSerializer::exportOther(const Mesh* pMesh, const String& filename, bool includeMaterials)
    {
        mpfFile = fopen(filename, "wb");

        writeFileHeader((unsigned short)mMaterialList.size());

        // Write materials if required
        if (includeMaterials)
        {
            for (int i = 0; i < pMesh->getNumSubMeshes(); ++i)
            {
                SubMesh* sm = pMesh->getSubMesh(i);
                writeMaterial(sm->getMaterial());
            }
        }

        writeMesh(pMesh);

        fclose(mpfFile);
    }
    //---------------------------------------------------------------------
    void MeshSerializer::import(DataChunk& chunk)
    {
        freeMemory();
        mpMesh = new Mesh("import");

        // Check header
        readFileHeader(chunk);

        while(!chunk.isEOF())
        {
            readChunk(chunk);
        }
    }
    //---------------------------------------------------------------------
    Mesh* MeshSerializer::getMesh(void)
    {
        return mpMesh;
    }
    //---------------------------------------------------------------------
    Material* MeshSerializer::getMaterial(size_t index)
    {
        assert(index >= 0 && index < mMaterialList.size());

        MaterialMap::iterator i = mMaterialList.begin();
        while(index)
            ++i;
        return i->second;
        
    }
    //---------------------------------------------------------------------
    Material* MeshSerializer::getMaterial(const String& name)
    {
        MaterialMap::iterator i = mMaterialList.find(name);

        if (i == mMaterialList.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Material " + name + " not found.", 
                "MeshSerializer::getMaterial");
        }

        return i->second;
    }
    //---------------------------------------------------------------------
    size_t MeshSerializer::getNumMaterials(void)
    {
        return mMaterialList.size();
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeFileHeader(unsigned short numMaterials)
    {
       
        unsigned short val = M_HEADER;
        writeData(&val, sizeof(unsigned short), 1);

        writeString(mVersion);

    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeChunkHeader(unsigned short id, unsigned long size)
    {
        writeData(&id, sizeof(unsigned short), 1);
        writeData(&size, sizeof(unsigned long), 1);
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
            writeSubMesh(pMesh->getSubMesh(i));
        }


    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeSubMesh(const SubMesh* s)
    {
        // Header
        writeChunkHeader(M_SUBMESH, calcSubMeshSize(s));

        // char* materialName
        writeString(s->getMaterial()->getName());

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
    void MeshSerializer::writeReals(const Real* pReal, unsigned short count)
    {
        writeData(pReal, sizeof(Real), count);
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeShorts(const unsigned short* pShort, unsigned short count)
    {
        writeData(pShort, sizeof(unsigned short), count);
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeLongs(const unsigned long* pLong, unsigned short count)
    {
        writeData(pLong, sizeof(unsigned long), count);
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

        return size;


    }
    //---------------------------------------------------------------------
    unsigned long MeshSerializer::calcSubMeshSize(const SubMesh* pSub)
    {
        unsigned long size = CHUNK_OVERHEAD_SIZE;

        // Material name
        size += (unsigned long)pSub->getMaterial()->getName().length() + 1;

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
    void MeshSerializer::writeData(const void* buf, size_t size, size_t count)
    {
        fwrite((void* const)buf, size, count, mpfFile);
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeString(const String& string)
    {
        fputs(string.c_str(), mpfFile);
        // Write terminating newline char
        fputc('\n', mpfFile);
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readFileHeader(DataChunk& chunk)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readChunk(DataChunk& chunk)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readMaterial(DataChunk& chunk)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readTextureLayer(DataChunk& chunk, Material* pMat)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readMesh(DataChunk& chunk)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readSubMesh(DataChunk& chunk)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readGeometry(DataChunk& chunk, GeometryData* dest)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readReals(DataChunk& chunk, Real* pDest, unsigned short count)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readShorts(DataChunk& chunk, unsigned short* pDest, unsigned short count)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readLongs(DataChunk& chunk, unsigned long* pDest, unsigned short count) 
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::readData(DataChunk& chunk, void* buf, size_t size, size_t count)
    {
    }
    //---------------------------------------------------------------------
    String readString(DataChunk& chunk)
    {
        char str[255];
        int readcount;
        readcount = chunk.readUpTo(str, 255);
        str[readcount] = '\0';
        return str;

    }
    //---------------------------------------------------------------------


}


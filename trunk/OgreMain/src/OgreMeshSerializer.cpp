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
#include "OgreException.h"


namespace Ogre {

    //---------------------------------------------------------------------
    MeshSerializer::MeshSerializer()
    {
        mpMesh = 0;
        mShouldDelete = true;
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
        mOStream = new std::ofstream(name, std::ios_base::binary);
        
        writeFileHeader(mMaterialList.size());

        // Write materials if present
        MaterialMap::iterator i;
        for (i = mMaterialList.begin(); i != mMaterialList.end(); ++i)
        {
            writeMaterial(i->second);
        }

        writeMesh(mpMesh);

        delete mOStream;
    }
    //---------------------------------------------------------------------
    void MeshSerializer::import(DataChunk& chunk)
    {
        //TODO
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
        const char *version = "1.0";
        *mOStream << M_HEADER;
        *mOStream << version;
        *mOStream << mMaterialList.size();
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeChunkHeader(unsigned short id, unsigned long size)
    {
        *mOStream << id;
        *mOStream << size;
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeMaterial(const Material* m)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeTextureLayer(const Material::TextureLayer* pTex)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeMesh(const Mesh* pMesh)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeSubMesh(const SubMesh* s)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeGeometry(const GeometryData* pGeom)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeReals(const Real* pReal, unsigned short count)
    {
    }
    //---------------------------------------------------------------------
    void MeshSerializer::writeShorts(const short* pShort, unsigned short count)
    {
    }
    //---------------------------------------------------------------------



}


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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreQuake3Level.h"
#include "OgreDataChunk.h"
#include "OgreLogManager.h"
#include "OgreTextureManager.h"

namespace Ogre {


    Quake3Level::Quake3Level()
    {

    }

    void Quake3Level::loadFromChunk(DataChunk& inChunk)
    {
        mChunk = inChunk;
        initialise();

#ifdef _DEBUG
        dumpContents();
#endif


    }

    void Quake3Level::initialise(void)
    {
        mHeader = (bsp_header_t*)mChunk.getPtr();
        mLumpStart = ((unsigned char*)mHeader) + sizeof(mHeader);

        mEntities = (unsigned char*)getLump(BSP_ENTITIES_LUMP);
        mNumEntities = getLumpSize(BSP_ENTITIES_LUMP);

        mElements = (int*)getLump(BSP_ELEMENTS_LUMP);
        mNumElements = getLumpSize(BSP_ELEMENTS_LUMP) / sizeof(int);

        mFaces = (bsp_face_t*)getLump(BSP_FACES_LUMP);
        mNumFaces = getLumpSize(BSP_FACES_LUMP) / sizeof(bsp_face_t);

        mLeafFaces = (int*)getLump(BSP_LFACES_LUMP);
        mNumLeafFaces = getLumpSize(BSP_LFACES_LUMP) / sizeof(int);

        mLeaves = (bsp_leaf_t*)getLump(BSP_LEAVES_LUMP);
        mNumLeaves = getLumpSize(BSP_LEAVES_LUMP) / sizeof(bsp_leaf_t);

        mLightmaps = (unsigned char*)getLump(BSP_LIGHTMAPS_LUMP);
        mNumLightmaps = getLumpSize(BSP_LIGHTMAPS_LUMP)/BSP_LIGHTMAP_BANKSIZE;

        mModels = (bsp_model_t*)getLump(BSP_MODELS_LUMP);
        mNumModels = getLumpSize(BSP_MODELS_LUMP) / sizeof(bsp_model_t);

        mNodes = (bsp_node_t*)getLump(BSP_NODES_LUMP);
        mNumNodes = getLumpSize(BSP_NODES_LUMP) / sizeof(bsp_node_t);

        mPlanes = (bsp_plane_t*) getLump(BSP_PLANES_LUMP);
        mNumPlanes = getLumpSize(BSP_PLANES_LUMP)/sizeof(bsp_plane_t);

        mShaders = (bsp_shader_t*) getLump(BSP_SHADERS_LUMP);
        mNumShaders = getLumpSize(BSP_SHADERS_LUMP)/sizeof(bsp_shader_t);

        mVis = (bsp_vis_t*)getLump(BSP_VISIBILITY_LUMP);

        mVertices = (bsp_vertex_t*) getLump(BSP_VERTICES_LUMP);
        mNumVertices = getLumpSize(BSP_VERTICES_LUMP)/sizeof(bsp_vertex_t);

    }

    void* Quake3Level::getLump(int lumpType)
    {
        return (unsigned char*)mHeader + mHeader->lumps[lumpType].offset;
    }

    int Quake3Level::getLumpSize(int lumpType)
    {
        return mHeader->lumps[lumpType].size;
    }

    void Quake3Level::dumpContents(void)
    {
        std::ofstream of;
        of.open("Quake3Level.log");


        of << "Quake3 level statistics" << std::endl;
        of << "-----------------------" << std::endl;
        of << "Entities     : " << mNumEntities << std::endl;
        of << "Faces        : " << mNumFaces << std::endl;
        of << "Leaf Faces   : " << mNumLeafFaces << std::endl;
        of << "Leaves       : " << mNumLeaves << std::endl;
        of << "Lightmaps    : " << mNumLightmaps << std::endl;
        of << "Elements     : " << mNumElements << std::endl;
        of << "Models       : " << mNumModels << std::endl;
        of << "Nodes        : " << mNumNodes << std::endl;
        of << "Planes       : " << mNumPlanes << std::endl;
        of << "Shaders      : " << mNumShaders << std::endl;
        of << "Vertices     : " << mNumVertices << std::endl;
        of << "Vis Clusters : " << mVis->cluster_count << std::endl;

        of << std::endl;
        of << "-= Shaders =-";
        of << std::endl;
        for (int i = 0; i < mNumShaders; ++i)
        {
            of << "Shader " << i << ": " << mShaders[i].name << std::endl;
        }

        of << std::endl;
        of << "-= Entities =-";
        of << std::endl;
        char* strEnt = strtok((char*)mEntities, "\0");
        while (strEnt != 0)
        {
            of << strEnt << std::endl;
            strEnt = strtok(0, "\0");
        }




        of.close();
    }

    void Quake3Level::extractLightmaps(void) const
    {
        // Lightmaps are always 128x128x24 (RGB)
        unsigned char* pLightmap = mLightmaps;
        for (int i = 0; i < mNumLightmaps; ++i)
        {
            char name[32];
            sprintf(name, "@lightmap%d", i);

            // Load, no mipmaps, brighten by factor 2.5
            Image img; img.loadRawData( DataChunk( pLightmap, 128 * 128 * 3 ), 128, 128, Image::FMT_RGB );
            TextureManager::getSingleton().loadImage( name, img, 0, 3.0f );
            pLightmap += BSP_LIGHTMAP_BANKSIZE;
        }


    }


}

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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
#include "OgreTerrainPageSource.h"
#include "OgreTerrainPage.h"
#include "OgreTerrainRenderable.h"
#include "OgreSceneNode.h"
#include "OgreTerrainSceneManager.h"

namespace Ogre {

    //-------------------------------------------------------------------------
    TerrainPage* TerrainPageSource::buildPage(Real* heightData, Material* pMaterial)
    {
        String name;

        // Create a Terrain Page
        TerrainPage* page = new TerrainPage((mPageSize-1) / (mTileSize-1));
        // Create a node for all tiles to be attached to
        // Note we sequentially name since page can be attached at different points
        // so page x/z is not appropriate
        static size_t pageIndex = 0;
        name = "page[";
        name << (int)pageIndex << "]";
        page->pageSceneNode = mSceneManager->createSceneNode(name);
        
        size_t q = 0;
        for ( size_t j = 0; j < mPageSize - 1; j += ( mTileSize - 1 ) )
        {
            size_t p = 0;

            for ( size_t i = 0; i < mPageSize - 1; i += ( mTileSize - 1 ) )
            {
                // Create scene node for the tile and the TerrainRenderable
                name = "tile[";
                name << pageIndex << "][" << (int)p << "," << (int)q << "]";

                SceneNode *c = page->pageSceneNode->createChildSceneNode( name );
                TerrainRenderable *tile = new TerrainRenderable(name);
                // Initialise the tile
                tile->setMaterial(pMaterial);
                tile->initialise(i, j, heightData);
                // Attach it to the page
                page->tiles[ p ][ q ] = tile;
                // Attach it to the node
                c ->attachObject( tile );
                p++;
            }

            q++;

        }

        pageIndex++;

        // calculate neighbours for page
        page->linkNeighbours();

        return page;
    }
}

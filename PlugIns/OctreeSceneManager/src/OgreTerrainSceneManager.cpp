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
/***************************************************************************
terrainscenemanager.cpp  -  description
-------------------
begin                : Mon Sep 23 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net

Enhancements 2003 - 2004 (C) The OGRE Team

***************************************************************************/
#include <OgreTerrainSceneManager.h>
#include <OgreImage.h>
#include <OgreConfigFile.h>
#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreCamera.h>
#include "OgreException.h"
#include "OgreStringConverter.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include <fstream>

#define TERRAIN_MATERIAL_NAME "TerrainSceneManager/Terrain"

namespace Ogre
{

TerrainSceneManager::TerrainSceneManager() : OctreeSceneManager( )
{
    //setDisplaySceneNodes( true );
    //setShowBoxes( true );

    mUseCustomMaterial = false;
    mUseNamedParameterLodMorph = false;
    mLodMorphParamIndex = 3;
    mTerrainRoot = 0;
    mTerrainMaterial = 0;


}

TerrainSceneManager::~TerrainSceneManager()
{
    size_t size = mTiles.size();

    for ( size_t i = 0; i < size; i++ )
        for ( size_t j = 0; j < size; j++ )
            delete mTiles[ i ][ j ];
}

void TerrainSceneManager::loadConfig(const String& filename)
{
    /* Set up the options */
    ConfigFile config;
    String val;

    config.load( filename );

    val = config.getSetting( "DetailTile" );
    if ( val != "" )
        setDetailTextureRepeat(atoi(val));

    val = config.getSetting( "MaxMipMapLevel" );
    if ( val != "" )
        setMaxGeoMipMapLevel(atoi( val ));

    Vector3 v = Vector3::UNIT_SCALE;

    val = config.getSetting( "ScaleX" );
    if ( val != "" )
        v.x = atof( val );

    val = config.getSetting( "ScaleY" );
    if ( val != "" )
        v.y = atof( val );

    val = config.getSetting( "ScaleZ" );
    if ( val != "" )
        v.z = atof( val );

    setScale(v);

    val = config.getSetting( "MaxPixelError" );
    if ( val != "" )
        setMaxPixelError(atoi( val ));

    val = config.getSetting( "TileSize" );
    if ( val != "" )
        setTileSize(atoi( val ));

    mHeightmapName = config.getSetting( "Terrain" );

    mDetailTextureName = config.getSetting( "DetailTexture" );

    mWorldTextureName = config.getSetting( "WorldTexture" );

    if ( config.getSetting( "VertexColors" ) == "yes" )
        mOptions.colored = true;

    if ( config.getSetting( "VertexNormals" ) == "yes" )
        mOptions.lit = true;

    if ( config.getSetting( "UseTriStrips" ) == "yes" )
        setUseTriStrips(true);

    if ( config.getSetting( "VertexProgramMorph" ) == "yes" )
        setUseLODMorph(true);

    val = config.getSetting( "CustomMaterialName" );
    if ( val != "" )
        setCustomMaterial(val);

    val = config.getSetting( "MorphLODFactorParamName" );
    if ( val != "" )
        setCustomMaterialMorphFactorParam(val);

    val = config.getSetting( "MorphLODFactorParamIndex" );
    if ( val != "" )
        setCustomMaterialMorphFactorParam(atoi(val));
}

void TerrainSceneManager::loadHeightmap(void)
{
    Image image;

    image.load( mHeightmapName );

    //check to make sure it's 2^n + 1 size.
    if ( image.getWidth() != image.getHeight() ||
        ! _checkSize( image.getWidth() ) )
    {
        String err = "Error: Invalid heightmap size : " +
            StringConverter::toString( image.getWidth() ) +
            "," + StringConverter::toString( image.getHeight() ) +
            ". Should be 2^n+1, 2^n+1";
        Except( Exception::ERR_INVALIDPARAMS, err, "TerrainSceneManager::loadHeightmap" );
    }

    int upperRange = 0;
    int size = image.getWidth();

    if ( image.getFormat() == PF_L8 )
    {
        upperRange = 255;

        // Parse the char data into floats
        mOptions.heightData = new Real[size*size];
        const uchar* pSrc = image. getData();
        Real* pDest = mOptions.heightData;
        for (int i = 0; i < size*size; ++i)
        {
            *pDest++ = *pSrc++ * mScale.y;
        }
    }
    /*
    else if ( image.getFormat() == PF_L16 )
    {
    }
    */
    else
    {
        Except( Exception::ERR_INVALIDPARAMS, "Error: Image is not a grayscale image.",
            "TerrainSceneManager::setWorldGeometry" );
    }


    // set up the octree size.
    float max_x = mOptions.scalex * size;

    float max_y = upperRange * mOptions.scaley;

    float max_z = mOptions.scalez * size;

    resize( AxisAlignedBox( 0, 0, 0, max_x, max_y, max_z ) );

    mOptions.world_size = size;

}

void TerrainSceneManager::setupTerrainMaterial(void)
{
    if (mCustomMaterialName == "")
    {
        // define our own material
        mTerrainMaterial = static_cast<Material*>(
            MaterialManager::getSingleton().getByName(TERRAIN_MATERIAL_NAME));
        if (!mTerrainMaterial)
        {
            mTerrainMaterial = createMaterial( "TerrainSceneManager/Terrain" );

        }
        else
        {
            mTerrainMaterial->getTechnique(0)->getPass(0)->removeAllTextureUnitStates();
        }

        if ( mWorldTextureName != "" )
        {
            mTerrainMaterial->getTechnique(0)->getPass(0)->
                createTextureUnitState( mWorldTextureName, 0 );
        }
        if ( mDetailTextureName != "" )
        {
            mTerrainMaterial->getTechnique(0)->getPass(0)->
                createTextureUnitState( mDetailTextureName, 1 );
        }

        mTerrainMaterial -> setLightingEnabled( mOptions.lit );

        if (mOptions.lodMorph)
        {
            // Create & assign LOD morphing vertex program
            

            // Set param index
            mLodMorphParamName = "";
            mLodMorphParamIndex = 4;
        }

        mTerrainMaterial->load();

    }
    else
    {
        // Custom material
        mTerrainMaterial = static_cast<Material*>(
            MaterialManager::getSingleton().getByName(mCustomMaterialName));
        mTerrainMaterial->load();

    }

    // now set up the linkage between vertex program and LOD morph param
    if (mOptions.lodMorph)
    {
        Technique* t = mTerrainMaterial->getBestTechnique();
        for (ushort i = 0; i < t->getNumPasses(); ++i)
        {
            Pass* p = t->getPass(i);
            if (p->hasVertexProgram())
            {
                // we have to assume vertex program includes LOD morph capability
                GpuProgramParametersSharedPtr params = 
                    p->getVertexProgramParameters();
                // Check to see if custom param is already there
                GpuProgramParameters::AutoConstantIterator aci = params->getAutoConstantIterator();
                bool found = false;
                while (aci.hasMoreElements())
                {
                    GpuProgramParameters::AutoConstantEntry& ace = aci.getNext();
                    if (ace.paramType == GpuProgramParameters::ACT_CUSTOM && 
                        ace.data == MORPH_CUSTOM_PARAM_ID)
                    {
                        found = true;
                    }
                }
                if (!found)
                {
                    if(mLodMorphParamName != "")
                    {
                        params->setNamedAutoConstant(mLodMorphParamName, 
                            GpuProgramParameters::ACT_CUSTOM, MORPH_CUSTOM_PARAM_ID);
                    }
                    else
                    {
                        params->setAutoConstant(mLodMorphParamIndex, 
                            GpuProgramParameters::ACT_CUSTOM, MORPH_CUSTOM_PARAM_ID);
                    }
                }

            }
        }
    }

}

void TerrainSceneManager::setupTerrainTiles(void)
{

    //create a root terrain node.
    if (!mTerrainRoot)
        mTerrainRoot = getRootSceneNode() -> createChildSceneNode( "Terrain" );

    //setup the tile array.
    int num_tiles = ( mOptions.world_size - 1 ) / ( mOptions.size - 1 );
    int i, j;
    for ( i = 0; i < num_tiles; i++ )
    {
        mTiles.push_back( TerrainRow() );

        for ( j = 0; j < num_tiles; j++ )
        {
            mTiles[ i ].push_back( 0 );
        }
    }

    char name[ 24 ];
    int p = 0;
    int q = 0;

    for ( j = 0; j < mOptions.world_size - 1; j += ( mOptions.size - 1 ) )
    {
        p = 0;

        for ( i = 0; i < mOptions.world_size - 1; i += ( mOptions.size - 1 ) )
        {
            mOptions.startx = i;
            mOptions.startz = j;
            sprintf( name, "tile[%d,%d]", p, q );

            SceneNode *c = mTerrainRoot -> createChildSceneNode( name );
            TerrainRenderable *tile = new TerrainRenderable(name);

            tile -> setMaterial( mTerrainMaterial );
            tile -> init( mOptions );

            mTiles[ p ][ q ] = tile;

            c -> attachObject( tile );
            p++;
        }

        q++;

    }


    //setup the neighbor links.
    int size = ( int ) mTiles.size();

    for ( j = 0; j < size; j++ )
    {
        for ( i = 0; i < size; i++ )
        {
            if ( j != size - 1 )
            {
                mTiles[ i ][ j ] -> _setNeighbor( TerrainRenderable::SOUTH, mTiles[ i ][ j + 1 ] );
                mTiles[ i ][ j + 1 ] -> _setNeighbor( TerrainRenderable::NORTH, mTiles[ i ][ j ] );
            }

            if ( i != size - 1 )
            {
                mTiles[ i ][ j ] -> _setNeighbor( TerrainRenderable::EAST, mTiles[ i + 1 ][ j ] );
                mTiles[ i + 1 ][ j ] -> _setNeighbor( TerrainRenderable::WEST, mTiles[ i ][ j ] );
            }

        }
    }

    // Dynamic terrain lighting?
    if(mOptions.lit)
    {
        for ( j = 0; j < size; j++ )
        {
            for ( i = 0; i < size; i++ )
            {
                mTiles[ i ][ j ] -> _calculateNormals( );
                //  mTiles[ i ][ j ] -> _generateVertexLighting( Vector3( 255, 100, 255 ), ColourValue(.25,.25,.25) );
            }
        }
    }


}

void TerrainSceneManager::setWorldGeometry( const String& filename )
{
   
    // Load the configuration
    loadConfig(filename);

    // Load heightmap data
    loadHeightmap();

    setupTerrainMaterial();

    setupTerrainTiles();


    delete [] mOptions.heightData;


}


void TerrainSceneManager::_renderVisibleObjects( void )
{

    for ( int i = 0; i < ( int ) mTiles.size(); i++ )
    {
        for ( int j = 0; j < ( int ) mTiles.size(); j++ )
        {
            mTiles[ i ][ j ] ->_alignNeighbors();
        }
    }

    mDestRenderSystem -> setLightingEnabled( false );

    OctreeSceneManager::_renderVisibleObjects();

}

float TerrainSceneManager::getHeightAt( float x, float z )
{


    Vector3 pt( x, 0, z );

    TerrainRenderable * t = getTerrainTile( pt );

    if ( t == 0 )
    {
        //  printf( "No tile found for point\n" );
        return -1;
    }

    float h = t -> getHeightAt( x, z );

    // printf( "Height is %f\n", h );
    return h;

}

TerrainRenderable * TerrainSceneManager::getTerrainTile( const Vector3 & pt )
{
    /* Since we don't know if the terrain is square, or has holes, we use a line trace
       to find the containing tile...
    */

    TerrainRenderable * tile = mTiles[ 0 ][ 0 ];

    while ( tile != 0 )
    {
        AxisAlignedBox b = tile -> getBoundingBox();
        const Vector3 *corners = b.getAllCorners();

        if ( pt.x < corners[ 0 ].x )
            tile = tile -> _getNeighbor( TerrainRenderable::WEST );
        else if ( pt.x > corners[ 4 ].x )
            tile = tile -> _getNeighbor( TerrainRenderable::EAST );
        else if ( pt.z < corners[ 0 ].z )
            tile = tile -> _getNeighbor( TerrainRenderable::NORTH );
        else if ( pt.z > corners[ 4 ].z )
            tile = tile -> _getNeighbor( TerrainRenderable::SOUTH );
        else
            return tile;
    }

    return 0;
}


bool TerrainSceneManager::intersectSegment( const Vector3 & start, const Vector3 & end, Vector3 * result )
{

    TerrainRenderable * t = getTerrainTile( start );

    if ( t == 0 )
    {
        *result = Vector3( -1, -1, -1 );
        return false;
    }

    return t -> intersectSegment( start, end, result );
}

void TerrainSceneManager::setUseTriStrips(bool useStrips)
{
    TerrainRenderable::_setUseTriStrips(useStrips);
}
void TerrainSceneManager::setUseLODMorph(bool morph)
{
    mOptions.lodMorph = morph;
    // Set true only if vertex programs are supported
    TerrainRenderable::_setUseLODMorph(
        morph && mDestRenderSystem->getCapabilities()->hasCapability(RSC_VERTEX_PROGRAM));
}

void TerrainSceneManager::setHeightmap(const String& heightmapName)
{
    mHeightmapName = heightmapName;
}
void TerrainSceneManager::setWorldTexture(const String& textureName)
{
    mWorldTextureName = textureName;
}
void TerrainSceneManager::setDetailTexture(const String& textureName)
{
    mDetailTextureName = textureName;

}
void TerrainSceneManager::setDetailTextureRepeat(int repeat)
{
    mOptions.detail_tile = repeat;
}
void TerrainSceneManager::setTileSize(int size) 
{
    mOptions.size = size;
    mTileSize = size;
}
void TerrainSceneManager::setMaxPixelError(int pixelError) 
{
    mOptions.max_pixel_error = pixelError;
}
void TerrainSceneManager::setScale(const Vector3& scale)
{
    mOptions.scalex = scale.x;
    mOptions.scaley = scale.y;
    mOptions.scalez = scale.z;
    mScale = scale;
}
void TerrainSceneManager::setMaxGeoMipMapLevel(int maxMip)
{
    mOptions.max_mipmap = maxMip;
}
void TerrainSceneManager::setCustomMaterial(const String& materialName)
{
    mCustomMaterialName = materialName;
    if (materialName != "")
        mUseCustomMaterial = true;
    else
        mUseCustomMaterial = false;
}
void TerrainSceneManager::setCustomMaterialMorphFactorParam(const String& paramName)
{
    mUseNamedParameterLodMorph = true;
    mLodMorphParamName = paramName;
    
}
void TerrainSceneManager::setCustomMaterialMorphFactorParam(size_t paramIndex)
{
    mUseNamedParameterLodMorph = false;
    mLodMorphParamIndex = paramIndex;
}

} //namespace

/***************************************************************************
terrainscenemanager.cpp  -  description
-------------------
begin                : Mon Sep 23 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Lesser General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
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
#include <OgreRenderSystem.h>
#include <fstream>

namespace Ogre
{

TerrainSceneManager::TerrainSceneManager() : OctreeSceneManager( )
{
    //setDisplaySceneNodes( true );
    //setShowBoxes( true );
}

TerrainSceneManager::~TerrainSceneManager()
{
    size_t size = mTiles.size();

    for ( size_t i = 0; i < size; i++ )
        for ( size_t j = 0; j < size; j++ )
            delete mTiles[ i ][ j ];
}

void TerrainSceneManager::setWorldGeometry( const String& filename )
{
    int i, j;

    /* Set up the options */
    ConfigFile config;

    config.load( filename );

    if ( config.getSetting( "DetailTile" ) != "" )
        mOptions.detail_tile = atoi( config.getSetting( "DetailTile" ) );

    mOptions.max_mipmap = atoi( config.getSetting( "MaxMipMapLevel" ) );

    mOptions.scalex = atof( config.getSetting( "ScaleX" ) );

    mOptions.scaley = atof( config.getSetting( "ScaleY" ) );

    mOptions.scalez = atof( config.getSetting( "ScaleZ" ) );

    mOptions.max_pixel_error = atoi( config.getSetting( "MaxPixelError" ) );

    mOptions.size = atoi( config.getSetting( "TileSize" ) );

    String terrain_filename = config.getSetting( "Terrain" );

    String detail_texture = config.getSetting( "DetailTexture" );

    String world_texture = config.getSetting( "WorldTexture" );

    if ( config.getSetting( "VertexColors" ) == "yes" )
        mOptions.colored = true;

    if ( config.getSetting( "VertexNormals" ) == "yes" )
        mOptions.lit = true;

    if ( config.getSetting( "UseTriStrips" ) == "yes" )
        setUseTriStrips(true);

    mScale = Vector3( mOptions.scalex, mOptions.scaley, mOptions.scalez );

    mTileSize = mOptions.size;

    Image image;

    image.load( terrain_filename );

    //check to make sure it's 2^n + 1 size.
    if ( image.getWidth() != image.getHeight() ||
            ! _checkSize( image.getWidth() ) )
    {
        String err = "Error: Invalid heightmap size : " +
                     StringConverter::toString( image.getWidth() ) +
                     "," + StringConverter::toString( image.getHeight() ) +
                     ". Should be 2^n+1, 2^n+1";
        Except( Exception::ERR_INVALIDPARAMS, err, "TerrainSceneManager::setWorldGeometry" );
    }

    if ( image.getFormat() != PF_L8 )
    {
        Except( Exception::ERR_INVALIDPARAMS, "Error: Image is not a grayscale image.",
                "TerrainSceneManager::setWorldGeometry" );
    }

    const uchar *data = image. getData();

    int size = image.getWidth();

    // set up the octree size.
    float max_x = mOptions.scalex * size;

    float max_y = 255 * mOptions.scaley;

    float max_z = mOptions.scalez * size;

    resize( AxisAlignedBox( 0, 0, 0, max_x, max_y, max_z ) );



    mOptions.data = data;

    mOptions.world_size = size;

    mTerrainMaterial = createMaterial( "Terrain" );

    if ( world_texture != "" )
        mTerrainMaterial->getTechnique(0)->getPass(0)->createTextureUnitState( world_texture, 0 );

    if ( detail_texture != "" )
    {
        mTerrainMaterial->getTechnique(0)->getPass(0)->createTextureUnitState( detail_texture, 1 );
    }

    mTerrainMaterial -> setLightingEnabled( mOptions.lit );

    mTerrainMaterial->load();


    //create a root terrain node.
    mTerrainRoot = getRootSceneNode() -> createChildSceneNode( "Terrain" );

    //setup the tile array.
    int num_tiles = ( mOptions.world_size - 1 ) / ( mOptions.size - 1 );

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
    size = ( int ) mTiles.size();

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

    /*
     for ( j = 0; j < size; j++ )
     {
         for ( i = 0; i < size; i++ )
         {
             mTiles[ i ][ j ] -> _generateVertexLighting( Vector3( 255, 50, 255 ), ColourValue( .25, .25, .25 ) );
         }
     }
     */



    //    intersectSegment( Vector3(255, getHeightAt(255,255), 255), Vector3(255,50,255), 0 );


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

} //namespace

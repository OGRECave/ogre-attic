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
#include <OgreCamera.h>
#include "OgreException.h"
#include "OgreStringConverter.h"

#include <fstream>

namespace Ogre
{

TerrainSceneManager::TerrainSceneManager() : OctreeSceneManager( )
{
    //setDisplaySceneNodes( true );
    //setShowBoxes( true );
}

TerrainSceneManager::~TerrainSceneManager()
{}

void TerrainSceneManager::setWorldGeometry( const String& filename )
{
    int i, j;

    /* Set up the options */
    ConfigFile config;
    TerrainOptions options;

    config.load( filename );

    if ( config.getSetting( "DetailTile" ) != "" )
        options.detail_tile = atoi( config.getSetting( "DetailTile" ) );

    options.max_mipmap = atoi( config.getSetting( "MaxMipMapLevel" ) );

    options.scalex = atof( config.getSetting( "ScaleX" ) );

    options.scaley = atof( config.getSetting( "ScaleY" ) );

    options.scalez = atof( config.getSetting( "ScaleZ" ) );

    options.max_pixel_error = atoi( config.getSetting( "MaxPixelError" ) );

    options.size = atoi( config.getSetting( "TileSize" ) );

    String terrain_filename = config.getSetting( "Terrain" );

    String detail_texture = config.getSetting( "DetailTexture" );

    String world_texture = config.getSetting( "WorldTexture" );

    if ( config.getSetting( "VertexColors" ) == "yes" )
        options.colored = true;

    if ( config.getSetting( "VertexNormals" ) == "yes" )
        options.lit = true;

    mScale = Vector3( options.scalex, options.scaley, options.scalez );

    mTileSize = options.size;

    mImage = new Image();

    mImage -> load( terrain_filename );

    //check to make sure it's 2^n + 1 size.
    if ( mImage->getWidth() != mImage->getHeight() ||
            ! _checkSize( mImage->getWidth() ) )
    {
        String err = "Error: Invalid heightmap size : " +
                     StringConverter::toString( mImage->getWidth() ) +
                     "," + StringConverter::toString( mImage->getHeight() ) +
                     ". Should be 2^n+1, 2^n+1";
        Except( Exception::ERR_INVALIDPARAMS, err, "TerrainSceneManager::setWorldGeometry" );
    }

    if ( mImage -> getFormat() != Image::FMT_GREY )
    {
        Except( Exception::ERR_INVALIDPARAMS, "Error: Image is not a grayscale image.",
                "TerrainSceneManager::setWorldGeometry" );
    }

    const uchar *data = mImage -> getData();

    int size = mImage->getWidth();

    // set up the octree size.
    float max_x = options.scalex * size;

    float max_y = 255 * size;

    float max_z = options.scalez * size;

    resize( AxisAlignedBox( 0, 0, 0, max_x, max_y, max_z ) );



    options.data = data;

    options.world_size = size;

    mTerrainMaterial = createMaterial( "Terrain" );

    if ( world_texture != "" )
        mTerrainMaterial -> addTextureLayer( world_texture, 0 );

    if ( detail_texture != "" )
    {
        mTerrainMaterial -> addTextureLayer( detail_texture, 1 );
    }

    mTerrainMaterial -> setLightingEnabled( options.lit );

    //create a root terrain node.

    mTerrainRoot = getRootSceneNode() -> createChild( "Terrain" );

    //setup the tile array.
    int num_tiles = ( options.world_size - 1 ) / ( options.size - 1 );

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

    for ( j = 0; j < options.world_size - 1; j += ( options.size - 1 ) )
    {
        p = 0;

        for ( i = 0; i < options.world_size - 1; i += ( options.size - 1 ) )
        {
            options.startx = i;
            options.startz = j;
            sprintf( name, "tile[%d,%d]", p, q );

            SceneNode *c = mTerrainRoot -> createChild( name );
            TerrainRenderable *tile = new TerrainRenderable();

            tile -> setMaterial( mTerrainMaterial );
            tile -> init( options );

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

    for ( j = 0; j < size; j++ )
    {
        for ( i = 0; i < size; i++ )
        {
            mTiles[ i ][ j ] -> _calculateNormals( );

        }
    }

}


void TerrainSceneManager::_updateSceneGraph( Camera * cam )
{
    /*
    Vector3 c = cam -> getPosition();
    c.y = getHeightAt(c.x, c.z ) + 3;
    cam -> setPosition( c );
    */
    OctreeSceneManager::_updateSceneGraph( cam );
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

    OctreeSceneManager::_renderVisibleObjects();

    TerrainRenderable::mRenderedTris = 0;

}

void TerrainSceneManager::_findVisibleObjects ( Camera * cam )
{
    OctreeSceneManager::_findVisibleObjects( cam );
}

float TerrainSceneManager::getHeightAt( float x, float z )
{
    //find which tile.
    const uchar * heightmap = mImage->getData();
    //for now, assume a static terrain that has only been scaled, not moved.

    //unscale the point.
    int width = mImage -> getWidth();
    int depth = mImage -> getHeight();

    float unit_x = x / mScale.x;
    float unit_z = z / mScale.z;

    int index_x = ( int ) unit_x;
    int index_z = ( int ) unit_z;


    //take care of boundaries...
    if ( index_x < 0 && index_z < 0 )
        return ( float ) heightmap[ 0 ] * mScale.y;
    else if ( index_x >= width - 1 && index_z >= depth - 1 )
    {
        return ( float ) heightmap[ width * depth - 1 ] * mScale.y;
    }

    else if ( index_x >= width - 1 )
    {
        return ( float ) heightmap[ width * index_z ] * mScale.y;
    }

    else if ( index_z >= depth - 1 )
    {
        return ( float ) heightmap[ width * ( depth - 1 ) + index_x ] * mScale.y;
    }

    //we know we are in the terrain, so bilinear interpolate...

    Real a = heightmap[ width * index_z + index_x ];

    Real b = heightmap[ width * index_z + index_x + 1 ];

    Real c = heightmap[ width * ( index_z + 1 ) + index_x ];

    Real d = heightmap[ width * ( index_z + 1 ) + index_x + 1 ];

    Real delta_x = unit_x - index_x;

    Real delta_z = unit_z - index_z;

    Real top = a * ( 1 - delta_x ) + delta_x * b;

    Real bottom = c * ( 1 - delta_x ) + delta_x * d;

    return ( top * ( 1 - delta_z ) + delta_z * bottom ) * mScale.y;


}

} //namespace

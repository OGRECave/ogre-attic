/***************************************************************************
terrainrenderable.cpp  -  description
-------------------
begin                : Sat Oct 5 2002
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

#include "OgreTerrainRenderable.h"
#include <OgreSceneNode.h>
#include <OgreRenderQueue.h>
#include <OgreRenderOperation.h>
#include <OgreCamera.h>
#include <OgreRoot.h>


namespace Ogre
{

int TerrainRenderable::mRenderedTris = 0;

String TerrainRenderable::mType = "TerrainMipMap";

LevelArray TerrainRenderable::mLevelIndex;
bool TerrainRenderable::mLevelInit = false;

TerrainRenderable::TerrainRenderable()
{
    mForcedRenderLevel = -1;
    mTerrain.pVertices = 0;
    mTerrain.pNormals = 0;
    mTerrain.pColours = 0;
    mTerrain.pTexCoords[ 0 ] = 0;
    mTerrain.pTexCoords[ 1 ] = 0;
    mMinLevelDistSqr = 0;

    mInit = false;
    mRenderLevelChanged = true;

    mColored = false;
    mLit = false;

    for ( int i = 0; i < 4; i++ )
    {
        mNeighbors[ i ] = 0;
    }

    _initLevelIndexes();

}

TerrainRenderable::~TerrainRenderable()
{

    deleteGeometry();
}

void TerrainRenderable::deleteGeometry()
{
    if ( mTerrain.pVertices != 0 )
        delete mTerrain.pVertices;

    if ( mTerrain.pNormals != 0 )
        delete mTerrain.pNormals;

    if ( mTerrain.pColours != 0 )
        delete mTerrain.pColours;

    if ( mTerrain.pTexCoords[ 0 ] != 0 )
        delete mTerrain.pTexCoords[ 0 ];

    if ( mTerrain.pTexCoords[ 1 ] != 0 )
        delete mTerrain.pTexCoords[ 1 ];

    if ( mMinLevelDistSqr != 0 )
        delete mMinLevelDistSqr;
}

void TerrainRenderable::init( TerrainOptions &options )
{


    //ensure that the size works
    if ( ! _checkSize( options.size ) )
    {
        printf( "Terrain block must size: 2^n+1 ( it's %d )\n", options.size );
        return ;
    }

    if ( options.max_mipmap != 0 )
    {
        int i = ( int ) 1 << ( options.max_mipmap - 1 ) ;

        if ( ( i + 1 ) > options.size )
        {
            printf( "Invalid maximum mipmap specifed, must be n, such that 2^(n-1)+1 < options.size \n" );
            return ;
        }
    }

    deleteGeometry();

    //calculate min and max heights;
    Real min = 256000, max = 0;

    int m = 0;

    int q = 0;

    /* Initialize the variables */
    mLit = options.lit;

    mColored = options.colored;

    mNearPlane = options.near_plane;

    mMaxPixelError = options.max_pixel_error;

    mVertResolution = options.vert_res;

    mTopCoord = options.top_coord;

    mSize = options.size;

    mWorldSize = options.world_size;

    mNumMipMaps = options.max_mipmap;

    mTerrain.numVertices = mSize * mSize;

    mTerrain.pVertices = new Real[ mSize * mSize * 3 ];

    mTerrain.pNormals = new Real[ mSize * mSize * 3 ];

    mTerrain.pTexCoords[ 0 ] = new Real[ mSize * mSize * 2 ];

    mTerrain.pTexCoords[ 1 ] = new Real[ mSize * mSize * 2 ];

    mTerrain.pColours = new unsigned long [ mSize * mSize ];

    mInit = true;

    mRenderLevel = 1;

    mMinLevelDistSqr = new Real[ mNumMipMaps ];


    mScale.x = options.scalex;

    mScale.y = options.scaley;

    mScale.z = options.scalez;


    for ( int i = 0; i < mSize*mSize; i++ )
    {
        mTerrain.pColours[ i ] = 0xFFFFFFFF;
    }


    int endx = options.startx + options.size;

    int endz = options.startz + options.size;

    Vector3 left, down, here;

    for ( int j = options.startz; j < endz; j++ )
    {
        for ( int i = options.startx; i < endx; i++ )
        {
            Real height = options._worldheight( i, j ) * options.scaley;
            mTerrain.pVertices[ m ] = ( Real ) i * options.scalex; //x
            mTerrain.pVertices[ m + 1 ] = height; //y
            mTerrain.pVertices[ m + 2 ] = ( Real ) j * options.scalez; //z

            mTerrain.pTexCoords[ 0 ][ q ] = ( Real ) i / ( Real ) options.world_size ;
            mTerrain.pTexCoords[ 0 ][ q + 1 ] = ( Real ) 1.0 - ( Real ) j / ( Real ) options.world_size;
            mTerrain.pTexCoords[ 1 ][ q ] = ( ( Real ) i / ( Real ) mSize ) * options.detail_tile;
            mTerrain.pTexCoords[ 1 ][ q + 1 ] = ( ( Real ) 1.0 - ( Real ) j / ( Real ) mSize ) * options.detail_tile;

            if ( height < min )
                min = ( Real ) height;

            if ( height > max )
                max = ( Real ) height;

            m += 3;

            q += 2;
        }
    }

    mBounds.setExtents( ( Real ) options.startx * options.scalex, min, ( Real ) options.startz * options.scalez,
                        ( Real ) ( endx - 1 ) * options.scalex, max, ( Real ) ( endz - 1 ) * options.scalez );


    mCenter = Vector3( ( options.startx * options.scalex + endx - 1 ) / 2,
                       ( min + max ) / 2,
                       ( options.startz * options.scalez + endz - 1 ) / 2 );


    Real C = _calculateCFactor();

    _calculateMinLevelDist2( C );

}

void TerrainRenderable::_getNormalAt( float x, float z, Vector3 * result )
{

    Vector3 here, left, down;
    here.x = x;
    here.y = getHeightAt( x, z );
    here.z = z;

    left.x = x - 1;
    left.y = getHeightAt( x - 1, z );
    left.z = z;

    down.x = x;
    down.y = getHeightAt( x, z + 1 );
    down.z = z + 1;

    left = left - here;

    down = down - here;

    left.normalise();
    down.normalise();

    *result = left.crossProduct( down );
    result -> normalise();

    // result->x = - result->x;
    // result->y = - result->y;
    // result->z = - result->z;
}

void TerrainRenderable::_calculateNormals()
{
    Vector3 norm;
    int m = 0;

    for ( int j = 0; j < mSize; j++ )
    {
        for ( int i = 0; i < mSize; i++ )
        {

            _getNormalAt( _vertex( i, j, 0 ), _vertex( i, j, 2 ), &norm );

            //  printf( "Normal = %5f,%5f,%5f\n", norm.x, norm.y, norm.z );

            mTerrain.pNormals[ m ] = norm.x;
            mTerrain.pNormals[ m + 1 ] = norm.y;
            mTerrain.pNormals[ m + 2 ] = norm.z;


            m += 3;
        }

    }

}

void TerrainRenderable::_notifyCurrentCamera( Camera* cam )
{

    if ( mForcedRenderLevel >= 0 )
    {
        mRenderLevel = mForcedRenderLevel;
        return ;
    }


    int old_level = mRenderLevel;

    Vector3 cpos = cam -> getPosition();
    Vector3 diff = mCenter - cpos;

    Real L = diff.squaredLength();

    current_L = L;


    mRenderLevel = -1;

    for ( int i = 0; i < mNumMipMaps; i++ )
    {
        if ( mMinLevelDistSqr[ i ] > L )
        {
            mRenderLevel = i - 1;
            break;
        }
    }

    if ( mRenderLevel < 0 )
        mRenderLevel = mNumMipMaps - 1;


    // mRenderLevel = 4;

}

void TerrainRenderable::_updateRenderQueue( RenderQueue* queue )
{
    queue -> addRenderable( this );
}

void TerrainRenderable::getRenderOperation( RenderOperation& rend )
{
    //setup indexes for vertices and uvs...

    if ( !mInit )
    {
        printf( "Uninitialized\n" );
        return ;
    }

    /*
    int j=0;
    for( j=0; j<mSize; j+= (mRenderLevel*2) ) //depth
    {
    int i=0;
    if( j != 0 )
    {
    //two degenerate tris to turn the corner...
    pIndexes[nupIndexes] = _index(i, j+mRenderLevel ); nupIndexes++;
    pIndexes[numIndexes] = _index(i, j+mRenderLevel ); numIndexes++;
}

    //forward strip...
    for( i=0; i<mSize; i+= mRenderLevel ) //accross
    {
    pIndexes[numIndexes] = _index(i, j ); numIndexes++;
    pIndexes[numIndexes] = _index(i, j+mRenderLevel ); numIndexes++;
}
    i -= mRenderLevel; //backtrack...


    //degenerate tris to turn the corner...

    pIndexes[numIndexes] = _index(i, j+mRenderLevel ); numIndexes++;
    pIndexes[numIndexes] = _index(i, j+mRenderLevel ); numIndexes++;

    //back strip
    for( i = mSize-mRenderLevel; i>=0; i-=mRenderLevel )
    {
    pIndexes[numIndexes] = _index(i, j+mRenderLevel ); numIndexes++;
    pIndexes[numIndexes] = _index(i, j ); numIndexes++;
}

}
    */

    int east = 0, west = 0, north = 0, south = 0;

    int step = 1 << mRenderLevel;

    int index_array = 0;

    int numIndexes = 0;

    TerrainIndexBuffer *buffer = 0;

    if ( mNeighbors[ EAST ] != 0 && mNeighbors[ EAST ] -> mRenderLevel > mRenderLevel )
    {
        east = step; index_array |= TILE_EAST;
    }

    if ( mNeighbors[ WEST ] != 0 && mNeighbors[ WEST ] -> mRenderLevel > mRenderLevel )
    {
        west = step; index_array |= TILE_WEST;
    }

    if ( mNeighbors[ NORTH ] != 0 && mNeighbors[ NORTH ] -> mRenderLevel > mRenderLevel )
    {
        north = step; index_array |= TILE_NORTH;
    }

    if ( mNeighbors[ SOUTH ] != 0 && mNeighbors[ SOUTH ] -> mRenderLevel > mRenderLevel )
    {
        south = step; index_array |= TILE_SOUTH;
    }

    if ( mLevelIndex[ mRenderLevel ][ index_array ] != 0 )
    {
        buffer = mLevelIndex[ mRenderLevel ][ index_array ];
    }

    else
    {
        int new_length = ( mSize / step ) * ( mSize / step ) * 2 * 2 * 2 ;
        //this is the maximum for a level.  It wastes a little, but shouldn't be a problem.
        buffer = new TerrainIndexBuffer( new_length );
        numIndexes = 0;

        for ( int j = north; j < mSize - 1 - south; j += step )
        {
            for ( int i = west; i < mSize - 1 - east; i += step )
            {

                //triangles
                buffer -> indexes[ numIndexes ] = _index( i, j ); numIndexes++;
                buffer -> indexes[ numIndexes ] = _index( i, j + step ); numIndexes++;
                buffer -> indexes[ numIndexes ] = _index( i + step, j ); numIndexes++;

                buffer -> indexes[ numIndexes ] = _index( i, j + step ); numIndexes++;
                buffer -> indexes[ numIndexes ] = _index( i + step, j + step ); numIndexes++;
                buffer -> indexes[ numIndexes ] = _index( i + step, j ); numIndexes++;

            }
        }

        int substep = step << 1;

        if ( west > 0 )
        {

            for ( int j = 0; j < mSize - 1; j += substep )
            {

                //skip the first bit of the corner if the north side is a different level as well.
                if ( j > 0 || north == 0 )
                {
                    buffer -> indexes[ numIndexes ] = _index( 0, j ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( step, j + step ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( step, j ); numIndexes++;
                }

                buffer -> indexes[ numIndexes ] = _index( step, j + step ); numIndexes++;
                buffer -> indexes[ numIndexes ] = _index( 0, j ); numIndexes++;
                buffer -> indexes[ numIndexes ] = _index( 0, j + step + step ); numIndexes++;

                if ( j < mSize - 1 - substep || south == 0 )
                {
                    buffer -> indexes[ numIndexes ] = _index( step, j + step ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( 0, j + step + step ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( step, j + step + step ); numIndexes++;

                }
            }

        }

        if ( east > 0 )
        {

            int x = mSize - 1;

            for ( int j = 0; j < mSize - 1; j += substep )
            {

                //skip the first bit of the corner if the north side is a different level as well.
                if ( j > 0 || north == 0 )
                {
                    buffer -> indexes[ numIndexes ] = _index( x, j ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( x - step, j ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( x - step, j + step ); numIndexes++;
                }

                buffer -> indexes[ numIndexes ] = _index( x, j ); numIndexes++;
                buffer -> indexes[ numIndexes ] = _index( x - step, j + step ); numIndexes++;
                buffer -> indexes[ numIndexes ] = _index( x, j + step + step ); numIndexes++;

                if ( j < mSize - 1 - substep || south == 0 )
                {
                    buffer -> indexes[ numIndexes ] = _index( x, j + step + step ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( x - step, j + step ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( x - step, j + step + step ); numIndexes++;
                }
            }

        }

        if ( south > 0 )
        {

            int x = mSize - 1;

            for ( int j = 0; j < mSize - 1; j += substep )
            {

                //skip the first bit of the corner if the north side is a different level as well.
                if ( j > 0 || west == 0 )
                {
                    buffer -> indexes[ numIndexes ] = _index( j, x - step ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( j, x ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( j + step, x - step ); numIndexes++;
                }

                buffer -> indexes[ numIndexes ] = _index( j + step, x - step ); numIndexes++;
                buffer -> indexes[ numIndexes ] = _index( j, x ); numIndexes++;
                buffer -> indexes[ numIndexes ] = _index( j + step + step, x ); numIndexes++;

                if ( j < mSize - 1 - substep || east == 0 )
                {
                    buffer -> indexes[ numIndexes ] = _index( j + step, x - step ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( j + step + step, x ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( j + step + step, x - step ); numIndexes++;
                }
            }

        }

        if ( north > 0 )
        {

            for ( int j = 0; j < mSize - 1; j += substep )
            {

                //skip the first bit of the corner if the north side is a different level as well.
                if ( j > 0 || west == 0 )
                {
                    buffer -> indexes[ numIndexes ] = _index( j, 0 ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( j, step ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( j + step, step ); numIndexes++;
                }

                buffer -> indexes[ numIndexes ] = _index( j, 0 ); numIndexes++;
                buffer -> indexes[ numIndexes ] = _index( j + step, step ); numIndexes++;
                buffer -> indexes[ numIndexes ] = _index( j + step + step, 0 ); numIndexes++;

                if ( j < mSize - 1 - substep || east == 0 )
                {
                    buffer -> indexes[ numIndexes ] = _index( j + step + step, 0 ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( j + step, step ); numIndexes++;
                    buffer -> indexes[ numIndexes ] = _index( j + step + step, step ); numIndexes++;
                }
            }

        }

        buffer -> length = numIndexes;
        mLevelIndex[ mRenderLevel ][ index_array ] = buffer;
    }


    rend.useIndexes = true;
    rend.vertexOptions = RenderOperation::VO_TEXTURE_COORDS;

    if ( mColored )
        rend.vertexOptions |= RenderOperation::VO_DIFFUSE_COLOURS;

    if ( mLit )
        rend.vertexOptions |= RenderOperation::VO_NORMALS;

    rend.operationType = RenderOperation::OT_TRIANGLE_LIST;

    if ( mColored )
    {
        rend.pDiffuseColour = mTerrain.pColours;
        rend.diffuseStride = 0;
    }

    else
        rend.pDiffuseColour = 0;

    rend.numTextureCoordSets = 2;

    rend.numTextureDimensions[ 0 ] = 2;

    rend.numTextureDimensions[ 1 ] = 2;

    rend.pTexCoords[ 0 ] = mTerrain.pTexCoords[ 0 ];

    rend.pTexCoords[ 1 ] = mTerrain.pTexCoords[ 1 ];

    rend.texCoordStride[ 0 ] = 0;

    rend.texCoordStride[ 1 ] = 0;

    if ( mLit )
        rend.pNormals = mTerrain.pNormals;
    else
        rend.pNormals = 0;

    rend.numVertices = mTerrain.numVertices;



    rend.pVertices = mTerrain.pVertices;

    rend.vertexStride = 0;

    rend.pIndexes = buffer -> indexes;

    rend.numIndexes = buffer -> length;

    mRenderedTris += ( rend.numIndexes / 3 );


    mRenderLevelChanged = false;

}

void TerrainRenderable::getWorldTransforms( Matrix4* xform )
{
    *xform = Matrix4::IDENTITY;
    *xform = *xform * mParentNode->_getFullTransform();
}

bool TerrainRenderable::_checkSize( int n )
{
    for ( int i = 0; i < 10; i++ )
    {
        if ( ( ( 1 << i ) + 1 ) == n )
            return true;
    }

    return false;
}


void TerrainRenderable::_calculateMinLevelDist2( Real C )
{
    //level 0 has no delta.
    mMinLevelDistSqr[ 0 ] = 0;

    for ( int level = 1; level < mNumMipMaps; level++ )
    {
        mMinLevelDistSqr[ level ] = 0;

        int step = 1 << level;

        for ( int j = 0; j < mSize - step; j += step )
        {
            for ( int i = 0; i < mSize - step; i += step )
            {
                //check each height inbetween the steps.
                Real h1 = _vertex( i, j, 1 );
                Real h2 = _vertex( i + step, j, 1 );
                Real h3 = _vertex( i + step, j + step, 1 );
                Real h4 = _vertex( i, j + step, 1 );

                for ( int z = 1; z < step; z++ )
                {
                    for ( int x = 1; x < step; x++ )
                    {

                        Real zpct = z / step;
                        Real xpct = x / step;

                        //interpolated height
                        float top = h3 * ( 1.0f - xpct ) + xpct * h4;
                        float bottom = h1 * ( 1.0f - xpct ) + xpct * h2;

                        Real interp_h = top * ( 1.0f - zpct ) + zpct * bottom;

                        Real actual_h = _vertex( i + x, j + z, 1 );
                        Real delta = fabs( interp_h - actual_h );

                        Real D2 = delta * delta * C * C;

                        if ( mMinLevelDistSqr[ level ] < D2 )
                            mMinLevelDistSqr[ level ] = D2;
                    }

                }
            }
        }

    }

    //make sure the levels are increasing...
    for ( int i = 1; i < mNumMipMaps; i++ )
    {
        if ( mMinLevelDistSqr[ i ] < mMinLevelDistSqr[ i - 1 ] )
            mMinLevelDistSqr[ i ] = mMinLevelDistSqr[ i - 1 ] + 1;
    }

}

void TerrainRenderable::_initLevelIndexes()
{
    if ( mLevelInit )
        return ;


    if ( mLevelIndex.size() == 0 )
    {
        for ( int i = 0; i < 16; i++ )
        {

            mLevelIndex.push_back( IndexArray() );

            for ( int j = 0; j < 16; j++ )
            {
                mLevelIndex[ i ].push_back( 0 );
            }

        }


    }

    mLevelInit = true;
}

void TerrainRenderable::_adjustRenderLevel( int i )
{

    mRenderLevel = i;
    _alignNeighbors();

}

void TerrainRenderable::_alignNeighbors()
{

    //ensure that there aren't any gaps...

    for ( int i = 0; i < 4;i++ )
    {
        if ( mNeighbors[ i ] != 0 && mNeighbors[ i ] ->mRenderLevel + 1 < mRenderLevel )
            mNeighbors[ i ] -> _adjustRenderLevel( mRenderLevel - 1 );
    }

    /*

    //always go up, rather than down...
          if ( mNeighbors[ EAST  ] != 0 && mNeighbors[ EAST ] ->mRenderLevel + 1 < mRenderLevel )
                 mNeighbors[ EAST ] -> _adjustRenderLevel(  mRenderLevel - 1 );

            if ( mNeighbors[ SOUTH  ] != 0 && mNeighbors[ SOUTH ] ->mRenderLevel + 1 < mRenderLevel )
                mNeighbors[ SOUTH ] -> _adjustRenderLevel(  mRenderLevel - 1 );
    */
}

Real TerrainRenderable::_calculateCFactor()
{
    Real A, T;

    A = ( Real ) mNearPlane / fabs( ( Real ) mTopCoord );

    T = 2 * ( Real ) mMaxPixelError / ( Real ) mVertResolution;

    return A / T;
}

float TerrainRenderable::getHeightAt( float x, float z )
{
    Vector3 start;
    Vector3 end;

    start.x = _vertex( 0, 0, 0 );
    start.y = _vertex( 0, 0, 1 );
    start.z = _vertex( 0, 0, 2 );

    end.x = _vertex( mSize - 1, mSize - 1, 0 );
    end.y = _vertex( mSize - 1, mSize - 1, 1 );
    end.z = _vertex( mSize - 1, mSize - 1, 2 );

    /* Safety catch, if the point asked for is outside
     * of this tile, it will ask the appropriate tile
     */

    if ( x < start.x )
    {
        if ( mNeighbors[ WEST ] != 0 )
            return mNeighbors[ WEST ] ->getHeightAt( x, z );
        else
            x = start.x;
    }

    if ( x > end.x )
    {
        if ( mNeighbors[ EAST ] != 0 )
            return mNeighbors[ EAST ] ->getHeightAt( x, z );
        else
            x = end.x;
    }

    if ( z < start.z )
    {
        if ( mNeighbors[ NORTH ] != 0 )
            return mNeighbors[ NORTH ] ->getHeightAt( x, z );
        else
            z = start.z;
    }

    if ( z > end.z )
    {
        if ( mNeighbors[ SOUTH ] != 0 )
            return mNeighbors[ SOUTH ] ->getHeightAt( x, z );
        else
            z = end.z;
    }



    float x_pct = ( x - start.x ) / ( end.x - start.x );
    float z_pct = ( z - start.z ) / ( end.z - start.z );

    float x_pt = x_pct * ( float ) ( mSize - 1 );
    float z_pt = z_pct * ( float ) ( mSize - 1 );

    int x_index = ( int ) x_pt;
    int z_index = ( int ) z_pt;

    x_pct = x_pt - x_index;
    z_pct = z_pt - z_index;

    //bilinear interpolate to find the height.

    float t1 = _vertex( x_index, z_index, 1 );
    float t2 = _vertex( x_index + 1, z_index, 1 );
    float b1 = _vertex( x_index, z_index + 1, 1 );
    float b2 = _vertex( x_index + 1, z_index + 1, 1 );

    float t = ( t1 * ( 1 - x_pct ) ) + ( t2 * ( x_pct ) );
    float b = ( b1 * ( 1 - x_pct ) ) + ( b2 * ( x_pct ) );

    float h = ( t * ( 1 - z_pct ) ) + ( b * ( z_pct ) );

    return h;
}

bool TerrainRenderable::intersectSegment( const Vector3 & start, const Vector3 & end, Vector3 * result )
{
    Vector3 dir = end - start;
    Vector3 ray = start;

    dir.normalise();

    //dir.x *= mScale.x;
    //dir.y *= mScale.y;
    //dir.z *= mScale.z;

    const Vector3 * corners = getBoundingBox().getAllCorners();

    //start with the next one...
    ray += dir;

    while ( ! ( ( ray.x < corners[ 0 ].x ) ||
                ( ray.x > corners[ 4 ].x ) ||
                ( ray.z < corners[ 0 ].z ) ||
                ( ray.z > corners[ 4 ].z ) ) )
    {
        //do quick checks...
        /*
        if( ray.y < corners[0].y && end.y < corners[0].y )
          {
            if( result != 0 )
              *result = Vector3(-1, -1, -1);
            return false;
          }
        if( ray.y > corners[4].y && end.y > corners[4].y )
          {
            if( result != 0 )
              *result = Vector3(-1, -1, -1);
            return false;
          }
            
              if( ray.y < corners[0].y && end.y < corners[0].y )
              {
              if( result != 0 )
              *result = Vector3(-1, -1, -1);
              return false;
              }
              
              if( ray.y > corners[4].y && end.y > corners[4].y )
              {
              if( result != 0 )
              *result = Vector3(-1, -1, -1);
              return false;
              }
        */

        float h = getHeightAt( ray.x, ray.z );
        // printf( "Ray = %f,%f,%f  height = %f\n", ray.x, ray.y, ray.z, h);

        if ( ray.y <= h )
        {
            if ( result != 0 )
                * result = ray;

            return true;
        }

        else
        {
            ray += dir;
        }

    }

    if ( ray.x < corners[ 0 ].x && mNeighbors[ WEST ] != 0 )
        return mNeighbors[ WEST ] ->intersectSegment( ray, end, result );
    else if ( ray.z < corners[ 0 ].z && mNeighbors[ NORTH ] != 0 )
        return mNeighbors[ NORTH ] ->intersectSegment( ray, end, result );
    else if ( ray.x > corners[ 4 ].x && mNeighbors[ EAST ] != 0 )
        return mNeighbors[ EAST ] ->intersectSegment( ray, end, result );
    else if ( ray.z > corners[ 4 ].z && mNeighbors[ SOUTH ] != 0 )
        return mNeighbors[ SOUTH ] ->intersectSegment( ray, end, result );
    else
    {
        if ( result != 0 )
            * result = Vector3( -1, -1, -1 );

        return false;
    }
}

void TerrainRenderable::_generateVertexLighting( const Vector3 &sun, ColourValue ambient )
{
    if ( !mColored )
    {
        printf( "Can't generate terrain vertex lighting with out vertex colors enabled.\n" );
        return ;
    }


    Vector3 pt;
    Vector3 normal;
    Vector3 light;

    //for each point in the terrain, see if it's in the line of sight for the sun.
    for ( int i = 0; i < mSize; i++ )
    {
        for ( int j = 0; j < mSize; j++ )
        {
            pt.x = _vertex( i, j, 0 );
            pt.y = _vertex( i, j, 1 );
            pt.z = _vertex( i, j, 2 );

            light = sun - pt;

            light.normalise();

            if ( ! intersectSegment( pt, sun, 0 ) )
            {
                //
                _getNormalAt( i, j, &normal );

                float l = light.dotProduct( normal );

                ColourValue v;
                v.r = ambient.r + l;
                v.g = ambient.g + l;
                v.b = ambient.b + l;

                if ( v.r > 1 ) v.r = 1;

                if ( v.g > 1 ) v.g = 1;

                if ( v.b > 1 ) v.b = 1;

                if ( v.r < 0 ) v.r = 0;

                if ( v.g < 0 ) v.g = 0;

                if ( v.b < 0 ) v.b = 0;

                Root::getSingleton().convertColourValue( v, & ( mTerrain.pColours[ _index( i, j ) ] ) );

            }

            else
            {
                Root::getSingleton().convertColourValue( ambient, & ( mTerrain.pColours[ _index( i, j ) ] ) );
            }

        }

    }

    printf( "." );
}


} //namespace

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



namespace Ogre
{

int TerrainRenderable::mRenderedTris = 0;

String TerrainRenderable::mType = "TerrainMipMap";

LevelArray TerrainRenderable::mLevelIndex;
bool TerrainRenderable::mLevelInit = false;

TerrainRenderable::TerrainRenderable()
{
    mTerrain.pVertices = 0;
    mTerrain.pNormals = 0;
    mTerrain.pColours = 0;
    mTerrain.pTexCoords[0] = 0;
    mTerrain.pTexCoords[1] = 0;
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
    if(mTerrain.pVertices != 0)
        delete mTerrain.pVertices;
    if(mTerrain.pNormals != 0 )
        delete mTerrain.pNormals;
    if(mTerrain.pColours != 0 )
        delete mTerrain.pColours;
    if(mTerrain.pTexCoords[0] != 0 )
        delete mTerrain.pTexCoords[0];
    if(mTerrain.pTexCoords[1] != 0 )
        delete mTerrain.pTexCoords[1];
    if(mMinLevelDistSqr != 0 )
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

    mColors = new unsigned long [ mSize * mSize ];

    mInit = true;

    mRenderLevel = 1;

    mMinLevelDistSqr = new Real[ mNumMipMaps ];




    for ( int i = 0; i < mSize*mSize; i++ )
    {
        mColors[ i ] = 0xFF0000FF;
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

void TerrainRenderable::_calculateNormals()
{
    Vector3 left, down, here, norm;
    int m = 0;

    for ( int j = 0; j < mSize; j++ )
    {
        for ( int i = 0; i < mSize; i++ )
        {

            here.x = _vertex( i, j, 0 );
            here.y = _vertex( i, j, 1 );
            here.z = _vertex( i, j, 2 );

            if ( i == 0 )
            {

                if ( mNeighbors[ WEST ] != 0 )
                {

                    left.x = mNeighbors[ WEST ] -> _vertex( mSize - 2, j, 0 );
                    left.y = mNeighbors[ WEST ] -> _vertex( mSize - 2, j, 1 );
                    left.z = mNeighbors[ WEST ] -> _vertex( mSize - 2, j, 2 );
                }

                else
                {
                    left.x = here.x - 1;
                    left.y = here.y;
                    left.z = here.z;
                }

            }

            else
            {
                left.x = _vertex( i - 1, j, 0 );
                left.y = _vertex( i - 1, j, 1 );
                left.z = _vertex( i - 1, j, 2 );
            }


            if ( j == mSize - 1 )
            {

                if ( mNeighbors[ SOUTH ] != 0 )
                {
                    down.x = mNeighbors[ SOUTH ] -> _vertex( i, 1, 0 );
                    down.y = mNeighbors[ SOUTH ] -> _vertex( i, 1, 1 );
                    down.z = mNeighbors[ SOUTH ] -> _vertex( i, 1, 2 );
                }

                else
                {
                    down.x = here.x;
                    down.y = here.y;
                    down.z = here.z + 1;
                }
            }

            else
            {
                down.x = _vertex( i, j + 1, 0 );
                down.y = _vertex( i, j + 1, 1 );
                down.z = _vertex( i, j + 1, 2 );

            }


            left = left - here;
            down = down - here;

            left.normalise();
            down.normalise();

            norm = left.crossProduct( down );
            norm.normalise();

            mTerrain.pNormals[ m ] = norm.x;
            mTerrain.pNormals[ m + 1 ] = norm.y;
            mTerrain.pNormals[ m + 2 ] = norm.z;


            m += 3;
        }

    }

}

void TerrainRenderable::_notifyCurrentCamera( Camera* cam )
{
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
        rend.pDiffuseColour = mColors;
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


void TerrainRenderable::_alignNeighbors()
{

    //ensure that there aren't any gaps...
    for ( int i = 0; i < 4;i++ )
    {
        if ( mNeighbors[ i ] != 0 && mNeighbors[ i ] ->mRenderLevel + 1 < mRenderLevel )
            mRenderLevel = mNeighbors[ i ] ->mRenderLevel + 1;
    }
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
    float offset_x, offset_z;
    float tile_x, tile_z;
    int index_x, index_z;
    float delta_x, delta_z;

    offset_x = x - _vertex( 0, 0, 0 );
    offset_z = z - _vertex( 0, 0, 2 );

    tile_x = ( offset_x / _vertex( mSize - 1, mSize - 1, 0 ) ) * mSize;
    tile_z = ( offset_z / _vertex( mSize - 1, mSize - 1, 2 ) ) * mSize;

    index_x = ( int ) tile_x;
    index_z = ( int ) tile_z;

    delta_x = tile_x - index_x;
    delta_z = tile_z - index_z;

    Real h1, h2, h3, h4;

    h1 = _vertex( index_x, index_z, 1 );

    if ( index_x + 1 < mSize )
        h2 = _vertex( index_x + 1, index_z, 1 );
    else if ( mNeighbors[ EAST ] != 0 )
        h2 = mNeighbors[ EAST ] ->_vertex( 1, index_z, 1 );
    else
        h2 = _vertex( index_x, index_z, 1 );

    if ( index_x + 1 < mSize && index_z + 1 < mSize )
        h3 = _vertex( index_x + 1, index_z + 1, 1 );
    else if ( index_x + 1 < mSize && mNeighbors[ SOUTH ] != 0 )
        h3 = mNeighbors[ SOUTH ] ->_vertex( index_x + 1, 1, 1 );
    else if ( index_z + 1 < mSize && mNeighbors[ EAST ] != 0 )
        h3 = mNeighbors[ EAST ] -> _vertex( 1, index_z + 1, 1 );
    else if ( mNeighbors[ EAST ] -> mNeighbors[ SOUTH ] != 0 )
        h3 = mNeighbors[ EAST ] -> mNeighbors[ SOUTH ] -> _vertex( 1, 1, 1 );
    else
        h3 = _vertex( index_x, index_z, 1 );

    if ( index_z < mSize )
        h4 = _vertex( index_x, index_z + 1, 1 );
    else if ( mNeighbors[ SOUTH ] != 0 )
        h4 = mNeighbors[ SOUTH ] ->_vertex( index_x, 1, 1 );
    else
        h4 = _vertex( index_x, index_z, 1 );

    //interpolated height
    float top = h4 * ( 1.0f - delta_x ) + delta_x * h3;

    float bottom = h1 * ( 1.0f - delta_x ) + delta_x * h2;

    return top*( 1.0f - delta_z ) + delta_z * bottom;

}

} //namespace

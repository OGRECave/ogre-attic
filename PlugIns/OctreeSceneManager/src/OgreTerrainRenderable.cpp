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
terrainrenderable.cpp  -  description
-------------------
begin                : Sat Oct 5 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net

Enhancements 2003 - 2004 (C) The OGRE Team

***************************************************************************/

#include "OgreTerrainRenderable.h"
#include <OgreSceneNode.h>
#include <OgreRenderQueue.h>
#include <OgreRenderOperation.h>
#include <OgreCamera.h>
#include <OgreRoot.h>

#include "OgreLogManager.h"
#include "OgreStringConverter.h"

namespace Ogre
{
#define MAIN_BINDING 0
#define DELTA_BINDING 1
    /*
#define POSITION_BINDING 0
#define NORMAL_BINDING 1
#define TEXCOORD_BINDING 2
#define COLOUR_BINDING 3
    */

TerrainBufferCache gIndexCache;

String TerrainRenderable::mType = "TerrainMipMap";
bool TerrainRenderable::msUseTriStrips = false;
bool TerrainRenderable::msUseLODMorph = false;
Real TerrainRenderable::msLODMorphStart = 0.5f;

LevelArray TerrainRenderable::mLevelIndex;
bool TerrainRenderable::mLevelInit = false;

TerrainRenderable::TerrainRenderable(const String& name)
    : mName(name), mTerrain(0), mPositionBuffer(0), mDeltaBuffers(0)
{
    mForcedRenderLevel = -1;
    mLastNextLevel = -1;

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
    _destroyLevelIndexes();
}

void TerrainRenderable::deleteGeometry()
{
    if(mTerrain)
        delete mTerrain;

    if (mPositionBuffer)
        delete [] mPositionBuffer;

    if (mDeltaBuffers)
        delete [] mDeltaBuffers;

    if ( mMinLevelDistSqr != 0 )
        delete [] mMinLevelDistSqr;
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

    mTerrain = new VertexData;
    mTerrain->vertexStart = 0;
    mTerrain->vertexCount = mSize * mSize;

    VertexDeclaration* decl = mTerrain->vertexDeclaration;
    VertexBufferBinding* bind = mTerrain->vertexBufferBinding;

    // positions
    size_t offset = 0;
    decl->addElement(MAIN_BINDING, offset, VET_FLOAT3, VES_POSITION);
    offset += VertexElement::getTypeSize(VET_FLOAT3);
    if (mLit)
    {
        decl->addElement(MAIN_BINDING, offset, VET_FLOAT3, VES_NORMAL);
        offset += VertexElement::getTypeSize(VET_FLOAT3);
    }
    // texture coord sets
    decl->addElement(MAIN_BINDING, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(MAIN_BINDING, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1);
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    if (mColored)
    {
        decl->addElement(MAIN_BINDING, offset, VET_COLOUR, VES_DIFFUSE);
        offset += VertexElement::getTypeSize(VET_COLOUR);
    }

    // Create shared vertex buffer
    mMainBuffer =
        HardwareBufferManager::getSingleton().createVertexBuffer(
            decl->getVertexSize(MAIN_BINDING),
            mTerrain->vertexCount, 
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    // Create system memory copy with just positions in it, for use in simple reads
    mPositionBuffer = new Real[mTerrain->vertexCount * 3];

    bind->setBinding(MAIN_BINDING, mMainBuffer);

    if (msUseLODMorph)
    {
        // Create additional element for delta
        decl->addElement(DELTA_BINDING, 0, VET_FLOAT1, VES_BLEND_WEIGHTS);
        // NB binding is not set here, it is set when deriving the LOD
    }


    mInit = true;

    mRenderLevel = 1;

    mMinLevelDistSqr = new Real[ mNumMipMaps ];


    mScale.x = options.scalex;

    mScale.y = options.scaley;

    mScale.z = options.scalez;

    int endx = options.startx + options.size;

    int endz = options.startz + options.size;

    Vector3 left, down, here;

    const VertexElement* poselem = decl->findElementBySemantic(VES_POSITION);
    const VertexElement* texelem0 = decl->findElementBySemantic(VES_TEXTURE_COORDINATES, 0);
    const VertexElement* texelem1 = decl->findElementBySemantic(VES_TEXTURE_COORDINATES, 1);
    Real* pSysPos = mPositionBuffer;

    unsigned char* pBase = static_cast<unsigned char*>(mMainBuffer->lock(HardwareBuffer::HBL_DISCARD));

    for ( int j = options.startz; j < endz; j++ )
    {
        for ( int i = options.startx; i < endx; i++ )
        {
            Real *pPos, *pTex0, *pTex1;
            poselem->baseVertexPointerToElement(pBase, &pPos);
            texelem0->baseVertexPointerToElement(pBase, &pTex0);
            texelem1->baseVertexPointerToElement(pBase, &pTex1);

            Real height = options._worldheight( i, j );

            *pSysPos++ = *pPos++ = ( Real ) i * options.scalex; //x
            *pSysPos++ = *pPos++ = height; //y
            *pSysPos++ = *pPos++ = ( Real ) j * options.scalez; //z

            *pTex0++ = ( Real ) i / ( Real ) options.world_size ;
            *pTex0++ = ( Real ) ( Real ) j / ( Real ) options.world_size;

            *pTex1++ = ( ( Real ) i / ( Real ) mSize ) * options.detail_tile;
            *pTex1++ = ( ( Real ) ( Real ) j / ( Real ) mSize ) * options.detail_tile;

            if ( height < min )
                min = ( Real ) height;

            if ( height > max )
                max = ( Real ) height;

            pBase += mMainBuffer->getVertexSize();
        }
    }

    mMainBuffer->unlock();

    mBounds.setExtents( ( Real ) options.startx * options.scalex, min, ( Real ) options.startz * options.scalez,
                        ( Real ) ( endx - 1 ) * options.scalex, max, ( Real ) ( endz - 1 ) * options.scalez );


    mCenter = Vector3( ( options.startx * options.scalex + (endx - 1) * options.scalex ) / 2,
                       ( min + max ) / 2,
                       ( options.startz * options.scalez + (endz - 1) * options.scalez ) / 2 );

    // Create delta buffer list if required to morph
    if (msUseLODMorph)
    {
        // Create delta buffer for all except the lowest mip
        mDeltaBuffers = new HardwareVertexBufferSharedPtr[mNumMipMaps - 1];
    }

    Real C = _calculateCFactor();

    _calculateMinLevelDist2( C );
    if (mLit)
        _calculateNormals();

}

void TerrainRenderable::_getNormalAt( float x, float z, Vector3 * result )
{

    assert(mLit && "No normals present");

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

    assert (mLit && "No normals present");

    HardwareVertexBufferSharedPtr vbuf = 
        mTerrain->vertexBufferBinding->getBuffer(MAIN_BINDING);
    const VertexElement* elem = mTerrain->vertexDeclaration->findElementBySemantic(VES_NORMAL);
    unsigned char* pBase = static_cast<unsigned char*>( vbuf->lock(HardwareBuffer::HBL_DISCARD) );
    Real* pNorm;

    for ( int j = 0; j < mSize; j++ )
    {
        for ( int i = 0; i < mSize; i++ )
        {

            _getNormalAt( _vertex( i, j, 0 ), _vertex( i, j, 2 ), &norm );

            //  printf( "Normal = %5f,%5f,%5f\n", norm.x, norm.y, norm.z );
            elem->baseVertexPointerToElement(pBase, &pNorm);
            *pNorm++ = norm.x;
            *pNorm++ = norm.y;
            *pNorm++ = norm.z;
            pBase += vbuf->getVertexSize();
        }

    }
    vbuf->unlock();
}

void TerrainRenderable::_notifyCurrentCamera( Camera* cam )
{

    if ( mForcedRenderLevel >= 0 )
    {
        mRenderLevel = mForcedRenderLevel;
        return ;
    }


    int old_level = mRenderLevel;

    Vector3 cpos = cam -> getDerivedPosition();
    Vector3 diff = mCenter - cpos;

    Real L = diff.squaredLength();

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

    if (msUseLODMorph)
    {
        // Get the next LOD level down
        int nextLevel = mNextLevelDown[mRenderLevel];
        if (nextLevel == 0)
        {
            // No next level, so never morph
            mLODMorphFactor = 0;
        }
        else
        {
            // Set the morph such that the morph happens in the last 0.25 of
            // the distance range
            Real range = mMinLevelDistSqr[nextLevel] - mMinLevelDistSqr[mRenderLevel];
            Real percent = (L - mMinLevelDistSqr[mRenderLevel]) / range;
            // scale result so that msLODMorphStart == 0, 1 == 1, clamp to 0 below that
            Real rescale = 1.0f / (1.0f - msLODMorphStart);
            mLODMorphFactor = std::max((percent - msLODMorphStart) * rescale, 0.0f);

            assert(mLODMorphFactor >= 0 && mLODMorphFactor <= 1);
        }

        // Bind the correct delta buffer if it has changed
        // nextLevel - 1 since the first entry is for LOD 1 (since LOD 0 never needs it)
        if (mLastNextLevel != nextLevel)
        {
            if (nextLevel > 0)
            {
                mTerrain->vertexBufferBinding->setBinding(DELTA_BINDING, 
                    mDeltaBuffers[nextLevel - 1]);
            }
            else
            {
                // bind dummy (incase bindings checked)
                mTerrain->vertexBufferBinding->setBinding(DELTA_BINDING, 
                    mDeltaBuffers[0]);
            }
        }
        mLastNextLevel = nextLevel;

    }

}

void TerrainRenderable::_updateRenderQueue( RenderQueue* queue )
{
    queue->addRenderable( this );
}

void TerrainRenderable::getRenderOperation( RenderOperation& op )
{
    //setup indexes for vertices and uvs...

    assert( mInit && "Uninitialized" );

    op.useIndexes = true;
    op.operationType = msUseTriStrips ? 
        RenderOperation::OT_TRIANGLE_STRIP : RenderOperation::OT_TRIANGLE_LIST;
    op.vertexData = mTerrain;
    op.indexData = getIndexData();

    mRenderLevelChanged = false;

}

void TerrainRenderable::getWorldTransforms( Matrix4* xform ) const
{
    *xform = mParentNode->_getFullTransform();
}

const Quaternion& TerrainRenderable::getWorldOrientation(void) const
{
    return mParentNode->_getDerivedOrientation();
}
const Vector3& TerrainRenderable::getWorldPosition(void) const
{
    return mParentNode->_getDerivedPosition();
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

    int i, j;

    for ( int level = 1; level < mNumMipMaps; level++ )
    {
        mMinLevelDistSqr[ level ] = 0;

        int step = 1 << level;
        // The step of the next higher LOD
        int higherstep = step >> 1;

        Real* pDeltas = 0;
        if (msUseLODMorph)
        {
            // Create a set of delta values (store at index - 1 since 0 has none)
            mDeltaBuffers[level - 1]  = createDeltaBuffer();
            // Lock, but don't discard (we want the pre-initialised zeros)
            pDeltas = static_cast<Real*>(
                mDeltaBuffers[level - 1]->lock(HardwareBuffer::HBL_NORMAL));
        }

        for ( j = 0; j < mSize - step; j += step )
        {
            for ( i = 0; i < mSize - step; i += step )
            {
                /* Form planes relating to the lower detail tris to be produced
                For tri lists and even tri strip rows, they are this shape:
                x---x
                | / |
                x---x
                For odd tri strip rows, they are this shape:
                x---x
                | \ |
                x---x
                */

                Vector3 v1(_vertex( i, j, 0 ), _vertex( i, j, 1 ), _vertex( i, j, 2 ));
                Vector3 v2(_vertex( i + step, j, 0 ), _vertex( i + step, j, 1 ), _vertex( i + step, j, 2 ));
                Vector3 v3(_vertex( i, j + step, 0 ), _vertex( i, j + step, 1 ), _vertex( i, j + step, 2 ));
                Vector3 v4(_vertex( i + step, j + step, 0 ), _vertex( i + step, j + step, 1 ), _vertex( i + step, j + step, 2 ));

                Plane t1, t2;
                bool backwardTri = false;
                if (!msUseTriStrips || j % 2 == 0)
                {
                    t1.redefine(v1, v3, v2);
                    t2.redefine(v2, v3, v4);
                }
                else
                {
                    t1.redefine(v1, v3, v4);
                    t2.redefine(v1, v4, v2);
                    backwardTri = true;
                }

                // include the bottommost row of vertices if this is the last row
                int zubound = (j == (mSize - step)? step : step - 1);
                for ( int z = 0; z <= zubound; z++ )
                {
                    // include the rightmost col of vertices if this is the last col
                    int xubound = (i == (mSize - step)? step : step - 1);
                    for ( int x = 0; x <= xubound; x++ )
                    {
                        int fulldetailx = i + x;
                        int fulldetailz = j + z;
                        if ( fulldetailx % step == 0 && 
                             fulldetailz % step == 0 )
                        {
                            // Skip, this one is a vertex at this level
                            continue;
                        }

                        Real zpct = (Real)z / (Real)step;
                        Real xpct = (Real)x / (Real)step;

                        //interpolated height
                        Vector3 actualPos(
                            _vertex( fulldetailx, fulldetailz, 0 ), 
                            _vertex( fulldetailx, fulldetailz, 1 ), 
                            _vertex( fulldetailx, fulldetailz, 2 ));
                        Real interp_h;
                        // Determine which tri we're on 
                        if ((xpct + zpct <= 1.0f && !backwardTri) ||
                            (xpct + (1-zpct) <= 1.0f && backwardTri))
                        {
                            // Solve for x/z
                            interp_h = 
                                (-(t1.normal.x * actualPos.x)
                                - t1.normal.z * actualPos.z
                                - t1.d) / t1.normal.y;
                        }
                        else
                        {
                            // Second tri
                            interp_h = 
                                (-(t2.normal.x * actualPos.x)
                                - t2.normal.z * actualPos.z
                                - t2.d) / t2.normal.y;
                        }

                        Real actual_h = _vertex( fulldetailx, fulldetailz, 1 );
                        Real delta = fabs( interp_h - actual_h );

                        Real D2 = delta * delta * C * C;

                        if ( mMinLevelDistSqr[ level ] < D2 )
                            mMinLevelDistSqr[ level ] = D2;
                        
                        // Should be save height difference?
                        // Don't morph along edges
                        if (msUseLODMorph && 
                            fulldetailx != 0  && fulldetailx != (mSize - 1) && 
                            fulldetailz != 0  && fulldetailz != (mSize - 1) )
                        {
                            // Save height difference 
                            pDeltas[fulldetailx + (fulldetailz * mSize)] = 
                                interp_h - actual_h;
                            if (interp_h == actual_h)
                            {
                                LogManager::getSingleton().logMessage(
                                    "No difference for " + getName() + " level " + 
                                    StringConverter::toString(level) + " at (" + 
                                    StringConverter::toString(fulldetailx) + ", " + 
                                    StringConverter::toString(fulldetailz) + ")"); 
                            }
                        }

                    }

                }
            }
        }

        // Unlock morph deltas if required
        if (msUseLODMorph)
        {
            mDeltaBuffers[level - 1]->unlock();
        }
    }



    // Post validate the whole set
    for ( i = 1; i < mNumMipMaps; i++ )
    {
    
        // Make sure no LOD transition within the tile
        // This is especially a problem when using large tiles with flat areas
        Vector3 delta(_vertex(0,0,0), mCenter.y, _vertex(0,0,2));
        delta = delta - mCenter;
        Real minDist = delta.squaredLength();
        mMinLevelDistSqr[ i ] = std::max(mMinLevelDistSqr[ i ], minDist);

        //make sure the levels are increasing...
        if ( mMinLevelDistSqr[ i ] < mMinLevelDistSqr[ i - 1 ] )
        {
            mMinLevelDistSqr[ i ] = mMinLevelDistSqr[ i - 1 ];
        }
    }

    // Now reverse traverse the list setting the 'next level down'
    Real lastDist = 0;
    int lastIndex = 0;
    for (i = mNumMipMaps - 1; i >= 0; --i)
    {
        if (mMinLevelDistSqr[i] != lastDist)
        {
            mNextLevelDown[i] = lastIndex;
            lastIndex = i;
            lastDist = mMinLevelDistSqr[i];
        }

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

            mLevelIndex.push_back( new IndexMap() );

        }

    }

    mLevelInit = true;
}

void TerrainRenderable::_destroyLevelIndexes()
{
    if ( mLevelInit )
    {
        for ( int i = 0; i < 16; i++ )
        {
            delete mLevelIndex[i];
        }
        mLevelIndex.clear();
        mLevelInit = false;
    }
}

void TerrainRenderable::_adjustRenderLevel( int i )
{

    mRenderLevel = i;
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

    end.x = _vertex( mSize - 2, mSize - 2, 0 );
    end.y = _vertex( mSize - 2, mSize - 2, 1 );
    end.z = _vertex( mSize - 2, mSize - 2, 2 );

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

    float x_pt = x_pct * ( float ) ( mSize - 2 );
    float z_pt = z_pct * ( float ) ( mSize - 2 );

    int x_index = ( int ) x_pt;
    int z_index = ( int ) z_pt;

    x_pct = x_pt - x_index;
    z_pct = z_pt - z_index;

    //bilinear interpolate to find the height.

    float t1 = _vertex( x_index, z_index, 1 );
    float t2 = _vertex( x_index + 1, z_index, 1 );
    float b1 = _vertex( x_index, z_index + 1, 1 );
    float b2 = _vertex( x_index + 1, z_index + 1, 1 );

    float midpoint = (b1 + t2) / 2.0;

    if (x_pct + z_pct <= 1) {
        b2 = midpoint + (midpoint - t1);
    } else {
        t1 = midpoint + (midpoint - b2);
    }

    float t = ( t1 * ( 1 - x_pct ) ) + ( t2 * ( x_pct ) );
    float b = ( b1 * ( 1 - x_pct ) ) + ( b2 * ( x_pct ) );

    float h = ( t * ( 1 - z_pct ) ) + ( b * ( z_pct ) );

    return h;
}

bool TerrainRenderable::intersectSegment( const Vector3 & start, const Vector3 & end, Vector3 * result )
{
    Vector3 dir = end - start;
    Vector3 ray = start;

    //special case...
    if ( dir.x == 0 && dir.z == 0 )
    {
        if ( ray.y <= getHeightAt( ray.x, ray.z ) )
        {
            if ( result != 0 )
                * result = start;

            return true;
        }
    }

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


        float h = getHeightAt( ray.x, ray.z );

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
    if ( !mColored || !mLit )
    {
        printf( "Can't generate terrain vertex lighting with out vertex normals and colors enabled.\n" );
        return ;
    }


    Vector3 pt;
    Vector3 normal;
    Vector3 light;

    HardwareVertexBufferSharedPtr vbuf = 
        mTerrain->vertexBufferBinding->getBuffer(MAIN_BINDING);
    const VertexElement* elem = mTerrain->vertexDeclaration->findElementBySemantic(VES_DIFFUSE);
    //for each point in the terrain, see if it's in the line of sight for the sun.
    for ( int i = 0; i < mSize; i++ )
    {
        for ( int j = 0; j < mSize; j++ )
        {
            //  printf( "Checking %f,%f,%f ", pt.x, pt.y, pt.z );
            pt.x = _vertex( i, j, 0 );
            pt.y = _vertex( i, j, 1 );
            pt.z = _vertex( i, j, 2 );

            light = sun - pt;

            light.normalise();

            if ( ! intersectSegment( pt, sun, 0 ) )
            {
                //
                _getNormalAt( _vertex( i, j, 0 ), _vertex( i, j, 2 ), &normal );

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

                RGBA colour;
                Root::getSingleton().convertColourValue( v, &colour );
                vbuf->writeData(
                    (_index( i, j ) * vbuf->getVertexSize()) + elem->getOffset(),
                    sizeof(RGBA), &colour);
            }

            else
            {
                RGBA colour;
                Root::getSingleton().convertColourValue( ambient, &colour );

                vbuf->writeData(
                    (_index( i, j ) * vbuf->getVertexSize()) + elem->getOffset(), 
                    sizeof(RGBA), &colour);
            }

        }

    }

    printf( "." );
}
//-----------------------------------------------------------------------
Real TerrainRenderable::getSquaredViewDepth(const Camera* cam) const
{
    Vector3 diff = mCenter - cam->getDerivedPosition();
    // Use squared length to avoid square root
    return diff.squaredLength();
}

//-----------------------------------------------------------------------
const LightList& TerrainRenderable::getLights(void) const
{
    return mParentNode->getLights();
}
//-----------------------------------------------------------------------
IndexData* TerrainRenderable::getIndexData(void)
{
    unsigned int stitchFlags = 0;

    if ( mNeighbors[ EAST ] != 0 && mNeighbors[ EAST ] -> mRenderLevel > mRenderLevel )
    {
        stitchFlags |= STITCH_EAST;
        stitchFlags |= 
            (mNeighbors[ EAST ] -> mRenderLevel - mRenderLevel) << STITCH_EAST_SHIFT;
    }

    if ( mNeighbors[ WEST ] != 0 && mNeighbors[ WEST ] -> mRenderLevel > mRenderLevel )
    {
        stitchFlags |= STITCH_WEST;
        stitchFlags |= 
            (mNeighbors[ WEST ] -> mRenderLevel - mRenderLevel) << STITCH_WEST_SHIFT;
    }

    if ( mNeighbors[ NORTH ] != 0 && mNeighbors[ NORTH ] -> mRenderLevel > mRenderLevel )
    {
        stitchFlags |= STITCH_NORTH;
        stitchFlags |= 
            (mNeighbors[ NORTH ] -> mRenderLevel - mRenderLevel) << STITCH_NORTH_SHIFT;
    }

    if ( mNeighbors[ SOUTH ] != 0 && mNeighbors[ SOUTH ] -> mRenderLevel > mRenderLevel )
    {
        stitchFlags |= STITCH_SOUTH;
        stitchFlags |= 
            (mNeighbors[ SOUTH ] -> mRenderLevel - mRenderLevel) << STITCH_SOUTH_SHIFT;
    }

    // Check preexisting
    IndexMap::iterator ii = mLevelIndex[ mRenderLevel ]->find( stitchFlags );
    IndexData* indexData;
    if ( ii == mLevelIndex[ mRenderLevel ]->end())
    {
        // Create
        if (msUseTriStrips)
        {
            indexData = generateTriStripIndexes(stitchFlags);
        }
        else
        {
            indexData = generateTriListIndexes(stitchFlags);
        }
        mLevelIndex[ mRenderLevel ]->insert(
            IndexMap::value_type(stitchFlags, indexData));
    }
    else
    {
        indexData = ii->second;
    }


    return indexData;


}
//-----------------------------------------------------------------------
IndexData* TerrainRenderable::generateTriStripIndexes(unsigned int stitchFlags)
{
    // The step used for the current level
    int step = 1 << mRenderLevel;
    // The step used for the lower level
    int lowstep = 1 << (mRenderLevel + 1);

    int numIndexes = 0;

    // Calculate the number of indexes required
    // This is the number of 'cells' at this detail level x 2
    // plus 3 degenerates to turn corners
    int numTrisAcross = (((mSize-1) / step) * 2) + 3;
    // Num indexes is number of tris + 2
    int new_length = numTrisAcross * ((mSize-1) / step) + 2;
    //this is the maximum for a level.  It wastes a little, but shouldn't be a problem.

    IndexData* indexData = new IndexData;
    indexData->indexBuffer = 
        HardwareBufferManager::getSingleton().createIndexBuffer(
        HardwareIndexBuffer::IT_16BIT,
        new_length, HardwareBuffer::HBU_STATIC_WRITE_ONLY);//, false);

    gIndexCache.mCache.push_back( indexData );

    unsigned short* pIdx = static_cast<unsigned short*>(
        indexData->indexBuffer->lock(0, 
        indexData->indexBuffer->getSizeInBytes(), 
        HardwareBuffer::HBL_DISCARD));

    // Stripified mesh
    for ( int j = 0; j < mSize - 1; j += step )
    {
        int i;
        // Forward strip
        // We just do the |/ here, final | done after
        for ( i = 0; i < mSize - 1; i += step )
        {
            int x[4], y[4];
            x[0] = x[1] = i;
            x[2] = x[3] = i + step;
            y[0] = y[2] = j;
            y[1] = y[3] = j + step;

            if (j == 0  && (stitchFlags & STITCH_NORTH))
            {
                // North reduction means rounding x[0] and x[2]
                if (x[0] % lowstep != 0)
                {
                    // Since we know we only drop down one level of LOD,
                    // removing 1 step of higher LOD should return to lower
                    x[0] -= step;
                }
                if (x[2] % lowstep != 0)
                {
                    x[2] -= step;
                }
            }

            // Never get a south tiling on a forward strip (always finish on 
            // a backward strip)

            if (i == 0  && (stitchFlags & STITCH_WEST))
            {
                // West reduction means rounding y[0] / y[1]
                if (y[0] % lowstep != 0)
                {
                    y[0] -= step;
                }
                if (y[1] % lowstep != 0)
                {
                    y[1] -= step;
                }
            }
            if (i == (mSize - 1 - step) && (stitchFlags & STITCH_EAST))
            {
                // East tiling means rounding y[2] & y[3]
                if (y[2] % lowstep != 0)
                {
                    y[2] -= step;
                }
                if (y[3] % lowstep != 0)
                {
                    y[3] -= step;
                }
            }

            //triangles
            if (i == 0)
            {
                // Starter
                *pIdx++ = _index( x[0], y[0] ); numIndexes++;
            }
            *pIdx++ = _index( x[1], y[1] ); numIndexes++;
            *pIdx++ = _index( x[2], y[2] ); numIndexes++;

            if (i == mSize - 1 - step)
            {
                // Emit extra index to finish row
                *pIdx++ = _index( x[3], y[3] ); numIndexes++;
                if (j < mSize - 1 - step)
                {
                    // Emit this index twice more (this is to turn around without
                    // artefacts)
                    // ** Hmm, looks like we can drop this and it's unnoticeable
                    //*pIdx++ = _index( x[3], y[3] ); numIndexes++;
                    //*pIdx++ = _index( x[3], y[3] ); numIndexes++;
                }
            }

        }
        // Increment row
        j += step;
        // Backward strip
        for ( i = mSize - 1; i > 0 ; i -= step )
        {
            int x[4], y[4];
            x[0] = x[1] = i;
            x[2] = x[3] = i - step;
            y[0] = y[2] = j;
            y[1] = y[3] = j + step;

            // Never get a north tiling on a backward strip (always
            // start on a forward strip)
            if (j == (mSize - 1 - step) && (stitchFlags & STITCH_SOUTH))
            {
                // South reduction means rounding x[1] / x[3]
                if (x[1] % lowstep != 0)
                {
                    x[1] -= step;
                }
                if (x[3] % lowstep != 0)
                {
                    x[3] -= step;
                }
            }

            if (i == step  && (stitchFlags & STITCH_WEST))
            {
                // West tiling on backward strip is rounding of y[2] / y[3]
                if (y[2] % lowstep != 0)
                {
                    y[2] -= step;
                }
                if (y[3] % lowstep != 0)
                {
                    y[3] -= step;
                }
            }
            if (i == mSize - 1 && (stitchFlags & STITCH_EAST))
            {
                // East tiling means rounding y[0] and y[1] on backward strip
                if (y[0] % lowstep != 0)
                {
                    y[0] -= step;
                }
                if (y[1] % lowstep != 0)
                {
                    y[1] -= step;
                }
            }

            //triangles
            if (i == mSize)
            {
                // Starter
                *pIdx++ = _index( x[0], y[0] ); numIndexes++;
            }
            *pIdx++ = _index( x[1], y[1] ); numIndexes++;
            *pIdx++ = _index( x[2], y[2] ); numIndexes++;

            if (i == step)
            {
                // Emit extra index to finish row
                *pIdx++ = _index( x[3], y[3] ); numIndexes++;
                if (j < mSize - 1 - step)
                {
                    // Emit this index once more (this is to turn around)
                    *pIdx++ = _index( x[3], y[3] ); numIndexes++;
                }
            }
        }
    }


    indexData->indexBuffer->unlock();
    indexData->indexCount = numIndexes;
    indexData->indexStart = 0;

    return indexData;

}
//-----------------------------------------------------------------------
IndexData* TerrainRenderable::generateTriListIndexes(unsigned int stitchFlags)
{

    int numIndexes = 0;
    int step = 1 << mRenderLevel;

    IndexData* indexData = 0;

    int north = stitchFlags & STITCH_NORTH ? step : 0;
    int south = stitchFlags & STITCH_SOUTH ? step : 0;
    int east = stitchFlags & STITCH_EAST ? step : 0;
    int west = stitchFlags & STITCH_WEST ? step : 0;

    int new_length = ( mSize / step ) * ( mSize / step ) * 2 * 2 * 2 ;
    //this is the maximum for a level.  It wastes a little, but shouldn't be a problem.

    indexData = new IndexData;
    indexData->indexBuffer = 
        HardwareBufferManager::getSingleton().createIndexBuffer(
        HardwareIndexBuffer::IT_16BIT,
        new_length, HardwareBuffer::HBU_STATIC_WRITE_ONLY);//, false);

    gIndexCache.mCache.push_back( indexData );

    unsigned short* pIdx = static_cast<unsigned short*>(
        indexData->indexBuffer->lock(0, 
        indexData->indexBuffer->getSizeInBytes(), 
        HardwareBuffer::HBL_DISCARD));

    // Do the core vertices, minus stitches
    for ( int j = north; j < mSize - 1 - south; j += step )
    {
        for ( int i = west; i < mSize - 1 - east; i += step )
        {
            //triangles
            *pIdx++ = _index( i, j ); numIndexes++;
            *pIdx++ = _index( i, j + step ); numIndexes++;
            *pIdx++ = _index( i + step, j ); numIndexes++;

            *pIdx++ = _index( i, j + step ); numIndexes++;
            *pIdx++ = _index( i + step, j + step ); numIndexes++;
            *pIdx++ = _index( i + step, j ); numIndexes++;
        }
    }

    // North stitching
    if ( north > 0 )
    {
        numIndexes += stitchEdge(NORTH, mRenderLevel, mNeighbors[NORTH]->mRenderLevel,
            west > 0, east > 0, &pIdx);
    }
    // East stitching
    if ( east > 0 )
    {
        numIndexes += stitchEdge(EAST, mRenderLevel, mNeighbors[EAST]->mRenderLevel,
            north > 0, south > 0, &pIdx);
    }
    // South stitching
    if ( south > 0 )
    {
        numIndexes += stitchEdge(SOUTH, mRenderLevel, mNeighbors[SOUTH]->mRenderLevel,
            east > 0, west > 0, &pIdx);
    }
    // West stitching
    if ( west > 0 )
    {
        numIndexes += stitchEdge(WEST, mRenderLevel, mNeighbors[WEST]->mRenderLevel,
            south > 0, north > 0, &pIdx);
    }


    indexData->indexBuffer->unlock();
    indexData->indexCount = numIndexes;
    indexData->indexStart = 0;

    return indexData;
}
//-----------------------------------------------------------------------
HardwareVertexBufferSharedPtr TerrainRenderable::createDeltaBuffer(void)
{
    // Delta buffer is a 1D float buffer of height offsets
    HardwareVertexBufferSharedPtr buf = 
        HardwareBufferManager::getSingleton().createVertexBuffer(
        VertexElement::getTypeSize(VET_FLOAT1), 
        mSize * mSize,
        HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    // Fill the buffer with zeros, we will only fill in delta
    void* pVoid = buf->lock(HardwareBuffer::HBL_DISCARD);
    memset(pVoid, 0, mSize * mSize * sizeof(Real));
    buf->unlock();

    return buf;

}
//-----------------------------------------------------------------------
void TerrainRenderable::updateCustomGpuParameter(
    const GpuProgramParameters::AutoConstantEntry& constantEntry, 
    GpuProgramParameters* params) const
{
    if (constantEntry.data == MORPH_CUSTOM_PARAM_ID)
    {
        // Update morph LOD factor
        params->setConstant(constantEntry.index, mLODMorphFactor);
    }

}
//-----------------------------------------------------------------------
int TerrainRenderable::stitchEdge(Neighbor neighbor, int hiLOD, int loLOD, 
    bool omitFirstTri, bool omitLastTri, unsigned short** ppIdx)
{
    assert(loLOD > hiLOD);
    /* 
    Now do the stitching; we can stitch from any level to any level.
    The stitch pattern is like this for each pair of vertices in the lower LOD
    (excuse the poor ascii art):

    lower LOD
    *-----------*
    |\  \ 3 /  /|
    |1\2 \ / 4/5|
    *--*--*--*--*
    higher LOD

    The algorithm is, for each pair of lower LOD vertices:
    1. Iterate over the higher LOD vertices, generating tris connected to the 
    first lower LOD vertex, up to and including 1/2 the span of the lower LOD 
    over the higher LOD (tris 1-2). Skip the first tri if it is on the edge 
    of the tile and that edge is to be stitched itself.
    2. Generate a single tri for the middle using the 2 lower LOD vertices and 
    the middle vertex of the higher LOD (tri 3). 
    3. Iterate over the higher LOD vertices from 1/2 the span of the lower LOD
    to the end, generating tris connected to the second lower LOD vertex 
    (tris 4-5). Skip the last tri if it is on the edge of a tile and that
    edge is to be stitched itself.

    The same algorithm works for all edges of the patch; stitching is done
    clockwise so that the origin and steps used change, but the general
    approach does not.
    */

    // Get pointer to be updated
    unsigned short* pIdx = *ppIdx;

    // Work out the steps ie how to increment indexes
    // Step from one vertex to another in the high detail version
    int step = 1 << hiLOD;
    // Step from one vertex to another in the low detail version
    int superstep = 1 << loLOD;
    // Step half way between low detail steps
    int halfsuperstep = superstep >> 1;

    // Work out the starting points and sign of increments
    // We always work the strip clockwise
    int startx, starty, endx, rowstep;
    bool horizontal;
    switch(neighbor)
    {
    case NORTH:
        startx = starty = 0;
        endx = mSize - 1;
        rowstep = step;
        horizontal = true;
        break;
    case SOUTH:
        // invert x AND y direction, helps to keep same winding
        startx = starty = mSize - 1;
        endx = 0;
        rowstep = -step;
        step = -step;
        superstep = -superstep;
        halfsuperstep = -halfsuperstep;
        horizontal = true;
        break;
    case EAST:
        startx = 0;
        endx = mSize - 1;
        starty = mSize - 1;
        rowstep = -step;
        horizontal = false;
        break;
    case WEST:
        startx = mSize - 1;
        endx = 0;
        starty = 0;
        rowstep = step;
        step = -step;
        superstep = -superstep;
        halfsuperstep = -halfsuperstep;
        horizontal = false;
        break;
    };

    int numIndexes = 0;

    for ( int j = startx; j != endx; j += superstep )
    {
        int k;
        for (k = 0; k != halfsuperstep; k += step)
        {
            int jk = j + k;
            //skip the first bit of the corner?
            if ( j != startx || k != 0 || !omitFirstTri )
            {
                if (horizontal)
                {
                    *pIdx++ = _index( j , starty ); numIndexes++;
                    *pIdx++ = _index( jk, starty + rowstep ); numIndexes++;
                    *pIdx++ = _index( jk + step, starty + rowstep ); numIndexes++;
                }
                else
                {
                    *pIdx++ = _index( starty, j ); numIndexes++;
                    *pIdx++ = _index( starty + rowstep, jk ); numIndexes++;
                    *pIdx++ = _index( starty + rowstep, jk + step); numIndexes++;
                }
            }
        }

        // Middle tri
        if (horizontal)
        {
            *pIdx++ = _index( j, starty ); numIndexes++;
            *pIdx++ = _index( j + halfsuperstep, starty + rowstep); numIndexes++;
            *pIdx++ = _index( j + superstep, starty ); numIndexes++;
        }
        else
        {
            *pIdx++ = _index( starty, j ); numIndexes++;
            *pIdx++ = _index( starty + rowstep, j + halfsuperstep ); numIndexes++;
            *pIdx++ = _index( starty, j + superstep ); numIndexes++;
        }

        for (k = halfsuperstep; k != superstep; k += step)
        {
            int jk = j + k;
            if ( j != endx - superstep || k != superstep - step || !omitLastTri )
            {
                if (horizontal)
                {
                    *pIdx++ = _index( j + superstep, starty ); numIndexes++;
                    *pIdx++ = _index( jk, starty + rowstep ); numIndexes++;
                    *pIdx++ = _index( jk + step, starty + rowstep ); numIndexes++;
                }
                else
                {
                    *pIdx++ = _index( starty, j + superstep ); numIndexes++;
                    *pIdx++ = _index( starty + rowstep, jk ); numIndexes++;
                    *pIdx++ = _index( starty + rowstep, jk + step ); numIndexes++;
                }
            }
        }
    }

    *ppIdx = pIdx;

    return numIndexes;

}


} //namespace

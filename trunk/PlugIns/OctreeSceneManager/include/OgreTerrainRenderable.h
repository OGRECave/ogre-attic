/***************************************************************************
                        terrainrenderable.h  -  description
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

#ifndef TERRAINRENDERABLE_H
#define TERRAINRENDERABLE_H

#include <OgreRenderable.h>
#include <OgreMovableObject.h>
#include <OgreAxisAlignedBox.h>
#include <OgreString.h>
#include <OgreGeometryData.h>

#include <vector>

namespace Ogre
{

/**
  * An index buffer for terrain tiles.  These are cached once created, and reused
  * for different tiles.
  */
class TerrainIndexBuffer
{
public:
    TerrainIndexBuffer( int s )
    {
        indexes = new unsigned short[ s ];
    }

    ~TerrainIndexBuffer()
    {
        delete []indexes;
    }

    unsigned short * indexes;
    int length;
};

typedef std::vector < TerrainIndexBuffer * > IndexArray;
typedef std::vector < IndexArray > LevelArray;

/**
 * A cache of TerrainIndexBuffers.  Used to keep track of the buffers, and
 * delete them when the program finishes.
 */
class TerrainBufferCache
{
  public:
  ~TerrainBufferCache()
    {
      for( int i=0; i<mCache.size(); i++ )
	{
	  delete mCache[i];
	}
    }
  IndexArray mCache;
};



inline Real _max( Real x, Real y )
{
    return ( x > y ) ? x : y;
}

/**
  * A simple class for encapsulating parameters passed in when initializing a TerrainRenderable
  */

class TerrainOptions
{
public:
    TerrainOptions()
    {
        data = 0;
        size = 0;
        world_size = 0;
        startx = 0;
        startz = 0;
        max_mipmap = 0;
        scalex = 1;
        scaley = 1;
        scalez = 1;
        max_pixel_error = 4;
        vert_res = 768;
        top_coord = 1;
        near_plane = 1;
        detail_tile = 1;
        lit = false;
        colored = false;
    };


    int _worldheight( int x, int z )
    {
        return data[ ( ( z * world_size ) + x ) ];
    };

    const uchar * data;     //pointer to the world 2D data.
    int size;         //size of this square block
    int world_size;   //size of the world.
    int startx;
    int startz; //starting coords of this block.
    int max_mipmap;  //max mip_map level
    float scalex, scaley, scalez;

    int max_pixel_error;
    int near_plane;
    int vert_res;
    int detail_tile;
    float top_coord;

    bool lit;
    bool colored;

};

#define TILE_NORTH  0x00000001
#define TILE_SOUTH  0x00000002
#define TILE_WEST   0x00000004
#define TILE_EAST   0x00000008

/**
   Represents a terrain tile.
    @remarks
      A TerrainRenderable represents a tile used to render a block of terrain using the geomipmap approach
      for LOD.
  *@author Jon Anderson
  */

class TerrainRenderable : public Renderable, public MovableObject
{
public:

    TerrainRenderable();
    ~TerrainRenderable();

    void deleteGeometry();

    enum Neighbor
    {
        NORTH = 0,
        SOUTH = 1,
        EAST = 2,
        WEST = 3,
        HERE = 4
    };

    /**
      Initializes the TerrainRenderable with the given options.
     */
    void init( TerrainOptions &options );

    //movable object methods
    /** Returns the name of the TerrainRenderable */
    virtual const String& getName( void ) const
    {
        return mName;
    };

    /** Returns the type of the movable. */
    virtual const String getMovableType( void ) const
    {
        return mType;
    };

    /** Returns the bounding box of this TerrainRenderable */
    const AxisAlignedBox& getBoundingBox( void ) const
    {
        return mBounds;
    };

    /** Updates the level of detail to be used for rendering this TerrainRenderable based on the passed in Camera */
    virtual void _notifyCurrentCamera( Camera* cam );

    virtual void _updateRenderQueue( RenderQueue* queue );

    /**
      Constructs a RenderOperation to render the TerrainRenderable.
      @remarks
        Each TerrainRenderable has a block of vertices that represent the terrain.  Index arrays are dynamically
        created for mipmap level, and then cached.
     */
    virtual void getRenderOperation( RenderOperation& rend );

    virtual Material* getMaterial( void ) const
    {
        return mMaterial;
    };

    virtual void getWorldTransforms( Matrix4* xform );

    /** Returns the size of the TerrainRenderable */
    inline int getSize()
    {
        return mSize;
    };

    /** Returns the mipmap level that will be rendered for this frame. */
    inline int getRenderLevel()
    {
        return mRenderLevel;
    };

    /** Forces the LOD to the given level from this point on. */
    inline void setForcedRenderLevel( int i )
    {
        mForcedRenderLevel = i;
    }

    /** Returns the maximum number of mipmaps used for LOD. */
    inline int getNumMipMaps()
    {
        return mNumMipMaps;
    };

    /** Calculates the normal at the given location */
    void _getNormalAt( float x, float y, Vector3 * result );

    /** Returns the terrain height at the given coordinates */
    float getHeightAt( float x, float y );

    /** Intersects the segment witht he terrain tile
     */
    bool intersectSegment( const Vector3 & start, const Vector3 & end, Vector3 * result );

    /** Sets the appropriate neighbor for this TerrainRenderable.  Neighbors are necessary
        to know when to bridge between LODs.
    */
    void _setNeighbor( Neighbor n, TerrainRenderable *t )
    {
        mNeighbors[ n ] = t;
    };

    /** Returns the neighbor TerrainRenderable.
     */
    TerrainRenderable * _getNeighbor( Neighbor n )
    {
        return mNeighbors[ n ];
    }


    void setMaterial( Material *m )
    {
        mMaterial = m;
    };

    /** Aligns mipmap levels between neighbors so that only 1 LOD level separates neighbors. */
    void _alignNeighbors();
    /** Calculates static normals for lighting the terrain. */
    void _calculateNormals();




    /** Generates terrain shadows and lighting using vertex colors
     */
    void _generateVertexLighting( const Vector3 &sun, ColourValue ambient );


    static int mRenderedTris;

    /** Overridden, see Renderable */
    Real getSquaredViewDepth(const Camera* cam) const;


protected:

    /** Returns the index into the height array for the given coords. */
    inline int _index( int x, int z )
    {
        return ( x + z * mSize );
    };

    /** Returns the  vertex coord for the given coordinates */
    inline float _vertex( int x, int z, int n )
    {
        return mTerrain.pVertices[ x * 3 + z * mSize * 3 + n ];
    };


    inline int _numNeighbors()
    {
        int n = 0;

        for ( int i = 0; i < 4; i++ )
        {
            if ( mNeighbors[ i ] != 0 )
                n++;
        }

        return n;
    }

    inline bool _hasNeighborRenderLevel( int i )
    {
        for ( int j = 0; j < 4; j++ )
        {
            if ( mNeighbors[ j ] != 0 && mNeighbors[ j ] ->mRenderLevel == i )
                return true;;
        }

        return false;

    }

    void _adjustRenderLevel( int i );

    void _initLevelIndexes();

    bool _checkSize( int n );

    void _calculateMinLevelDist2( Real C );

    Real _calculateCFactor();

    GeometryData mTerrain;

    int mNumMipMaps;
    int mRenderLevel;

    Real *mMinLevelDistSqr;

    TerrainRenderable *mNeighbors [ 4 ];

    AxisAlignedBox mBounds;
    Vector3 mCenter;
    Vector3 mScale;

    int mSize;
    int mWorldSize;

    String mName;
    static String mType;

    Material *mMaterial;

    bool mRenderLevelChanged;
    bool mInit;

    static LevelArray mLevelIndex;
    static bool mLevelInit;

    int mNearPlane;
    int mMaxPixelError;
    int mVertResolution;
    Real mTopCoord;

    Real old_L;

    Real current_L;

    bool mColored;
    bool mLit;

    int mForcedRenderLevel;

};

}

#endif

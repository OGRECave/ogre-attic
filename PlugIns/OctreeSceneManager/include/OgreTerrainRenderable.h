/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2004 The OGRE Team
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
terrainrenderable.h  -  description
-------------------
begin                : Sat Oct 5 2002
copyright            : (C) 2002 by Jon Anderson
email                : janders@users.sf.net

Enhancements 2003 - 2004 (C) The OGRE Team

***************************************************************************/

#ifndef TERRAINRENDERABLE_H
#define TERRAINRENDERABLE_H

#include "OgreTerrainPrerequisites.h"
#include <OgreRenderable.h>
#include <OgreMovableObject.h>
#include <OgreAxisAlignedBox.h>
#include <OgreString.h>
#include <OgreHardwareBufferManager.h>

#include <vector>

#define MORPH_CUSTOM_PARAM_ID 77

namespace Ogre
{

    typedef std::map <unsigned int, IndexData* > IndexMap;
    typedef std::vector < IndexData* > IndexArray;
    typedef std::vector < IndexMap* > LevelArray;

    /**
    * A cache of TerrainIndexBuffers.  Used to keep track of the buffers, and
    * delete them when the program finishes.
    */
    class TerrainBufferCache
    {
    public:
        void shutdown(void)
        {
            for( size_t i=0; i<mCache.size(); i++ )
            {
                delete mCache[i];
            }
            mCache.clear();
        }
        ~TerrainBufferCache()
        {
            shutdown();
        }

        IndexArray mCache;
    };

    inline Real _max( Real x, Real y )
    {
        return ( x > y ) ? x : y;
    }

    /** A simple class for encapsulating parameters which are commonly needed by 
    both TerrainSceneManager and TerrainRenderable.
    */
    class TerrainOptions
    {
    public:
        TerrainOptions()
        {
            pageSize = 0;
            tileSize = 0;
            tilesPerPage = 0;
            maxGeoMipMapLevel = 0;
            scale = Vector3::UNIT_SCALE;
            maxPixelError = 4;
            detailTile = 1;
            lit = false;
            coloured = false;
            lodMorph = false;
            lodMorphStart = 0.5;
            useTriStrips = false;
            primaryCamera = 0;
            terrainMaterial = 0;
        };
        /// The size of one edge of a terrain page, in vertices
        size_t pageSize;
        /// The size of one edge of a terrain tile, in vertices
        size_t tileSize; 
        /// Precalculated number of tiles per page
        size_t tilesPerPage;
        /// The primary camera, used for error metric calculation and page choice
        const Camera* primaryCamera;
        /// The maximum terrain geo-mipmap level
        size_t maxGeoMipMapLevel;
        /// The scale factor to apply to the terrain (each vertex is 1 unscaled unit
        /// away from the next, and height is from 0 to 1)
        Vector3 scale;
        /// The maximum pixel error allowed
        size_t maxPixelError;
        /// Whether we should use triangle strips
        bool useTriStrips;
        /// The number of times to repeat a detail texture over a tile
        size_t detailTile;
        /// Whether LOD morphing is enabled
        bool lodMorph;
        /// At what point (parametric) should LOD morphing start
        Real lodMorphStart;
        /// Whether dynamic lighting is enabled
        bool lit;
        /// Whether vertex colours are enabled
        bool coloured;
        /// Pointer to the material to use to render the terrain
        MaterialPtr terrainMaterial;

    };

#define STITCH_NORTH_SHIFT 0
#define STITCH_SOUTH_SHIFT 8
#define STITCH_WEST_SHIFT  16
#define STITCH_EAST_SHIFT  24

#define STITCH_NORTH  128 << STITCH_NORTH_SHIFT
#define STITCH_SOUTH  128 << STITCH_SOUTH_SHIFT
#define STITCH_WEST   128 << STITCH_WEST_SHIFT
#define STITCH_EAST   128 << STITCH_EAST_SHIFT

    /**
    Represents a terrain tile.
    @remarks
    A TerrainRenderable represents a tile used to render a block of terrain using the geomipmap approach
    for LOD.
    *@author Jon Anderson
    */

    class _OgreTerrainExport TerrainRenderable : public Renderable, public MovableObject
    {
    public:

        TerrainRenderable(const String& name);
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

        /** Initializes the TerrainRenderable.
        @param startx, startz 
        The starting points of the top-left of this tile, in terms of the
        number of vertices.
        @param pageHeightData The source height data for the entire parent page
        */
        void initialise(int startx, int startz, Real* pageHeightData);

        //movable object methods
        /** Returns the name of the TerrainRenderable */
        virtual const String& getName( void ) const
        {
            return mName;
        };

        /** Returns the type of the movable. */
        virtual const String& getMovableType( void ) const
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

        virtual MaterialPtr& getMaterial( void ) const
        {
            return mMaterial;
        };

        virtual void getWorldTransforms( Matrix4* xform ) const;

        virtual const Quaternion& getWorldOrientation(void) const;
        virtual const Vector3& getWorldPosition(void) const;

        /** Returns the mipmap level that will be rendered for this frame. */
        inline int getRenderLevel() const
        {
            return mRenderLevel;
        };

        /** Forces the LOD to the given level from this point on. */
        inline void setForcedRenderLevel( int i )
        {
            mForcedRenderLevel = i;
        }

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


        void setMaterial(MaterialPtr& m )
        {
            mMaterial = m;
        };

        /** Calculates static normals for lighting the terrain. */
        void _calculateNormals();




        /** Generates terrain shadows and lighting using vertex colors
        */
        void _generateVertexLighting( const Vector3 &sun, ColourValue ambient );


        /** Overridden, see Renderable */
        Real getSquaredViewDepth(const Camera* cam) const;

        /** Overridden from MovableObject */
        Real getBoundingRadius(void) const { return 0; /* not needed */ }

        /** @copydoc Renderable::getLights */
        const LightList& getLights(void) const;

        /// Overridden from Renderable to allow the morph LOD entry to be set
        void _updateCustomGpuParameter(
            const GpuProgramParameters::AutoConstantEntry& constantEntry,
            GpuProgramParameters* params) const;
        /// Get the static list of indexes cached (internal use only)
        static TerrainBufferCache& _getIndexCache(void) {return msIndexCache;}
    protected:
        /// Static list of index buffers
        static TerrainBufferCache msIndexCache;
        /// Static link to static shared options
        static const TerrainOptions* msOptions;

        /** Returns the index into the height array for the given coords. */
        inline size_t _index( int x, int z ) const
        {
            return ( x + z * msOptions->tileSize );
        };

        /** Returns the  vertex coord for the given coordinates */
        inline float _vertex( int x, int z, int n )
        {
            return mPositionBuffer[x * 3 + z * msOptions->tileSize * 3 + n];
        };


        inline int _numNeighbors() const
        {
            int n = 0;

            for ( int i = 0; i < 4; i++ )
            {
                if ( mNeighbors[ i ] != 0 )
                    n++;
            }

            return n;
        }

        inline bool _hasNeighborRenderLevel( int i ) const
        {
            for ( int j = 0; j < 4; j++ )
            {
                if ( mNeighbors[ j ] != 0 && mNeighbors[ j ] ->mRenderLevel == i )
                    return true;
            }

            return false;

        }

        void _adjustRenderLevel( int i );

        void _initLevelIndexes();
        void _destroyLevelIndexes();

        bool _checkSize( int n );

        void _calculateMinLevelDist2( Real C );

        Real _calculateCFactor();

        VertexData* mTerrain;

        /// The current LOD level
        int mRenderLevel;
        /// The previous 'next' LOD level down, for frame coherency
        int mLastNextLevel; 
        /// The morph factor between this and the next LOD level down
        Real mLODMorphFactor;
        /// List of squared distances at which LODs change
        Real *mMinLevelDistSqr;
        /// Connection to tiles four neighbours
        TerrainRenderable *mNeighbors [ 4 ];
        /// Bounding box of this tile
        AxisAlignedBox mBounds;
        /// The center point of this tile
        Vector3 mCenter;
        /// Name of this renderable
        String mName;
        /// The MovableObject type
        static String mType;
        /// Current material used by this tile
        MaterialPtr mMaterial;    
        /// Whether this tile has been initialised    
        bool mInit;
        /// Shared array of IndexData (reuse indexes across tiles)
        static LevelArray mLevelIndex;
        /// Whether the level array as been initialised yet
        static bool mLevelInit;
        /// The buffer with all the renderable geometry in it
        HardwareVertexBufferSharedPtr mMainBuffer;
        /// Optional set of delta buffers, used to morph from one LOD to the next
        HardwareVertexBufferSharedPtr* mDeltaBuffers;
        /// System-memory buffer with just positions in it, for CPU operations
        Real* mPositionBuffer;
        /// Forced rendering LOD level, optional
        int mForcedRenderLevel;
        /// Array of LOD indexes specifying which LOD is the next one down
        /// (deals with clustered error metrics which cause LODs to be skipped)
        int mNextLevelDown[10];
        /// Gets the index data for this tile based on current settings
        IndexData* getIndexData(void);
        /// Internal method for generating stripified terrain indexes
        IndexData* generateTriStripIndexes(unsigned int stitchFlags);
        /// Internal method for generating triangle list terrain indexes
        IndexData* generateTriListIndexes(unsigned int stitchFlags);
        /** Utility method to generate stitching indexes on the edge of a tile
        @param neighbor The neighbor direction to stitch
        @param hiLOD The LOD of this tile
        @param loLOD The LOD of the neighbor
        @param omitFirstTri Whether the first tri of the stitch (always clockwise
        relative to the centre of this tile) is to be omitted because an 
        adjoining edge is also being stitched
        @param omitLastTri Whether the last tri of the stitch (always clockwise
        relative to the centre of this tile) is to be omitted because an 
        adjoining edge is also being stitched
        @param pIdx Pointer to a pointer to the index buffer to push the results 
        into (this pointer will be updated)
        @returns The number of indexes added
        */
        int stitchEdge(Neighbor neighbor, int hiLOD, int loLOD, 
            bool omitFirstTri, bool omitLastTri, unsigned short** ppIdx);

        /// Create a blank delta buffer for usein morphing
        HardwareVertexBufferSharedPtr createDeltaBuffer(void);

    };

}

#endif

/*****************************************************************************

	File: NatureTerrainPatch.h
	Desc: A block ot terrain generated from a heightmap
	Date: 2003/02/22

	Author: Martin Persson

*****************************************************************************/

#ifndef __NATURETERRAINPATCH_H
#define __NATURETERRAINPATCH_H

#include <OgrePrerequisites.h>
#include <OgreSimpleRenderable.h>
#include <OgreMaterialManager.h>
#include <OgreHardwareBufferManager.h>

#include "NatureConfig.h"
#include "NaturePatch.h"
#include "NatureSceneManager.h"

namespace Ogre
{

//----------------------------------------------------------------------------

class NatureTerrainPatch : public NaturePatch
{
    class QuadTreeNode
    {
    public:
	enum
	{
	    QUAD_ENABLED = 0x80,
	    ERROR_MASK   = 0x7f
	};

	/// sets errorvalue (0..127)
	inline void setError(unsigned char err)
	{
	    if ((err = err & ERROR_MASK) > (mData & ERROR_MASK))	
		mData = mData | err;
	}

	/// returns the errorvalue (0..127)
	inline unsigned char getError()
	{
	    return (mData & ERROR_MASK);
	}

	/// returns true if this node is enabled
	inline bool isEnabled()
	{
	    return ((mData & QUAD_ENABLED) == QUAD_ENABLED);
	}

	/// marks this node as enabled
	inline void enable()
	{
	    mData = mData | QUAD_ENABLED;
	}

	/// marks this node as disabled
	inline void disable()
	{
	    mData = mData & (~QUAD_ENABLED);
	}

    private:
	unsigned char mData;
    };

public:
    NatureTerrainPatch();

    ~NatureTerrainPatch();

    /// Initializes patch, calculates error metrics
    bool initialise(Vector3 world, Vector3 zone, Vector3 scale,
		    NaturePatchData *data);

    /// Select which vertices to render
    void prepareMesh();
    /// Creates vertex/index/colour... buffers for mesh
    void generateMesh();
    
   
    void getRenderOperation(RenderOperation& op);
    
    void _notifyCurrentCamera(Camera *cam);

    /// Returns height in world units for given position (x/z unscaled)
    Real getHeightAt(int x, int z) const
    {
	return mHeight[z * mData->terrain.heightMapWidth + x]
	       * mScale.y + mWorld.y;
    }

    /// Returns true if a vertex on the north edge is enabled
    bool isNorthEdgeVertexEnabled(int vertexPos)
    {
	return mQuadTree[mManager->mNorthEdgeQuad[vertexPos]].isEnabled();
    }

    /// Returns true if a vertex on the south edge is enabled
    bool isSouthEdgeVertexEnabled(int vertexPos)
    {
	return mQuadTree[mManager->mSouthEdgeQuad[vertexPos]].isEnabled();
    }

    /// Returns true if a vertex on the west edge is enabled
    bool isWestEdgeVertexEnabled(int vertexPos)
    {
	return mQuadTree[mManager->mWestEdgeQuad[vertexPos]].isEnabled();
    }

    /// Returns true if a vertex on the east edge is enabled
    bool isEastEdgeVertexEnabled(int vertexPos)
    {
	return mQuadTree[mManager->mEastEdgeQuad[vertexPos]].isEnabled();
    }
    /** Overridden from MovableObject */
    Real getBoundingRadius(void) const { return 0; /* not needed */ }

private:
    /// Renders a quad
    void renderQuad(int cx, int cz, int node, int width);

    /// Recursively renders selected vertices (well, render to vertexbuffer ;))
    void render(int cx, int cz, int node, int level);

    /// Computes the error metric table for quadtree
    void computeError();

    /// Calculates the error metric for a single quad
    int calculateError(int cx, int cz, int width);

    /// Recursivly selects which quads to render
    void triangulate(int cx, int cz, int node, int level);

    /// Free allocated caches
    void freeCaches();

    /// Calculates normal for vertex
    void getNormal(int x, int z, Vector3 *normal);

    /// Adds a vertex to the vertex buffer or returns its index if it
    inline int NatureTerrainPatch::addVertex(int x, int y, int z)
    {
        unsigned int idx = z * QUADTREE_SIZE + x;
        unsigned int v = mManager->mVertexLookup[idx];

        // if this vertex has already been added, return its index
        if (v != 0xffff) return v;

        int numReals = 3;

#if USE_NORMALS
        numReals += 3;
#endif
#if USE_TEXTURES
        numReals += 4;
#endif

        v = mVertexCount++;
        Real *pBuffer = &mManager->mDataBuffer[v * numReals];

        // add the vertex to the vertexbuffer
        *pBuffer++ = x * mScale.x + mWorld.x * mScale.x;
        *pBuffer++ = y * mScale.y + mWorld.y * mScale.y;
        *pBuffer++ = z * mScale.z + mWorld.z * mScale.z;

#if USE_NORMALS
        // calculate normal
        Vector3 normal;
        getNormal(x, z, &normal);

        // add the normal to the normalbuffer
        *pBuffer++ = normal.x;
        *pBuffer++ = normal.y;
        *pBuffer++ = normal.z;
#endif

#if USE_TEXTURES
        // calculate texture coordinates
        // detail texture
        *pBuffer++ = 1.0 - (static_cast<Real>(x) / EDGE_LENGTH);
        *pBuffer++ = 1.0 - (static_cast<Real>(z) / EDGE_LENGTH);
        // terrain texture
        *pBuffer++ = 0.5*(x + mWorld.x) / mManager->mZoneSize;
        *pBuffer++ = 0.5*(z + mWorld.z) / mManager->mZoneSize;
#endif
        // add vertex to lookup table
        mManager->mVertexLookup[idx] = v;
        return v;
    }

    /// Array of quadtreenodes
    QuadTreeNode mQuadTree[QUADTREE_NODES];

    /// Convenience pointer to quadtreemanager
    NaturePatchManager *mManager;

    /// Pointer to elevation data
    unsigned char *mHeight;

    /// Distance to camera when last rendered
    float mDistance;

    VertexData* mVertexData;
    IndexData* mIndexData;

    /// Local caches
    ulong  *mColourCache;

    // Number of indexes / vertices in the cache
    int	mIndexCount;
    int	mVertexCount;
};

} // namespace Ogre

#endif

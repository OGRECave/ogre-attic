/*****************************************************************************

	File: NaturePatchManager.h
	Desc: 
	Date: 2003/02/22

	Author: Martin Persson

*****************************************************************************/

#ifndef __NATUREPATCHMANAGER_H
#define __NATUREPATCHMANAGER_H

#include <OgrePrerequisites.h>
#include <OgreCamera.h>
#include <OgreSingleton.h>

#include "NatureConfig.h"
#include "NaturePatchLoader.h"

#include <queue>
using namespace std;

namespace Ogre
{

//----------------------------------------------------------------------------

class NaturePatchManager : public Singleton<NaturePatchManager>
{
    friend class NaturePatch;
    friend class NatureTerrainPatch;

public:
    NaturePatchManager();

    ~NaturePatchManager();

    bool initialise(SceneNode *sceneMgr, NaturePatchLoader *loader);

    /// Test material thingy
    Material *createTerrainMaterial();

    /// Sets target quality and rebuilds all the patches
    void setTargetQuality(Real quality);
    /// Sets minimum quality and rebuilds all the patches
    void setMinimumQuality(Real quality);

    /// Called by patches to add themselves to the renderQueue
    void addToRenderQueue(NaturePatch *patch);
    /// Rebuilds patches in renderQueue
    void renderPatches();
    /// Updates patches in array based on cameraposition
    void updatePatches(Camera *cam);
    /// Gets all the patches within an AABB in world coords as GeometryData structs
    virtual void getPatchRenderOpsInBox(const AxisAlignedBox& box, std::list<LegacyRenderOperation>& opList);


private:
    void unloadPatch(int x, int y);
    bool loadPatch(int x, int y, int edge);

    /// Initialises the lookup tables
    bool initLookupTables();
    /// Frees buffers allocated by initLookupTables()
    void freeLookupTables();

    /// Initialises shared buffers
    bool initSharedBuffers();
    /// Frees buffers allocated by initSharedBuffers()
    void freeSharedBuffers();
   
    /**
     *	Prepares quad lookup table, this table is used to quickly find out
     *	what index a quad has based on its center x,y coord
     *
     *	ex. to get the northwest child of the root node:
     *	    mQuadLookupTable[EDGE_LENGTH/4 * QUADTREE_SIZE + EDGE_LENGTH/4]
     */
    void computeQuadLookup(int cx, int cz, int node, int level);

    /**
     *	Prepares neighbor lookup table, this table is used to quickly find out
     *	what index a quads neighbor has, based on its own node index
     *
     *	ex. to get the east neighbor of a node with index i:
     *	    mEastNeighbor[i]
     */
    void computeNeighborLookup(int cx, int cz, int node, int level);

    ///	Clears the vertex lookup table
    inline void clearVertexLookup()
    {
	memset(mVertexLookup, 0xffffffff, 
	       sizeof(ushort) * QUADTREE_SIZE * QUADTREE_SIZE);
    }


    // TODO: fix neighbor pointers instead of using this lame buffer
    class NaturePatch *mPatches[324];

    int mCenterPatchX;
    int mCenterPatchY;
    int mPageSize;


    // Quad node lookup table for NatureTerrainPatches
    short *mQuadNodeLookup;

    // Neighbor node lookup tables for NatureTerrainPatches
    short *mNorthNeighbor;
    short *mSouthNeighbor;
    short *mWestNeighbor;
    short *mEastNeighbor;
     
    // Edge quad lookup tables for NatureTerrainPatches
    short *mNorthEdgeQuad;
    short *mSouthEdgeQuad;
    short *mWestEdgeQuad;
    short *mEastEdgeQuad;

    // Shared buffers used when rendering NatureTerrainPatches
    Real   *mVertexBuffer;
    Real   *mCoordBuffer[2];
    Real   *mNormalBuffer;
    ushort *mIndexBuffer;
    ushort *mVertexLookup;

    /// Quality settings
    Real mMinimumQuality;
    Real mTargetQuality;

    /// Size of the map in unscaled units
    Real mMapSize;
    /// Size of each zone in unscaled units
    Real mZoneSize;


    SceneNode	      *mSceneRoot;
    NaturePatchLoader *mMapLoader;

    queue<NaturePatch *> mRenderQueue;

    bool mInited;

    NaturePatch* getPatchAtPosition(const Vector3& pos);
};

#define g_NaturePatchManager NaturePatchManager::getSingleton()




} // namespace Ogre

#endif

/*****************************************************************************

	File: NaturePatchManager.cpp
	Desc: 
	Date: 2003/02/22

	Author: Martin Persson

*****************************************************************************/

#include "NaturePatch.h"
#include "NatureTerrainPatch.h"
#include "NaturePatchManager.h"

namespace Ogre
{

//----------------------------------------------------------------------------

template<> NaturePatchManager* Singleton<NaturePatchManager>::ms_Singleton = 0;
NaturePatchManager* NaturePatchManager::getSingletonPtr(void)
{
    return ms_Singleton;
}
NaturePatchManager& NaturePatchManager::getSingleton(void)
{  
    assert( ms_Singleton );  return ( *ms_Singleton );  
}

//----------------------------------------------------------------------------

NaturePatchManager::NaturePatchManager()
    : mDataBuffer(0)
{
    mInited = false;

    mQuadNodeLookup = 0;
    
    mNorthNeighbor = mSouthNeighbor = mWestNeighbor = mEastNeighbor = 0;
    mNorthEdgeQuad = mSouthEdgeQuad = mWestEdgeQuad = mEastEdgeQuad = 0;

    mIndexBuffer  = mVertexLookup   = 0;

    mMinimumQuality = 10.0f;
    mTargetQuality  = 5.0f;

    for (int i = 0; i < 324; i++)
        mPatches[i] = 0;
}

//----------------------------------------------------------------------------

NaturePatchManager::~NaturePatchManager()
{
    // delete lookup tables
    freeLookupTables();

    // delete shared buffers
    freeSharedBuffers();

    for (int i = 0; i < 324; i++)
    {
	if (mPatches[i] != 0)
	{
	    mMapLoader->releaseData(mPatches[i]->mData);
	    delete mPatches[i];
	}
    }
}

//----------------------------------------------------------------------------

Material *NaturePatchManager::createTerrainMaterial()
{
#if 0
    static int count = 0;
    // START TEST!!!
    Material *mMaterial = mSceneRoot->getCreator()->createMaterial("NatureQuadMaterial" + toString(count++));
#if USE_TEXTURES
    TextureUnitState *layer;

    layer = mMaterial->addTextureLayer("grass_1024.jpg", 0);
    layer->setColourOperation(LBO_REPLACE);

    layer = mMaterial->addTextureLayer("terr_dirt-grass.jpg", 0);
    layer->setColourOperationEx(LBX_BLEND_DIFFUSE_COLOUR, LBS_TEXTURE, LBS_CURRENT);
    layer->setTextureScale(0.5, 0.5);

    layer = mMaterial->addTextureLayer("snow_1024.jpg", 0);
    layer->setColourOperationEx(LBX_BLEND_DIFFUSE_ALPHA, LBS_TEXTURE, LBS_CURRENT);
    layer->setTextureScale(0.5, 0.5);

    layer = mMaterial->addTextureLayer("lightmap_test.jpg", 1);
    layer->setColourOperationEx(LBX_MODULATE, LBS_TEXTURE, LBS_CURRENT);

/*  // test...
    layer = mMaterial->addTextureLayer("grass_1024.jpg", 0);

    layer = mMaterial->addTextureLayer("grass_blend.png", 0);
    layer->setColourOperationEx(LBX_BLEND_TEXTURE_ALPHA, LBS_TEXTURE, LBS_CURRENT);

    layer = mMaterial->addTextureLayer("terr_rock6.jpg", 0);
    layer->setColourOperationEx(LBX_BLEND_CURRENT_ALPHA, LBS_TEXTURE, LBS_CURRENT);
*/
#endif
#if USE_NORMALS
    mMaterial->setLightingEnabled(true);
//  mMaterial->setShadingMode(SO_PHONG);
#else
    mMaterial->setLightingEnabled(false);
#endif
    
//	mMaterial->setTextureFiltering(TFO_NONE);

    return mMaterial;
#endif
    return 0;
}

//----------------------------------------------------------------------------

bool NaturePatchManager::initialise(SceneNode *sceneRoot,
				    NaturePatchLoader *loader)
{
    if (!mInited)
    {
	mSceneRoot  = sceneRoot;
	mMapLoader = loader;

	if (!initLookupTables())
	    return false;
    
	if (!initSharedBuffers())
	{
	    freeLookupTables();
	    return false;
	}

	// get the map and zone size
	mMapSize  = loader->getMapSize();
	mZoneSize = loader->getZoneSize();

	mPageSize = 17;	// get this from loader

	// set to some some large value will force reload of all patches
	mCenterPatchX = -10000000;
	mCenterPatchY = -10000000;
	
	mInited = true;
    }

    return true;
}

//----------------------------------------------------------------------------

void NaturePatchManager::freeLookupTables()
{
    // delete lookup tables
    if (mQuadNodeLookup != 0)	
	delete[] mQuadNodeLookup;

    if (mNorthNeighbor != 0)	
	delete[] mNorthNeighbor;

    if (mSouthNeighbor != 0)	
	delete[] mSouthNeighbor;

    if (mWestNeighbor != 0)	
	delete[] mWestNeighbor;

    if (mEastNeighbor != 0)	
	delete[] mEastNeighbor;

    if (mNorthEdgeQuad != 0)
	delete[] mNorthEdgeQuad;

    if (mSouthEdgeQuad != 0)
	delete[] mSouthEdgeQuad;

    if (mWestEdgeQuad != 0)
	delete[] mWestEdgeQuad;

    if (mEastEdgeQuad != 0)
	delete[] mEastEdgeQuad;


    // clear the pointers
    mQuadNodeLookup = 0;
    mNorthNeighbor = mSouthNeighbor = mWestNeighbor = mEastNeighbor = 0;
    mNorthEdgeQuad = mSouthEdgeQuad = mWestEdgeQuad = mEastEdgeQuad = 0;
}

//----------------------------------------------------------------------------

bool NaturePatchManager::initLookupTables()
{
    // allocate new memory for the lookup tables
    mQuadNodeLookup = new short[QUADTREE_SIZE * QUADTREE_SIZE];
    mNorthNeighbor  = new short[QUADTREE_NODES];
    mSouthNeighbor  = new short[QUADTREE_NODES];
    mWestNeighbor   = new short[QUADTREE_NODES];
    mEastNeighbor   = new short[QUADTREE_NODES];

    mSouthEdgeQuad  = new short[QUADTREE_NODES];
    mNorthEdgeQuad  = new short[QUADTREE_NODES];
    mWestEdgeQuad   = new short[QUADTREE_NODES];
    mEastEdgeQuad   = new short[QUADTREE_NODES];

    if (mQuadNodeLookup == 0 || mNorthNeighbor == 0 ||
	mSouthNeighbor  == 0 || mWestNeighbor  == 0 ||
	mEastNeighbor   == 0 || mNorthEdgeQuad == 0 || 
	mSouthEdgeQuad  == 0 || mWestEdgeQuad  == 0 ||
	mEastEdgeQuad   == 0)
    {
	freeLookupTables();
	return false;
    }

    // clear the quad node lookup table
    memset(mQuadNodeLookup, 0, sizeof(short)*QUADTREE_SIZE*QUADTREE_SIZE);

    // calculate lookup table for quad nodes
    computeQuadLookup(EDGE_LENGTH / 2, EDGE_LENGTH / 2, 0, 0);

    // calculate neighbor lookup tables
    computeNeighborLookup(EDGE_LENGTH / 2, EDGE_LENGTH / 2, 0, 0);

    return true;
}

//----------------------------------------------------------------------------

void NaturePatchManager::freeSharedBuffers()
{
    // delete shared buffers
    if (mDataBuffer != 0)
        delete[] mDataBuffer;
    
    if (mIndexBuffer != 0)
        delete[] mIndexBuffer;
    
    if (mVertexLookup != 0)
        delete[] mVertexLookup;

    // clear the pointers
    mDataBuffer = 0;
    mIndexBuffer  = mVertexLookup   = 0;
}

//----------------------------------------------------------------------------

bool NaturePatchManager::initSharedBuffers()
{
    // allocate new memory for shared buffers
    mDataBuffer   = new Real[QUADTREE_SIZE * QUADTREE_SIZE * (3 + 3 + 4)];
    mIndexBuffer  = new ushort[EDGE_LENGTH * EDGE_LENGTH * 2 * 3];
    mVertexLookup = new ushort[QUADTREE_SIZE * QUADTREE_SIZE];

    // return false if allocation failed
    if (mDataBuffer == 0 || mIndexBuffer == 0 || mVertexLookup == 0)
    {
        freeSharedBuffers();
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------

void NaturePatchManager::computeQuadLookup(int cx, int cz, int node, int level)
{
    // if not at lowest level, descend into children
    if (level < (QUADTREE_DEPTH - 1))
    {
	int w4 = EDGE_LENGTH >> (level + 2);

	// northwest child
	computeQuadLookup(cx - w4, cz - w4, (node<<2)+1, level+1);
	// northeast child
	computeQuadLookup(cx + w4, cz - w4, (node<<2)+2, level+1);
	// southwest child
	computeQuadLookup(cx - w4, cz + w4, (node<<2)+3, level+1);
	// southeast child
	computeQuadLookup(cx + w4, cz + w4, (node<<2)+4, level+1);
    }

    mQuadNodeLookup[cz * QUADTREE_SIZE + cx] = node;
}

//----------------------------------------------------------------------------

void NaturePatchManager::computeNeighborLookup(int cx,int cz,int node,int level)
{
    int nx, nz, nIdx;
    int width = EDGE_LENGTH >> level;

    // calculate index of north neighbor
    nx = cx;
    nz = cz - width;
    nIdx = (nz * QUADTREE_SIZE) + nx;

    if (nz < 0)
    {
	nIdx += EDGE_LENGTH * QUADTREE_SIZE;
	mNorthNeighbor[node] = -mQuadNodeLookup[nIdx];
	mNorthEdgeQuad[cx]   = node;
    }
    else
	mNorthNeighbor[node] = mQuadNodeLookup[nIdx];

    // calculate index of south neighbor
    nx = cx;
    nz = cz + width;
    nIdx = (nz * QUADTREE_SIZE) + nx;

    if (nz > EDGE_LENGTH)
    {
	nIdx -= EDGE_LENGTH * QUADTREE_SIZE;
	mSouthNeighbor[node] = -mQuadNodeLookup[nIdx];
	mSouthEdgeQuad[cx]   = node;
    }
    else
	mSouthNeighbor[node] = mQuadNodeLookup[nIdx];

    // calculate index of west neighbor
    nx = cx - width;
    nz = cz;
    nIdx = (nz * QUADTREE_SIZE) + nx;

    if (nx < 0)
    {
	nIdx += EDGE_LENGTH;
	mWestNeighbor[node] = -mQuadNodeLookup[nIdx];
	mWestEdgeQuad[cz]   = node;
    }
    else
	mWestNeighbor[node] = mQuadNodeLookup[nIdx];

    // calculate index of east neighbor
    nx = cx + width;
    nz = cz;
    nIdx = (nz * QUADTREE_SIZE) + nx;

    if (nx > EDGE_LENGTH)
    {
	nIdx -= EDGE_LENGTH;
	mEastNeighbor[node] = -mQuadNodeLookup[nIdx];
	mEastEdgeQuad[cz]   = node;
    }
    else
	mEastNeighbor[node] = mQuadNodeLookup[nIdx];

    // if not at lowest level, descend into children
    if (level < (QUADTREE_DEPTH - 1))
    {
	int w4 = EDGE_LENGTH >> (level + 2);

	// northwest child
	computeNeighborLookup(cx - w4, cz - w4, (node<<2)+1, level+1);
	// northeast child
	computeNeighborLookup(cx + w4, cz - w4, (node<<2)+2, level+1);
	// southwest child
	computeNeighborLookup(cx - w4, cz + w4, (node<<2)+3, level+1);
	// southeast child
	computeNeighborLookup(cx + w4, cz + w4, (node<<2)+4, level+1);
    }
}

//----------------------------------------------------------------------------

void NaturePatchManager::setTargetQuality(Real quality)
{
    mTargetQuality = quality;

    for (int i = 0; i < 324; i++)
    {
	if (mPatches[i] != 0)
	    addToRenderQueue(mPatches[i]);
    }
}

//----------------------------------------------------------------------------

void NaturePatchManager::setMinimumQuality(Real quality)
{
    mMinimumQuality = quality;

    for (int i = 0; i < 324; i++)
    {
  	if (mPatches[i] != 0)
	    addToRenderQueue(mPatches[i]);
    }
}

//----------------------------------------------------------------------------

void NaturePatchManager::renderPatches()
{
    queue<NaturePatch *> regenQueue;
    bool empty = mRenderQueue.empty();

    while (!mRenderQueue.empty())
    {
	NaturePatch *patch = mRenderQueue.front();
	mRenderQueue.pop();

	// rebuild mesh
	patch->mNeedRendering = false;
	patch->prepareMesh();

/*	if (patch->mNorthNeighbor != 0) patch->mNorthNeighbor->prepareMesh();
	if (patch->mSouthNeighbor != 0) patch->mSouthNeighbor->prepareMesh();
	if (patch->mWestNeighbor  != 0) patch->mWestNeighbor->prepareMesh();
	if (patch->mEastNeighbor  != 0) patch->mEastNeighbor->prepareMesh();
*/
	if (patch->mNorthNeighbor != 0) patch->mNorthNeighbor->mNeedRendering = true;
	if (patch->mSouthNeighbor != 0) patch->mSouthNeighbor->mNeedRendering = true;
	if (patch->mWestNeighbor  != 0) patch->mWestNeighbor->mNeedRendering = true;
	if (patch->mEastNeighbor  != 0) patch->mEastNeighbor->mNeedRendering = true;

	// add to regen queue
	regenQueue.push(patch);
    }

    while (!regenQueue.empty())
    {
	NaturePatch *patch = regenQueue.front();
	regenQueue.pop();

	// regenerate the mesh
	patch->generateMesh();

	// regenerate neighbors (to keep edges up to date)
	if (patch->mNorthNeighbor != 0) patch->mNorthNeighbor->generateMesh();
	if (patch->mSouthNeighbor != 0) patch->mSouthNeighbor->generateMesh();
	if (patch->mWestNeighbor  != 0) patch->mWestNeighbor->generateMesh();
	if (patch->mEastNeighbor  != 0) patch->mEastNeighbor->generateMesh();
    }
}

//----------------------------------------------------------------------------

void NaturePatchManager::addToRenderQueue(NaturePatch *patch)
{
    mRenderQueue.push(patch);
}

//----------------------------------------------------------------------------

bool NaturePatchManager::loadPatch(int x, int y, int edge)
{
    bool loaded = false;

    // calculate index into patch array
    int ax = (x < 0) ? ((x % mPageSize) + mPageSize) : (x % mPageSize);
    int ay = (y < 0) ? ((y % mPageSize) + mPageSize) : (y % mPageSize);

    int idx = ay * mPageSize + ax;

    if (mPatches[idx] == 0)
    {
        NaturePatch::NaturePatchData *data;
        Vector3 world, zone, scale;

        // request data for this patch from the loader
        data = mMapLoader->requestData(x, y, &world, &zone, &scale);

        if (data != 0)
        {
            switch (data->type)
            {
            case NaturePatch::TYPE_TERRAIN:
                mPatches[idx] = new NatureTerrainPatch();
                break;
	    
            // INFO: ADD MORE PATCH TYPES HERE

            default:
                std::cout << "ERROR: Unsupported patch type!!!\n";
            }

	    if (mPatches[idx] != 0)
	    {
		mPatches[idx]->initialise(world, zone, scale, data);

		addToRenderQueue(mPatches[idx]);

		String name = "NaturePatch["+toString(x)+","+toString(y)+"]";
		SceneNode *sn = mSceneRoot->createChildSceneNode(name);
		sn->attachObject(mPatches[idx]);

//		std::cout << "ADDING: " << name << std::endl;

		// setup neighbor pointers
	    NaturePatch *n = 0, *s = 0, *w = 0, *e = 0;
		int maxIdx = mPageSize * mPageSize;

	        if ((edge & 0x01) == 0)
		{
		    // link north neighbor
		    if (idx >= mPageSize) n = mPatches[idx - mPageSize];
		    else n = mPatches[maxIdx + (idx - mPageSize)];
		}

		if ((edge & 0x02) == 0)
		{
		    // link south neighbor
		    if (idx < (maxIdx - mPageSize)) s = mPatches[idx + mPageSize];
		    else s = mPatches[(idx + mPageSize) - maxIdx];
		}

		if ((edge & 0x04) == 0)
		{
		    // link west neighbor
		    if ((idx % mPageSize) > 0) w = mPatches[idx - 1];
		    else w = mPatches[idx + (mPageSize - 1)];
		}

		if ((edge & 0x08) == 0)
		{
		    // link east neighbor
		    if ((idx % mPageSize) < (mPageSize - 1)) e = mPatches[idx + 1];
		    else e = mPatches[idx - (mPageSize - 1)];
		}

		// attach patch to its neighbors
		mPatches[idx]->attach(n, s, w, e);
	    
		loaded = true;
	    }
	}
    }

    return loaded;
}

//----------------------------------------------------------------------------

void NaturePatchManager::unloadPatch(int x, int y)
{
    // calculate index into patch array
    int ax = (x < 0) ? ((x % mPageSize) + mPageSize) : (x % mPageSize);
    int ay = (y < 0) ? ((y % mPageSize) + mPageSize) : (y % mPageSize);

    int idx = ay * mPageSize + ax;

//    std::cout << "UNLOADING: " << ax << ", " << x << "  idx: " << idx << std::endl;

    if (mPatches[idx] != 0)
    {
	// unlink neighbors
	mPatches[idx]->detach();

	// remove patch from the scene
	String name = "NaturePatch[" + toString(x) + "," + toString(y) + "]";
	mSceneRoot->removeAndDestroyChild(name);

//	std::cout << "REMOVING: " << name << std::endl;

	// inform the patchloader that we no longer need this data
	mMapLoader->releaseData(mPatches[idx]->mData);

	// delete patch
	delete mPatches[idx];
	mPatches[idx] = 0;
    }
}

//----------------------------------------------------------------------------

void NaturePatchManager::updatePatches(Camera *cam)
{
    int cx, cy;
    mMapLoader->getPatchAtPosition(cam->getPosition(), &cx, &cy);


    int diffx = cx - mCenterPatchX;
    int diffy = cy - mCenterPatchY;
    int ps2 = mPageSize / 2, x, y;

    // load in new patches
    for (y = -ps2; y <= ps2; y++)
    {
	for (x = -ps2; x <= ps2; x++)
	{
	    int edge = 0;

	    if (y == -ps2)     edge |= 0x01;
	    else if (y == ps2) edge |= 0x02;
	    if (x == -ps2)     edge |= 0x04;
	    else if (x == ps2) edge |= 0x08;

	    // TODO: move patch loading to separate thread?
	    if (loadPatch(x + mCenterPatchX, y + mCenterPatchY, edge))
		return;
	}
    }

    if (diffy != 0 || diffx != 0)
    {
	// unload patches on x axis
	if (diffy > 0)
	{
	    if (diffy > mPageSize) diffy = mPageSize;

	    for (y = -ps2; y < (-ps2 + diffy); y++)
		for (x = -ps2; x <= ps2; x++)
		    unloadPatch(x + mCenterPatchX, y + mCenterPatchY);
	}
	else if (diffy < 0)
	{
	    if (diffy < -mPageSize) diffy = -mPageSize;
	    
	    for (y = ps2; y > (ps2 + diffy); y--)
		for (x = -ps2; x <= ps2; x++)
		    unloadPatch(x + mCenterPatchX, y + mCenterPatchY);
	}

	// unload patches on y axis
	if (diffx > 0)
	{
	    if (diffx > mPageSize) diffx = mPageSize;
	    
	    for (x = -ps2; x < -ps2 + diffx; x++)
		for (y = -ps2; y <= ps2; y++)
		    unloadPatch(x + mCenterPatchX, y + mCenterPatchY);
	}
	else if (diffx < 0)
	{
	    if (diffx < -mPageSize) diffx = -mPageSize;
	    
	    for (x = ps2; x > (ps2 + diffx); x--)
		for (y = -ps2; y <= ps2; y++)
		    unloadPatch(x + mCenterPatchX, y + mCenterPatchY);
	}

	// update center position
	mCenterPatchX = cx;
	mCenterPatchY = cy;
    }
}

NaturePatch* NaturePatchManager::getPatchAtPosition(const Vector3& pos)
{
    // NB only works if loaded
    // Get location
    int x, y;
    mMapLoader->getPatchAtPosition(pos, &x, &y);

    int idx = y * mPageSize + x;

    return mPatches[idx];

}

void NaturePatchManager::getPatchRenderOpsInBox(const AxisAlignedBox& box, std::list<RenderOperation>& opList)
{
    // Get the patches at the 4 corners at the bottom of the box, ie 0, 3, 6, 7
    std::set<NaturePatch*> uniqueSet;

    const Vector3* corners = box.getAllCorners();

    uniqueSet.insert(getPatchAtPosition(corners[0]));
    uniqueSet.insert(getPatchAtPosition(corners[3]));
    uniqueSet.insert(getPatchAtPosition(corners[6]));
    uniqueSet.insert(getPatchAtPosition(corners[7]));

    // Iterate over uniques
    std::set<NaturePatch*>::iterator i, iend;
	RenderOperation op;
    for (i = uniqueSet.begin(); i != iend; ++i)
    {
		(*i)->getRenderOperation(op);
        opList.push_back(op);
    }


}


} // namespace Ogre

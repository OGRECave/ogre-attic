/*****************************************************************************

	File: NatureTerrainPatch.cpp
	Desc: A block ot terrain generated from a heightmap
	Date: 2003/02/22

	Author: Martin Persson

*****************************************************************************/

#include <iostream>

#include <OgreCamera.h>

#include "NatureTerrainPatch.h"

namespace Ogre
{

//----------------------------------------------------------------------------

NatureTerrainPatch::NatureTerrainPatch()
    : NaturePatch(),
      mVertexData(0),
      mIndexData(0)
{
    memset(mQuadTree, 0, sizeof(QuadTreeNode) * QUADTREE_NODES);
    
    mNeedRendering = false;

    mDistance = 500.0f;

    mColourCache = 0;

    mVertexCount = 0;
    mIndexCount  = 0;
}

//----------------------------------------------------------------------------

NatureTerrainPatch::~NatureTerrainPatch()
{
    freeCaches();
}

//----------------------------------------------------------------------------

void NatureTerrainPatch::freeCaches()
{
    if (mVertexData)
    {
        delete mVertexData;
        mVertexData = 0;
    }

    if (mIndexData)
    {
        delete mIndexData;
        mIndexData = 0;
    }

    if (mColourCache != 0)
    {
	delete[] mColourCache;
	mColourCache = 0;
    }

}

//----------------------------------------------------------------------------

bool NatureTerrainPatch::initialise(Vector3 world, Vector3 zone,
				    Vector3 scale, NaturePatchData *data)
{
    // setup pointer to NaturePatchManager
    mManager = &NaturePatchManager::getSingleton();

    // setup pointer to elevation data
    mHeight   = data->terrain.pHeightMap;
    mMaterial = data->terrain.pMaterial;

    // setup world position and scaling info
    mWorld = world;
    mZone  = zone;
    mScale = scale;

    // save patch data structure
    mData = data;

    // set bounding box extents, TODO: fix this so y gets set correctly
    mBounds.setExtents(mWorld.x * mScale.x,
		       mWorld.y,
		       mWorld.z * mScale.z,
		       mWorld.x * mScale.x + EDGE_LENGTH * mScale.x,
		       mWorld.y + 255 * mScale.y,	    // wrong
		       mWorld.z  * mScale.z + EDGE_LENGTH * mScale.z);

    // calculate errors for the quadtree
    computeError();

    return true;
}

//----------------------------------------------------------------------------

int NatureTerrainPatch::calculateError(int cx, int cz, int width)
{
    int xAdd = (width >> 1);
    int zAdd = (width >> 1) * mData->terrain.heightMapWidth;

    // calculate index of center vertex
    int cIdx = cz * mData->terrain.heightMapWidth + cx;

    int nIdx = cIdx - zAdd;	// index of north vertex
    int sIdx = cIdx + zAdd;	// index of south vertex
    int wIdx = cIdx - xAdd;	// index of west vertex
    int eIdx = cIdx + xAdd;	// index of east vertex

    int nwIdx = nIdx - xAdd;	// index of northwest vertex
    int neIdx = nIdx + xAdd;    // index of northeast vertex
    int swIdx = sIdx - xAdd;    // index of southwest vertex
    int seIdx = sIdx + xAdd;    // index of southeast vertex

    int e1, e2;

    // north error
    e1 = abs((mHeight[nIdx] - ((mHeight[nwIdx] + mHeight[neIdx]) >> 1)));

    // south error
    e2 = abs((mHeight[sIdx] - ((mHeight[swIdx] + mHeight[seIdx]) >> 1)));
    if (e1 < e2) e1 = e2;

    // west error
    e2 = abs((mHeight[wIdx] - ((mHeight[nwIdx] + mHeight[swIdx]) >> 1)));
    if (e1 < e2) e1 = e2;

    // east error
    e2 = abs((mHeight[eIdx] - ((mHeight[neIdx] + mHeight[seIdx]) >> 1)));
    if (e1 < e2) e1 = e2;

    // diagonal 1 error
    e2 = abs((mHeight[cIdx] - ((mHeight[nwIdx] + mHeight[seIdx]) >> 1)));
    if (e1 < e2) e1 = e2;

    // diagonal 2 error
    e2 = abs((mHeight[cIdx] - ((mHeight[neIdx] + mHeight[swIdx]) >> 1)));
    if (e1 < e2) e1 = e2;

    // clamp value to [0..127]
    if (e1 > 127) return 127;

    // return max error
    return e1;
}

//----------------------------------------------------------------------------

void NatureTerrainPatch::prepareMesh()
{
    if (!mNeedRendering)
    {
	for (int q = 0; q < QUADTREE_NODES; q++)
	    mQuadTree[q].disable();

	triangulate(EDGE_LENGTH / 2, EDGE_LENGTH / 2, 0, 0);

	mNeedRendering = true;
    }
}

//----------------------------------------------------------------------------

void NatureTerrainPatch::generateMesh()
{
    if (mNeedRendering)
    {
	// prepare for rendering
	mManager->clearVertexLookup();

	mVertexCount = 0;
	mIndexCount  = 0;

	// free any previous caches
	freeCaches();

	render(EDGE_LENGTH / 2, EDGE_LENGTH / 2, 0, 0);

    mVertexData = new VertexData;
    mVertexData->vertexStart = 0;
    mVertexData->vertexCount = mVertexCount;

    VertexDeclaration* decl = mVertexData->vertexDeclaration;
    VertexBufferBinding* bind = mVertexData->vertexBufferBinding;

    size_t offset = 0;

    // positions
    decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
    offset += VertexElement::getTypeSize(VET_FLOAT3);

#if USE_NORMALS
    // normals
    decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
    offset += VertexElement::getTypeSize(VET_FLOAT3);
#endif

#if USE_TEXTURES
    // texture coord sets
    decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
    offset += VertexElement::getTypeSize(VET_FLOAT2);
    decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1);
    offset += VertexElement::getTypeSize(VET_FLOAT2);
#endif

#if USE_COLOURS
	// update colours
    decl->addElement(1, 0, VET_COLOUR, VES_DIFFUSE);
#endif

    HardwareVertexBufferSharedPtr vbuf =
        HardwareBufferManager::getSingleton().createVertexBuffer(
            offset, 
            mVertexData->vertexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);
    bind->setBinding(0, vbuf);

	// update buffer
    vbuf->writeData(0, vbuf->getSizeInBytes(), mManager->mDataBuffer);

#if USE_COLOURS
    /* TODO
	// update colours
	mColourCache = new unsigned long[mVertexCount];

	// TODO: fix colours... this is just a test
	for (int i = 0; i < mVertexCount; i++)
	{
	    int h = static_cast<int>(mVertexCache[(i * 3) + 1] / mScale.y);

	    if (h > 120)
		mColourCache[i] = 0xffffffff;
	    else if (h > 80)
		mColourCache[i] = 0x00ffffff;
	    else
		mColourCache[i] = 0x00000000;
	}
    */
#endif

    mIndexData = new IndexData;
    mIndexData->indexStart = 0;
    mIndexData->indexCount = mIndexCount;

    mIndexData->indexBuffer =
        HardwareBufferManager::getSingleton().createIndexBuffer(
            HardwareIndexBuffer::IT_16BIT,
            mIndexData->indexCount,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);

    // update index buffer
    mIndexData->indexBuffer->writeData(0, 
        mIndexData->indexBuffer->getSizeInBytes(), mManager->mIndexBuffer);

	mNeedRendering = false;
    }
//    saveASC(mVertexCount * 3, mIndexCount);
}

//----------------------------------------------------------------------------
#if 0
// simple, non-seaming version
void NatureTerrainPatch::renderQuad(int cx, int cz, int node, int width)
{
    int w2 = (width >> 1), v[6];
    
    int cIdx = cz * mData->heightMapWidth + cx;
    int xAdd = w2;
    int zAdd = w2 * mData->heightMapWidth;

    ushort *indexBuffer = &mManager->mIndexBuffer[mIndexCount];

    // add the base vertices
    v[1] = addVertex(cx - w2, mHeight[cIdx - xAdd - zAdd], cz - w2);
    v[2] = addVertex(cx + w2, mHeight[cIdx + xAdd - zAdd], cz - w2);
    v[3] = addVertex(cx - w2, mHeight[cIdx - xAdd + zAdd], cz + w2);
    v[4] = addVertex(cx + w2, mHeight[cIdx + xAdd + zAdd], cz + w2);

    // figure out which vertices to draw, TODO: check width > 2
    if (width > 2 || mQuadTree[node].isEnabled())
    {
        int neighbor;

	// add the center vertex
	v[0] = addVertex(cx, mHeight[cIdx], cz);

	// render north triangles
	neighbor = mManager->mNorthNeighbor[node];
	if (neighbor > 0 && mQuadTree[neighbor].isEnabled())
	{
	    v[5] = addVertex(cx, mHeight[cIdx - zAdd], cz - w2);

	    *indexBuffer++ = v[2];
	    *indexBuffer++ = v[5];
	    *indexBuffer++ = v[0];

	    *indexBuffer++ = v[5];
	    *indexBuffer++ = v[1];
	    *indexBuffer++ = v[0];
	}
	else if
	{
	    *indexBuffer++ = v[2];
	    *indexBuffer++ = v[1];
	    *indexBuffer++ = v[0];
	}

	// render south triangles
	neighbor = mManager->mSouthNeighbor[node];
	if (neighbor > 0 && mQuadTree[neighbor].isEnabled())
	{
	    v[5] = addVertex(cx, mHeight[cIdx + zAdd], cz + w2);

	    *indexBuffer++ = v[3];
	    *indexBuffer++ = v[5];
	    *indexBuffer++ = v[0];

	    *indexBuffer++ = v[5];
	    *indexBuffer++ = v[4];
	    *indexBuffer++ = v[0];
	}
	else if
	{
	    *indexBuffer++ = v[3];
	    *indexBuffer++ = v[4];
	    *indexBuffer++ = v[0];
	}

	// render west triangles
	neighbor = mManager->mWestNeighbor[node];
	if (neighbor > 0 && mQuadTree[neighbor].isEnabled())
	{
	    v[5] = addVertex(cx - w2, mHeight[cIdx - xAdd], cz);

	    *indexBuffer++ = v[1];
	    *indexBuffer++ = v[5];
	    *indexBuffer++ = v[0];

	    *indexBuffer++ = v[5];
	    *indexBuffer++ = v[3];
	    *indexBuffer++ = v[0];
	}
	else if
        {
	    *indexBuffer++ = v[1];
	    *indexBuffer++ = v[3];
	    *indexBuffer++ = v[0];
	}

	// render east triangles
	neighbor = mManager->mEastNeighbor[node];
	if (neighbor > 0 && mQuadTree[neighbor].isEnabled())
	{
	    v[5] = addVertex(cx + w2, mHeight[cIdx + xAdd], cz);

	    *indexBuffer++ = v[4];
	    *indexBuffer++ = v[5];
	    *indexBuffer++ = v[0];

	    *indexBuffer++ = v[5];
	    *indexBuffer++ = v[2];
	    *indexBuffer++ = v[0];
	}
	else if
        {
	    *indexBuffer++ = v[4];
	    *indexBuffer++ = v[2];
	    *indexBuffer++ = v[0];
	}

    }
    else // quad disabled, just render 2 tri's
    {
	// find how we should draw the diagonal
	int child = (node - 1) & 0x03;

	if (child == 0 || child == 3)
	{
	    *indexBuffer++ = v[2];
	    *indexBuffer++ = v[1];
	    *indexBuffer++ = v[4];

	    *indexBuffer++ = v[1];
	    *indexBuffer++ = v[3];
	    *indexBuffer++ = v[4];
	}
	else
	{
	    *indexBuffer++ = v[2];
	    *indexBuffer++ = v[1];
	    *indexBuffer++ = v[3];

	    *indexBuffer++ = v[3];
	    *indexBuffer++ = v[4];
	    *indexBuffer++ = v[2];
	}
    }

    // update index counter
    mIndexCount += (indexBuffer - (&mManager->mIndexBuffer[mIndexCount]));
}
#endif

//----------------------------------------------------------------------------

void NatureTerrainPatch::renderQuad(int cx, int cz, int node, int width)
{
    int w2 = (width >> 1), v[6];
    
    int cIdx = cz * mData->terrain.heightMapWidth + cx;
    int xAdd = w2;
    int zAdd = w2 * mData->terrain.heightMapWidth;

    ushort *indexBuffer = &mManager->mIndexBuffer[mIndexCount];

    // add the base vertices
    v[1] = addVertex(cx - w2, mHeight[cIdx - xAdd - zAdd], cz - w2);
    v[2] = addVertex(cx + w2, mHeight[cIdx + xAdd - zAdd], cz - w2);
    v[3] = addVertex(cx - w2, mHeight[cIdx - xAdd + zAdd], cz + w2);
    v[4] = addVertex(cx + w2, mHeight[cIdx + xAdd + zAdd], cz + w2);

    // figure out which vertices to draw, TODO: check width > 2
    if (width > 2 || mQuadTree[node].isEnabled())
    {
        int neighbor;

	// add the quads center vertex
	v[0] = addVertex(cx, mHeight[cIdx], cz);

	// render north triangles
	neighbor = mManager->mNorthNeighbor[node];
	if ((neighbor > 0 && mQuadTree[neighbor].isEnabled()))
	{
	    // add a center vertex on the base of the north triangle,
	    // splitting it into 2 triangles
	    v[5] = addVertex(cx, mHeight[cIdx - zAdd], cz - w2);

	    // add indexes for the first triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[2];
	    *indexBuffer++ = v[5];

	    // add indexes for the second triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[5];
	    *indexBuffer++ = v[1];
	}
	else if (neighbor <= 0 && mNorthNeighbor != 0)
	{
	    // add the first 2 indexes of the first triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[2];

	    // iterate through the vertices on the neighbor edge to find which ones are enabled
	    for (int pos = cx + w2 - 2; pos > cx - w2; pos -= 2)
	    {
		// check if neighborvertex on south edge at pos is enabled
		if (mNorthNeighbor->isSouthEdgeVertexEnabled(pos))
		{
		    // add a vertex matching the vertex on the neighbor edge, close the open triangle and add the first 2 indexes of the next triangle
		    v[5] = addVertex(pos, mHeight[(cz - w2) * mData->terrain.heightMapWidth + pos], cz - w2);
		    *indexBuffer++ = v[5];
		    *indexBuffer++ = v[0];
		    *indexBuffer++ = v[5];
		}
	    }

	    // close the open triangle
	    *indexBuffer++ = v[1];
	}
	else
	{
	    // neighbor quad is disabled, just render 1 triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[2];
	    *indexBuffer++ = v[1];
	}

	// render south triangles
	neighbor = mManager->mSouthNeighbor[node];
	if ((neighbor > 0 && mQuadTree[neighbor].isEnabled()))
	{
	    // add a center vertex on the base of the south triangle,
	    // splitting it into 2 triangles
	    v[5] = addVertex(cx, mHeight[cIdx + zAdd], cz + w2);

	    // add indexes for the first triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[3];
	    *indexBuffer++ = v[5];

	    // add indexes for the second triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[5];
	    *indexBuffer++ = v[4];
	}
	else if (neighbor <= 0 && mSouthNeighbor != 0)
	{
	    // add the first 2 indexes of the first triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[3];

	    // iterate through the vertices on the neighbor edge to find which ones are enabled
	    for (int pos = cx - w2 + 2; pos < cx + w2; pos += 2)
	    {
		// check if neighborvertex on north edge at pos is enabled
		if (mSouthNeighbor->isNorthEdgeVertexEnabled(pos))
		{
		    // add a vertex matching the vertex on the neighbor edge, close the open triangle and add the first 2 indexes of the next triangle
		    v[5] = addVertex(pos, mHeight[(cz + w2) * mData->terrain.heightMapWidth + pos], cz + w2);
		    *indexBuffer++ = v[5];
		    *indexBuffer++ = v[0];
		    *indexBuffer++ = v[5];
		}
	    }

	    // close the open triangle
	    *indexBuffer++ = v[4];
	}
	else
	{
	    // neighbor quad is disabled, just render 1 triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[3];
	    *indexBuffer++ = v[4];
	}

	// render west triangles
	neighbor = mManager->mWestNeighbor[node];
	if ((neighbor > 0 && mQuadTree[neighbor].isEnabled()))
	{
	    // add a center vertex on the base of the west triangle,
	    // splitting it into 2 triangles
	    v[5] = addVertex(cx - w2, mHeight[cIdx - xAdd], cz);

	    // add indexes for the first triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[1];
	    *indexBuffer++ = v[5];

	    // add indexes for the second triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[5];
	    *indexBuffer++ = v[3];
	}
	else if (neighbor <= 0 && mWestNeighbor != 0)
	{
	    // add the first 2 indexes of the first triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[1];

	    // iterate through the vertices on the neighbor edge to find which ones are enabled
	    for (int pos = cz - w2 + 2; pos < cz + w2; pos += 2)
	    {
		// check if neighborvertex on east edge at pos is enabled
		if (mWestNeighbor->isEastEdgeVertexEnabled(pos))
		{
		    // add a vertex matching the vertex on the neighbor edge, close the open triangle and add the first 2 indexes of the next triangle
		    v[5] = addVertex(cx - w2, mHeight[pos * mData->terrain.heightMapWidth + (cx - w2)], pos);
		    *indexBuffer++ = v[5];
		    *indexBuffer++ = v[0];
		    *indexBuffer++ = v[5];
		}
	    }

	    // close the open triangle
	    *indexBuffer++ = v[3];
	}
	else
        {
	    // neighbor quad is disabled, just render 1 triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[1];
	    *indexBuffer++ = v[3];
	}

	// render east triangles
	neighbor = mManager->mEastNeighbor[node];
	if ((neighbor > 0 && mQuadTree[neighbor].isEnabled()))
	{
	    // add a center vertex on the base of the east triangle,
	    // splitting it into 2 triangles
	    v[5] = addVertex(cx + w2, mHeight[cIdx + xAdd], cz);

	    // add indexes for the first triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[4];
	    *indexBuffer++ = v[5];

	    // add indexes for the second triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[5];
	    *indexBuffer++ = v[2];
	}
	else if (neighbor <= 0 && mEastNeighbor != 0)
	{
	    // add the first 2 indexes of the first triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[4];

	    // iterate through the vertices on the neighbor edge to find which ones are enabled
	    for (int pos = cz + w2 - 2; pos > cz - w2; pos -= 2)
	    {
		// check if neighborvertex on west edge at pos is enabled
		if (mEastNeighbor->isWestEdgeVertexEnabled(pos))
		{
		    // add a vertex matching the vertex on the neighbor edge, close the open triangle and add the first 2 indexes of the next triangle
		    v[5] = addVertex(cx + w2, mHeight[pos * mData->terrain.heightMapWidth + (cx + w2)], pos);
		    *indexBuffer++ = v[5];
		    *indexBuffer++ = v[0];
		    *indexBuffer++ = v[5];
		}
	    }

	    // close the open triangle
	    *indexBuffer++ = v[2];
	}
	else
        {
	    // neighbor quad is disabled, just render 1 triangle
	    *indexBuffer++ = v[0];
	    *indexBuffer++ = v[4];
	    *indexBuffer++ = v[2];
	}

    }
    else // quad disabled, just render 2 tri's
    {
	// find how we should draw the diagonal
	int child = (node - 1) & 0x03;

	if (child == 0 || child == 3)
	{
	    *indexBuffer++ = v[2];
	    *indexBuffer++ = v[1];
	    *indexBuffer++ = v[4];

	    *indexBuffer++ = v[1];
	    *indexBuffer++ = v[3];
	    *indexBuffer++ = v[4];
	}
	else
	{
	    *indexBuffer++ = v[2];
	    *indexBuffer++ = v[1];
	    *indexBuffer++ = v[3];

	    *indexBuffer++ = v[3];
	    *indexBuffer++ = v[4];
	    *indexBuffer++ = v[2];
	}
    }

    // update index counter
    mIndexCount += (indexBuffer - (&mManager->mIndexBuffer[mIndexCount]));
}


//----------------------------------------------------------------------------

void NatureTerrainPatch::render(int cx, int cz, int node, int level)
{
    int width = EDGE_LENGTH >> level;

    // if not quad is enabled and not at lowest level, descend
    if (mQuadTree[node].isEnabled() && level < (QUADTREE_DEPTH - 1))
    {
	int w4 = (width >> 2);

	render(cx - w4, cz - w4, (node<<2)+1, level + 1);
	render(cx + w4, cz - w4, (node<<2)+2, level + 1);
	render(cx - w4, cz + w4, (node<<2)+3, level + 1);
	render(cx + w4, cz + w4, (node<<2)+4, level + 1);
    } 
    else
    {
	// if quad at lowest level or disabled, render it
	renderQuad(cx, cz, node, width);
    }
}

//----------------------------------------------------------------------------

// TODO: move errorMatrix to manager?
void NatureTerrainPatch::computeError()
{
    unsigned char errorMatrix[QUADTREE_SIZE * QUADTREE_SIZE];
    int width = 2, w2 = 1;

    memset(errorMatrix, 0, QUADTREE_SIZE * QUADTREE_SIZE);

    // go through all nodes of all levels, bottom -> top
    while (width <= EDGE_LENGTH)
    {
	int zAdd = w2 * QUADTREE_SIZE;

	for (int z = w2; z <= (EDGE_LENGTH - w2); z += width)
	{
	    int cIdx = z * QUADTREE_SIZE + w2;

	    for (int x = w2; x <= (EDGE_LENGTH - w2); x += width, cIdx += width)
	    {
		// get the errorvalue for this node
		int err = calculateError(x, z, width), perr, pIdx;

		// get errorvalues from lower level nodes
		if (width > 2)
		{
		    perr = errorMatrix[cIdx - zAdd];
		    if (perr > err) err = perr;

		    perr = errorMatrix[cIdx + zAdd];
		    if (perr > err) err = perr;

		    perr = errorMatrix[cIdx - w2];
		    if (perr > err) err = perr;

		    perr = errorMatrix[cIdx + w2];
		    if (perr > err) err = perr;
		}

		// propagate the error to the corners
		pIdx = cIdx - zAdd - w2;
		if (err > errorMatrix[pIdx]) errorMatrix[pIdx] = err;

		pIdx = cIdx - zAdd + w2;
		if (err > errorMatrix[pIdx]) errorMatrix[pIdx] = err;

		pIdx = cIdx + zAdd - w2;
		if (err > errorMatrix[pIdx]) errorMatrix[pIdx] = err;

		pIdx = cIdx + zAdd + w2;
		if (err > errorMatrix[pIdx]) errorMatrix[pIdx] = err;

		// set the errorvalue for this node
		mQuadTree[mManager->mQuadNodeLookup[cIdx]].setError(err);
	    }
	}

	w2    = width;
	width = width << 1;
    }
}

//----------------------------------------------------------------------------

void NatureTerrainPatch::triangulate(int cx, int cz, int node, int level)
{
    int width = EDGE_LENGTH >> level;

    // calculate d2 * desired quality
    Real d2c = static_cast<Real>(mQuadTree[node].getError())
	     * mManager->mTargetQuality;
    if (d2c < 1.0f) d2c = 1.0f;

    // calculate subdivide value
    Real f = mDistance / (mManager->mMinimumQuality * d2c);

    if (f < 1.0)
    {
	mQuadTree[node].enable();

	// if not at lowest level, descend into children
	if (level < (QUADTREE_DEPTH - 1))
	{
	    int w4 = (width >> 2);

	    triangulate(cx - w4, cz - w4, (node<<2)+1, level + 1);
	    triangulate(cx + w4, cz - w4, (node<<2)+2, level + 1);
	    triangulate(cx - w4, cz + w4, (node<<2)+3, level + 1);
	    triangulate(cx + w4, cz + w4, (node<<2)+4, level + 1);
	}
    }

    // use f for geomorph?
//    return f;
}

//----------------------------------------------------------------------------
void NatureTerrainPatch::getNormal(int x, int z, Vector3 *normal)
{   
    Vector3 here, left, down;
                
    here.x = x;
    here.y = mHeight[z * mData->terrain.heightMapWidth + x]
        * mScale.y + mWorld.y;
    here.z = z;
    
    left.x = x - 1;
    if (x > 0)
        left.y = mHeight[z * mData->terrain.heightMapWidth + (x - 1)]
            * mScale.y + mWorld.y;
    else if (mWestNeighbor != 0)
        left.y = mWestNeighbor->getHeightAt(EDGE_LENGTH - 1, z);
    else
        left.y = here.y;

    left.z = z;

    down.x = x;
    if (z < EDGE_LENGTH)
        down.y = mHeight[(z + 1) * mData->terrain.heightMapWidth + x]
            * mScale.y + mWorld.y;
    else if (mSouthNeighbor != 0)
        down.y = mSouthNeighbor->getHeightAt(x, 1);
    else
        down.y = here.y;

    down.z = z + 1;

    left = left - here;
    down = down - here;

    left.normalise();
    down.normalise();

    *normal = left.crossProduct(down);
    normal->normalise();
}
//----------------------------------------------------------------------------

void NatureTerrainPatch::getRenderOperation(RenderOperation& op)
{
    op.useIndexes = true;
    op.operationType = RenderOperation::OT_TRIANGLE_LIST;

    op.vertexData = mVertexData;
    op.vertexData->vertexStart = 0;
    op.vertexData->vertexCount = mVertexCount;

    op.indexData = mIndexData;
    op.indexData->indexStart = 0;
    op.indexData->indexCount = mIndexCount;

    //std::cerr << "index count = " << mIndexCount << std::endl;
}

void NatureTerrainPatch::_notifyCurrentCamera(Camera *cam)
{
    // TODO: fix this...

    Vector3 diff = mWorld + Vector3(32, 0, 32) - cam->getPosition();
    Real dist = diff.length();

    if (dist < 300) dist = 300;
    else if (dist > 700) dist = 700;
    dist -= 200;

    if (fabs(dist - mDistance) > 100 || (dist == 300 && dist != mDistance))
    {
	mDistance = dist;
	mManager->addToRenderQueue(this);
    }

}

} // namespace Ogre

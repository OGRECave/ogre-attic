/*****************************************************************************

	File: NaturePatch.h
	Desc: 
	Date: 2003/02/06

	Author: Martin Persson

*****************************************************************************/

#include "NatureSceneManager.h"
#include <OgreMaterialManager.h>
#include <asm_math.h>

#include <iostream>

namespace Ogre
{
    /* Welcome to MACRO heaven! :) (blame vc++'s inlining...) */

#define GET_HEIGHT(x, z)					\
    static_cast<int>(mHeightMap[(z << PATCH_LOG2) + z + x])

// Returns the vertex for x,z, 0xffff if no record exists
#define GET_VERTEX(x, z)					\
    mVertexLookup[(z << PATCH_LOG2) + z + x]

// Allocates a new vertex for x,z
#define ALLOC_VERTEX(vbi, x, z)					\
    vbi = (mVertexCount++) * 3;					\
    mVertexLookup[(z << PATCH_LOG2) + z + x] = vbi;

// Stores a vertex in the vertexbuffer
#define STORE_VERTEX(vbi, xp, yp, zp)				\
    mVertexBuffer[vbi + 0] = (xp) * mScale.x + mWorld.x;	\
    mVertexBuffer[vbi + 1] = (yp) * mScale.y + mWorld.y;	\
    mVertexBuffer[vbi + 2] = (zp) * mScale.z + mWorld.z;

// Returns the y coord of a vertex
#define GET_VERTEX_YPOS(vbi)					\
    ((mVertexBuffer[vbi + 1] - mWorld.y) / mScale.y)

// Allocates a vertex and stores its xyz coords
#define STORE_VERTEX_NEW(xp, yp, zp)				\
{								\
    register int vbi = GET_VERTEX(xp, zp);			\
    if (vbi == 0xffff) {					\
	ALLOC_VERTEX(vbi, xp, zp);				\
    }								\
    STORE_VERTEX(vbi, xp, yp, zp);				\
}

// Stores a triangle in the indexbuffer
#define ADD_TRIANGLE(li, ri, ai)				\
    mIndexBuffer[mIndexCount++] = (li / 3);		    	\
    mIndexBuffer[mIndexCount++] = (ri / 3);		    	\
    mIndexBuffer[mIndexCount++] = (ai / 3);

// Stores texture-coordinates
#define STORE_TEXCOORD(vbi, x, y)				\
    mTexCoordBuffer[0][(vbi*2)/3 + 0] = 1.0 - (x/(Real)PATCH_SIZE); \
    mTexCoordBuffer[0][(vbi*2)/3 + 1] = (y/(Real)PATCH_SIZE);

#if STITCH_EDGES
// Stores a pair of edge vertices
#define STORE_EDGE_VERTICES(x1, z1, x2, z2, index1, index2)	\
{								\
    register int off1 = 0;					\
    register int off2 = 0;					\
    if (z1 == 0 && z2 == 0)			    /* NORTH */	\
    { off1 = x1; off2 = off1 + (x2 - x1); }	    		\
    else if (z1 == PATCH_SIZE && z2 == PATCH_SIZE)  /* SOUTH */	\
    { off1 = (PATCH_SIZE+1) + x1; off2 = off1 + (x2 - x1); }	\
    else if (x1 == 0 && x2 == 0)		    /* WEST */	\
    { off1 = ((PATCH_SIZE+1)*2) + z1; off2 = off1 + (z2 - z1);}	\
    else					    /* EAST */	\
    { off1 = ((PATCH_SIZE+1)*3) + z1; off2 = off1 + (z2 - z1);}	\
    mEdgeVertices[off1] = index1;				\
    mEdgeVertices[off2] = index2;				\
}
#endif

// Allocates a new BinTriNode and clears its childpointers
#define ALLOC_TRI(tri)						\
    tri = mBinTriBuffer++;					\
    tri->leftChild = tri->rightChild = 0;			\
    tri->byEdge = false;

unsigned int NaturePatch::mGenNameCount = 0;

//----------------------------------------------------------------------------

NaturePatch::NaturePatch()
{ 
    mVariance = 0;
    mCenter = Vector3::ZERO;

    mVertexCache      = 0;
    mIndexCache       = 0;
    mTexCoordCache[0] = 0;
    mTexCoordCache[1] = 0;

    mMeshQuality     = DEFAULT_MESH_QUALITY;
    mQualityModifier = QUALITY_MODIFIER;

    mNeedsRebuild = false;
    mNeedsStitching = 0;

    // Get default material
    mMaterial = reinterpret_cast<Material*>
	(MaterialManager::getSingleton().getByName("BaseWhite"));

    // Generate name for this NaturePatch instance
    mName = "NaturePatch" + toString(mGenNameCount++);
}

//----------------------------------------------------------------------------

NaturePatch::~NaturePatch()
{
    if (mVertexCache != 0)
	delete[] mVertexCache;
    
    if (mIndexCache != 0)
	delete[] mIndexCache;

    if (mTexCoordCache[0] != 0)
	delete[] mTexCoordCache[0];

//    if (mTexCoordCache[1] != 0)
//	delete[] mTexCoordCache[1];

    if (mVariance != 0)
	delete[] mVariance;
}

//----------------------------------------------------------------------------

bool NaturePatch::init(const Vector3& worldPos, const Vector3& scale,
		       unsigned char *heightMap)
{
    mWorld     = worldPos;
    mScale     = scale;
    mHeightMap = heightMap;

    // Allocate memory for the variance buffer
    if (mVariance != 0) delete[] mVariance;
    if ((mVariance = new unsigned char[VARBUF_SIZE]) == 0)
	return false;

    // Pre-calculate the variance buffer
    computeVariance(0, PATCH_SIZE, PATCH_SIZE, 0, 0, 0, 2, 2);
    computeVariance(PATCH_SIZE, 0, 0, PATCH_SIZE, PATCH_SIZE, PATCH_SIZE,3,2);

    // Calculate max height (min == 0)
    int maxHeight = 0;
    for (int y = 0; y <= PATCH_SIZE; y++)
    {
	for (int x = 0; x <= PATCH_SIZE; x++)
	{
	    register int h = GET_HEIGHT(x, y);
	    if (maxHeight < h) maxHeight = h;
	}
    }
 
    // Set the extents of the bounding box
    mBounds.setExtents(mWorld.x,
		       mWorld.y,
		       mWorld.z,
		       mWorld.x + (PATCH_SIZE * mScale.x),
		       mWorld.y + (maxHeight * mScale.y),
		       mWorld.z + (PATCH_SIZE * mScale.z));

    initMesh();

    mCenter = Vector3(mWorld.x + ((PATCH_SIZE/2) * mScale.x),
		      mWorld.y + ((maxHeight/2) * mScale.y),
		      mWorld.z + ((PATCH_SIZE/2) * mScale.z));

    return true;
}

//----------------------------------------------------------------------------

void NaturePatch::initMesh()
{
    mBaseVertex[0].x = 0;
    mBaseVertex[0].z = PATCH_SIZE;
    mBaseVertex[0].y = GET_HEIGHT(static_cast<int>(mBaseVertex[0].x),
			          static_cast<int>(mBaseVertex[0].z));

    mBaseVertex[1].x = PATCH_SIZE;
    mBaseVertex[1].z = 0;
    mBaseVertex[1].y = GET_HEIGHT(static_cast<int>(mBaseVertex[1].x),
			          static_cast<int>(mBaseVertex[1].z));

    mBaseVertex[2].x = 0;
    mBaseVertex[2].z = 0;
    mBaseVertex[2].y = GET_HEIGHT(static_cast<int>(mBaseVertex[2].x),
			          static_cast<int>(mBaseVertex[2].z));

    mBaseVertex[3].x = PATCH_SIZE;
    mBaseVertex[3].z = PATCH_SIZE;
    mBaseVertex[3].y = GET_HEIGHT(static_cast<int>(mBaseVertex[3].x),
			          static_cast<int>(mBaseVertex[3].z));

    resetMesh();
}

//----------------------------------------------------------------------------

void NaturePatch::resetMesh()
{
    // Setup initial neighborhood
    mBaseLeft.baseNeighbor  = &mBaseRight;
    mBaseRight.baseNeighbor = &mBaseLeft;

    // Clear the unlinked neighbors
    mBaseLeft.leftNeighbor  = mBaseLeft.rightNeighbor = 0;
    mBaseRight.leftNeighbor = mBaseRight.rightNeighbor = 0;

    // Kill the children!
    mBaseLeft.leftChild  = mBaseLeft.rightChild  = 0;
    mBaseRight.leftChild = mBaseRight.rightChild = 0;

    mBaseLeft.byEdge = mBaseRight.byEdge = true;
}

//----------------------------------------------------------------------------

int NaturePatch::computeVariance(int lx, int lz,    // left vertex coords
				 int rx, int rz,    // right vertex coords
				 int ax,  int az,   // apex vertex coords
				 int node, int lvl) // node index & level
{
    // Get the center of the hypotenusus
    int cx = (lx + rx) >> 1;
    int cz = (lz + rz) >> 1;

    // Calculate local variance (shift for slightly increased accuracy)
    int v = abs((GET_HEIGHT(cx, cz) << 1) -
		(GET_HEIGHT(lx, lz) + GET_HEIGHT(rx, rz)));

    // Calculate variance of children
    if (lvl < (BINTRI_DEPTH - 2))
    {
	int cv;

	// Calculate variance of left child
	cv = computeVariance(ax, az, lx, lz, cx, cz, (node<<1), lvl+1);
	if (cv > v) v = cv;

	// Calculate variance of right child
	cv = computeVariance(rx, rz, ax, az, cx, cz, (node<<1)+1, lvl+1);
	if (cv > v) v = cv;
    }

    // Assign and return the max variance
    mVariance[node] = (v >> 1);
    return v;
}

//----------------------------------------------------------------------------

void NaturePatch::tesselate(BinTriNode *tri,
			    int lx, int lz,	// left vertex coords
			    int rx, int rz,	// right vertex coords
			    int ax, int az,	// apex vertex coords
			    int node, int lvl)	// nodeindex & level
{
    int cx = (lx + rx) >> 1;
    int cz = (lz + rz) >> 1;

    if (tri->leftChild == 0)
    {
/*	if (tri->byEdge == true || (tri->baseNeighbor != 0 && tri->baseNeighbor->byEdge == true))
	{
	    if (lvl < (BINTRI_DEPTH - 4))
	    {
		split(tri);
	    }
	}
	else
*/	{
	    // We have the VARBUF_SIZE check because we only store variance info
	    // down to the second highest level of the bintree
	    if ((node >= VARBUF_SIZE && mMeshQuality >= 2.0) ||
		(node < VARBUF_SIZE && (mVariance[node] * mMeshQuality) >= 1.0))
	    {
		split(tri);
	    }
	}
    }

    // If the tri has been split descend into its children
    if (tri->leftChild && lvl < (BINTRI_DEPTH - 1))
    {
	tesselate(tri->leftChild, ax, az, lx, lz, cx, cz, node<<1, lvl+1);
	tesselate(tri->rightChild, rx, rz, ax, az, cx, cz, (node<<1)+1,lvl+1);
    }
}

//----------------------------------------------------------------------------

void NaturePatch::split(BinTriNode *tri)
{
    /* If base neighbor exists but isn't in diamond with tri,
       force it to split first */
    if (tri->baseNeighbor && tri->baseNeighbor->baseNeighbor != tri)
    {
	split(tri->baseNeighbor);
    }

    // Allocate memory for both childs
    ALLOC_TRI(tri->leftChild);
    ALLOC_TRI(tri->rightChild);

    // Setup neighbor pointers from the parent
    tri->leftChild->baseNeighbor   = tri->leftNeighbor;
    tri->leftChild->leftNeighbor   = tri->rightChild;
    tri->rightChild->rightNeighbor = tri->leftChild;

    // Make sure parents' left-neighbor points to the new child
    if (tri->leftNeighbor != 0)
    {
	if (tri->leftNeighbor->baseNeighbor == tri)
	    tri->leftNeighbor->baseNeighbor = tri->leftChild;
	else
	    tri->leftNeighbor->rightNeighbor = tri->leftChild;
    }
    else tri->leftChild->byEdge = true;

    // Make sure parents' right-neighbor points to the new child
    tri->rightChild->baseNeighbor = tri->rightNeighbor;
    if (tri->rightNeighbor != 0)
    {
	if (tri->rightNeighbor->baseNeighbor == tri)
	    tri->rightNeighbor->baseNeighbor = tri->rightChild;
	else
	    tri->rightNeighbor->leftNeighbor = tri->rightChild;
    }
    else tri->rightChild->byEdge = true;

    // Setup base neighbor pointers
    if (tri->baseNeighbor != 0)
    {
	if (tri->baseNeighbor->leftChild)
	{
	    tri->baseNeighbor->leftChild->rightNeighbor = tri->rightChild;
	    tri->baseNeighbor->rightChild->leftNeighbor = tri->leftChild;
	    tri->leftChild->rightNeighbor = tri->baseNeighbor->rightChild;
	    tri->rightChild->leftNeighbor = tri->baseNeighbor->leftChild;
	}
	else
	    // Split base neighbor in diamond with tri
	    split(tri->baseNeighbor);
    }
    else
    {
	// Tri is by the edge, just reset neighbors
	tri->leftChild->rightNeighbor = 0;
	tri->rightChild->leftNeighbor = 0;

	tri->leftChild->byEdge = tri->rightChild->byEdge = true;
    }
}

//----------------------------------------------------------------------------

int NaturePatch::render(BinTriNode *tri,
	 		int lx, int lz,		// left vertex coords
			int rx, int rz,		// right vertex coords
			int ax, int az,		// apex vertex coords
			int li, int ri, int ai, // indices into vertex buffer
			int lvl)		// bintri level
{
    int maxlvl = lvl;
    bool avgHeight = false;

    // If tri has any childs, store the center vertex and descend
    if (tri->leftChild)
    {
	// Calc center vertex and its index into vertex buffer
	int cx = (lx + rx) >> 1;
	int cz = (lz + rz) >> 1;

	int ci = GET_VERTEX(cx, cz);
	if (ci == 0xffff)
	{
	    ALLOC_VERTEX(ci, cx, cz);
	}
	else
	{
	    avgHeight = true;
	}

#ifdef LAME_GEOMORPH_TEST
	// Descend into children
	int clvl = render(tri->leftChild, ax, az, lx, lz, cx, cz, ai, li, ci, lvl+1);
	if (maxlvl < clvl) maxlvl = clvl;

	clvl = render(tri->rightChild, rx, rz, ax, az, cx, cz, ri, ai, ci, lvl+1);
	if (maxlvl < clvl) maxlvl = clvl;

	// Calculate the amount of morphing needed
	Real morph = 1.0f - ((Real)(maxlvl-lvl) / (Real)(BINTRI_DEPTH-lvl));

	Real realHeight = GET_HEIGHT(cx, cz);
	Real diffHeight = realHeight - (((GET_HEIGHT(lx,lz) + GET_HEIGHT(rx,rz))/2.0f));
	Real newHeight	= realHeight - (diffHeight * morph);

	if (!avgHeight)
	{
	    STORE_VERTEX(ci, cx, newHeight, cz);
	    STORE_TEXCOORD(ci, cx, cz);	
	}
	else
	{
//	    STORE_VERTEX(ci, cx, (newHeight + GET_VERTEX_YPOS(ci)) / 2.0f, cz);
	}
#else
	// Descend into children
	render(tri->leftChild, ax, az, lx, lz, cx, cz, ai, li, ci, lvl + 1);
	render(tri->rightChild, rx, rz, ax, az, cx, cz, ri, ai, ci, lvl + 1);

	if (!avgHeight)
	{
	    STORE_VERTEX(ci, cx, GET_HEIGHT(cx, cz), cz);
	    STORE_TEXCOORD(ci, cx, cz);	
	}
#endif
    } 
    else 
    {
	// Add triangle to the indexbuffer
	ADD_TRIANGLE(li, ri, ai);

#if STITCH_EDGES
	if (!tri->baseNeighbor)
	{
	    STORE_EDGE_VERTICES(lx, lz, rx, rz, li, ri);
	}
	if (!tri->leftNeighbor)
	{
	    STORE_EDGE_VERTICES(ax, az, lx, lz, ai, li);
	}
	if (!tri->rightNeighbor)
	{
	    STORE_EDGE_VERTICES(rx, rz, ax, az, ri, ai);
	}
#endif
    }

    return maxlvl;
}

//----------------------------------------------------------------------------

unsigned int NaturePatch::buildMesh(BinTriNode *binTriBuffer,
				    Real *vertexBuffer,
				    ushort *indexBuffer,
				    ushort *vertexLookup,
				    Real *texCoordBuffer[2])
{
    /* TODO

    // Setup buffer pointers
    mBinTriBuffer = binTriBuffer;
    mIndexBuffer  = indexBuffer;
    mVertexBuffer = vertexBuffer;
    mVertexLookup = vertexLookup;

    mTexCoordBuffer[0] = texCoordBuffer[0];
//    mTexCoordBuffer[1] = texCoordBuffer[1];

    // Reset and tesselate the mesh
    resetMesh();

    tesselate(&mBaseLeft, static_cast<int>(mBaseVertex[0].x),
			  static_cast<int>(mBaseVertex[0].z),
			  static_cast<int>(mBaseVertex[1].x),
			  static_cast<int>(mBaseVertex[1].z),
			  static_cast<int>(mBaseVertex[2].x), 
			  static_cast<int>(mBaseVertex[2].z),
	      2, 2);
    
    tesselate(&mBaseRight, static_cast<int>(mBaseVertex[1].x),
			   static_cast<int>(mBaseVertex[1].z),
			   static_cast<int>(mBaseVertex[0].x),
			   static_cast<int>(mBaseVertex[0].z),
			   static_cast<int>(mBaseVertex[3].x),
			   static_cast<int>(mBaseVertex[3].z),
	      3, 2);

#if STITCH_EDGES 
    memset(mEdgeVertices, 0xffff, (PATCH_SIZE+1) * 4 * sizeof(ushort));
#if DRAW_EDGE_SKIRTS
    mUsedSkirts[0] = mUsedSkirts[1] = mUsedSkirts[2] = mUsedSkirts[3] = 0;
#endif
#endif

    // Store base vertices
    memset(mVertexLookup, 0xffff, ((PATCH_SIZE+1)*(PATCH_SIZE+1))
				  * sizeof(ushort));
    mVertexCount = mIndexCount = 0;

    STORE_VERTEX_NEW(static_cast<int>(mBaseVertex[0].x),
		     static_cast<int>(mBaseVertex[0].y),
		     static_cast<int>(mBaseVertex[0].z));

    STORE_VERTEX_NEW(static_cast<int>(mBaseVertex[1].x),
		     static_cast<int>(mBaseVertex[1].y),
		     static_cast<int>(mBaseVertex[1].z));

    STORE_VERTEX_NEW(static_cast<int>(mBaseVertex[2].x),
		     static_cast<int>(mBaseVertex[2].y),
		     static_cast<int>(mBaseVertex[2].z));

    STORE_VERTEX_NEW(static_cast<int>(mBaseVertex[3].x),
		     static_cast<int>(mBaseVertex[3].y),
		     static_cast<int>(mBaseVertex[3].z));

    // Store base texture coordinates
    STORE_TEXCOORD(0, mBaseVertex[0].x, mBaseVertex[0].z);
    STORE_TEXCOORD(3, mBaseVertex[1].x, mBaseVertex[1].z);
    STORE_TEXCOORD(6, mBaseVertex[2].x, mBaseVertex[2].z);
    STORE_TEXCOORD(9, mBaseVertex[3].x, mBaseVertex[3].z);

    // Build the vertexbuffers
    render(&mBaseLeft, static_cast<int>(mBaseVertex[0].x),
		       static_cast<int>(mBaseVertex[0].z),
		       static_cast<int>(mBaseVertex[1].x),
		       static_cast<int>(mBaseVertex[1].z),
		       static_cast<int>(mBaseVertex[2].x),
		       static_cast<int>(mBaseVertex[2].z),
	   0, 3, 6, 2);
    
    render(&mBaseRight, static_cast<int>(mBaseVertex[1].x),
			static_cast<int>(mBaseVertex[1].z),
		        static_cast<int>(mBaseVertex[0].x),
			static_cast<int>(mBaseVertex[0].z),
		        static_cast<int>(mBaseVertex[3].x),
			static_cast<int>(mBaseVertex[3].z),
	   3, 0, 9, 2);

    // Create new buffers
    mVertexCacheSize = (mVertexCount * 3);
    mIndexCacheSize  = mIndexCount;

    if (mVertexCache != 0)	delete[] mVertexCache;
    if (mIndexCache != 0)	delete[] mIndexCache;
    if (mTexCoordCache[0] != 0) delete[] mTexCoordCache[0];
//    if (mTexCoordCache[1] != 0) delete[] mTexCoordCache[1];

    // Allocate buffers
#if STITCH_EDGES & DRAW_EDGE_SKIRTS
    mVertexCache      = new Real[mVertexCacheSize + (MAX_EDGE_SKIRTS*4)*6];
    mIndexCache       = new ushort[mIndexCacheSize + (MAX_EDGE_SKIRTS*4)*9];
    mTexCoordCache[0] = new Real[(mVertexCount + (MAX_EDGE_SKIRTS*4)*2) * 2];
//    mTexCoordCache[1] = new Real[(mVertexCount + (MAX_EDGE_SKIRTS*4)*2) * 2];
#else
    mVertexCache      = new Real[mVertexCacheSize];
    mIndexCache       = new ushort[mIndexCacheSize];	
    mTexCoordCache[0] = new Real[mVertexCount * 2];
//    mTexCoordCache[1] = new Real[mVertexCount * 2];
#endif

    // Copy data to the new buffers for the RenderOperation 
    memcpy(mVertexCache, vertexBuffer, mVertexCacheSize * sizeof(Real));
    memcpy(mIndexCache, indexBuffer, mIndexCacheSize * sizeof(ushort));
    memcpy(mTexCoordCache[0], texCoordBuffer[0], mVertexCount*sizeof(Real)*2);
//    memcpy(mTexCoordCache[1], texCoordBuffer[1], mVertexCount*sizeof(Real)*2);

#if DRAW_EDGE_SKIRTS
    mVertexBuffer      = mVertexCache;
    mIndexBuffer       = mIndexCache;
    mTexCoordBuffer[0] = mTexCoordCache[0];
//    mTexCoordBuffer[1] = mTexCoordCache[1];
#endif
#if STITCH_EDGES
    mNeedsStitching    = 0x0f;
#endif
//    mNeedsRebuild = false;

*/
    // Return number of triangles generated
    return (mIndexCount / 3);
}

//---------------------------------------------------------------------------


#if STITCH_EDGES 
#if DRAW_EDGE_SKIRTS

// Removes the skirts on an edge by removing its indexes from the indexcache
void NaturePatch::clearSkirtEdge(int direction)
{
    if (mUsedSkirts[direction] == 0)
    {
	return;
    }
    
    // Calculate offset to first free index before skirts were added
    int count = mUsedSkirts[0] + mUsedSkirts[1]
	      +	mUsedSkirts[2] + mUsedSkirts[3];
    
    int offset = mIndexCacheSize - (count * 9);

    // Clear the edge
    mUsedSkirts[direction] = 0;

    // Copy the indexes of the remaining skirts
    for (int i = 0; i < 4; i++)
    {
	if (mUsedSkirts[i] != 0)
	{
	    memcpy(&mIndexCache[offset], mSkirtIndexes[i],
		   mUsedSkirts[i] * 9 * sizeof(ushort));

	    offset += mUsedSkirts[i] * 9;
	}
    }

    // Update size of the indexcache
    mIndexCacheSize = offset;
}

#endif

void NaturePatch::stitch(int direction, NaturePatch *neighborPatch)
{
    int neighborDir, lo = 0, ln = 0, i, n;
    ushort *neighborEdge;
    ushort *ourEdge;

    if (!(mNeedsStitching & (1 << direction)))
	return;

    switch (direction)
    {
	case EDGE_NORTH: neighborDir = EDGE_SOUTH; break;
	case EDGE_SOUTH: neighborDir = EDGE_NORTH; break;
	case EDGE_WEST:  neighborDir = EDGE_EAST; break;
	case EDGE_EAST:  neighborDir = EDGE_WEST; break;
	default: return;
    }

/*    printf("STITCHING: %s direction %d with %s direction %d -- ",
	(const char *)mName, direction, (const char *)neighborPatch->getName(),
	neighborDir); */

    ourEdge = &mEdgeVertices[direction * (PATCH_SIZE+1)];
    neighborEdge = &neighborPatch->mEdgeVertices[neighborDir * (PATCH_SIZE+1)];

    // Reset the ypos of each edge vertex where vertex exists on both patches
    for (i = 0; i < PATCH_SIZE+1; i++)
    {
	if (ourEdge[i] != 0xffff && neighborEdge[i] != 0xffff)
	{
	    int x = static_cast<int>(((mVertexCache[ourEdge[i] + 0] - mWorld.x)
				     / mScale.x) + 0.5);
    	    int z = static_cast<int>(((mVertexCache[ourEdge[i] + 2] - mWorld.z) 
				     / mScale.z) + 0.5);

	    Real height = (GET_HEIGHT(x, z) * mScale.y) + mWorld.y;

	    mVertexCache[ourEdge[i] + 1] = height;
    	    neighborPatch->mVertexCache[neighborEdge[i] + 1] = height;
	} 
    }

#if DRAW_EDGE_SKIRTS
    // Clear the skirts on the neighbors edge
    neighborPatch->clearSkirtEdge(neighborDir);
    int idxOff = 0;
#endif

    // Find and fix cracks
    for (i = 0; i < PATCH_SIZE+1; i++)
    {
	if (ourEdge[i] != 0xffff)
	{
	    if (neighborEdge[i] == 0xffff)
	    {
		// our edge has a vertex thats not on the neighbors edge,
		// find the next neighbor vertex
		for (n = i+1; n < (PATCH_SIZE+1); n++)
		{
		    if (neighborEdge[n] != 0xffff)
			break;
		}

		// next vertex found, calculate new heigh of our vertex
		Real diffHeight = neighborPatch->mVertexCache[neighborEdge[n] + 1] - 
				  neighborPatch->mVertexCache[neighborEdge[ln] + 1];
		
		Real newHeight = neighborPatch->mVertexCache[neighborEdge[ln] + 1]
				 + diffHeight * (Real)((i-ln)/(Real)(n-ln));
		// update our vertex to the new height
		mVertexCache[ourEdge[i] + 1] = newHeight;
	    
#if DRAW_EDGE_SKIRTS
		if (mUsedSkirts[direction] < MAX_EDGE_SKIRTS)
		{
		    register int ni = (mVertexCount++) * 3;
		    mVertexCache[ni + 0] = mVertexCache[ourEdge[i] + 0];
		    mVertexCache[ni + 1] = mVertexCache[ourEdge[i] + 1] - 1.0;
		    mVertexCache[ni + 2] = mVertexCache[ourEdge[i] + 2];
		    
		    register int ni2 = (mVertexCount++) * 3;
		    mVertexCache[ni2 + 0] = neighborPatch->
			mVertexCache[neighborEdge[n] + 0];
		    mVertexCache[ni2 + 1] = neighborPatch->
			mVertexCache[neighborEdge[n] + 1];
		    mVertexCache[ni2 + 2] = neighborPatch->
			mVertexCache[neighborEdge[n] + 2];
		    
		    int tx = static_cast<int>((mVertexCache[ni + 0] - 
					      mWorld.x) / mScale.x);
		    int tz = static_cast<int>((mVertexCache[ni + 2] -
					      mWorld.z) / mScale.z);
		    STORE_TEXCOORD(ni, tx, tz);

		    tx = static_cast<int>((mVertexCache[ni2 + 0] - 
					  mWorld.x) / mScale.x);
		    tz = static_cast<int>((mVertexCache[ni2 + 2] -
					  mWorld.z) / mScale.z);
		    STORE_TEXCOORD(ni2, tx, tz);

		    if (direction == EDGE_NORTH || direction == EDGE_EAST)
		    {
			mSkirtIndexes[direction][idxOff++] = ourEdge[i] / 3;
			mSkirtIndexes[direction][idxOff++] = ni / 3;
			mSkirtIndexes[direction][idxOff++] = ourEdge[lo] / 3;
		    
			mSkirtIndexes[direction][idxOff++] = ni2 / 3;
			mSkirtIndexes[direction][idxOff++] = ni / 3;
			mSkirtIndexes[direction][idxOff++] = ourEdge[i] / 3;

			mSkirtIndexes[direction][idxOff++] = ourEdge[lo] / 3;
			mSkirtIndexes[direction][idxOff++] = ni / 3;
			mSkirtIndexes[direction][idxOff++] = ni2 / 3;
		    
			ADD_TRIANGLE(ourEdge[i], ni, ourEdge[lo]);
			ADD_TRIANGLE(ni2, ni, ourEdge[i]);
			ADD_TRIANGLE(ourEdge[lo], ni, ni2);
		    }
		    else
		    {
			mSkirtIndexes[direction][idxOff++] = ourEdge[lo] / 3;
			mSkirtIndexes[direction][idxOff++] = ni / 3;
			mSkirtIndexes[direction][idxOff++] = ourEdge[i] / 3;
		    
			mSkirtIndexes[direction][idxOff++] = ourEdge[i] / 3; 
			mSkirtIndexes[direction][idxOff++] = ni / 3;
			mSkirtIndexes[direction][idxOff++] = ni2 / 3;

			mSkirtIndexes[direction][idxOff++] = ni2 / 3;
			mSkirtIndexes[direction][idxOff++] = ni / 3;
			mSkirtIndexes[direction][idxOff++] = ourEdge[lo] / 3;
		    
			ADD_TRIANGLE(ourEdge[lo], ni, ourEdge[i]);
			ADD_TRIANGLE(ourEdge[i], ni, ni2);
			ADD_TRIANGLE(ni2, ni, ourEdge[lo]);
		    }
		    
		    mUsedSkirts[direction]++;
		}
#endif
	    }
	    lo = i;
	}

	if (neighborEdge[i] != 0xffff)
	{
	    if (ourEdge[i] == 0xffff)
	    {
		// the neighbor edge has a vertex thats not on our edge
		// find our next vertex
		for (n = i+1; n < (PATCH_SIZE+1); n++)
		{
		    if (ourEdge[n] != 0xffff)
			break;
		}

		// next vertex found, calculate new heigh of neighbors vertex
		Real diffHeight = mVertexCache[ourEdge[n] + 1] - 
				  mVertexCache[ourEdge[lo] + 1];
		
		Real newHeight = mVertexCache[ourEdge[lo] + 1] +
				 diffHeight * ((Real)(i-lo)/(Real)(n-lo));

		// update neighbor vertex to the new height
		neighborPatch->mVertexCache[neighborEdge[i] + 1] = newHeight;

#if DRAW_EDGE_SKIRTS
		if (mUsedSkirts[direction] < MAX_EDGE_SKIRTS)
		{
		    register int ni = (mVertexCount++) * 3;
		    mVertexCache[ni + 0] = neighborPatch->
			mVertexCache[neighborEdge[i] + 0];
		    mVertexCache[ni + 1] = neighborPatch->
			mVertexCache[neighborEdge[i] + 1] - 1.0;
		    mVertexCache[ni + 2] = neighborPatch->
			mVertexCache[neighborEdge[i] + 2];
		    
		    register int ni2 = (mVertexCount++) * 3;
		    mVertexCache[ni2 + 0] = mVertexCache[ni + 0];
		    mVertexCache[ni2 + 1] = mVertexCache[ni + 1] + 1.0;
		    mVertexCache[ni2 + 2] = mVertexCache[ni + 2];
		    
		    int tx = static_cast<int>((mVertexCache[ni + 0] - 
					      mWorld.x) / mScale.x);
		    int tz = static_cast<int>((mVertexCache[ni + 2] -
					      mWorld.z) / mScale.z);
		    STORE_TEXCOORD(ni, tx, tz);
		    STORE_TEXCOORD(ni2, tx, tz);

		    if (direction == EDGE_NORTH || direction == EDGE_EAST)
		    {
			mSkirtIndexes[direction][idxOff++] = ourEdge[lo] / 3;
			mSkirtIndexes[direction][idxOff++] = ni / 3;
			mSkirtIndexes[direction][idxOff++] = ni2 / 3;

			mSkirtIndexes[direction][idxOff++] = ni2 / 3;
			mSkirtIndexes[direction][idxOff++] = ni / 3;
			mSkirtIndexes[direction][idxOff++] = ourEdge[n] / 3;

			mSkirtIndexes[direction][idxOff++] = ourEdge[n] / 3;
			mSkirtIndexes[direction][idxOff++] = ni / 3;
			mSkirtIndexes[direction][idxOff++] = ourEdge[lo] / 3;
		    
			ADD_TRIANGLE(ourEdge[lo], ni, ni2);
			ADD_TRIANGLE(ni2, ni, ourEdge[n]);
			ADD_TRIANGLE(ourEdge[n], ni, ourEdge[lo]);
		    }
		    else
		    {
			mSkirtIndexes[direction][idxOff++] = ni2 / 3;
			mSkirtIndexes[direction][idxOff++] = ni / 3;
			mSkirtIndexes[direction][idxOff++] = ourEdge[lo] / 3;

			mSkirtIndexes[direction][idxOff++] = ourEdge[n] / 3;
			mSkirtIndexes[direction][idxOff++] = ni / 3;
			mSkirtIndexes[direction][idxOff++] = ni2 / 3;

			mSkirtIndexes[direction][idxOff++] = ourEdge[lo] / 3;
			mSkirtIndexes[direction][idxOff++] = ni / 3;
			mSkirtIndexes[direction][idxOff++] = ourEdge[n] / 3;
		    
			ADD_TRIANGLE(ni2, ni, ourEdge[lo]);
			ADD_TRIANGLE(ourEdge[n], ni, ni2);
			ADD_TRIANGLE(ourEdge[lo], ni, ourEdge[n]);
		    }

		    mUsedSkirts[direction]++;
		}
#endif
	    }
	    ln = i;
	}
    }

#if DRAW_EDGE_SKIRTS
    mVertexCacheSize = (mVertexCount * 3);
    mIndexCacheSize  = mIndexCount;
#endif

    // Update status flags to make sure that edges only gets stitched once
//    neighborPatch->mNeedsStitching &= ~(1 << neighborDir);
    mNeedsStitching &= ~(1 << direction);

/*    std::cout <<  mUsedSkirts[direction] << " skirts added" << std::endl; */
}

#endif

//----------------------------------------------------------------------------

void NaturePatch::getRenderOperation(RenderOperation& op)
{
    /* TODO
    rend.useIndexes = true;

    rend.vertexOptions = LegacyRenderOperation::VO_TEXTURE_COORDS;
    //rend.vertexOptions |= LegacyRenderOperation::VO_DIFFUSE_COLOURS;
    rend.operationType = LegacyRenderOperation::OT_TRIANGLE_LIST;

    // use colors
    rend.pDiffuseColour = 0;
    rend.diffuseStride  = 0;

    // textures
    rend.numTextureCoordSets = 1;
    rend.numTextureDimensions[0] = 2;
//    rend.numTextureDimensions[1] = 2;
    rend.pTexCoords[0] = mTexCoordCache[0];
//    rend.pTexCoords[1] = mTexCoordCache[1];

    rend.texCoordStride[0] = 0;
//    rend.texCoordStride[1] = 0;

    // vertices & indexes
    rend.vertexStride = 0;
    rend.pVertices    = mVertexCache;
    rend.pIndexes     = mIndexCache;
    rend.numVertices  = mVertexCacheSize/3;
    rend.numIndexes   = mIndexCacheSize;

    // lighting
    rend.pNormals = 0;

    */
    return RenderOperation();
}

//----------------------------------------------------------------------------

void NaturePatch::_notifyCurrentCamera(Camera *cam)
{
    Real quality = 1.0;
    Vector3 diff = mCenter - cam->getPosition();
    Real dist = diff.length();

    if (dist > 400) quality = 0.30;
    else if (dist > 300) quality = 0.60;
    else quality = 1.0;

/*    if (dist > 400) quality = 0.15;
    else if (dist > 300) quality = 0.30;
    else if (dist > 200) quality = 0.60;
    else quality = 1.5;
*/
    quality = quality * mQualityModifier;

    if (mMeshQuality != quality)
    {
	mMeshQuality = quality;
	mNeedsRebuild = true;
    }
}

//----------------------------------------------------------------------------

uint NaturePatch::getMemoryUsage()
{
    unsigned int memUsage = 0;

    // vertexcache
    memUsage += ((mVertexCount*3) + ((MAX_EDGE_SKIRTS*4)*6)) * sizeof(Real);
    // indexcache
    memUsage += (mIndexCount + (MAX_EDGE_SKIRTS*4)*9) * sizeof(Real);
    // texturecoordcache
    memUsage += ((mVertexCount + (MAX_EDGE_SKIRTS*4)*2)*2) * sizeof(ushort);

    memUsage += VARBUF_SIZE * 2;

    return memUsage + sizeof(NaturePatch);
}

} // Ogre namespace


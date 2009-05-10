/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef __Ogre_TerrainQuadTreeNode_H__
#define __Ogre_TerrainQuadTreeNode_H__

#include "OgreTerrainPrerequisites.h"
#include "OgreCommon.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreMovableObject.h"



namespace Ogre
{
	class HardwareVertexBufferSharedPtr;
	
	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Terrain
	*  Some details on the terrain component
	*  @{
	*/


	/** A node in a quad tree used to store a patch of terrain.
	@remarks
		<b>Algorithm overview:</b>
	@par
		Our goal is to perform traditional chunked LOD with geomorphing. But, 
		instead of just dividing the terrain into tiles, we will divide them into
		a hierarchy of tiles, a quadtree, where any level of the quadtree can 
		be a rendered tile (to the exclusion of its children). The idea is to 
		collect together children into a larger batch with their siblings as LOD 
		decreases, to improve performance.
	@par
		The minBatchSize and maxBatchSize parameters on Terrain a key to 
		defining this behaviour. Both values are expressed in vertices down one axis.
		maxBatchSize determines the number of tiles on one side of the terrain,
		which is numTiles = (terrainSize-1) / (maxBatchSize-1). This in turn determines the depth
		of the quad tree, which is sqrt(numTiles). The minBatchSize determines
		the 'floor' of how low the number of vertices can go in a tile before it
		has to be grouped together with its siblings to drop any lower. We also do not group 
		a tile with its siblings unless all of them are at this minimum batch size, 
		rather than trying to group them when they all end up on the same 'middle' LOD;
		this is for several reasons; firstly, tiles hitting the same 'middle' LOD is
		less likely and more transient if they have different levels of 'roughness',
		and secondly since we're sharing a vertex / index pool between all tiles, 
		only grouping at the min level means that the number of combinations of 
		buffer sizes for any one tile is greatly simplified, making it easier to 
		pool data. To be more specific, any tile / quadtree node can only have
		log2(maxBatchSize-1) - log2(minBatchSize-1) + 1 LOD levels (and if you set them 
		to the same value, LOD can only change by going up/down the quadtree).
		The numbers of vertices / indices in each of these levels is constant for
		the same (relative) LOD index no matter where you are in the tree, therefore
		buffers can potentially be reused more easily.
	*/
	class _OgreTerrainExport TerrainQuadTreeNode : public TerrainAlloc
	{
	public:
		/** Constructor.
		@param terrain The ultimate parent terrain
		@param parent Optional parent node (in which case xoff, yoff are 0 and size must be entire terrain)
		@param xoff,off Offsets from the start of the terrain data in 2D
		@param size The size of the node in vertices at the highest LOD
		@param lod The base LOD level
		@param depth The depth that this node is at in the tree (or convenience)
		@param quadrant The index of the quadrant (0, 1, 2, 3)
		*/
		TerrainQuadTreeNode(Terrain* terrain, TerrainQuadTreeNode* parent, 
			uint16 xoff, uint16 yoff, uint16 size, uint16 lod, uint16 depth, uint16 quadrant);
		virtual ~TerrainQuadTreeNode();

		/// Get the horizontal offset into the main terrain data of this node
		uint16 getXOffset() const { return mOffsetX; }
		/// Get the vertical offset into the main terrain data of this node
		uint16 getYOffset() const { return mOffsetY; }
		/// Is this a leaf node (no children)
		bool isLeaf() const;
		/// Get the base LOD level this node starts at
		uint16 getBaseLod() const { return mBaseLod; }
		/// Get the number of LOD levels this node can represent itself (only > 1 for leaf nodes)
		uint16 getLodCount() const;
		/// Get child node
		TerrainQuadTreeNode* getChild(unsigned short child) const;
		/// Get parent node
		TerrainQuadTreeNode* getParent() const;

		/// Prepare node and children (perform CPU tasks, may be background thread)
		void prepare();
		/// Load node and children (perform GPU tasks, will be render thread)
		void load();
		/// Unload node and children (perform GPU tasks, will be render thread)
		void unload();
		/// Unprepare node and children (perform CPU tasks, may be background thread)
		void unprepare();

		struct _OgreTerrainExport LodLevel : public TerrainAlloc
		{
			/// Number of vertices rendered down one side (not including skirts)
			uint16 batchSize;
			/// index data referencing the main vertex data but in CPU buffers (built in background)
			IndexData* cpuIndexData;
			/// "Real" index data on the gpu
			IndexData* gpuIndexData;
			/// Maximum delta height between this and the next lower lod
			Real maxHeightDelta;

			LodLevel() : cpuIndexData(0), gpuIndexData(0) {}
		};
		typedef vector<LodLevel*>::type LodLevelList;

		/** Get the LodLevel information for a given lod.
		@param lod The lod level index relative to this classes own list; if you
			want to use a global lod level, subtract getBaseLod() first.
		*/
		const LodLevel* getLodLevel(uint16 lod);

		/** Notify the node (and children) that deltas are going to be calculated for a given range.
		@remarks
			Based on this call, we can know whether or not to reset the max height.
		*/
		void preDeltaCalculation(const Rect& rect);

		/** Notify the node (and children) of a height delta value. */
		void notifyDelta(uint16 x, uint16 y, uint16 lod, Real delta);

		/** Assign vertex data to the tree, from a depth and at a given resolution.
		@param treeDepthStart The first depth of tree that should use this data, owns the data
		@param treeDepthEnd The end of the depth that should use this data (exclusive)
		@param resolution The resolution of the data to use
		*/
		void assignVertexData(uint16 treeDepthStart, uint16 treeDepthEnd, uint16 resolution);

		/** Tell a node that it should use an anscestor's vertex data.
		@param treeDepthEnd The end of the depth that should use this data (exclusive)
		@param resolution The resolution of the data to use
		*/
		void useAncestorVertexData(TerrainQuadTreeNode* owner, uint16 treeDepthEnd, uint16 resolution);


		/** Merge a point (relative to terrain node) into the local bounds, 
			and that of children if applicable.
		@param x,y The point on the terrain to which this position corresponds 
			(affects which nodes update their bounds)
		@param pos The position relative to the terrain centre
		*/
		void mergeIntoBounds(long x, long y, const Vector3& pos);
		/** Reset the bounds of this node and all its children for the region given.
		@param rect The region for which bounds should be reset, in top-level terrain coords
		*/
		void resetBounds(const Rect& rect);
		
		/** Returns true if the given rectangle overlaps the terrain area that
			this node references.
		 @param rect The region in top-level terrain coords
		*/
		bool rectIntersectsNode(const Rect& rect);
		/** Returns true if the given point is in the terrain area that
		 this node references.
		 @param x,y The point in top-level terrain coords
		 */
		bool pointIntersectsNode(long x, long y);

		/// Get the AABB (local coords) of this node
		const AxisAlignedBox& getAABB() const;
		/// Get the bounding radius of this node
		Real getBoundingRadius() const;

	protected:
		Terrain* mTerrain;
		TerrainQuadTreeNode* mParent;
		TerrainQuadTreeNode* mChildren[4];
		LodLevelList mLodLevels;

		uint16 mOffsetX, mOffsetY;
		uint16 mBoundaryX, mBoundaryY;
		/// the number of vertices at the original terrain resolution this node encompasses
		uint16 mSize;
		uint16 mBaseLod;
		uint16 mDepth;
		uint16 mQuadrant;
		Vector3 mLocalCentre; // relative to terrain centre
		AxisAlignedBox mAABB; //relative to mLocalCentre
		Real mBoundingRadius; //relative to mLocalCentre

		struct VertexDataRecord : public TerrainAlloc
		{
			VertexData* cpuVertexData;
			VertexData* gpuVertexData;
			/// Base resolution of the data (size down one side, for the whole terrain)
			uint16 resolution;
			/// Number of quadtree levels (including this one) this data applies to
			uint16 treeLevels;
			/// Number of rows and columns of skirts
			uint16 numSkirtRowsCols;
			/// Is the GPU vertex data out of date?
			bool gpuVertexDataDirty;

			VertexDataRecord(uint16 res, uint16 lvls) 
				: cpuVertexData(0), gpuVertexData(0), resolution(res), 
				treeLevels(lvls), gpuVertexDataDirty(false) {}
		};
		
		TerrainQuadTreeNode* mNodeWithVertexData;
		VertexDataRecord* mVertexDataRecord;

		/** MovableObject implementation to provide the hook to the scene.
		@remarks
			In one sense, it would be most convenient to have a single MovableObject
			to represent the whole Terrain object, and then internally perform
			some quadtree frustum culling to narrow down which specific tiles are rendered.
			However, the one major flaw with that is that exposing the bounds to 
			the SceneManager at that level prevents it from doing anything smarter
			in terms of culling - for example a portal or occlusion culling SceneManager
			would have no opportunity to process the leaf nodes in those terms, and
			a simple frustum cull may give significantly poorer results. 
		@par
			Therefore, we in fact register a MovableObject at every node, and 
			use the LOD factor to determine which one is currently active. LODs
			must be mutually exclusive and to deal with precision errors, we really
			need to evaluate them all at once, rather than as part of the 
			_notifyCurrentCamera function. Therefore we register a GlobalRenderTargetListener
			to listen in on pre-renders of all viewports and to precalculate
			which nodes will be displayed when it comes to purely a LOD basis.
		*/
		class Movable : public MovableObject
		{
		protected:
			TerrainQuadTreeNode* mParent;
		public:
			Movable(TerrainQuadTreeNode* parent);
			~Movable();
			
	        // necessary overrides
			const String& getMovableType(void) const;
			const AxisAlignedBox& getBoundingBox(void) const;
			Real getBoundingRadius(void) const;
			void _updateRenderQueue(RenderQueue* queue);
			void visitRenderables(Renderable::Visitor* visitor,  bool debugRenderables = false);
			
			
		};
		Movable* mMovable;


		const VertexDataRecord* getVertexDataRecord() const;
		void createCpuVertexData();
		/* Update the vertex buffer - the rect in question is relative to the whole terrain, 
			not the local vertex data (which may use a subset)
		*/
		void updateVertexBuffer(HardwareVertexBufferSharedPtr& vbuf, const Rect& rect);
		void createCpuIndexData();
		void destroyCpuVertexData();
		void destroyCpuIndexData();

		void createGpuVertexData();
		void destroyGpuVertexData();
		void updateGpuVertexData();
		void createGpuIndexData();
		void destroyGpuIndexData();

		void createTriangleListBuffer(uint16 batchSize, IndexData* destData);
		void createTriangleStripBuffer(uint16 batchSize, IndexData* destData);
		
		uint16 calcSkirtVertexIndex(uint16 mainIndex, bool isCol);

	};





	/** @} */
	/** @} */
}




#endif 
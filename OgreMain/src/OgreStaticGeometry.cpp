/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2005 The OGRE Team
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
#include "OgreStableHeaders.h"
#include "OgreStaticGeometry.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"
#include "OgreSceneNode.h"
#include "OgreException.h"
#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreLogManager.h"

namespace Ogre {

	#define REGION_RANGE 1024
	#define REGION_HALF_RANGE 512
	#define REGION_MAX_INDEX 511
	#define REGION_MIN_INDEX -512

	//--------------------------------------------------------------------------
	StaticGeometry::StaticGeometry(SceneManager* owner, const String& name):
		mOwner(owner),
		mName(name),
		mBuilt(false),
		mUpperDistance(0.0f),
		mSquaredUpperDistance(0.0f),
		mCastShadows(false),
		mRegionDimensions(Vector3(1000,1000,1000)),
		mHalfRegionDimensions(Vector3(500,500,500)),
		mOrigin(Vector3(0,0,0)),
		mVisible(true)
	{
	}
	//--------------------------------------------------------------------------
	StaticGeometry::~StaticGeometry()
	{
		reset();
	}
	//--------------------------------------------------------------------------
	StaticGeometry::Region* StaticGeometry::getRegion(const AxisAlignedBox& bounds,
		bool autoCreate)
	{
		if (bounds.isNull())
			return 0;

		// Get the region which has the largest overlapping volume
		const Vector3 min = bounds.getMinimum();
		const Vector3 max = bounds.getMaximum();

		// Get the min and max region indexes
		ushort minx, miny, minz;
		ushort maxx, maxy, maxz;
		getRegionIndexes(min, minx, miny, minz);
		getRegionIndexes(max, maxx, maxy, maxz);
		Real maxVolume = 0.0f;
		ushort finalx, finaly, finalz;
		for (ushort x = minx; x < maxx; ++x)
		{
			for (ushort y = miny; y < maxy; ++y)
			{
				for (ushort z = minz; z < maxz; ++z)
				{
					Real vol = getVolumeIntersection(bounds, x, y, z);
					if (vol > maxVolume)
					{
						maxVolume = vol;
						finalx = x;
						finaly = y;
						finalz = z;
					}

				}
			}
		}

		assert(maxVolume > 0.0f && 
			"Static geometry: Problem determining closest volume match!");

		return getRegion(finalx, finaly, finalz, autoCreate);

	}
	//--------------------------------------------------------------------------
	Real StaticGeometry::getVolumeIntersection(const AxisAlignedBox& box,  
		ushort x, ushort y, ushort z)
	{
		// Get bounds of indexed region
		AxisAlignedBox regionBounds = getRegionBounds(x, y, z);
		AxisAlignedBox intersectBox = regionBounds.intersection(box);
		return intersectBox.volume();
	}
	//--------------------------------------------------------------------------
	AxisAlignedBox StaticGeometry::getRegionBounds(ushort x, ushort y, ushort z)
	{
		Vector3 min(
			((Real)x - REGION_HALF_RANGE) * mRegionDimensions.x + mOrigin.x, 
			((Real)y - REGION_HALF_RANGE) * mRegionDimensions.y + mOrigin.y, 
			((Real)z - REGION_HALF_RANGE) * mRegionDimensions.z + mOrigin.z
			);
		Vector3 max = min + mRegionDimensions;
		return AxisAlignedBox(min, max);
	}
	//--------------------------------------------------------------------------
 	Vector3 StaticGeometry::getRegionCentre(ushort x, ushort y, ushort z)
	{
		return Vector3(
			((Real)x - REGION_HALF_RANGE) * mRegionDimensions.x + mOrigin.x
				+ mHalfRegionDimensions.x, 
			((Real)y - REGION_HALF_RANGE) * mRegionDimensions.y + mOrigin.y
				+ mHalfRegionDimensions.y, 
			((Real)z - REGION_HALF_RANGE) * mRegionDimensions.z + mOrigin.z
				+ mHalfRegionDimensions.z
			);
	}
	//--------------------------------------------------------------------------
	StaticGeometry::Region* StaticGeometry::getRegion(
			ushort x, ushort y, ushort z, bool autoCreate)
	{
		uint32 index = packIndex(x, y, z);
		Region* ret = getRegion(index);
		if (!ret && autoCreate)
		{
			// Make a name
			StringUtil::StrStreamType str;
			str << mName << ":" << index;
			// Calculate the region centre
			Vector3 centre = getRegionCentre(x, y, z);
			ret = new Region(str.str(), index, centre);
			mRegionMap[index] = ret;
		}
		return ret;
	}
	//--------------------------------------------------------------------------
	StaticGeometry::Region* StaticGeometry::getRegion(uint32 index)
	{
		RegionMap::iterator i = mRegionMap.find(index);
		if (i != mRegionMap.end())
		{
			return i->second;
		}
		else 
		{
			return 0;
		}
		
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::getRegionIndexes(const Vector3& point, 
		ushort& x, ushort& y, ushort& z)
	{
		// Scale the point into multiples of region and adjust for origin
		Vector3 scaledPoint = (point - mOrigin) / mRegionDimensions;

		// Round down to 'bottom left' point which represents the cell index
		int ix = Math::IFloor(scaledPoint.x);
		int iy = Math::IFloor(scaledPoint.y);
		int iz = Math::IFloor(scaledPoint.z);

		// Check bounds
		if (ix < REGION_MIN_INDEX || ix > REGION_MAX_INDEX 
			|| iy < REGION_MIN_INDEX || iy > REGION_MAX_INDEX 
			|| iz < REGION_MIN_INDEX || iz > REGION_MAX_INDEX)
		{
			Except(Exception::ERR_INVALIDPARAMS,
				"Point out of bounds",
				"StaticGeometry::getRegionIndexes");
		}
		// Adjust for the fact that we use unsigned values for simplicity
		// (requires less faffing about for negatives give 10-bit packing
		x = static_cast<ushort>(ix + REGION_HALF_RANGE);
		y = static_cast<ushort>(iy + REGION_HALF_RANGE);
		z = static_cast<ushort>(iz + REGION_HALF_RANGE);


	}
	//--------------------------------------------------------------------------
	uint32 StaticGeometry::packIndex(ushort x, ushort y, ushort z)
	{
		return x + (y << 10) + (z << 20);
	}
	//--------------------------------------------------------------------------
	StaticGeometry::Region* StaticGeometry::getRegion(const Vector3& point, 
		bool autoCreate)
	{
		ushort x, y, z;
		getRegionIndexes(point, x, y, z);
		return getRegion(x, y, z, autoCreate);
	}
	//--------------------------------------------------------------------------
	AxisAlignedBox StaticGeometry::calculateBounds(VertexData* vertexData, 
		const Vector3& position, const Quaternion& orientation, 
		const Vector3& scale)
	{
		const VertexElement* posElem = 
			vertexData->vertexDeclaration->findElementBySemantic(
				VES_POSITION);
		HardwareVertexBufferSharedPtr vbuf = 
			vertexData->vertexBufferBinding->getBuffer(posElem->getSource());
		unsigned char* vertex = 
			static_cast<unsigned char*>(
				vbuf->lock(HardwareBuffer::HBL_READ_ONLY));
		Real* pReal;

		Vector3 min, max;
		bool first = true;

		for(size_t j = 0; j < vertexData->vertexCount; ++j, vertex += vbuf->getVertexSize())
		{
			posElem->baseVertexPointerToElement(vertex, &pReal);

			Vector3 pt;

			pt.x = (*pReal++);
			pt.y = (*pReal++);
			pt.z = (*pReal++);
			// Transform to world (scale, rotate, translate)
			pt = (orientation * (pt * scale)) + position;
			if (first)
			{
				min = max = pt;
				first = false;
			}
			else
			{
				min.makeFloor(pt);
				max.makeCeil(pt);
			}

		}
		vbuf->unlock();
		return AxisAlignedBox(min, max);
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::addEntity(Entity* ent, const Vector3& position,
		const Quaternion& orientation, const Vector3& scale)
	{
		MeshPtr& msh = ent->getMesh();
		// Validate
		if (msh->isLodManual())
		{
			LogManager::getSingleton().logMessage(
				"WARNING (StaticGeometry): Manual LOD is not supported. "
				"Using only highest LOD level for mesh " + msh->getName());
		}
				
		AxisAlignedBox sharedWorldBounds;
		// queue this entities submeshes and choice of material
		// also build the lists of geometry to be used for the source of lods
		for (uint i = 0; i < ent->getNumSubEntities(); ++i)
		{
			SubEntity* se = ent->getSubEntity(i);
			QueuedSubMesh* q = new QueuedSubMesh();

			// Get the geometry for this SubMesh
			q->geometryLodList = determineGeometry(q->submesh);
			q->submesh = se->getSubMesh();
			q->materialName = se->getMaterialName();
			q->orientation = orientation;
			q->position = position;
			q->scale = scale;
			// Determine the bounds based on the highest LOD
			q->worldBounds = calculateBounds(
				(*q->geometryLodList)[0].vertexData, 
					position, orientation, scale);
			
			mQueuedSubMeshes.push_back(q);
		}
	}
	//--------------------------------------------------------------------------
	StaticGeometry::SubMeshLodGeometryLinkList* 
	StaticGeometry::determineGeometry(SubMesh* sm)
	{
		// First, determine if we've already seen this submesh before
		SubMeshGeometryLookup::iterator i = 
			mSubMeshGeometryLookup.find(sm);
		if (i != mSubMeshGeometryLookup.end())
		{
			return i->second;
		}
		// Otherwise, we have to create a new one
		SubMeshLodGeometryLinkList* lodList = new SubMeshLodGeometryLinkList();
		ushort numLods = sm->parent->isLodManual() ? 1 : 
			sm->parent->getNumLodLevels();
		lodList->resize(numLods);
		for (ushort lod = 0; lod < numLods; ++lod)
		{
			SubMeshLodGeometryLink& geomLink = (*lodList)[lod];
			const Mesh::MeshLodUsage& lodUsage = sm->parent->getLodLevel(lod);
			// Can use the original mesh geometry?
			if (sm->useSharedVertices)
			{
				if (sm->parent->getNumSubMeshes() == 1)
				{
					// Ok, this is actually our own anyway
					geomLink.vertexData = sm->parent->sharedVertexData;
					geomLink.indexData = sm->indexData;
				}
				else
				{
					// We have to split it
					splitGeometry(sm->parent->sharedVertexData, 
						sm->indexData, &geomLink);
				}
			}
			else
			{
				if (lod == 0)
				{
					// Ok, we can use the existing geometry; should be in full 
					// use by just this SubMesh
					geomLink.vertexData = sm->vertexData;
					geomLink.indexData = sm->indexData;
				}
				else
				{
					// We have to split it
					splitGeometry(sm->vertexData, 
						sm->indexData, &geomLink);
				}
			}
		}

		return lodList;
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::splitGeometry(VertexData* vd, IndexData* id, 
			StaticGeometry::SubMeshLodGeometryLink* targetGeomLink)
	{
		// Firstly we need to scan to see how many vertices are being used 
		// and while we're at it, build the remap we can use later
		bool use32bitIndexes = 
			id->indexBuffer->getType() == HardwareIndexBuffer::IT_32BIT;
		uint16 *p16;
		uint32 *p32;
		IndexRemap indexRemap;
		if (use32bitIndexes)
		{
			p32 = static_cast<uint32*>(id->indexBuffer->lock(
				id->indexStart, id->indexCount, HardwareBuffer::HBL_READ_ONLY));
			buildIndexRemap(p32, id->indexCount, indexRemap);
			id->indexBuffer->unlock();
		}
		else
		{
			p16 = static_cast<uint16*>(id->indexBuffer->lock(
				id->indexStart, id->indexCount, HardwareBuffer::HBL_READ_ONLY));
			buildIndexRemap(p16, id->indexCount, indexRemap);
			id->indexBuffer->unlock();
		}
		if (indexRemap.size() == vd->vertexCount)
		{
			// ha, complete usage after all
			targetGeomLink->vertexData = vd;
			targetGeomLink->indexData = id;
			return;
		}
		
		
		// Create the new vertex data records
		targetGeomLink->vertexData = vd->clone(false);
		// Convenience
		VertexData* newvd = targetGeomLink->vertexData;
		IndexData* newid = targetGeomLink->indexData;
			
		unsigned short numvbufs = vd->vertexBufferBinding->getBufferCount();
		// Copy buffers from old to new
		for (unsigned short b = 0; b < numvbufs; ++b)
		{
			// Lock old buffer
			HardwareVertexBufferSharedPtr oldBuf = 
				vd->vertexBufferBinding->getBuffer(b);
			// Create new buffer
			HardwareVertexBufferSharedPtr newBuf = 
				HardwareBufferManager::getSingleton().createVertexBuffer(
					oldBuf->getVertexSize(),
					indexRemap.size(), 
					HardwareBuffer::HBU_STATIC);
			// rebind
			newvd->vertexBufferBinding->setBinding(b, newBuf);

			// Copy all the elements of the buffer across, by iterating over
			// the IndexRemap which describes how to move the old vertices 
			// to the new ones. By nature of the map the remap is in order of
			// indexes in the old buffer, but note that we're not guaranteed to
			// address every vertex (which is kinda why we're here)
			uchar* pSrcBase = static_cast<uchar*>(
				oldBuf->lock(HardwareBuffer::HBL_READ_ONLY));
			uchar* pDstBase = static_cast<uchar*>(
				newBuf->lock(HardwareBuffer::HBL_DISCARD));
			size_t vertexSize = oldBuf->getVertexSize();
			for (IndexRemap::iterator r = indexRemap.begin(); 
				r != indexRemap.end(); ++r)
			{
				uchar* pSrc = pSrcBase + r->first * vertexSize; 
				uchar* pDst = pDstBase + r->second * vertexSize; 
				memcpy(pDst, pSrc, vertexSize);
			}
			// unlock
			oldBuf->unlock();
			newBuf->unlock();

		}

		// Now create a new index buffer
		HardwareIndexBufferSharedPtr ibuf = 
			HardwareBufferManager::getSingleton().createIndexBuffer(
				id->indexBuffer->getType(), id->indexCount, 
				HardwareBuffer::HBU_STATIC);
		
		if (use32bitIndexes)
		{
			uint32 *pSrc32, *pDst32;
			pSrc32 = static_cast<uint32*>(id->indexBuffer->lock(
				id->indexStart, id->indexCount, HardwareBuffer::HBL_READ_ONLY));
			pSrc32 = static_cast<uint32*>(ibuf->lock(
				HardwareBuffer::HBL_DISCARD));
			remapIndexes(pSrc32, pDst32, indexRemap, id->indexCount);
		}
		else
		{
			uint16 *pSrc16, *pDst16;
			pSrc16 = static_cast<uint16*>(id->indexBuffer->lock(
				id->indexStart, id->indexCount, HardwareBuffer::HBL_READ_ONLY));
			pSrc16 = static_cast<uint16*>(ibuf->lock(
				HardwareBuffer::HBL_DISCARD));
			remapIndexes(pSrc16, pDst16, indexRemap, id->indexCount);
		}
		
		targetGeomLink->indexData = new IndexData();
		targetGeomLink->indexData->indexStart = 0;
		targetGeomLink->indexData->indexCount = id->indexCount;
		targetGeomLink->indexData->indexBuffer = ibuf;
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::addSceneNode(const SceneNode* node)
	{
		SceneNode::ConstObjectIterator obji = node->getAttachedObjectIterator();
		while (obji.hasMoreElements())
		{
			MovableObject* mobj = obji.getNext();
			if (mobj->getMovableType() == "Entity")
			{
				addEntity(static_cast<Entity*>(mobj), 
					node->_getDerivedPosition(), 
					node->_getDerivedOrientation(),
					node->_getDerivedScale());
			}
		}
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::build(void)
	{
		// Make sure there's nothing from previous builds
		destroy();

		// Firstly allocate meshes to regions
		for (QueuedSubMeshList::iterator qi = mQueuedSubMeshes.begin();
			qi != mQueuedSubMeshes.end(); ++qi)
		{
			QueuedSubMesh* qsm = *qi;
			Region* region = getRegion(qsm->worldBounds, true);
			region->assign(qsm);
		}

		// Now tell each region to build itself
		for (RegionMap::iterator ri = mRegionMap.begin(); 
			ri != mRegionMap.end(); ++ri)
		{
			ri->second->build();
		}

	}
	//--------------------------------------------------------------------------
	void StaticGeometry::destroy(void)
	{
		// delete the regions
		for (RegionMap::iterator i = mRegionMap.begin(); 
			i != mRegionMap.end(); ++i)
		{
			delete i->second;
		}
		mRegionMap.clear();
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::reset(void)
	{
		destroy();
		for (QueuedSubMeshList::iterator i = mQueuedSubMeshes.begin();
			i != mQueuedSubMeshes.end(); ++i)
		{
			delete *i;
		}
		mQueuedSubMeshes.clear();

	}
	//--------------------------------------------------------------------------
	void StaticGeometry::setVisible(bool visible)
	{
		mVisible = visible;
		// tell any existing regions
		for (RegionMap::iterator ri = mRegionMap.begin(); 
			ri != mRegionMap.end(); ++ri)
		{
			ri->second->setVisible(visible);
		}
	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	StaticGeometry::Region::Region(const String& name, uint32 regionID, const Vector3& centre) 
		: mName(name), mRegionID(regionID), mCentre(centre)
	{
		// First LOD mandatory, and always from 0
		mLodSquaredDistances.push_back(0.0f);
	}
	//--------------------------------------------------------------------------
	StaticGeometry::Region::~Region()
	{
		// delete
		for (LODBucketList::iterator i = mLodBucketList.begin(); 
			i != mLodBucketList.end(); ++i)
		{
			delete *i;
		}
		mLodBucketList.clear();

		// no need to delete queued meshes, these are managed in StaticGeometry

	}
	//--------------------------------------------------------------------------
	void StaticGeometry::Region::assign(QueuedSubMesh* qmesh)
	{
		mQueuedSubMeshes.push_back(qmesh);
		// update lod distances
		ushort lodLevels = qmesh->submesh->parent->getNumLodLevels();
		assert(qmesh->geometryLodList->size() == lodLevels);
		
		while(mLodSquaredDistances.size() < lodLevels)
		{
			mLodSquaredDistances.push_back(0.0f);
		}
		// Make sure LOD levels are max of all at the requested level
		for (ushort lod = 1; lod < lodLevels; ++lod)
		{
			const Mesh::MeshLodUsage& meshLod = 
				qmesh->submesh->parent->getLodLevel(lod);
			mLodSquaredDistances[lod] = std::max(mLodSquaredDistances[lod], 
				meshLod.fromDepthSquared);
		}

		// update bounds
		mWorldAABB.merge(qmesh->worldBounds);
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::Region::build(void)
	{
		// We need to create enough LOD buckets to deal with the highest LOD
		// we encountered in all the meshes queued
		// We also need to copy meshes with less lods than the max to the lower
		// lods
		for (ushort lod = 0; lod < mLodSquaredDistances.size(); ++lod)
		{
			LODBucket* lodBucket = 
				new LODBucket(this, lod, mLodSquaredDistances[lod]);
			mLodBucketList.push_back(lodBucket);
		
		}
		
	}
	//--------------------------------------------------------------------------
	const String& StaticGeometry::Region::getName(void) const
	{
	}
	//--------------------------------------------------------------------------
	const String& StaticGeometry::Region::getMovableType(void) const
	{
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::Region::_notifyCurrentCamera(Camera* cam)
	{
	}
	//--------------------------------------------------------------------------
	const AxisAlignedBox& StaticGeometry::Region::getBoundingBox(void) const
	{
	}
	//--------------------------------------------------------------------------
	Real StaticGeometry::Region::getBoundingRadius(void) const
	{
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::Region::_updateRenderQueue(RenderQueue* queue)
	{
	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	StaticGeometry::LODBucket::LODBucket(Region* parent, unsigned short lod,
		Real lodDist)
		: mParent(parent), mLod(lod), mSquaredDistance(lodDist)
	{
	}
	//--------------------------------------------------------------------------
	StaticGeometry::LODBucket::~LODBucket()
	{
		// delete
		for (MaterialBucketMap::iterator i = mMaterialBucketMap.begin(); 
			i != mMaterialBucketMap.end(); ++i)
		{
			delete i->second;
		}
		mMaterialBucketMap.clear();

		// no need to delete queued meshes, these are managed in StaticGeometry
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::LODBucket::assign(QueuedSubMesh* qmesh)
	{
		mQueuedSubMeshes.push_back(qmesh);
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::LODBucket::build(void)
	{
	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	StaticGeometry::MaterialBucket::MaterialBucket(LODBucket* parent, 
		const String& materialName)
		: mParent(parent), mMaterialName(materialName)
	{
	}
	//--------------------------------------------------------------------------
	StaticGeometry::MaterialBucket::~MaterialBucket()
	{
		// delete
		for (GeometryBucketList::iterator i = mGeometryBucketList.begin(); 
			i != mGeometryBucketList.end(); ++i)
		{
			delete *i;
		}
		mGeometryBucketList.clear();

		// no need to delete queued meshes, these are managed in StaticGeometry
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::MaterialBucket::assign(QueuedSubMesh* qmesh)
	{
		mQueuedSubMeshes.push_back(qmesh);
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::MaterialBucket::build(void)
	{
	}
	//--------------------------------------------------------------------------
	//--------------------------------------------------------------------------
	StaticGeometry::GeometryBucket::GeometryBucket(MaterialBucket* parent)
		: mParent(parent)
	{
	}
	//--------------------------------------------------------------------------
	StaticGeometry::GeometryBucket::~GeometryBucket()
	{
	}
	//--------------------------------------------------------------------------
	const MaterialPtr& StaticGeometry::GeometryBucket::getMaterial(void) const
	{
	}
	//--------------------------------------------------------------------------
	Technique* StaticGeometry::GeometryBucket::getTechnique(void) const
	{
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::GeometryBucket::getRenderOperation(RenderOperation& op)
	{
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::GeometryBucket::getWorldTransforms(Matrix4* xform) const
	{
	}
	//--------------------------------------------------------------------------
	const Quaternion& StaticGeometry::GeometryBucket::getWorldOrientation(void) const
	{
	}
	//--------------------------------------------------------------------------
	const Vector3& StaticGeometry::GeometryBucket::getWorldPosition(void) const
	{
	}
	//--------------------------------------------------------------------------
	Real StaticGeometry::GeometryBucket::getSquaredViewDepth(const Camera* cam) const
	{
	}
	//--------------------------------------------------------------------------
	const LightList& StaticGeometry::GeometryBucket::getLights(void) const
	{
	}
	//--------------------------------------------------------------------------
	bool StaticGeometry::GeometryBucket::getCastsShadows(void) const
	{
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::GeometryBucket::assign(QueuedSubMesh* qmesh)
	{
		mQueuedSubMeshes.push_back(qmesh);
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::GeometryBucket::build(void)
	{
	}
	//--------------------------------------------------------------------------

}


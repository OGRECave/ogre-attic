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
		uint32 maxVolumeIndex = 0;
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
						maxVolumeIndex = packIndex(x, y, z);
					}

				}
			}
		}

		assert(maxVolumeIndex && 
			"Static geometry: Problem determining closest volume match!");

		return getRegion(maxVolumeIndex, autoCreate);
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
	StaticGeometry::Region* StaticGeometry::getRegion(ushort x, ushort y, ushort z, bool autoCreate)
	{
		return getRegion(packIndex(x, y, z), autoCreate);
	}
	//--------------------------------------------------------------------------
	StaticGeometry::Region* StaticGeometry::getRegion(uint32 index, bool autoCreate)
	{
		Region* ret = 0;
		RegionMap::iterator i = mRegionMap.find(index);
		if (i != mRegionMap.end())
		{
			ret = i->second;
		}
		else if (autoCreate)
		{
			// Make a name
			StringUtil::StrStreamType str;
			str << mName << ":" << index;
			ret = new Region(str.str(), index);
			mRegionMap[index] = ret;
		}
		return ret;

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
		AxisAlignedBox sharedWorldBounds;
		if (msh->sharedVertexData)
		{
			sharedWorldBounds = calculateBounds(msh->sharedVertexData, 
				position, orientation, scale);
		}
		// queue this entities submeshes and choice of material
		for (uint i = 0; i < ent->getNumSubEntities(); ++i)
		{
			SubEntity* se = ent->getSubEntity(i);
			QueuedSubMesh* q = new QueuedSubMesh();
			q->submesh = se->getSubMesh();
			q->materialName = se->getMaterialName();
			q->orientation = orientation;
			q->position = position;
			q->scale = scale;
			if (q->submesh->useSharedVertices)
				q->worldBounds = sharedWorldBounds;
			else
				q->worldBounds = calculateBounds(q->submesh->vertexData, 
					position, orientation, scale);
			
			mQueuedSubMeshes.push_back(q);
		}
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
	StaticGeometry::Region::Region(const String& name, uint32 regionID) 
		: mName(name), mRegionID(regionID)
	{
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
	}
	//--------------------------------------------------------------------------
	void StaticGeometry::Region::build(void)
	{
		// scan & determine how many mesh LODs we have (== max LOD of any SubMesh)
		ushort maxLods = 1;
		for (QueuedSubMeshList::iterator qi = mQueuedSubMeshes.begin();
			qi != mQueuedSubMeshes.end(); ++qi)
		{
			QueuedSubMesh* qsm = *qi;
			maxLods = std::max(maxLods, qsm->submesh->parent->getNumLodLevels());
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
		: mParent(parent), mLod(lod), mLodDist(lodDist)
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


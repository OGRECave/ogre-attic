/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
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
#include "OgreMesh.h"

#include "OgreSubMesh.h"
#include "OgreMaterialManager.h"
#include "OgreLogManager.h"
#include "OgreDataChunk.h"
#include "OgreMeshSerializer.h"
#include "OgreSkeletonManager.h"
#include "OgreSkeleton.h"
#include "OgreHardwareBufferManager.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreMeshManager.h"
#include "OgreEdgeListBuilder.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    Mesh::Mesh(const String& name)
    {
        mName = name;
		sharedVertexData = NULL;

        // Default to load from file
        mManuallyDefined = false;
        //mUpdateBounds = true;
        setSkeletonName("");
        mBoneAssignmentsOutOfDate = false;
		mNumLods = 1;
		// Init first (manual) lod
		MeshLodUsage lod;
		lod.fromDepthSquared = 0.0f;
		mMeshLodUsageList.push_back(lod);
		mIsLodManual = false;

		mVertexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY;
		mIndexBufferUsage = HardwareBuffer::HBU_STATIC_WRITE_ONLY;
		mVertexBufferShadowBuffer = true;
		mIndexBufferShadowBuffer = true;

        mBoundRadius = 0.0f;

        mPreparedForShadowVolumes = false;

        mEdgeData = 0;

    }

    //-----------------------------------------------------------------------
    Mesh::~Mesh()
    {
        if (mIsLoaded)
        {
            unload();
        }
    }

    //-----------------------------------------------------------------------
    SubMesh* Mesh::createSubMesh()
    {
        SubMesh* sub = new SubMesh();
        sub->parent = this;

        mSubMeshList.push_back(sub);

        return sub;
    }
    //-----------------------------------------------------------------------
    SubMesh* Mesh::createSubMesh(const String& name)
	{
		SubMesh *sub = createSubMesh();
		nameSubMesh(name, (ushort)mSubMeshList.size()-1);
		return sub ;
	}
    //-----------------------------------------------------------------------
    unsigned short Mesh::getNumSubMeshes() const
    {
        return static_cast< unsigned short >( mSubMeshList.size() );
    }

    //---------------------------------------------------------------------
	void Mesh::nameSubMesh(const String& name, ushort index) 
	{
		mSubMeshNameMap[name] = index ;
	}
	
    //-----------------------------------------------------------------------
    SubMesh* Mesh::getSubMesh(const String& name) const
	{
		ushort index = _getSubMeshIndex(name);
		return getSubMesh(index);
	}
    //-----------------------------------------------------------------------
    SubMesh* Mesh::getSubMesh(unsigned short index) const
    {
        SubMeshList::const_iterator i = mSubMeshList.begin();
        return const_cast<SubMesh*>(i[index]);
    }
    //-----------------------------------------------------------------------
    void Mesh::load()
    {
        // Load from specified 'name'
        if (mIsLoaded)
        {
            unload();
        }

        if (!mManuallyDefined)
        {
            MeshSerializer serializer;
            LogManager::getSingleton().logMessage("Mesh: Loading " + mName + ".");

            DataChunk chunk;
            MeshManager::getSingleton()._findResourceData(mName, chunk);

            // Determine file type
            std::vector<String> extVec = mName.split(".");

            String& ext = extVec[extVec.size() - 1];
            ext.toLowerCase();

            if (ext == "mesh")
            {
                serializer.importMesh(chunk, this);
            }
            else
            {
                // Unsupported format
                chunk.clear();
                Except(999, "Unsupported object file format.",
                    "Mesh::load");
            }

            chunk.clear();
        }

        // Prepare for shadow volumes?
        if (MeshManager::getSingleton().getPrepareAllMeshesForShadowVolumes())
        {
            prepareForShadowVolume();
        }

		mIsLoaded = true;

        //_updateBounds();

    }

    //-----------------------------------------------------------------------
    void Mesh::unload()
    {
        // Teardown submeshes
        for (SubMeshList::iterator i = mSubMeshList.begin();
            i != mSubMeshList.end(); ++i)
        {
            delete *i;
        }
        if (sharedVertexData)
        {
            delete sharedVertexData;
            sharedVertexData = NULL;
        }

        OGRE_DELETE(mEdgeData);
		// Clear SubMesh lists
		mSubMeshList.clear();
		mSubMeshNameMap.clear();
        mIsLoaded = false;
    }

    //-----------------------------------------------------------------------
    void Mesh::setManuallyDefined(bool manual)
    {
        mManuallyDefined = manual;
    }

    //-----------------------------------------------------------------------
    Mesh* Mesh::clone(const String& newName)
    {
        // This is a bit like a copy constructor, but with the additional aspect of registering the clone with
        //  the MeshManager

        // New Mesh is assumed to be manually defined rather than loaded since you're cloning it for a reason
        Mesh* newMesh = MeshManager::getSingleton().createManual(newName);

        // Copy submeshes first
        std::vector<SubMesh*>::iterator subi;
        SubMesh* newSub;
        for (subi = mSubMeshList.begin(); subi != mSubMeshList.end(); ++subi)
        {
            newSub = newMesh->createSubMesh();
            newSub->mMaterialName = (*subi)->mMaterialName;
            newSub->mMatInitialised = (*subi)->mMatInitialised;
            newSub->parent = newMesh;
            newSub->useSharedVertices = (*subi)->useSharedVertices;

            if (!(*subi)->useSharedVertices)
            {
                // Copy unique vertex data
				newSub->vertexData = (*subi)->vertexData->clone();
            }

            // Copy index data
			newSub->indexData = (*subi)->indexData->clone();
            // Copy any bone assignments
            newSub->mBoneAssignments = (*subi)->mBoneAssignments;
            newSub->mBoneAssignmentsOutOfDate = (*subi)->mBoneAssignmentsOutOfDate;
            newSub->mMatInitialised = (*subi)->mMatInitialised;

        }

        // Copy shared geometry, if any
        if (sharedVertexData)
        {
            newMesh->sharedVertexData = sharedVertexData->clone();
        }

		// Copy submesh names
		newMesh->mSubMeshNameMap = mSubMeshNameMap ;
        // Copy any bone assignments
        newMesh->mBoneAssignments = mBoneAssignments;
        // Copy bounds
        newMesh->mAABB = mAABB;
        newMesh->mBoundRadius = mBoundRadius;
        // copy BoneAssignment information
        newMesh->mBoneAssignmentsOutOfDate = mBoneAssignmentsOutOfDate;

		newMesh->mIsLodManual = mIsLodManual;
		newMesh->mNumLods = mNumLods;
		newMesh->mMeshLodUsageList = mMeshLodUsageList;

		newMesh->mVertexBufferUsage = mVertexBufferUsage;
		newMesh->mIndexBufferUsage = mIndexBufferUsage;
		newMesh->mVertexBufferShadowBuffer = mVertexBufferShadowBuffer;
		newMesh->mIndexBufferShadowBuffer = mIndexBufferShadowBuffer;

        newMesh->mSkeletonName = mSkeletonName;
        newMesh->mSkeleton = mSkeleton;

        newMesh->load();
        newMesh->touch();

        return newMesh;

    }
    //-----------------------------------------------------------------------
    /*
    void Mesh::_updateBounds(void)
    {
        Vector3 min, max;
        bool first = true;
        bool useShared = false;

		Real maxSquaredLength = -1.0f;

        // Loop through SubMeshes, find extents
        SubMeshList::iterator i;
        for (i = mSubMeshList.begin(); i != mSubMeshList.end(); ++i)
        {
            if (!(*i)->useSharedVertices)
            {
				(*i)->vertexData->getBounds(&mAABB, &maxSquaredLength);
            }
        }

        // Check shared
        if (sharedVertexData)
        {
			sharedVertexData->getBounds(&mAABB, &maxSquaredLength);
        }

        // Pad out the AABB a little, helps with most bounds tests
		mAABB.setExtents(mAABB.getMinimum() - Vector3::UNIT_SCALE, 
			mAABB.getMaximum() + Vector3::UNIT_SCALE);
        // Pad out the sphere a little too
		mBoundRadius = Math::Sqrt(maxSquaredLength) * 1.25;
        mUpdateBounds = false;

    }
    */
    //-----------------------------------------------------------------------
    const AxisAlignedBox& Mesh::getBounds(void) const
    {
        /*
        if (mUpdateBounds)
            _updateBounds();
        */
        return mAABB;
    }
    //-----------------------------------------------------------------------
    void Mesh::_setBounds(const AxisAlignedBox& bounds)
    {
        mAABB = bounds;
        // Pad out the AABB a little, helps with most bounds tests
        mAABB.setExtents(mAABB.getMinimum() - Vector3::UNIT_SCALE,
        mAABB.getMaximum() + Vector3::UNIT_SCALE);

		// Set sphere bouds; not the tightest by since we're using
		// manual AABB it is the only way
		Real sqLen1 = mAABB.getMinimum().squaredLength();
		Real sqLen2 = mAABB.getMaximum().squaredLength();
		mBoundRadius = Math::Sqrt(std::max(sqLen1, sqLen2)); 
        // Pad out the sphere a little too
        mBoundRadius = mBoundRadius + 1;
		
        //mUpdateBounds = false;
    }
    //-----------------------------------------------------------------------
    void Mesh::_setBoundingSphereRadius(Real radius)
    {
        mBoundRadius = radius;
    }
    //-----------------------------------------------------------------------
    void Mesh::setSkeletonName(const String& skelName)
    {
        mSkeletonName = skelName;

        if (skelName == "")
        {
            // No skeleton
            mSkeleton = 0;
        }
        else
        {
            // Load skeleton
            try {
                mSkeleton = SkeletonManager::getSingleton().load(skelName);
            }
            catch (...)
            {
                mSkeleton = 0;
                // Log this error
                String msg = "Unable to load skeleton ";
                msg << skelName << " for Mesh " << mName
                    << " This Mesh will not be animated. ";
                LogManager::getSingleton().logMessage(msg);

            }


        }
    }
    //-----------------------------------------------------------------------
    bool Mesh::hasSkeleton(void) const
    {
        return !(mSkeletonName.empty());
    }
    //-----------------------------------------------------------------------
    Skeleton* Mesh::getSkeleton(void) const
    {
        return mSkeleton;
    }
    //-----------------------------------------------------------------------
    void Mesh::addBoneAssignment(const VertexBoneAssignment& vertBoneAssign)
    {
        mBoneAssignments.insert(
            VertexBoneAssignmentList::value_type(vertBoneAssign.vertexIndex, vertBoneAssign));
        mBoneAssignmentsOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    void Mesh::clearBoneAssignments(void)
    {
        mBoneAssignments.clear();
        mBoneAssignmentsOutOfDate = true;
    }
    //-----------------------------------------------------------------------
    void Mesh::_initAnimationState(AnimationStateSet* animSet)
    {
        // Delegate to Skeleton
        assert(mSkeleton && "Skeleton not present");
        mSkeleton->_initAnimationState(animSet);

        // Take the opportunity to update the compiled bone assignments
        if (mBoneAssignmentsOutOfDate)
            _compileBoneAssignments();

        SubMeshList::iterator i;
        for (i = mSubMeshList.begin(); i != mSubMeshList.end(); ++i)
        {
            if ((*i)->mBoneAssignmentsOutOfDate)
            {
                (*i)->_compileBoneAssignments();
            }
        }
    }
    //-----------------------------------------------------------------------
    typedef std::multimap<Real, Mesh::VertexBoneAssignmentList::iterator> WeightIteratorMap;
    unsigned short Mesh::_rationaliseBoneAssignments(size_t vertexCount, Mesh::VertexBoneAssignmentList& assignments)
    {
        // Iterate through, finding the largest # bones per vertex
        unsigned short maxBones = 0;
        unsigned short currBones;
        currBones = 0;
        VertexBoneAssignmentList::iterator i;

        for (size_t v = 0; v < vertexCount; ++v)
        {
            // Get number of entries for this vertex
            currBones = static_cast<unsigned short>(assignments.count(v));

            // Deal with max bones update 
            // (note this will record maxBones even if they exceed limit)
            if (maxBones < currBones)
                maxBones = currBones;
            // does the number of bone assignments exceed limit?
            if (currBones > OGRE_MAX_BLEND_WEIGHTS)
            {
                // To many bone assignments on this vertex
                // Find the start & end (end is in iterator terms ie exclusive)
                std::pair<VertexBoneAssignmentList::iterator, VertexBoneAssignmentList::iterator> range;
                // map to sort by weight
                WeightIteratorMap weightToAssignmentMap;
                range = assignments.equal_range(v);
                // Add all the assignments to map
                for (i = range.first; i != range.second; ++i)
                {
                    // insert value weight->iterator
                    weightToAssignmentMap.insert(
                        WeightIteratorMap::value_type(i->second.weight, i));
                }
                // Reverse iterate over weight map, remove lowest n
                unsigned short numToRemove = currBones - OGRE_MAX_BLEND_WEIGHTS;
                WeightIteratorMap::iterator remIt = weightToAssignmentMap.begin();

                while (numToRemove--)
                {
                    // Erase this one
                    assignments.erase(remIt->second);
                    ++remIt;
                }
            } // if (currBones > OGRE_MAX_BLEND_WEIGHTS)

            // Make sure the weights are normalised
            // Do this irrespective of whether we had to remove assignments or not
            //   since it gives us a guarantee that weights are normalised
            //  We assume this, so it's a good idea since some modellers may not
            std::pair<VertexBoneAssignmentList::iterator, VertexBoneAssignmentList::iterator> normalise_range = assignments.equal_range(v);
            Real totalWeight = 0;
            // Find total first
            for (i = normalise_range.first; i != normalise_range.second; ++i)
            {
                totalWeight += i->second.weight;
            }
            // Now normalise if total weight is outside tolerance
            if (!Math::RealEqual(totalWeight, 1.0f))
            {
                for (i = normalise_range.first; i != normalise_range.second; ++i)
                {
                    i->second.weight = i->second.weight / totalWeight;
                }
            }

        }

		if (maxBones > OGRE_MAX_BLEND_WEIGHTS)
		{
            // Warn that we've reduced bone assignments
            LogManager::getSingleton().logMessage("WARNING: the mesh '" + mName + "' "
                "includes vertices with more than " + 
                StringConverter::toString(OGRE_MAX_BLEND_WEIGHTS) + " bone assignments. "
                "The lowest weighted assignments beyond this limit have been removed, so "
                "your animation may look slightly different. To eliminate this, reduce "
                "the number of bone assignments per vertex on your mesh to " + 
                StringConverter::toString(OGRE_MAX_BLEND_WEIGHTS) + ".");
            // we've adjusted them down to the max
            maxBones = OGRE_MAX_BLEND_WEIGHTS;

        }

        return maxBones;
    }
    //-----------------------------------------------------------------------
    void  Mesh::_compileBoneAssignments(void)
    {
        unsigned short maxBones = 
            _rationaliseBoneAssignments(sharedVertexData->vertexCount, mBoneAssignments);

        if (maxBones == 0)
        {
            // No bone assignments
            return;
        }

        compileBoneAssignments(mBoneAssignments, maxBones, 
            sharedVertexData);
        mBoneAssignmentsOutOfDate = false;

    }
    //---------------------------------------------------------------------
    void Mesh::compileBoneAssignments(        
        const VertexBoneAssignmentList& boneAssignments,
        unsigned short numBlendWeightsPerVertex, 
        VertexData* targetVertexData)
    {
        // Create or reuse blend weight / indexes buffer
        // Indices are always a UBYTE4 no matter how many weights per vertex
        // Weights are more specific though since they are Reals
        VertexDeclaration* decl = targetVertexData->vertexDeclaration;
        VertexBufferBinding* bind = targetVertexData->vertexBufferBinding;
        unsigned short bindIndex;

        const VertexElement* testElem = 
            decl->findElementBySemantic(VES_BLEND_INDICES);
        if (testElem)
        {
            // Already have a buffer, unset it & delete elements
            bindIndex = testElem->getSource();
            // unset will cause deletion of buffer
            bind->unsetBinding(bindIndex);
            decl->removeElement(VES_BLEND_INDICES);
            decl->removeElement(VES_BLEND_WEIGHTS);
        }
        else
        {
            // Get new binding
            bindIndex = bind->getNextIndex();
        }

        HardwareVertexBufferSharedPtr vbuf = 
            HardwareBufferManager::getSingleton().createVertexBuffer(
                sizeof(unsigned char)*4 + sizeof(Real)*numBlendWeightsPerVertex,
                targetVertexData->vertexCount, 
                HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
                true // use shadow buffer
                );
        // bind new buffer
        bind->setBinding(bindIndex, vbuf);
        const VertexElement *pIdxElem, *pWeightElem;

        // add new vertex elements
        // Note, insert directly after position to abide by pre-Dx9 format restrictions
        if(decl->getElement(0)->getSemantic() == VES_POSITION)
        {
            const VertexElement& idxElem = 
                decl->insertElement(1, bindIndex, 0, VET_UBYTE4, VES_BLEND_INDICES);
            const VertexElement& wtElem = 
                decl->insertElement(2, bindIndex, sizeof(unsigned char)*4, 
                VertexElement::multiplyTypeCount(VET_FLOAT1, numBlendWeightsPerVertex),
                VES_BLEND_WEIGHTS);
            pIdxElem = &idxElem;
            pWeightElem = &wtElem;
        }
        else
        {
            // Position is not the first semantic, therefore this declaration is
            // not pre-Dx9 compatible anyway, so just tack it on the end
            const VertexElement& idxElem = 
                decl->addElement(bindIndex, 0, VET_UBYTE4, VES_BLEND_INDICES);
            const VertexElement& wtElem = 
                decl->addElement(bindIndex, sizeof(unsigned char)*4, 
                VertexElement::multiplyTypeCount(VET_FLOAT1, numBlendWeightsPerVertex),
                VES_BLEND_WEIGHTS);
            pIdxElem = &idxElem;
            pWeightElem = &wtElem;
        }

        // Assign data
        size_t v;
        VertexBoneAssignmentList::const_iterator i;
        i = boneAssignments.begin();
        unsigned char *pBase = static_cast<unsigned char*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD)); 
        // Iterate by vertex
        Real *pWeight;
        unsigned char *pIndex;
        for (v = 0; v < targetVertexData->vertexCount; ++v)
        {
            /// Convert to specific pointers
            pWeightElem->baseVertexPointerToElement(pBase, &pWeight);
            pIdxElem->baseVertexPointerToElement(pBase, &pIndex);
            for (unsigned short bone = 0; bone < numBlendWeightsPerVertex; ++bone)
            {
                // Do we still have data for this vertex?
                if (i->second.vertexIndex == v)
                {
                    // If so, write weight
                    *pWeight++ = i->second.weight;
                    *pIndex++ = i->second.boneIndex;
                    ++i;
                }
                else
                {
                    // Ran out of assignments for this vertex, use weight 0 to indicate empty
                    *pWeight++ = 0.0f;
                    *pIndex++ = 0;
                }
            }
            pBase += vbuf->getVertexSize();
        }

        vbuf->unlock();

    }
    //---------------------------------------------------------------------
    void Mesh::_notifySkeleton(Skeleton* pSkel)
    {
        mSkeleton = pSkel;
        mSkeletonName = pSkel->getName();
    }
    //---------------------------------------------------------------------
    Mesh::BoneAssignmentIterator Mesh::getBoneAssignmentIterator(void)
    {
        return BoneAssignmentIterator(mBoneAssignments.begin(),
            mBoneAssignments.end());
    }
    //---------------------------------------------------------------------
    const String& Mesh::getSkeletonName(void) const
    {
        return mSkeletonName;
    }
    //---------------------------------------------------------------------
    void Mesh::generateLodLevels(const LodDistanceList& lodDistances, 
        ProgressiveMesh::VertexReductionQuota reductionMethod, Real reductionValue)
    {
        mMeshLodUsageList.clear();
        mIsLodManual = false;

        char msg[128];
        sprintf(msg, "Generating %d lower LODs for mesh %s.",
            lodDistances.size(), mName.c_str());
        LogManager::getSingleton().logMessage(msg);

        SubMeshList::iterator isub, isubend;
        isubend = mSubMeshList.end();
        for (isub = mSubMeshList.begin(); isub != isubend; ++isub)
        {
            // Set up data for reduction
            VertexData* pVertexData = (*isub)->useSharedVertices ? sharedVertexData : (*isub)->vertexData;

            ProgressiveMesh pm(pVertexData, (*isub)->indexData);
            pm.build(
            static_cast<ushort>(lodDistances.size()), 
                &((*isub)->mLodFaceList), 
                reductionMethod, reductionValue);

        }

        // Iterate over the lods and record usage
        LodDistanceList::const_iterator idist, idistend;
        idistend = lodDistances.end();
        // Record first LOD (full detail)
        MeshLodUsage lod;
        lod.fromDepthSquared = 0.0f;
        mMeshLodUsageList.push_back(lod);

        for (idist = lodDistances.begin(); idist != idistend; ++idist)
        {
            // Record usage
            lod.fromDepthSquared = (*idist) * (*idist);
            mMeshLodUsageList.push_back(lod);

        }
        mNumLods = static_cast<ushort>(lodDistances.size() + 1);
    }
    //---------------------------------------------------------------------
    ushort Mesh::getNumLodLevels(void) const
    {
        return mNumLods;
    }
    //---------------------------------------------------------------------
    const Mesh::MeshLodUsage& Mesh::getLodLevel(ushort index) const
    {
        assert(index < mMeshLodUsageList.size());
        if (mIsLodManual && mMeshLodUsageList[index].manualMesh == NULL)
        {
            // Load the mesh now
            mMeshLodUsageList[index].manualMesh = 
                MeshManager::getSingleton().load(mMeshLodUsageList[index].manualName);
        }
        return mMeshLodUsageList[index];
    }
    //---------------------------------------------------------------------
	struct ManualLodSortLess : 
	public std::binary_function<const Mesh::MeshLodUsage&, const Mesh::MeshLodUsage&, bool>
	{
		bool operator() (const Mesh::MeshLodUsage& mesh1, const Mesh::MeshLodUsage& mesh2)
		{
			// sort ascending by depth
			return mesh1.fromDepthSquared < mesh2.fromDepthSquared;
		}
	};
	void Mesh::createManualLodLevel(Real fromDepth, const String& meshName)
	{
		mIsLodManual = true;
		MeshLodUsage lod;
		lod.fromDepthSquared = fromDepth * fromDepth;
		lod.manualName = meshName;
		lod.manualMesh = NULL;
		mMeshLodUsageList.push_back(lod);
		++mNumLods;

		std::sort(mMeshLodUsageList.begin(), mMeshLodUsageList.end(), ManualLodSortLess());
	}
    //---------------------------------------------------------------------
	void Mesh::updateManualLodLevel(ushort index, const String& meshName)
	{
		// Basic prerequisites
		assert(mIsLodManual && "Not using manual LODs!");
		assert(index != 0 && "Can't modify first lod level (full detail)");
		assert(index < mMeshLodUsageList.size() && "Index out of bounds");
		// get lod
		MeshLodUsage* lod = &(mMeshLodUsageList[index]);

		lod->manualName = meshName;
		lod->manualMesh = NULL;
	}
    //---------------------------------------------------------------------
	ushort Mesh::getLodIndex(Real depth) const
	{
		return getLodIndexSquaredDepth(depth * depth);
	}
    //---------------------------------------------------------------------
	ushort Mesh::getLodIndexSquaredDepth(Real squaredDepth) const
	{
		MeshLodUsageList::const_iterator i, iend;
		iend = mMeshLodUsageList.end();
		ushort index = 0;
		for (i = mMeshLodUsageList.begin(); i != iend; ++i, ++index)
		{
			if (i->fromDepthSquared > squaredDepth)
			{
				return index - 1;
			}
		}

		// If we fall all the way through, use the highest value
		return static_cast<ushort>(mMeshLodUsageList.size() - 1);


	}
    //---------------------------------------------------------------------
	void Mesh::_setLodInfo(unsigned short numLevels, bool isManual)
	{
		mNumLods = numLevels;
		mMeshLodUsageList.resize(numLevels);
		// Resize submesh face data lists too
		for (SubMeshList::iterator i = mSubMeshList.begin(); i != mSubMeshList.end(); ++i)
		{
			(*i)->mLodFaceList.resize(numLevels - 1);
		}
		mIsLodManual = isManual;
	}
    //---------------------------------------------------------------------
	void Mesh::_setLodUsage(unsigned short level, Mesh::MeshLodUsage& usage)
	{
		mMeshLodUsageList[level] = usage;
	}
    //---------------------------------------------------------------------
	void Mesh::_setSubMeshLodFaceList(unsigned short subIdx, unsigned short level, 
		IndexData* facedata)
	{
		SubMesh* sm = mSubMeshList[subIdx];
		sm->mLodFaceList[level - 1] = facedata;

	}
    //---------------------------------------------------------------------
	ushort Mesh::_getSubMeshIndex(const String& name) const
	{
		SubMeshNameMap::const_iterator i = mSubMeshNameMap.find(name) ;
		if (i == mSubMeshNameMap.end())
            Except(Exception::ERR_ITEM_NOT_FOUND, "No SubMesh named " + name + " found.", 
                "Mesh::_getSubMeshIndex");

		return i->second;
	}
    //---------------------------------------------------------------------
    void Mesh::removeLodLevels(void)
    {
        if (!mIsLodManual)
        {
            // Remove data from SubMeshes
            SubMeshList::iterator isub, isubend;
            isubend = mSubMeshList.end();
            for (isub = mSubMeshList.begin(); isub != isubend; ++isub)
            {
                (*isub)->removeLodLevels();
            }
        }

        mMeshLodUsageList.clear();

        // Reinitialise
        mNumLods = 1;
		// Init first (manual) lod
		MeshLodUsage lod;
		lod.fromDepthSquared = 0.0f;
		mMeshLodUsageList.push_back(lod);
		mIsLodManual = false;


    }
    //---------------------------------------------------------------------
    Real Mesh::getBoundingSphereRadius(void) const
    {
        return mBoundRadius;
    }
    //---------------------------------------------------------------------
	void Mesh::setVertexBufferPolicy(HardwareBuffer::Usage vbUsage, bool shadowBuffer)
	{
		mVertexBufferUsage = vbUsage;
		mVertexBufferShadowBuffer = shadowBuffer;
	}
    //---------------------------------------------------------------------
	void Mesh::setIndexBufferPolicy(HardwareBuffer::Usage vbUsage, bool shadowBuffer)
	{
		mIndexBufferUsage = vbUsage;
		mIndexBufferShadowBuffer = shadowBuffer;
	}
    //---------------------------------------------------------------------
    HardwareVertexBufferSharedPtr Mesh::getTangentsBuffer(VertexData *vertexData, 
        unsigned short texCoordSet)
    {
	    VertexDeclaration *vDecl = vertexData->vertexDeclaration ;
	    VertexBufferBinding *vBind = vertexData->vertexBufferBinding ;

	    const VertexElement *tex3D = vDecl->findElementBySemantic(VES_TEXTURE_COORDINATES, texCoordSet);
	    bool needsToBeCreated = false;
    	
	    if (!tex3D) 
        { // no tex coords with index 1
			    needsToBeCreated = true ;
	    } 
        else if (tex3D->getType() != VET_FLOAT3) 
        { // no 3d-coords tex buffer
		    vDecl->removeElement(VES_TEXTURE_COORDINATES, texCoordSet);
		    vBind->unsetBinding(tex3D->getSource());
		    needsToBeCreated = true ;
	    }
    	
	    HardwareVertexBufferSharedPtr tex3DBuf ;
	    if (needsToBeCreated) 
        {
		    tex3DBuf = HardwareBufferManager::getSingleton().createVertexBuffer(
			    3*sizeof(float), vertexData->vertexCount,
			    HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY, 
			    true );
		    int source = vBind->getNextIndex(); // find next available source 
		    vBind->setBinding(source, tex3DBuf);
		    vDecl->addElement(source, 0, VET_FLOAT3, VES_TEXTURE_COORDINATES, texCoordSet);
	    } 
        else 
        {
		    tex3DBuf = vBind->getBuffer(tex3D->getSource());
	    }
    	
	    return tex3DBuf;
    }
    //---------------------------------------------------------------------
    void Mesh::buildTangentVectors(unsigned short sourceTexCoordSet, 
        unsigned short destTexCoordSet)
    {

	    // our temp. buffers
	    unsigned short	vertInd[3];
	    Vector3         vertPos[3];
        Real            u[3], v[3];
	    // setup a new 3D texture coord-set buffer for every sub mesh
	    int nSubMesh = getNumSubMeshes();
	    for (int sm = 0; sm < nSubMesh; sm++)
	    {
		    // retrieve buffer pointers
		    unsigned short	*pVIndices;	// the face indices buffer, read only
		    Real			*p2DTC;		// pointer to 2D tex.coords, read only
		    Real			*p3DTC;		// pointer to 3D tex.coords, write/read (discard)
		    Real			*pVPos;		// vertex position buffer, read only

		    SubMesh *pSubMesh = getSubMesh(sm);

		    // retrieve buffer pointers
		    // first, indices
		    IndexData *indexData = pSubMesh->indexData;
		    HardwareIndexBufferSharedPtr buffIndex = indexData->indexBuffer ;
		    pVIndices = (unsigned short*) buffIndex->lock(HardwareBuffer::HBL_READ_ONLY); // ***LOCK***
		    // then, vertices
		    VertexData *usedVertexData ;
		    if (pSubMesh->useSharedVertices) {
			    usedVertexData = sharedVertexData;
		    } else {
			    usedVertexData = pSubMesh->vertexData;
		    }
		    VertexDeclaration *vDecl = usedVertexData->vertexDeclaration;
		    VertexBufferBinding *vBind = usedVertexData->vertexBufferBinding;


		    // get a new 3D tex.coord.buffer or an existing one
		    HardwareVertexBufferSharedPtr buff3DTC = getTangentsBuffer(usedVertexData, destTexCoordSet);
		    // clear it
		    p3DTC = (Real*) buff3DTC->lock(HardwareBuffer::HBL_DISCARD); // ***LOCK***
		    memset(p3DTC,0,buff3DTC->getSizeInBytes());
		    // find a 2D tex coord buffer
		    const VertexElement *elem2DTC = vDecl->findElementBySemantic(VES_TEXTURE_COORDINATES, sourceTexCoordSet);

            if (!elem2DTC || elem2DTC->getType() != VET_FLOAT2)
            {
                Except(Exception::ERR_INVALIDPARAMS, 
                    "SubMesh " + StringConverter::toString(sm) + " of Mesh " + mName + 
                    " has no 2D texture coordinates, therefore we cannot calculate tangents.", 
                    "Mesh::buildTangentVectors");
            }
		    HardwareVertexBufferSharedPtr buff2DTC = vBind->getBuffer(elem2DTC->getSource());
		    p2DTC = (Real*) buff2DTC->lock(HardwareBuffer::HBL_READ_ONLY); // ***LOCK***
		    // find a vertex coord buffer
		    const VertexElement *elemVPos = vDecl->findElementBySemantic(VES_POSITION);
		    HardwareVertexBufferSharedPtr buffVPos = vBind->getBuffer(elemVPos->getSource());
		    pVPos = (Real*) buffVPos->lock(HardwareBuffer::HBL_READ_ONLY); // ***LOCK***
    		
		    size_t numFaces = indexData->indexCount / 3 ;
    		
		    // loop through all faces to calculate the tangents and normals
		    size_t n;
		    for (n = 0; n < numFaces; ++n)
		    {
			    int i;
			    for (i = 0; i < 3; ++i)
			    {
				    // get indexes of vertices that form a polygon in the position buffer
				    vertInd[i] = *pVIndices++;
				    // get the vertices positions from the position buffer
				    vertPos[i].x = pVPos[3 * vertInd[i] + 0];
				    vertPos[i].y = pVPos[3 * vertInd[i] + 1];
				    vertPos[i].z = pVPos[3 * vertInd[i] + 2];
				    // get the vertices tex.coords from the 2D tex.coords buffer
				    u[i] = p2DTC[2 * vertInd[i] + 0];
				    v[i] = p2DTC[2 * vertInd[i] + 1];
			    }
			    // calculate the TSB
                Vector3 tangent = Math::calculateTangentSpaceVector(
                    vertPos[0], vertPos[1], vertPos[2], 
                    u[0], v[0], u[1], v[1], u[2], v[2]);
			    // write new tex.coords 
                // note we only write the tangent, not the binormal since we can calculate
                // the binormal in the vertex program
			    for (i = 0; i < 3; ++i)
			    {
				    // write values (they must be 0 and we must add them so we can average
                    // all the contributions from all the faces
				    p3DTC[3 * vertInd[i] + 0] += tangent.x;
				    p3DTC[3 * vertInd[i] + 1] += tangent.y;
				    p3DTC[3 * vertInd[i] + 2] += tangent.z;
			    }
		    }
		    // now loop through all vertices and normalize them
		    size_t numVerts = usedVertexData->vertexCount ;
		    for (n = 0; n < numVerts * 3; n += 3)
		    {
			    // read the vertex
			    Vector3 temp(p3DTC[n + 0], p3DTC[n + 1], p3DTC[n + 2]);
			    // normalize the vertex
			    temp.normalise();
			    // write it back
			    p3DTC[n + 0] = temp.x;
			    p3DTC[n + 1] = temp.y;
			    p3DTC[n + 2] = temp.z;
		    }
		    // unlock buffers
		    buffIndex->unlock();
		    buff3DTC->unlock();
		    buff2DTC->unlock();
		    buffVPos->unlock();
	    }
        
    }

    //---------------------------------------------------------------------
    void Mesh::buildEdgeList(void)
    {
        // Delete any existing edge information
        OGRE_DELETE(mEdgeData);

        EdgeListBuilder eb;
        size_t vertexSetCount = 0;

        if (sharedVertexData)
        {
            eb.addVertexData(sharedVertexData);
            vertexSetCount++;
        }

        // Prepare the builder using the submesh information
        SubMeshList::iterator i, iend;
        iend = mSubMeshList.end();
        for (i = mSubMeshList.begin(); i != iend; ++i)
        {
            SubMesh* s = *i;
            if (s->useSharedVertices)
            {
                // Use shared vertex data, index as set 0
                eb.addIndexData(s->indexData, 0, s->operationType);
            }
            else
            {
                // own vertex data, add it and reference it directly
                eb.addVertexData(s->vertexData);
                eb.addIndexData(s->indexData, vertexSetCount++, s->operationType);
            }
        }

        mEdgeData = eb.build();

#if OGRE_DEBUG_MODE
        // Override default log
        Log* log = LogManager::getSingleton().createLog(mName + "_prepshadow.log", false, false);
        mEdgeData->log(log);
#endif


    }
    //---------------------------------------------------------------------
    void Mesh::prepareForShadowVolume(void)
    {
        if (sharedVertexData)
        {
            sharedVertexData->prepareForShadowVolume();
        }
        SubMeshList::iterator i, iend;
        iend = mSubMeshList.end();
        for (i = mSubMeshList.begin(); i != iend; ++i)
        {
            SubMesh* s = *i;
            if (!s->useSharedVertices)
            {
                s->vertexData->prepareForShadowVolume();
            }
        }
        mPreparedForShadowVolumes = true;
    }
    //---------------------------------------------------------------------
    EdgeData* Mesh::getEdgeList(void)
    {
        if (!mEdgeData)
        {
            buildEdgeList();
        }
        return mEdgeData;
    }
    //---------------------------------------------------------------------
    void Mesh::softwareVertexBlend(const VertexData* sourceVertexData, 
        const VertexData* targetVertexData, const Matrix4* pMatrices, 
        bool blendNormals)
    {
        // Source vectors
        Vector3 sourceVec, sourceNorm;
        // Accumulation vectors
        Vector3 accumVecPos, accumVecNorm;

        Real *pSrcPos, *pSrcNorm, *pDestPos, *pDestNorm, *pBlendWeight;
        unsigned char* pBlendIdx;
        bool srcPosNormShareBuffer = false;
        bool destPosNormShareBuffer = false;
        bool weightsIndexesShareBuffer = false;


        // Get elements for source
        const VertexElement* srcElemPos = 
            sourceVertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        const VertexElement* srcElemNorm = 
            sourceVertexData->vertexDeclaration->findElementBySemantic(VES_NORMAL);
        const VertexElement* srcElemBlendIndices = 
            sourceVertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_INDICES);
        const VertexElement* srcElemBlendWeights = 
            sourceVertexData->vertexDeclaration->findElementBySemantic(VES_BLEND_WEIGHTS);
        assert (srcElemPos && srcElemBlendIndices && srcElemBlendWeights && 
            "You must supply at least positions, blend indices and blend weights");
        // Get elements for target
        const VertexElement* destElemPos = 
            targetVertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        const VertexElement* destElemNorm = 
            targetVertexData->vertexDeclaration->findElementBySemantic(VES_NORMAL);
        
        // Do we have normals and want to blend them?
        bool includeNormals = blendNormals && (srcElemNorm != NULL) && (destElemNorm != NULL);


        // Get buffers for source
        HardwareVertexBufferSharedPtr srcPosBuf, srcNormBuf, srcIdxBuf, srcWeightBuf;
        srcPosBuf = sourceVertexData->vertexBufferBinding->getBuffer(srcElemPos->getSource());
        srcIdxBuf = sourceVertexData->vertexBufferBinding->getBuffer(srcElemBlendIndices->getSource());
        srcWeightBuf = sourceVertexData->vertexBufferBinding->getBuffer(srcElemBlendWeights->getSource());
        if (includeNormals)
        {
            srcNormBuf = sourceVertexData->vertexBufferBinding->getBuffer(srcElemNorm->getSource());
            srcPosNormShareBuffer = (srcPosBuf.get() == srcNormBuf.get());
        }
        weightsIndexesShareBuffer = (srcIdxBuf.get() == srcWeightBuf.get());
        // Get buffers for target
        HardwareVertexBufferSharedPtr destPosBuf, destNormBuf;
        destPosBuf = targetVertexData->vertexBufferBinding->getBuffer(destElemPos->getSource());
        if (includeNormals)
        {
            destNormBuf = targetVertexData->vertexBufferBinding->getBuffer(destElemNorm->getSource());
            destPosNormShareBuffer = (destPosBuf.get() == destNormBuf.get());
        }

        // Lock source buffers for reading
        assert (srcElemPos->getOffset() == 0 && 
            "Positions must be first element in dedicated buffer!");
        pSrcPos = static_cast<Real*>(
            srcPosBuf->lock(HardwareBuffer::HBL_READ_ONLY));
        if (includeNormals)
        {
            if (srcPosNormShareBuffer)
            {
                // Same buffer, must be packed directly after position
                assert (srcElemNorm->getOffset() == sizeof(Real) * 3 && 
                    "Normals must be packed directly after positions in buffer!");
                // pSrcNorm will not be used
            }
            else
            {
                // Different buffer
                assert (srcElemNorm->getOffset() == 0 && 
                    "Normals must be first element in dedicated buffer!");
                pSrcNorm = static_cast<Real*>(
                    srcNormBuf->lock(HardwareBuffer::HBL_READ_ONLY));
            }
        }

        // Indices must be first in a buffer and be 4 bytes
        assert(srcElemBlendIndices->getOffset() == 0 &&
               srcElemBlendIndices->getType() == VET_UBYTE4 && 
               "Blend indices must be first in a buffer and be VET_UBYTE4");
        pBlendIdx = static_cast<unsigned char*>(
            srcIdxBuf->lock(HardwareBuffer::HBL_READ_ONLY));
        if (weightsIndexesShareBuffer)
        {
            // Weights must be packed directly after the indices
            assert(srcElemBlendWeights->getOffset() == sizeof(unsigned char)*4 &&
                "Blend weights must be directly after indices in the buffer");
            srcElemBlendWeights->baseVertexPointerToElement(pBlendIdx, &pBlendWeight);
        }
        else
        {
            // Weights must be at the start of the buffer
            assert(srcElemBlendWeights->getOffset() == 0 &&
                "Blend weights must be at the start of a dedicated buffer");
            // Lock buffer
            pBlendWeight = static_cast<Real*>(
                srcWeightBuf->lock(HardwareBuffer::HBL_READ_ONLY));
        }
        unsigned short numWeightsPerVertex = 
            VertexElement::getTypeCount(srcElemBlendWeights->getType());


        // Lock destination buffers for writing
        assert (destElemPos->getOffset() == 0 && 
            "Positions must be first element in dedicated buffer!");
        pDestPos = static_cast<Real*>(
            destPosBuf->lock(HardwareBuffer::HBL_DISCARD));
        if (includeNormals)
        {
            if (destPosNormShareBuffer)
            {
                // Same buffer, must be packed directly after position
                assert (destElemNorm->getOffset() == sizeof(Real) * 3 && 
                    "Normals must be packed directly after positions in buffer!");
                // pDestNorm will not be used
            }
            else
            {
                // Different buffer
                assert (destElemNorm->getOffset() == 0 && 
                    "Normals must be first element in dedicated buffer!");
                pDestNorm = static_cast<Real*>(
                    destNormBuf->lock(HardwareBuffer::HBL_DISCARD));
            }
        }

        // Loop per vertex
        for (size_t vertIdx = 0; vertIdx < targetVertexData->vertexCount; ++vertIdx)
        {
            // Load source vertex elements
            sourceVec.x = *pSrcPos++;
            sourceVec.y = *pSrcPos++;
            sourceVec.z = *pSrcPos++;

            if (includeNormals) 
            {
                if (srcPosNormShareBuffer)
                {
                    sourceNorm.x = *pSrcPos++;
                    sourceNorm.y = *pSrcPos++;
                    sourceNorm.z = *pSrcPos++;
                }
                else
                {
                    sourceNorm.x = *pSrcNorm++;
                    sourceNorm.y = *pSrcNorm++;
                    sourceNorm.z = *pSrcNorm++;
                }
            }
            // Load accumulators
            accumVecPos = Vector3::ZERO;
            accumVecNorm = Vector3::ZERO;

            // Loop per blend weight 
            for (unsigned short blendIdx = 0; 
                blendIdx < numWeightsPerVertex; ++blendIdx)
            {
                // Blend by multiplying source by blend matrix and scaling by weight
                // Add to accumulator
                // NB weights must be normalised!!
                if (*pBlendWeight != 0.0) 
                {
                    // Blend position, use 3x4 matrix
                    const Matrix4& mat = pMatrices[*pBlendIdx];
                    accumVecPos.x += 
                        (mat[0][0] * sourceVec.x + 
                         mat[0][1] * sourceVec.y + 
                         mat[0][2] * sourceVec.z + 
                         mat[0][3])
                         * (*pBlendWeight);
                    accumVecPos.y += 
                        (mat[1][0] * sourceVec.x + 
                         mat[1][1] * sourceVec.y + 
                         mat[1][2] * sourceVec.z + 
                         mat[1][3])
                         * (*pBlendWeight);
                    accumVecPos.z += 
                        (mat[2][0] * sourceVec.x + 
                         mat[2][1] * sourceVec.y + 
                         mat[2][2] * sourceVec.z + 
                         mat[2][3])
                         * (*pBlendWeight);
                    if (includeNormals)
                    {
                        // Blend normal
                        // We should blend by inverse transpose here, but because we're assuming the 3x3
                        // aspect of the matrix is orthogonal (no non-uniform scaling), the inverse transpose
                        // is equal to the main 3x3 matrix
                        // Note because it's a normal we just extract the rotational part, saves us renormalising here
                        accumVecNorm.x += 
                            (mat[0][0] * sourceNorm.x + 
                             mat[0][1] * sourceNorm.y + 
                             mat[0][2] * sourceNorm.z) 
                             * (*pBlendWeight);
                        accumVecNorm.y += 
                            (mat[1][0] * sourceNorm.x + 
                             mat[1][1] * sourceNorm.y + 
                             mat[1][2] * sourceNorm.z)
                            * (*pBlendWeight);
                        accumVecNorm.z += 
                            (mat[2][0] * sourceNorm.x + 
                             mat[2][1] * sourceNorm.y + 
                             mat[2][2] * sourceNorm.z)
                            * (*pBlendWeight);
                    }

                }
                ++pBlendWeight;
                ++pBlendIdx;
            }
            // Finish off blend info pointers
            // Make sure we skip over 4 index elements no matter how many we used
            pBlendIdx += 4 - numWeightsPerVertex;
            if(weightsIndexesShareBuffer)
            {
                // Skip index over weights
                pBlendIdx += sizeof(Real) * numWeightsPerVertex;
                // Re-base weights
                srcElemBlendWeights->baseVertexPointerToElement(pBlendIdx, &pBlendWeight);
            }


            // Stored blended vertex in hardware buffer
            *pDestPos++ = accumVecPos.x;
            *pDestPos++ = accumVecPos.y;
            *pDestPos++ = accumVecPos.z;

            // Stored blended vertex in temp buffer
            if (includeNormals)
            {
                // Normalise
                accumVecNorm.normalise();
                if (destPosNormShareBuffer)
                {
                    // Pack into same buffer
                    *pDestPos++ = accumVecNorm.x;
                    *pDestPos++ = accumVecNorm.y;
                    *pDestPos++ = accumVecNorm.z;
                }
                else
                {
                    *pDestNorm++ = accumVecNorm.x;
                    *pDestNorm++ = accumVecNorm.y;
                    *pDestNorm++ = accumVecNorm.z;
                }
            }
        }
        // Unlock source buffers
        srcPosBuf->unlock();
        srcIdxBuf->unlock();
        if (!weightsIndexesShareBuffer)
        {
            srcWeightBuf->unlock();
        }
        if (includeNormals && !srcPosNormShareBuffer)
        {
            srcNormBuf->unlock();
        }
        // Unlock destination buffers
        destPosBuf->unlock();
        if (includeNormals && !destPosNormShareBuffer)
        {
            destNormBuf->unlock();

        }



    }

}


/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#include "OgreMeshSerializer.h"
#include "OgreSkeletonManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreMeshManager.h"
#include "OgreEdgeListBuilder.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    MeshPtr::MeshPtr(const ResourcePtr& r) : SharedPtr<Mesh>()
    {
		// lock & copy other mutex pointer
		OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
		OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
        pRep = static_cast<Mesh*>(r.getPointer());
        pUseCount = r.useCountPointer();
        if (pUseCount)
        {
            ++(*pUseCount);
        }
    }
    //-----------------------------------------------------------------------
    MeshPtr& MeshPtr::operator=(const ResourcePtr& r)
    {
        if (pRep == static_cast<Mesh*>(r.getPointer()))
            return *this;
        release();
		// lock & copy other mutex pointer
		OGRE_LOCK_MUTEX(*r.OGRE_AUTO_MUTEX_NAME)
		OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
        pRep = static_cast<Mesh*>(r.getPointer());
        pUseCount = r.useCountPointer();
        if (pUseCount)
        {
            ++(*pUseCount);
        }
        return *this;
    }
    //-----------------------------------------------------------------------
    void MeshPtr::destroy(void)
    {
        // We're only overriding so that we can destroy after full definition of Mesh
        SharedPtr<Mesh>::destroy();
    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    Mesh::Mesh(ResourceManager* creator, const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : Resource(creator, name, handle, group, isManual, loader),
        mBoundRadius(0.0f), 
        mBoneAssignmentsOutOfDate(false),
        mIsLodManual(false), 
        mNumLods(1), 
        mVertexBufferUsage(HardwareBuffer::HBU_STATIC_WRITE_ONLY),
        mIndexBufferUsage(HardwareBuffer::HBU_STATIC_WRITE_ONLY),
        mVertexBufferShadowBuffer(true),
        mIndexBufferShadowBuffer(true),
        mPreparedForShadowVolumes(false),
        mEdgeListsBuilt(false),
        mAutoBuildEdgeLists(true), // will be set to false by serializers of 1.30 and above
        sharedVertexData(0)
    {

        setSkeletonName("");
		// Init first (manual) lod
		MeshLodUsage lod;
		lod.fromDepthSquared = 0.0f;
        lod.edgeData = NULL;
        lod.manualMesh.setNull();
		mMeshLodUsageList.push_back(lod);

    }
    //-----------------------------------------------------------------------
    Mesh::~Mesh()
    {
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload(); 
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
        if (index >= mSubMeshList.size())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                "Index out of bounds.",
                "Mesh::getSubMesh");
        }

        return mSubMeshList[index];
    }
	//-----------------------------------------------------------------------
	void Mesh::load(void)
	{
		OGRE_LOCK_AUTO_MUTEX

		// Overridden to ensure edge lists get built from manual or
		// loaded meshes
		Resource::load();

		// Prepare for shadow volumes?
		if (MeshManager::getSingleton().getPrepareAllMeshesForShadowVolumes())
		{
			if (mEdgeListsBuilt || mAutoBuildEdgeLists)
			{
				prepareForShadowVolume();
			}

			if (!mEdgeListsBuilt && mAutoBuildEdgeLists)
			{
				buildEdgeList();
			}
		}
	}
	//-----------------------------------------------------------------------
    void Mesh::loadImpl()
    {
        // Load from specified 'name'
        MeshSerializer serializer;
        LogManager::getSingleton().logMessage("Mesh: Loading " + mName + ".");

        DataStreamPtr stream = 
            ResourceGroupManager::getSingleton().openResource(mName, mGroup);
        serializer.importMesh(stream, this);

    }

    //-----------------------------------------------------------------------
    void Mesh::unloadImpl()
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
		// Clear SubMesh lists
		mSubMeshList.clear();
		mSubMeshNameMap.clear();
        // Removes all LOD data
        removeLodLevels();
        mPreparedForShadowVolumes = false;

        // Clear bone assignments
        mBoneAssignments.clear();
        mBoneAssignmentsOutOfDate = false;

        // Removes reference to skeleton
        setSkeletonName(StringUtil::BLANK);
    }

    //-----------------------------------------------------------------------
    MeshPtr Mesh::clone(const String& newName, const String& newGroup)
    {
        // This is a bit like a copy constructor, but with the additional aspect of registering the clone with
        //  the MeshManager

        // New Mesh is assumed to be manually defined rather than loaded since you're cloning it for a reason
        String theGroup;
        if (newGroup == StringUtil::BLANK)
        {
            theGroup = this->getGroup();
        }
        else
        {
            theGroup = newGroup;
        }
        MeshPtr newMesh = MeshManager::getSingleton().createManual(newName, theGroup);

        // Copy submeshes first
        std::vector<SubMesh*>::iterator subi;
        SubMesh* newSub;
        for (subi = mSubMeshList.begin(); subi != mSubMeshList.end(); ++subi)
        {
            newSub = newMesh->createSubMesh();
            newSub->mMaterialName = (*subi)->mMaterialName;
            newSub->mMatInitialised = (*subi)->mMatInitialised;
            newSub->operationType = (*subi)->operationType;
            newSub->useSharedVertices = (*subi)->useSharedVertices;

            if (!(*subi)->useSharedVertices)
            {
                // Copy unique vertex data
				newSub->vertexData = (*subi)->vertexData->clone();
            }

            // Copy index data
            delete newSub->indexData;
			newSub->indexData = (*subi)->indexData->clone();
            // Copy any bone assignments
            newSub->mBoneAssignments = (*subi)->mBoneAssignments;
            newSub->mBoneAssignmentsOutOfDate = (*subi)->mBoneAssignmentsOutOfDate;

            // Copy lod face lists
            newSub->mLodFaceList.reserve((*subi)->mLodFaceList.size());
            ProgressiveMesh::LODFaceList::const_iterator facei;
            for (facei = (*subi)->mLodFaceList.begin(); facei != (*subi)->mLodFaceList.end(); ++facei) {
                IndexData* newIndexData = (*facei)->clone();
                newSub->mLodFaceList.push_back(newIndexData);
            }
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
        newMesh->mBoneAssignmentsOutOfDate = mBoneAssignmentsOutOfDate;
        // Copy bounds
        newMesh->mAABB = mAABB;
        newMesh->mBoundRadius = mBoundRadius;

		newMesh->mIsLodManual = mIsLodManual;
		newMesh->mNumLods = mNumLods;
		newMesh->mMeshLodUsageList = mMeshLodUsageList;
        // Unreference edge lists, otherwise we'll delete the same lot twice, build on demand
        MeshLodUsageList::iterator lodi;
        for (lodi = newMesh->mMeshLodUsageList.begin(); lodi != newMesh->mMeshLodUsageList.end(); ++lodi) {
            MeshLodUsage& lod = *lodi;
            lod.edgeData = NULL;
            // TODO: Copy manual lod meshes
        }

		newMesh->mVertexBufferUsage = mVertexBufferUsage;
		newMesh->mIndexBufferUsage = mIndexBufferUsage;
		newMesh->mVertexBufferShadowBuffer = mVertexBufferShadowBuffer;
		newMesh->mIndexBufferShadowBuffer = mIndexBufferShadowBuffer;

        newMesh->mSkeletonName = mSkeletonName;
        newMesh->mSkeleton = mSkeleton;

		// Keep prepared shadow volume info (buffers may already be prepared)
		newMesh->mPreparedForShadowVolumes = mPreparedForShadowVolumes;

		// mEdgeListsBuilt and edgeData of mMeshLodUsageList
		// will up to date on demand. Not copied since internal references, and mesh
		// data may be altered

        newMesh->load();
        newMesh->touch();

        return newMesh;

    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& Mesh::getBounds(void) const
    {
        return mAABB;
    }
    //-----------------------------------------------------------------------
    void Mesh::_setBounds(const AxisAlignedBox& bounds, bool pad)
    {
        mAABB = bounds;
        Vector3 max = mAABB.getMaximum();
        Vector3 min = mAABB.getMinimum();

        // Set sphere bounds; not the tightest by since we're using
        // manual AABB it is the only way
        Real sqLen1 = min.squaredLength();
        Real sqLen2 = max.squaredLength();

        mBoundRadius = Math::Sqrt(std::max(sqLen1, sqLen2)); 
        if (pad) 
        {
            // Pad out the AABB a little, helps with most bounds tests
            Vector3 scaler = (max - min) * MeshManager::getSingleton().getBoundsPaddingFactor();
            mAABB.setExtents(min  - scaler, max + scaler);
            // Pad out the sphere a little too
            mBoundRadius = mBoundRadius + (mBoundRadius * MeshManager::getSingleton().getBoundsPaddingFactor());
        } 
        else 
        {
            mAABB.setExtents(min, max);
            mBoundRadius = mBoundRadius;
        }
		
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
            mSkeleton.setNull();
        }
        else
        {
            // Load skeleton
            try {
                mSkeleton = SkeletonManager::getSingleton().load(skelName, mGroup);
            }
            catch (...)
            {
                mSkeleton.setNull();
                // Log this error
                String msg = "Unable to load skeleton ";
                msg += skelName + " for Mesh " + mName
                    + ". This Mesh will not be animated. "
                    + "You can ignore this message if you are using an offline tool.";
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
    const SkeletonPtr& Mesh::getSkeleton(void) const
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
        assert(!mSkeleton.isNull() && "Skeleton not present");
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
                sizeof(unsigned char)*4 + sizeof(float)*numBlendWeightsPerVertex,
                targetVertexData->vertexCount, 
                HardwareBuffer::HBU_STATIC_WRITE_ONLY, 
                true // use shadow buffer
                );
        // bind new buffer
        bind->setBinding(bindIndex, vbuf);
        const VertexElement *pIdxElem, *pWeightElem;

        // add new vertex elements
        // Note, insert directly after all elements using the same source as 
        // position to abide by pre-Dx9 format restrictions
        const VertexElement* firstElem = decl->getElement(0);
        if(firstElem->getSemantic() == VES_POSITION)
        {
            unsigned short insertPoint = 1;
            while (insertPoint < decl->getElementCount() &&
                decl->getElement(insertPoint)->getSource() == firstElem->getSource())
            {
                ++insertPoint;
            }
            const VertexElement& idxElem = 
                decl->insertElement(insertPoint, bindIndex, 0, VET_UBYTE4, VES_BLEND_INDICES);
            const VertexElement& wtElem = 
                decl->insertElement(insertPoint+1, bindIndex, sizeof(unsigned char)*4, 
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
        VertexBoneAssignmentList::const_iterator i, iend;
        i = boneAssignments.begin();
		iend = boneAssignments.end();
        unsigned char *pBase = static_cast<unsigned char*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD)); 
        // Iterate by vertex
        float *pWeight;
        unsigned char *pIndex;
        for (v = 0; v < targetVertexData->vertexCount; ++v)
        {
            /// Convert to specific pointers
            pWeightElem->baseVertexPointerToElement(pBase, &pWeight);
            pIdxElem->baseVertexPointerToElement(pBase, &pIndex);
            for (unsigned short bone = 0; bone < numBlendWeightsPerVertex; ++bone)
            {
                // Do we still have data for this vertex?
                if (i != iend && i->second.vertexIndex == v)
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
    void Mesh::_notifySkeleton(SkeletonPtr& pSkel)
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
#if OGRE_DEBUG_MODE
        Real prev = 0;
        for (LodDistanceList::const_iterator it = lodDistances.begin();
            it != lodDistances.end(); ++it)
        {
            Real cur = (*it) * (*it);
            assert(cur >= prev && "The lod distances must be sort ascending");
            prev = cur;
        }
#endif

        removeLodLevels();

		StringUtil::StrStreamType str;
		str << "Generating " << lodDistances.size() 
			<< " lower LODs for mesh " << mName;
        LogManager::getSingleton().logMessage(str.str());

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
        mMeshLodUsageList.resize(lodDistances.size() + 1);
        MeshLodUsageList::iterator ilod = mMeshLodUsageList.begin();
        for (idist = lodDistances.begin(); idist != idistend; ++idist)
        {
            // Record usage
            MeshLodUsage& lod = *++ilod;
            lod.fromDepthSquared = (*idist) * (*idist);
            lod.edgeData = 0;
            lod.manualMesh.setNull();
        }
        mNumLods = static_cast<ushort>(lodDistances.size() + 1);
    }
    //---------------------------------------------------------------------
    ushort Mesh::getNumLodLevels(void) const
    {
        return mNumLods;
    }
    //---------------------------------------------------------------------
    const MeshLodUsage& Mesh::getLodLevel(ushort index) const
    {
        assert(index < mMeshLodUsageList.size());
        if (mIsLodManual && index > 0 && mMeshLodUsageList[index].manualMesh.isNull())
        {
            // Load the mesh now
			try {
				mMeshLodUsageList[index].manualMesh = 
					MeshManager::getSingleton().load(
						mMeshLodUsageList[index].manualName,
						mGroup);
				// get the edge data, if required
				if (!mMeshLodUsageList[index].edgeData)
				{
					mMeshLodUsageList[index].edgeData = 
						mMeshLodUsageList[index].manualMesh->getEdgeList(0);
				}
			}
			catch (Exception& e)
			{	
				StringUtil::StrStreamType str;
				str << "Error while loading manual LOD level " 
					<< mMeshLodUsageList[index].manualName 
					<< " - this LOD level will not be rendered. You can "
					<< "ignore this error in offline mesh tools.";
				LogManager::getSingleton().logMessage(str.str());
			}

        }
        return mMeshLodUsageList[index];
    }
    //---------------------------------------------------------------------
	struct ManualLodSortLess : 
	public std::binary_function<const MeshLodUsage&, const MeshLodUsage&, bool>
	{
		bool operator() (const MeshLodUsage& mesh1, const MeshLodUsage& mesh2)
		{
			// sort ascending by depth
			return mesh1.fromDepthSquared < mesh2.fromDepthSquared;
		}
	};
	void Mesh::createManualLodLevel(Real fromDepth, const String& meshName)
	{

		// Basic prerequisites
        assert(fromDepth > 0 && "The LOD depth must be greater than zero");
        assert((mIsLodManual || mNumLods == 1) && "Generated LODs already in use!");

		mIsLodManual = true;
		MeshLodUsage lod;
		lod.fromDepthSquared = fromDepth * fromDepth;
		lod.manualName = meshName;
		lod.manualMesh.setNull();
        lod.edgeData = 0;
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
		lod->manualMesh.setNull();
        if (lod->edgeData) delete lod->edgeData;
        lod->edgeData = 0;
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
        assert(!mEdgeListsBuilt && "Can't modify LOD after edge lists built");

		// Basic prerequisites
        assert(numLevels > 0 && "Must be at least one level (full detail level must exist)");

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
	void Mesh::_setLodUsage(unsigned short level, MeshLodUsage& usage)
	{
        assert(!mEdgeListsBuilt && "Can't modify LOD after edge lists built");

		// Basic prerequisites
		assert(level != 0 && "Can't modify first lod level (full detail)");
		assert(level < mMeshLodUsageList.size() && "Index out of bounds");

		mMeshLodUsageList[level] = usage;
	}
    //---------------------------------------------------------------------
	void Mesh::_setSubMeshLodFaceList(unsigned short subIdx, unsigned short level, 
		IndexData* facedata)
	{
        assert(!mEdgeListsBuilt && "Can't modify LOD after edge lists built");

		// Basic prerequisites
		assert(!mIsLodManual && "Not using generated LODs!");
        assert(subIdx <= mSubMeshList.size() && "Index out of bounds");
		assert(level != 0 && "Can't modify first lod level (full detail)");
		assert(level <= mSubMeshList[subIdx]->mLodFaceList.size() && "Index out of bounds");

		SubMesh* sm = mSubMeshList[subIdx];
		sm->mLodFaceList[level - 1] = facedata;

	}
    //---------------------------------------------------------------------
	ushort Mesh::_getSubMeshIndex(const String& name) const
	{
		SubMeshNameMap::const_iterator i = mSubMeshNameMap.find(name) ;
		if (i == mSubMeshNameMap.end())
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "No SubMesh named " + name + " found.", 
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

        freeEdgeList();
        mMeshLodUsageList.clear();

        // Reinitialise
        mNumLods = 1;
		// Init first (manual) lod
		MeshLodUsage lod;
		lod.fromDepthSquared = 0.0f;
        lod.edgeData = 0;
        lod.manualMesh.setNull();
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
    void Mesh::organiseTangentsBuffer(VertexData *vertexData, 
        unsigned short destCoordSet)
    {
	    VertexDeclaration *vDecl = vertexData->vertexDeclaration ;
	    VertexBufferBinding *vBind = vertexData->vertexBufferBinding ;

	    const VertexElement *tex3D = vDecl->findElementBySemantic(VES_TEXTURE_COORDINATES, destCoordSet);
	    bool needsToBeCreated = false;
    	
	    if (!tex3D) 
        { // no tex coords with index 1
			    needsToBeCreated = true ;
	    } 
        else if (tex3D->getType() != VET_FLOAT3) 
        { 
            // tex buffer exists, but not 3D
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Texture coordinate set " + StringConverter::toString(destCoordSet) + 
                "already exists but is not 3D, therefore cannot contain tangents. Pick "
                "an alternative destination coordinate set. ", 
                "Mesh::organiseTangentsBuffer");
	    }
    	
	    HardwareVertexBufferSharedPtr newBuffer;
	    if (needsToBeCreated) 
        {
            // What we need to do, to be most efficient with our vertex streams, 
            // is to tack the new 3D coordinate set onto the same buffer as the 
            // previous texture coord set
            const VertexElement* prevTexCoordElem = 
                vertexData->vertexDeclaration->findElementBySemantic(
                    VES_TEXTURE_COORDINATES, destCoordSet - 1);
            if (!prevTexCoordElem)
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                    "Cannot locate the texture coordinate element preceding the "
                    "destination texture coordinate set to which to append the new "
                    "tangents.", "Mesh::orgagniseTangentsBuffer");
            }
            // Find the buffer associated with  this element
            HardwareVertexBufferSharedPtr origBuffer = 
                vertexData->vertexBufferBinding->getBuffer(
                    prevTexCoordElem->getSource());
            // Now create a new buffer, which includes the previous contents
            // plus extra space for the 3D coords
		    newBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
                origBuffer->getVertexSize() + 3*sizeof(float), 
                vertexData->vertexCount,
			    origBuffer->getUsage(), 
			    origBuffer->hasShadowBuffer() );
            // Add the new element
		    vDecl->addElement(
                prevTexCoordElem->getSource(), 
                origBuffer->getVertexSize(), 
                VET_FLOAT3, 
                VES_TEXTURE_COORDINATES, 
                destCoordSet);
            // Now copy the original data across
            unsigned char* pSrc = static_cast<unsigned char*>(
                origBuffer->lock(HardwareBuffer::HBL_READ_ONLY));
            unsigned char* pDest = static_cast<unsigned char*>(
                newBuffer->lock(HardwareBuffer::HBL_DISCARD));
            size_t vertSize = origBuffer->getVertexSize();
            for (size_t v = 0; v < vertexData->vertexCount; ++v)
            {
                // Copy original vertex data
                memcpy(pDest, pSrc, vertSize);
                pSrc += vertSize;
                pDest += vertSize;
                // Set the new part to 0 since we'll accumulate in this
                memset(pDest, 0, sizeof(float)*3);
                pDest += sizeof(float)*3;
            }
            origBuffer->unlock();
            newBuffer->unlock();

            // Rebind the new buffer
            vBind->setBinding(prevTexCoordElem->getSource(), newBuffer);
	    } 
    }
    //---------------------------------------------------------------------
    void Mesh::buildTangentVectors(unsigned short sourceTexCoordSet, 
        unsigned short destTexCoordSet)
    {
        if (destTexCoordSet == 0)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Destination texture coordinate set must be greater than 0", 
                "Mesh::buildTangentVectors");
        }

	    // our temp. buffers
	    uint32			vertInd[3];
	    Vector3         vertPos[3];
        Real            u[3], v[3];
	    // setup a new 3D texture coord-set buffer for every sub mesh
	    int nSubMesh = getNumSubMeshes();
        bool sharedGeometryDone = false;
	    for (int sm = 0; sm < nSubMesh; sm++)
	    {
		    // retrieve buffer pointers
		    uint16			*pVIndices16;	// the face indices buffer, read only
		    uint32			*pVIndices32;	// the face indices buffer, read only
		    float			*p2DTC;		// pointer to 2D tex.coords, read only
		    float			*p3DTC;		// pointer to 3D tex.coords, write/read (discard)
		    float			*pVPos;		// vertex position buffer, read only

		    SubMesh *pSubMesh = getSubMesh(sm);

		    // retrieve buffer pointers
		    // first, indices
		    IndexData *indexData = pSubMesh->indexData;
		    HardwareIndexBufferSharedPtr buffIndex = indexData->indexBuffer;
			bool use32bit = false;
			if (buffIndex->getType() == HardwareIndexBuffer::IT_32BIT)
			{
		    	pVIndices32 = static_cast<uint32*>(
					buffIndex->lock(HardwareBuffer::HBL_READ_ONLY)); 
				use32bit = true;
			}
			else
			{
		    	pVIndices16 = static_cast<uint16*>(
					buffIndex->lock(HardwareBuffer::HBL_READ_ONLY)); 
			}
		    // then, vertices
		    VertexData *usedVertexData ;
		    if (pSubMesh->useSharedVertices) {
                // Don't do shared geometry more than once
                if (sharedGeometryDone)
                    continue;
			    usedVertexData = sharedVertexData;
                sharedGeometryDone = true;
		    } else {
			    usedVertexData = pSubMesh->vertexData;
		    }
		    VertexDeclaration *vDecl = usedVertexData->vertexDeclaration;
		    VertexBufferBinding *vBind = usedVertexData->vertexBufferBinding;


		    // make sure we have a 3D coord to place data in
		    organiseTangentsBuffer(usedVertexData, destTexCoordSet);

            // Get the target element
            const VertexElement* destElem = vDecl->findElementBySemantic(VES_TEXTURE_COORDINATES, destTexCoordSet);
            // Get the source element
            const VertexElement* srcElem = vDecl->findElementBySemantic(VES_TEXTURE_COORDINATES, sourceTexCoordSet);

            if (!srcElem || srcElem->getType() != VET_FLOAT2)
            {
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                    "SubMesh " + StringConverter::toString(sm) + " of Mesh " + mName + 
                    " has no 2D texture coordinates at the selected set, therefore we cannot calculate tangents.", 
                    "Mesh::buildTangentVectors");
            }
            HardwareVertexBufferSharedPtr srcBuf, destBuf, posBuf;
            unsigned char *pSrcBase, *pDestBase, *pPosBase;
            size_t srcInc, destInc, posInc;

            srcBuf = vBind->getBuffer(srcElem->getSource());
            // Is the source and destination buffer the same?
            if (srcElem->getSource() == destElem->getSource())
            {
                // lock source for read and write
                pSrcBase = static_cast<unsigned char*>(
                    srcBuf->lock(HardwareBuffer::HBL_NORMAL)); 
                srcInc = srcBuf->getVertexSize();
                pDestBase = pSrcBase;
                destInc = srcInc;
            }
            else
            {
                pSrcBase = static_cast<unsigned char*>(
                    srcBuf->lock(HardwareBuffer::HBL_READ_ONLY)); 
                srcInc = srcBuf->getVertexSize();
                destBuf = vBind->getBuffer(destElem->getSource());
                destInc = destBuf->getVertexSize();
                pDestBase = static_cast<unsigned char*>(
                    destBuf->lock(HardwareBuffer::HBL_NORMAL));
            }
		    
		    // find a vertex coord buffer
		    const VertexElement *elemVPos = vDecl->findElementBySemantic(VES_POSITION);
            if (elemVPos->getSource() == srcElem->getSource())
            {
                pPosBase = pSrcBase;
                posInc = srcInc;
            }
            else if (elemVPos->getSource() == destElem->getSource())
            {
                pPosBase = pDestBase;
                posInc = destInc;
            }
            else
            {
                // A different buffer
                posBuf = vBind->getBuffer(elemVPos->getSource());
                pPosBase = static_cast<unsigned char*>(
                    posBuf->lock(HardwareBuffer::HBL_READ_ONLY));
                posInc = posBuf->getVertexSize();
            }
    		
		    size_t numFaces = indexData->indexCount / 3 ;
    		
		    // loop through all faces to calculate the tangents and normals
		    size_t n;
		    for (n = 0; n < numFaces; ++n)
		    {
			    int i;
			    for (i = 0; i < 3; ++i)
			    {
				    // get indexes of vertices that form a polygon in the position buffer
				    if (use32bit)
					{
						vertInd[i] = *pVIndices32++;
					}
					else
					{
						vertInd[i] = *pVIndices16++;
					}
				    // get the vertices positions from the position buffer
                    unsigned char* vBase = pPosBase + (posInc * vertInd[i]);
                    elemVPos->baseVertexPointerToElement(vBase, &pVPos);
				    vertPos[i].x = pVPos[0];
				    vertPos[i].y = pVPos[1];
				    vertPos[i].z = pVPos[2];
				    // get the vertices tex.coords from the 2D tex.coords buffer
                    vBase = pSrcBase + (srcInc * vertInd[i]);
                    srcElem->baseVertexPointerToElement(vBase, &p2DTC);
				    u[i] = p2DTC[0];
				    v[i] = p2DTC[1];
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
                    unsigned char* vBase = pDestBase + (destInc * vertInd[i]);
                    destElem->baseVertexPointerToElement(vBase, &p3DTC);
				    p3DTC[0] += tangent.x;
				    p3DTC[1] += tangent.y;
				    p3DTC[2] += tangent.z;
			    }
		    }
		    // now loop through all vertices and normalize them
		    size_t numVerts = usedVertexData->vertexCount ;
		    for (n = 0; n < numVerts; ++n)
		    {
                destElem->baseVertexPointerToElement(pDestBase, &p3DTC);
			    // read the vertex
			    Vector3 temp(p3DTC[0], p3DTC[1], p3DTC[2]);
			    // normalize the vertex
			    temp.normalise();
			    // write it back
			    p3DTC[0] = temp.x;
			    p3DTC[1] = temp.y;
			    p3DTC[2] = temp.z;

                pDestBase += destInc;
		    }
		    // unlock buffers
            srcBuf->unlock();
            if (!destBuf.isNull())
            {
                destBuf->unlock();
            }
            if (!posBuf.isNull())
            {
                posBuf->unlock();
            }
		    buffIndex->unlock();
	    }
        
    }

    //---------------------------------------------------------------------
    bool Mesh::suggestTangentVectorBuildParams(unsigned short& outSourceCoordSet, 
        unsigned short& outDestCoordSet)
    {
        // Go through all the vertex data and locate source and dest (must agree)
        bool sharedGeometryDone = false;
        bool foundExisting = false;
        bool firstOne = true;
        SubMeshList::iterator i, iend;
        iend = mSubMeshList.end();
        for (i = mSubMeshList.begin(); i != iend; ++i)
        {
            SubMesh* sm = *i;
            VertexData* vertexData;

            if (sm->useSharedVertices)
            {
                if (sharedGeometryDone)
                    continue;
                vertexData = sharedVertexData;
                sharedGeometryDone = true;
            }
            else
            {
                vertexData = sm->vertexData;
            }

            const VertexElement *sourceElem = 0;
            unsigned short proposedDest = 0;
            unsigned short t = 0;
            for (t = 0; t < OGRE_MAX_TEXTURE_COORD_SETS; ++t)
            {
                const VertexElement* testElem = 
                    vertexData->vertexDeclaration->findElementBySemantic(
                        VES_TEXTURE_COORDINATES, t);
                if (!testElem)
                    break; // finish if we've run out, t will be the target

                if (!sourceElem)
                {
                    // We're still looking for the source texture coords
                    if (testElem->getType() == VET_FLOAT2)
                    {
                        // Ok, we found it
                        sourceElem = testElem;
                    }
                }
                else
                {
                    // We're looking for the destination
                    // Check to see if we've found a possible
                    if (testElem->getType() == VET_FLOAT3)
                    {
                        // This is a 3D set, might be tangents
                        foundExisting = true;
                    }

                }

            }

            // After iterating, we should have a source and a possible destination (t)
            if (!sourceElem)
            {
                OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                    "Cannot locate an appropriate 2D texture coordinate set for "
                    "all the vertex data in this mesh to create tangents from. ",
                    "Mesh::suggestTangentVectorBuildParams");
            }
            // Check that we agree with previous decisions, if this is not the 
            // first one
            if (!firstOne)
            {
                if (sourceElem->getIndex() != outSourceCoordSet)
                {
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                        "Multiple sets of vertex data in this mesh disagree on "
                        "the appropriate index to use for the source texture coordinates. "
                        "This ambiguity must be rectified before tangents can be generated.",
                        "Mesh::suggestTangentVectorBuildParams");
                }
                if (t != outDestCoordSet)
                {
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                        "Multiple sets of vertex data in this mesh disagree on "
                        "the appropriate index to use for the target texture coordinates. "
                        "This ambiguity must be rectified before tangents can be generated.",
                        "Mesh::suggestTangentVectorBuildParams");
                }
            }

            // Otherwise, save this result
            outSourceCoordSet = sourceElem->getIndex();
            outDestCoordSet = t;

            firstOne = false;

       }

        return foundExisting;
        
    }
    //---------------------------------------------------------------------
    void Mesh::buildEdgeList(void)
    {
        if (mEdgeListsBuilt)
            return;

        // Loop over LODs
        for (unsigned int lodIndex = 0; lodIndex < mMeshLodUsageList.size(); ++lodIndex)
        {
            // use getLodLevel to enforce loading of manual mesh lods
            MeshLodUsage& usage = const_cast<MeshLodUsage&>(getLodLevel(lodIndex));

            if (mIsLodManual && lodIndex != 0)
            {
                // Delegate edge building to manual mesh
                // It should have already built it's own edge list while loading
				if (!usage.manualMesh.isNull())
				{
					usage.edgeData = usage.manualMesh->getEdgeList(0);
				}
            }
            else
            {
                // Build
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
                        if (lodIndex == 0)
                        {
                            eb.addIndexData(s->indexData, 0, s->operationType);
                        }
                        else
                        {
                            eb.addIndexData(s->mLodFaceList[lodIndex-1], 0, 
                                s->operationType);
                        }
                    }
                    else
                    {
                        // own vertex data, add it and reference it directly
                        eb.addVertexData(s->vertexData);
                        if (lodIndex == 0)
                        {
                            // Base index data
                            eb.addIndexData(s->indexData, vertexSetCount++, 
                                s->operationType);
                        }
                        else
                        {
                            // LOD index data
                            eb.addIndexData(s->mLodFaceList[lodIndex-1], 
                                vertexSetCount++, s->operationType);
                        }

                    }
                }

                usage.edgeData = eb.build();

                #if OGRE_DEBUG_MODE
                    // Override default log
                    Log* log = LogManager::getSingleton().createLog(
                        mName + "_lod" + StringConverter::toString(lodIndex) + 
                        "_prepshadow.log", false, false);
                    usage.edgeData->log(log);
                #endif

            }
        }
        mEdgeListsBuilt = true;
    }
    //---------------------------------------------------------------------
    void Mesh::freeEdgeList(void)
    {
        if (!mEdgeListsBuilt)
            return;

        // Loop over LODs
        MeshLodUsageList::iterator i, iend;
        iend = mMeshLodUsageList.end();
        unsigned short index = 0;
        for (i = mMeshLodUsageList.begin(); i != iend; ++i, ++index)
        {
            MeshLodUsage& usage = *i;

            if (!mIsLodManual || index == 0)
            {
                // Only delete if we own this data
                // Manual LODs > 0 own their own 
                delete usage.edgeData;
            }
            usage.edgeData = NULL;
        }

        mEdgeListsBuilt = false;
    }
    //---------------------------------------------------------------------
    void Mesh::prepareForShadowVolume(void)
    {
        if (mPreparedForShadowVolumes)
            return;

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
    EdgeData* Mesh::getEdgeList(unsigned int lodIndex)
    {
        // Build edge list on demand
        if (!mEdgeListsBuilt && mAutoBuildEdgeLists)
        {
            buildEdgeList();
        }

        return getLodLevel(lodIndex).edgeData;
    }
    //---------------------------------------------------------------------
    const EdgeData* Mesh::getEdgeList(unsigned int lodIndex) const
    {
        return getLodLevel(lodIndex).edgeData;
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

        float *pSrcPos, *pSrcNorm, *pDestPos, *pDestNorm, *pBlendWeight;
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
        pSrcPos = static_cast<float*>(
            srcPosBuf->lock(HardwareBuffer::HBL_READ_ONLY));
        if (includeNormals)
        {
            if (srcPosNormShareBuffer)
            {
                // Same buffer, must be packed directly after position
                assert (srcElemNorm->getOffset() == sizeof(float) * 3 && 
                    "Normals must be packed directly after positions in buffer!");
                // pSrcNorm will not be used
            }
            else
            {
                // Different buffer
                assert (srcElemNorm->getOffset() == 0 && 
                    "Normals must be first element in dedicated buffer!");
                pSrcNorm = static_cast<float*>(
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
            pBlendWeight = static_cast<float*>(
                srcWeightBuf->lock(HardwareBuffer::HBL_READ_ONLY));
        }
        unsigned short numWeightsPerVertex = 
            VertexElement::getTypeCount(srcElemBlendWeights->getType());


        // Lock destination buffers for writing
        assert (destElemPos->getOffset() == 0 && 
            "Positions must be first element in dedicated buffer!");
        pDestPos = static_cast<float*>(
            destPosBuf->lock(HardwareBuffer::HBL_DISCARD));
        if (includeNormals)
        {
            if (destPosNormShareBuffer)
            {
                // Same buffer, must be packed directly after position
                assert (destElemNorm->getOffset() == sizeof(float) * 3 && 
                    "Normals must be packed directly after positions in buffer!");
                // Must be no other information in the buffer
                assert(destPosBuf->getVertexSize() == 
                    destElemPos->getSize() + destElemNorm->getSize() && 
                    "When software skinning, position & normal buffer must not include "
                    "any other vertex elements!");
                // pDestNorm will not be used
            }
            else
            {
                // Different buffer
                assert (destElemNorm->getOffset() == 0 && 
                    "Normals must be first element in dedicated buffer!");
                // Must be no other information in the buffer
                assert(destPosBuf->getVertexSize() == 
                    destElemPos->getSize() && 
                    "When software skinning, dedicated position buffer must not include "
                    "any other vertex elements!");
                assert(destNormBuf->getVertexSize() == 
                    destElemNorm->getSize() && 
                    "When software skinning, dedicated normal buffer must not include "
                    "any other vertex elements!");
                pDestNorm = static_cast<float*>(
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
                pBlendIdx += sizeof(float) * numWeightsPerVertex;
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
    //---------------------------------------------------------------------
	size_t Mesh::calculateSize(void) const
	{
		// calculate GPU size
		size_t ret = 0;
		size_t i;
		// Shared vertices
		if (sharedVertexData)
		{
			for (i = 0; 
				i < sharedVertexData->vertexBufferBinding->getBufferCount(); 
				++i)
			{
				ret += sharedVertexData->vertexBufferBinding
					->getBuffer(i)->getSizeInBytes();
			}
		}

		SubMeshList::const_iterator si;
		for (si = mSubMeshList.begin(); si != mSubMeshList.end(); ++si)
		{
			// Dedicated vertices
			if (!(*si)->useSharedVertices)
			{
				for (i = 0; 
					i < (*si)->vertexData->vertexBufferBinding->getBufferCount(); 
					++i)
				{
					ret += (*si)->vertexData->vertexBufferBinding
						->getBuffer(i)->getSizeInBytes();
				}
			}
			// Index data
			ret += (*si)->indexData->indexBuffer->getSizeInBytes();

		}
		return ret;
	}

}


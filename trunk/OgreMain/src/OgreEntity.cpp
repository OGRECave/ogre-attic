/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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
#include "OgreEntity.h"

#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreSubEntity.h"
#include "OgreException.h"
#include "OgreSceneManager.h"
#include "OgreLogManager.h"
#include "OgreSkeleton.h"
#include "OgreBone.h"
#include "OgreCamera.h"
#include "OgreTagPoint.h"
#include "OgreAxisAlignedBox.h"
#include "OgreHardwareBufferManager.h"
#include "OgreVector4.h"
#include "OgreRoot.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreSkeletonInstance.h"
#include "OgreEdgeListBuilder.h"

namespace Ogre {
    String Entity::msMovableType = "Entity";
    //-----------------------------------------------------------------------
    Entity::Entity () 
    {
		mFullBoundingBox = new AxisAlignedBox;
        mNormaliseNormals = false;
        mFrameAnimationLastUpdated = 0;
        mHardwareSkinning = false;
        mSkeletonInstance = 0;
    }
    //-----------------------------------------------------------------------
    Entity::Entity( const String& name, Mesh* mesh, SceneManager* creator) :
        mName(name),
        mMesh(mesh),
        mCreatorSceneManager(creator)
    {
		mFullBoundingBox = new AxisAlignedBox;
        mHardwareSkinning = false;
        mSharedBlendedVertexData = NULL;
	
        // Is mesh skeletally animated?
        if (mMesh->hasSkeleton() && mMesh->getSkeleton() != NULL)
        {
            mSkeletonInstance = new SkeletonInstance(mMesh->getSkeleton());
            mSkeletonInstance->load();
        }
        else
        {
            mSkeletonInstance = 0;
        }

        // Build main subentity list
		buildSubEntityList(mesh, &mSubEntityList);

        // Check if mesh is using manual LOD
		if (mesh->isLodManual())
		{
			ushort i, numLod;
			numLod = mesh->getNumLodLevels();
			// NB skip LOD 0 which is the original
			for (i = 1; i < numLod; ++i)
			{
				SubEntityList* sublist = new SubEntityList();
				const Mesh::MeshLodUsage& usage = mesh->getLodLevel(i);
                // Manually create entity
                Entity* lodEnt = new Entity(name + "Lod" + StringConverter::toString(i), 
                    usage.manualMesh, mCreatorSceneManager);
				mLodEntityList.push_back(lodEnt);
			}

		}


        // Initialise the AnimationState, if Mesh has animation
        if (hasSkeleton())
        {
            mesh->_initAnimationState(&mAnimationState);
            mNumBoneMatrices = mSkeletonInstance->getNumBones();
            mBoneMatrices = new Matrix4[mNumBoneMatrices];
            prepareTempBlendBuffers();
        }
        else
        {
            mBoneMatrices = 0;
            mNumBoneMatrices = 0;
        }

        reevaluateVertexProcessing();

        mDisplaySkeleton = false;

		mMeshLodFactorInv = 1.0f;
        mMeshLodIndex = 0;
		mMaxMeshLodIndex = 0; 		// Backwards, remember low value = high detail
		mMinMeshLodIndex = 99;

		mMaterialLodFactorInv = 1.0f;
		mMaxMaterialLodIndex = 0; 		// Backwards, remember low value = high detail
		mMinMaterialLodIndex = 99;


        mFrameAnimationLastUpdated = 0;
    }
    //-----------------------------------------------------------------------
    Entity::~Entity()
    {
        // Delete submeshes
		SubEntityList::iterator i, iend;
		iend = mSubEntityList.end();
        for (i = mSubEntityList.begin(); i != iend; ++i)
        {
			// Delete SubEntity
            delete *i;
        }
		// Delete LOD entities
		LODEntityList::iterator li, liend;
		liend = mLodEntityList.end();
		for (li = mLodEntityList.begin(); li != liend; ++li)
		{
			// Delete 
			delete (*li);
		}
		if (mBoneMatrices)
            delete [] mBoneMatrices;

		delete mFullBoundingBox;

        // Delete shadow renderables
        ShadowRenderableList::iterator si, siend;
        siend = mShadowRenderables.end();
        for (si = mShadowRenderables.begin(); si != siend; ++si)
        {
            delete *si;
        }
        // Delete skeleton instance
        if (mSkeletonInstance)
            delete mSkeletonInstance; 
    }
    //-----------------------------------------------------------------------
    Mesh* Entity::getMesh(void)
    {
        return mMesh;
    }
    //-----------------------------------------------------------------------
    const String& Entity::getName(void) const
    {
        return mName;
    }
    //-----------------------------------------------------------------------
    SubEntity* Entity::getSubEntity(unsigned int index)
    {
        if (index >= mSubEntityList.size())
            Except(Exception::ERR_INVALIDPARAMS,
                "Index out of bounds.",
                "Entity::getSubEntity");
        return mSubEntityList[index];
    }
    //-----------------------------------------------------------------------
    SubEntity* Entity::getSubEntity(const String& name)
    {
		ushort index = mMesh->_getSubMeshIndex(name);
		return getSubEntity(index);
    }
    //-----------------------------------------------------------------------
    unsigned int Entity::getNumSubEntities(void) const
    {
        return static_cast< unsigned int >( mSubEntityList.size() );
    }
    //-----------------------------------------------------------------------
    Entity* Entity::clone( const String& newName)
    {
        Entity* newEnt;
        newEnt = mCreatorSceneManager->createEntity( newName, getMesh()->getName() );
        // Copy material settings
        SubEntityList::iterator i;
        unsigned int n = 0;
        for (i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i, ++n)
        {
            newEnt->getSubEntity(n)->setMaterialName((*i)->getMaterialName());
        }
        newEnt->mAnimationState = mAnimationState;
        return newEnt;
    }
    //-----------------------------------------------------------------------
    void Entity::setMaterialName(const String& name)
    {
        // Set for all subentities
        SubEntityList::iterator i;
        for (i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i)
        {
            (*i)->setMaterialName(name);
        }

    }
    //-----------------------------------------------------------------------
    void Entity::_notifyCurrentCamera(Camera* cam)
    {
        // Calculate the LOD
		if (mParentNode)
		{
			Real squaredDepth = mParentNode->getSquaredViewDepth(cam);

            // Do Mesh LOD
			// Adjust this depth by the entity bias factor
			Real tmp = squaredDepth * mMeshLodFactorInv;
			// Now adjust it by the camera bias
			tmp = tmp * cam->_getLodBiasInverse();
			// Get the index at this biased depth
			mMeshLodIndex = mMesh->getLodIndexSquaredDepth(tmp);
			// Apply maximum detail restriction (remember lower = higher detail)
			mMeshLodIndex = std::max(mMaxMeshLodIndex, mMeshLodIndex);
			// Apply minimum detail restriction (remember higher = lower detail)
			mMeshLodIndex = std::min(mMinMeshLodIndex, mMeshLodIndex);
			
            // Now do material LOD
			// Adjust this depth by the entity bias factor
			tmp = squaredDepth * mMaterialLodFactorInv;
			// Now adjust it by the camera bias
			tmp = tmp * cam->_getLodBiasInverse();
            SubEntityList::iterator i, iend;
            iend = mSubEntityList.end();
            for (i = mSubEntityList.begin(); i != iend; ++i)
            {
			    // Get the index at this biased depth
                unsigned short idx = (*i)->mpMaterial->getLodIndexSquaredDepth(tmp);
			    // Apply maximum detail restriction (remember lower = higher detail)
			    idx = std::max(mMaxMaterialLodIndex, idx);
			    // Apply minimum detail restriction (remember higher = lower detail)
			    (*i)->mMaterialLodIndex = std::min(mMinMaterialLodIndex, idx);
            }


		}
        // Notify any child objects
		ChildObjectList::iterator child_itr = mChildObjectList.begin();
		ChildObjectList::iterator child_itr_end = mChildObjectList.end();
		for( ; child_itr != child_itr_end; child_itr++)
		{
			(*child_itr).second->_notifyCurrentCamera(cam);
		}


    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& Entity::getBoundingBox(void) const
    {
        // Get from Mesh
		*mFullBoundingBox = mMesh->getBounds();
		mFullBoundingBox->merge(getChildObjectsBoundingBox());

        // Don't scale here, this is taken into account when world BBox calculation is done

        return *mFullBoundingBox;
    }
    //-----------------------------------------------------------------------
	AxisAlignedBox Entity::getChildObjectsBoundingBox(void) const
	{
		AxisAlignedBox aa_box;
		AxisAlignedBox full_aa_box;
		full_aa_box.setNull();
		
		ChildObjectList::const_iterator child_itr = mChildObjectList.begin();
		ChildObjectList::const_iterator child_itr_end = mChildObjectList.end();
		for( ; child_itr != child_itr_end; child_itr++)
		{
			aa_box = child_itr->second->getBoundingBox();
            TagPoint* tp = (TagPoint*)child_itr->second->getParentNode();
            // Use transform local to skeleton since world xform comes later
			aa_box.transform(tp->_getFullLocalTransform());
			
			full_aa_box.merge(aa_box);
		}

		return full_aa_box;
	}
    //-----------------------------------------------------------------------
    void Entity::_updateRenderQueue(RenderQueue* queue)
    {
        // Check we're not using a manual LOD
		if (mMeshLodIndex > 0 && mMesh->isLodManual())
		{
			// Use alternate entity
			assert( static_cast< size_t >( mMeshLodIndex - 1 ) < mLodEntityList.size() && 
				"No LOD EntityList - did you build the manual LODs after creating the entity?");
			// index - 1 as we skip index 0 (original lod)
			mLodEntityList[mMeshLodIndex - 1]->_updateRenderQueue(queue);
            return;
		}

		// Add each visible SubEntity to the queue
        SubEntityList::iterator i, iend;
        iend = mSubEntityList.end();
        for (i = mSubEntityList.begin(); i != iend; ++i)
        {
          if((*i)->isVisible())  
            queue->addRenderable(*i, mRenderQueueID, RENDERABLE_DEFAULT_PRIORITY);
        }

        // Since we know we're going to be rendered, take this opportunity to 
        // update the animation
        if (hasSkeleton())
        {
            updateAnimation();
			
			//--- pass this point,  we are sure that the transformation matrix of each bone and tagPoint have been updated			
			ChildObjectList::iterator child_itr = mChildObjectList.begin();
			ChildObjectList::iterator child_itr_end = mChildObjectList.end();
			for( ; child_itr != child_itr_end; child_itr++)
			{
				(*child_itr).second->_updateRenderQueue(queue);
			}
        }

        // HACK to display bones
        // This won't work if the entity is not centered at the origin
        // TODO work out a way to allow bones to be rendered when Entity not centered
        if (mDisplaySkeleton && hasSkeleton())
        {
            int numBones = mSkeletonInstance->getNumBones();
            for (int b = 0; b < numBones; ++b)
            {
                Bone* bone = mSkeletonInstance->getBone(b);
                queue->addRenderable(bone, mRenderQueueID);
            }
        }




    }
    //-----------------------------------------------------------------------
    AnimationState* Entity::getAnimationState(const String& name)
    {
        AnimationStateSet::iterator i = mAnimationState.find(name);

        if (i == mAnimationState.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "No animation entry found named " + name, 
            "Entity::getAnimationState");
        }

        return &(i->second);
    }
    //-----------------------------------------------------------------------
    AnimationStateSet* Entity::getAllAnimationStates(void)
    {
        return &mAnimationState;
    }
    //-----------------------------------------------------------------------
    const String& Entity::getMovableType(void) const
    {
        return msMovableType;
    }
    //-----------------------------------------------------------------------
    void Entity::updateAnimation(void)
    {
        // We only do these tasks if they have not already been done for 
        // this frame 
        Root& root = Root::getSingleton();
        unsigned long currentFrameNumber = root.getCurrentFrameNumber();
        if (mFrameAnimationLastUpdated != currentFrameNumber)
        {
            cacheBoneMatrices();

            // Software blend?
            bool hwSkinning = isHardwareSkinningEnabled();
            if (!hwSkinning ||
                root._getCurrentSceneManager()->getShadowTechnique() == SHADOWTYPE_STENCIL_ADDITIVE ||
                root._getCurrentSceneManager()->getShadowTechnique() == SHADOWTYPE_STENCIL_MODULATIVE)
            {
                // Ok, we need to do a software blend
                // Blend normals in s/w only if we're not using h/w skinning,
                // since shadows only require positions
                bool blendNormals = !hwSkinning;
                // Firstly, check out working vertex buffers
                if (mSharedBlendedVertexData)
                {
                    // Blend shared geometry
                    // NB we suppress hardware upload while doing blend if we're
                    // hardware skinned, because the only reason for doing this
                    // is for shadow, which need only be uploaded then
                    mTempBlendedBuffer.checkoutTempCopies(true, blendNormals);
                    mTempBlendedBuffer.bindTempCopies(mSharedBlendedVertexData, 
                        mHardwareSkinning);
                    Mesh::softwareVertexBlend(mMesh->sharedVertexData, 
                        mSharedBlendedVertexData, mBoneMatrices, blendNormals);
                }
                SubEntityList::iterator i, iend;
                iend = mSubEntityList.end();
                for (i = mSubEntityList.begin(); i != iend; ++i)
                {
                    // Blend dedicated geometry
                    SubEntity* se = *i;
                    if (se->isVisible() && se->mBlendedVertexData)
                    {
                        se->mTempBlendedBuffer.checkoutTempCopies(true, blendNormals);
                        se->mTempBlendedBuffer.bindTempCopies(se->mBlendedVertexData, 
                            mHardwareSkinning);
                        Mesh::softwareVertexBlend(se->mSubMesh->vertexData, 
                            se->mBlendedVertexData, mBoneMatrices, blendNormals);
                    }

                }

            }

            // Trigger update of bounding box if necessary
            if (!mChildObjectList.empty())
                mParentNode->needUpdate();
            mFrameAnimationLastUpdated = currentFrameNumber;
        }
    }
    //-----------------------------------------------------------------------
    void Entity::cacheBoneMatrices(void)
    {
		// Get the appropriate meshes skeleton here
		// Can use lower LOD mesh skeleton if mesh LOD is manual
		// We make the assumption that lower LOD meshes will have
		//   fewer bones than the full LOD, therefore marix stack will be
		//   big enough.
		Mesh* theMesh;
		if (mMesh->isLodManual() && mMeshLodIndex > 1)
		{
			// Use lower detail skeleton
			theMesh = mMesh->getLodLevel(mMeshLodIndex).manualMesh;
			// Lower detail may not have skeleton
			if (!theMesh->hasSkeleton())
			{
				mNumBoneMatrices = 0;
				return;
			}
		}
		else
		{
			// Use normal mesh
			theMesh = mMesh;
		}
	
        mSkeletonInstance->setAnimationState(mAnimationState);
        mSkeletonInstance->_getBoneMatrices(mBoneMatrices);
		//--- Update the child object's transforms
		ChildObjectList::iterator child_itr = mChildObjectList.begin();
		ChildObjectList::iterator child_itr_end = mChildObjectList.end();
		for( ; child_itr != child_itr_end; child_itr++)
		{
            (*child_itr).second->getParentNode()->_update(true, true);
		}
		
        // Apply our current world transform to these too, since these are used as
        // replacement world matrices
		unsigned short i;
        Matrix4 worldXform = _getParentNodeFullTransform();
		mNumBoneMatrices = mSkeletonInstance->getNumBones();

        for (i = 0; i < mNumBoneMatrices; ++i)
        {
            mBoneMatrices[i] = worldXform * mBoneMatrices[i];
        }

    }
    //-----------------------------------------------------------------------
    void Entity::setDisplaySkeleton(bool display)
    {
        mDisplaySkeleton = display;
    }
    //-----------------------------------------------------------------------
	void Entity::setMeshLodBias(Real factor, ushort maxDetailIndex, ushort minDetailIndex)
	{
		assert(factor > 0.0f && "Bias factor must be > 0!");
		mMeshLodFactorInv = 1.0f / factor;
		mMaxMeshLodIndex = maxDetailIndex;
		mMinMeshLodIndex = minDetailIndex;

	}
    //-----------------------------------------------------------------------
	void Entity::setMaterialLodBias(Real factor, ushort maxDetailIndex, ushort minDetailIndex)
	{
		assert(factor > 0.0f && "Bias factor must be > 0!");
		mMaterialLodFactorInv = 1.0f / factor;
		mMaxMaterialLodIndex = maxDetailIndex;
		mMinMaterialLodIndex = minDetailIndex;

	}
    //-----------------------------------------------------------------------
	void Entity::buildSubEntityList(Mesh* mesh, SubEntityList* sublist)
	{
        // Create SubEntities
        unsigned short i, numSubMeshes;
        SubMesh* subMesh;
        SubEntity* subEnt;

        numSubMeshes = mesh->getNumSubMeshes();
        for (i = 0; i < numSubMeshes; ++i)
        {
            subMesh = mesh->getSubMesh(i);
            subEnt = new SubEntity(this, subMesh);
            if (subMesh->isMatInitialised())
                subEnt->setMaterialName(subMesh->getMaterialName());
            sublist->push_back(subEnt);
        }
	}
    //-----------------------------------------------------------------------
    void Entity::setRenderDetail(SceneDetailLevel renderDetail) 
    {
        SubEntityList::iterator i, iend;
        iend = mSubEntityList.end();

        for( i = mSubEntityList.begin(); i != iend; ++i ) 
        { 
            (*i)->setRenderDetail(renderDetail); 
        } 
    }

    //-----------------------------------------------------------------------
	void Entity::attachObjectToBone(const String &boneName, MovableObject *pMovable, const Quaternion &offsetOrientation, const Vector3 &offsetPosition)
	{
		if(pMovable->isAttached())
		{
            Except(Exception::ERR_INVALIDPARAMS, "Object already attached to a sceneNode or a Bone", 
            "Entity::attachObjectToBone");
		}
        if (!hasSkeleton())
        {
            Except(Exception::ERR_INVALIDPARAMS, "This entity's mesh has no skeleton to attach object to.", 
            "Entity::attachObjectToBone");
        }
        Bone* bone = mSkeletonInstance->getBone(boneName);
        if (!bone)
        {
            Except(Exception::ERR_INVALIDPARAMS, "Cannot locate bone named " + boneName, 
            "Entity::attachObjectToBone");
        }

		TagPoint *tp = mSkeletonInstance->createTagPointOnBone(
            bone, offsetOrientation, offsetPosition);
		tp->setParentEntity(this);
		tp->setChildObject(pMovable);
		
		attachObjectImpl(pMovable, tp);
	}

    //-----------------------------------------------------------------------
	void Entity::attachObjectImpl(MovableObject *pObject, TagPoint *pAttachingPoint)
	{
		mChildObjectList[pObject->getName()] = pObject;
		pObject->_notifyAttached(pAttachingPoint, true);
	}

    //-----------------------------------------------------------------------
	MovableObject* Entity::detachObjectFromBone(const String &name)
	{
        ChildObjectList::iterator i = mChildObjectList.find(name);

        if (i == mChildObjectList.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "No child object entry found named " + name, 
            "Entity::detachObjectFromBone");
		}

        i->second->_notifyAttached((TagPoint*)0);
		mChildObjectList.erase(i);
        return i->second;
	}
    //-----------------------------------------------------------------------
    Entity::ChildObjectListIterator Entity::getAttachedObjectIterator()
    {
        return ChildObjectListIterator(mChildObjectList.begin(), mChildObjectList.end());
    }
    //-----------------------------------------------------------------------
	Real Entity::getBoundingRadius(void) const
	{
		Real rad = mMesh->getBoundingSphereRadius();
        // Scale by largest scale factor
        if (mParentNode)
        {
            const Vector3& s = mParentNode->_getDerivedScale();
            rad *= std::max(s.x, std::max(s.y, s.z));
        }
        return rad;
	}
    //-----------------------------------------------------------------------
    void Entity::prepareTempBlendBuffers(void)
    {
        if (mSharedBlendedVertexData) 
        {
            delete mSharedBlendedVertexData;
            mSharedBlendedVertexData = 0;
        }

        if (hasSkeleton())
        {
            // Shared data
            if (mMesh->sharedVertexData)
            {
                // Create temporary vertex blend info
                // Prepare temp vertex data if needed
                // Clone without copying data, remove blending info
                // (since blend is performed in software)
                mSharedBlendedVertexData = 
                    cloneVertexDataRemoveBlendInfo(mMesh->sharedVertexData);
                extractTempBufferInfo(mSharedBlendedVertexData, &mTempBlendedBuffer);
            }

            SubEntityList::iterator i, iend;
            iend = mSubEntityList.end();
            for (i = mSubEntityList.begin(); i != iend; ++i)
            {
                SubEntity* s = *i;
                s->prepareTempBlendBuffers();
            }


        }

    }
    //-----------------------------------------------------------------------
    void Entity::extractTempBufferInfo(VertexData* sourceData, TempBlendedBufferInfo* info)
    {
        VertexDeclaration* decl = sourceData->vertexDeclaration;
        VertexBufferBinding* bind = sourceData->vertexBufferBinding;
        const VertexElement *posElem = decl->findElementBySemantic(VES_POSITION);
        const VertexElement *normElem = decl->findElementBySemantic(VES_NORMAL);

        assert(posElem && "Positions are required");

        info->posBindIndex = posElem->getSource();
        info->srcPositionBuffer = bind->getBuffer(info->posBindIndex);

        if (!normElem)
        {
            info->posNormalShareBuffer = false;
            info->srcNormalBuffer.release();
        }
        else
        {
            info->normBindIndex = normElem->getSource();
            if (info->normBindIndex == info->posBindIndex)
            {
                info->posNormalShareBuffer = true;
                info->srcNormalBuffer.release();
            }
            else
            {
                info->posNormalShareBuffer = false;
                info->srcNormalBuffer = bind->getBuffer(info->normBindIndex);
            }
        }
    }
    //-----------------------------------------------------------------------
    VertexData* Entity::cloneVertexDataRemoveBlendInfo(const VertexData* source)
    {
        // Clone without copying data
        VertexData* ret = source->clone(false);
        const VertexElement* blendIndexElem = 
            source->vertexDeclaration->findElementBySemantic(VES_BLEND_INDICES);
        const VertexElement* blendWeightElem = 
            source->vertexDeclaration->findElementBySemantic(VES_BLEND_WEIGHTS);
        // Remove blend index
        if (blendIndexElem)
        {
            // Remove buffer reference
            ret->vertexBufferBinding->unsetBinding(blendIndexElem->getSource());

        }
        if (blendWeightElem && 
            blendWeightElem->getSource() != blendIndexElem->getSource())
        {
            // Remove buffer reference
            ret->vertexBufferBinding->unsetBinding(blendWeightElem->getSource());
        }
        // remove elements from declaration
        ret->vertexDeclaration->removeElement(VES_BLEND_INDICES);
        ret->vertexDeclaration->removeElement(VES_BLEND_WEIGHTS);

        // Copy reference to wcoord buffer
        if (!source->hardwareShadowVolWBuffer.isNull())
            ret->hardwareShadowVolWBuffer = source->hardwareShadowVolWBuffer;

        return ret;
    }
    //-----------------------------------------------------------------------
    EdgeData* Entity::getEdgeList(void)
    {
        // Get from Mesh
        return mMesh->getEdgeList(mMeshLodIndex);
    }
    //-----------------------------------------------------------------------
    void Entity::reevaluateVertexProcessing(void)
    {
        // init
        mHardwareSkinning = false; 
        mVertexProgramInUse = false; // assume false because we just assign this
        bool firstPass = true;

        SubEntityList::iterator i, iend;
        iend = mSubEntityList.end();
        for (i = mSubEntityList.begin(); i != iend; ++i, firstPass = false)
        {
            Material* m = (*i)->getMaterial();
            // Make sure it's loaded
            m->load();
            Technique* t = m->getBestTechnique();
            if (!t)
            {
                // No supported techniques
                continue;
            }
            Pass* p = t->getPass(0);
            if (!p)
            {
                // No passes, invalid
                continue;
            }
            if (p->hasVertexProgram())
            {
                // If one material uses a vertex program, set this flag 
                // Causes some special processing like forcing a separate light cap
                mVertexProgramInUse = true;
                
                // All materials must support skinning for us to consider using
                // hardware skinning - if one fails we use software
                if (firstPass)
                {
                    mHardwareSkinning = p->getVertexProgram()->isSkeletalAnimationIncluded();
                }
                else
                {
                    mHardwareSkinning = mHardwareSkinning &&
                        p->getVertexProgram()->isSkeletalAnimationIncluded();
                }
            }
        }

    }
    //-----------------------------------------------------------------------
    ShadowCaster::ShadowRenderableListIterator 
    Entity::getShadowVolumeRenderableIterator(
        ShadowTechnique shadowTechnique, const Light* light, 
        HardwareIndexBufferSharedPtr* indexBuffer, 
        bool extrude, Real extrusionDistance, unsigned long flags)
    {
        assert(indexBuffer && "Only external index buffers are supported right now");
        assert((*indexBuffer)->getType() == HardwareIndexBuffer::IT_16BIT && 
            "Only 16-bit indexes supported for now");

        // Potentially delegate to LOD entity
        if (mMesh->isLodManual() && mMeshLodIndex > 0)
        {
            // delegate, we're using manual LOD and not the top lod index
            return mLodEntityList[mMeshLodIndex-1]->getShadowVolumeRenderableIterator(
                shadowTechnique, light, indexBuffer, extrude, 
                extrusionDistance, flags);
        }
        
        bool hasSkeleton = this->hasSkeleton();


        // Prep mesh if required
        // NB This seems to result in memory corruptions, having problems
        // tracking them down. For now, ensure that shadows are enabled
        // before any entities are created
        if(!mMesh->isPreparedForShadowVolumes())
        {
            mMesh->prepareForShadowVolume();
            // reset frame last updated to force update of buffers
            mFrameAnimationLastUpdated = 0;
            // re-prepare buffers
            prepareTempBlendBuffers();
        }


        // Update any animation 
        if (hasSkeleton)
        {
            updateAnimation();
        }

        // Calculate the object space light details
        Vector4 lightPos = light->getAs4DVector();
        // Only use object-space light if we're not doing transforms
        // Since when animating the positions are already transformed into 
        // world space so we need world space light position
        if (!hasSkeleton)
        {
            Matrix4 world2Obj = mParentNode->_getFullTransform().inverse();
            lightPos =  world2Obj * lightPos; 
        }

        // We need to search the edge list for silhouette edges
        EdgeData* edgeList = getEdgeList();

        // Init shadow renderable list if required
        bool init = mShadowRenderables.empty();

        EdgeData::EdgeGroupList::iterator egi;
        ShadowRenderableList::iterator si, siend;
        EntityShadowRenderable* esr = 0;
        if (init)
            mShadowRenderables.resize(edgeList->edgeGroups.size());

        bool updatedSharedGeomNormals = false;
        siend = mShadowRenderables.end();
        egi = edgeList->edgeGroups.begin();
        for (si = mShadowRenderables.begin(); si != siend; ++si, ++egi)
        {
            if (init)
            {
                const VertexData *pVertData = 0;
                if (hasSkeleton)
                {
                    // Use temp buffers
                    pVertData = findBlendedVertexData(egi->vertexData);
                }
                else
                {
                    pVertData = egi->vertexData;
                }

                // Try to find corresponding SubEntity; this allows the 
                // linkage of visibility between ShadowRenderable and SubEntity
                SubEntity* subent = findSubEntityForVertexData(egi->vertexData);
                // Create a new renderable, create a separate light cap if
                // we're using a vertex program (either for this model, or
				// for extruding the shadow volume) since otherwise we can 
				// get depth-fighting on the light cap

                *si = new EntityShadowRenderable(this, indexBuffer, pVertData, 
                    mVertexProgramInUse || !extrude, subent);
            }
            else if (hasSkeleton)
            {
                // If we have a skeleton, we have no guarantee that the position
                // buffer we used last frame is the same one we used last frame
                // since a temporary buffer is requested each frame
                // therefore, we need to update the EntityShadowRenderable
                // with the current position buffer
                static_cast<EntityShadowRenderable*>(*si)->rebindPositionBuffer();
                
            }
            // Get shadow renderable
            esr = static_cast<EntityShadowRenderable*>(*si);
            HardwareVertexBufferSharedPtr esrPositionBuffer = esr->getPositionBuffer();
            // For animated entities we need to recalculate the face normals
            if (hasSkeleton)
            {
                if (egi->vertexData != mMesh->sharedVertexData || !updatedSharedGeomNormals)
                {
                    // recalculate face normals
                    edgeList->updateFaceNormals(egi->vertexSet, esrPositionBuffer);
                    // If we're not extruding in software we still need to update 
                    // the latter part of the buffer (the hardware extruded part)
                    // with the latest animated positions
                    if (!extrude)
                    {
                        // Lock, we'll be locking the (suppressed hardware update) shadow buffer
                        Real* pSrc = static_cast<Real*>(
                            esrPositionBuffer->lock(HardwareBuffer::HBL_NORMAL));
                        Real* pDest = pSrc + (egi->vertexData->vertexCount * 3);
                        memcpy(pDest, pSrc, sizeof(Real) * 3 * egi->vertexData->vertexCount);
                        esrPositionBuffer->unlock();
                    }
                    if (egi->vertexData == mMesh->sharedVertexData)
                    {
                        updatedSharedGeomNormals = true;
                    }
                }
            }
            // Extrude vertices in software if required
            if (extrude)
            {
                extrudeVertices(esrPositionBuffer, 
                    egi->vertexData->vertexCount, 
                    lightPos, extrusionDistance);

            }
            // Stop suppressing hardware update now, if we were
            esrPositionBuffer->suppressHardwareUpdate(false);

        }
        // Calc triangle light facing
        updateEdgeListLightFacing(edgeList, lightPos);

        // Generate indexes and update renderables
        generateShadowVolume(edgeList, *indexBuffer, light,
            mShadowRenderables, flags);


        return ShadowRenderableListIterator(mShadowRenderables.begin(), 
            mShadowRenderables.end());
    }
    //-----------------------------------------------------------------------
    const VertexData* Entity::findBlendedVertexData(const VertexData* orig)
    {
        if (orig == mMesh->sharedVertexData)
        {
            return mSharedBlendedVertexData;
        }
        SubEntityList::iterator i, iend;
        iend = mSubEntityList.end();
        for (i = mSubEntityList.begin(); i != iend; ++i)
        {
            SubEntity* se = *i;
            if (orig == se->getSubMesh()->vertexData)
            {
                return se->getBlendedVertexData();
            }
        }
        // None found
        Except(Exception::ERR_ITEM_NOT_FOUND, 
            "Cannot find blended version of the vertex data specified.",
            "Entity::findBlendedVertexData");
    }
    //-----------------------------------------------------------------------
    SubEntity* Entity::findSubEntityForVertexData(const VertexData* orig)
    {
        if (orig == mMesh->sharedVertexData)
        {
            return 0;
        }

        SubEntityList::iterator i, iend;
        iend = mSubEntityList.end();
        for (i = mSubEntityList.begin(); i != iend; ++i)
        {
            SubEntity* se = *i;
            if (orig == se->getSubMesh()->vertexData)
            {
                return se;
            }
        }

        // None found
        return 0;
    }
    //-----------------------------------------------------------------------
    void Entity::_notifyAttached(Node* parent, bool isTagPoint)
    {
        MovableObject::_notifyAttached(parent, isTagPoint);
        // Also notify LOD entities
        LODEntityList::iterator i, iend;
        iend = mLodEntityList.end();
        for (i = mLodEntityList.begin(); i != iend; ++i)
        {
            (*i)->_notifyAttached(parent, isTagPoint);
        }

    }
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    Entity::EntityShadowRenderable::EntityShadowRenderable(Entity* parent, 
        HardwareIndexBufferSharedPtr* indexBuffer, const VertexData* vertexData,
        bool createSeparateLightCap, SubEntity* subent, bool isLightCap)
        : mParent(parent), mSubEntity(subent)
    {
        // Save link to vertex data
        mOriginalVertexData = vertexData;

        // Initialise render op
        mRenderOp.indexData = new IndexData();
        mRenderOp.indexData->indexBuffer = *indexBuffer;
        mRenderOp.indexData->indexStart = 0;
        // index start and count are sorted out later

        // Create vertex data which just references position component (and 2 component)
        mRenderOp.vertexData = new VertexData();
        mRenderOp.vertexData->vertexDeclaration = 
            HardwareBufferManager::getSingleton().createVertexDeclaration();
        mRenderOp.vertexData->vertexBufferBinding = 
            HardwareBufferManager::getSingleton().createVertexBufferBinding();
        // Map in position data
        mRenderOp.vertexData->vertexDeclaration->addElement(0,0,VET_FLOAT3, VES_POSITION);
        mOriginalPosBufferBinding = 
            vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION)->getSource();
        mPositionBuffer = vertexData->vertexBufferBinding->getBuffer(mOriginalPosBufferBinding);
        mRenderOp.vertexData->vertexBufferBinding->setBinding(0, mPositionBuffer);
        // Map in w-coord buffer (if present)
        if(!vertexData->hardwareShadowVolWBuffer.isNull())
        {
            mRenderOp.vertexData->vertexDeclaration->addElement(1,0,VET_FLOAT1, VES_TEXTURE_COORDINATES, 0);
            mWBuffer = vertexData->hardwareShadowVolWBuffer;
            mRenderOp.vertexData->vertexBufferBinding->setBinding(1, mWBuffer);
        }
        // Use same vertex start as input
        mRenderOp.vertexData->vertexStart = vertexData->vertexStart;

        if (isLightCap)
        {
            // Use original vertex count, no extrusion
            mRenderOp.vertexData->vertexCount = vertexData->vertexCount;
        }
        else
        {
            // Vertex count must take into account the doubling of the buffer,
            // because second half of the buffer is the extruded copy
            mRenderOp.vertexData->vertexCount = 
                vertexData->vertexCount * 2;
            if (createSeparateLightCap)
            {
                // Create child light cap
                mLightCap = new EntityShadowRenderable(parent, 
                    indexBuffer, vertexData, false, subent, true);
            }
        }

    }
    //-----------------------------------------------------------------------
    Entity::EntityShadowRenderable::~EntityShadowRenderable()
    {
        delete mRenderOp.indexData;
        delete mRenderOp.vertexData;
    }
    //-----------------------------------------------------------------------
    void Entity::EntityShadowRenderable::getWorldTransforms(Matrix4* xform) const
    {
        unsigned short numBones = mParent->_getNumBoneMatrices();

		if (!numBones)
        {
            *xform = mParent->_getParentNodeFullTransform();
        }
        else
        {
            // pretransformed
            *xform = Matrix4::IDENTITY;
        }
    }
    //-----------------------------------------------------------------------
    const Quaternion& Entity::EntityShadowRenderable::getWorldOrientation(void) const
    {
        return mParent->getParentNode()->_getDerivedOrientation();
    }
    //-----------------------------------------------------------------------
    const Vector3& Entity::EntityShadowRenderable::getWorldPosition(void) const
    {
        return mParent->getParentNode()->_getDerivedPosition();
    }
    //-----------------------------------------------------------------------
    void Entity::EntityShadowRenderable::rebindPositionBuffer(void)
    {
        mPositionBuffer = mOriginalVertexData->vertexBufferBinding->getBuffer(
            mOriginalPosBufferBinding);
        mRenderOp.vertexData->vertexBufferBinding->setBinding(0, mPositionBuffer);
        if (mLightCap)
        {
            static_cast<EntityShadowRenderable*>(mLightCap)->rebindPositionBuffer();
        }

    }
    //-----------------------------------------------------------------------
    bool Entity::EntityShadowRenderable::isVisible(void) const
    {
        if (mSubEntity)
        {
            return mSubEntity->isVisible();
        }
        else
        {
            return ShadowRenderable::isVisible();
        }
    }


}

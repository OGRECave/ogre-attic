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

namespace Ogre {
    String Entity::msMovableType = "Entity";
    //-----------------------------------------------------------------------
    Entity::Entity ()
    {
    }
    //-----------------------------------------------------------------------
    Entity::Entity( const String& name, Mesh* mesh, SceneManager* creator) :
        mName(name),
        mMesh(mesh),
        mCreatorSceneManager(creator)
    {
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
				buildSubEntityList(usage.manualMesh, sublist);
				mLodSubEntityList.push_back(sublist);
			}

		}


        // Initialise the AnimationState, if Mesh has animation
        if (mesh->hasSkeleton())
        {
            mesh->_initAnimationState(&mAnimationState);
            mNumBoneMatrices = mesh->_getNumBoneMatrices();
            mBoneMatrices = new Matrix4[mNumBoneMatrices];
        }
        else
        {
            mBoneMatrices = 0;
            mNumBoneMatrices = 0;
        }

        mDisplaySkeleton = false;
        mRenderQueueID = RENDER_QUEUE_MAIN;

		mMeshLodFactorInv = 1.0f;
        mMeshLodIndex = 0;
		mMaxMeshLodIndex = 0; 		// Backwards, remember low value = high detail
		mMinMeshLodIndex = 99;



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
		// Delete LOD submeshes
		LODSubEntityList::iterator li, liend;
		liend = mLodSubEntityList.end();
		for (li = mLodSubEntityList.begin(); li != liend; ++li)
		{
			iend = (*li)->end();
			for (i = (*li)->begin(); i != iend; ++i)
			{
				// Delete SubEntity
				delete *i;
			}
			// Delete SubMeshList itself
			delete (*li);

		}
        if (mBoneMatrices)
            delete [] mBoneMatrices;

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
    unsigned int Entity::getNumSubEntities(void)
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
        int n = 0;
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
			// Adjust this depth by the entity bias factor
			squaredDepth = squaredDepth * mMeshLodFactorInv;
			// Now adjust it by the camera bias
			squaredDepth = squaredDepth * cam->_getLodBiasInverse();
			// Get the index at this biased depth
			mMeshLodIndex = mMesh->getLodIndexSquaredDepth(squaredDepth);
			// Apply maximum detail restriction (remember lower = higher detail)
			mMeshLodIndex = std::max(mMaxMeshLodIndex, mMeshLodIndex);
			// Apply minimum detail restriction (remember higher = lower detail)
			mMeshLodIndex = std::min(mMinMeshLodIndex, mMeshLodIndex);
			

		}
    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& Entity::getBoundingBox(void) const
    {
        // Get from Mesh
        return mMesh->getBounds();
    }
    //-----------------------------------------------------------------------
    void Entity::_updateRenderQueue(RenderQueue* queue)
    {
		SubEntityList* subEntList;
        // Check we're not using a manual LOD
		if (mMeshLodIndex > 0 && mMesh->isLodManual())
		{
			// Use alternate subentities
			assert((mMeshLodIndex-1) < mLodSubEntityList.size() && 
				"No LOD SubEntityList - did you build the manual LODs after creating the entity?");
			// index - 1 as we skip index 0 (original lod)
			subEntList = mLodSubEntityList[mMeshLodIndex - 1];
		}
		else
		{
			subEntList = &mSubEntityList;
		}

		// Add each SubEntity to the queue
        SubEntityList::iterator i, iend;
        iend = subEntList->end();
        for (i = subEntList->begin(); i != iend; ++i)
        {
            queue->addRenderable(*i, mRenderQueueID, RENDERABLE_DEFAULT_PRIORITY);
        }

        // Since we know we're going to be rendered, take this opportunity to 
        // cache bone matrices & apply world matrix to them
        if (mMesh->hasSkeleton())
        {
            cacheBoneMatrices();
        }

        // HACK to display bones
        // This won't work if the entity is not centered at the origin
        // TODO work out a way to allow bones to be rendered when Entity not centered
        if (mDisplaySkeleton && mMesh->hasSkeleton())
        {
            Skeleton* pSkel = mMesh->getSkeleton();
            int numBones = pSkel->getNumBones();
            for (int b = 0; b < numBones; ++b)
            {
                Bone* bone = pSkel->getBone(b);
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
    const String Entity::getMovableType(void) const
    {
        return msMovableType;
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

        theMesh->_getBoneMatrices(mAnimationState, mBoneMatrices);
        // Apply our current world transform to these too, since these are used as
        // replacement world matrices
        int i;
        Matrix4 worldXform = mParentNode->_getFullTransform();
		mNumBoneMatrices = theMesh->_getNumBoneMatrices();

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
    void Entity::setRenderQueueGroup(RenderQueueGroupID queueID)
    {
        mRenderQueueID = queueID;
    }
    //-----------------------------------------------------------------------
    RenderQueueGroupID Entity::getRenderQueueGroup(void)
    {
        return mRenderQueueID;
    }
    //-----------------------------------------------------------------------
	void Entity::setLodBias(Real factor, ushort maxDetailIndex, ushort minDetailIndex)
	{
		assert(factor > 0.0f && "Bias factor must be > 0!");
		mMeshLodFactorInv = 1.0f / factor;
		mMaxMeshLodIndex = maxDetailIndex;
		mMinMeshLodIndex = minDetailIndex;

	}
    //-----------------------------------------------------------------------
	void Entity::buildSubEntityList(Mesh* mesh, SubEntityList* sublist)
	{
        // Create SubEntities
        int i, numSubMeshes;
        SubMesh* subMesh;
        SubEntity* subEnt;

        numSubMeshes = mesh->getNumSubMeshes();
        for (i = 0; i < numSubMeshes; ++i)
        {
            subMesh = mesh->getSubMesh(i);
            subEnt = new SubEntity();
            subEnt->mParentEntity = this;
            subEnt->mSubMesh = subMesh;
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
}

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
#include "OgreSubEntity.h"

#include "OgreEntity.h"
#include "OgreSceneManager.h"
#include "OgreMaterialManager.h"
#include "OgreSubMesh.h"
#include "OgreTagPoint.h"
#include "OgreLogManager.h"
#include "OgreMesh.h"
#include "OgreException.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SubEntity::SubEntity (Entity* parent, SubMesh* subMeshBasis)
        : Renderable(), mParentEntity(parent), mSubMesh(subMeshBasis)
    {
        mpMaterial = MaterialManager::getSingleton().getByName("BaseWhite");
        mMaterialLodIndex = 0;
        mRenderDetail = SDL_SOLID;
        mVisible = true;
        mSkelAnimVertexData = 0;
		mSoftwareMorphAnimVertexData = 0;
		mHardwareMorphAnimVertexData = 0;



    }
    //-----------------------------------------------------------------------
    SubEntity::~SubEntity()
    {
        if (mSkelAnimVertexData)
            delete mSkelAnimVertexData;
		if (mHardwareMorphAnimVertexData)
			delete mHardwareMorphAnimVertexData;
		if (mSoftwareMorphAnimVertexData)
			delete mSoftwareMorphAnimVertexData;
    }
    //-----------------------------------------------------------------------
    SubMesh* SubEntity::getSubMesh(void)
    {
        return mSubMesh;
    }
    //-----------------------------------------------------------------------
    const String& SubEntity::getMaterialName(void) const
    {
        return mMaterialName;
    }
    //-----------------------------------------------------------------------
    void SubEntity::setMaterialName( const String& name)
    {

        //String oldName = mMaterialName;
        mMaterialName = name;
        mpMaterial = MaterialManager::getSingleton().getByName(mMaterialName);

        if (mpMaterial.isNull())
        {
            LogManager::getSingleton().logMessage("Can't assign material " + name + 
                " to SubEntity of " + mParentEntity->getName() + " because this "
                "Material does not exist. Have you forgotten to define it in a "
                ".material script?");
            mpMaterial = MaterialManager::getSingleton().getByName("BaseWhite");
            if (mpMaterial.isNull())
            {
                OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Can't assign default material "
                    "to SubEntity of " + mParentEntity->getName() + ". Did "
                    "you forget to call MaterialManager::initialise()?",
                    "SubEntity.setMaterialName");
            }
        }
        // Ensure new material loaded (will not load again if already loaded)
        mpMaterial->load();

        // tell parent to reconsider material vertex processing options
        mParentEntity->reevaluateVertexProcessing();


    }
    //-----------------------------------------------------------------------
    const MaterialPtr& SubEntity::getMaterial(void) const
    {
        return mpMaterial;
    }
    //-----------------------------------------------------------------------
    Technique* SubEntity::getTechnique(void) const
    {
        return mpMaterial->getBestTechnique(mMaterialLodIndex);
    }
    //-----------------------------------------------------------------------
    void SubEntity::getRenderOperation(RenderOperation& op)
    {
		// Use LOD
        mSubMesh->_getRenderOperation(op, mParentEntity->mMeshLodIndex);
		// Deal with any vertex data overrides
		op.vertexData = getVertexDataForBinding();

    }
	//-----------------------------------------------------------------------
	VertexData* SubEntity::getVertexDataForBinding(void)
	{
		if (mSubMesh->useSharedVertices)
		{
			return mParentEntity->getVertexDataForBinding();
		}
		else
		{
			Entity::VertexDataBindChoice c = mParentEntity->chooseVertexDataForBinding();
			switch(c)
			{
			case Entity::BIND_ORIGINAL:
				return mSubMesh->vertexData;
			case Entity::BIND_HARDWARE_MORPH:
				return mHardwareMorphAnimVertexData;
			case Entity::BIND_SOFTWARE_MORPH:
				return mSoftwareMorphAnimVertexData;
			case Entity::BIND_SOFTWARE_SKELETAL:
				return mSkelAnimVertexData;
			};
			// keep compiler happy
			return mSubMesh->vertexData;

		}
	}
    //-----------------------------------------------------------------------
    void SubEntity::getWorldTransforms(Matrix4* xform) const
    {
        if (!mParentEntity->mNumBoneMatrices)
        {
			// no bones
            *xform = mParentEntity->_getParentNodeFullTransform();
        }
        else
        {
			// bones
            if (!mParentEntity->isHardwareAnimationEnabled())
            {
                // Software skinning involves pretransforming
                // No transform required
                *xform = Matrix4::IDENTITY;
            }
            else
            {
                // Bones, use cached matrices built when Entity::_updateRenderQueue was called
                int i;
                for (i = 0; i < mParentEntity->mNumBoneMatrices; ++i)
                {
                    *xform = mParentEntity->mBoneMatrices[i];
                    ++xform;
                }
            }
        }
    }
    //-----------------------------------------------------------------------
    const Quaternion& SubEntity::getWorldOrientation(void) const
    {
        return mParentEntity->mParentNode->_getDerivedOrientation();
    }
    //-----------------------------------------------------------------------
    const Vector3& SubEntity::getWorldPosition(void) const
    {
        return mParentEntity->mParentNode->_getDerivedPosition();
    }

    //-----------------------------------------------------------------------
    unsigned short SubEntity::getNumWorldTransforms(void) const
    {
        if (!mParentEntity->mNumBoneMatrices ||
            !mParentEntity->isHardwareAnimationEnabled())
        {
            // No skeletal animation, or software skinning (pretransformed)
            return 1;
        }
        else
        {
            // Hardware skinning, pass all matrices
            return mParentEntity->mNumBoneMatrices;
        }
    }
    //-----------------------------------------------------------------------
    Real SubEntity::getSquaredViewDepth(const Camera* cam) const
    {
        Node* n = mParentEntity->getParentNode();
        assert(n);
        return n->getSquaredViewDepth(cam);
    }
    //-----------------------------------------------------------------------
    bool SubEntity::getNormaliseNormals(void) const
    {
        return mParentEntity->mNormaliseNormals;
    }
    //-----------------------------------------------------------------------
    const LightList& SubEntity::getLights(void) const
    {
        SceneNode* n = mParentEntity->getParentSceneNode();
        assert(n);
        return n->findLights(mParentEntity->getBoundingRadius());
    }
    //-----------------------------------------------------------------------
    void SubEntity::setVisible(bool visible)
    {
        mVisible = visible;
    }
    //-----------------------------------------------------------------------
    bool SubEntity::isVisible(void) const
    {
        return mVisible;

    }
    //-----------------------------------------------------------------------
    void SubEntity::prepareTempBlendBuffers(void)
    {
		if (mSubMesh->useSharedVertices)
			return;

        if (mSkelAnimVertexData) 
        {
            delete mSkelAnimVertexData;
            mSkelAnimVertexData = 0;
        }
		if (mSoftwareMorphAnimVertexData) 
		{
			delete mSoftwareMorphAnimVertexData;
			mSoftwareMorphAnimVertexData = 0;
		}
		if (mHardwareMorphAnimVertexData) 
		{
			delete mHardwareMorphAnimVertexData;
			mHardwareMorphAnimVertexData = 0;
		}

		if (mParentEntity->hasMorphAnimation())
		{
			// Create temporary vertex blend info
			// Prepare temp vertex data if needed
			// Clone without copying data, remove blending info
			// (since blend is performed in software)
			mSoftwareMorphAnimVertexData = 
				mParentEntity->cloneVertexDataRemoveBlendInfo(mSubMesh->vertexData);
			mParentEntity->extractTempBufferInfo(mSoftwareMorphAnimVertexData, &mTempMorphAnimInfo);

			// Also clone for hardware usage, don't remove blend info since we'll
			// need it if we also hardware skeletally animate
			mHardwareMorphAnimVertexData = mSubMesh->vertexData->clone(false);
		}

		if (mParentEntity->hasSkeleton())
		{
			// Create temporary vertex blend info
			// Prepare temp vertex data if needed
			// Clone without copying data, remove blending info
			// (since blend is performed in software)
			mSkelAnimVertexData = 
				mParentEntity->cloneVertexDataRemoveBlendInfo(mSubMesh->vertexData);
			mParentEntity->extractTempBufferInfo(mSkelAnimVertexData, &mTempSkelAnimInfo);

		}
    }
    //-----------------------------------------------------------------------
    bool SubEntity::getCastsShadows(void) const
    {
        return mParentEntity->getCastShadows();
    }
	//-----------------------------------------------------------------------
	VertexData* SubEntity::_getSkelAnimVertexData(void) 
	{
		assert (mSkelAnimVertexData && "Not software skinned!");
		return mSkelAnimVertexData;
	}
	//-----------------------------------------------------------------------
	VertexData* SubEntity::_getSoftwareMorphAnimVertexData(void)
	{
		assert (mSoftwareMorphAnimVertexData && "Not morph animated!");
		return mSoftwareMorphAnimVertexData;
	}
	//-----------------------------------------------------------------------
	VertexData* SubEntity::_getHardwareMorphAnimVertexData(void)
	{
		assert (mHardwareMorphAnimVertexData && "Not morph animated!");
		return mHardwareMorphAnimVertexData;
	}
	//-----------------------------------------------------------------------
	TempBlendedBufferInfo* SubEntity::_getSkelAnimTempBufferInfo(void) 
	{
		return &mTempSkelAnimInfo;
	}
	//-----------------------------------------------------------------------
	TempBlendedBufferInfo* SubEntity::_getMorphAnimTempBufferInfo(void) 
	{
		return &mTempMorphAnimInfo;
	}
	//-----------------------------------------------------------------------
	void SubEntity::_updateCustomGpuParameter(
		const GpuProgramParameters::AutoConstantEntry& constantEntry,
		GpuProgramParameters* params) const
	{
		if (constantEntry.paramType == GpuProgramParameters::ACT_ANIMATION_PARAMETRIC)
		{
			// get the parametric morph value
			if (mHardwareMorphAnimVertexData)
			{
				params->setConstant(constantEntry.index, 
					mHardwareMorphAnimVertexData->hwMorphParametric);
			}
			else
			{
				params->setConstant(constantEntry.index, 
					mParentEntity->mHardwareMorphAnimVertexData->hwMorphParametric);
			}
		}
		else
		{
			// default
			return Renderable::_updateCustomGpuParameter(constantEntry, params);
		}
	}
	//-----------------------------------------------------------------------------
	void SubEntity::copyOriginalVertexDataToMorph(void)
	{
		if (!mSubMesh->useSharedVertices)
		{
			const VertexElement* srcPosElem = 
				mSubMesh->vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
			HardwareVertexBufferSharedPtr srcBuf = 
				mSubMesh->vertexData->vertexBufferBinding->getBuffer(
				srcPosElem->getSource());

			// Bind to software
			const VertexElement* destPosElem = 
				mSoftwareMorphAnimVertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
			mSoftwareMorphAnimVertexData->vertexBufferBinding->setBinding(
				destPosElem->getSource(), srcBuf);

			// Bind to hardware as both pos1 and pos2
			if (!mHardwareMorphAnimVertexData->hwMorphTargetElement)
				mHardwareMorphAnimVertexData->allocatehwMorphTargetElement();
			destPosElem = 
				mHardwareMorphAnimVertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
			mHardwareMorphAnimVertexData->vertexBufferBinding->setBinding(
				destPosElem->getSource(), srcBuf);
			mHardwareMorphAnimVertexData->vertexBufferBinding->setBinding(
				mHardwareMorphAnimVertexData->hwMorphTargetElement->getSource(),
				srcBuf);
			mHardwareMorphAnimVertexData->hwMorphParametric = 0.0f;

		}
	}


}

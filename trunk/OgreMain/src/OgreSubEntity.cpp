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
#include "OgreSubEntity.h"

#include "OgreEntity.h"
#include "OgreSceneManager.h"
#include "OgreMaterialManager.h"
#include "OgreSubMesh.h"
#include "OgreTagPoint.h"
#include "OgreLogManager.h"
#include "OgreMesh.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SubEntity::SubEntity (Entity* parent, SubMesh* subMeshBasis)
        :mParentEntity(parent), mSubMesh(subMeshBasis)
    {
        mpMaterial = static_cast<Material*>(MaterialManager::getSingleton().getByName("BaseWhite"));
        mMaterialLodIndex = 0;
        mRenderDetail = SDL_SOLID;
        mVisible = true;
        mBlendedVertexData = 0;
        mBlendedVertexData = NULL;



    }
    //-----------------------------------------------------------------------
    SubEntity::~SubEntity()
    {
        if (mBlendedVertexData)
            delete mBlendedVertexData;
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
        // Update SceneManager re material change
        //mParentEntity->mCreatorSceneManager->_notifyMaterialUsage(oldName, mMaterialName, this);
        mpMaterial = (Material*)MaterialManager::getSingleton().getByName(mMaterialName);

        if (!mpMaterial)
        {
            LogManager::getSingleton().logMessage("Can't assign material " + name + 
                " to SubEntity of " + mParentEntity->getName() + " because this "
                "Material does not exist. Have you forgotten to define it in a "
                ".material script?");
            mpMaterial = (Material*)MaterialManager::getSingleton().getByName("BaseWhite");
        }
        // Ensure new material loaded (will not load again if already loaded)
        mpMaterial->load();

        // tell parent to reconsider material vertex processing options
        mParentEntity->reevaluateVertexProcessing();


    }
    //-----------------------------------------------------------------------
    Material* SubEntity::getMaterial(void) const
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
        // Do we need to use software skinned vertex data?
        if (mParentEntity->hasSkeleton() && !mParentEntity->mHardwareSkinning)
        {
            op.vertexData = mSubMesh->useSharedVertices ? 
                mParentEntity->mSharedBlendedVertexData : mBlendedVertexData;

        }
    }
    //-----------------------------------------------------------------------
    void SubEntity::getWorldTransforms(Matrix4* xform) const
    {
        if (!mParentEntity->mNumBoneMatrices)
        {
            *xform = mParentEntity->_getParentNodeFullTransform();
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
        if (!mParentEntity->mNumBoneMatrices)
            return 1;
        else
            return mParentEntity->mNumBoneMatrices;
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
        Node* n = mParentEntity->getParentNode();
        assert(n);
        return n->getLights();
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
        if (mBlendedVertexData) 
        {
            delete mBlendedVertexData;
            mBlendedVertexData = 0;
        }
        // Clone without copying data
        mBlendedVertexData = 
            mParentEntity->cloneVertexDataRemoveBlendInfo(mSubMesh->vertexData);
        mParentEntity->extractTempBufferInfo(mBlendedVertexData, &mTempBlendedBuffer);
    }

}

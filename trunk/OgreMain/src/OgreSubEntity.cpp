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
#include "OgreSubEntity.h"

#include "OgreEntity.h"
#include "OgreSceneManager.h"
#include "OgreMaterialManager.h"
#include "OgreSubMesh.h"
#include "OgreTagPoint.h"
#include "OgreLogManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SubEntity::SubEntity ()
    {
        mpMaterial = 0;
        mRenderDetail = SDL_SOLID;
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


    }
    //-----------------------------------------------------------------------
    Material* SubEntity::getMaterial(void) const
    {
        return mpMaterial;
    }
    //-----------------------------------------------------------------------
    void SubEntity::getRenderOperation(RenderOperation& rend)
    {
		// Use LOD
        mSubMesh->_getRenderOperation(rend, mParentEntity->mMeshLodIndex);
    }
    //-----------------------------------------------------------------------
    void SubEntity::getWorldTransforms(Matrix4* xform)
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
    unsigned short SubEntity::getNumWorldTransforms(void)
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

}

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreSubEntity.h"

#include "OgreEntity.h"
#include "OgreSceneManager.h"
#include "OgreMaterialManager.h"
#include "OgreSubMesh.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SubEntity::SubEntity ()
    {
        mpMaterial = 0;
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
        mSubMesh->_getRenderOperation(rend);
    }
    //-----------------------------------------------------------------------
    void SubEntity::getWorldTransform(Matrix4& xform)
    {
        xform = mParentEntity->getParentNode()->_getFullTransform();
    }

}

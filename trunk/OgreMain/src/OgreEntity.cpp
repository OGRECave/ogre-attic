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
#include "OgreEntity.h"

#include "OgreMesh.h"
#include "OgreSubMesh.h"
#include "OgreSubEntity.h"
#include "OgreException.h"
#include "OgreSceneManager.h"

namespace Ogre {
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
                subEnt->setMaterialName(subMesh->getMaterial()->getName());
            mSubEntityList.push_back(subEnt);
        }

    }
    //-----------------------------------------------------------------------
    Entity::~Entity()
    {
        // Delete submeshes
        for (SubEntityList::iterator i = mSubEntityList.begin();
            i != mSubEntityList.end(); ++i)
        {
            delete *i;
        }
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
        return mSubEntityList.size();
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
        // do nothing
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
        // Add each SubEntity to the queue
        SubEntityList::iterator i, iend;
        iend = mSubEntityList.end();
        for (i = mSubEntityList.begin(); i != iend; ++i)
        {
            queue->addRenderable(*i);
        }

    }
}

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
#include "OgreSceneNode.h"

#include "OgreException.h"
#include "OgreEntity.h"
#include "OgreCamera.h"
#include "OgreLight.h"
#include "OgreMath.h"
#include "OgreSceneManager.h"
#include "OgreMovableObject.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SceneNode::SceneNode(SceneManager* creator) 
    : mCreator(creator)
    {
    }
    //-----------------------------------------------------------------------
    SceneNode::~SceneNode()
    {
    }
    //-----------------------------------------------------------------------
    void SceneNode::_update(Camera* cam, bool updateChildren)
    {
        // Update transforms
        _updateFromParent();

        // Tell attached objects about camera position (incase any extra processing they want to do)
        std::vector<MovableObject*>::iterator i;
        for (i = mObjects.begin(); i != mObjects.end(); ++i)
        {
            (*i)->_notifyCurrentCamera(cam);
        }

        if (updateChildren)
        {
            std::vector<Node*>::iterator i;
            for (i = mChildren.begin(); i != mChildren.end(); ++i)
            {
                SceneNode* sceneChild = static_cast<SceneNode*>(*i);
                sceneChild->_update(cam, updateChildren);
            }
        }

        _updateBounds();

    }

    //-----------------------------------------------------------------------
    void SceneNode::attachObject(MovableObject* obj)
    {
        mObjects.push_back(obj);
        obj->_notifyAttached(this);

        // Also add to name index
        mObjectsByName[obj->getName()] = obj;
    }
    //-----------------------------------------------------------------------
    unsigned short SceneNode::numAttachedObjects(void)
    {
        return static_cast< unsigned short >( mObjects.size() );
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::getAttachedObject(unsigned short index)
    {
        if (index < mObjects.size())
        {
            std::vector<MovableObject*>::iterator i = mObjects.begin();
            return i[index];
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Object index out of bounds.", "SceneNode::getAttchedEntity");
        }
        return 0;
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::getAttachedObject(const String& name)
    {
        // Look up 
        std::map<String, MovableObject*>::iterator i = mObjectsByName.find(name);

        if (i == mObjectsByName.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Attached object " + 
                name + " not found.", "SceneNode::getAttachedObject");
        }

        return i->second;

    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::detachObject(unsigned short index)
    {
        MovableObject* ret;
        if (index < mObjects.size())
        {

            std::vector<MovableObject*>::iterator i = mObjects.begin() + index;
            ret = *i;
            mObjects.erase(i);
            ret->_notifyAttached(0);

            // Also remove from name index
            std::map<String, MovableObject*>::iterator idxi = 
                mObjectsByName.find(ret->getName());
            mObjectsByName.erase(idxi);


            return ret;

            /* XXX temas: Original code
            std::vector<MovableObject*>::iterator i = mObjects.begin();
            ret = i[index];
            mObjects.erase(&i[index]);
            ret->_notifyAttached(0);
            return ret;
            */
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Object index out of bounds.", "SceneNode::getAttchedEntity");
        }
        return 0;

    }
    //-----------------------------------------------------------------------
    void SceneNode::attachCamera(Camera* cam)
    {
        attachObject(cam);
    }
    //-----------------------------------------------------------------------
    void SceneNode::attachLight(Light* lgt)
    {
        attachObject(lgt);
    }
    //-----------------------------------------------------------------------
    void SceneNode::detachAllObjects(void)
    {
        mObjects.clear();
    }
    //-----------------------------------------------------------------------
    void SceneNode::_updateBounds(void)
    {
        // Reset bounds first
        mWorldAABB.setNull();

        // Update bounds from own attached objects
        std::vector<MovableObject*>::iterator i;
        AxisAlignedBox bx;
        for (i = mObjects.begin(); i != mObjects.end(); ++i)
        {
            // Get local bounds of object
            bx = (*i)->getBoundingBox();
            // Transform by aggregated transform
            bx.transform(_getFullTransform());

            mWorldAABB.merge(bx);
        }

        // Merge with children
        std::vector<Node*>::iterator child;
        for (child = mChildren.begin(); child != mChildren.end(); ++child)
        {
            SceneNode* sceneChild = static_cast<SceneNode*>(*child);
            mWorldAABB.merge(sceneChild->mWorldAABB);
        }

    }
    //-----------------------------------------------------------------------
    void SceneNode::_findVisibleObjects(Camera* cam, RenderQueue* queue, bool includeChildren)
    {
        // Check self visible
        if (!cam->isVisible(mWorldAABB))
            return;

        // Add all entities
        std::vector<MovableObject*>::iterator iobj;
        std::vector<MovableObject*>::iterator iobjend = mObjects.end();
        for (iobj = mObjects.begin(); iobj != iobjend; ++iobj)
        {
            (*iobj)->_updateRenderQueue(queue);
        }

        if (includeChildren)
        {
            std::vector<Node*>::iterator child, childend;
            childend = mChildren.end();
            for (child = mChildren.begin(); child != childend; ++child)
            {
                SceneNode* sceneChild = static_cast<SceneNode*>(*child);
                sceneChild->_findVisibleObjects(cam, queue, includeChildren);
            }
        }

    }
    //-----------------------------------------------------------------------
    Node* SceneNode::createChildImpl(void)
    {
        return mCreator->createSceneNode();
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::createChild(const Vector3& translate, const Quaternion& rotate)
    {
        // Provided as a passthrough to superclass just for compatibility with 
        // the previous version of the SceneNode interface (before I abstracted to Node)
        return static_cast<SceneNode*>(Node::createChild(translate, rotate));
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::getChild(unsigned short index)
    {
        // Provided as a passthrough to superclass just for compatibility with 
        // the previous version of the SceneNode interface (before I abstracted to Node)
        return static_cast<SceneNode*>(Node::getChild(index));
    }
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::removeChild(unsigned short index)
    {
        // Provided as a passthrough to superclass just for compatibility with 
        // the previous version of the SceneNode interface (before I abstracted to Node)
        return static_cast<SceneNode*>(Node::removeChild(index));
    }
    //-----------------------------------------------------------------------
    AxisAlignedBox SceneNode::_getWorldAABB(void) const
    {
        return mWorldAABB;
    }


}

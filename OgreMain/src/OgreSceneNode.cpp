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
#include "OgreSceneNode.h"

#include "OgreException.h"
#include "OgreEntity.h"
#include "OgreCamera.h"
#include "OgreLight.h"
#include "OgreMath.h"
#include "OgreSceneManager.h"
#include "OgreMovableObject.h"
#include "OgreWireBoundingBox.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    SceneNode::SceneNode(SceneManager* creator) 
    : Node(), mWireBoundingBox(0), mShowBoundingBox(false), mCreator(creator)
    {
        needUpdate();
    }
    //-----------------------------------------------------------------------
    SceneNode::SceneNode(SceneManager* creator, const String& name) 
    : Node(name), mWireBoundingBox(0), mShowBoundingBox(false), mCreator(creator)
    {
        needUpdate();
    }
    //-----------------------------------------------------------------------
    SceneNode::~SceneNode()
    {
		if (mWireBoundingBox) {
			delete mWireBoundingBox;
		}
    }
    //-----------------------------------------------------------------------
    void SceneNode::_update(bool updateChildren, bool parentHasChanged)
    {
        Node::_update(updateChildren, parentHasChanged);
        _updateBounds();

    }

    //-----------------------------------------------------------------------
    void SceneNode::attachObject(MovableObject* obj)
    {
        obj->_notifyAttached(this);

        // Also add to name index
        std::pair<ObjectMap::iterator, bool> insresult = 
            mObjectsByName.insert(ObjectMap::value_type(obj->getName(), obj));
        assert(insresult.second && "Object was not attached because an object of the "
            "same name was already attached to this node.");

        // Make sure bounds get updated (must go right to the top)
        needUpdate();
    }
    //-----------------------------------------------------------------------
    unsigned short SceneNode::numAttachedObjects(void)
    {
        return static_cast< unsigned short >( mObjectsByName.size() );
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::getAttachedObject(unsigned short index)
    {
        if (index < mObjectsByName.size())
        {
            ObjectMap::iterator i = mObjectsByName.begin();
            // Increment (must do this one at a time)            
            while (index--)++i;

            return i->second;
        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Object index out of bounds.", "SceneNode::getAttachedObject");
        }
        return 0;
    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::getAttachedObject(const String& name)
    {
        // Look up 
        ObjectMap::iterator i = mObjectsByName.find(name);

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
        if (index < mObjectsByName.size())
        {

            ObjectMap::iterator i = mObjectsByName.begin();
            // Increment (must do this one at a time)            
            while (index--)++i;

            ret = i->second;
            mObjectsByName.erase(i);
            ret->_notifyAttached((SceneNode*)0);

            // Make sure bounds get updated (must go right to the top)
            needUpdate();

            return ret;

        }
        else
        {
            Except(Exception::ERR_INVALIDPARAMS, "Object index out of bounds.", "SceneNode::getAttchedEntity");
        }
        return 0;

    }
    //-----------------------------------------------------------------------
    MovableObject* SceneNode::detachObject(const String& name)
    {
        ObjectMap::iterator it = mObjectsByName.find(name);
        if (it == mObjectsByName.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Object " + name + " is not attached "
                "to this node.", "SceneNode::detachObject");
        }
        MovableObject* ret = it->second;
        mObjectsByName.erase(it);
        ret->_notifyAttached((SceneNode*)0);
        // Make sure bounds get updated (must go right to the top)
        needUpdate();
        
        return ret;

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
		ObjectMap::iterator itr;
		MovableObject* ret;
		for ( itr = mObjectsByName.begin(); itr != mObjectsByName.end(); itr++ )
		{
		  ret = itr->second;
		  ret->_notifyAttached((SceneNode*)0);
		}
        mObjectsByName.clear();
        // Make sure bounds get updated (must go right to the top)
        needUpdate();
    }
    //-----------------------------------------------------------------------
    void SceneNode::_updateBounds(void)
    {
        // Reset bounds first
        mWorldAABB.setNull();

        // Update bounds from own attached objects
        ObjectMap::iterator i;
        AxisAlignedBox bx;
        for (i = mObjectsByName.begin(); i != mObjectsByName.end(); ++i)
        {
            // Merge world bounds of each object
            mWorldAABB.merge(i->second->getWorldBoundingBox(true));
        }

        // Merge with children
        ChildNodeMap::iterator child;
        for (child = mChildren.begin(); child != mChildren.end(); ++child)
        {
            SceneNode* sceneChild = static_cast<SceneNode*>(child->second);
            mWorldAABB.merge(sceneChild->mWorldAABB);
        }

    }
    //-----------------------------------------------------------------------
    void SceneNode::_findVisibleObjects(Camera* cam, RenderQueue* queue, bool includeChildren, bool displayNodes)
    {
        // Check self visible
        if (!cam->isVisible(mWorldAABB))
            return;

        // Add all entities
        ObjectMap::iterator iobj;
        ObjectMap::iterator iobjend = mObjectsByName.end();
        for (iobj = mObjectsByName.begin(); iobj != iobjend; ++iobj)
        {
            // Tell attached objects about camera position (incase any extra processing they want to do)
            iobj->second->_notifyCurrentCamera(cam);
            if (iobj->second->isVisible())
            {
                iobj->second->_updateRenderQueue(queue);
            }
        }

        if (includeChildren)
        {
            ChildNodeMap::iterator child, childend;
            childend = mChildren.end();
            for (child = mChildren.begin(); child != childend; ++child)
            {
                SceneNode* sceneChild = static_cast<SceneNode*>(child->second);
                sceneChild->_findVisibleObjects(cam, queue, includeChildren, displayNodes);
            }
        }

        if (displayNodes)
        {
            // Include self in the render queue
            queue->addRenderable(this);
        }

		// Check if the bounding box should be shown.
		// See if our flag is set or if the scene manager flag is set.
		if (mShowBoundingBox || mCreator->getShowBoundingBoxes()) 
		{ 
			_addBoundingBoxToQueue(queue);
		}


    }


	void SceneNode::_addBoundingBoxToQueue(RenderQueue* queue) {
		// Create a WireBoundingBox if needed.
		if (mWireBoundingBox == NULL) {
			mWireBoundingBox = new WireBoundingBox();
		}
		mWireBoundingBox->setupBoundingBox(mWorldAABB);
		queue->addRenderable(mWireBoundingBox);
	}

	void SceneNode::showBoundingBox(bool bShow) {
		mShowBoundingBox = bShow;
	}

	bool SceneNode::getShowBoundingBox() {
		return mShowBoundingBox;
	}


    //-----------------------------------------------------------------------
    Node* SceneNode::createChildImpl(void)
    {
        return mCreator->createSceneNode();
    }
    //-----------------------------------------------------------------------
    Node* SceneNode::createChildImpl(const String& name)
    {
        return mCreator->createSceneNode(name);
    }
    //-----------------------------------------------------------------------
    AxisAlignedBox SceneNode::_getWorldAABB(void) const
    {
        return mWorldAABB;
    }
    //-----------------------------------------------------------------------
    SceneNode::ObjectIterator SceneNode::getAttachedObjectIterator(void)
    {
        return ObjectIterator(mObjectsByName.begin(), mObjectsByName.end());
    }
    //-----------------------------------------------------------------------
    SceneManager* SceneNode::getCreator(void)
    {
        return mCreator;
    }
    //-----------------------------------------------------------------------
    void SceneNode::removeAndDestroyChild(const String& name)
    {
        SceneNode* pChild = static_cast<SceneNode*>(getChild(name));
        pChild->removeAndDestroyAllChildren();

        removeChild(name);
        pChild->getCreator()->destroySceneNode(name);

    }
    //-----------------------------------------------------------------------
    void SceneNode::removeAndDestroyChild(unsigned short index)
    {
        SceneNode* pChild = static_cast<SceneNode*>(getChild(index));
        pChild->removeAndDestroyAllChildren();

        removeChild(index);
        pChild->getCreator()->destroySceneNode(pChild->getName());
    }
    //-----------------------------------------------------------------------
    void SceneNode::removeAndDestroyAllChildren(void)
    {
        ChildNodeMap::iterator i, iend;
        iend = mChildren.end();
        for (i = mChildren.begin(); i != iend; ++i)
        {
            SceneNode* sn = static_cast<SceneNode*>(i->second);
            sn->removeAndDestroyAllChildren();
            sn->getCreator()->destroySceneNode(sn->getName());
        }
	    mChildren.clear();
        needUpdate();
    }


}

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
    : Node(), mLightListDirty(true), mWireBoundingBox(0), mShowBoundingBox(false), 
    mCreator(creator), mYawFixed(false), mAutoTrackTarget(0)
    {
        needUpdate();
    }
    //-----------------------------------------------------------------------
    SceneNode::SceneNode(SceneManager* creator, const String& name) 
    : Node(name), mLightListDirty(true), mWireBoundingBox(0), mShowBoundingBox(false), 
    mCreator(creator), mYawFixed(false), mAutoTrackTarget(0)
    {
        needUpdate();
    }
    //-----------------------------------------------------------------------
    SceneNode::~SceneNode()
    {
        // Detach all objects, do this manually to avoid needUpdate() call 
        // which can fail because of deleted items
		ObjectMap::iterator itr;
		MovableObject* ret;
		for ( itr = mObjectsByName.begin(); itr != mObjectsByName.end(); itr++ )
		{
		  ret = itr->second;
		  ret->_notifyAttached((SceneNode*)0);
		}
        mObjectsByName.clear();

        if (mWireBoundingBox) {
			delete mWireBoundingBox;
		}
    }
    //-----------------------------------------------------------------------
    void SceneNode::_update(bool updateChildren, bool parentHasChanged)
    {
        Node::_update(updateChildren, parentHasChanged);
        _updateBounds();
        mLightListDirty = true;

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
    unsigned short SceneNode::numAttachedObjects(void) const
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
    void SceneNode::detachObject(MovableObject* obj)
    {
        ObjectMap::iterator i, iend;
        iend = mObjectsByName.end();
        for (i = mObjectsByName.begin(); i != iend; ++i)
        {
            if (i->second == obj)
            {
                mObjectsByName.erase(i);
                break;
            }
        }
        obj->_notifyAttached((SceneNode*)0);

        // Make sure bounds get updated (must go right to the top)
        needUpdate();

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

	bool SceneNode::getShowBoundingBox() const {
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
    SceneManager* SceneNode::getCreator(void) const
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
    //-----------------------------------------------------------------------
	SceneNode* SceneNode::createChildSceneNode(const Vector3& translate, 
        const Quaternion& rotate)
	{
		return static_cast<SceneNode*>(this->createChild(translate, rotate));
	}
    //-----------------------------------------------------------------------
    SceneNode* SceneNode::createChildSceneNode(const String& name, const Vector3& translate, 
		const Quaternion& rotate)
	{
		return static_cast<SceneNode*>(this->createChild(name, translate, rotate));
	}
    //-----------------------------------------------------------------------
    const LightList& SceneNode::getLights(void) const
    {
        // TEMP FIX
        // If a scene node is static and lights have moved, light list won't change
        // can't use a simple global boolean flag since this is only called for
        // visible nodes, so temporarily visible nodes will not be updated
        // Since this is only called for visible nodes, skip the check for now
        //if (mLightListDirty)
        {
            // Use SceneManager to calculate
            mCreator->_populateLightList(this->_getDerivedPosition(), mLightList);
            mLightListDirty = false;
        }
        return mLightList;

    }
    //-----------------------------------------------------------------------
    void SceneNode::setAutoTracking(bool enabled, SceneNode* target, 
        const Vector3& localDirectionVector,
        const Vector3& offset)
    {
        if (enabled)
        {
            mAutoTrackTarget = target;
            mAutoTrackOffset = offset;
            mAutoTrackLocalDirection = localDirectionVector;
        }
        else
        {
            mAutoTrackTarget = 0;
        }
        mCreator->_notifyAutotrackingSceneNode(this, enabled);
    }
    //-----------------------------------------------------------------------
    void SceneNode::setFixedYawAxis(bool useFixed, const Vector3& fixedAxis)
    {
        mYawFixed = useFixed;
        mYawFixedAxis = fixedAxis;
    }

    //-----------------------------------------------------------------------
    void SceneNode::setDirection(Real x, Real y, Real z, TransformSpace relativeTo, 
        const Vector3& localDirectionVector)
    {
        setDirection(Vector3(x,y,z), relativeTo, localDirectionVector);
    }

    //-----------------------------------------------------------------------
    void SceneNode::setDirection(const Vector3& vec, TransformSpace relativeTo, 
        const Vector3& localDirectionVector)
    {
        // Do nothing if given a zero vector
        if (vec == Vector3::ZERO) return;

        // Adjust vector so that it is relative to local Z
        Vector3 zAdjustVec;
        if (localDirectionVector == Vector3::NEGATIVE_UNIT_Z)
        {
            zAdjustVec = -vec;
        }
        else
        {
            Quaternion localToUnitZ = localDirectionVector.getRotationTo(Vector3::UNIT_Z);
            zAdjustVec = localToUnitZ * vec;
        }
        zAdjustVec.normalise();

        Quaternion targetOrientation;
        if( mYawFixed )
        {
            Vector3 xVec = mYawFixedAxis.crossProduct( zAdjustVec );
            xVec.normalise();

            Vector3 yVec = zAdjustVec.crossProduct( xVec );
            yVec.normalise();
            
            targetOrientation.FromAxes( xVec, yVec, zAdjustVec );
        }
        else
        {

            // Get axes from current quaternion
            Vector3 axes[3];
            _getDerivedOrientation().ToAxes(axes);
            Quaternion rotQuat;
            if (-zAdjustVec == axes[2])
            {
                // Oops, a 180 degree turn (infinite possible rotation axes)
                // Default to yaw i.e. use current UP
                rotQuat.FromAngleAxis(Math::PI, axes[1]);
            }
            else
            {
                // Derive shortest arc to new direction
                rotQuat = axes[2].getRotationTo(zAdjustVec);

            }
            targetOrientation = rotQuat * mOrientation;
        }

        if (relativeTo == TS_LOCAL || !mParent)
        {
            mOrientation = targetOrientation;
        }
        else
        {
            if (relativeTo == TS_PARENT)
            {
                mOrientation = targetOrientation * mParent->getOrientation().Inverse();
            }
            else if (relativeTo == TS_WORLD)
            {
                mOrientation = targetOrientation * mParent->_getDerivedOrientation().Inverse();
            }
        }


    }
    //-----------------------------------------------------------------------
    void SceneNode::lookAt( const Vector3& targetPoint, TransformSpace relativeTo, 
        const Vector3& localDirectionVector)
    {
        this->setDirection(targetPoint - _getDerivedPosition(), relativeTo, 
            localDirectionVector);
    }
    //-----------------------------------------------------------------------
    void SceneNode::_autoTrack(void)
    {
        // NB assumes that all scene nodes have been updated
        if (mAutoTrackTarget)
        {
            lookAt(mAutoTrackTarget->_getDerivedPosition() + mAutoTrackOffset, 
                TS_WORLD, mAutoTrackLocalDirection);
            // update self & children
            _update(true, true);
        }
    }


}

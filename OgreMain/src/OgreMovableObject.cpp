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

#include "OgreMovableObject.h"
#include "OgreSceneNode.h"
#include "OgreTagPoint.h"
#include "OgreLight.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    MovableObject::MovableObject()
    {
        mParentNode = 0;
        mVisible = true;
        mUserObject = 0;
        mRenderQueueID = RENDER_QUEUE_MAIN;
        mQueryFlags = 0xFFFFFFFF;
        mWorldAABB.setNull();
        mParentIsTagPoint = false;
        mCastShadows = true;
    }
    //-----------------------------------------------------------------------
    MovableObject::~MovableObject()
    {
        if (mParentNode && !mParentIsTagPoint)
        {
            // detach from parent
            static_cast<SceneNode*>(mParentNode)->detachObject(this);
        }
    }
    //-----------------------------------------------------------------------
    void MovableObject::_notifyAttached(Node* parent, bool isTagPoint)
    {
        mParentNode = parent;
        mParentIsTagPoint = isTagPoint;
    }
    //-----------------------------------------------------------------------
    Node* MovableObject::getParentNode(void) const
    {
        return mParentNode;
    }
    //-----------------------------------------------------------------------
    bool MovableObject::isAttached(void) const
    {
        return (mParentNode != 0);

    }
    //-----------------------------------------------------------------------
    void MovableObject::setVisible(bool visible)
    {
        mVisible = visible;
    }
    //-----------------------------------------------------------------------
    bool MovableObject::isVisible(void) const
    {
        return mVisible;

    }
    //-----------------------------------------------------------------------
    void MovableObject::setRenderQueueGroup(RenderQueueGroupID queueID)
    {
        mRenderQueueID = queueID;
    }
    //-----------------------------------------------------------------------
    RenderQueueGroupID MovableObject::getRenderQueueGroup(void) const
    {
        return mRenderQueueID;
    }
    //-----------------------------------------------------------------------
	Matrix4 MovableObject::_getParentNodeFullTransform(void) const
	{
		
		if(mParentNode)
		{
			// object attached to a sceneNode
			return mParentNode->_getFullTransform();
		}
        // fallback
        return Matrix4::IDENTITY;
	}
    //-----------------------------------------------------------------------
    const AxisAlignedBox& MovableObject::getWorldBoundingBox(bool derive) const
    {
        if (derive)
        {
            mWorldAABB = this->getBoundingBox();
            mWorldAABB.transform(_getParentNodeFullTransform());
        }

        return mWorldAABB;

    }
    //-----------------------------------------------------------------------
	const Sphere& MovableObject::getWorldBoundingSphere(bool derive) const
	{
		if (derive)
		{
			mWorldBoundingSphere.setRadius(getBoundingRadius());
			mWorldBoundingSphere.setCenter(mParentNode->_getDerivedPosition());
		}
		return mWorldBoundingSphere;
	}

    //-----------------------------------------------------------------------
    const AxisAlignedBox& MovableObject::getLightCapBounds(void) 
    {
        // Same as original bounds
        return getWorldBoundingBox();
    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& MovableObject::getDarkCapBounds(const Light& light) 
    {
        // Extrude own light cap bounds
        mWorldDarkCapBounds = getLightCapBounds();
        this->extrudeBounds(mWorldDarkCapBounds, light.getAs4DVector(), 
            light.getAttenuationRange());
        return mWorldDarkCapBounds;

    }


}


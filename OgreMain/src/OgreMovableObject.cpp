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

#include "OgreMovableObject.h"
#include "OgreSceneNode.h"
#include "OgreTagPoint.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    MovableObject::MovableObject()
    {
        mParentNode = 0;
        mVisible = true;
        mUserObject = 0;
		mAttachingPoint = 0;
        mRenderQueueID = RENDER_QUEUE_MAIN;
    }
    //-----------------------------------------------------------------------
    void MovableObject::_notifyAttached(SceneNode* parent)
    {
        // Cannot be attached to a bone too.
        assert(!mAttachingPoint);

        mParentNode = parent;
    }
    //-----------------------------------------------------------------------
    SceneNode* MovableObject::getParentNode(void)
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
    RenderQueueGroupID MovableObject::getRenderQueueGroup(void)
    {
        return mRenderQueueID;
    }
    //-----------------------------------------------------------------------
	Matrix4 MovableObject::_getParentNodeFullTransform(void)
	{
		
		if(mParentNode)
		{
			// object attached to a sceneNode
			return mParentNode->_getFullTransform();
		}
		else if(mAttachingPoint)
		{
			// object is attached to an attachingPoint (tagPoint)
			return mAttachingPoint->_getFullTransform();
		}

        // fallback
        return Matrix4::IDENTITY;
	}
    //-----------------------------------------------------------------------
    TagPoint* MovableObject::getAttachmentPoint(void)
    {
        return mAttachingPoint;
    }
    //-----------------------------------------------------------------------
    bool MovableObject::isAttached(void)
    {
        return (mParentNode != 0 || mAttachingPoint != 0);
    }
    //-----------------------------------------------------------------------
    void MovableObject::_notifyAttached(TagPoint* parent)
    {
        // Cannot be attached to a scene node too.
        assert(!mParentNode);

        mAttachingPoint = parent;

    }


}


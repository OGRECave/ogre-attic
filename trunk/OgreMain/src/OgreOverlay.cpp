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

#include "OgreOverlay.h"
#include "OgreRoot.h"
#include "OgreSceneManager.h"
#include "OgreGuiContainer.h"
#include "OgreCamera.h"


namespace Ogre {

    //---------------------------------------------------------------------
    Overlay::Overlay(const String& name)
    {
        mName = name;
        mRotate = 0.0f;
        mScaleX = 1.0f;
        mScaleY = 1.0f;
        mScrollX = 0.0f;
        mScrollY = 0.0f;
        mVisible = false;
        mTransformOutOfDate = true;
        mZOrder = 100; // Default
        mRootNode = Root::getSingleton().getSceneManager(ST_GENERIC)->createSceneNode();

    }
    //---------------------------------------------------------------------
    Overlay::~Overlay()
    {
        mRootNode->getCreator()->destroySceneNode(mRootNode->getName());
    }
    //---------------------------------------------------------------------
    const String& Overlay::getName(void) const
    {
        return mName;
    }
    //---------------------------------------------------------------------
    void Overlay::setZOrder(ushort zorder)
    {
        mZOrder = zorder;

        // Notify attached 2D elements
        GuiContainerList::iterator i, iend;
        iend = m2DElements.end();
        for (i = m2DElements.begin(); i != iend; ++i)
        {
            (*i)->_notifyZOrder(zorder);
        }

    }
    //---------------------------------------------------------------------
    ushort Overlay::getZOrder(void) const
    {
        return mZOrder;
    }
    //---------------------------------------------------------------------
    bool Overlay::isVisible(void)
    {
        return mVisible;
    }
    //---------------------------------------------------------------------
    void Overlay::show(void)
    {
        mVisible = true;
    }
    //---------------------------------------------------------------------
    void Overlay::hide(void)
    {
        mVisible = false;
    }
    //---------------------------------------------------------------------
    void Overlay::add2D(GuiContainer* cont)
    {
        m2DElements.push_back(cont);
        // Notify parent
        cont->_notifyParent(0, this);
        // Set Z order, scaled to separate overlays
        // NB max 100 container levels per overlay, should be plenty
        cont->_notifyZOrder(mZOrder * 100);
    }
    //---------------------------------------------------------------------
    void Overlay::remove2D(GuiContainer* cont)
    {
        m2DElements.remove(cont);
    }
    //---------------------------------------------------------------------
    void Overlay::add3D(SceneNode* node)
    {
        mRootNode->addChild(node);
    }
    //---------------------------------------------------------------------
    void Overlay::remove3D(SceneNode* node)
    {
        mRootNode->removeChild(node->getName());
    }
    //---------------------------------------------------------------------
    void Overlay::clear(void)
    {
        mRootNode->removeAllChildren();
        m2DElements.clear();
        // Note no deallocation, memory handled by GuiManager & SceneManager
    }
    //---------------------------------------------------------------------
    void Overlay::setScroll(Real x, Real y)
    {
        mScrollX = x;
        mScrollY = y;
        mTransformOutOfDate = true;
    }
    //---------------------------------------------------------------------
    Real Overlay::getScrollX(void)
    {
        return mScrollX;
    }
    //---------------------------------------------------------------------
    Real Overlay::getScrollY(void)
    {
        return mScrollY;
    }
      //---------------------------------------------------------------------
    GuiContainer* Overlay::getChild(const String& name)
    {

        GuiContainerList::iterator i, iend;
        iend = m2DElements.end();
        for (i = m2DElements.begin(); i != iend; ++i)
        {
            if ((*i)->getName() == name)
			{
				return *i;

			}
        }
        return NULL;
    }
  //---------------------------------------------------------------------
    void Overlay::scroll(Real xoff, Real yoff)
    {
        mScrollX += xoff;
        mScrollY += yoff;
        mTransformOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void Overlay::setRotate(Real degrees)
    {
        mRotate = degrees;
        mTransformOutOfDate = true;
    }
    //---------------------------------------------------------------------
    Real Overlay::getRotate(void)
    {
        return mRotate;
    }
    //---------------------------------------------------------------------
    void Overlay::rotate(Real degrees)
    {
        setRotate(mRotate += degrees);
    }
    //---------------------------------------------------------------------
    void Overlay::setScale(Real x, Real y)
    {
        mScaleX = x;
        mScaleY = y;
        mTransformOutOfDate = true;
    }
    //---------------------------------------------------------------------
    Real Overlay::getScaleX(void)
    {
        return mScaleX;
    }
    //---------------------------------------------------------------------
    Real Overlay::getScaleY(void)
    {
        return mScaleY;
    }
    //---------------------------------------------------------------------
    void Overlay::_getWorldTransforms(Matrix4* xform)
    {
        if (mTransformOutOfDate)
        {
            updateTransform();
        }
        *xform = mTransform;

    }
    //---------------------------------------------------------------------
    void Overlay::_findVisibleObjects(Camera* cam, RenderQueue* queue)
    {
        if (!mVisible)
            return;

        // Add 3D elements
        mRootNode->setPosition(cam->getDerivedPosition());
        mRootNode->setOrientation(cam->getDerivedOrientation());
        mRootNode->_update(cam);
        // Set up the default queue group for the objects about to be added
        RenderQueueGroupID oldgrp = queue->getDefaultQueueGroup();
        queue->setDefaultQueueGroup(RENDER_QUEUE_OVERLAY);
        mRootNode->_findVisibleObjects(cam, queue, true, false);
        // Reset the group
        queue->setDefaultQueueGroup(oldgrp);


        // Add 2D elements
        GuiContainerList::iterator i, iend;
        iend = m2DElements.end();
        for (i = m2DElements.begin(); i != iend; ++i)
        {
            (*i)->_update();

            (*i)->_updateRenderQueue(queue);
        }
       
    }
    //---------------------------------------------------------------------
    void Overlay::updateTransform(void)
    {
        // Ordering:
        //    1. Scale
        //    2. Rotate
        //    3. Translate

        
        Matrix3 rot3x3, scale3x3;
        rot3x3.FromEulerAnglesXYZ(0,0,Math::DegreesToRadians(mRotate));
        scale3x3 = Matrix3::ZERO;
        scale3x3[0][0] = mScaleX;
        scale3x3[1][1] = mScaleY;
        scale3x3[2][2] = 1.0f;

        mTransform = Matrix4::IDENTITY;
        mTransform = rot3x3 * scale3x3;
        mTransform.setTrans(Vector3(mScrollX, mScrollY, 0));

        mTransformOutOfDate = false;
    }
    //---------------------------------------------------------------------
    void Overlay::load(void)
    {
        // Do nothing
    }
    //---------------------------------------------------------------------
    void Overlay::unload(void)
    {
        // Do nothing
    }

	GuiElement* Overlay::findElementAt(Real x, Real y)
	{
		GuiElement* ret = NULL;
		int currZ = -1;
        GuiContainerList::iterator i, iend;
        iend = m2DElements.end();
        for (i = m2DElements.begin(); i != iend; ++i)
        {
			int z = (*i)->getZOrder();
			if (z > currZ)
			{
				GuiElement* elementFound = (*i)->findElementAt(x,y);
				if(elementFound)
				{
					currZ = z;
					ret = elementFound;
				}
			}
        }
		return ret;
	}
}


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
#include "OgreViewport.h"

#include "OgreLogManager.h"
#include "OgreRenderTarget.h"
#include "OgreCamera.h"
#include "OgreMath.h"

namespace Ogre {
    //---------------------------------------------------------------------
    Viewport::Viewport(Camera* cam, RenderTarget* target, Real left, Real top, Real width, Real height, int ZOrder)
    {
        char msg[200];


        sprintf(msg, "Creating viewport on target '%s', rendering from camera "
            "'%s', relative dimensions L:%.2f,T:%.2f,W:%.2f,H:%.2f, Z-Order:%d",
            target->getName().c_str(), cam->getName().c_str(),
            left, top, width, height, ZOrder);
        LogManager::getSingleton().logMessage(msg);
        mCamera = cam;
        mTarget = target;

        mRelLeft = left;
        mRelTop = top;
        mRelWidth = width;
        mRelHeight = height;
        mZOrder = ZOrder;

        mBackColour = ColourValue::Black;
        mClearEveryFrame = true;


        // Calculate actual dimensions
        _updateDimensions();

        mUpdated = true;
        mShowOverlays = true;
    }
    //---------------------------------------------------------------------
    Viewport::~Viewport()
    {

    }
    //---------------------------------------------------------------------
    bool Viewport::_isUpdated(void) const
    {
        return mUpdated;
    }
    //---------------------------------------------------------------------
    void Viewport::_clearUpdatedFlag(void)
    {
        mUpdated = false;
    }
    //---------------------------------------------------------------------
    void Viewport::_updateDimensions(void)
    {
        Real height = (Real) mTarget->getHeight();
        Real width = (Real) mTarget->getWidth();

        mActLeft = (int) (mRelLeft * width);
        mActTop = (int) (mRelTop * height);
        mActWidth = (int) (mRelWidth * width);
        mActHeight = (int) (mRelHeight * height);

        // Note that we don't propagate any changes to the Camera
        // This is because the Camera projects into a space with
        // range (-1,1), which then gets extrapolated to the viewport
        // dimensions. Note that if the aspect ratio of the camera
        // is not the same as that of the viewport, the image will
        // be distorted in some way.

        // This allows cameras to be used to render to many viewports,
        // which can have their own dimensions and aspect ratios.


        char msg[256];

        sprintf(msg, "Viewport for camera '%s' - actual dimensions L:%d,T:%d,W:%d,H:%d",
            mCamera->getName().c_str(), mActLeft, mActTop, mActWidth, mActHeight);
        LogManager::getSingleton().logMessage(msg);

        mUpdated = true;
    }
    //---------------------------------------------------------------------
    RenderTarget* Viewport::getTarget(void) const
    {
        return mTarget;
    }
    //---------------------------------------------------------------------
    Camera* Viewport::getCamera(void) const
    {
        return mCamera;
    }
    //---------------------------------------------------------------------
    Real Viewport::getLeft(void) const
    {
        return mRelLeft;
    }
    //---------------------------------------------------------------------
    Real Viewport::getTop(void) const
    {
        return mRelTop;
    }
    //---------------------------------------------------------------------
    Real Viewport::getWidth(void) const
    {
        return mRelWidth;
    }
    //---------------------------------------------------------------------
    Real Viewport::getHeight(void) const
    {
        return mRelHeight;
    }
    //---------------------------------------------------------------------
    int Viewport::getActualLeft(void) const
    {
        return mActLeft;
    }
    //---------------------------------------------------------------------
    int Viewport::getActualTop(void) const
    {
        return mActTop;
    }
    //---------------------------------------------------------------------
    int Viewport::getActualWidth(void) const
    {
        return mActWidth;
    }
    //---------------------------------------------------------------------
    int Viewport::getActualHeight(void) const
    {
        return mActHeight;
    }
    //---------------------------------------------------------------------
    void Viewport::setDimensions(Real left, Real top, Real width, Real height)
    {
        mRelLeft = left;
        mRelTop = top;
        mRelWidth = width;
        mRelHeight = height;
        _updateDimensions();
    }
    //---------------------------------------------------------------------
    void Viewport::update(void)
    {
        // Tell Camera to render into me
        mCamera->_renderScene(this, mShowOverlays);
    }
    //---------------------------------------------------------------------
    void Viewport::setBackgroundColour(ColourValue colour)
    {
        mBackColour = colour;
    }
    //---------------------------------------------------------------------
    const ColourValue& Viewport::getBackgroundColour(void) const
    {
        return mBackColour;
    }
    //---------------------------------------------------------------------
    void Viewport::setClearEveryFrame(bool clear)
    {
        mClearEveryFrame = clear;
    }
    //---------------------------------------------------------------------
    bool Viewport::getClearEveryFrame(void) const
    {
        return mClearEveryFrame;
    }
    //---------------------------------------------------------------------
    void Viewport::getActualDimensions(int &left, int&top, int &width, int &height) const
    {
        left = mActLeft;
        top = mActTop;
        width = mActWidth;
        height = mActHeight;

    }
    //---------------------------------------------------------------------
    unsigned int Viewport::_getNumRenderedFaces(void) const
    {
        return mCamera->_getNumRenderedFaces();
    }
    //---------------------------------------------------------------------
    void Viewport::setCamera(Camera* cam)
    {
        mCamera = cam;

    }
    //---------------------------------------------------------------------
    void Viewport::setOverlaysEnabled(bool enabled)
    {
        mShowOverlays = enabled;
    }
    //---------------------------------------------------------------------
    bool Viewport::getOverlaysEnabled(void)
    {
        return mShowOverlays;
    }
}

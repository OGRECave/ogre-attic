/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
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

#include "OgreAutoParamDataSource.h"
#include "OgreRenderable.h"
#include "OgreCamera.h"

namespace Ogre {

    //-----------------------------------------------------------------------------
    AutoParamDataSource::AutoParamDataSource()
        : mCurrentRenderable(NULL), mCurrentCamera(NULL), 
         mWorldMatrixDirty(true),
         mWorldViewMatrixDirty(true),
         mInverseWorldMatrixDirty(true),
         mInverseWorldViewMatrixDirty(true),
         mInverseViewMatrixDirty(true),
         mCameraPositionObjectSpaceDirty(true)
    {
    }
    //-----------------------------------------------------------------------------
    AutoParamDataSource::~AutoParamDataSource()
    {
    }
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setCurrentRenderable(const Renderable* rend)
    {
        mCurrentRenderable = rend;
        mWorldMatrixDirty = true;
        mWorldViewMatrixDirty = true;
        mInverseWorldMatrixDirty = true;
        mInverseWorldViewMatrixDirty = true;
        mCameraPositionObjectSpaceDirty = true;
    }
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setCurrentCamera(const Camera* cam)
    {
        mCurrentCamera = cam;
        mWorldViewMatrixDirty = true;
        mInverseViewMatrixDirty = true;
        mInverseWorldViewMatrixDirty = true;
        mCameraPositionObjectSpaceDirty = true;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getWorldMatrix(void) const
    {
        if (mWorldMatrixDirty)
        {
            mCurrentRenderable->getWorldTransforms(mWorldMatrix);
            mWorldMatrixDirty = false;
        }
        return mWorldMatrix[0];
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getViewMatrix(void) const
    {
        return mCurrentCamera->getViewMatrix();
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getWorldViewMatrix(void) const
    {
        if (mWorldViewMatrixDirty)
        {
            mWorldViewMatrix = getViewMatrix() * getWorldMatrix();
            mWorldViewMatrixDirty = false;
        }
        return mWorldViewMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getInverseWorldMatrix(void) const
    {
        if (mInverseWorldMatrixDirty)
        {
            mInverseWorldMatrix = getWorldMatrix().inverse();
            mInverseWorldMatrixDirty = false;
        }
        return mInverseWorldMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getInverseWorldViewMatrix(void) const
    {
        if (mInverseWorldViewMatrixDirty)
        {
            mInverseWorldViewMatrix = getWorldViewMatrix().inverse();
            mInverseWorldViewMatrixDirty = false;
        }
        return mInverseWorldViewMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getInverseViewMatrix(void) const
    {
        if (mInverseViewMatrixDirty)
        {
            mInverseViewMatrix = getViewMatrix().inverse();
            mInverseViewMatrixDirty = false;
        }
        return mInverseViewMatrix;
    }
    //-----------------------------------------------------------------------------
    const Vector4& AutoParamDataSource::getCameraPositionObjectSpace(void) const
    {
        if (mCameraPositionObjectSpaceDirty)
        {
            mCameraPositionObjectSpace = 
                getInverseWorldMatrix() * mCurrentCamera->getDerivedPosition();
            mCameraPositionObjectSpaceDirty = false;
        }
        return mCameraPositionObjectSpace;
    }

}


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
#include "OgreRenderTarget.h"

namespace Ogre {
    const Matrix4 PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE(
        0.5,    0,  0, -0.5, 
        0, -0.5,  0, -0.5, 
        0,    0,  0,   1,
        0,    0,  0,   1);

    //-----------------------------------------------------------------------------
    AutoParamDataSource::AutoParamDataSource()
        : mWorldMatrixDirty(true),
         mWorldViewMatrixDirty(true),
         mViewProjMatrixDirty(true),
         mWorldViewProjMatrixDirty(true),
         mInverseWorldMatrixDirty(true),
         mInverseWorldViewMatrixDirty(true),
         mInverseViewMatrixDirty(true),
         mCameraPositionObjectSpaceDirty(true),
         mTextureViewProjMatrixDirty(true),
         mCurrentRenderable(NULL),
         mCurrentCamera(NULL), 
         mCurrentTextureProjector(NULL), 
         mCurrentRenderTarget(NULL)
    {
        mBlankLight.setDiffuseColour(ColourValue::Black);
        mBlankLight.setSpecularColour(ColourValue::Black);
        mBlankLight.setAttenuation(0,0,0,0);
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
        mViewProjMatrixDirty = true;
		mWorldViewProjMatrixDirty = true;
		mInverseWorldMatrixDirty = true;
		mInverseWorldViewMatrixDirty = true;
		mCameraPositionObjectSpaceDirty = true;
    }
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setCurrentCamera(const Camera* cam)
    {
        mCurrentCamera = cam;
        mWorldViewMatrixDirty = true;
        mViewProjMatrixDirty = true;
        mWorldViewProjMatrixDirty = true;
        mInverseViewMatrixDirty = true;
        mInverseWorldViewMatrixDirty = true;
        mCameraPositionObjectSpaceDirty = true;
    }
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setCurrentLightList(const LightList* ll)
    {
        mCurrentLightList = ll;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getWorldMatrix(void) const
    {
        if (mWorldMatrixDirty)
        {
            mCurrentRenderable->getWorldTransforms(mWorldMatrix);
            mWorldMatrixCount = mCurrentRenderable->getNumWorldTransforms();
            mWorldMatrixDirty = false;
        }
        return mWorldMatrix[0];
    }
    //-----------------------------------------------------------------------------
    size_t AutoParamDataSource::getWorldMatrixCount(void) const
    {
        if (mWorldMatrixDirty)
        {
            mCurrentRenderable->getWorldTransforms(mWorldMatrix);
            mWorldMatrixCount = mCurrentRenderable->getNumWorldTransforms();
            mWorldMatrixDirty = false;
        }
        return mWorldMatrixCount;
    }
    //-----------------------------------------------------------------------------
    const Matrix4* AutoParamDataSource::getWorldMatrixArray(void) const
    {
        if (mWorldMatrixDirty)
        {
            mCurrentRenderable->getWorldTransforms(mWorldMatrix);
            mWorldMatrixCount = mCurrentRenderable->getNumWorldTransforms();
            mWorldMatrixDirty = false;
        }
        return mWorldMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getViewMatrix(void) const
    {
        return mCurrentCamera->getViewMatrix();
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getViewProjectionMatrix(void) const
    {
        if (mViewProjMatrixDirty)
        {
            mViewProjMatrix = getProjectionMatrix() * getViewMatrix();
            mViewProjMatrixDirty = false;
        }
        return mViewProjMatrix;
    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getProjectionMatrix(void) const
    {
        // NB use API-independent projection matrix since GPU programs
        // bypass the API-specific handedness and use right-handed coords
        mProjectionMatrix = mCurrentCamera->getStandardProjectionMatrix();
        if (mCurrentRenderTarget && mCurrentRenderTarget->requiresTextureFlipping())
        {
            // Because we're not using setProjectionMatrix, this needs to be done here
            mProjectionMatrix[1][1] = -mProjectionMatrix[1][1];
        }
        return mProjectionMatrix;
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
    const Matrix4& AutoParamDataSource::getWorldViewProjMatrix(void) const
    {
        if (mWorldViewProjMatrixDirty)
        {
            mWorldViewProjMatrix = getProjectionMatrix() * getWorldViewMatrix();
            mWorldViewProjMatrixDirty = false;
        }
        return mWorldViewProjMatrix;
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
    //-----------------------------------------------------------------------------
    const Light& AutoParamDataSource::getLight(size_t index) const
    {
        // If outside light range, return a blank light to ensure zeroised for program
        if (mCurrentLightList->size() <= index)
        {
            return mBlankLight;
        }
        else
        {
            return *((*mCurrentLightList)[index]);
        }
    }
    //-----------------------------------------------------------------------------
	void AutoParamDataSource::setAmbientLightColour(const ColourValue& ambient)
	{
		mAmbientLight = ambient;
	}
    //-----------------------------------------------------------------------------
	const ColourValue& AutoParamDataSource::getAmbientLightColour(void) const
	{
		return mAmbientLight;
		
	}
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setTextureProjector(const Frustum* frust)
    {
        mCurrentTextureProjector = frust;
        mTextureViewProjMatrixDirty = true;

    }
    //-----------------------------------------------------------------------------
    const Matrix4& AutoParamDataSource::getTextureViewProjMatrix(void) const
    {
        if (mTextureViewProjMatrixDirty)
        {
            mTextureViewProjMatrix = 
                PROJECTIONCLIPSPACE2DTOIMAGESPACE_PERSPECTIVE * 
                mCurrentTextureProjector->getViewMatrix() * 
                mCurrentTextureProjector->getStandardProjectionMatrix();
            mTextureViewProjMatrixDirty = false;
        }
        return mTextureViewProjMatrix;
    }
    //-----------------------------------------------------------------------------
    void AutoParamDataSource::setCurrentRenderTarget(const RenderTarget* target)
    {
        mCurrentRenderTarget = target;
    }
    //-----------------------------------------------------------------------------
    const RenderTarget* AutoParamDataSource::getCurrentRenderTarget(void) const
    {
        return mCurrentRenderTarget;
    }

}


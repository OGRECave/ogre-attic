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
// RenderSystem implementation
// Note that most of this class is abstract since
//  we cannot know how to implement the behaviour without
//  being aware of the 3D API. However there are a few
//  simple functions which can have a base implementation

#include "OgreRenderSystem.h"

#include "OgreRoot.h"
#include "OgreViewport.h"
#include "OgreException.h"
#include "OgreRenderTarget.h"
#include "OgreRenderWindow.h"
#include "OgreMeshManager.h"
#include "OgreMaterial.h"
#include "OgreTimer.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    RenderSystem::RenderSystem()
    {
        mActiveViewport = 0;
		mActiveRenderTarget = NULL;
        mTextureManager = 0;
        mCapabilities = 0;
        mVSync = true;


        // This means CULL clockwise vertices, i.e. front of poly is counter-clockwise
        // This makes it the same as OpenGL and other right-handed systems
        mCullingMode = CULL_CLOCKWISE;
        mInvertVertexWinding = false;

        // instanciate RenderSystemCapabilities
        mCapabilities = new RenderSystemCapabilities();
    }

    //-----------------------------------------------------------------------
    RenderSystem::~RenderSystem()
    {
        shutdown();
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_initRenderTargets(void)
    {

        // Init stats
        for(
            RenderTargetMap::iterator it = mRenderTargets.begin();
            it != mRenderTargets.end();
            ++it )
        {
            it->second->resetStatistics();
        }

    }
    //-----------------------------------------------------------------------
    void RenderSystem::_updateAllRenderTargets(void)
    {
        // Update all in order of priority
        // This ensures render-to-texture targets get updated before render windows
		RenderTargetPriorityMap::iterator itarg, itargend;
		itargend = mPrioritisedRenderTargets.end();
		for( itarg = mPrioritisedRenderTargets.begin(); itarg != itargend; ++itarg )
		{
			if( itarg->second->isActive() )
				itarg->second->update();
		}
    }
    //-----------------------------------------------------------------------
    RenderWindow* RenderSystem::initialise(bool autoCreateWindow)
    {
        // Have I been registered by call to Root::setRenderSystem?
		/** Don't do this anymore, just allow via Root
        RenderSystem* regPtr = Root::getSingleton().getRenderSystem();
        if (!regPtr || regPtr != this)
            // Register self - library user has come to me direct
            Root::getSingleton().setRenderSystem(this);
		*/


        // Subclasses should take it from here
        // They should ALL call this superclass method from
        //   their own initialise() implementations.

        return 0;
    }
    //---------------------------------------------------------------------------------------------
    void RenderSystem::attachRenderTarget( RenderTarget &target )
    {
		assert( target.getPriority() < OGRE_NUM_RENDERTARGET_GROUPS );

        mRenderTargets.insert( RenderTargetMap::value_type( target.getName(), &target ) );
        mPrioritisedRenderTargets.insert(
            RenderTargetPriorityMap::value_type(target.getPriority(), &target ));
    }

    //---------------------------------------------------------------------------------------------
    RenderTarget * RenderSystem::getRenderTarget( const String &name )
    {
        RenderTargetMap::iterator it = mRenderTargets.find( name );
        RenderTarget *ret = NULL;

        if( it != mRenderTargets.end() )
        {
            ret = it->second;
        }

        return ret;
    }

    //---------------------------------------------------------------------------------------------
    RenderTarget * RenderSystem::detachRenderTarget( const String &name )
    {
        RenderTargetMap::iterator it = mRenderTargets.find( name );
        RenderTarget *ret = NULL;

        if( it != mRenderTargets.end() )
        {
            ret = it->second;
			
			/* Remove the render target from the priority groups. */
            RenderTargetPriorityMap::iterator itarg, itargend;
            itargend = mPrioritisedRenderTargets.end();
			for( itarg = mPrioritisedRenderTargets.begin(); itarg != itargend; ++itarg )
            {
				if( itarg->second == ret ) {
					mPrioritisedRenderTargets.erase( itarg );
					break;
				}
            }

            mRenderTargets.erase( it );
        }

        return ret;
    }
    //-----------------------------------------------------------------------
    Viewport* RenderSystem::_getViewport(void)
    {
        return mActiveViewport;
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_setTextureUnitSettings(size_t texUnit, TextureUnitState& tl)
    {
        // This method is only ever called to set a texture unit to valid details
        // The method _disableTextureUnit is called to turn a unit off

        // Texture name
        _setTexture(texUnit, true, tl.getTextureName());

        // Set texture coordinate set
        _setTextureCoordSet(texUnit, tl.getTextureCoordSet());

        // Set texture layer filtering
        _setTextureUnitFiltering(texUnit, 
            tl.getTextureFiltering(FT_MIN), 
            tl.getTextureFiltering(FT_MAG), 
            tl.getTextureFiltering(FT_MIP));

        // Set texture layer filtering
        _setTextureLayerAnisotropy(texUnit, tl.getTextureAnisotropy());

		// Set blend modes
        _setTextureBlendMode(texUnit, tl.getColourBlendMode());
        _setTextureBlendMode(texUnit, tl.getAlphaBlendMode());

        // Texture addressing mode
        _setTextureAddressingMode(texUnit, tl.getTextureAddressingMode() );

        // Set texture effects
        TextureUnitState::EffectMap::iterator effi;
        // Iterate over new effects
        bool anyCalcs = false;
        for (effi = tl.mEffects.begin(); effi != tl.mEffects.end(); ++effi)
        {
            switch (effi->second.type)
            {
            case TextureUnitState::ET_ENVIRONMENT_MAP:
                if (effi->second.subtype == TextureUnitState::ENV_CURVED)
                {
                    _setTextureCoordCalculation(texUnit, TEXCALC_ENVIRONMENT_MAP);
                    anyCalcs = true;
                }
                else if (effi->second.subtype == TextureUnitState::ENV_PLANAR)
                {
                    _setTextureCoordCalculation(texUnit, TEXCALC_ENVIRONMENT_MAP_PLANAR);
                    anyCalcs = true;
                }
                else if (effi->second.subtype == TextureUnitState::ENV_REFLECTION)
                {
                    _setTextureCoordCalculation(texUnit, TEXCALC_ENVIRONMENT_MAP_REFLECTION);
                    anyCalcs = true;
                }
                else if (effi->second.subtype == TextureUnitState::ENV_NORMAL)
                {
                    _setTextureCoordCalculation(texUnit, TEXCALC_ENVIRONMENT_MAP_NORMAL);
                    anyCalcs = true;
                }
                break;
	    case TextureUnitState::ET_SCROLL:
	    case TextureUnitState::ET_ROTATE:
	    case TextureUnitState::ET_TRANSFORM:
	      break;
            }
        }
        // Ensure any previous texcoord calc settings are reset if there are now none
        if (!anyCalcs)
        {
            _setTextureCoordCalculation(texUnit, TEXCALC_NONE);
            _setTextureCoordSet(texUnit, tl.getTextureCoordSet());
        }

        // Change tetxure matrix 
        _setTextureMatrix(texUnit, tl.getTextureTransform());

        // Set alpha rejection
        _setAlphaRejectSettings(tl.getAlphaRejectFunction(), 
            tl.getAlphaRejectValue());

    }
    //-----------------------------------------------------------------------
    void RenderSystem::_disableTextureUnit(size_t texUnit)
    {
        _setTexture(texUnit, false, "");
    }
    //---------------------------------------------------------------------
    void RenderSystem::_disableTextureUnitsFrom(size_t texUnit)
    {
        for (size_t i = texUnit; i < mCapabilities->getNumTextureUnits(); ++i)
        {
            _disableTextureUnit(i);
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_setTextureUnitFiltering(size_t unit, FilterOptions minFilter,
            FilterOptions magFilter, FilterOptions mipFilter)
    {
        _setTextureUnitFiltering(unit, FT_MIN, minFilter);
        _setTextureUnitFiltering(unit, FT_MAG, magFilter);
        _setTextureUnitFiltering(unit, FT_MIP, mipFilter);
    }
    //-----------------------------------------------------------------------
    CullingMode RenderSystem::_getCullingMode(void) const
    {
        return mCullingMode;
    }
    //-----------------------------------------------------------------------
    bool RenderSystem::getWaitForVerticalBlank(void) const
    {
        return mVSync;
    }
    //-----------------------------------------------------------------------
    void RenderSystem::setWaitForVerticalBlank(bool enabled)
    {
        mVSync = enabled;
    }
    //-----------------------------------------------------------------------
    void RenderSystem::shutdown(void)
    {
        // Remove all the render targets.
        for( RenderTargetMap::iterator it = mRenderTargets.begin(); it != mRenderTargets.end(); ++it )
        {
            delete it->second;
        }
		mRenderTargets.clear();

		mPrioritisedRenderTargets.clear();
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_beginGeometryCount(void)
    {
        mFaceCount = mVertexCount = 0;

    }
    //-----------------------------------------------------------------------
    unsigned int RenderSystem::_getFaceCount(void) const
    {
        return static_cast< unsigned int >( mFaceCount );
    }
    //-----------------------------------------------------------------------
    unsigned int RenderSystem::_getVertexCount(void) const
    {
        return static_cast< unsigned int >( mVertexCount );
    }
    //-----------------------------------------------------------------------
    /*
    bool RenderSystem::_isVertexBlendSupported(void)
    {
        // TODO: implement vertex blending support in DX8 & possibly GL_ARB_VERTEX_BLEND (in subclasses)
        // DX7 support not good enough - only 4 matrices supported
        return false;
    }
    */
    //-----------------------------------------------------------------------
    void RenderSystem::softwareVertexBlend(VertexData* vertexData, Matrix4* pMatrices)
    {
        // Source vectors
        Vector3 sourceVec, sourceNorm;
        // Accumulation vectors
        Vector3 accumVecPos, accumVecNorm;
        Matrix3 rot3x3;

        Real *pSrcPos, *pSrcNorm, *pDestPos, *pDestNorm, *pBlendWeight;
        unsigned char* pBlendIdx;
        bool posNormShareBuffer = false;

        const VertexElement* elemPos = 
            vertexData->vertexDeclaration->findElementBySemantic(VES_POSITION);
        const VertexElement* elemNorm = 
            vertexData->vertexDeclaration->findElementBySemantic(VES_NORMAL);
        
        HardwareVertexBufferSharedPtr posBuf, normBuf;
        posBuf = vertexData->vertexBufferBinding->getBuffer(elemPos->getSource());
        if (elemNorm)
        {
            normBuf = vertexData->vertexBufferBinding->getBuffer(elemNorm->getSource());
            posNormShareBuffer = (posBuf.get() == normBuf.get());
        }
        // Lock buffers for writing
        assert (elemPos->getOffset() == 0 && 
            "Positions must be first element in dedicated buffer!");
        pDestPos = static_cast<Real*>(
            posBuf->lock(HardwareBuffer::HBL_DISCARD));
        if (elemNorm)
        {
            if (posNormShareBuffer)
            {
                // Same buffer, must be packed directly after position
                assert (elemNorm->getOffset() == sizeof(Real) * 3 && 
                    "Normals must be packed directly after positions in buffer!");
                // pDestNorm will not be used
            }
            else
            {
                // Different buffer
                assert (elemNorm->getOffset() == 0 && 
                    "Normals must be first element in dedicated buffer!");
                pDestNorm = static_cast<Real*>(
                    normBuf->lock(HardwareBuffer::HBL_DISCARD));
            }
        }

        // Loop per vertex
        pSrcPos = vertexData->softwareBlendInfo->pSrcPositions;
        pSrcNorm = vertexData->softwareBlendInfo->pSrcNormals;
        pBlendIdx = vertexData->softwareBlendInfo->pBlendIndexes;
        pBlendWeight = vertexData->softwareBlendInfo->pBlendWeights;
        // Make sure we have the source pointers we need
        assert(pSrcPos && pBlendIdx && pBlendWeight && (pSrcNorm || !elemNorm));
		Vector3 tempVec;
        for (size_t vertIdx = 0; vertIdx < vertexData->vertexCount; ++vertIdx)
        {
            // Load source vertex elements
            sourceVec.x = *pSrcPos++;
            sourceVec.y = *pSrcPos++;
            sourceVec.z = *pSrcPos++;

            if (elemNorm) 
            {
                sourceNorm.x = *pSrcNorm++;
                sourceNorm.y = *pSrcNorm++;
                sourceNorm.z = *pSrcNorm++;
            }
            // Load accumulators
            accumVecPos = Vector3::ZERO;
            accumVecNorm = Vector3::ZERO;

            // Loop per blend weight 
            for (unsigned short blendIdx = 0; 
                blendIdx < vertexData->softwareBlendInfo->numWeightsPerVertex; ++blendIdx)
            {
                // Blend by multiplying source by blend matrix and scaling by weight
                // Add to accumulator
                // NB weights must be normalised!!
                if (*pBlendWeight != 0.0) 
                {
                    // Blend position
					tempVec = pMatrices[*pBlendIdx] * sourceVec;
					tempVec *= *pBlendWeight;
                    accumVecPos += tempVec;
                    if (elemNorm)
                    {
                        // Blend normal
                        // We should blend by inverse transpose here, but because we're assuming the 3x3
                        // aspect of the matrix is orthogonal (no non-uniform scaling), the inverse transpose
                        // is equal to the main 3x3 matrix
                        // Note because it's a normal we just extract the rotational part, saves us renormalising here
                        pMatrices[*pBlendIdx].extract3x3Matrix(rot3x3);
						tempVec = rot3x3 * sourceNorm;
						tempVec *= *pBlendWeight;
                        accumVecNorm += tempVec;
                    }

                }
                ++pBlendWeight;
                ++pBlendIdx;
            }

            // Stored blended vertex in hardware buffer
            *pDestPos++ = accumVecPos.x;
            *pDestPos++ = accumVecPos.y;
            *pDestPos++ = accumVecPos.z;

            // Stored blended vertex in temp buffer
            if (elemNorm)
            {
				// Normalise
				accumVecNorm.normalise();
                if (posNormShareBuffer)
                {
                    // Pack into same buffer
                    *pDestPos++ = accumVecNorm.x;
                    *pDestPos++ = accumVecNorm.y;
                    *pDestPos++ = accumVecNorm.z;
                }
                else
                {
                    *pDestNorm++ = accumVecNorm.x;
                    *pDestNorm++ = accumVecNorm.y;
                    *pDestNorm++ = accumVecNorm.z;
                }
            }
        }
        posBuf->unlock();
        if (elemNorm && !posNormShareBuffer)
        {
            normBuf->unlock();
        }

    }
    //-----------------------------------------------------------------------
    void RenderSystem::_setWorldMatrices(const Matrix4* m, unsigned short count)
    {
        if (!mCapabilities->hasCapability(RSC_VERTEXBLENDING))
        {
            // Save these matrices for software blending later
            for (unsigned short i = 0; i < count; ++i)
            {
                mWorldMatrices[i] = m[i];
            }
            // Set hardware matrix to nothing
            _setWorldMatrix(Matrix4::IDENTITY);
        }
        // TODO: implement vertex blending support in DX8 & possibly GL_ARB_VERTEX_BLEND (in subclasses)
    }
    //---------------------------------------------------------------------
    void RenderSystem::setStencilBufferParams(CompareFunction func, ulong refValue, 
        ulong mask, StencilOperation stencilFailOp, 
        StencilOperation depthFailOp, StencilOperation passOp)
    {
        setStencilBufferFunction(func);
        setStencilBufferReferenceValue(refValue);
        setStencilBufferMask(mask);
        setStencilBufferFailOperation(stencilFailOp);
        setStencilBufferDepthFailOperation(depthFailOp);
        setStencilBufferPassOperation(passOp);
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_render(const RenderOperation& op)
    {
        // Update stats
        size_t val;

        if (op.useIndexes)
            val = op.indexData->indexCount;
        else
            val = op.vertexData->vertexCount;

        switch(op.operationType)
        {
		case RenderOperation::OT_TRIANGLE_LIST:
            mFaceCount += val / 3;
            break;
        case RenderOperation::OT_TRIANGLE_STRIP:
        case RenderOperation::OT_TRIANGLE_FAN:
            mFaceCount += val - 2;
            break;
	    case RenderOperation::OT_POINT_LIST:
	    case RenderOperation::OT_LINE_LIST:
	    case RenderOperation::OT_LINE_STRIP:
	        break;
	    }

        mVertexCount += op.vertexData->vertexCount;

        if (op.vertexData->softwareBlendInfo && op.vertexData->softwareBlendInfo->automaticBlend)
        {
            // Software Blend
            softwareVertexBlend(const_cast<VertexData*>(op.vertexData), mWorldMatrices);
        }
    }
    //-----------------------------------------------------------------------
    void RenderSystem::setInvertVertexWinding(bool invert)
    {
        mInvertVertexWinding = invert;
    }

}


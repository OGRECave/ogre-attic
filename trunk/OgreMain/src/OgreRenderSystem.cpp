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
// RenderSystem implementation
// Note that most of this class is abstract since
//  we cannot know how to implement the behaviour without
//  being aware of the 3D API. However there are a few
//  simple functions which can have a base implementation

#include "OgreRenderSystem.h"

#include "OgreRoot.h"
#include "OgreViewport.h"
#include "OgreException.h"
#include "OgreRenderWindow.h"
#include "OgreMeshManager.h"
#include "OgreMaterial.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    RenderSystem::RenderSystem()
    {
        mActiveViewport = 0;
        mTextureManager = 0;
        mVSync = true;


        // This means CULL clockwise vertices, i.e. front of poly is counter-clockwise
        // This makes it the same as OpenGL and other right-handed systems
        mCullingMode = CULL_CLOCKWISE;

        // Create an initially sized software vertex blend buffer
        // Enough for 5000 vertices
        mTempVertexBlendBuffer.resize(5000 * 3);

    }

    //-----------------------------------------------------------------------
    RenderSystem::~RenderSystem()
    {
        // Destroy render windows created
        /*
        for (RenderWindowMap::iterator i = mRenderWindows.begin();
                i != mRenderWindows.end(); ++i)
        {
            delete (*i).second;
        }
        */

    }
    //-----------------------------------------------------------------------
    void RenderSystem::addFrameListener(FrameListener* newListener)
    {
        // Insert, unique only (set)
        mFrameListeners.insert(newListener);
    }
    //-----------------------------------------------------------------------
    void RenderSystem::removeFrameListener(FrameListener* oldListener)
    {
        // Remove, 1 only (set)
        mFrameListeners.erase(oldListener);
    }
    //-----------------------------------------------------------------------
    bool RenderSystem::fireFrameStarted(FrameEvent& evt)
    {
        // Tell all listeners
        std::set<FrameListener*>::iterator i;
        for (i= mFrameListeners.begin(); i != mFrameListeners.end(); ++i)
        {
            if (!(*i)->frameStarted(evt))
                return false;
        }

        return true;

    }
    //-----------------------------------------------------------------------
    bool RenderSystem::fireFrameEnded(FrameEvent& evt)
    {
        // Tell all listeners
        std::set<FrameListener*>::iterator i;
        for (i= mFrameListeners.begin(); i != mFrameListeners.end(); ++i)
        {
            if (!(*i)->frameEnded(evt))
                return false;
        }
        return true;
    }
    //-----------------------------------------------------------------------
    void RenderSystem::startRendering(void)
    {

        // Init stats
        for(
            RenderWindowMap::iterator it = mRenderWindows.begin();
            it != mRenderWindows.end();
            ++it )
        {
            it->second->resetStatistics();
        }

    }
    //-----------------------------------------------------------------------
    RenderWindow* RenderSystem::initialise(bool autoCreateWindow)
    {
        // Have I been registered by call to Root::setRenderSystem?
        RenderSystem* regPtr = Root::getSingleton().getRenderSystem();
        if (!regPtr || regPtr != this)
            // Register self - library user has come to me direct
            Root::getSingleton().setRenderSystem(this);

        // Init mesh manager
        MeshManager::getSingleton()._initialise();

        // Subclasses should take it from here
        // They should ALL call this superclass method from
        //   their own initialise() implementations.

        return 0;
    }
    //-----------------------------------------------------------------------
    RenderWindow* RenderSystem::getRenderWindow(const String &name)
    {
        RenderWindowMap::iterator it = mRenderWindows.find(name);

        if( it != mRenderWindows.end() )
            Except(
                Exception::ERR_ITEM_NOT_FOUND,
                "Cannot find window called '" + name + "'",
                "RenderSystem::getRenderWindow");

        return( it->second );
    }
    //-----------------------------------------------------------------------
    void RenderSystem::destroyRenderWindow(const String &name)
    {
        mRenderWindows.erase(name);

    }
    //-----------------------------------------------------------------------
    Viewport* RenderSystem::_getViewport(void)
    {
        return mActiveViewport;
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_setTextureUnitSettings(int texUnit, Material::TextureLayer& tl)
    {
        // This method is only ever called to set a texture unit to valid details
        // The method _disableTextureUnit is called to turn a unit off

        Material::TextureLayer& curr = mTextureUnits[texUnit];
        bool currIsBlank = curr.isBlank();

        // Texture name
        String texName = tl.getTextureName();
        if (currIsBlank || curr.getTextureName() != texName)
        {
            _setTexture(texUnit, true, texName);
        }

        // Set texture coordinate set
        int coordSet = tl.getTextureCoordSet();
        if (currIsBlank || curr.getTextureCoordSet() != coordSet)
        {
            _setTextureCoordSet(texUnit, coordSet);
        }

        // Set blend modes
        LayerBlendModeEx newBlend = tl.getColourBlendMode();
        if (currIsBlank || curr.getColourBlendMode() != newBlend)
        {
            _setTextureBlendMode(texUnit, newBlend);
        }
        newBlend = tl.getAlphaBlendMode();
        if (currIsBlank || curr.getAlphaBlendMode() != newBlend)
        {
            _setTextureBlendMode(texUnit, newBlend);
        }

        // Set addressing
        Material::TextureLayer::TextureAddressingMode addr = tl.getTextureAddressingMode();
        if (currIsBlank || curr.getTextureAddressingMode() != addr)
        {
            _setTextureAddressingMode(texUnit, addr );
        }


        // Set texture effects
        Material::TextureLayer::EffectMap::iterator effi;
        bool currEnv = false;
        bool currEnvPlanar = false;
        // bool currTexMod = false;
        // Iterate over current effects
        for (effi = curr.mEffects.begin(); effi != curr.mEffects.end(); ++effi)
        {
            switch (effi->second.type)
            {
            case Material::TextureLayer::ET_ENVIRONMENT_MAP:
                if (effi->second.subtype == Material::TextureLayer::ENV_CURVED)
                {
                    currEnv = true;
                }
                else if (effi->second.subtype == Material::TextureLayer::ENV_PLANAR)
                {
                    currEnvPlanar = true;
                }
                break;
	    case Material::TextureLayer::ET_BUMP_MAP:
	    case Material::TextureLayer::ET_SCROLL:
	    case Material::TextureLayer::ET_ROTATE:
	    case Material::TextureLayer::ET_TRANSFORM:
	      break;
            }
        }
        // Iterate over new effects
        bool anyCalcs = false;
        for (effi = tl.mEffects.begin(); effi != tl.mEffects.end(); ++effi)
        {
            switch (effi->second.type)
            {
            case Material::TextureLayer::ET_ENVIRONMENT_MAP:
                if (effi->second.subtype == Material::TextureLayer::ENV_CURVED)
                {
                    if (currIsBlank || !currEnv)
                    {
                        _setTextureCoordCalculation(texUnit, TEXCALC_ENVIRONMENT_MAP);
                    }
                    anyCalcs = true;
                }
                else if (effi->second.subtype == Material::TextureLayer::ENV_PLANAR)
                {
                    if (currIsBlank || !currEnvPlanar)
                    {
                        _setTextureCoordCalculation(texUnit, TEXCALC_ENVIRONMENT_MAP_PLANAR);
                    }
                    anyCalcs = true;
                }
                break;
	    case Material::TextureLayer::ET_BUMP_MAP:
	    case Material::TextureLayer::ET_SCROLL:
	    case Material::TextureLayer::ET_ROTATE:
	    case Material::TextureLayer::ET_TRANSFORM:
	      break;
            }
        }
        // Ensure any previous texcoord calc settings are reset if there are now none
        if ((!anyCalcs) && (currEnv || currEnvPlanar))
        {
            _setTextureCoordCalculation(texUnit, TEXCALC_NONE);
            _setTextureCoordSet(texUnit, tl.getTextureCoordSet());
        }


        // Change tetxure matrix if required
        // NB concatenate with any existing texture matrix created for generate
        const Matrix4& xform = tl.getTextureTransform();
        // HACK for now - turn off texture calc if any texture generation effects
        if( !( curr.getTextureTransform() == xform ) && !anyCalcs )
        {
            _setTextureMatrix(texUnit, xform);
        }

        // Set alpha rejection
        unsigned char alphaVal = tl.getAlphaRejectValue();
        CompareFunction alphaFunc = tl.getAlphaRejectFunction();
        if (currIsBlank ||
            (curr.getAlphaRejectFunction() != alphaFunc) ||
            (curr.getAlphaRejectValue() != alphaVal))
        {
            _setAlphaRejectSettings(alphaFunc, alphaVal);
        }

        // Now that the changes have been made, update the record of current texture unit settings
        curr = tl;

    }
    //-----------------------------------------------------------------------
    void RenderSystem::_disableTextureUnit(int texUnit)
    {
        _setTexture(texUnit, false, "");
        mTextureUnits[texUnit].setBlank();


    }
    //-----------------------------------------------------------------------
    /*
    void RenderSystem::_setMaterial(Material &mat)
    {

        // Set surface properties
        _setSurfaceParams(mat.ambient, mat.diffuse, mat.specular, mat.emmissive, mat.shininess);

        // Set global blending
        _setSceneBlending(mat.getSourceBlendFactor(), mat.getDestBlendFactor());

        // Set textures
        // Note that it is assumed caller has checked that there are
        // enough texture units to support multitexturing all layers
        // If not they should be calling _setTexture separately per multipass render
        // If the texture layers exceed the number of supported texture
        // units, the remaining textures will not be displayed
        int matTexLayers = mat.getNumTextureLayers();
        for (int texLayer = 0; texLayer < _getNumTextureUnits(); ++texLayer)
        {
            if (texLayer >= matTexLayers)
            {
                // Run out of material texture layers before h/w
                // Turn off these units
                _setTexture(texLayer, false, "");
            }
            else
            {
                Material::TextureLayer* tl = mat.getTextureLayer(texLayer);
                _setTextureUnitSettings(texLayer, *tl);
            }
        }
    }
    */
    //-----------------------------------------------------------------------
    CullingMode RenderSystem::_getCullingMode(void)
    {
        return mCullingMode;
    }
    //-----------------------------------------------------------------------
    bool RenderSystem::getWaitForVerticalBlank(void)
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
        // Close all windows
        for( RenderWindowMap::iterator it = mRenderWindows.begin(); it != mRenderWindows.end(); ++it )
        {
            it->second->destroy();
        }

    }
    //-----------------------------------------------------------------------
    void RenderSystem::_beginGeometryCount(void)
    {
        mFaceCount = mVertexCount = 0;

    }
    //-----------------------------------------------------------------------
    unsigned int RenderSystem::_getFaceCount(void)
    {
        return mFaceCount;
    }
    //-----------------------------------------------------------------------
    unsigned int RenderSystem::_getVertexCount(void)
    {
        return mVertexCount;
    }
    //-----------------------------------------------------------------------
    void RenderSystem::_render(RenderOperation& op)
    {
        // Update stats
        int val;

        if (op.useIndexes)
            val = op.numIndexes;
        else
            val = op.numVertices;

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

        mVertexCount += op.numVertices;

        // Vertex blending: do software if required
        if ((op.vertexOptions & RenderOperation::VO_BLEND_WEIGHTS) && 
            !this->_isVertexBlendSupported())
        {
            // Software blending required
            softwareVertexBlend(op, mWorldMatrices);
            // Turn off main world matrix
            this->_setWorldMatrix(Matrix4::IDENTITY);
        }
    }
    //-----------------------------------------------------------------------
    bool RenderSystem::_isVertexBlendSupported(void)
    {
        // TODO: implement vertex blending support in DX8 & possibly GL_ARB_VERTEX_BLEND (in subclasses)
        // DX7 support not good enough - only 4 matrices supported
        return false;
    }
    //-----------------------------------------------------------------------
    unsigned short RenderSystem::_getNumVertexBlendMatrices(void)
    {
        // TODO: implement vertex blending support in DX8 & possibly GL_ARB_VERTEX_BLEND (in subclasses)
        return 1;
    }
    //-----------------------------------------------------------------------
    void RenderSystem::softwareVertexBlend(RenderOperation& op, Matrix4* pMatrices)
    {
        // Source vector
        Vector3 sourceVec;
        // Accumulation vector
        Vector3 accumVec;

        Real* pVertElem;
        RenderOperation::VertexBlendData* pBlend;

        // Check buffer size
        unsigned long numVertReals = op.numVertices * 3;
        if (mTempVertexBlendBuffer.size() < numVertReals)
        {
            mTempVertexBlendBuffer.resize(numVertReals);
        }

        // Loop per vertex
        pVertElem = op.pVertices;
        pBlend = op.pBlendingWeights;
        for (unsigned long vertIdx = 0; 
            vertIdx < numVertReals; vertIdx += 3)
        {
            // Load source vertex elements
            sourceVec.x = *pVertElem++;
            sourceVec.y = *pVertElem++;
            sourceVec.z = *pVertElem++;
            // Load accumulator
            accumVec = Vector3::ZERO;

            // Loop per blend weight 
            for (unsigned short blendIdx = 0; blendIdx < op.numBlendWeightsPerVertex; ++blendIdx)
            {
                // Blend by multiplying source by blend matrix and scaling by weight
                // Add to accumulator
                // NB weights must be normalised!!
                if (pBlend->blendWeight != 0.0) 
                {
                    accumVec += (pMatrices[pBlend->matrixIndex] * sourceVec) 
                        * pBlend->blendWeight;
                }
                pBlend++;
            }

            // Stored blended vertex in temp buffer
            mTempVertexBlendBuffer[vertIdx] = accumVec.x;
            mTempVertexBlendBuffer[vertIdx+1] = accumVec.y;
            mTempVertexBlendBuffer[vertIdx+2] = accumVec.z;

        }

        // Re-point the render operation vertex buffer
        op.pVertices = mTempVertexBlendBuffer.begin();

        


    }
    //-----------------------------------------------------------------------
    void RenderSystem::_setWorldMatrices(const Matrix4* m, unsigned short count)
    {
        if (!_isVertexBlendSupported())
        {
            // Save these matrices for software blending later
            int i;
            for (i = 0; i < count; ++i)
            {
                mWorldMatrices[i] = m[i];
            }
        }
        // TODO: implement vertex blending support in DX8 & possibly GL_ARB_VERTEX_BLEND (in subclasses)
    }

}


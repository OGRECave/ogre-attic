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

#include "OgrePanelGuiElement.h"
#include "OgreMaterial.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreStringConverter.h"
#include "OgreHardwareBufferManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {
    //---------------------------------------------------------------------
    String PanelGuiElement::msTypeName = "Panel";
    PanelGuiElement::CmdTiling PanelGuiElement::msCmdTiling;
    PanelGuiElement::CmdTransparent PanelGuiElement::msCmdTransparent;
    //---------------------------------------------------------------------
    // vertex buffer bindings, set at compile time (we could look these up but no point)
    #define POSITION_BINDING 0
    #define TEXCOORD_BINDING 1

    //---------------------------------------------------------------------
    PanelGuiElement::PanelGuiElement(const String& name)
        : GuiContainer(name)
    {
        mTransparent = false;
        // Init tiling
        for (ushort i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS; ++i)
        {
            mTileX[i] = 1.0f;
            mTileY[i] = 1.0f;
        }

        // Defer creation of texcoord buffer until we know how big it needs to be
        mNumTexCoordsInBuffer = 0;

        // No normals or colours
        if (createParamDictionary("PanelGuiElement"))
        {
            addBaseParameters();
        }

    }
    //---------------------------------------------------------------------
    PanelGuiElement::~PanelGuiElement()
    {
        delete mRenderOp.vertexData;
    }
    //---------------------------------------------------------------------
    void PanelGuiElement::initialise(void)
    {
        // Setup render op in advance
        mRenderOp.vertexData = new VertexData();
        // Vertex declaration: 1 position, add texcoords later depending on #layers
        // Create as separate buffers so we can lock & discard separately
        VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;
        decl->addElement(POSITION_BINDING, 0, VET_FLOAT3, VES_POSITION);

        // Basic vertex data
        mRenderOp.vertexData->vertexStart = 0;
        mRenderOp.vertexData->vertexCount = 4;

        // Vertex buffer #1
        HardwareVertexBufferSharedPtr vbuf = 
            HardwareBufferManager::getSingleton().createVertexBuffer(
            decl->getVertexSize(POSITION_BINDING), mRenderOp.vertexData->vertexCount, 
            HardwareBuffer::HBU_STATIC_WRITE_ONLY// mostly static except during resizing
            );
        // Bind buffer
        mRenderOp.vertexData->vertexBufferBinding->setBinding(POSITION_BINDING, vbuf);

        // No indexes & issue as a strip
        mRenderOp.useIndexes = false;
        mRenderOp.operationType = RenderOperation::OT_TRIANGLE_STRIP;
    }
    //---------------------------------------------------------------------
    void PanelGuiElement::setTiling(Real x, Real y, ushort layer)
    {
        assert (layer < OGRE_MAX_TEXTURE_COORD_SETS);
        assert (x != 0 && y != 0);

        mTileX[layer] = x;
        mTileY[layer] = y;

        updateTextureGeometry();

    }
    //---------------------------------------------------------------------
    Real PanelGuiElement::getTileX(ushort layer) const
    {
        return mTileX[layer];
    }
    //---------------------------------------------------------------------
    Real PanelGuiElement::getTileY(ushort layer) const
    {
        return mTileY[layer];
    }
    //---------------------------------------------------------------------
    void PanelGuiElement::setTransparent(bool isTransparent)
    {
        mTransparent = isTransparent;
    }
    //---------------------------------------------------------------------
    bool PanelGuiElement::isTransparent(void) const
    {
        return mTransparent;
    }
    //---------------------------------------------------------------------
    const String& PanelGuiElement::getTypeName(void) const
    {
        return msTypeName;
    }
    //---------------------------------------------------------------------
    void PanelGuiElement::getRenderOperation(RenderOperation& op)
    {
        op = mRenderOp;
    }
    //---------------------------------------------------------------------
    void PanelGuiElement::setMaterialName(const String& matName)
    {
        GuiContainer::setMaterialName(matName);
        updateTextureGeometry();
    }
    //---------------------------------------------------------------------
    void PanelGuiElement::_updateRenderQueue(RenderQueue* queue)
    {
        if (mVisible)
        {

            if (!mTransparent && mpMaterial)
            {
                GuiElement::_updateRenderQueue(queue);
            }

            // Also add children
            ChildIterator it = getChildIterator();
            while (it.hasMoreElements())
            {
                // Give children ZOrder 1 higher than this
                it.getNext()->_updateRenderQueue(queue);
            }
        }
    }
    //---------------------------------------------------------------------
    void PanelGuiElement::updatePositionGeometry(void)
    {
        /*
            0-----2
            |    /|
            |  /  |
            |/    |
            1-----3
        */
        Real left, right, top, bottom;

        /* Convert positions into -1, 1 coordinate space (homogenous clip space).
            - Left / right is simple range conversion
            - Top / bottom also need inverting since y is upside down - this means
              that top will end up greater than bottom and when computing texture
              coordinates, we have to flip the v-axis (ie. subtract the value from
              1.0 to get the actual correct value).
        */
        left = _getDerivedLeft() * 2 - 1;
        right = left + (mWidth * 2);
        top = -((_getDerivedTop() * 2) - 1);
        bottom =  top -  (mHeight * 2);

        HardwareVertexBufferSharedPtr vbuf = 
            mRenderOp.vertexData->vertexBufferBinding->getBuffer(POSITION_BINDING);
        Real* pPos = static_cast<Real*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD) );
        // Use the furthest away depth value, since materials should have depth-check off
        // This initialised the depth buffer for any 3D objects in front
        Real zValue = Root::getSingleton().getRenderSystem()->getMaximumDepthInputValue();
        
        *pPos++ = left;
        *pPos++ = top;
        *pPos++ = zValue;

        *pPos++ = left;
        *pPos++ = bottom;
        *pPos++ = zValue;

        *pPos++ = right;
        *pPos++ = top;
        *pPos++ = zValue;

        *pPos++ = right;
        *pPos++ = bottom;
        *pPos++ = zValue;
        
        vbuf->unlock();
    }
    //---------------------------------------------------------------------
    void PanelGuiElement::updateTextureGeometry(void)
    {
        // Generate for as many texture layers as there are in material
        if (mpMaterial)
        {
            // Assume one technique and pass for the moment
            size_t numLayers = mpMaterial->getTechnique(0)->getPass(0)->getNumTextureUnitStates();

            VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;
            // Check the number of texcoords we have in our buffer now
            if (mNumTexCoordsInBuffer > numLayers)
            {
                // remove extras
                for (size_t i = mNumTexCoordsInBuffer; i > numLayers; --i)
                {
                    decl->removeElement(VES_TEXTURE_COORDINATES, i);
                }
            }
            else if (mNumTexCoordsInBuffer < numLayers)
            {
                // Add extra texcoord elements
                size_t offset = VertexElement::getTypeSize(VET_FLOAT2) * mNumTexCoordsInBuffer;
                for (size_t i = mNumTexCoordsInBuffer; i < numLayers; ++i)
                {
                    decl->addElement(TEXCOORD_BINDING,
                        offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, i);
                    offset += VertexElement::getTypeSize(VET_FLOAT2);
                   
                }
            }

            // if number of layers changed at all, we'll need to reallocate buffer
            if (mNumTexCoordsInBuffer != numLayers)
            {
                // NB reference counting will take care of the old one if it exists
                HardwareVertexBufferSharedPtr newbuf = 
                    HardwareBufferManager::getSingleton().createVertexBuffer(
                    decl->getVertexSize(TEXCOORD_BINDING), mRenderOp.vertexData->vertexCount, 
                    HardwareBuffer::HBU_STATIC_WRITE_ONLY // mostly static except during resizing
                    );
                // Bind buffer, note this will unbind the old one and destroy the buffer it had
                mRenderOp.vertexData->vertexBufferBinding->setBinding(TEXCOORD_BINDING, newbuf);
                // Set num tex coords in use now
                mNumTexCoordsInBuffer = numLayers;
            }

            // Get the tcoord buffer & lock
            HardwareVertexBufferSharedPtr vbuf = 
                mRenderOp.vertexData->vertexBufferBinding->getBuffer(TEXCOORD_BINDING);
            Real* pVBStart = static_cast<Real*>(
                vbuf->lock(HardwareBuffer::HBL_DISCARD) );

            size_t uvSize = VertexElement::getTypeSize(VET_FLOAT2) / sizeof(Real);
            size_t vertexSize = decl->getVertexSize(TEXCOORD_BINDING) / sizeof(Real);
            for (ushort i = 0; i < numLayers; ++i)
            {
                // Calc upper tex coords
                Real upperX = 1.0f * mTileX[i];
                Real upperY = 1.0f * mTileY[i];
                
                /*
                    0-----2
                    |    /|
                    |  /  |
                    |/    |
                    1-----3
                */
                // Find start offset for this set
                Real* pTex = pVBStart + (i * uvSize);

                pTex[0] = 0.0f;
                pTex[1] = 0.0f;

                pTex += vertexSize; // jump by 1 vertex stride
                pTex[0] = 0.0f;
                pTex[1] = upperY;

                pTex += vertexSize;
                pTex[0] = upperX;
                pTex[1] = 0.0f;

                pTex += vertexSize;
                pTex[0] = upperX;
                pTex[1] = upperY;
            }
			vbuf->unlock();
        }
    }
    //-----------------------------------------------------------------------
    void PanelGuiElement::addBaseParameters(void)
    {
        GuiContainer::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("tiling", 
            "The number of times to repeat the background texture."
            , PT_STRING),
            &msCmdTiling);

        dict->addParameter(ParameterDef("transparent", 
            "Sets whether the panel is transparent, i.e. invisible itself "
            "but it's contents are still displayed."
            , PT_BOOL),
            &msCmdTransparent);
    }
    //-----------------------------------------------------------------------
    // Command objects
    //-----------------------------------------------------------------------
    String PanelGuiElement::CmdTiling::doGet(const void* target) const
    {
        // NB only returns 1st layer tiling
        String ret = "0 " + StringConverter::toString(
            static_cast<const PanelGuiElement*>(target)->getTileX() );
        ret += " " + StringConverter::toString(
            static_cast<const PanelGuiElement*>(target)->getTileY() );
        return ret;
    }
    void PanelGuiElement::CmdTiling::doSet(void* target, const String& val)
    {
        // 3 params: <layer> <x_tile> <y_tile>
        // Param count is validated higher up
        std::vector<String> vec = StringUtil::split(val);
        ushort layer = (ushort)StringConverter::parseUnsignedInt(vec[0]);
        Real x_tile = StringConverter::parseReal(vec[1]);
        Real y_tile = StringConverter::parseReal(vec[2]);

        static_cast<PanelGuiElement*>(target)->setTiling(x_tile, y_tile, layer);
    }
    //-----------------------------------------------------------------------
    String PanelGuiElement::CmdTransparent::doGet(const void* target) const
    {
        return StringConverter::toString(
            static_cast<const PanelGuiElement*>(target)->isTransparent() );
    }
    void PanelGuiElement::CmdTransparent::doSet(void* target, const String& val)
    {
        static_cast<PanelGuiElement*>(target)->setTransparent(
            StringConverter::parseBool(val));
    }


}




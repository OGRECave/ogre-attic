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

#include "OgreBorderPanelGuiElement.h"
#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreStringConverter.h"
#include "OgreOverlayManager.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreException.h"
#include "OgreRenderQueue.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {
    //---------------------------------------------------------------------
    String BorderPanelGuiElement::msTypeName = "BorderPanel";
    BorderPanelGuiElement::CmdBorderSize BorderPanelGuiElement::msCmdBorderSize;
    BorderPanelGuiElement::CmdBorderMaterial BorderPanelGuiElement::msCmdBorderMaterial;
    BorderPanelGuiElement::CmdBorderLeftUV BorderPanelGuiElement::msCmdBorderLeftUV;
    BorderPanelGuiElement::CmdBorderTopUV BorderPanelGuiElement::msCmdBorderTopUV;
    BorderPanelGuiElement::CmdBorderBottomUV BorderPanelGuiElement::msCmdBorderBottomUV;
    BorderPanelGuiElement::CmdBorderRightUV BorderPanelGuiElement::msCmdBorderRightUV;
    BorderPanelGuiElement::CmdBorderTopLeftUV BorderPanelGuiElement::msCmdBorderTopLeftUV;
    BorderPanelGuiElement::CmdBorderBottomLeftUV BorderPanelGuiElement::msCmdBorderBottomLeftUV;
    BorderPanelGuiElement::CmdBorderTopRightUV BorderPanelGuiElement::msCmdBorderTopRightUV;
    BorderPanelGuiElement::CmdBorderBottomRightUV BorderPanelGuiElement::msCmdBorderBottomRightUV;

    #define BCELL_UV(x) (x * 4 * 2)
    #define POSITION_BINDING 0
    #define TEXCOORD_BINDING 1
    //---------------------------------------------------------------------
    BorderPanelGuiElement::BorderPanelGuiElement(const String& name)
      : PanelGuiElement(name), 
        mLeftBorderSize(0),
        mRightBorderSize(0),
        mTopBorderSize(0),
        mBottomBorderSize(0),
        mPixelLeftBorderSize(0),
        mPixelRightBorderSize(0),
        mPixelTopBorderSize(0),
        mPixelBottomBorderSize(0),
        mpBorderMaterial(0),
        mBorderRenderable(0)
    {
        if (createParamDictionary("BorderPanelGuiElement"))
        {
            addBaseParameters();
        }
    }
    //---------------------------------------------------------------------
    BorderPanelGuiElement::~BorderPanelGuiElement()
    {
        delete mRenderOp2.vertexData;
        delete mRenderOp2.indexData;
        delete mBorderRenderable;
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::initialise(void)
    {
        PanelGuiElement::initialise();

        // superclass will handle the interior panel area 

        // Setup render op in advance
        mRenderOp2.vertexData = new VertexData();
        mRenderOp2.vertexData->vertexCount = 4 * 8; // 8 cells, can't necessarily share vertices cos
                                                    // texcoords may differ
        mRenderOp2.vertexData->vertexStart = 0;

        // Vertex declaration
        VertexDeclaration* decl = mRenderOp2.vertexData->vertexDeclaration;
        // Position and texture coords each have their own buffers to allow
        // each to be edited separately with the discard flag
        decl->addElement(POSITION_BINDING, 0, VET_FLOAT3, VES_POSITION);
        decl->addElement(TEXCOORD_BINDING, 0, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);

        // Vertex buffer #1, position
        HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton()
            .createVertexBuffer(
                decl->getVertexSize(POSITION_BINDING), 
                mRenderOp2.vertexData->vertexCount,
                HardwareBuffer::HBU_STATIC_WRITE_ONLY);
        // bind position
        VertexBufferBinding* binding = mRenderOp2.vertexData->vertexBufferBinding;
        binding->setBinding(POSITION_BINDING, vbuf);

        // Vertex buffer #2, texcoords
        vbuf = HardwareBufferManager::getSingleton()
            .createVertexBuffer(
                decl->getVertexSize(TEXCOORD_BINDING), 
                mRenderOp2.vertexData->vertexCount,
                HardwareBuffer::HBU_STATIC_WRITE_ONLY, true);
        // bind texcoord
        binding->setBinding(TEXCOORD_BINDING, vbuf);

        mRenderOp2.operationType = RenderOperation::OT_TRIANGLE_LIST;
        mRenderOp2.useIndexes = true;
        // Index data
        mRenderOp2.indexData = new IndexData();
        mRenderOp2.indexData->indexCount = 8 * 6;
        mRenderOp2.indexData->indexStart = 0;

        /* Each cell is
            0-----2
            |    /|
            |  /  |
            |/    |
            1-----3
        */
        mRenderOp2.indexData->indexBuffer = HardwareBufferManager::getSingleton().
            createIndexBuffer(
                HardwareIndexBuffer::IT_16BIT, 
                mRenderOp2.indexData->indexCount, 
                HardwareBuffer::HBU_STATIC_WRITE_ONLY);

        ushort* pIdx = static_cast<ushort*>(
            mRenderOp2.indexData->indexBuffer->lock(
                0, 
                mRenderOp2.indexData->indexBuffer->getSizeInBytes(), 
                HardwareBuffer::HBL_DISCARD) );

        for (int cell = 0; cell < 8; ++cell)
        {
            ushort base = cell * 4;
            *pIdx++ = base;
            *pIdx++ = base + 1;
            *pIdx++ = base + 2;

            *pIdx++ = base + 2;
            *pIdx++ = base + 1;
            *pIdx++ = base + 3;
        }

        mRenderOp2.indexData->indexBuffer->unlock();

        // Create sub-object for rendering border
        mBorderRenderable = new BorderRenderable(this);
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::addBaseParameters(void)
    {
        PanelGuiElement::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("border_size", 
            "The sizes of the borders relative to the screen size, in the order "
            "left, right, top, bottom."
            , PT_STRING),
            &msCmdBorderSize);
        dict->addParameter(ParameterDef("border_material", 
            "The material to use for the border."
            , PT_STRING),
            &msCmdBorderMaterial);
        dict->addParameter(ParameterDef("border_topleft_uv", 
            "The texture coordinates for the top-left corner border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderTopLeftUV);
        dict->addParameter(ParameterDef("border_topright_uv", 
            "The texture coordinates for the top-right corner border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderTopRightUV);
        dict->addParameter(ParameterDef("border_bottomright_uv", 
            "The texture coordinates for the bottom-right corner border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderBottomRightUV);
        dict->addParameter(ParameterDef("border_bottomleft_uv", 
            "The texture coordinates for the bottom-left corner border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderBottomLeftUV);
        dict->addParameter(ParameterDef("border_left_uv", 
            "The texture coordinates for the left edge border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderLeftUV);
        dict->addParameter(ParameterDef("border_top_uv", 
            "The texture coordinates for the top edge border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderTopUV);
        dict->addParameter(ParameterDef("border_right_uv", 
            "The texture coordinates for the right edge border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderRightUV);
        dict->addParameter(ParameterDef("border_bottom_uv", 
            "The texture coordinates for the bottom edge border texture. 2 sets of uv values, "
            "one for the top-left corner, the other for the bottom-right corner."
            , PT_STRING),
            &msCmdBorderBottomUV);

    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setBorderSize(Real size)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelLeftBorderSize = mPixelRightBorderSize = 
                mPixelTopBorderSize = mPixelBottomBorderSize = size;
        }
        else
        {
            mLeftBorderSize = mRightBorderSize = 
                mTopBorderSize = mBottomBorderSize = size;
        }
        mGeomPositionsOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setBorderSize(Real sides, Real topAndBottom)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelLeftBorderSize = mPixelRightBorderSize = sides;
            mPixelTopBorderSize = mPixelBottomBorderSize = topAndBottom;
        }
        else
        {
            mLeftBorderSize = mRightBorderSize = sides;
            mTopBorderSize = mBottomBorderSize = topAndBottom;
        }
        mGeomPositionsOutOfDate = true;


    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setBorderSize(Real left, Real right, Real top, Real bottom)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelLeftBorderSize = left;
            mPixelRightBorderSize = right;
            mPixelTopBorderSize = top;
            mPixelBottomBorderSize = bottom;
        }
        else
        {
            mLeftBorderSize = left;
            mRightBorderSize = right;
            mTopBorderSize = top;
            mBottomBorderSize = bottom;
        }
        mGeomPositionsOutOfDate = true;
    }
    //---------------------------------------------------------------------
    Real BorderPanelGuiElement::getLeftBorderSize(void) const
    {
        if (mMetricsMode == GMM_PIXELS)
        {
			return mPixelLeftBorderSize;
		}
		else
		{
			return mLeftBorderSize;
		}
    }
    //---------------------------------------------------------------------
    Real BorderPanelGuiElement::getRightBorderSize(void) const
    {
        if (mMetricsMode == GMM_PIXELS)
        {
			return mPixelRightBorderSize;
		}
		else
		{
			return mRightBorderSize;
		}
    }
    //---------------------------------------------------------------------
    Real BorderPanelGuiElement::getTopBorderSize(void) const
    {
        if (mMetricsMode == GMM_PIXELS)
        {
			return mPixelTopBorderSize;
		}
		else
		{
			return mTopBorderSize;
		}
    }
    //---------------------------------------------------------------------
    Real BorderPanelGuiElement::getBottomBorderSize(void) const
    {
        if (mMetricsMode == GMM_PIXELS)
        {
			return mPixelBottomBorderSize;
		}
		else
		{
			return mBottomBorderSize;
		}
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setCellUV(BorderCellIndex idx, Real& u1, 
        Real& v1, Real& u2, Real& v2)
    {
        /* Each cell is
            0-----2
            |    /|
            |  /  |
            |/    |
            1-----3
        */
        // No choice but to lock / unlock each time here, but lock only small sections
        
        HardwareVertexBufferSharedPtr vbuf = 
            mRenderOp2.vertexData->vertexBufferBinding->getBuffer(TEXCOORD_BINDING);
        // Can't use discard since this discards whole buffer
        Real* pUV = static_cast<Real*>(
            vbuf->lock(
                BCELL_UV(idx) * sizeof(Real), 
                sizeof(Real)*8, 
                HardwareBuffer::HBL_NORMAL) );

        *pUV++ = u1; *pUV++ = v1;
        *pUV++ = u1; *pUV++ = v2;
        *pUV++ = u2; *pUV++ = v1;
        *pUV++ = u2; *pUV++ = v2;

        vbuf->unlock();
       
    }
    //---------------------------------------------------------------------
    String BorderPanelGuiElement::getCellUVString(BorderCellIndex idx) const
    {
        /* Each cell is
            0-----2
            |    /|
            |  /  |
            |/    |
            1-----3
        */
        // No choice but to lock / unlock each time here, but lock only small sections
        
        HardwareVertexBufferSharedPtr vbuf = 
            mRenderOp2.vertexData->vertexBufferBinding->getBuffer(TEXCOORD_BINDING);
        // Lock just the portion we need in read-only mode
        // Can't use discard since this discards whole buffer
        Real* pUV = static_cast<Real*>(
            vbuf->lock(
                BCELL_UV(idx) * sizeof(Real), 
                sizeof(Real)*8, 
                HardwareBuffer::HBL_READ_ONLY) );

        String ret = StringConverter::toString(pUV[0]) + " " +
		            StringConverter::toString(pUV[1]) + " " +
		            StringConverter::toString(pUV[6]) + " " +
		            StringConverter::toString(pUV[7]);
        vbuf->unlock();
        return ret;
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setLeftBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
        setCellUV(BCELL_LEFT, u1, v1, u2, v2);
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setRightBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
        setCellUV(BCELL_RIGHT, u1, v1, u2, v2);
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setTopBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
        setCellUV(BCELL_TOP, u1, v1, u2, v2);
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setBottomBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
        setCellUV(BCELL_BOTTOM, u1, v1, u2, v2);
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setTopLeftBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
        setCellUV(BCELL_TOP_LEFT, u1, v1, u2, v2);
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setTopRightBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
        setCellUV(BCELL_TOP_RIGHT, u1, v1, u2, v2);
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setBottomLeftBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
        setCellUV(BCELL_BOTTOM_LEFT, u1, v1, u2, v2);
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setBottomRightBorderUV(Real u1, Real v1, Real u2, Real v2)
    {
        setCellUV(BCELL_BOTTOM_RIGHT, u1, v1, u2, v2);
    }

    //---------------------------------------------------------------------
    String BorderPanelGuiElement::getLeftBorderUVString() const
    {
        return getCellUVString(BCELL_LEFT);
    }
    //---------------------------------------------------------------------
    String BorderPanelGuiElement::getRightBorderUVString() const
    {
        return getCellUVString(BCELL_RIGHT);
    }
    //---------------------------------------------------------------------
    String BorderPanelGuiElement::getTopBorderUVString() const
    {
        return getCellUVString(BCELL_TOP);
    }
    //---------------------------------------------------------------------
    String BorderPanelGuiElement::getBottomBorderUVString() const
    {
        return getCellUVString(BCELL_BOTTOM);
    }
    //---------------------------------------------------------------------
    String BorderPanelGuiElement::getTopLeftBorderUVString() const
    {
        return getCellUVString(BCELL_TOP_LEFT);
    }
    //---------------------------------------------------------------------
    String BorderPanelGuiElement::getTopRightBorderUVString() const
    {
        return getCellUVString(BCELL_TOP_RIGHT);
    }
    //---------------------------------------------------------------------
    String BorderPanelGuiElement::getBottomLeftBorderUVString() const
    {
        return getCellUVString(BCELL_BOTTOM_LEFT);
    }
    //---------------------------------------------------------------------
    String BorderPanelGuiElement::getBottomRightBorderUVString() const
    {
        return getCellUVString(BCELL_BOTTOM_RIGHT);
    }





    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setBorderMaterialName(const String& name)
    {
        mBorderMaterialName = name;
        mpBorderMaterial = (Material*)MaterialManager::getSingleton().getByName(name);
        if (!mpBorderMaterial)
			Except( Exception::ERR_ITEM_NOT_FOUND, "Could not find material " + name,
				"BorderPanelGuiElement::setBorderMaterialName" );
        mpBorderMaterial->load();
        // Set some prerequisites to be sure
        mpBorderMaterial->setLightingEnabled(false);
        mpBorderMaterial->setDepthCheckEnabled(false);

    }
    //---------------------------------------------------------------------
    const String& BorderPanelGuiElement::getBorderMaterialName(void) const
    {
        return mBorderMaterialName;
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::updatePositionGeometry(void)
    {
        /*
        Grid is like this:
        +--+---------------+--+
        |0 |       1       |2 |
        +--+---------------+--+
        |  |               |  |
        |  |               |  |
        |3 |    center     |4 |
        |  |               |  |
        +--+---------------+--+
        |5 |       6       |7 |
        +--+---------------+--+
        */
        // Convert positions into -1, 1 coordinate space (homogenous clip space)
        // Top / bottom also need inverting since y is upside down
        Real left[8], right[8], top[8], bottom[8];
        // Horizontal
        left[0] = left[3] = left[5] = _getDerivedLeft() * 2 - 1;
        left[1] = left[6] = right[0] = right[3] = right[5] = left[0] + (mLeftBorderSize * 2);
        right[2] = right[4] = right[7] = left[0] + (mWidth * 2);
        left[2] = left[4] = left[7] = right[1] = right[6] = right[2] - (mRightBorderSize * 2);
        // Vertical
        top[0] = top[1] = top[2] = -((_getDerivedTop() * 2) - 1);
        top[3] = top[4] = bottom[0] = bottom[1] = bottom[2] = top[0] - (mTopBorderSize * 2);
        bottom[5] = bottom[6] = bottom[7] = top[0] -  (mHeight * 2);
        top[5] = top[6] = top[7] = bottom[3] = bottom[4] = bottom[5] + (mBottomBorderSize * 2);

        // Lock the whole position buffer in discard mode
        HardwareVertexBufferSharedPtr vbuf = 
            mRenderOp2.vertexData->vertexBufferBinding->getBuffer(POSITION_BINDING);
        Real* pPos = static_cast<Real*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD) );
        // Use the furthest away depth value, since materials should have depth-check off
        // This initialised the depth buffer for any 3D objects in front
        Real zValue = Root::getSingleton().getRenderSystem()->getMaximumDepthInputValue();
        for (ushort cell = 0; cell < 8; ++cell)
        {
            /*
                0-----2
                |    /|
                |  /  |
                |/    |
                1-----3
            */
            *pPos++ = left[cell];
            *pPos++ = top[cell];
            *pPos++ = zValue;

            *pPos++ = left[cell];
            *pPos++ = bottom[cell];
            *pPos++ = zValue;

            *pPos++ = right[cell];
            *pPos++ = top[cell];
            *pPos++ = zValue;

            *pPos++ = right[cell];
            *pPos++ = bottom[cell];
            *pPos++ = zValue;

        }
        vbuf->unlock();

        // Also update center geometry
        // NB don't use superclass because we need to make it smaller because of border
        vbuf = mRenderOp.vertexData->vertexBufferBinding->getBuffer(POSITION_BINDING);
        pPos = static_cast<Real*>(
            vbuf->lock(HardwareBuffer::HBL_DISCARD) );
        // Use cell 1 and 3 to determine positions
        *pPos++ = left[1];
        *pPos++ = top[3];
        *pPos++ = zValue;

        *pPos++ = left[1];
        *pPos++ = bottom[3];
        *pPos++ = zValue;

        *pPos++ = right[1];
        *pPos++ = top[3];
        *pPos++ = zValue;

        *pPos++ = right[1];
        *pPos++ = bottom[3];
        *pPos++ = zValue;

        vbuf->unlock();
        
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::_updateRenderQueue(RenderQueue* queue)
    {
        // Add self twice to the queue
        // Have to do this to allow 2 materials
        if (mVisible)
        {

            // Add outer
            queue->addRenderable(mBorderRenderable, RENDER_QUEUE_OVERLAY, mZOrder);

			// do inner last so the border artifacts don't overwrite the children
            // Add inner
            PanelGuiElement::_updateRenderQueue(queue);
        }
    }
    //-----------------------------------------------------------------------
    void BorderPanelGuiElement::setMetricsMode(GuiMetricsMode gmm)
    {
        PanelGuiElement::setMetricsMode(gmm);
        if (gmm != GMM_RELATIVE)
        {
            mPixelBottomBorderSize = mBottomBorderSize;
            mPixelLeftBorderSize = mLeftBorderSize;
            mPixelRightBorderSize = mRightBorderSize;
            mPixelTopBorderSize = mTopBorderSize;
        }
    }
    //-----------------------------------------------------------------------
    void BorderPanelGuiElement::_update(void)
    {
        PanelGuiElement::_update();

        if (mMetricsMode != GMM_RELATIVE && 
            (OverlayManager::getSingleton().hasViewportChanged() || mGeomPositionsOutOfDate))
        {
            mLeftBorderSize = mPixelLeftBorderSize * mPixelScaleX;
            mRightBorderSize = mPixelRightBorderSize * mPixelScaleX;
            mTopBorderSize = mPixelTopBorderSize * mPixelScaleY;
            mBottomBorderSize = mPixelBottomBorderSize * mPixelScaleY;
            mGeomPositionsOutOfDate = true;
        }
    }
    //-----------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // Command objects
    //---------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderSize::doGet(const void* target) const
    {
		const BorderPanelGuiElement* t = static_cast<const BorderPanelGuiElement*>(target);
        return String(
			StringConverter::toString(t->getLeftBorderSize()) + " " +
			StringConverter::toString(t->getRightBorderSize()) + " " +
			StringConverter::toString(t->getTopBorderSize()) + " " +
			StringConverter::toString(t->getBottomBorderSize())	);
    }
    void BorderPanelGuiElement::CmdBorderSize::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderPanelGuiElement*>(target)->setBorderSize(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderMaterial::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const BorderPanelGuiElement*>(target)->getBorderMaterialName();
    }
    void BorderPanelGuiElement::CmdBorderMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderPanelGuiElement*>(target)->setBorderMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderBottomLeftUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelGuiElement*>(target)->getBottomLeftBorderUVString();
    }
    void BorderPanelGuiElement::CmdBorderBottomLeftUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderPanelGuiElement*>(target)->setBottomLeftBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderBottomRightUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelGuiElement*>(target)->getBottomRightBorderUVString();
    }
    void BorderPanelGuiElement::CmdBorderBottomRightUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderPanelGuiElement*>(target)->setBottomRightBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderTopLeftUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelGuiElement*>(target)->getTopLeftBorderUVString();
    }
    void BorderPanelGuiElement::CmdBorderTopLeftUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderPanelGuiElement*>(target)->setTopLeftBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderTopRightUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelGuiElement*>(target)->getTopRightBorderUVString();
    }
    void BorderPanelGuiElement::CmdBorderTopRightUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderPanelGuiElement*>(target)->setTopRightBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderLeftUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelGuiElement*>(target)->getLeftBorderUVString();
    }
    void BorderPanelGuiElement::CmdBorderLeftUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderPanelGuiElement*>(target)->setLeftBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderRightUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelGuiElement*>(target)->getRightBorderUVString();
    }
    void BorderPanelGuiElement::CmdBorderRightUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderPanelGuiElement*>(target)->setRightBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderTopUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelGuiElement*>(target)->getTopBorderUVString();
    }
    void BorderPanelGuiElement::CmdBorderTopUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderPanelGuiElement*>(target)->setTopBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderBottomUV::doGet(const void* target) const
    {
        // No need right now..
		return  static_cast<const BorderPanelGuiElement*>(target)->getBottomBorderUVString();
    }
    void BorderPanelGuiElement::CmdBorderBottomUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderPanelGuiElement*>(target)->setBottomBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //---------------------------------------------------------------------
    const String& BorderPanelGuiElement::getTypeName(void) const
    {
        return msTypeName;
    }



}


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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/

#include "OgreBorderPanelGuiElement.h"
#include "OgreMaterialManager.h"
#include "OgreMaterial.h"
#include "OgreStringConverter.h"

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

    #define BCELL_UV(x) mRenderOp2.pTexCoords[0] + (x * 4 * 2)
    //---------------------------------------------------------------------
    BorderPanelGuiElement::BorderPanelGuiElement(const String& name)
        : PanelGuiElement(name)
    {
        // superclass will handle the interior panel area 

        // Setup render op in advance
        // TODO make this more VB friendly
        mRenderOp2.numTextureCoordSets = 1;
        mRenderOp2.numTextureDimensions[0] = 2;
        mRenderOp2.numVertices = 4 * 8; // 8 cells, can't necessarily share vertices cos
                                        // texcoords may differ
        mRenderOp2.operationType = RenderOperation::OT_TRIANGLE_LIST;
        // Only 1 set of texcoords allowed
        mRenderOp2.pTexCoords[0] = new Real[4 * 8 * 2];
        mRenderOp2.texCoordStride[0] = 0;
        mRenderOp2.numTextureDimensions[0] = 2;

        mRenderOp2.pVertices = new Real[4 * 8 * 3];
        mRenderOp2.useIndexes = true;
        mRenderOp2.numIndexes = 8 * 6;
        // No normals or colours
        mRenderOp2.vertexOptions = RenderOperation::VO_TEXTURE_COORDS;
        mRenderOp2.vertexStride = 0;

        /* Each cell is
            0-----2
            |    /|
            |  /  |
            |/    |
            1-----3
        */
        mRenderOp2.pIndexes = new ushort[6 * 8];
        ushort* pIdx = mRenderOp2.pIndexes;
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

        if (createParamDictionary("BorderPanelGuiElement"))
        {
            addBaseParameters();
        }

        // Create sub-object for rendering border
        mBorderRenderable = new BorderRenderable(this);
    }
    //---------------------------------------------------------------------
    BorderPanelGuiElement::~BorderPanelGuiElement()
    {
        delete mRenderOp2.pTexCoords[0];
        delete mRenderOp2.pVertices;
        delete mRenderOp2.pIndexes;
        delete mBorderRenderable;
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
        mLeftBorderSize = mRightBorderSize = 
            mTopBorderSize = mBottomBorderSize = size;
        updatePositionGeometry();
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setBorderSize(Real sides, Real topAndBottom)
    {
        mLeftBorderSize = mRightBorderSize = sides;
        mTopBorderSize = mBottomBorderSize = topAndBottom;
        updatePositionGeometry();

    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::setBorderSize(Real left, Real right, Real top, Real bottom)
    {
        mLeftBorderSize = left;
        mRightBorderSize = right;
        mTopBorderSize = top;
        mBottomBorderSize = bottom;
        updatePositionGeometry();
    }
    //---------------------------------------------------------------------
    Real BorderPanelGuiElement::getLeftBorderSize(void)
    {
        return mLeftBorderSize;
    }
    //---------------------------------------------------------------------
    Real BorderPanelGuiElement::getRightBorderSize(void)
    {
        return mRightBorderSize;
    }
    //---------------------------------------------------------------------
    Real BorderPanelGuiElement::getTopBorderSize(void)
    {
        return mTopBorderSize;
    }
    //---------------------------------------------------------------------
    Real BorderPanelGuiElement::getBottomBorderSize(void)
    {
        return mBottomBorderSize;
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
        Real* pUV = BCELL_UV(idx);

        *pUV++ = u1; *pUV++ = v1;
        *pUV++ = u1; *pUV++ = v2;
        *pUV++ = u2; *pUV++ = v1;
        *pUV++ = u2; *pUV++ = v2;
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
    void BorderPanelGuiElement::setBorderMaterialName(const String& name)
    {
        mBorderMaterialName = name;
        mpBorderMaterial = (Material*)MaterialManager::getSingleton().getByName(name);
        assert(mpBorderMaterial);
        mpBorderMaterial->load();
        // Set some prerequisites to be sure
        mpBorderMaterial->setLightingEnabled(false);
        mpBorderMaterial->setDepthCheckEnabled(false);

    }
    //---------------------------------------------------------------------
    const String& BorderPanelGuiElement::getBorderMaterialName(void)
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

        Real* pPos = mRenderOp2.pVertices;
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
            *pPos++ = -1;

            *pPos++ = left[cell];
            *pPos++ = bottom[cell];
            *pPos++ = -1;

            *pPos++ = right[cell];
            *pPos++ = top[cell];
            *pPos++ = -1;

            *pPos++ = right[cell];
            *pPos++ = bottom[cell];
            *pPos++ = -1;

        }

        // Also update center geometry
        // NB don't use superclass because we need to make it smaller because of border
        pPos = mRenderOp.pVertices;
        // Use cell 1 and 3 to determine positions
        *pPos++ = left[1];
        *pPos++ = top[3];
        *pPos++ = -1;

        *pPos++ = left[1];
        *pPos++ = bottom[3];
        *pPos++ = -1;

        *pPos++ = right[1];
        *pPos++ = top[3];
        *pPos++ = -1;

        *pPos++ = right[1];
        *pPos++ = bottom[3];
        *pPos++ = -1;

        
    }
    //---------------------------------------------------------------------
    void BorderPanelGuiElement::_updateRenderQueue(RenderQueue* queue)
    {
        // Add self twice to the queue
        // Have to do this to allow 2 materials
        if (mVisible)
        {
            // Add inner
            PanelGuiElement::_updateRenderQueue(queue);

            // Add outer
            queue->addRenderable(mBorderRenderable, RENDER_QUEUE_OVERLAY, mZOrder);
        }
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // Command objects
    //---------------------------------------------------------------------
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderSize::doGet(void* target)
    {
        // No need right now..
        return String("");
    }
    void BorderPanelGuiElement::CmdBorderSize::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<BorderPanelGuiElement*>(target)->setBorderSize(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderMaterial::doGet(void* target)
    {
        // No need right now..
        return static_cast<BorderPanelGuiElement*>(target)->getBorderMaterialName();
    }
    void BorderPanelGuiElement::CmdBorderMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<BorderPanelGuiElement*>(target)->setBorderMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderBottomLeftUV::doGet(void* target)
    {
        // No need right now..
        return String("");
    }
    void BorderPanelGuiElement::CmdBorderBottomLeftUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<BorderPanelGuiElement*>(target)->setBottomLeftBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderBottomRightUV::doGet(void* target)
    {
        // No need right now..
        return String("");
    }
    void BorderPanelGuiElement::CmdBorderBottomRightUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<BorderPanelGuiElement*>(target)->setBottomRightBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderTopLeftUV::doGet(void* target)
    {
        // No need right now..
        return String("");
    }
    void BorderPanelGuiElement::CmdBorderTopLeftUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<BorderPanelGuiElement*>(target)->setTopLeftBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderTopRightUV::doGet(void* target)
    {
        // No need right now..
        return String("");
    }
    void BorderPanelGuiElement::CmdBorderTopRightUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<BorderPanelGuiElement*>(target)->setTopRightBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderLeftUV::doGet(void* target)
    {
        // No need right now..
        return String("");
    }
    void BorderPanelGuiElement::CmdBorderLeftUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<BorderPanelGuiElement*>(target)->setLeftBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderRightUV::doGet(void* target)
    {
        // No need right now..
        return String("");
    }
    void BorderPanelGuiElement::CmdBorderRightUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<BorderPanelGuiElement*>(target)->setRightBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderTopUV::doGet(void* target)
    {
        // No need right now..
        return String("");
    }
    void BorderPanelGuiElement::CmdBorderTopUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<BorderPanelGuiElement*>(target)->setTopBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
    //-----------------------------------------------------------------------
    String BorderPanelGuiElement::CmdBorderBottomUV::doGet(void* target)
    {
        // No need right now..
        return String("");
    }
    void BorderPanelGuiElement::CmdBorderBottomUV::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<BorderPanelGuiElement*>(target)->setBottomBorderUV(
            StringConverter::parseReal(vec[0]),
            StringConverter::parseReal(vec[1]),
            StringConverter::parseReal(vec[2]),
            StringConverter::parseReal(vec[3])
            );
    }
}


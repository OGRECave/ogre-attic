/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
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

#include "OgrePanelGuiElement.h"
#include "OgreMaterial.h"
#include "OgreStringConverter.h"


namespace Ogre {
    //---------------------------------------------------------------------
    String PanelGuiElement::msTypeName = "Panel";
    PanelGuiElement::CmdTiling PanelGuiElement::msCmdTiling;
    PanelGuiElement::CmdTransparent PanelGuiElement::msCmdTransparent;
    //---------------------------------------------------------------------
    PanelGuiElement::PanelGuiElement(const String& name)
        : GuiContainer(name)
    {
        mTransparent = false;
        // Setup render op in advance
        // TODO make this more VB friendly
        mRenderOp.numTextureCoordSets = 1;
        mRenderOp.numTextureDimensions[0] = 2;
        mRenderOp.numVertices = 4;
        mRenderOp.operationType = RenderOperation::OT_TRIANGLE_STRIP;
        for (ushort i = 0; i < OGRE_MAX_TEXTURE_COORD_SETS; ++i)
        {
            mTileX[i] = 1.0f;
            mTileY[i] = 1.0f;
            mRenderOp.pTexCoords[i] = 0;
            mRenderOp.texCoordStride[i] = 0;
            mRenderOp.numTextureDimensions[i] = 2;
        }
        mRenderOp.pVertices = new Real[4*3];
        mRenderOp.useIndexes = false;
        // No normals or colours
        mRenderOp.vertexOptions = RenderOperation::VO_TEXTURE_COORDS;
        mRenderOp.vertexStride = 0;

        if (createParamDictionary("PanelGuiElement"))
        {
            addBaseParameters();
        }
    }
    //---------------------------------------------------------------------
    PanelGuiElement::~PanelGuiElement()
    {
        delete mRenderOp.pVertices;
        for (ushort i = 0; i < OGRE_MAX_TEXTURE_LAYERS; ++i)
        {
            if (mRenderOp.pTexCoords[i])
            {
                delete mRenderOp.pTexCoords[i];
            }
        }

    }
    //---------------------------------------------------------------------
    void PanelGuiElement::setTiling(Real x, Real y, ushort layer)
    {
        assert (layer >= 0 && layer < OGRE_MAX_TEXTURE_COORD_SETS);
        assert (x != 0 && y != 0);

        mTileX[layer] = x;
        mTileY[layer] = y;

        updateTextureGeometry();

    }
    //---------------------------------------------------------------------
    Real PanelGuiElement::getTileX(ushort layer)
    {
        return mTileX[layer];
    }
    //---------------------------------------------------------------------
    Real PanelGuiElement::getTileY(ushort layer)
    {
        return mTileY[layer];
    }
    //---------------------------------------------------------------------
    void PanelGuiElement::setTransparent(bool isTransparent)
    {
        mTransparent = isTransparent;
    }
    //---------------------------------------------------------------------
    bool PanelGuiElement::isTransparent(void)
    {
        return mTransparent;
    }
    //---------------------------------------------------------------------
    const String& PanelGuiElement::getTypeName(void)
    {
        return msTypeName;
    }
    //---------------------------------------------------------------------
    void PanelGuiElement::getRenderOperation(RenderOperation& rend)
    {

        rend = mRenderOp;

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

            if (!mTransparent)
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

        // Convert positions into -1, 1 coordinate space (homogenous clip space)
        // Left / right is simple range conversion
        // Top / bottom also need inverting since y is upside down
        left = _getDerivedLeft() * 2 - 1;
        right = left + (mWidth * 2);
        top = -((_getDerivedTop() * 2) - 1);
        bottom =  top -  (mHeight * 2);

        Real* pPos = mRenderOp.pVertices;
        
        // Use -1 for Z position, furthest forward in homogenous clip space
        *pPos++ = left;
        *pPos++ = top;
        *pPos++ = -1;

        *pPos++ = left;
        *pPos++ = bottom;
        *pPos++ = -1;

        *pPos++ = right;
        *pPos++ = top;
        *pPos++ = -1;

        *pPos++ = right;
        *pPos++ = bottom;
        *pPos++ = -1;
    }
    //---------------------------------------------------------------------
    void PanelGuiElement::updateTextureGeometry(void)
    {
        // Generate for as many texture layers as there are in material
        if (mpMaterial)
        {
            ushort numLayers = mpMaterial->getNumTextureLayers();
            mRenderOp.numTextureCoordSets = numLayers;
            for (ushort i = 0; i < numLayers; ++i)
            {
                // Allocate if necessary
                if (mRenderOp.pTexCoords[i] == 0)
                {
                    mRenderOp.pTexCoords[i] = new Real[4*2];
                }
                // Calc upper tex coords
                Real upperX = 1.0f / mTileX[i];
                Real upperY = 1.0f / mTileY[i];
                /*
                    0-----2
                    |    /|
                    |  /  |
                    |/    |
                    1-----3
                */
                Real* pTex = mRenderOp.pTexCoords[i];

                *pTex++ = 0.0f;
                *pTex++ = upperY;

                *pTex++ = 0.0f;
                *pTex++ = 0.0f;

                *pTex++ = upperX;
                *pTex++ = upperY;

                *pTex++ = upperX;
                *pTex++ = 0.0f;


            }
        }
    }
    //-----------------------------------------------------------------------
    void PanelGuiElement::addBaseParameters(void)
    {
        GuiElement::addBaseParameters();
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
    String PanelGuiElement::CmdTiling::doGet(void* target)
    {
        // NB only returns 1st layer tiling
        String ret = "0 " + StringConverter::toString(
            static_cast<PanelGuiElement*>(target)->getTileX() );
        ret += " " + StringConverter::toString(
            static_cast<PanelGuiElement*>(target)->getTileY() );
        return ret;
    }
    void PanelGuiElement::CmdTiling::doSet(void* target, const String& val)
    {
        // 3 params: <layer> <x_tile> <y_tile>
        // Param count is validated higher up
        std::vector<String> vec = val.split();
        ushort layer = (ushort)StringConverter::parseUnsignedInt(vec[0]);
        Real x_tile = StringConverter::parseReal(vec[1]);
        Real y_tile = StringConverter::parseReal(vec[2]);

        static_cast<PanelGuiElement*>(target)->setTiling(x_tile, y_tile, layer);
    }
    //-----------------------------------------------------------------------
    String PanelGuiElement::CmdTransparent::doGet(void* target)
    {
        return StringConverter::toString(
            static_cast<PanelGuiElement*>(target)->isTransparent() );
    }
    void PanelGuiElement::CmdTransparent::doSet(void* target, const String& val)
    {
        static_cast<PanelGuiElement*>(target)->setTransparent(
            StringConverter::parseBool(val));
    }


}




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


namespace Ogre {
    //---------------------------------------------------------------------
    String PanelGuiElement::msTypeName = "Panel";
    //---------------------------------------------------------------------
    PanelGuiElement::PanelGuiElement(const String& name)
        : GuiContainer(name)
    {
        mTransparent = 0;
        for (ushort i = 0; i < OGRE_MAX_TEXTURE_LAYERS; ++i)
        {
            mTileX[i] = 1.0f;
            mTileY[i] = 1.0f;
        }


    }
    //---------------------------------------------------------------------
    void PanelGuiElement::setTiling(Real x, Real y, ushort layer)
    {
        assert (layer >= 0 && layer < OGRE_MAX_TEXTURE_LAYERS);

        mTileX[layer] = x;
        mTileY[layer] = y;

        updateTextureGeometry();

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
    }
    //---------------------------------------------------------------------
    void PanelGuiElement::updateTextureGeometry(void)
    {
    }

}




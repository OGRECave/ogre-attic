
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/

#include "OgreGuiElement.h"
#include "OgreMaterialManager.h"
#include "OgreOverlay.h"
#include "OgreGuiContainer.h"


namespace Ogre {


    //---------------------------------------------------------------------
    // Define static members
    GuiElementCommands::CmdLeft GuiElement::msLeftCmd;
    GuiElementCommands::CmdTop GuiElement::msTopCmd;
    GuiElementCommands::CmdWidth GuiElement::msWidthCmd;
    GuiElementCommands::CmdHeight GuiElement::msHeightCmd;
    GuiElementCommands::CmdMaterial GuiElement::msMaterialCmd;
    GuiElementCommands::CmdCaption GuiElement::msCaptionCmd;
    //---------------------------------------------------------------------
    GuiElement::GuiElement(const String& name)
        : mName(name)
    {
        mParent = 0;
        mLeft = 0.0f;
        mTop = 0.0f;
        mWidth = 1.0f;
        mHeight = 1.0f;
        mVisible = true;
        mpMaterial = 0;
        mDerivedOutOfDate = true;
        mZOrder = 0;
       
    }
    //---------------------------------------------------------------------
    GuiElement::~GuiElement()
    {
    }
    //---------------------------------------------------------------------
    const String& GuiElement::getName(void) const
    {
        return mName;
    }
    //---------------------------------------------------------------------
    void GuiElement::show(void)
    {
        mVisible = true;
    }
    //---------------------------------------------------------------------
    void GuiElement::hide(void)
    {
        mVisible = false;
    }
    //---------------------------------------------------------------------
    bool GuiElement::isVisible(void)
    {
        return mVisible;
    }
    //---------------------------------------------------------------------
    void GuiElement::setDimensions(Real width, Real height)
    {
        mWidth = width;
        mHeight = height;
        mDerivedOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void GuiElement::setPosition(Real left, Real top)
    {
        mLeft = left;
        mTop = top;
        mDerivedOutOfDate = true;
    }
    //---------------------------------------------------------------------
    void GuiElement::setWidth(Real width)
    {
        mWidth = width;
    }
    //---------------------------------------------------------------------
    Real GuiElement::getWidth(void) const
    {
        return mWidth;
    }
    //---------------------------------------------------------------------
    void GuiElement::setHeight(Real height)
    {
        mHeight = height;
    }
    //---------------------------------------------------------------------
    Real GuiElement::getHeight(void) const
    {
        return mHeight;
    }
    //---------------------------------------------------------------------
    void GuiElement::setLeft(Real left)
    {
        mLeft = left;
        mDerivedOutOfDate = true;
    }
    //---------------------------------------------------------------------
    Real GuiElement::getLeft(void) const
    {
        return mLeft;
    }
    //---------------------------------------------------------------------
    void GuiElement::setTop(Real top)
    {
        mTop = top;
        mDerivedOutOfDate = true;
    }
    //---------------------------------------------------------------------
    Real GuiElement::getTop(void) const
    {
        return mTop;
    }
    //---------------------------------------------------------------------
    const String& GuiElement::getMaterialName(void) const
    {
        return mMaterialName;

    }
    //---------------------------------------------------------------------
    void GuiElement::setMaterialName(const String& matName)
    {
        mMaterialName = matName;
        mpMaterial = (Material*)MaterialManager::getSingleton().getByName(matName);
        assert(mpMaterial);
        mpMaterial->load();
        // Set some prerequisites to be sure
        mpMaterial->setLightingEnabled(false);
        mpMaterial->setDepthCheckEnabled(false);

    }
    //---------------------------------------------------------------------
    Material* GuiElement::getMaterial(void) const
    {
        return mpMaterial;
    }
    //---------------------------------------------------------------------
    void GuiElement::getWorldTransforms(Matrix4* xform)
    {
        mOverlay->_getWorldTransforms(xform);
    }
    //---------------------------------------------------------------------
    bool GuiElement::useIdentityProjection(void)
    {
        return true;
    }
    //---------------------------------------------------------------------
    bool GuiElement::useIdentityView(void)
    {
        return true;
    }
    //---------------------------------------------------------------------
    void GuiElement::_update(void)
    {
        _updateFromParent();
        // NB container subclasses will update children too

        // Tell self to update own position geometry
        updatePositionGeometry();
    }
    //---------------------------------------------------------------------
    void GuiElement::_updateFromParent(void)
    {
        if (mParent)
        {
            mDerivedLeft = mParent->_getDerivedLeft() + mLeft;
            mDerivedTop = mParent->_getDerivedTop() + mTop;
        }
        else
        {
            mDerivedLeft = mLeft;
            mDerivedTop = mTop;
        }
        mDerivedOutOfDate = false;

    }
    //---------------------------------------------------------------------
    void GuiElement::_notifyParent(GuiContainer* parent, Overlay* overlay)
    {
        mParent = parent;
        mOverlay = overlay;

        mDerivedOutOfDate = true;
    }
    //---------------------------------------------------------------------
    Real GuiElement::_getDerivedLeft(void)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        return mDerivedLeft;
    }
    //---------------------------------------------------------------------
    Real GuiElement::_getDerivedTop(void)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        return mDerivedTop;
    }
    //---------------------------------------------------------------------
    void GuiElement::_notifyZOrder(ushort newZOrder)
    {
        mZOrder = newZOrder;
    }
    //---------------------------------------------------------------------
    void GuiElement::_updateRenderQueue(RenderQueue* queue)
    {
        if (mVisible)
        {
            queue->addRenderable(this, RENDER_QUEUE_OVERLAY, mZOrder);
        }
      
    }
    //-----------------------------------------------------------------------
    void GuiElement::addBaseParameters(void)    
    {
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("left", 
            "The position of the left border of the gui element as a proportion of screen space."
            , PT_REAL),
            &msLeftCmd);
        dict->addParameter(ParameterDef("top", 
            "The position of the top border of the gui element as a proportion of screen space."
            , PT_REAL),
            &msTopCmd);
        dict->addParameter(ParameterDef("width", 
            "The width of the element as a proportion of screen space."
            , PT_REAL),
            &msWidthCmd);
        dict->addParameter(ParameterDef("height", 
            "The height of the element as a proportion of screen space."
            , PT_REAL),
            &msHeightCmd);
        dict->addParameter(ParameterDef("material", 
            "The name of the material to use."
            , PT_STRING),
            &msMaterialCmd);
        dict->addParameter(ParameterDef("caption", 
            "The element caption, if supported."
            , PT_STRING),
            &msCaptionCmd);
    }
    //-----------------------------------------------------------------------
    void GuiElement::setCaption( const String& caption )
    {
        mCaption = caption;
    }
    //-----------------------------------------------------------------------
    const String& GuiElement::getCaption() const
    {
        return mCaption;
    }




}



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
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#include "OgreGuiElement.h"
#include "OgreMaterialManager.h"
#include "OgreOverlay.h"
#include "OgreGuiContainer.h"
#include "OgreMouseEvent.h"
#include "OgreEventMulticaster.h"
#include "OgreEventListeners.h"
#include "OgreOverlayManager.h"

namespace Ogre {


    //---------------------------------------------------------------------
    // Define static members
    GuiElementCommands::CmdLeft GuiElement::msLeftCmd;
    GuiElementCommands::CmdTop GuiElement::msTopCmd;
    GuiElementCommands::CmdWidth GuiElement::msWidthCmd;
    GuiElementCommands::CmdHeight GuiElement::msHeightCmd;
    GuiElementCommands::CmdMaterial GuiElement::msMaterialCmd;
    GuiElementCommands::CmdCaption GuiElement::msCaptionCmd;
    GuiElementCommands::CmdMetricsMode GuiElement::msMetricsModeCmd;
    GuiElementCommands::CmdHorizontalAlign GuiElement::msHorizontalAlignCmd;
    GuiElementCommands::CmdVerticalAlign GuiElement::msVerticalAlignCmd;
    //---------------------------------------------------------------------
    GuiElement::GuiElement(const String& name)
        : mName(name),
		MouseTarget(),
		MouseMotionTarget()
    {
        mParent = 0;
        mLeft = 0.0f;
        mTop = 0.0f;
        mWidth = 1.0f;
        mHeight = 1.0f;
		mMouseListener = 0;
        mVisible = true;
        mpMaterial = 0;
        mDerivedOutOfDate = true;
        mZOrder = 0;
		mCloneable = true;
        mMetricsMode = GMM_RELATIVE;
        mHorzAlign = GHA_LEFT;
        mVertAlign = GVA_TOP;
        mGeomPositionsOutOfDate = true;       
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
        if (mMetricsMode == GMM_PIXELS)
        {
            mPixelWidth = (short int)width;
            mPixelHeight = (short int)height;
        }
        else
        {
            mWidth = width;
            mHeight = height;
        }
        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    void GuiElement::setPosition(Real left, Real top)
    {
        if (mMetricsMode == GMM_PIXELS)
        {
            mPixelLeft = (short int)left;
            mPixelTop = (short int)top;
        }
        else
        {
            mLeft = left;
            mTop = top;
        }
        mDerivedOutOfDate = true;
        _positionsOutOfDate();

    }
    //---------------------------------------------------------------------
    void GuiElement::setWidth(Real width)
    {
        if (mMetricsMode == GMM_PIXELS)
        {
            mPixelWidth = (short int)width;
        }
        else
        {
            mWidth = width;
        }
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    Real GuiElement::getWidth(void) const
    {
        return mWidth;
    }
    //---------------------------------------------------------------------
    void GuiElement::setHeight(Real height)
    {
        if (mMetricsMode == GMM_PIXELS)
        {
            mPixelHeight = (short int)height;
        }
        else
        {
            mHeight = height;
        }
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    Real GuiElement::getHeight(void) const
    {
        return mHeight;
    }
    //---------------------------------------------------------------------
    void GuiElement::setLeft(Real left)
    {
        if (mMetricsMode == GMM_PIXELS)
        {
            mPixelLeft = (short int)left;
        }
        else
        {
            mLeft = left;
        }
        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    Real GuiElement::getLeft(void) const
    {
        return mLeft;
    }
    //---------------------------------------------------------------------
    void GuiElement::setTop(Real top)
    {
        if (mMetricsMode == GMM_PIXELS)
        {
            mPixelTop = (short int)top;
        }
        else
        {
            mTop = top;
        }

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
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
	void GuiElement::_positionsOutOfDate(void)
	{
		mGeomPositionsOutOfDate = true;
	}

    //---------------------------------------------------------------------
    void GuiElement::_update(void)
    {
        // Check size if pixel-based
        if (mMetricsMode == GMM_PIXELS &&
            (mDerivedOutOfDate || OverlayManager::getSingleton().hasViewportChanged()))
        {
            // Derive parametric version of dimensions
            Real vpWidth, vpHeight;
            vpWidth = (Real) (OverlayManager::getSingleton().getViewportWidth());
            vpHeight = (Real) (OverlayManager::getSingleton().getViewportHeight());

            mLeft = (Real) mPixelLeft / vpWidth;
            mWidth = (Real) mPixelWidth / vpWidth;
            mTop = (Real) mPixelTop / vpHeight;
            mHeight = (Real) mPixelHeight / vpHeight;
        }
        _updateFromParent();
        // NB container subclasses will update children too

        // Tell self to update own position geometry
        if (mGeomPositionsOutOfDate)
        {
            updatePositionGeometry();
            mGeomPositionsOutOfDate = false;
        }
    }
    //---------------------------------------------------------------------
    void GuiElement::_updateFromParent(void)
    {
        Real parentLeft, parentTop, parentBottom, parentRight;

        if (mParent)
        {
            parentLeft = mParent->_getDerivedLeft();
            parentTop = mParent->_getDerivedTop();
            if (mHorzAlign == GHA_CENTER || mHorzAlign == GHA_RIGHT)
            {
                parentRight = parentLeft + mParent->getWidth();
            }
            if (mVertAlign == GVA_CENTER || mVertAlign == GVA_BOTTOM)
            {
                parentBottom = parentTop + mParent->getHeight();
            }

        }
        else
        {
            parentLeft = parentTop = 0.0f;
            parentRight = parentBottom = 1.0f;
        }

        // Sort out position based on alignment
        // NB all we do is derived the origin, we don't automatically sort out the position
        // This is more flexible than forcing absolute right & middle 
        switch(mHorzAlign)
        {
        case GHA_CENTER:
            mDerivedLeft = ((parentLeft + parentRight) * 0.5f) + mLeft;
            break;
        case GHA_LEFT:
            mDerivedLeft = parentLeft + mLeft;
            break;
        case GHA_RIGHT:
            mDerivedLeft = parentRight + mLeft;
            break;
        };
        switch(mVertAlign)
        {
        case GVA_CENTER:
            mDerivedTop = ((parentTop + parentBottom) * 0.5f) + mTop;
            break;
        case GVA_TOP:
            mDerivedTop = parentTop + mTop;
            break;
        case GVA_BOTTOM:
            mDerivedTop = parentBottom + mTop;
            break;
        };

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
            "The position of the left border of the gui element."
            , PT_REAL),
            &msLeftCmd);
        dict->addParameter(ParameterDef("top", 
            "The position of the top border of the gui element."
            , PT_REAL),
            &msTopCmd);
        dict->addParameter(ParameterDef("width", 
            "The width of the element."
            , PT_REAL),
            &msWidthCmd);
        dict->addParameter(ParameterDef("height", 
            "The height of the element."
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
        dict->addParameter(ParameterDef("metrics_mode", 
            "The type of metrics to use, either 'relative' to the screen, or 'pixels'."
            , PT_STRING),
            &msMetricsModeCmd);
        dict->addParameter(ParameterDef("horz_align", 
            "The horizontal alignment, 'left', 'right' or 'center'."
            , PT_STRING),
            &msHorizontalAlignCmd);
        dict->addParameter(ParameterDef("vert_align", 
            "The vertical alignment, 'top', 'bottom' or 'center'."
            , PT_STRING),
            &msVerticalAlignCmd);
    }
    //-----------------------------------------------------------------------
    void GuiElement::setCaption( const String& caption )
    {
        mCaption = caption;
        _positionsOutOfDate();
    }
    //-----------------------------------------------------------------------
    const String& GuiElement::getCaption() const
    {
        return mCaption;
    }
    //-----------------------------------------------------------------------
    void GuiElement::setColour(const ColourValue& col)
    {
        mColour = col;
    }
    //-----------------------------------------------------------------------
    ColourValue GuiElement::getColour(void) const
    {
        return mColour;
    }
    //-----------------------------------------------------------------------
    void GuiElement::setMetricsMode(GuiMetricsMode gmm)
    {
        mMetricsMode = gmm;
        if (mMetricsMode == GMM_PIXELS)
        {
            // Copy settings into pixel versions
            // Relative versions will be derived at viewport change time
            mPixelLeft = (short int)mLeft;
            mPixelTop = (short int)mTop;
            mPixelWidth = (short int)mWidth;
            mPixelHeight = (short int)mHeight;
        }
        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //-----------------------------------------------------------------------
    GuiMetricsMode GuiElement::getMetricsMode(void)
    {
        return mMetricsMode;
    }
    //-----------------------------------------------------------------------
    void GuiElement::setHorizontalAlignment(GuiHorizontalAlignment gha)
    {
        mHorzAlign = gha;
        _positionsOutOfDate();
    }
    //-----------------------------------------------------------------------
    GuiHorizontalAlignment GuiElement::getHorizontalAlignment(void)
    {
        return mHorzAlign;
    }
    //-----------------------------------------------------------------------
    void GuiElement::setVerticalAlignment(GuiVerticalAlignment gva)
    {
        mVertAlign = gva;
        _positionsOutOfDate();
    }
    //-----------------------------------------------------------------------
    GuiVerticalAlignment GuiElement::getVerticalAlignment(void)
    {
        return mVertAlign;
    }
    //-----------------------------------------------------------------------


    //-----------------------------------------------------------------------
	bool GuiElement::contains(Real x, Real y) const
	{
	return (x >= mDerivedLeft) && (x < mDerivedLeft + mWidth ) && (y >= mDerivedTop) && (y < mDerivedTop + mHeight );
	}

    //-----------------------------------------------------------------------
	GuiElement* GuiElement::findElementAt(Real x, Real y) 		// relative to parent
	{
		GuiElement* ret = NULL;
		if (contains(x , y ))
		{
			ret = this;
		}
		return ret;
	}

    //-----------------------------------------------------------------------
	void GuiElement::processEvent(InputEvent* e) 
	{

		switch(e->getID()) 
		{
		case MouseEvent::ME_MOUSE_PRESSED:
		case MouseEvent::ME_MOUSE_RELEASED:
		case MouseEvent::ME_MOUSE_CLICKED:
		case MouseEvent::ME_MOUSE_ENTERED:
		case MouseEvent::ME_MOUSE_EXITED:
			processMouseEvent(static_cast<MouseEvent*>(e));
			break;
		case MouseEvent::ME_MOUSE_MOVED:
		case MouseEvent::ME_MOUSE_DRAGGED:
			processMouseMotionEvent(static_cast<MouseEvent*>(e));
			break;
		}
	}

    //-----------------------------------------------------------------------
	PositionTarget* GuiElement::getPositionTargetParent() 
	{ 
		return static_cast<MouseTarget*> (mParent);		// need to choose 1 parent of the EventTarget
	}
    //-----------------------------------------------------------------------
	GuiContainer* GuiElement::getParent() 
	{ 
		return mParent;		
	}

    void GuiElement::copyFromTemplate(GuiElement* templateGui)
	{
		templateGui->copyParametersTo(this);
		return;
	}

}


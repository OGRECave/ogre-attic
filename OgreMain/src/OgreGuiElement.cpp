
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreStableHeaders.h"

#include "OgreGuiElement.h"
#include "OgreGuiManager.h"
#include "OgreMaterialManager.h"
#include "OgreOverlay.h"
#include "OgreGuiContainer.h"
#include "OgreMouseEvent.h"
#include "OgreEventMulticaster.h"
#include "OgreEventListeners.h"
#include "OgreOverlayManager.h"
#include "OgreException.h"
#include "OgreRenderQueue.h"

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
    GuiElementCommands::CmdVisible GuiElement::msVisibleCmd;
    //---------------------------------------------------------------------
    GuiElement::GuiElement(const String& name)
        : MouseTarget(),
          MouseMotionTarget(),
          mName(name)
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
		mEnabled = true;
        mPixelLeft = 0.0;
        mPixelTop = 0.0;
        mPixelWidth = 1.0;
        mPixelHeight = 1.0;
        mPixelScaleX = 1.0;
        mPixelScaleY = 1.0;
        mSourceTemplate = NULL;
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
    bool GuiElement::isVisible(void) const
    {
        return mVisible;
    }
    //---------------------------------------------------------------------
    void GuiElement::setDimensions(Real width, Real height)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelWidth = width;
            mPixelHeight = height;
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
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelLeft = left;
            mPixelTop = top;
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
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelWidth = width;
        }
        else
        {
            mWidth = width;
        }
        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    Real GuiElement::getWidth(void) const
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
			return mPixelWidth;
		}
		else
		{
        	return mWidth;
		}
    }
    //---------------------------------------------------------------------
    void GuiElement::setHeight(Real height)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelHeight = height;
        }
        else
        {
            mHeight = height;
        }
        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    Real GuiElement::getHeight(void) const
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
			return mPixelHeight;
		}
		else
		{
			return mHeight;
		}
    }
    //---------------------------------------------------------------------
    void GuiElement::setLeft(Real left)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelLeft = left;
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
        if (mMetricsMode != GMM_RELATIVE)
        {
			return mPixelLeft;
		}
		else
		{
        	return mLeft;
		}
    }
    //---------------------------------------------------------------------
    void GuiElement::setTop(Real top)
    {
        if (mMetricsMode != GMM_RELATIVE)
        {
            mPixelTop = top;
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
        if (mMetricsMode != GMM_RELATIVE)
        {
			return mPixelTop;
		}
		else
		{
	        return mTop;
		}
    }
    //---------------------------------------------------------------------
    void GuiElement::_setLeft(Real left)
    {
        mLeft = left;
        mPixelLeft = left / mPixelScaleX;

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    void GuiElement::_setTop(Real top)
    {
        mTop = top;
        mPixelTop = top / mPixelScaleY;

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    void GuiElement::_setWidth(Real width)
    {
        mWidth = width;
        mPixelWidth = width / mPixelScaleX;

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    void GuiElement::_setHeight(Real height)
    {
        mHeight = height;
        mPixelHeight = height / mPixelScaleY;

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    void GuiElement::_setPosition(Real left, Real top)
    {
        mLeft = left;
        mTop  = top;
        mPixelLeft = left / mPixelScaleX;
        mPixelTop  = top / mPixelScaleY;

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //---------------------------------------------------------------------
    void GuiElement::_setDimensions(Real width, Real height)
    {
        mWidth  = width;
        mHeight = height;
        mPixelWidth  = width / mPixelScaleX;
        mPixelHeight = height / mPixelScaleY;

        mDerivedOutOfDate = true;
        _positionsOutOfDate();
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
		if (!mpMaterial)
			Except( Exception::ERR_ITEM_NOT_FOUND, "Could not find material " + matName,
				"GuiElement::setMaterialName" );
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
    void GuiElement::getWorldTransforms(Matrix4* xform) const
    {
        mOverlay->_getWorldTransforms(xform);
    }
    //-----------------------------------------------------------------------
    const Quaternion& GuiElement::getWorldOrientation(void) const
    {
        return mOverlay->getWorldOrientation();
    }
    //-----------------------------------------------------------------------
    const Vector3& GuiElement::getWorldPosition(void) const
    {
        return mOverlay->getWorldPosition();
    }
    //---------------------------------------------------------------------
    bool GuiElement::useIdentityProjection(void) const
    {
        return true;
    }
    //---------------------------------------------------------------------
    bool GuiElement::useIdentityView(void) const
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
        switch (mMetricsMode)
        {
        case GMM_PIXELS :
            if (OverlayManager::getSingleton().hasViewportChanged() || mGeomPositionsOutOfDate)
            {
                Real vpWidth, vpHeight;
                OverlayManager& oMgr = OverlayManager::getSingleton();
                vpWidth = (Real) (oMgr.getViewportWidth());
                vpHeight = (Real) (oMgr.getViewportHeight());

                mPixelScaleX = 1.0 / vpWidth;
                mPixelScaleY = 1.0 / vpHeight;

                mLeft = mPixelLeft * mPixelScaleX;
                mTop = mPixelTop * mPixelScaleY;
                mWidth = mPixelWidth * mPixelScaleX;
                mHeight = mPixelHeight * mPixelScaleY;
            }
            break;

        case GMM_RELATIVE_ASPECT_ADJUSTED :
            if (OverlayManager::getSingleton().hasViewportChanged() || mGeomPositionsOutOfDate)
            {
                Real vpWidth, vpHeight;
                OverlayManager& oMgr = OverlayManager::getSingleton();
                vpWidth = (Real) (oMgr.getViewportWidth());
                vpHeight = (Real) (oMgr.getViewportHeight());

                mPixelScaleX = 1.0 / (10000.0 * (vpWidth / vpHeight));
                mPixelScaleY = 1.0 /  10000.0;

                mLeft = mPixelLeft * mPixelScaleX;
                mTop = mPixelTop * mPixelScaleY;
                mWidth = mPixelWidth * mPixelScaleX;
                mHeight = mPixelHeight * mPixelScaleY;
            }
            break;
        default:
            break;
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

        if (mParent != 0)
        {
            Rectangle parent;
            Rectangle child;

            mParent->_getClippingRegion(parent);

            child.left   = mDerivedLeft;
            child.top    = mDerivedTop;
            child.right  = mDerivedLeft + mWidth;
            child.bottom = mDerivedTop + mHeight;

            mClippingRegion = intersect(parent, child);
        }
        else
        {
            mClippingRegion.left   = mDerivedLeft;
            mClippingRegion.top    = mDerivedTop;
            mClippingRegion.right  = mDerivedLeft + mWidth;
            mClippingRegion.bottom = mDerivedTop + mHeight;
        }
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
    void GuiElement::_getClippingRegion(Rectangle &clippingRegion)
    {
        if (mDerivedOutOfDate)
        {
            _updateFromParent();
        }
        clippingRegion = mClippingRegion;
    }
    //---------------------------------------------------------------------
    void GuiElement::_notifyZOrder(ushort newZOrder)
    {
        mZOrder = newZOrder;
    }

    //---------------------------------------------------------------------
    void GuiElement::_notifyWorldTransforms(const Matrix4& xform)
    {
        mXForm = xform;
    }

    //---------------------------------------------------------------------
    void GuiElement::_notifyViewport()
    {
        switch (mMetricsMode)
        {
        case GMM_PIXELS :
            {
                Real vpWidth, vpHeight;
                OverlayManager& oMgr = OverlayManager::getSingleton();
                vpWidth = (Real) (oMgr.getViewportWidth());
                vpHeight = (Real) (oMgr.getViewportHeight());

                mPixelScaleX = 1.0 / vpWidth;
                mPixelScaleY = 1.0 / vpHeight;
            }
            break;

        case GMM_RELATIVE_ASPECT_ADJUSTED :
            {
                Real vpWidth, vpHeight;
                OverlayManager& oMgr = OverlayManager::getSingleton();
                vpWidth = (Real) (oMgr.getViewportWidth());
                vpHeight = (Real) (oMgr.getViewportHeight());

                mPixelScaleX = 1.0 / (10000.0 * (vpWidth / vpHeight));
                mPixelScaleY = 1.0 /  10000.0;
            }
            break;

        case GMM_RELATIVE :
            mPixelScaleX = 1.0;
            mPixelScaleY = 1.0;
            mPixelLeft = mLeft;
            mPixelTop = mTop;
            mPixelWidth = mWidth;
            mPixelHeight = mHeight;
            break;
        }

        mLeft = mPixelLeft * mPixelScaleX;
        mTop = mPixelTop * mPixelScaleY;
        mWidth = mPixelWidth * mPixelScaleX;
        mHeight = mPixelHeight * mPixelScaleY;

        mGeomPositionsOutOfDate = true;
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
            "The type of metrics to use, either 'relative' to the screen, 'pixels' or 'relative_aspect_adjusted'."
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
        dict->addParameter(ParameterDef("visible", 
            "Initial visibility of element, either 'true' or 'false' (default true)."
            , PT_STRING),
            &msVisibleCmd);
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
    const ColourValue& GuiElement::getColour(void) const
    {
        return mColour;
    }
    //-----------------------------------------------------------------------
    void GuiElement::setMetricsMode(GuiMetricsMode gmm)
    {
        switch (gmm)
        {
        case GMM_PIXELS :
            {
                Real vpWidth, vpHeight;
                OverlayManager& oMgr = OverlayManager::getSingleton();
                vpWidth = (Real) (oMgr.getViewportWidth());
                vpHeight = (Real) (oMgr.getViewportHeight());

                mPixelScaleX = 1.0 / vpWidth;
                mPixelScaleY = 1.0 / vpHeight;

                if (mMetricsMode == GMM_RELATIVE)
                {
                    mPixelLeft = mLeft;
                    mPixelTop = mTop;
                    mPixelWidth = mWidth;
                    mPixelHeight = mHeight;
                }
            }
            break;

        case GMM_RELATIVE_ASPECT_ADJUSTED :
            {
                Real vpWidth, vpHeight;
                OverlayManager& oMgr = OverlayManager::getSingleton();
                vpWidth = (Real) (oMgr.getViewportWidth());
                vpHeight = (Real) (oMgr.getViewportHeight());

                mPixelScaleX = 1.0 / (10000.0 * (vpWidth / vpHeight));
                mPixelScaleY = 1.0 /  10000.0;

                if (mMetricsMode == GMM_RELATIVE)
                {
                    mPixelLeft = mLeft;
                    mPixelTop = mTop;
                    mPixelWidth = mWidth;
                    mPixelHeight = mHeight;
                }
            }
            break;

        case GMM_RELATIVE :
            mPixelScaleX = 1.0;
            mPixelScaleY = 1.0;
            mPixelLeft = mLeft;
            mPixelTop = mTop;
            mPixelWidth = mWidth;
            mPixelHeight = mHeight;
            break;
        }

        mLeft = mPixelLeft * mPixelScaleX;
        mTop = mPixelTop * mPixelScaleY;
        mWidth = mPixelWidth * mPixelScaleX;
        mHeight = mPixelHeight * mPixelScaleY;

        mMetricsMode = gmm;
        mDerivedOutOfDate = true;
        _positionsOutOfDate();
    }
    //-----------------------------------------------------------------------
    GuiMetricsMode GuiElement::getMetricsMode(void) const
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
    GuiHorizontalAlignment GuiElement::getHorizontalAlignment(void) const
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
    GuiVerticalAlignment GuiElement::getVerticalAlignment(void) const
    {
        return mVertAlign;
    }
    //-----------------------------------------------------------------------


    //-----------------------------------------------------------------------
	bool GuiElement::contains(Real x, Real y) const
	{
        return mClippingRegion.inside(x, y);
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

		if (!mEnabled || e->isConsumed())
		{
			return;
		}
		switch(e->getID()) 
		{
		case ActionEvent::AE_ACTION_PERFORMED:
			processActionEvent(static_cast<ActionEvent*>(e));
			break;
		case MouseEvent::ME_MOUSE_PRESSED:
		case MouseEvent::ME_MOUSE_RELEASED:
		case MouseEvent::ME_MOUSE_CLICKED:
		case MouseEvent::ME_MOUSE_ENTERED:
		case MouseEvent::ME_MOUSE_EXITED:
		case MouseEvent::ME_MOUSE_DRAGENTERED:
		case MouseEvent::ME_MOUSE_DRAGEXITED:
		case MouseEvent::ME_MOUSE_DRAGDROPPED:
			processMouseEvent(static_cast<MouseEvent*>(e));
			break;
		case MouseEvent::ME_MOUSE_MOVED:
		case MouseEvent::ME_MOUSE_DRAGGED:
		case MouseEvent::ME_MOUSE_DRAGMOVED:
			processMouseMotionEvent(static_cast<MouseEvent*>(e));
			break;
		}
	}

    //-----------------------------------------------------------------------
	PositionTarget* GuiElement::getPositionTargetParent() const 
	{ 
		return static_cast<MouseTarget*> (mParent);		// need to choose 1 parent of the EventTarget
	}
    //-----------------------------------------------------------------------
	GuiContainer* GuiElement::getParent() 
	{ 
		return mParent;		
	}

    const LightList& GuiElement::getLights(void) const
    {
        // Guielements should not be lit by the scene, this will not get called
        static LightList ll;
        return ll;
    }

    void GuiElement::copyFromTemplate(GuiElement* templateGui)
	{
		templateGui->copyParametersTo(this);
    mSourceTemplate = templateGui ;
		return;
	}

    GuiElement* GuiElement::clone(const String& instanceName)
    {
        GuiElement* newElement;

        newElement = GuiManager::getSingleton().createGuiElement(getTypeName(), instanceName + "/" + mName);
        copyParametersTo(newElement);

        return newElement;
    }

    //-----------------------------------------------------------------------
	bool GuiElement::isEnabled() const
	{ 
		return mEnabled;
	}

    //-----------------------------------------------------------------------
	void GuiElement::setEnabled(bool b) 
	{
		mEnabled = b;
	}


}


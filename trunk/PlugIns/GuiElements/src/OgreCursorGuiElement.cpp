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

#include "OgreCursorGuiElement.h"	
#include "OgreEventProcessor.h"	
#include "OgreInput.h"
#include "OgreOverlayManager.h"
#include "OgreStringConverter.h"


namespace Ogre {

    //---------------------------------------------------------------------
    String CursorGuiElement::msTypeName = "Cursor";

    CursorGuiElement::CmdOffsetX CursorGuiElement::msCmdOffsetX;
    CursorGuiElement::CmdOffsetY CursorGuiElement::msCmdOffsetY;

	CursorGuiElement::CursorGuiElement(const String& name) :
		PanelGuiElement(name), mOffsetX(0.0), mOffsetY(0.0)
	{
        mVisible = false; // cursors are initially hidden

        if (createParamDictionary("CursorGuiElement"))
        {
            addBaseParameters();
        }

		setWidth(0.01);
		setHeight(0.01);
	}
    //---------------------------------------------------------------------
    const String& CursorGuiElement::getTypeName(void) const
    {
        return msTypeName;
    }
    //---------------------------------------------------------------------
    void CursorGuiElement::show(void)
    {
        if (!mVisible) {
            OverlayManager &om = OverlayManager::getSingleton();
       		setLeft(om.getMouseX() - mOffsetX);
	    	setTop(om.getMouseY() - mOffsetY);
            om.addMouseMotionListener(this);
        }
        PanelGuiElement::show();
    }
    //---------------------------------------------------------------------
    void CursorGuiElement::hide(void)
    {
        if (mVisible) {
            OverlayManager &om = OverlayManager::getSingleton();
            om.removeMouseMotionListener(this);
        }
        PanelGuiElement::hide();
    }
    //---------------------------------------------------------------------
	void CursorGuiElement::setOffsetX(Real x)
	{
		setLeft(mLeft + mOffsetX - x);
        mOffsetX = x;
	}
    //---------------------------------------------------------------------
	void CursorGuiElement::setOffsetY(Real y)
	{
		setTop(mTop + mOffsetY - y);
        mOffsetY = y;
	}
    //---------------------------------------------------------------------
	GuiElement* CursorGuiElement::findElementAt(Real x, Real y) 		// relative to parent
	{
		return NULL;	// override this so it won't find itself!
	}
    //---------------------------------------------------------------------
	void CursorGuiElement::mouseMoved(MouseEvent* e)
	{
		setLeft(e->getX() - mOffsetX);
		setTop(e->getY() - mOffsetY);
	}
    //---------------------------------------------------------------------
	void CursorGuiElement::mouseDragged(MouseEvent* e)
	{
		mouseMoved(e);
	}
    //---------------------------------------------------------------------
    void CursorGuiElement::addBaseParameters(void)
    {
        PanelGuiElement::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("x_offset", 
            "Specifies the x offset that will be added to the mouse coordinates.", PT_STRING),
            &msCmdOffsetX);
        dict->addParameter(ParameterDef("y_offset", 
            "Specifies the y offset that will be added to the mouse coordinates.", PT_STRING),
            &msCmdOffsetY);
    }
    //-----------------------------------------------------------------------
    String CursorGuiElement::CmdOffsetX::doGet(const void* target) const
    {
        return StringConverter::toString(
                static_cast<const CursorGuiElement*>(target)->getOffsetX() );
    }
    void CursorGuiElement::CmdOffsetX::doSet(void* target, const String& val)
    {
        static_cast<CursorGuiElement*>(target)->setOffsetX(StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String CursorGuiElement::CmdOffsetY::doGet(const void* target) const
    {
        return StringConverter::toString(
                static_cast<const CursorGuiElement*>(target)->getOffsetY() );
    }
    void CursorGuiElement::CmdOffsetY::doSet(void* target, const String& val)
    {
        static_cast<CursorGuiElement*>(target)->setOffsetY(StringConverter::parseReal(val));
    }

}


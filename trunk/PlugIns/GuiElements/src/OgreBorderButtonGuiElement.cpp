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

#include "OgreBorderButtonGuiElement.h"	
//#include "OgreActionEvent.h"
//#include "OgreMouseEvent.h"
#include "OgreGuiManager.h"


namespace Ogre {

    //-----------------------------------------------------------------------
    String BorderButtonGuiElement::msTypeName = "BorderButton";
	BorderButtonGuiElement::CmdBorderDownMaterial BorderButtonGuiElement::msCmdBorderDownMaterial;
	BorderButtonGuiElement::CmdBorderUpMaterial BorderButtonGuiElement::msCmdBorderUpMaterial;
	BorderButtonGuiElement::CmdBorderHiliteDownMaterial BorderButtonGuiElement::msCmdBorderHiliteDownMaterial;
	BorderButtonGuiElement::CmdBorderHiliteUpMaterial BorderButtonGuiElement::msCmdBorderHiliteUpMaterial;
	BorderButtonGuiElement::CmdBorderDisabledMaterial BorderButtonGuiElement::msCmdBorderDisabledMaterial;
	BorderButtonGuiElement::CmdBorderButtonCaption BorderButtonGuiElement::msCmdBorderButtonCaption;

    //-----------------------------------------------------------------------
	BorderButtonGuiElement::BorderButtonGuiElement(const String& name) :
		BorderPanelGuiElement(name),
		GuiPressable(name)
	{
		mButtonDown = false;
		mChildrenProcessEvents = false;

		mBorderDownMaterialName = "";
		mBorderUpMaterialName = "";
		mBorderHiliteDownMaterialName = "";
		mBorderHiliteUpMaterialName = "";
		mBorderDisabledMaterialName = "";

		mInsideObject = 0;

        if (createParamDictionary("BorderButtonGuiElement"))
        {
            addBaseParameters();
        }
		setSource(this);
	}

    //-----------------------------------------------------------------------
	void BorderButtonGuiElement::processEvent(InputEvent* e) 
	{
		BorderPanelGuiElement::processEvent(e);

		updateMaterials();
	}

    //-----------------------------------------------------------------------
	void BorderButtonGuiElement::updateMaterials(bool init) 
	{
		bool buttonStatus;

		// did the button's status change between last time
		buttonStatus = (mButtonDown != (isPressed() && isMouseWithin()));

		mButtonDown = (isPressed() && isMouseWithin());

		String borderMaterialName;

		if (mMouseWithin)
		{
			if (mButtonDown)
				borderMaterialName = mBorderHiliteDownMaterialName;
			else
				borderMaterialName = mBorderHiliteUpMaterialName;

			if (borderMaterialName.empty())
			{
				if (mButtonDown)
					borderMaterialName = mBorderDownMaterialName;
				else
					borderMaterialName = mBorderUpMaterialName;
			}
		}
		else
		{
			if (mButtonDown)
				borderMaterialName = mBorderDownMaterialName;
			else
				borderMaterialName = mBorderUpMaterialName;
		}

		BorderPanelGuiElement::setBorderMaterialName(borderMaterialName);

		if (buttonStatus && !init)
		{
			ChildIterator it = getChildIterator();
			while (it.hasMoreElements())
			{
				if (mMetricsMode == GMM_PIXELS)
					changeChild(it.getNext(), ((mButtonDown)?1:-1));
				else
					changeChild(it.getNext(), ((mButtonDown)?0.003:-0.003));
			}
		}
	}

    //-----------------------------------------------------------------------
	void BorderButtonGuiElement::changeChild(GuiElement* e, Real add)
	{
		e->setLeft(e->getLeft() + add);
		e->setTop(e->getTop() + add);

		e->setWidth(e->getWidth() - 2 * add);
		e->setHeight(e->getHeight() - 2 * add);
	}

    //---------------------------------------------------------------------
    void BorderButtonGuiElement::addBaseParameters(void)
    {
        BorderPanelGuiElement::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("border_down_material", 
            "The material to use for the border when the button is down."
            , PT_STRING),
            &BorderButtonGuiElement::msCmdBorderDownMaterial);

        dict->addParameter(ParameterDef("border_up_material", 
            "The material to use for the border when the button is up."
            , PT_STRING),
            &BorderButtonGuiElement::msCmdBorderUpMaterial);

        dict->addParameter(ParameterDef("border_hilited_down_material", 
            "The highlighted material to use for the border when the button is down."
            , PT_STRING),
            &BorderButtonGuiElement::msCmdBorderHiliteDownMaterial);

        dict->addParameter(ParameterDef("border_hilited_up_material", 
            "The highlighted material to use for the border when the button is up."
            , PT_STRING),
            &BorderButtonGuiElement::msCmdBorderHiliteUpMaterial);

        dict->addParameter(ParameterDef("border_disabled_material", 
            "The material to use for the border when the button is disabled."
            , PT_STRING),
            &BorderButtonGuiElement::msCmdBorderDisabledMaterial);

        dict->addParameter(ParameterDef("caption", 
            "The text in the middle of the button."
            , PT_STRING),
            &BorderButtonGuiElement::msCmdBorderButtonCaption);
	}


    //-----------------------------------------------------------------------
    void BorderButtonGuiElement::setBorderDownMaterialName(const String& name)
    {
        mBorderDownMaterialName = name;

    }
    //-----------------------------------------------------------------------
    void BorderButtonGuiElement::setBorderUpMaterialName(const String& name)
    {
        mBorderUpMaterialName = name;
		setPressed(false);
		updateMaterials(true);
    }

    //-----------------------------------------------------------------------
    void BorderButtonGuiElement::setBorderHiliteDownMaterialName(const String& name)
    {
        mBorderHiliteDownMaterialName = name;
    }
    //-----------------------------------------------------------------------
    void BorderButtonGuiElement::setBorderHiliteUpMaterialName(const String& name)
    {
        mBorderHiliteUpMaterialName = name;
    }
    //-----------------------------------------------------------------------
    void BorderButtonGuiElement::setBorderDisabledMaterialName(const String& name)
    {
        mBorderDisabledMaterialName = name;
    }

    //---------------------------------------------------------------------
    const String& BorderButtonGuiElement::getBorderDownMaterialName(void) const
    {
        return mBorderDownMaterialName;
    }
    //---------------------------------------------------------------------
    const String& BorderButtonGuiElement::getBorderUpMaterialName(void) const
    {
        return mBorderUpMaterialName;
    }
    //---------------------------------------------------------------------
    const String& BorderButtonGuiElement::getBorderHiliteDownMaterialName(void) const
    {
        return mBorderHiliteDownMaterialName;
    }
    //---------------------------------------------------------------------
    const String& BorderButtonGuiElement::getBorderHiliteUpMaterialName(void) const
    {
        return mBorderHiliteUpMaterialName;
    }
    //---------------------------------------------------------------------
    const String& BorderButtonGuiElement::getBorderDisabledMaterialName(void) const
    {
        return mBorderDisabledMaterialName;
    }

    //-----------------------------------------------------------------------
    void BorderButtonGuiElement::setButtonCaption(const String& templateName, const String& name)
    {
		if (mInsideObject)
		{
			removeChild(mInsideObject->getName());
			GuiManager::getSingleton().destroyGuiElement(mInsideObject);
			mInsideObject = NULL;
		}
		if (name == "")
		{
			return;

		}
		mInsideObject = 
			GuiManager::getSingleton().createGuiElementFromTemplate(templateName, "", mName + "/caption");

		// change left/top etc to relative
//		mInsideObject->setLeft(mInsideObject->getLeft()*mWidth);
//		mInsideObject->setWidth(mInsideObject->getWidth()*mWidth);
//		mInsideObject->setTop(mInsideObject->getTop()*mHeight);
//		mInsideObject->setWidth(mInsideObject->getHeight()*mHeight);
		mInsideObject->setCaption(name);

		// do not make this cloneable, otherwise there will be 2 copies of it when it is cloned,
		// one copy when the children are copied, and another copy when setButtonCaption is set.
		mInsideObject->setCloneable(false);

		addChild((GuiContainer*)mInsideObject);

    }
    //-----------------------------------------------------------------------
    String BorderButtonGuiElement::getButtonCaption() const
	{
		return (mInsideObject)?mInsideObject->getCaption():String("");
	}

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // Command objects
    //---------------------------------------------------------------------

    //-----------------------------------------------------------------------
    String BorderButtonGuiElement::CmdBorderDownMaterial::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const BorderButtonGuiElement*>(target)->getBorderDownMaterialName();
    }
    //-----------------------------------------------------------------------
    void BorderButtonGuiElement::CmdBorderDownMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderButtonGuiElement*>(target)->setBorderDownMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String BorderButtonGuiElement::CmdBorderUpMaterial::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const BorderButtonGuiElement*>(target)->getBorderUpMaterialName();
    }
    //-----------------------------------------------------------------------
    void BorderButtonGuiElement::CmdBorderUpMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderButtonGuiElement*>(target)->setBorderUpMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String BorderButtonGuiElement::CmdBorderHiliteDownMaterial::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const BorderButtonGuiElement*>(target)->getBorderHiliteDownMaterialName();
    }
    //-----------------------------------------------------------------------
    void BorderButtonGuiElement::CmdBorderHiliteDownMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderButtonGuiElement*>(target)->setBorderHiliteDownMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String BorderButtonGuiElement::CmdBorderHiliteUpMaterial::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const BorderButtonGuiElement*>(target)->getBorderHiliteUpMaterialName();
    }
    //-----------------------------------------------------------------------
    void BorderButtonGuiElement::CmdBorderHiliteUpMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderButtonGuiElement*>(target)->setBorderHiliteUpMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String BorderButtonGuiElement::CmdBorderDisabledMaterial::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const BorderButtonGuiElement*>(target)->getBorderDisabledMaterialName();
    }
    //-----------------------------------------------------------------------
    void BorderButtonGuiElement::CmdBorderDisabledMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<BorderButtonGuiElement*>(target)->setBorderDisabledMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String BorderButtonGuiElement::CmdBorderButtonCaption::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const BorderButtonGuiElement*>(target)->getButtonCaption();
    }
    //-----------------------------------------------------------------------
    void BorderButtonGuiElement::CmdBorderButtonCaption::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val, "\t\n ", 1);


		if (vec.size() < 2)
		{
			static_cast<BorderButtonGuiElement*>(target)->setButtonCaption(val, String(""));
		}
		else
		{
			static_cast<BorderButtonGuiElement*>(target)->setButtonCaption(vec[0], vec[1]);
		}
    }
    //---------------------------------------------------------------------
    const String& BorderButtonGuiElement::getTypeName(void) const
    {
        return msTypeName;
    }

}

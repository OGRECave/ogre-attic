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

#include "OgreButtonGuiElement.h"
#include "OgreGuiManager.h"
#include "OgreStringConverter.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    String ButtonGuiElement::msTypeName = "Button";
	ButtonGuiElement::CmdButtonDownMaterial ButtonGuiElement::msCmdButtonDownMaterial;
	ButtonGuiElement::CmdButtonUpMaterial ButtonGuiElement::msCmdButtonUpMaterial;
	ButtonGuiElement::CmdButtonHiliteDownMaterial ButtonGuiElement::msCmdButtonHiliteDownMaterial;
	ButtonGuiElement::CmdButtonHiliteUpMaterial ButtonGuiElement::msCmdButtonHiliteUpMaterial;
	ButtonGuiElement::CmdButtonDisabledMaterial ButtonGuiElement::msCmdButtonDisabledMaterial;
    ButtonGuiElement::CmdCaptionColour ButtonGuiElement::msCmdCaptionColour;
    ButtonGuiElement::CmdCaptionDisabledColour ButtonGuiElement::msCmdCaptionDisabledColour;
	ButtonGuiElement::CmdButtonCaption ButtonGuiElement::msCmdButtonCaption;

    //-----------------------------------------------------------------------
	ButtonGuiElement::ButtonGuiElement(const String& name) :
		PanelGuiElement(name),
		GuiPressable(name)
	{
		mButtonDown = false;
		mChildrenProcessEvents = false;

		mDownMaterialName = "";
		mUpMaterialName = "";
		mHiliteDownMaterialName = "";
		mHiliteUpMaterialName = "";
		mDisabledMaterialName = "";

		mSetCaptionColor = false;
		mSetCaptionDisabledColor = false;

		mInsideObject = 0;

        if (createParamDictionary("ButtonGuiElement"))
        {
            addBaseParameters();
        }
		setSource(this);
	}

    //-----------------------------------------------------------------------
	void ButtonGuiElement::processEvent(InputEvent* e) 
	{
		PanelGuiElement::processEvent(e);

		updateMaterials();
	}

    //-----------------------------------------------------------------------
	void ButtonGuiElement::updateMaterials(bool init) 
	{
		bool buttonStatus;

		// did the button's status change between last time
		buttonStatus = (mButtonDown != (isPressed() && isMouseWithin()));

		mButtonDown = (isPressed() && isMouseWithin());

		String materialName;

		if (!isEnabled())
		{
			if (mInsideObject)
			{
				if (mSetCaptionDisabledColor)
					mInsideObject->setColour(mCaptionDisabledColour);
				else if (mSetCaptionColor)
					mInsideObject->setColour(mCaptionColour);
			}

			if (mDisabledMaterialName.empty())
				materialName = mDisabledMaterialName;
			else
				materialName = mUpMaterialName;
		}
		else
		{
			if (mSetCaptionColor && mInsideObject)
				mInsideObject->setColour(mCaptionColour);

			if (mMouseWithin)
			{
				if (mButtonDown)
					materialName = mHiliteDownMaterialName;
				else
					materialName = mHiliteUpMaterialName;

				if (materialName.empty())
				{
					if (mButtonDown)
						materialName = mDownMaterialName;
					else
						materialName = mUpMaterialName;
				}
			}
			else
			{
				if (mButtonDown)
					materialName = mDownMaterialName;
				else
					materialName = mUpMaterialName;
			}
		}

		PanelGuiElement::setMaterialName(materialName);

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
	void ButtonGuiElement::changeChild(GuiElement* e, Real add)
	{
		e->setLeft(e->getLeft() + add);
		e->setTop(e->getTop() + add);

		e->setWidth(e->getWidth() - 2 * add);
		e->setHeight(e->getHeight() - 2 * add);
	}

    //---------------------------------------------------------------------
    void ButtonGuiElement::addBaseParameters(void)
    {
        PanelGuiElement::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("button_down_material", 
            "The material to use when the button is down."
            , PT_STRING),
            &ButtonGuiElement::msCmdButtonDownMaterial);

        dict->addParameter(ParameterDef("button_up_material", 
            "The material to use when the button is up."
            , PT_STRING),
            &ButtonGuiElement::msCmdButtonUpMaterial);

        dict->addParameter(ParameterDef("button_hilite_down_material", 
            "The material to use when the button is highlighted and down."
            , PT_STRING),
            &ButtonGuiElement::msCmdButtonHiliteDownMaterial);

        dict->addParameter(ParameterDef("button_hilite_up_material", 
            "The material to use when the button is highlighted and up."
            , PT_STRING),
            &ButtonGuiElement::msCmdButtonHiliteUpMaterial);

        dict->addParameter(ParameterDef("button_disabled_material", 
            "The material to use when the button is disabled."
            , PT_STRING),
            &ButtonGuiElement::msCmdButtonDisabledMaterial);

        dict->addParameter(ParameterDef("caption_colour", 
            "Sets the caption's font colour."
            , PT_STRING),
            &msCmdCaptionColour);

        dict->addParameter(ParameterDef("caption_disabled_colour", 
            "Sets the caption's font colour when the button is disabled."
            , PT_STRING),
            &msCmdCaptionDisabledColour);

        dict->addParameter(ParameterDef("caption", 
            "The text in the middle of the button."
            , PT_STRING),
            &ButtonGuiElement::msCmdButtonCaption);
	}

    //-----------------------------------------------------------------------
    void ButtonGuiElement::setDownMaterialName(const String& name)
    {
        mDownMaterialName = name;
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::setUpMaterialName(const String& name)
    {
        mUpMaterialName = name;
		setPressed(false);
		updateMaterials(true);
    }

    //-----------------------------------------------------------------------
    void ButtonGuiElement::setHiliteDownMaterialName(const String& name)
    {
        mHiliteDownMaterialName = name;
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::setHiliteUpMaterialName(const String& name)
    {
        mHiliteUpMaterialName = name;
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::setDisabledMaterialName(const String& name)
    {
        mDisabledMaterialName = name;
    }
    //---------------------------------------------------------------------
    void ButtonGuiElement::setCaptionColour(const ColourValue& col)
    {
        mCaptionColour = col;
		mSetCaptionColor = true;
		updateMaterials();
    }
    //---------------------------------------------------------------------
    void ButtonGuiElement::setCaptionDisabledColour(const ColourValue& col)
    {
        mCaptionDisabledColour = col;
		mSetCaptionDisabledColor = true;
		updateMaterials();
    }

    //-----------------------------------------------------------------------
    void ButtonGuiElement::setButtonCaption(const String& templateName, const String& name)
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

		addChild(mInsideObject);

		// to make sure everything is right (colour)
		updateMaterials();
    }
    //-----------------------------------------------------------------------
    String ButtonGuiElement::getButtonCaption() const
	{
		return (mInsideObject)?mInsideObject->getCaption():String("");
	}

    //---------------------------------------------------------------------
    const String& ButtonGuiElement::getDownMaterialName(void) const
    {
        return mDownMaterialName;
    }
    //---------------------------------------------------------------------
    const String& ButtonGuiElement::getUpMaterialName(void) const
    {
        return mUpMaterialName;
    }

    //---------------------------------------------------------------------
    const String& ButtonGuiElement::getHiliteDownMaterialName(void) const
    {
        return mHiliteDownMaterialName;
    }
    //---------------------------------------------------------------------
    const String& ButtonGuiElement::getHiliteUpMaterialName(void) const
    {
        return mHiliteUpMaterialName;
    }
    //---------------------------------------------------------------------
    const String& ButtonGuiElement::getDisabledMaterialName(void) const
    {
        return mDisabledMaterialName;
    }
    //---------------------------------------------------------------------
    ColourValue ButtonGuiElement::getCaptionColour(void) const
    {
        return mCaptionColour;
    }
    //---------------------------------------------------------------------
    ColourValue ButtonGuiElement::getCaptionDisabledColour(void) const
    {
        return mCaptionDisabledColour;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // Command objects
    //---------------------------------------------------------------------

    //-----------------------------------------------------------------------
    String ButtonGuiElement::CmdButtonDownMaterial::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const ButtonGuiElement*>(target)->getDownMaterialName();
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::CmdButtonDownMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<ButtonGuiElement*>(target)->setDownMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String ButtonGuiElement::CmdButtonUpMaterial::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const ButtonGuiElement*>(target)->getUpMaterialName();
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::CmdButtonUpMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<ButtonGuiElement*>(target)->setUpMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String ButtonGuiElement::CmdButtonHiliteDownMaterial::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const ButtonGuiElement*>(target)->getHiliteDownMaterialName();
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::CmdButtonHiliteDownMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<ButtonGuiElement*>(target)->setHiliteDownMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String ButtonGuiElement::CmdButtonHiliteUpMaterial::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const ButtonGuiElement*>(target)->getHiliteUpMaterialName();
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::CmdButtonHiliteUpMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<ButtonGuiElement*>(target)->setHiliteUpMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String ButtonGuiElement::CmdButtonDisabledMaterial::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const ButtonGuiElement*>(target)->getDisabledMaterialName();
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::CmdButtonDisabledMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<ButtonGuiElement*>(target)->setDisabledMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String ButtonGuiElement::CmdCaptionColour::doGet(const void* target) const
    {
        return StringConverter::toString(static_cast<const ButtonGuiElement*>(target)->getCaptionColour());
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::CmdCaptionColour::doSet(void* target, const String& val)
    {
        static_cast<ButtonGuiElement*>(target)->setCaptionColour(StringConverter::parseColourValue(val));
    }
    //-----------------------------------------------------------------------
    String ButtonGuiElement::CmdCaptionDisabledColour::doGet(const void* target) const
    {
        return StringConverter::toString(static_cast<const ButtonGuiElement*>(target)->getCaptionDisabledColour());
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::CmdCaptionDisabledColour::doSet(void* target, const String& val)
    {
        static_cast<ButtonGuiElement*>(target)->setCaptionDisabledColour(StringConverter::parseColourValue(val));
    }
    //-----------------------------------------------------------------------
    String ButtonGuiElement::CmdButtonCaption::doGet(const void* target) const
    {
        // No need right now..
        return static_cast<const ButtonGuiElement*>(target)->getButtonCaption();
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::CmdButtonCaption::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val, "\t\n ", 1);


		if (vec.size() < 2)
		{
			static_cast<ButtonGuiElement*>(target)->setButtonCaption(val, String(""));
		}
		else
		{
			static_cast<ButtonGuiElement*>(target)->setButtonCaption(vec[0], vec[1]);
		}
    }
    //---------------------------------------------------------------------
    const String& ButtonGuiElement::getTypeName(void) const
    {
        return msTypeName;
    }

}


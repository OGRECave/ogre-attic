
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

#include "OgreButtonGuiElement.h"	
#include "OgreActionEvent.h"
#include "OgreMouseEvent.h"
#include "OgreGuiManager.h"


namespace Ogre {

    //-----------------------------------------------------------------------
    String ButtonGuiElement::msTypeName = "Button";
	ButtonGuiElement::CmdBorderDownMaterial ButtonGuiElement::msCmdBorderDownMaterial;
	ButtonGuiElement::CmdBorderUpMaterial ButtonGuiElement::msCmdBorderUpMaterial;
	ButtonGuiElement::CmdButtonCaption ButtonGuiElement::msCmdButtonCaption;

    //-----------------------------------------------------------------------
	ButtonGuiElement::ButtonGuiElement(const String& name) :
		BorderPanelGuiElement(name),
		ActionTarget()
	{
		mPressed = false;
		mActionCommand = "";
		mInsideObject = 0;

        if (createParamDictionary("ButtonGuiElement"))
        {
            addBaseParameters();
        }

	}


    //-----------------------------------------------------------------------
	void ButtonGuiElement::processEvent(InputEvent* e) 
	{
		GuiElement::processEvent(e);

		if (!e->isConsumed())
		{
			switch(e->getID()) 
			{
			case ActionEvent::AE_ACTION_PERFORMED:
				processActionEvent(static_cast<ActionEvent*>(e));
				break;
			case MouseEvent::ME_MOUSE_PRESSED:
				setPressed(true);
				break;
			case MouseEvent::ME_MOUSE_RELEASED:
				setPressed(false);
				break;
			case MouseEvent::ME_MOUSE_EXITED:
				if (isPressed())
				{
					setPressed(false);
				}
				break;
			case MouseEvent::ME_MOUSE_CLICKED:
				fireActionPerformed();
				break;
			default:
				break;
			}
		}
	}

    //-----------------------------------------------------------------------
	bool ButtonGuiElement::isPressed()
	{ 
		return mPressed;
	}

    //-----------------------------------------------------------------------
	String ButtonGuiElement::getActionCommand()
	{ 
		return (mActionCommand == "")? getName(): mActionCommand;
	}

    //-----------------------------------------------------------------------
	void ButtonGuiElement::setPressed(bool b, bool init) 
	{
		if (mPressed == b && !init)
		{
			return;

		}

		mPressed = b;
		BorderPanelGuiElement::setBorderMaterialName((b)?mBorderDownMaterialName:mBorderUpMaterialName);
		if (!init)
		{
			ChildIterator it = getChildIterator();
			while (it.hasMoreElements())
			{
				changeChild(it.getNext(), ((b)?0.003:-0.003));
			}
		}
	}
    //-----------------------------------------------------------------------
	void ButtonGuiElement::changeChild(GuiElement* e, Real add)
	{
		e->setLeft(e->getLeft() + add);
		e->setTop(e->getTop() + add);

		e->setWidth(e->getWidth() - 2* add);
		e->setHeight(e->getHeight() - 2* add);
	}

    //-----------------------------------------------------------------------
	void ButtonGuiElement::fireActionPerformed()
	{
		ActionEvent* ae = new ActionEvent(this, ActionEvent::AE_ACTION_PERFORMED, 0, 0, getActionCommand());
		processEvent(ae);
		delete ae;
	}


    //---------------------------------------------------------------------
    void ButtonGuiElement::addBaseParameters(void)
    {
        BorderPanelGuiElement::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("border_down_material", 
            "The material to use for the border when the button is down."
            , PT_STRING),
            &ButtonGuiElement::msCmdBorderDownMaterial);

        dict->addParameter(ParameterDef("border_up_material", 
            "The material to use for the border when the button is up."
            , PT_STRING),
            &ButtonGuiElement::msCmdBorderUpMaterial);

        dict->addParameter(ParameterDef("caption", 
            "The text in the middle of the button."
            , PT_STRING),
            &ButtonGuiElement::msCmdButtonCaption);
	}

    //-----------------------------------------------------------------------
	GuiElement* ButtonGuiElement::findElementAt(Real x, Real y) 		// relative to parent
		// don't look in children (ie text)
	{

		GuiElement* ret = NULL;

		if (mVisible)
		{
			ret = GuiElement::findElementAt(x,y);	//default to the current container if no others are found
		}
		return ret;
	}


    //-----------------------------------------------------------------------
    void ButtonGuiElement::setBorderDownMaterialName(const String& name)
    {
        mBorderDownMaterialName = name;

    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::setBorderUpMaterialName(const String& name)
    {
        mBorderUpMaterialName = name;
		setPressed(false, true);

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

		addChild((GuiContainer*)mInsideObject);

    }
    //-----------------------------------------------------------------------
    String ButtonGuiElement::getButtonCaption()
	{
		return (mInsideObject)?mInsideObject->getCaption():String("");
	}

    //---------------------------------------------------------------------
    const String& ButtonGuiElement::getBorderDownMaterialName(void)
    {
        return mBorderDownMaterialName;
    }
    //---------------------------------------------------------------------
    const String& ButtonGuiElement::getBorderUpMaterialName(void)
    {
        return mBorderUpMaterialName;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // Command objects
    //---------------------------------------------------------------------

    //-----------------------------------------------------------------------
    String ButtonGuiElement::CmdBorderDownMaterial::doGet(void* target)
    {
        // No need right now..
        return static_cast<ButtonGuiElement*>(target)->getBorderDownMaterialName();
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::CmdBorderDownMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<ButtonGuiElement*>(target)->setBorderDownMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String ButtonGuiElement::CmdBorderUpMaterial::doGet(void* target)
    {
        // No need right now..
        return static_cast<ButtonGuiElement*>(target)->getBorderUpMaterialName();
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::CmdBorderUpMaterial::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<ButtonGuiElement*>(target)->setBorderUpMaterialName(val);
    }
    //-----------------------------------------------------------------------
    String ButtonGuiElement::CmdButtonCaption::doGet(void* target)
    {
        // No need right now..
        return static_cast<ButtonGuiElement*>(target)->getButtonCaption();
    }
    //-----------------------------------------------------------------------
    void ButtonGuiElement::CmdButtonCaption::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();


		if (vec.size() < 2)
		{
			static_cast<ButtonGuiElement*>(target)->setButtonCaption(val, String(""));
		}
		else
		{
			static_cast<ButtonGuiElement*>(target)->setButtonCaption(vec[0], vec[1]);
		}
    }

}


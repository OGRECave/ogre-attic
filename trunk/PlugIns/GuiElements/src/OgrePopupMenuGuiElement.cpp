/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2003 The OGRE Team
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

#include "OgrePopupMenuGuiElement.h"	
#include "OgreStringConverter.h"	
#include "OgreGuiManager.h"	
#include "OgreStringResource.h"	
#include "OgreException.h"	
#include "OgreMouseEvent.h"	
#include <algorithm>

namespace Ogre {

    String PopupMenuGuiElement::msTypeName = "PopupMenu";
	PopupMenuGuiElement::CmdItemTemplate PopupMenuGuiElement::msCmdItemTemplate;
	PopupMenuGuiElement::CmdVSpacing PopupMenuGuiElement::msCmdVSpacing;
	PopupMenuGuiElement::CmdHSpacing PopupMenuGuiElement::msCmdHSpacing;
	PopupMenuGuiElement::CmdItemPanelMaterial PopupMenuGuiElement::msCmdItemPanelMaterial;
	PopupMenuGuiElement::CmdItemPanelMaterialSelected PopupMenuGuiElement::msCmdItemPanelMaterialSelected;

	PopupMenuGuiElement::PopupMenuGuiElement(const String& name) :
		PanelGuiElement(name),
		GuiPressable(name)
	{
        if (createParamDictionary("PopupMenuGuiElement"))
        {
            addBaseParameters();
        }
		mChildrenProcessEvents = false;

		mSelectedElement = 0;
		mVSpacing = 0;
		mHSpacing = 0;
		mPixelVSpacing = 0;
		mPixelHSpacing = 0;
		mItemPanelMaterial = "";
		mItemPanelMaterialSelected = "";

		mSelectedMenuItem = new StringResource("");
		mSeparatormenuItem = new StringResource("-----------------------");
		setSource(this);
		addMouseMotionListener(this);
	}
	//---------------------------------------------------------------------
	PopupMenuGuiElement::~PopupMenuGuiElement()
	{
		for (ResourceList::iterator i = mResourceList.begin(); i != mResourceList.end(); ++i)
			delete *i;
	}

    //---------------------------------------------------------------------
    void PopupMenuGuiElement::addBaseParameters(void)
    {
        PanelGuiElement::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("item_template", 
            "The template of List Item objects."
            , PT_STRING),
            &PopupMenuGuiElement::msCmdItemTemplate);

        dict->addParameter(ParameterDef("v_spacing", 
            "The vertical spacing of the elements"
            , PT_STRING),
            &PopupMenuGuiElement::msCmdVSpacing);
        dict->addParameter(ParameterDef("h_spacing", 
            "The horizontal spacing of the elements from the edge of the list"
            , PT_STRING),
            &PopupMenuGuiElement::msCmdHSpacing);
        dict->addParameter(ParameterDef("item_material", 
            "The material of the item panel"
            , PT_STRING),
            &PopupMenuGuiElement::msCmdItemPanelMaterial);
        dict->addParameter(ParameterDef("item_material_selected", 
            "The material of the item panel when it is selected"
            , PT_STRING),
            &PopupMenuGuiElement::msCmdItemPanelMaterialSelected);
	}
    //---------------------------------------------------------------------
    // Command objects
    //---------------------------------------------------------------------

    //-----------------------------------------------------------------------
    String PopupMenuGuiElement::CmdItemTemplate::doGet(const void* target) const
    {
        return static_cast<const PopupMenuGuiElement*>(target)->getItemTemplateName();
    }
    void PopupMenuGuiElement::CmdItemTemplate::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<PopupMenuGuiElement*>(target)->setItemTemplateName(val);
    }
    //-----------------------------------------------------------------------
    String PopupMenuGuiElement::CmdVSpacing::doGet(const void* target) const
    {
        return static_cast<const PopupMenuGuiElement*>(target)->getVSpacing();
    }
    void PopupMenuGuiElement::CmdVSpacing::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<PopupMenuGuiElement*>(target)->setVSpacing(val);
    }
    //-----------------------------------------------------------------------
    String PopupMenuGuiElement::CmdHSpacing::doGet(const void* target) const
    {
        return static_cast<const PopupMenuGuiElement*>(target)->getHSpacing();
    }
    void PopupMenuGuiElement::CmdHSpacing::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<PopupMenuGuiElement*>(target)->setHSpacing(val);
    }
    //-----------------------------------------------------------------------
    String PopupMenuGuiElement::CmdItemPanelMaterialSelected::doGet(const void* target) const
    {
        return static_cast<const PopupMenuGuiElement*>(target)->getItemPanelMaterialSelected();
    }
    void PopupMenuGuiElement::CmdItemPanelMaterialSelected::doSet(void* target, const String& val)
    {
        static_cast<PopupMenuGuiElement*>(target)->setItemPanelMaterialSelected(val);
    }
    //-----------------------------------------------------------------------
    String PopupMenuGuiElement::CmdItemPanelMaterial::doGet(const void* target) const
    {
        return static_cast<const PopupMenuGuiElement*>(target)->getItemPanelMaterial();
    }
    void PopupMenuGuiElement::CmdItemPanelMaterial::doSet(void* target, const String& val)
    {
        static_cast<PopupMenuGuiElement*>(target)->setItemPanelMaterial(val);
    }
    //-----------------------------------------------------------------------
	String PopupMenuGuiElement::getItemTemplateName() const
	{
		return mItemTemplateName;
	}
	void PopupMenuGuiElement::setItemTemplateName(const String& val)
	{
		mItemTemplateName = val;
	}
    //-----------------------------------------------------------------------

	void PopupMenuGuiElement::setHSpacing(const String& val)
	{
		mHSpacing = StringConverter::parseReal(val);
	}
	String PopupMenuGuiElement::getHSpacing() const
	{
		return  StringConverter::toString(mHSpacing);
	}

    //-----------------------------------------------------------------------

	void PopupMenuGuiElement::setVSpacing(const String& val)
	{
		mVSpacing = StringConverter::parseReal(val);
	}
	String PopupMenuGuiElement::getVSpacing() const
	{
		return  StringConverter::toString(mVSpacing);
	}
    //-----------------------------------------------------------------------
	String PopupMenuGuiElement::getItemPanelMaterial() const
	{
		return mItemPanelMaterial;
	}
	void PopupMenuGuiElement::setItemPanelMaterial(const String& val)
	{
		mItemPanelMaterial = val;
	}
    //-----------------------------------------------------------------------
	String PopupMenuGuiElement::getItemPanelMaterialSelected() const
	{
		return mItemPanelMaterialSelected;

	}
	void PopupMenuGuiElement::setItemPanelMaterialSelected(const String& val)
	{
		mItemPanelMaterialSelected = val;
	}
    //-----------------------------------------------------------------------

	void PopupMenuGuiElement::addMenuItem(Resource* r)
	{
		GuiElement* mInsideObject = 
			GuiManager::getSingleton().createGuiElementFromTemplate(mItemTemplateName, "", getListItemName(r));

		// create a back panel for the item

		GuiContainer* pBackPanel = static_cast<GuiContainer*>
			(GuiManager::getSingleton().createGuiElement("Panel",getListItemPanelName(r)));

		pBackPanel->setLeft(0);
		pBackPanel->setWidth(getWidth());
		pBackPanel->setHeight(mInsideObject->getHeight());
		addChild(pBackPanel);

		mInsideObject->setCaption(r->getName());
		mInsideObject->setLeft(mVSpacing);
		mInsideObject->setTop(0);
		mInsideObject->setWidth(pBackPanel->getWidth()-mVSpacing);

		pBackPanel->addChild((GuiContainer*)mInsideObject);

		setSelectedItem(mInsideObject,false);
	}

	void PopupMenuGuiElement::removeMenuItem(Resource* r)
	{
		GuiContainer* backPanel = static_cast<GuiContainer*> (getChild(getListItemPanelName(r)));

		backPanel->removeChild(getListItemName(r));
		removeChild(getListItemPanelName(r));

		GuiManager::getSingleton().destroyGuiElement(getListItemName(r));
		GuiManager::getSingleton().destroyGuiElement(getListItemPanelName(r));
	}
    //-----------------------------------------------------------------------

	void PopupMenuGuiElement::addListItem(Resource* r)
	{
		mResourceList.push_back(r);

		if (mResourceList.size() == 1)
		{
			addMenuItem(r);
			mSelectedString = r->getName();
			layoutItems();
		}
	}

	void PopupMenuGuiElement::removeListItem(Resource* r)
	{
		bool bFound = false;
        ResourceList::iterator i;

        for (i = mResourceList.begin(); i != mResourceList.end(); ++i)
        {
            if (*i == r)
			{
				delete *i;
				mResourceList.erase(i);
				bFound = true;
				break;
			}
        }

		if (!bFound)
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot find Resource " + r->getName() + 
                " to remove from list.", "PopupMenuGuiElement::removeListItem");
        }
	}

	String PopupMenuGuiElement::getListItemName(Resource* r)
	{
		return mName + "/" + r->getName();
	}

	String PopupMenuGuiElement::getListItemPanelName(Resource* r)
	{
		return getListItemName(r) + "/" + "BackPanel";
	}

	void PopupMenuGuiElement::layoutItems()
	{
		Real currentTop = 0;	//mVSpacing;
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            GuiElement* currentElement = it.getNext();

			currentElement->setTop(currentTop);
			currentTop += currentElement->getHeight() + mVSpacing;
        }
		this->setHeight(currentTop - mVSpacing);

		_update();
	}

	void PopupMenuGuiElement::mouseDragged(MouseEvent* e) {}

	void PopupMenuGuiElement::mouseMoved(MouseEvent* e) 
	{
		MouseEvent* me = static_cast<MouseEvent*>(e);
		GuiElement* newSelect = GuiContainer::findElementAt(me->getX(), me->getY());

		if (newSelect == this || !isPressed())	// in case there are fringe pixels not in the list
		{
			newSelect = NULL;
		}

		if (newSelect != mSelectedElement)
		{
			if (mSelectedElement)
			{
				setSelectedItem(mSelectedElement, false);
			}

			if (newSelect)
			{
				setSelectedItem(newSelect, true);
			}

			mSelectedElement = newSelect;
		}
	}

	void PopupMenuGuiElement::mouseExited(void) 
	{
		if (mSelectedElement)
		{
			setSelectedItem(mSelectedElement, false);
			mSelectedElement = NULL;
		}
	}

	void PopupMenuGuiElement::mousePressed(void)
	{
        ResourceList::iterator i;

        for (i = mResourceList.begin(); i != mResourceList.end(); ++i)
        {
			if (mSelectedString == (*i)->getName())
			{
				removeMenuItem(*i);
				break;
			}
        }

        for (i = mResourceList.begin(); i != mResourceList.end(); ++i)
        {
			addMenuItem(*i);
        }

		layoutItems();
	}

	void PopupMenuGuiElement::mouseReleased(void) 
	{
        ResourceList::iterator i;
		bool selectedItem=false;

		if (mSelectedElement)
		{
			setActionCommand(mSelectedElement->getName());
			setSelectedItem(mSelectedElement, false);
			mSelectedString = mSelectedElement->getCaption();
			mSelectedElement = NULL;
			selectedItem = true;
		}

        for (i = mResourceList.begin(); i != mResourceList.end(); ++i)
        {
			if (mSelectedString != (*i)->getName())
			{
				removeMenuItem(*i);
			}
        }

		layoutItems();

		if (selectedItem)
		{
			fireActionPerformed();
		}
	}

	Resource* PopupMenuGuiElement::getSelectedItem()
	{
		Resource* selectedResource = NULL;
        return selectedResource;
	}

	void PopupMenuGuiElement::setSelectedItem(Resource* r, bool on)
	{
		// do later
	}
	void PopupMenuGuiElement::setSelectedItem(Resource* r)
	{
		// do later
	}

	void PopupMenuGuiElement::setSelectedItem(GuiElement* item)
	{
		// do later
	}

	void PopupMenuGuiElement::setSelectedItem(GuiElement* item, bool on)
	{
		if (on)
		{
			item->getParent()->setMaterialName(mItemPanelMaterialSelected);
		}
		else
		{
			if (mItemPanelMaterial == "")
			{
				// default to the list material
				item->getParent()->setMaterialName(mMaterialName);
			}
			else
			{
				item->getParent()->setMaterialName(mItemPanelMaterial);
			}
		}
	}
	ResourceListConstIterator PopupMenuGuiElement::getConstIterator() const
	{
        return ResourceListConstIterator(mResourceList.begin());
	}

	ResourceListConstIterator PopupMenuGuiElement::getConstEndIterator() const
	{
        return ResourceListConstIterator(mResourceList.end());
	}

	Resource* PopupMenuGuiElement::popFront()
	{
		Resource* r = mResourceList.front();
		mResourceList.pop_front();

        return r;
	}
	
	size_t PopupMenuGuiElement::getListSize() const
	{
		return mResourceList.size();
	}

}

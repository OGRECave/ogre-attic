
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

#include "OgreListGuiElement.h"	
#include "OgreStringConverter.h"	
#include "OgreGuiManager.h"	
#include "OgreResource.h"	
#include "OgreException.h"	


namespace Ogre {

    String ListGuiElement::msTypeName = "List";
	ListGuiElement::CmdItemTemplate ListGuiElement::msCmdItemTemplate;
	ListGuiElement::CmdVSpacing ListGuiElement::msCmdVSpacing;
	ListGuiElement::CmdHSpacing ListGuiElement::msCmdHSpacing;
	ListGuiElement::CmdItemPanelMaterial ListGuiElement::msCmdItemPanelMaterial;
	ListGuiElement::CmdItemPanelMaterialSelected ListGuiElement::msCmdItemPanelMaterialSelected;


	ListGuiElement::ListGuiElement(const String& name) :
		PanelGuiElement(name)
	{
        if (createParamDictionary("ListGuiElement"))
        {
            addBaseParameters();
        }

		mSelectedElement = 0;
		mVSpacing = 0;
		mHSpacing = 0;
		mPixelVSpacing = 0;
		mPixelHSpacing = 0;
		mItemPanelMaterial = "";
		mItemPanelMaterialSelected = "";
	}

    //---------------------------------------------------------------------
    void ListGuiElement::addBaseParameters(void)
    {
        PanelGuiElement::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("item_template", 
            "The template of List Item objects."
            , PT_STRING),
            &ListGuiElement::msCmdItemTemplate);

        dict->addParameter(ParameterDef("v_spacing", 
            "The vertical spacing of the elements"
            , PT_STRING),
            &ListGuiElement::msCmdVSpacing);
        dict->addParameter(ParameterDef("h_spacing", 
            "The horizontal spacing of the elements from the edge of the list"
            , PT_STRING),
            &ListGuiElement::msCmdHSpacing);
        dict->addParameter(ParameterDef("item_material", 
            "The material of the item panel"
            , PT_STRING),
            &ListGuiElement::msCmdItemPanelMaterial);
        dict->addParameter(ParameterDef("item_material_selected", 
            "The material of the item panel when it is selected"
            , PT_STRING),
            &ListGuiElement::msCmdItemPanelMaterialSelected);
	}
    //---------------------------------------------------------------------
    // Command objects
    //---------------------------------------------------------------------

    //-----------------------------------------------------------------------
    String ListGuiElement::CmdItemTemplate::doGet(void* target)
    {
        return static_cast<ListGuiElement*>(target)->getItemTemplateName();
    }
    void ListGuiElement::CmdItemTemplate::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<ListGuiElement*>(target)->setItemTemplateName(val);
    }
    //-----------------------------------------------------------------------
    String ListGuiElement::CmdVSpacing::doGet(void* target)
    {
        return static_cast<ListGuiElement*>(target)->getVSpacing();
    }
    void ListGuiElement::CmdVSpacing::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<ListGuiElement*>(target)->setVSpacing(val);
    }
    //-----------------------------------------------------------------------
    String ListGuiElement::CmdHSpacing::doGet(void* target)
    {
        return static_cast<ListGuiElement*>(target)->getHSpacing();
    }
    void ListGuiElement::CmdHSpacing::doSet(void* target, const String& val)
    {
        std::vector<String> vec = val.split();

        static_cast<ListGuiElement*>(target)->setHSpacing(val);
    }
    //-----------------------------------------------------------------------
    String ListGuiElement::CmdItemPanelMaterialSelected::doGet(void* target)
    {
        return static_cast<ListGuiElement*>(target)->getItemPanelMaterialSelected();
    }
    void ListGuiElement::CmdItemPanelMaterialSelected::doSet(void* target, const String& val)
    {
        static_cast<ListGuiElement*>(target)->setItemPanelMaterialSelected(val);
    }
    //-----------------------------------------------------------------------
    String ListGuiElement::CmdItemPanelMaterial::doGet(void* target)
    {
        return static_cast<ListGuiElement*>(target)->getItemPanelMaterial();
    }
    void ListGuiElement::CmdItemPanelMaterial::doSet(void* target, const String& val)
    {
        static_cast<ListGuiElement*>(target)->setItemPanelMaterial(val);
    }
    //-----------------------------------------------------------------------
	String ListGuiElement::getItemTemplateName()
	{
		return mItemTemplateName;

	}
	void ListGuiElement::setItemTemplateName(const String& val)
	{
		mItemTemplateName = val;

	}
    //-----------------------------------------------------------------------

	void ListGuiElement::setHSpacing(const String& val)
	{
		mHSpacing = StringConverter::parseReal(val);

	}
	String ListGuiElement::getHSpacing()
	{
		return  StringConverter::toString(mHSpacing);

	}

    //-----------------------------------------------------------------------

	void ListGuiElement::setVSpacing(const String& val)
	{
		mVSpacing = StringConverter::parseReal(val);

	}
	String ListGuiElement::getVSpacing()
	{
		return  StringConverter::toString(mVSpacing);

	}
    //-----------------------------------------------------------------------
	String ListGuiElement::getItemPanelMaterial()
	{
		return mItemPanelMaterial;

	}
	void ListGuiElement::setItemPanelMaterial(const String& val)
	{
		mItemPanelMaterial = val;

	}
    //-----------------------------------------------------------------------
	String ListGuiElement::getItemPanelMaterialSelected()
	{
		return mItemPanelMaterialSelected;

	}
	void ListGuiElement::setItemPanelMaterialSelected(const String& val)
	{
		mItemPanelMaterialSelected = val;

	}
    //-----------------------------------------------------------------------

	void ListGuiElement::addListItem(Resource* r)
	{
		GuiElement* mInsideObject = 
			GuiManager::getSingleton().createGuiElementFromTemplate(mItemTemplateName, "", getListItemName(r));

		// create a back panel for the item

		GuiContainer* pBackPanel = static_cast<GuiContainer*>
			(GuiManager::getSingleton().createGuiElement("Panel",getListItemPanelName(r)));

		pBackPanel->setLeft(mVSpacing);
		pBackPanel->setWidth(getWidth()-mVSpacing);
		pBackPanel->setHeight(mInsideObject->getHeight());
		addChild((GuiContainer*)pBackPanel);

		mInsideObject->setCaption(r->getName());
		mInsideObject->setLeft(0);
		mInsideObject->setTop(0);
		mInsideObject->setWidth(pBackPanel->getWidth());
		mInsideObject->addMouseListener(this);


		mResourceList.push_back(r);
		pBackPanel->addChild((GuiContainer*)mInsideObject);


		setSelectedItem(mInsideObject,false);
		layoutItems();

	}

	void ListGuiElement::removeListItem(Resource* r)
	{
		GuiContainer* backPanel = static_cast<GuiContainer*> (getChild(getListItemPanelName(r)));

		backPanel->removeChild(getListItemName(r));
		removeChild(getListItemPanelName(r));

		GuiManager::getSingleton().destroyGuiElement(getListItemName(r));
		GuiManager::getSingleton().destroyGuiElement(getListItemPanelName(r));

        ;
		bool bFound = false;
        ResourceList::iterator i;
        for (i = mResourceList.begin(); i != mResourceList.end(); ++i)
        {
            if (*i == r)
			{
				mResourceList.erase(i);
				bFound = true;
				break;

			}
        }
		if (!bFound)
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot find Resource " + r->getName() + 
                " to remove from list.", "ListGuiElement::removeListItem");
        }

		mResourceList.erase(i);
		layoutItems();

	}

	String ListGuiElement::getListItemName(Resource* r)
	{
		return mName + "/" + r->getName();

	}

	String ListGuiElement::getListItemPanelName(Resource* r)
	{
		return getListItemName(r) + "/" + "BackPanel";

	}

	void ListGuiElement::layoutItems()
	{

		Real currentTop = mVSpacing;
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            GuiElement* currentElement = it.getNext();

			currentElement->setTop(currentTop);
			currentTop += currentElement->getHeight() + mVSpacing;
        }
		
	}


	void ListGuiElement::setSelectedItem(GuiElement* item, bool on)
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

	void ListGuiElement::mousePressed(MouseEvent* e) 
	{
		if (mSelectedElement)
		{
			setSelectedItem(mSelectedElement,false);


		}

		mSelectedElement = static_cast<GuiElement*>(static_cast<MouseTarget*>(e->getSource()));
		setSelectedItem(mSelectedElement,true);




	}

}



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

#include "OgreListGuiElement.h"	
#include "OgreStringConverter.h"	
#include "OgreGuiManager.h"	
#include "OgreResource.h"	
#include "OgreException.h"	


namespace Ogre {

    String ListGuiElement::msTypeName = "List";
	ListGuiElement::CmdItemTemplate ListGuiElement::msCmdItemTemplate;
	ListGuiElement::CmdScrollBar ListGuiElement::msCmdScrollBar;
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
        mFirstVisibleItem = 0;
        mScrollBar = 0;
		mItemPanelMaterial = "";
		mItemPanelMaterialSelected = "";

	}
	//---------------------------------------------------------------------
	ListGuiElement::~ListGuiElement()
	{
        for (ResourceList::iterator i = mResourceList.begin(); i != mResourceList.end(); ++i)
			delete *i;
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

        dict->addParameter(ParameterDef("scroll_bar", 
            "The name of the scroll bar template"
            , PT_STRING),
            &ListGuiElement::msCmdScrollBar);

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
    String ListGuiElement::CmdItemTemplate::doGet(const void* target) const
    {
        return static_cast<const ListGuiElement*>(target)->getItemTemplateName();
    }
    void ListGuiElement::CmdItemTemplate::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<ListGuiElement*>(target)->setItemTemplateName(val);
    }
    //-----------------------------------------------------------------------
    String ListGuiElement::CmdVSpacing::doGet(const void* target) const
    {
		const ListGuiElement *t = static_cast<const ListGuiElement*>(target);
		return StringConverter::toString(t->getVSpacing());
    }
    void ListGuiElement::CmdVSpacing::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<ListGuiElement*>(target)->setVSpacing(StringConverter::parseReal(val));
    }

    //-----------------------------------------------------------------------
    String ListGuiElement::CmdScrollBar::doGet(const void* target) const
    {
        return static_cast<const ListGuiElement*>(target)->getScrollBarName();
    }
    void ListGuiElement::CmdScrollBar::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<ListGuiElement*>(target)->setScrollBarName(val);
    }


    //-----------------------------------------------------------------------
    String ListGuiElement::CmdHSpacing::doGet(const void* target) const
    {
		const ListGuiElement *t = static_cast<const ListGuiElement*>(target);
		return StringConverter::toString(t->getHSpacing());
    }
    void ListGuiElement::CmdHSpacing::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<ListGuiElement*>(target)->setHSpacing(StringConverter::parseReal(val));
    }
    //-----------------------------------------------------------------------
    String ListGuiElement::CmdItemPanelMaterialSelected::doGet(const void* target) const
    {
        return static_cast<const ListGuiElement*>(target)->getItemPanelMaterialSelected();
    }
    void ListGuiElement::CmdItemPanelMaterialSelected::doSet(void* target, const String& val)
    {
        static_cast<ListGuiElement*>(target)->setItemPanelMaterialSelected(val);
    }
    //-----------------------------------------------------------------------
    String ListGuiElement::CmdItemPanelMaterial::doGet(const void* target) const
    {
        return static_cast<const ListGuiElement*>(target)->getItemPanelMaterial();
    }
    void ListGuiElement::CmdItemPanelMaterial::doSet(void* target, const String& val)
    {
        static_cast<ListGuiElement*>(target)->setItemPanelMaterial(val);
    }
    //-----------------------------------------------------------------------
	String ListGuiElement::getItemTemplateName() const
	{
		return mItemTemplateName;

	}
	void ListGuiElement::setItemTemplateName(const String& val)
	{
		mItemTemplateName = val;

	}
    //-----------------------------------------------------------------------

	void ListGuiElement::setHSpacing(Real val)
	{
		mHSpacing = val;
	}
	Real ListGuiElement::getHSpacing() const
	{
		return  mHSpacing;

	}

    //-----------------------------------------------------------------------

	void ListGuiElement::setVSpacing(Real val)
	{
		mVSpacing = val;
	}

	Real ListGuiElement::getVSpacing() const
	{
		return  mVSpacing;
	}


	String ListGuiElement::getScrollBarName() const
	{
		assert(mScrollBar);
		return  mScrollBar->getName();

	}

	void ListGuiElement::setScrollBarName(const String& val)
	{

		if (mScrollBar != 0)
		{
			removeChild(mScrollBar->getName());
			GuiManager::getSingleton().destroyGuiElement(mScrollBar->getName());
		}
		mScrollBar = static_cast<ScrollBarGuiElement*> (
			GuiManager::getSingleton().createGuiElementFromTemplate(val, "", mName + "/ScrollBar"));

		mScrollBar->setLeft(getWidth()-mScrollBar->getWidth()-0.001);
		mScrollBar->setTop(0);
		mScrollBar->setHeight(getHeight());

		addChild(mScrollBar);
		mScrollBar->layoutItems();
		mScrollBar->addScrollListener(this);

	}


    //-----------------------------------------------------------------------
	String ListGuiElement::getItemPanelMaterial() const
	{
		return mItemPanelMaterial;

	}
	void ListGuiElement::setItemPanelMaterial(const String& val)
	{
		mItemPanelMaterial = val;

	}
    //-----------------------------------------------------------------------
	String ListGuiElement::getItemPanelMaterialSelected() const
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

		pBackPanel->setLeft(mHSpacing);
		Real scrollBarWidth = (mScrollBar) ? mScrollBar->getWidth() : 0;
		pBackPanel->setWidth(getWidth()-mHSpacing-scrollBarWidth);
		pBackPanel->setHeight(mInsideObject->getHeight());
		pBackPanel->setChildrenProcessEvents(false);
		pBackPanel->addMouseListener(this);
		pBackPanel->addMouseMotionListener(this);

		addChild(pBackPanel);

		mInsideObject->setCaption(r->getName());
		mInsideObject->setLeft(0);
		mInsideObject->setTop(0);
		mInsideObject->setWidth(pBackPanel->getWidth());


		mResourceList.push_back(r);
		pBackPanel->addChild(mInsideObject);


		layoutItems();


		if (mSelectedElement == NULL)
		{
			setSelectedItem(mInsideObject,true);
			mSelectedElement = mInsideObject;
		}
		else
		{
			setSelectedItem(mInsideObject,false);
		}

	}

	void ListGuiElement::removeListItem(Resource* r)
	{
		GuiContainer* backPanel = static_cast<GuiContainer*> (getChild(getListItemPanelName(r)));

		if (mSelectedElement == backPanel->getChild(getListItemName(r)))
			mSelectedElement = 0;


		backPanel->removeChild(getListItemName(r));
		removeChild(getListItemPanelName(r));


		GuiManager::getSingleton().destroyGuiElement(getListItemName(r));
		GuiManager::getSingleton().destroyGuiElement(getListItemPanelName(r));

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
                " to remove from list.", "ListGuiElement::removeListItem");
        }

		layoutItems();

	}

	String ListGuiElement::getListItemName(Resource* r) const
	{
		return mName + "/" + r->getName();

	}

	String ListGuiElement::getListItemPanelName(Resource* r) const
	{
		return getListItemName(r) + "/" + "BackPanel";

	}


	void ListGuiElement::layoutItems()
	{

		Real currentTop = mVSpacing;
		unsigned int currentItemNo = 0;
		mVisibleRange = 0;
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            GuiElement* currentElement = it.getNext();

			if (currentElement->getName() == mName + "/ScrollBar")
			{
				continue;
			}
			if (currentItemNo < mFirstVisibleItem) 
			{
				currentElement->hide();
			}
			else 
			{
			currentElement->setTop(currentTop);
			currentElement->_update();
			currentTop += currentElement->getHeight() + mVSpacing;
				if (currentTop > mHeight)
				{
					currentElement->hide();
				}
				else
				{
					mVisibleRange ++;
					currentElement->show();
				}
			}
			currentItemNo++;
        }
        if (mScrollBar)
			mScrollBar->setLimits(mFirstVisibleItem, mVisibleRange, mChildren.size()-1); // don't count the scroll bar child as a list item

        }
	void ListGuiElement::scrollPerformed(ScrollEvent* se)
	{
		mFirstVisibleItem = se->getTopVisible();
		layoutItems();
	}



	void ListGuiElement::setSelectedItem(GuiElement* item)
	{
		if (mSelectedElement)
		{
			setSelectedItem(mSelectedElement,false);
		}

		mSelectedElement = item;
		setSelectedItem(mSelectedElement,true);
		if (mScrollBar)
            mScrollBar->scrollToIndex(getSelectedIndex());
	}

	void ListGuiElement::setSelectedIndex(size_t index)
	{
		if (mSelectedElement)
		{
			setSelectedItem(mSelectedElement,false);
		}

		if (index < 0)
		{
			index = 0;
		}
		else if (index > getListSize())
		{
			index = getListSize();
		}

        ChildIterator it = getChildIterator();
		unsigned int indexCount = 0;
        while (it.hasMoreElements())
        {
            GuiElement* currentElement = it.getNext();
			if (currentElement->getName() == mName + "/ScrollBar")
			{
				continue;
			}
			if (indexCount == index)
			{
				mSelectedElement = static_cast<GuiContainer*>(currentElement)->getChildIterator().getNext();
				break;
			}
			indexCount++;
        }


		setSelectedItem(mSelectedElement,true);
		if (mScrollBar)
            mScrollBar->scrollToIndex(index);
	}


	void ListGuiElement::setSelectedItem(GuiElement* item, bool on)
	{
		if (item != NULL)
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

	}
	void ListGuiElement::mouseDragged(MouseEvent* e) 
	{
		// test to see if list should scroll up
		if ((e->getY() < _getDerivedTop() )&& (mFirstVisibleItem > 0))
		{
			setSelectedIndex(mFirstVisibleItem-1);
		}

		// test to see if list should scroll down
		if ((e->getY() > _getDerivedTop() + getHeight()) && (mFirstVisibleItem + mVisibleRange < getListSize()))
		{
			setSelectedIndex(mFirstVisibleItem+mVisibleRange+1);
		}
		else
		{
			GuiElement *dragTarget = findElementAt(e->getX(), e->getY());
			if (dragTarget != NULL)
			{
				if ((dragTarget->getParent() == this) &&  // is the dragTarget a child of ListGui?
					(dragTarget != mScrollBar) &&		  // ignore dragging onto the scrollbar
					(dragTarget != mSelectedElement->getParent())) // is this list item not already selected

				{
					// drag target is a backpanel for a list item
					// get the child of the backpanel (backpanel is a container).. there is only 1 child
					setSelectedItem(static_cast<GuiContainer*>(dragTarget)->getChildIterator().getNext());


				}
			}
		}
	}
	void ListGuiElement::mouseMoved(MouseEvent* e) 
	{



	}
	void ListGuiElement::mousePressed(MouseEvent* e) 
	{
		if (mSelectedElement)
		{
			setSelectedItem(mSelectedElement,false);


		}

		GuiContainer* backPanelSelected = static_cast<GuiContainer*>(static_cast<MouseTarget*>(e->getSource()));

		// get the child of the backpanel.. there is only 1 child
		mSelectedElement = backPanelSelected->getChildIterator().getNext();
		setSelectedItem(mSelectedElement,true);
	}

	void ListGuiElement::setSelectedItem(Resource* r)
	{
		GuiContainer* backPanel = static_cast<GuiContainer*> (getChild(getListItemPanelName(r)));

		setSelectedItem(backPanel->getChild(getListItemName(r)));
	}

	void ListGuiElement::setSelectedItem(Resource* r, bool on)
	{
		GuiContainer* backPanel = static_cast<GuiContainer*> (getChild(getListItemPanelName(r)));

		setSelectedItem(backPanel->getChild(getListItemName(r)), on);
	}

	Resource* ListGuiElement::getSelectedItem()
	{
		Resource* selectedResource = NULL;
        ResourceList::iterator i;
		if (mSelectedElement != NULL)
		{

			for (i = mResourceList.begin(); i != mResourceList.end(); ++i)
			{
				if ((*i)->getName() == mSelectedElement->getCaption())
				{
					selectedResource = *i;
					break;
				}
			}
		}
        return selectedResource;
	}

	int ListGuiElement::getSelectedIndex() const
	{
		int selectedIndex = -1;
		if (mSelectedElement != NULL)
		{
			ResourceList::const_iterator i;
			int currentIndex = 0;
			for (i = mResourceList.begin(); i != mResourceList.end(); ++i, currentIndex++)
			{
				if ((*i)->getName() == mSelectedElement->getCaption())
				{
					selectedIndex = currentIndex;
					break;
				}
			}
		}
        return selectedIndex;
	}



	ResourceListConstIterator ListGuiElement::getConstIterator() const
	{
        return ResourceListConstIterator(mResourceList.begin());
	}

	ResourceListConstIterator ListGuiElement::getConstEndIterator() const
	{
        return ResourceListConstIterator(mResourceList.end());
	}
	Resource* ListGuiElement::popFront()
	{
		Resource* r = mResourceList.front();
		mResourceList.pop_front();

        return r;
	}
	
	size_t ListGuiElement::getListSize() const
	{
		return mResourceList.size();
	}

    const String& ListGuiElement::getTypeName(void) const
    {
        return msTypeName;
    }


}




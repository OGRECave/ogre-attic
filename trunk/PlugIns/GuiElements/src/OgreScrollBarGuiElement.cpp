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

#include "OgreScrollBarGuiElement.h"	
#include "OgreStringConverter.h"	
#include "OgreGuiManager.h"	
#include "OgreResource.h"	
#include "OgreException.h"	


namespace Ogre {

    String ScrollBarGuiElement::msTypeName = "ScrollBar";
	ScrollBarGuiElement::CmdUpButton ScrollBarGuiElement::msCmdUpButton;
	ScrollBarGuiElement::CmdDownButton ScrollBarGuiElement::msCmdDownButton;
	ScrollBarGuiElement::CmdScrollBit ScrollBarGuiElement::msCmdScrollBit;
	static Real mouseDragBitOffset = 0;

	ScrollBarGuiElement::ScrollBarGuiElement(const String& name) :
		PanelGuiElement(name)
	{
        if (createParamDictionary("ScrollBarGuiElement"))
        {
            addBaseParameters();
        }
		mUpButton = 0;
		mDownButton = 0;
		mScrollBit = 0;
		mTotalItems = 0;
		mStartingItem = 0;
		mVisibilityRange = 0;
		mouseHeldAtY = -1;

		mSpacing = 0.001;
	}

    //---------------------------------------------------------------------
    void ScrollBarGuiElement::addBaseParameters(void)
    {
        PanelGuiElement::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("up_button", 
            "The template of Up Button."
            , PT_STRING),
            &ScrollBarGuiElement::msCmdUpButton);

        dict->addParameter(ParameterDef("down_button", 
            "The template of Down Button."
            , PT_STRING),
            &ScrollBarGuiElement::msCmdDownButton);

        dict->addParameter(ParameterDef("scroll_bit", 
            "The template of Scroll Bit."
            , PT_STRING),
            &ScrollBarGuiElement::msCmdScrollBit);
	}
    //---------------------------------------------------------------------
    // Command objects
    //---------------------------------------------------------------------

    //-----------------------------------------------------------------------
    String ScrollBarGuiElement::CmdUpButton::doGet(const void* target) const
    {
        return static_cast<const ScrollBarGuiElement*>(target)->getUpButtonName();
    }
    void ScrollBarGuiElement::CmdUpButton::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<ScrollBarGuiElement*>(target)->setUpButtonName(val);
    }
    //-----------------------------------------------------------------------
    String ScrollBarGuiElement::CmdDownButton::doGet(const void* target) const
    {
        return static_cast<const ScrollBarGuiElement*>(target)->getDownButtonName();
    }
    void ScrollBarGuiElement::CmdDownButton::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<ScrollBarGuiElement*>(target)->setDownButtonName(val);
    }
    //-----------------------------------------------------------------------
    String ScrollBarGuiElement::CmdScrollBit::doGet(const void* target) const
    {
        return static_cast<const ScrollBarGuiElement*>(target)->getScrollBitName();
    }
    void ScrollBarGuiElement::CmdScrollBit::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val);

        static_cast<ScrollBarGuiElement*>(target)->setScrollBitName(val);
    }
    //-----------------------------------------------------------------------

	String ScrollBarGuiElement::getUpButtonName() const
	{
		return mUpButtonName;
	}
	String ScrollBarGuiElement::getDownButtonName() const
	{
		return mDownButtonName;
	}
	String ScrollBarGuiElement::getScrollBitName() const
	{
		return mScrollBitName;
	}
//-----------------------------------------------------------------------

	void ScrollBarGuiElement::setUpButtonName(const String& val)
	{
		mUpButtonName = val;
		Real buttonSize = getWidth();
		mUpButton = static_cast<ButtonGuiElement*> (
			GuiManager::getSingleton().createGuiElementFromTemplate(mUpButtonName, "", mName + "/" + "UpButton"));


		// do not make this cloneable, otherwise there will be 2 copies of it when it is cloned,
		// one copy when the children are copied, and another copy when setUpButtonName is set.
		mUpButton->setCloneable(false);

		addChild(mUpButton);
//		mUpButton->setButtonCaption("SS/Templates/BasicText", "UP");
		mUpButton->addActionListener(this);
	}
	void ScrollBarGuiElement::setDownButtonName(const String& val)
	{
		Real buttonSize = getWidth();
		mDownButtonName = val;
		mDownButton = static_cast<ButtonGuiElement*> (
			GuiManager::getSingleton().createGuiElementFromTemplate(mDownButtonName, "", mName + "/" + "DownButton"));

		// do not make this cloneable, otherwise there will be 2 copies of it when it is cloned,
		// one copy when the children are copied, and another copy when setDownButtonName is set.
		mDownButton->setCloneable(false);
		addChild(mDownButton);
//		mDownButton->setButtonCaption("SS/Templates/BasicText", "DOWN");
		mDownButton->addActionListener(this);
	}
	void ScrollBarGuiElement::setScrollBitName(const String& val)
	{
		Real buttonSize = getWidth();
		mScrollBitName = val;
		mScrollBit = static_cast<PanelGuiElement*> (
			GuiManager::getSingleton().createGuiElementFromTemplate(mScrollBitName, "", mName + "/" + "ScrollBit"));
		// do not make this cloneable, otherwise there will be 2 copies of it when it is cloned,
		// one copy when the children are copied, and another copy when setScrollBitName is set.
		mScrollBit->setCloneable(false);
		mScrollBit->addMouseMotionListener(this);
		mScrollBit->addMouseListener(this);
		addMouseListener(this);

		addChild(mScrollBit);

	}
//-----------------------------------------------------------------------

	void ScrollBarGuiElement::setLimits(size_t first, size_t visibleRange, size_t total)
	{
		mTotalItems = total;
		mStartingItem = first;
		mVisibilityRange = visibleRange;

		layoutItems();

	}
	void ScrollBarGuiElement::layoutItems()
	{
		Real buttonWidth = getWidth() - (mSpacing * 2);
		Real buttonHeight = (buttonWidth * 4.0F) / 3.0F;	// adjust for screen ratio
		Real horzSpacing = mSpacing;
		Real vertSpacing = (mSpacing * 4.0F) / 3.0F;
	    Real bitTop    = buttonHeight + vertSpacing;
		Real bitHeight = getHeight() - (2 * bitTop);

		mUpButton->setLeft(horzSpacing);
		mUpButton->setTop(vertSpacing);
		mUpButton->setWidth(buttonWidth);
		mUpButton->setHeight(buttonHeight);	// buttons are square

		mDownButton->setLeft(horzSpacing);
		mDownButton->setTop(getHeight() - (buttonHeight + vertSpacing));
		mDownButton->setWidth(buttonWidth);
		mDownButton->setHeight(buttonHeight);	// buttons are square

		mScrollBit->setLeft(horzSpacing);
		mScrollBit->setTop(buttonHeight + vertSpacing);
		mScrollBit->setWidth(buttonWidth);

		if (mTotalItems == 0)
		{
			mScrollBit->setTop(bitTop);
			mScrollBit->setHeight(bitHeight);
		}
		else
		{
			mScrollBit->setTop(bitTop + (bitHeight * ((Real)mStartingItem / (Real)mTotalItems)));
			mScrollBit->setHeight(bitHeight * ((Real)mVisibilityRange / (Real)mTotalItems));
		}
	}

	void ScrollBarGuiElement::updateScrollBit()
	{
		Real buttonWidth = getWidth() - (mSpacing * 2);
		Real buttonHeight = buttonWidth * (4.0F / 3.0F);	// adjust for screen ratio
		Real vertSpacing = mSpacing * (4.0F / 3.0F);
		Real bitTop    = buttonHeight + vertSpacing;
		Real bitHeight = getHeight() - (2 * bitTop);

		if (mTotalItems == 0)
		{
			mScrollBit->setTop(bitTop);
		}
		else
		{
			mScrollBit->setTop(bitTop + (bitHeight * ((Real)mStartingItem / (Real)mTotalItems)));
		}
	}

	//---------------------------------------------------------------------
	const String& ScrollBarGuiElement::getTypeName(void) const
	{
		return msTypeName;
	}

	void ScrollBarGuiElement::actionPerformed(ActionEvent* e) 
	{
		if (e->getActionCommand() == mUpButton->getName())
		{
			if (mStartingItem >0)
			{
				mStartingItem--;
				updateScrollBit();
				fireScrollPerformed();
			}
		}
		else if (e->getActionCommand() == mDownButton->getName())
		{
			if (mStartingItem < mTotalItems-mVisibilityRange)
			{
				mStartingItem++;
				updateScrollBit();
				fireScrollPerformed();
			}
		}


	}
    //-----------------------------------------------------------------------
	void ScrollBarGuiElement::fireScrollPerformed()
	{
		ScrollEvent* se = new ScrollEvent(this, ScrollEvent::SE_SCROLL_PERFORMED, 0, 0, mStartingItem, mVisibilityRange, mTotalItems);
		processEvent(se);
		delete se;
	}

    //-----------------------------------------------------------------------
	void ScrollBarGuiElement::processEvent(InputEvent* e) 
	{
		PanelGuiElement::processEvent(e);

		if (!e->isConsumed())
		{
			switch(e->getID()) 
			{
			case ScrollEvent::SE_SCROLL_PERFORMED:
				processScrollEvent(static_cast<ScrollEvent*>(e));
				break;
			default:
				break;
			}
		}
	}
	void ScrollBarGuiElement::mouseMoved(MouseEvent* e)
	{


	}
	void ScrollBarGuiElement::mouseDragged(MouseEvent* e)
	{
		if ( mouseHeldAtY == -1 || mouseDragBitOffset == -1 )
		{
			int err =1;
		}
		else
		{
			Real buttonHeight = (getWidth() * 4.0F) / 3.0F;	// adjust for screen ratio
			Real vertSpacing = (mSpacing * 4.0F) / 3.0F;
			Real moveY = (e->getY()-getTop()-vertSpacing-buttonHeight) - mouseDragBitOffset;
			moveScrollBitTo(moveY);
		}

	}
	void ScrollBarGuiElement::mousePressed(MouseEvent* e) 
	{
		Real buttonHeight = (getWidth() * 4.0F) / 3.0F;	// adjust for screen ratio
		Real vertSpacing = (mSpacing * 4.0F) / 3.0F;

		Real mouseY = e->getY() - mDerivedTop;
		if ((MouseTarget*)e->getSource() == (GuiElement*)(mScrollBit))
		{
			mouseHeldAtY = mouseY;
			mouseDragBitOffset = e->getY() - getTop() - mScrollBit->getTop();
		}
		else if ((MouseTarget*)e->getSource() == (GuiElement*)this)
		{
			size_t newStartingItem = (int)mStartingItem;
			if ( mouseY < mScrollBit->getTop() )
			{
				if ( newStartingItem < 5 )
					newStartingItem = 0;
				else
					newStartingItem -= 5;
			}
			else
			{
				newStartingItem += 5;
				size_t maxStartingItem = mTotalItems - mVisibilityRange;
				if ( newStartingItem > maxStartingItem )
					newStartingItem = maxStartingItem;
			}
			if ( newStartingItem != mStartingItem )
			{
				mStartingItem = newStartingItem;
				updateScrollBit();
				fireScrollPerformed();
			}
			/*Real topToScroll = mouseY;
			if (mouseY > mScrollBit->getTop())
			{
				// always take scroll point from the top of scrollBit
				topToScroll -= mScrollBit->getHeight();

			}

			moveScrollBitTo(topToScroll - buttonHeight + vertSpacing);*/

			//mouseHeldAtY = mouseY;
		}
	}
	void ScrollBarGuiElement::mouseReleased(MouseEvent* e) 
	{
		mouseHeldAtY = -1;
		mouseDragBitOffset = -1;
	}
	void ScrollBarGuiElement::scrollToIndex(size_t index)
	{
		if (index >= mStartingItem + mVisibilityRange)
		{
			// scroll down
			mStartingItem = index - mVisibilityRange;
		}
		else if (index < mStartingItem)
		{
			// scroll up
			mStartingItem = index;
		}
		layoutItems();
		fireScrollPerformed();
	}

	void ScrollBarGuiElement::moveScrollBitTo(Real moveY)
	{
		Real buttonHeight = (getWidth() * 4.0F) / 3.0F;	// adjust for screen ratio
		Real vertSpacing = (mSpacing * 4.0F) / 3.0F;

		if (moveY <0)
		{
			moveY = 0;
		}
		Real maxY = getHeight() - buttonHeight - vertSpacing*2.f - mScrollBit->getHeight();
		if (moveY > maxY)
		{
			moveY = maxY;
		}
		mScrollBit->setTop(buttonHeight + vertSpacing + moveY);
		mStartingItem = ((mScrollBit->getTop() - buttonHeight - vertSpacing) * mTotalItems) / (getHeight() - buttonHeight - vertSpacing*2);
		fireScrollPerformed();
	}
}


/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as 
published by the Free Software Foundation; either version 2.1 of the 
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
License for more details.

You should have received a copy of the GNU Lesser General Public License 
along with this library; if not, write to the Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/

#include "OgreActionEvent.h"
#include "OgreTextBoxGuiElement.h"
#include "OgreKeyEvent.h"
#include "OgreInput.h"
#include "OgreGuiManager.h"
#include "OgreException.h"

namespace Ogre {

    //---------------------------------------------------------------------
    String TextBoxGuiElement::msTypeName = "TextBox";
    TextBoxGuiElement::CmdBackPanel TextBoxGuiElement::msCmdBackPanel;
    TextBoxGuiElement::CmdTextArea TextBoxGuiElement::msCmdTextArea;
    //---------------------------------------------------------------------
    TextBoxGuiElement::TextBoxGuiElement(const String& name)
        : PanelGuiElement(name)
    {
        if (createParamDictionary("TextBoxGuiElement"))
        {
            addBaseParameters();
        }

        mActionOnReturn = false;
		setTransparent(true);
		mBackPanel = 0;
		mTextArea = 0;
		mChildrenProcessEvents = false;
    }


    //---------------------------------------------------------------------
    TextBoxGuiElement::~TextBoxGuiElement()
    {
    }
    //---------------------------------------------------------------------
    const String& TextBoxGuiElement::getTypeName(void) const
    {
        return msTypeName;
    }
    //---------------------------------------------------------------------
    void TextBoxGuiElement::addBaseParameters(void)
    {
        PanelGuiElement::addBaseParameters();
        ParamDictionary* dict = getParamDictionary();

        dict->addParameter(ParameterDef("back_panel", 
           "The template name of the panel to be used behind the text."
            , PT_STRING),
            &msCmdBackPanel);
        dict->addParameter(ParameterDef("text_area", 
           "The template name of the textArea to be used for the text."
            , PT_STRING),
            &msCmdTextArea);

    }
    //---------------------------------------------------------------------------------------------
    // Char height command object
    //
    String TextBoxGuiElement::CmdBackPanel::doGet( const void* target ) const
    {
        return static_cast< const TextBoxGuiElement* >( target )->getBackPanelName();
    }
    void TextBoxGuiElement::CmdBackPanel::doSet( void* target, const String& val )
    {
        std::vector<String> vec = StringUtil::split(val, "\t\n ", 1);

		if (vec.size() < 2)
		{
			static_cast<TextBoxGuiElement*>(target)->setBackPanel(val, 5);
		}
		else
		{
			static_cast<TextBoxGuiElement*>(target)->setBackPanel(vec[0], StringConverter::parseInt(vec[1]));
		}

    }
    //---------------------------------------------------------------------------------------------
    String TextBoxGuiElement::CmdTextArea::doGet( const void* target ) const
    {
        return static_cast< const TextBoxGuiElement* >( target )->getTextAreaName();
    }

    //-----------------------------------------------------------------------
    void TextBoxGuiElement::CmdTextArea::doSet(void* target, const String& val)
    {
        std::vector<String> vec = StringUtil::split(val, "\t\n ", 1);


		if (vec.size() < 2)
		{
			static_cast<TextBoxGuiElement*>(target)->setTextArea(val, String(""));
		}
		else
		{
			static_cast<TextBoxGuiElement*>(target)->setTextArea(vec[0], vec[1]);
		}
    }

    //-----------------------------------------------------------------------
    String TextBoxGuiElement::getTextAreaName() const
	{
		return mTextAreaTemplateName + " " + mCaption;
	}
    //---------------------------------------------------------------------------------------------
    String TextBoxGuiElement::getBackPanelName() const
	{
		return mBackPanelTemplateName;
	}
    
    //-----------------------------------------------------------------------
    void TextBoxGuiElement::setTextArea(const String& templateName, const String& name)
    {
		mTextAreaTemplateName = templateName;
		if (mTextArea)
		{
			removeChild(mTextArea->getName());
			GuiManager::getSingleton().destroyGuiElement(mTextArea);
			mTextArea = NULL;
		}

		mTextArea = static_cast<TextAreaGuiElement*>
			(GuiManager::getSingleton().createGuiElementFromTemplate(mTextAreaTemplateName, "", mName + "/textArea"));

		mCaption = name;
		setCaptionToTextArea();

		// do not make this cloneable, otherwise there will be 2 copies of it when it is cloned,
		// one copy when the children are copied, and another copy when setTextArea is set.
		mTextArea->setCloneable(false);
		mTextArea->setTop(0);
		mTextArea->setLeft(mTextArea->getSpaceWidth()/2);

		if (mBackPanel)
		{
//			mBackPanel->setHeight(mTextArea->getCharHeight());
			mBackPanel->addChild(mTextArea);
		}
		else
		{
			// The textarea was created first, so add it to backpanel when backpanel is created

		}
    }
    //-----------------------------------------------------------------------
    void TextBoxGuiElement::setBackPanel(const String& templateName, int size)
    {
		mTextAreaSize = size;
		mBackPanelTemplateName = templateName;
		if (mBackPanel)
		{
			removeChild(mBackPanel->getName());
			GuiManager::getSingleton().destroyGuiElement(mBackPanel);
			mBackPanel = NULL;
		}

		mBackPanel = static_cast<GuiContainer*>
			(GuiManager::getSingleton().createGuiElementFromTemplate(mBackPanelTemplateName, "", mName + "/backPanel"));

		// do not make this cloneable, otherwise there will be 2 copies of it when it is cloned,
		// one copy when the children are copied, and another copy when setTextArea is set.
		mBackPanel->setCloneable(false);
		mBackPanel->setDimensions(getWidth(),getHeight());
		mBackPanel->setTop(0);
		mBackPanel->setLeft(0);
		mBackPanel->setWidth(getWidth());

		addChild(mBackPanel);
		if (mTextArea)
		{
//			mBackPanel->setHeight(mTextArea->getCharHeight());
			mBackPanel->addChild(mTextArea);
		}
		else
		{
			// The BackPanel was created first, so add it to backpanel when textarea is created

		}
    }

  void TextBoxGuiElement::setCaption(const String& text)
  {
    mCaption = text;
    if (mTextArea != NULL)
    		mTextArea->setCaption(text);
  }

  const String& TextBoxGuiElement::getCaption(void) const
  {
    if (mTextArea != NULL)
      return mTextArea->getCaption();
    return mCaption;
  }


    //---------------------------------------------------------------------------------------------
	void TextBoxGuiElement::setCaptionToTextArea() 
	{
		mTextArea->setCaption(mCaption);
	}
    //---------------------------------------------------------------------------------------------
	void TextBoxGuiElement::processEvent(InputEvent* e) 
	{
		PanelGuiElement::processEvent(e);

		if (mTextArea)
		{
			if (!e->isConsumed())
			{
				switch(e->getID()) 
				{
				case KeyEvent::KE_KEY_PRESSED:
					KeyEvent* ke = static_cast<KeyEvent*> (e);

                    switch (ke->getKey())
                    {
                    case KC_BACK :
		                mCaption = mCaption.substr(0,mCaption.length() -1);
			            setCaptionToTextArea();
                        break;

                    case KC_RETURN :
                        if (mActionOnReturn)
                        {
                            ActionEvent* ae = new ActionEvent(this, ActionEvent::AE_ACTION_PERFORMED, 0, 0, mName);
		                    processActionEvent(ae);
		                    delete ae;
                        }
                        // fall through

                    default : {
			            OgreChar newKey = ke->getKeyChar();
                        if (newKey != '\0')
                        {
			                Font* font = static_cast<Font*> (FontManager::getSingleton().getByName(mTextArea->getFontName()));
			                if (!font)
			                    Except( Exception::ERR_ITEM_NOT_FOUND, "Could not find font " + mTextArea->getFontName(),
				                    "TextBoxGuiElement::processEvent" );
                  						
			                if (font->getGlyphAspectRatio(newKey)*mTextArea->getCharHeight() + mTextArea->getWidth() < getWidth() - mTextArea->getSpaceWidth())
			                {
			                    mCaption += newKey;
				                setCaptionToTextArea();
			                }
                        }
                        }
                        break;
				    }
					break;
				}
			}
		}
	}

}


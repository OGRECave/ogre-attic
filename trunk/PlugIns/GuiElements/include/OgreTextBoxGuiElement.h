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

#ifndef _TextBoxGuiElement_H__
#define _TextBoxGuiElement_H__

#include "OgrePrerequisites.h"
#include "OgreTextAreaGuiElement.h"
#include "OgrePanelGuiElement.h"

namespace Ogre
{
    /** GuiElement representing a flat, single-material (or transparent) panel which can contain other elements.
    @remarks
    This class subclasses GuiContainer because it can contain other elements. Like other
    containers, if hidden it's contents are also hidden, if moved it's contents also move etc. 
    The panel itself is a 2D rectangle which is either completely transparent, or is rendered 
    with a single material. The texture(s) on the panel can be tiled depending on your requirements.
    @par
    This component is suitable for backgrounds and grouping other elements. Note that because
    it has a single repeating material it cannot have a discrete border (unless the texture has one and
    the texture is tiled only once). For a bordered panel, see it's subclass BorderTextBoxGuiElement.
    @par
    Note that the material can have all the usual effects applied to it like multiple texture
    layers, scrolling / animated textures etc. For multiple texture layers, you have to set 
    the tiling level for each layer.
    */
    class _OgreGuiElementExport TextBoxGuiElement : public PanelGuiElement
    {
    public:
        /** Constructor. */
        TextBoxGuiElement(const String& name);
        ~TextBoxGuiElement();


        /** See GuiElement. */
        const String& getTypeName(void) const;

        // inherited
        void setCaption(const String& text);
        const String& getCaption(void) const;

	    void addBaseParameters(void);

		inline bool isKeyEnabled() const
		{ return true; }
		
		void processEvent(InputEvent* e) ;

        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdBackPanel : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdTextArea : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
    void setBackPanel(const String& templateName, int size);
    void setTextArea(const String& templateName, const String& name);
    String getBackPanelName() const;
    String getTextAreaName() const;

    protected:

		void setCaptionToTextArea() ;

        bool mActionOnReturn;
		GuiContainer* mBackPanel;
		TextAreaGuiElement* mTextArea;
		String mTextAreaTemplateName;
		String mBackPanelTemplateName;
        static String msTypeName;
        static CmdBackPanel msCmdBackPanel;
        static CmdTextArea msCmdTextArea;
		int mTextAreaSize;
    };
}

#endif


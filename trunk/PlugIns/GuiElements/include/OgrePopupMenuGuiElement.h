/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#ifndef __PopupMenuGuiElement_H__
#define __PopupMenuGuiElement_H__

#include "OgreGuiElementPrerequisites.h"
#include "OgreListSelectionTarget.h"
#include "OgrePanelGuiElement.h"
#include "OgreListChanger.h"
#include "OgreGuiPressable.h"
#include "OgreStringResource.h"

namespace Ogre {


	typedef std::list<Resource*> ResourceList;
    
	/**
	 * The listener interface for receiving "interesting" mouse events 
	 * (press, release, click, enter, and exit) on a component.
	 * (To track mouse moves and mouse drags, use the MouseMotionListener.)
	 * <P>
	 * The class that is interested in processing a mouse event
	 * either implements this interface (and all the methods it
	 * contains) or extends the abstract <code>MouseAdapter</code> class
	 * (overriding only the methods of interest).
	 * <P>
	 * The listener object created from that class is then registered with a
	 * component using the component's <code>addMouseListener</code> 
	 * method. A mouse event is generated when the mouse is pressed, released
	 * clicked (pressed and released). A mouse event is also generated when
	 * the mouse cursor enters or leaves a component. When a mouse event
	 * occurs the relevant method in the listener object is invoked, and 
	 * the <code>MouseEvent</code> is passed to it.
	 *
	 */
	class _OgreGuiElementExport PopupMenuGuiElement : public PanelGuiElement, public ListSelectionTarget, public ListChanger, public GuiPressable, public MouseMotionListener
    {
	public :
//	    void addBaseParameters(void);

		PopupMenuGuiElement(const String& name);
		virtual ~PopupMenuGuiElement();

        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdItemTemplate : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdVSpacing : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdHSpacing : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdItemPanelMaterial : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

        /** Command object for specifying texture coordinates for the border (see ParamCommand).*/
        class CmdItemPanelMaterialSelected : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

		void setItemTemplateName(const String& val);
		void addListItem(Resource* r);
		void removeListItem(Resource* r);
		String getItemTemplateName() const;
		String getVSpacing() const;
		String getHSpacing() const;
		void setVSpacing(const String& val);
		void setHSpacing(const String& val);

		String getItemPanelMaterial() const;
		String getItemPanelMaterialSelected() const;
		void setItemPanelMaterial(const String& val);
		void setItemPanelMaterialSelected(const String& val);
		Resource* getSelectedItem();

		void setSelectedItem(Resource* r, bool on);
		void setSelectedItem(Resource* r);
		ResourceListConstIterator getConstIterator() const;
		ResourceListConstIterator getConstEndIterator() const;
		Resource* popFront() ;
		size_t getListSize() const;

    protected:
		void setSelectedItem(GuiElement* item, bool on);
		void setSelectedItem(GuiElement* item);

		void mouseMoved(MouseEvent* e);
		void mouseDragged(MouseEvent* e);
		void mouseExited(void);
		void mousePressed(void);
		void mouseReleased(void);

	    void addBaseParameters(void);
		String getListItemName(Resource* r);
		String getListItemPanelName(Resource* r);

		void addMenuItem(Resource* r);
		void removeMenuItem(Resource* r);

        static CmdItemTemplate msCmdItemTemplate;
        static CmdVSpacing msCmdVSpacing;
        static CmdHSpacing msCmdHSpacing;
        static CmdItemPanelMaterial msCmdItemPanelMaterial;
        static CmdItemPanelMaterialSelected msCmdItemPanelMaterialSelected;
        static String msTypeName;

		void layoutItems();
		Real mVSpacing;
		Real mHSpacing;

		ushort mPixelVSpacing;
		ushort mPixelHSpacing;
		String mItemTemplateName;
		String mItemPanelMaterial;
		String mItemPanelMaterialSelected;


		ResourceList mResourceList;
		StringResource *mSelectedMenuItem;
		StringResource *mSeparatormenuItem;

		GuiElement* mSelectedElement;

		String mSelectedString;
    };
}

#endif  // __PopupMenuGuiElement_H__

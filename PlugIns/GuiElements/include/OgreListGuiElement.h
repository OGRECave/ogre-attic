/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
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
#ifndef __ListGuiElement_H__
#define __ListGuiElement_H__

#include "OgreGuiElementPrerequisites.h"
#include "OgreListSelectionTarget.h"
#include "OgreBorderPanelGuiElement.h"
#include "OgreListChanger.h"
#include "OgreScrollBarGuiElement.h"
#include "OgreEventListeners.h"

namespace Ogre {


    
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

	class _OgreGuiElementExport ListGuiElement : public PanelGuiElement, public ListSelectionTarget, public ListChanger, public ScrollListener, public MouseListener, public MouseMotionListener
    {
	public :

		ListGuiElement(const String& name);
		~ListGuiElement();

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

        /** Command object for specifying the scrollbar for the list (see ParamCommand).*/
        class CmdScrollBar : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };



		virtual void setItemTemplateName(const String& val);
		virtual void addListItem(Resource* r);
		virtual void removeListItem(Resource* r);
		virtual String getItemTemplateName() const;
 		virtual Real getVSpacing() const;
 		virtual Real getHSpacing() const;
 		virtual void setVSpacing(Real val);
 		virtual void setHSpacing(Real val);

		void mouseClicked(MouseEvent* e) {};
		void mouseEntered(MouseEvent* e)  {};
		void mouseExited(MouseEvent* e)  {};
		void mousePressed(MouseEvent* e) ;
		void mouseReleased(MouseEvent* e) {};
		void mouseDragged(MouseEvent* e) ;
		void mouseMoved(MouseEvent* e) ;
		void setSelectedIndex(size_t index);

		virtual String getItemPanelMaterial() const;
		virtual String getItemPanelMaterialSelected() const;
		virtual void setItemPanelMaterial(const String& val);
		virtual void setItemPanelMaterialSelected(const String& val);

		virtual void setScrollBarName(const String& val);
		virtual String getScrollBarName() const;

		void scrollPerformed(ScrollEvent* se);

		virtual void setSelectedItem(Resource* r, bool on);
		virtual void setSelectedItem(Resource* r);


		virtual Resource* getSelectedItem();
		virtual int getSelectedIndex() const;

		virtual ResourceListConstIterator getConstIterator() const;
		virtual ResourceListConstIterator getConstEndIterator() const;

		virtual Resource* popFront() ;
		virtual size_t getListSize() const;

        /** See GuiElement. */
        const String& getTypeName(void) const;


    protected:
		virtual void setSelectedItem(GuiElement* item, bool on);
		virtual void setSelectedItem(GuiElement* item);

	    void addBaseParameters(void);
		String getListItemName(Resource* r) const;
		String getListItemPanelName(Resource* r) const;

        static CmdItemTemplate msCmdItemTemplate;
        static CmdVSpacing msCmdVSpacing;
		static CmdScrollBar msCmdScrollBar;
        static CmdHSpacing msCmdHSpacing;
        static CmdItemPanelMaterial msCmdItemPanelMaterial;
        static CmdItemPanelMaterialSelected msCmdItemPanelMaterialSelected;
        static String msTypeName;

		virtual void layoutItems();
		size_t mFirstVisibleItem, mVisibleRange;
		Real mVSpacing;
		Real mHSpacing;

		ushort mPixelVSpacing;
		ushort mPixelHSpacing;
		String mItemTemplateName;
		String mItemPanelMaterial;
		String mItemPanelMaterialSelected;
		ScrollBarGuiElement* mScrollBar;

		ResourceList mResourceList; 

		GuiElement* mSelectedElement;
    };

}


#endif  // __ListGuiElement_H__


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
#ifndef __CursorGuiElement_H__
#define __CursorGuiElement_H__

#include "OgreGuiElementPrerequisites.h"
#include "OgreEventListeners.h"
#include "OgrePanelGuiElement.h"

namespace Ogre {

    
	/** GuiElement representing a visual cursor.
     * @remarks
	 * The CursorGuiElement may be used to create visible cursors for use with
     * the OverlayManager's cursor facility.
     * @par
     * The CursorGuiElements class is sub-classed from the the PanelGuiElement
     * class.  The element size, material and other attributes may be specified to
     * define the appearance of the cursor.
     * @par
     * In addition, the CursorGuiElement class provide a mechanism for specifing
     * the cursor's <I>hot spot</I> in relation to the PanelGuiClass's visual area.  This
     * is specified by setting the x_offset and y_offset attributes of the cursor.
     * @par
     * To use a cursor, it must first be created (typically from an overlay script)
     * and then be passed as both parameters to the overlayManager's <I>setCursorGui</I>
     * method.  The OverlayManager will activate the cursor by calling it's <I>show</I>
     * method.  In response, the cursor will position itself with the <I>hot spot</I>
     * over the current mouse coordinates.  All further mouse cursor events from the
     * OverlayManager will then be sent to the new cursor, causing it to track with the
     * mouse.
     * @par
     * Any previous cursor maintained by the OverlayManager is deactivated (by calling
     * it's <I>hide</I> method) and subsequently discarded.  If the previous cursor is to
     * be retained for further use, then the OverlayManager's <I>getCursorGui</I> method
     * should be called to retrieve it prior to calling <I>setCursorGui</I>.
     * @par
     * Unlike most other Gui elements, cursor's are created with the mVisible attribute
     * initially set to false.
     *
     * @see OverlayManager::getCursorGui.
     * @see OverlayManager::setCursorGui.
	 */

	class CursorGuiElement : public PanelGuiElement, public MouseMotionListener
    {
	public :
		CursorGuiElement(const String& name);

		const String& getTypeName(void) const;

        /** Inherited from PanelGuiElement, should only be called from the OverlayManager */
        void show(void);
        void hide(void);

        /** Used to set the cursor's hot spot X coordinate */
        void setOffsetX(Real x);
        /** Used to set the cursor's hot spot Y coordinate */
        void setOffsetY(Real y);
        /** Used to get the cursor's hot spot X coordinate */
        Real getOffsetX(void) const { return mOffsetX; }
        /** Used to get the cursor's hot spot Y coordinate */
        Real getOffsetY(void) const { return mOffsetY; }
		
        /** Used to update the location of the cursor on screen, should only be called from the OverlayManager */
        void mouseMoved(MouseEvent* e);
        /** Used to update the location of the cursor on screen, should only be called from the OverlayManager */
		void mouseDragged(MouseEvent* e);

		float getViewDepth( Camera * ) const { return 0.0; }

		GuiElement* findElementAt(Real x, Real y);

        /** Command object for specifying the cursor's X offset (see ParamCommand).*/
        class CmdOffsetX : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for specifying the cursor's Y offset (see ParamCommand).*/
        class CmdOffsetY : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };

    protected:
        static String msTypeName;

        /** The current X coordinate of the cursor's <I>hot spot</I> */
        Real mOffsetX;
        /** The current Y coordinate of the cursor's <I>hot spot</I> */
        Real mOffsetY;
        
            // Command objects
        static CmdOffsetX  msCmdOffsetX;
        static CmdOffsetY  msCmdOffsetY;

        void addBaseParameters();
    };

}


#endif  // __CursorGuiElement_H__

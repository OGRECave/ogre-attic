/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/

#ifndef __GuiElement_H__
#define __GuiElement_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"

namespace Ogre {


    /** Abstract definition of a 2D element to be displayed in an Overlay.
    @remarks
        This class abstracts all the details of a 2D element which will appear in
        an overlay. In fact, not all GuiElement instances can be directly added to an
        Overlay, only those which are GuiContainer instances (a subclass of this class).
        GuiContainer objects can contain any GuiElement however. This is just to 
        enforce some level of grouping on widgets.
    @remarks
        GuiElements should be managed using GuiManager. This class is responsible for
        instantiating / deleting elements, and also for accepting new types of element
        from plugins etc.
    @remarks
        Because this class is designed to be extensible, it subclasses from StringInterface
        so its parameters can be set in a generic way.
    */
    class _OgreExport GuiElement : public StringInterface, public Renderable
    {
    public:

    protected:
        String mName;
        bool mVisible;
        Real mLeft;
        Real mTop;
        Real mWidth;
        Real mHeight;
        Material* pMaterial;

    public:
        /// Constructor: do not call direct, use GuiManager::createElement
        GuiElement(const String& name);
        virtual ~GuiElement();

        /** Gets the name of this overlay. */
        const String& getName(void) const;


        /** Shows this element if it was hidden. */
        void show(void);

        /** Hides this element if it was visible. */
        void hide(void);

        /** Sets the dimensions of this element in relation to the screen (1.0 = screen width/height). */
        void setDimensions(Real width, Real height);

        /** Sets the position of the top-left corner of the element, relative to the screen size
            (1.0 = screen width / height) */
        void setPosition(Real left, Real top);

        /** Sets the width of this element in relation to the screen (where 1.0 = screen width) */
        void setWidth(Real width);
        /** Gets the width of this element in relation to the screen (where 1.0 = screen width) */
        Real getWidth(void) const;

        /** Sets the height of this element in relation to the screen (where 1.0 = screen height) */
        void setHeight(Real height);
        /** Gets the height of this element in relation to the screen (where 1.0 = screen height) */
        Real getHeight(void) const;

        /** Sets the left of this element in relation to the screen (where 0 = far left, 1.0 = far right) */
        void setLeft(Real left);
        /** Gets the left of this element in relation to the screen (where 0 = far left, 1.0 = far right)  */
        Real getLeft(void) const;

        /** Sets the top of this element in relation to the screen (where 0 = top, 1.0 = bottom) */
        void setTop(Real Top);
        /** Gets the top of this element in relation to the screen (where 0 = top, 1.0 = bottom)  */
        Real getTop(void) const;


        /** Gets the name of the material this element uses. */
        virtual const String& getMaterialName(void) const;

        /** Sets the name of the material this element will use. */
        virtual void setMaterialName(const String& matName);


        // --- Renderable Overrides ---
        /** See Renderable */
        Material* getMaterial(void) const;
        /** See Renderable */
        void getRenderOperation(RenderOperation& rend);
        /** See Renderable */
        void getWorldTransforms(Matrix4* xform);



    };



}


#endif


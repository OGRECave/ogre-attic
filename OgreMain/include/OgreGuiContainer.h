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

#ifndef __GuiContainer_H__
#define __GuiContainer_H__

#include "OgrePrerequisites.h"
#include "OgreGuiElement.h"

namespace Ogre {


    /** A 2D element which contains other GuiElement instances.
    @remarks
        This is a specialisation of GuiElement for 2D elements that contain other
        elements. These are also the smallest elements that can be attached directly
        to an Overlay.
    @remarks
        GuiContainers should be managed using GuiManager. This class is responsible for
        instantiating / deleting elements, and also for accepting new types of element
        from plugins etc.
    */
    class _OgreExport GuiContainer : public GuiElement
    {
    public:
        typedef std::map<String, GuiElement*> ChildMap;
        typedef MapIterator<ChildMap> ChildElementIterator;
    protected:
        ChildMap mChildren;
    public:
        /// Constructor: do not call direct, use GuiManager::createContainer
        GuiContainer(const String& name);
        virtual ~GuiElement();

        /** Adds another GuiElement to this container. */
        void addElement(GuiElement* elem);
        /** Removes a named element from this container. */
        void removeElement(const String& elemName);




    };



}


#endif


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
#ifndef __GuiElementFactory_H__
#define __GuiElementFactory_H__

#include "OgrePrerequisites.h"
#include "OgreGuiElement.h"

namespace Ogre {

    /** Defines the interface which all components wishing to 
        supply GuiElement subclasses must implement.
    @remarks
        To allow the GuiElement types available for inclusion on 
        overlays to be extended, OGRE allows external apps or plugins
        to register their ability to create custom GuiElements with
        the GuiManager, using the addGuiElementFactory method. Classes
        wanting to do this must implement this interface.
    @par
        Each GuiElementFactory creates a single type of GuiElement, 
        identified by a 'type name' which must be unique.
    */
    class _OgreExport GuiElementFactory
    {
    public:
        /** Creates a new GuiElement instance with the name supplied. */
        virtual GuiElement* createGuiElement(const String& instanceName) = 0;
        /** Destroys a GuiElement which this factory created previously. */
        virtual void destroyGuiElement(GuiElement* pElement) { delete pElement; };
        /** Gets the string uniquely identifying the type of element this factory creates. */
        virtual const String& getTypeName(void) const = 0;
    };

}



#endif

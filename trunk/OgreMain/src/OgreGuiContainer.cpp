
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/

#include "OgreGuiContainer.h"
#include "OgreException.h"


namespace Ogre {

    //---------------------------------------------------------------------
    GuiContainer::GuiContainer(const String& name)
        : GuiElement(name)
    {
    }
    //---------------------------------------------------------------------
    GuiContainer::~GuiContainer()
    {
    }
    //---------------------------------------------------------------------
    void GuiContainer::addChild(GuiElement* elem)
    {
        String name = elem->getName();
        ChildMap::iterator i = mChildren.find(name);
        if (i != mChildren.end())
        {
            Except(Exception::ERR_DUPLICATE_ITEM, "Child with name " + name + 
                " already defined.", "GuiContainer::addChild");
        }

        mChildren.insert(ChildMap::value_type(name, elem));

        // tell child about ZOrder
        elem->_notifyZOrder(mZOrder + 1);


    }
    //---------------------------------------------------------------------
    void GuiContainer::removeChild(const String& name)
    {
        ChildMap::iterator i = mChildren.find(name);
        if (i == mChildren.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Child with name " + name + 
                " not found.", "GuiContainer::removeChild");
        }

        mChildren.erase(i);

    }
    //---------------------------------------------------------------------
    GuiElement* GuiContainer::getChild(const String& name)
    {
        ChildMap::iterator i = mChildren.find(name);
        if (i == mChildren.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Child with name " + name + 
                " not found.", "GuiContainer::getChild");
        }

        return i->second;
    }
    //---------------------------------------------------------------------
    GuiContainer::ChildIterator GuiContainer::getChildIterator(void)
    {
        return ChildIterator(mChildren.begin(), mChildren.end());
    }
    //---------------------------------------------------------------------
    void GuiContainer::addChild(GuiContainer* cont)
    {
        // Add to main map first 
        // This will pick up duplicates
        GuiElement* pElem = cont;
        addChild(pElem);

        // Now add to specific map too
        mChildContainers.insert(ChildContainerMap::value_type(cont->getName(), cont));

    }
    //---------------------------------------------------------------------
    GuiContainer::ChildContainerIterator GuiContainer::getChildContainerIterator(void)
    {
        return ChildContainerIterator(mChildContainers.begin(), mChildContainers.end());
    }
    //---------------------------------------------------------------------
    void GuiContainer::_update(void)
    {
        // call superclass
        GuiElement::_update();

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            it.getNext()->_update();
        }


    }
    //---------------------------------------------------------------------
    void GuiContainer::_notifyZOrder(ushort newZOrder)
    {
        GuiElement::_notifyZOrder(newZOrder);

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            // Give children ZOrder 1 higher than this
            it.getNext()->_notifyZOrder(newZOrder + 1);
        }

    }
    //---------------------------------------------------------------------
    void GuiContainer::_updateRenderQueue(RenderQueue* queue)
    {
        if (mVisible)
        {

            GuiElement::_updateRenderQueue(queue);

            // Also add children
            ChildIterator it = getChildIterator();
            while (it.hasMoreElements())
            {
                // Give children ZOrder 1 higher than this
                it.getNext()->_updateRenderQueue(queue);
            }
        }

    }

}


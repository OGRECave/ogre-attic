
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
#include "OgreStableHeaders.h"

#include "OgreGuiContainer.h"
#include "OgreException.h"
#include "OgreMouseEvent.h"
#include "OgreGuiManager.h"

namespace Ogre {

    //---------------------------------------------------------------------
    GuiContainer::GuiContainer(const String& name)
        : GuiElement(name),
		mChildrenProcessEvents(true)
    {
    }
    //---------------------------------------------------------------------
    GuiContainer::~GuiContainer()
    {
        GuiContainer::ChildIterator ci = getChildIterator();
        while (ci.hasMoreElements())
        {
            GuiElement* child = ci.getNext();
            child->_setParent(0);
        }
    }
    //---------------------------------------------------------------------
    void GuiContainer::addChild(GuiElement* elem)
    {
        if (elem->isContainer())
		{
			addChildImpl(static_cast<GuiContainer*>(elem));
		}
		else
		{
			addChildImpl(elem);
		}

	}
    //---------------------------------------------------------------------
    void GuiContainer::addChildImpl(GuiElement* elem)
    {
        String name = elem->getName();
        ChildMap::iterator i = mChildren.find(name);
        if (i != mChildren.end())
        {
            Except(Exception::ERR_DUPLICATE_ITEM, "Child with name " + name + 
                " already defined.", "GuiContainer::addChild");
        }

        mChildren.insert(ChildMap::value_type(name, elem));
        // tell child about parent & ZOrder
        elem->_notifyParent(this, mOverlay);
	    elem->_notifyZOrder(mZOrder + 1);
	    elem->_notifyWorldTransforms(mXForm);
	    elem->_notifyViewport();

    }
    //---------------------------------------------------------------------
    void GuiContainer::addChildImpl(GuiContainer* cont)
    {
        // Add to main map first 
        // This will pick up duplicates
        GuiElement* pElem = cont;
        addChildImpl(pElem);

        /*
        cont->_notifyParent(this, mOverlay);
        cont->_notifyZOrder(mZOrder + 1);
	    cont->_notifyWorldTransforms(mXForm);

		// tell children of new container the current overlay
        ChildIterator it = cont->getChildIterator();
        while (it.hasMoreElements())
        {
            // Give children ZOrder 1 higher than this
            GuiElement* pElemChild = it.getNext();
			pElemChild->_notifyParent(cont, mOverlay);
            pElemChild->_notifyZOrder(cont->getZOrder() + 1);
    	    pElemChild->_notifyWorldTransforms(mXForm);
        }
        */

        // Now add to specific map too
        mChildContainers.insert(ChildContainerMap::value_type(cont->getName(), cont));

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

        GuiElement* element = i->second;
        mChildren.erase(i);

            // remove from container list (if found)
        ChildContainerMap::iterator j = mChildContainers.find(name);
        if (j != mChildContainers.end())
            mChildContainers.erase(j);

        element->_setParent(0);
    }
    //---------------------------------------------------------------------
    void GuiContainer::_addChild(GuiElement* elem)
    {
        if (elem->isContainer())
		{
			addChildImpl(static_cast<GuiContainer*>(elem));
		}
		else
		{
			addChildImpl(elem);
		}
	}
    //---------------------------------------------------------------------
    void GuiContainer::_removeChild(const String& name)
    {
        ChildMap::iterator i = mChildren.find(name);
        if (i == mChildren.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Child with name " + name + 
                " not found.", "GuiContainer::removeChild");
        }

        GuiElement* element = i->second;
        mChildren.erase(i);

            // remove from container list (if found)
        ChildContainerMap::iterator j = mChildContainers.find(name);
        if (j != mChildContainers.end())
            mChildContainers.erase(j);

        element->_setParent(0);
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
    GuiContainer::ChildContainerIterator GuiContainer::getChildContainerIterator(void)
    {
        return ChildContainerIterator(mChildContainers.begin(), mChildContainers.end());
    }

    //---------------------------------------------------------------------
	void GuiContainer::_positionsOutOfDate(void)
	{
		GuiElement::_positionsOutOfDate();

        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
			it.getNext()->_positionsOutOfDate();
        }
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
    void GuiContainer::_notifyWorldTransforms(const Matrix4& xform)
    {
        GuiElement::_notifyWorldTransforms(xform);

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            it.getNext()->_notifyWorldTransforms(xform);
        }
    }
    //---------------------------------------------------------------------
    void GuiContainer::_notifyViewport()
    {
        GuiElement::_notifyViewport();

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            it.getNext()->_notifyViewport();
        }
    }
    //---------------------------------------------------------------------
    void GuiContainer::_notifyParent(GuiContainer* parent, Overlay* overlay)
    {
        GuiElement::_notifyParent(parent, overlay);

        // Update children
        ChildIterator it = getChildIterator();
        while (it.hasMoreElements())
        {
            // Notify the children of the overlay 
            it.getNext()->_notifyParent(this, overlay);
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


	GuiElement* GuiContainer::findElementAt(Real x, Real y) 		// relative to parent
	{

		GuiElement* ret = NULL;

		int currZ = -1;

		if (mVisible)
		{
			ret = GuiElement::findElementAt(x,y);	//default to the current container if no others are found
			if (ret && mChildrenProcessEvents)
			{
				ChildIterator it = getChildIterator();
				while (it.hasMoreElements())
				{
					GuiElement* currentGuiElement = it.getNext();
					if (currentGuiElement->isVisible() && currentGuiElement->isEnabled())
					{
						int z = currentGuiElement->getZOrder();
						if (z > currZ)
						{
							GuiElement* elementFound = currentGuiElement->findElementAt(x ,y );
							if (elementFound)
							{
								currZ = z;
								ret = elementFound;
							}
						}
					}
				}
			}
		}
		return ret;
	}

    void GuiContainer::copyFromTemplate(GuiElement* templateGui)
    {
        GuiElement::copyFromTemplate(templateGui);

		    if (templateGui->isContainer() && isContainer())
		    {
    	     GuiContainer::ChildIterator it = static_cast<GuiContainer*>(templateGui)->getChildIterator();
  		     while (it.hasMoreElements())
			     {
				       GuiElement* oldChildElement = it.getNext();
				       if (oldChildElement->isCloneable())
				       {
                   GuiElement* newChildElement = 
                       GuiManager::getSingleton().createGuiElement(oldChildElement->getTypeName(), mName+"/"+oldChildElement->getName());
                   oldChildElement->copyParametersTo(newChildElement);
                   addChild((GuiContainer*)newChildElement);
               }
           }
        }
    }

    GuiElement* GuiContainer::clone(const String& instanceName)
    {
        GuiContainer *newContainer;

        newContainer = static_cast<GuiContainer*>(GuiElement::clone(instanceName));

    	  ChildIterator it = getChildIterator();
  		  while (it.hasMoreElements())
			  {
				    GuiElement* oldChildElement = it.getNext();
				    if (oldChildElement->isCloneable())
				    {
                GuiElement* newChildElement = oldChildElement->clone(instanceName);
                newContainer->_addChild(newChildElement);
            }
        }

        return newContainer;
    }

}


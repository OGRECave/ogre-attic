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

#include <vector>
#include "OgreStableHeaders.h"

#include "OgreString.h"
#include "OgreGuiManager.h"
#include "OgreGuiElement.h"
#include "OgreGuiContainer.h"
#include "OgreGuiElementFactory.h"
#include "OgreException.h"
#include "OgreLogManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    template<> GuiManager* Singleton<GuiManager>::ms_Singleton = 0;
    GuiManager* GuiManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    GuiManager& GuiManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    //---------------------------------------------------------------------
    GuiManager::GuiManager()
    {
    }
    //---------------------------------------------------------------------
    GuiManager::~GuiManager()
    {
        destroyAllGuiElements(false);
        destroyAllGuiElements(true);
    }

    //---------------------------------------------------------------------
    GuiManager::ElementMap& GuiManager::getElementMap(bool isTemplate)
    {
		return (isTemplate)?mTemplates:mInstances;
	}

    //---------------------------------------------------------------------
    GuiElement* GuiManager::createGuiElementFromTemplate(const String& templateName, const String& typeName, const String& instanceName, bool isTemplate)
    {

		GuiElement* newObj  = NULL;

		if (templateName == "")
		{
			newObj = createGuiElement(typeName, instanceName, isTemplate);
		}
		else
		{
			// no template 
			GuiElement* templateGui = getGuiElement(templateName, true);
	
			String typeNameToCreate;
			if (typeName == "")
			{
				typeNameToCreate = templateGui->getTypeName();
			}
			else
			{
				typeNameToCreate = typeName;
			}

			newObj = createGuiElement(typeNameToCreate, instanceName, isTemplate);

			((GuiContainer*)newObj)->copyFromTemplate(templateGui);
		}
        
		return newObj;
	}


    //---------------------------------------------------------------------
    GuiElement* GuiManager::cloneGuiElementFromTemplate(const String& templateName, const String& instanceName)
    {
        GuiElement* templateGui = getGuiElement(templateName, true);
        return templateGui->clone(instanceName);
    }

    //---------------------------------------------------------------------
    GuiElement* GuiManager::createGuiElement(const String& typeName, const String& instanceName, bool isTemplate)
    {
		return createGuiElementImpl(typeName, instanceName, getElementMap(isTemplate));
	}

    //---------------------------------------------------------------------
    GuiElement* GuiManager::createGuiElementImpl(const String& typeName, const String& instanceName, ElementMap& elementMap)
    {
        // Check not duplicated
        ElementMap::iterator ii = elementMap.find(instanceName);
        if (ii != elementMap.end())
        {
            Except(Exception::ERR_DUPLICATE_ITEM, "GuiElement with name " + instanceName +
                " already exists.", "GuiManager::createGuiElement" );
        }
		GuiElement* newElem = createGuiElementFromFactory(typeName, instanceName);
        newElem->initialise();

        // Register
        elementMap.insert(ElementMap::value_type(instanceName, newElem));

        return newElem;


    }

    //---------------------------------------------------------------------
    GuiElement* GuiManager::createGuiElementFromFactory(const String& typeName, const String& instanceName)
    {
        // Look up factory
        FactoryMap::iterator fi = mFactories.find(typeName);
        if (fi == mFactories.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element type " + typeName,
                "GuiManager::createGuiElement");
        }

        // create
        return fi->second->createGuiElement(instanceName);
	}

    //---------------------------------------------------------------------
    GuiElement* GuiManager::getGuiElement(const String& name, bool isTemplate)
	{
		return getGuiElementImpl(name, getElementMap(isTemplate));
	}
    //---------------------------------------------------------------------
    GuiElement* GuiManager::getGuiElementImpl(const String& name, ElementMap& elementMap)
    {
        // Locate instance
        ElementMap::iterator ii = elementMap.find(name);
        if (ii == elementMap.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "GuiElement with name " + name +
                " not found.", "GuiManager::destroyGugetGuiElementiElement" );
        }

        return ii->second;
    }
    //---------------------------------------------------------------------
    void GuiManager::destroyGuiElement(const String& instanceName, bool isTemplate)
	{
		destroyGuiElementImpl(instanceName, getElementMap(isTemplate));
	}

    //---------------------------------------------------------------------
    void GuiManager::destroyGuiElement(GuiElement* pInstance, bool isTemplate)
	{
		destroyGuiElementImpl(pInstance->getName(), getElementMap(isTemplate));
	}

    //---------------------------------------------------------------------
    void GuiManager::destroyGuiElementImpl(const String& instanceName, ElementMap& elementMap)
    {
        // Locate instance
        ElementMap::iterator ii = elementMap.find(instanceName);
        if (ii == elementMap.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "GuiElement with name " + instanceName +
                " not found.", "GuiManager::destroyGuiElement" );
        }
        // Look up factory
        const String& typeName = ii->second->getTypeName();
        FactoryMap::iterator fi = mFactories.find(typeName);
        if (fi == mFactories.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element type " + typeName,
                "GuiManager::destroyGuiElement");
        }

        fi->second->destroyGuiElement(ii->second);
		elementMap.erase(ii);
    }
    //---------------------------------------------------------------------
    void GuiManager::destroyAllGuiElements(bool isTemplate)
	{
		destroyAllGuiElementsImpl(getElementMap(isTemplate));
	}
    //---------------------------------------------------------------------
    void GuiManager::destroyAllGuiElementsImpl(ElementMap& elementMap)
    {
        ElementMap::iterator i;

        while ((i = elementMap.begin()) != elementMap.end())
        {
            GuiElement* element = i->second;

            // Get factory to delete
            FactoryMap::iterator fi = mFactories.find(element->getTypeName());
            if (fi == mFactories.end())
            {
                Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element " 
                    + element->getName(),
                    "GuiManager::destroyAllGuiElements");
            }

            // remove from parent, if any
            GuiContainer* parent;
            if ((parent = element->getParent()) != 0)
            {
                parent->_removeChild(element->getName());
            }

            // children of containers will be auto-removed when container is destroyed.
            // destroy the element and remove it from the list
            fi->second->destroyGuiElement(element);
            elementMap.erase(i);
        }
    }
    //---------------------------------------------------------------------
    void GuiManager::addGuiElementFactory(GuiElementFactory* elemFactory)
    {
        // Add / replace
        mFactories[elemFactory->getTypeName()] = elemFactory;

        LogManager::getSingleton().logMessage("GuiElementFactory for type " + elemFactory->getTypeName()
            + " registered.");
    }


}

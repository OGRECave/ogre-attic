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

#include "OgreStableHeaders.h"

#include "OgreString.h"
#include "OgreGuiManager.h"
#include "OgreOverlayElement.h"
#include "OgreOverlayContainer.h"
#include "OgreOverlayElementFactory.h"
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
        destroyAllOverlayElements(false);
        destroyAllOverlayElements(true);
    }

    //---------------------------------------------------------------------
    GuiManager::ElementMap& GuiManager::getElementMap(bool isTemplate)
    {
		return (isTemplate)?mTemplates:mInstances;
	}

    //---------------------------------------------------------------------
    OverlayElement* GuiManager::createOverlayElementFromTemplate(const String& templateName, const String& typeName, const String& instanceName, bool isTemplate)
    {

		OverlayElement* newObj  = NULL;

		if (templateName == "")
		{
			newObj = createOverlayElement(typeName, instanceName, isTemplate);
		}
		else
		{
			// no template 
			OverlayElement* templateGui = getOverlayElement(templateName, true);
	
			String typeNameToCreate;
			if (typeName == "")
			{
				typeNameToCreate = templateGui->getTypeName();
			}
			else
			{
				typeNameToCreate = typeName;
			}

			newObj = createOverlayElement(typeNameToCreate, instanceName, isTemplate);

			((OverlayContainer*)newObj)->copyFromTemplate(templateGui);
		}
        
		return newObj;
	}


    //---------------------------------------------------------------------
    OverlayElement* GuiManager::cloneOverlayElementFromTemplate(const String& templateName, const String& instanceName)
    {
        OverlayElement* templateGui = getOverlayElement(templateName, true);
        return templateGui->clone(instanceName);
    }

    //---------------------------------------------------------------------
    OverlayElement* GuiManager::createOverlayElement(const String& typeName, const String& instanceName, bool isTemplate)
    {
		return createOverlayElementImpl(typeName, instanceName, getElementMap(isTemplate));
	}

    //---------------------------------------------------------------------
    OverlayElement* GuiManager::createOverlayElementImpl(const String& typeName, const String& instanceName, ElementMap& elementMap)
    {
        // Check not duplicated
        ElementMap::iterator ii = elementMap.find(instanceName);
        if (ii != elementMap.end())
        {
            Except(Exception::ERR_DUPLICATE_ITEM, "OverlayElement with name " + instanceName +
                " already exists.", "GuiManager::createOverlayElement" );
        }
		OverlayElement* newElem = createOverlayElementFromFactory(typeName, instanceName);
        newElem->initialise();

        // Register
        elementMap.insert(ElementMap::value_type(instanceName, newElem));

        return newElem;


    }

    //---------------------------------------------------------------------
    OverlayElement* GuiManager::createOverlayElementFromFactory(const String& typeName, const String& instanceName)
    {
        // Look up factory
        FactoryMap::iterator fi = mFactories.find(typeName);
        if (fi == mFactories.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element type " + typeName,
                "GuiManager::createOverlayElement");
        }

        // create
        return fi->second->createOverlayElement(instanceName);
	}

    //---------------------------------------------------------------------
    OverlayElement* GuiManager::getOverlayElement(const String& name, bool isTemplate)
	{
		return getOverlayElementImpl(name, getElementMap(isTemplate));
	}
    //---------------------------------------------------------------------
    OverlayElement* GuiManager::getOverlayElementImpl(const String& name, ElementMap& elementMap)
    {
        // Locate instance
        ElementMap::iterator ii = elementMap.find(name);
        if (ii == elementMap.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "OverlayElement with name " + name +
                " not found.", "GuiManager::getOverlayElementImpl" );
        }

        return ii->second;
    }
    //---------------------------------------------------------------------
    void GuiManager::destroyOverlayElement(const String& instanceName, bool isTemplate)
	{
		destroyOverlayElementImpl(instanceName, getElementMap(isTemplate));
	}

    //---------------------------------------------------------------------
    void GuiManager::destroyOverlayElement(OverlayElement* pInstance, bool isTemplate)
	{
		destroyOverlayElementImpl(pInstance->getName(), getElementMap(isTemplate));
	}

    //---------------------------------------------------------------------
    void GuiManager::destroyOverlayElementImpl(const String& instanceName, ElementMap& elementMap)
    {
        // Locate instance
        ElementMap::iterator ii = elementMap.find(instanceName);
        if (ii == elementMap.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "OverlayElement with name " + instanceName +
                " not found.", "GuiManager::destroyOverlayElement" );
        }
        // Look up factory
        const String& typeName = ii->second->getTypeName();
        FactoryMap::iterator fi = mFactories.find(typeName);
        if (fi == mFactories.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element type " + typeName,
                "GuiManager::destroyOverlayElement");
        }

        fi->second->destroyOverlayElement(ii->second);
		elementMap.erase(ii);
    }
    //---------------------------------------------------------------------
    void GuiManager::destroyAllOverlayElements(bool isTemplate)
	{
		destroyAllOverlayElementsImpl(getElementMap(isTemplate));
	}
    //---------------------------------------------------------------------
    void GuiManager::destroyAllOverlayElementsImpl(ElementMap& elementMap)
    {
        ElementMap::iterator i;

        while ((i = elementMap.begin()) != elementMap.end())
        {
            OverlayElement* element = i->second;

            // Get factory to delete
            FactoryMap::iterator fi = mFactories.find(element->getTypeName());
            if (fi == mFactories.end())
            {
                Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element " 
                    + element->getName(),
                    "GuiManager::destroyAllOverlayElements");
            }

            // remove from parent, if any
            OverlayContainer* parent;
            if ((parent = element->getParent()) != 0)
            {
                parent->_removeChild(element->getName());
            }

            // children of containers will be auto-removed when container is destroyed.
            // destroy the element and remove it from the list
            fi->second->destroyOverlayElement(element);
            elementMap.erase(i);
        }
    }
    //---------------------------------------------------------------------
    void GuiManager::addOverlayElementFactory(OverlayElementFactory* elemFactory)
    {
        // Add / replace
        mFactories[elemFactory->getTypeName()] = elemFactory;

        LogManager::getSingleton().logMessage("OverlayElementFactory for type " + elemFactory->getTypeName()
            + " registered.");
    }


}

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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

#include "OgreString.h"
#include "OgreGuiManager.h"
#include "OgreGuiElement.h"
#include "OgreGuiElementFactory.h"
#include "OgreException.h"
#include "OgreLogManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    template<> GuiManager* Singleton<GuiManager>::ms_Singleton = 0;
    //---------------------------------------------------------------------
    GuiElement* GuiManager::createGuiElement(const String& typeName, const String& instanceName)
    {
        // Check not duplicated
        InstanceMap::iterator ii = mInstances.find(instanceName);
        if (ii != mInstances.end())
        {
            Except(Exception::ERR_DUPLICATE_ITEM, "GuiElement with name " + instanceName +
                " already exists.", "GuiManager::createGuiElement" );
        }
        // Look up factory
        FactoryMap::iterator fi = mFactories.find(typeName);
        if (fi == mFactories.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element type " + typeName,
                "GuiManager::createGuiElement");
        }

        // create
        GuiElement* newElem = fi->second->createGuiElement(instanceName);

        // Register
        mInstances.insert(InstanceMap::value_type(instanceName, newElem));

        return newElem;


    }
    //---------------------------------------------------------------------
    GuiElement* GuiManager::getGuiElement(const String& name)
    {
        // Locate instance
        InstanceMap::iterator ii = mInstances.find(name);
        if (ii == mInstances.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "GuiElement with name " + name +
                " not found.", "GuiManager::destroyGugetGuiElementiElement" );
        }

        return ii->second;
    }
    //---------------------------------------------------------------------
    void GuiManager::destroyGuiElement(const String& instanceName)
    {
        // Locate instance
        InstanceMap::iterator ii = mInstances.find(instanceName);
        if (ii == mInstances.end())
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

    }
    //---------------------------------------------------------------------
    void GuiManager::destroyGuiElement(GuiElement* pInstance)
    {
        destroyGuiElement(pInstance->getName());
    }
    //---------------------------------------------------------------------
    void GuiManager::destroyAllGuiElements(void)
    {
        InstanceMap::iterator i = mInstances.begin();
        InstanceMap::iterator iend = mInstances.end();
        while (i++ != iend)
        {
            // Get factory to delete
            FactoryMap::iterator fi = mFactories.find(i->second->getTypeName());
            if (fi == mFactories.end())
            {
                Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot locate factory for element " 
                    + i->second->getName(),
                    "GuiManager::destroyAllGuiElements");
            }

            // Destroy
            fi->second->destroyGuiElement(i->second);
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
    //---------------------------------------------------------------------
    GuiManager& GuiManager::getSingleton(void)
    {
        return Singleton<GuiManager>::getSingleton();
    }
}

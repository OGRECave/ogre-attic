/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/

#ifndef __GuiManager_H__
#define __GuiManager_H__

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"

namespace Ogre {

    /** This class acts as a repository and regitrar of overlay components.
    @remarks
        GuiManager's job is to manage the lifecycle of GuiElement (subclass)
        instances, and also to register plugin suppliers of new components.
    */
    class _OgreExport GuiManager : public Singleton<GuiManager>
    {
    public:
        /** Creates a new GuiElement of the type requested.
        @remarks
            The type of element to create is passed in as a string because this
            allows plugins to register new types of component.
        @param typeName The type of element to create.
        @param instanceName The name to give the new instance.
        */
        GuiElement* createGuiElement(const String& typeName, const String& instanceName);

        /** Gets a reference to an existing element. */
        GuiElement* getGuiElement(const String& name);
        
        /** Destroys a GuiElement. 
        @remarks
            Make sure you're not still using this in an Overlay. If in
            doubt, let OGRE destroy elements on shutdown.
        */
        void destroyGuiElement(const String& instanceName);

        /** Destroys a GuiElement. 
        @remarks
            Make sure you're not still using this in an Overlay. If in
            doubt, let OGRE destroy elements on shutdown.
        */
        void destroyGuiElement(GuiElement* pInstance);

        /** Destroys all the GuiElements created so far.
        @remarks
            Best to leave this to the engine to call internally, there
            should rarely be a need to call it yourself.
        */
        void destroyAllGuiElements(void);
      
        /** Registers a new GuiElementFactory with this manager.
        @remarks
            Should be used by plugins or other apps wishing to provide
            a new GuiElement subclass.
        */
        void addGuiElementFactory(GuiElementFactory* elemFactory);

        /** Override standard Singleton retrieval.
            @remarks
                Why do we do this? Well, it's because the Singleton implementation is in a .h file,
                which means it gets compiled into anybody who includes it. This is needed for the Singleton
                template to work, but we actually only want it compiled into the implementation of the
                class based on the Singleton, not all of them. If we don't change this, we get link errors
                when trying to use the Singleton-based class from an outside dll.
            @par
                This method just delegates to the template version anyway, but the implementation stays in this
                single compilation unit, preventing link errors.
        */
        static GuiManager& getSingleton(void);

    protected:
        typedef std::map<String, GuiElementFactory*> FactoryMap;
        FactoryMap mFactories;

        typedef std::map<String, GuiElement*> InstanceMap;
        InstanceMap mInstances;


    };



}




#endif

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

#ifndef __GuiManager_H__
#define __GuiManager_H__

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"
#include "OgreIteratorWrappers.h"

namespace Ogre {

    /** This class acts as a repository and regitrar of overlay components.
    @remarks
        GuiManager's job is to manage the lifecycle of OverlayElement (subclass)
        instances, and also to register plugin suppliers of new components.
    */
    class _OgreExport GuiManager : public Singleton<GuiManager>
    {
    public:
        GuiManager();
        virtual ~GuiManager();
        /** Creates a new OverlayElement of the type requested.
        @remarks
            The type of element to create is passed in as a string because this
            allows plugins to register new types of component.
        @param typeName The type of element to create.
        @param instanceName The name to give the new instance.
        */
        OverlayElement* createOverlayElement(const String& typeName, const String& instanceName, bool isTemplate = false);

        /** Gets a reference to an existing element. */
        OverlayElement* getOverlayElement(const String& name, bool isTemplate = false);
        
        /** Destroys a OverlayElement. 
        @remarks
            Make sure you're not still using this in an Overlay. If in
            doubt, let OGRE destroy elements on shutdown.
        */
        void destroyOverlayElement(const String& instanceName, bool isTemplate = false);

        /** Destroys a OverlayElement. 
        @remarks
            Make sure you're not still using this in an Overlay. If in
            doubt, let OGRE destroy elements on shutdown.
        */
        void destroyOverlayElement(OverlayElement* pInstance, bool isTemplate = false);

        /** Destroys all the OverlayElement  created so far.
        @remarks
            Best to leave this to the engine to call internally, there
            should rarely be a need to call it yourself.
        */
        void destroyAllOverlayElements(bool isTemplate = false);




        /** Registers a new OverlayElementFactory with this manager.
        @remarks
            Should be used by plugins or other apps wishing to provide
            a new OverlayElement subclass.
        */
        void addOverlayElementFactory(OverlayElementFactory* elemFactory);

        OverlayElement* createOverlayElementFromTemplate(const String& templateName, const String& typeName, const String& instanceName, bool isTemplate = false);
        /**
         *  @remarks
         *  Creates a new OverlayElement object from the specified template name.  The new
         *  object's name, and all of it's children, will be instanceName/orignalName.
         */
        OverlayElement* cloneOverlayElementFromTemplate(const String& templateName, const String& instanceName);

        OverlayElement* createOverlayElementFromFactory(const String& typeName, const String& instanceName);

        typedef std::map<String, OverlayElement*> ElementMap;
        typedef MapIterator<ElementMap> TemplateIterator;
        /** Returns an iterator over all templates in this manager.*/
        TemplateIterator getTemplateIterator ()
        {
          return TemplateIterator (mTemplates.begin (), mTemplates.end ()) ;
        }
        /* Returns whether the Element with the given name is a Template */
        bool isTemplate (String strName) const {
          return (mTemplates.find (strName) != mTemplates.end()) ;
        }

        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static GuiManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static GuiManager* getSingletonPtr(void);
	protected:
        typedef std::map<String, OverlayElementFactory*> FactoryMap;
        FactoryMap mFactories;

        ElementMap mInstances;
        ElementMap mTemplates;



	    ElementMap& getElementMap(bool isTemplate);

        OverlayElement* createOverlayElementImpl(const String& typeName, const String& instanceName, ElementMap& elementMap);

        OverlayElement* getOverlayElementImpl(const String& name, ElementMap& elementMap);
        
        void destroyOverlayElementImpl(const String& instanceName, ElementMap& elementMap);

        void destroyOverlayElementImpl(OverlayElement* pInstance, ElementMap& elementMap);

        void destroyAllOverlayElementsImpl(ElementMap& elementMap);

    };



}




#endif

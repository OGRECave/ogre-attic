/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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
#ifndef _ResourceGroupManager_H__
#define _ResourceGroupManager_H__

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"

namespace Ogre {

    /** This abstract class defines an interface which is called back during
        resource group loading to indicate the progress of the load. 
    */
    class _OgreExport ResourceGroupListener
    {
    public:
        /** This event is fired  when a resource group begins loading.
        @param groupName The name of the group being loaded
        @param stageCount The number of progress stages which will be 
            raised as events in total.
        */
        virtual resourceGroupStarted(const String& groupName, size_t stageCount) = 0;
        /** This event is fired when a resource begins loading. 
        @param resourceName The name of the resource being loaded.
        @param stageCount The number of progress stages within this resource
        */
        virtual resourceStarted(const String& resourceName, size_t stageCount) = 0;
        /** This event is fired as a progress point along the loading of a resource. 
            How many of these are raised depends on the resource (and the stageCount
            that was passed as part of the last resourceStarted call), but it will
            always be at least one.
        */
        virtual resourceProgress(void) = 0;
        /** This event is fired when a resource finishes loading. */
        virtual resourceEnded(const String& resourceName) = 0;
        /** This event is fired when a resource group finished loading. */
        virtual resourceGroupEnded(const String& groupName) = 0;
    };
    /** This singleton class manages the list of resource groups, and notifying
        the various resource managers of their obligations to load / unload
        resources in a group. It also provides facilities to monitor resource
        loading per group (to do progress bars etc), provided the resources 
        that are required are pre-registered.
    @par
        Defining new resource groups,  and declaring the resources you intend to
        use in advance is entirely optional. If you don't declare a resource
        before you try to load it (or something else does based on another trigger), 
        the resource will still get loaded - the difference is that you will 
        not get ResourceGroupListener notifications of this, nor will you be 
        able to unload it as a group (except as part of the General group, which
        everything undefined becomes part of).
    */
    class _OgreExport ResourceGroupManager : public Singleton<ResourceGroupManager>
    {
    public:
        /// Map of resource types (strings) to ResourceManagers, used to notify them to load / unload group contents
        typedef std::map<String, ResourceManager*> ResourceManagerMap;
    protected:
        ResourceManagerMap mResourceManagerMap;

        typedef std::vector<ResourceGroupListener*> ResourceGroupListenerList;
        ResourceGroupListenerList mResourceGroupListenerList;

        /// List of possible file locations
        typedef std::list<ArchiveEx*> LocationList;
        /// Map from resource group to locations
        typedef std::map<String, LocationList> ResourceGroupLocationMap;
        ResourceGroupLocationMap mResourceGroupLocationMap;

        /// Nested struct defining a resource declaration
        struct ResourceDeclaration
        {
            String resourceName;
            String resourceType;
        };
        /// List of resource declarations
        typedef std::list<ResourceDeclaration> ResourceDeclarationList;
        // Group resource declarations by loading priority of the type (defined by ResourceManager)
        // (e.g. skeletons and materials before meshes)
        typedef std::map<Real, ResourceDeclarationList> ResourceDeclarationPriorityMap;
        ResourceDeclarationPriorityMap mResourceDeclarationPriorityMap;




    public:
        ResourceGroupManager();
        virtual ~ResourceGroupManager();

        /** Create a resource group.
        @remarks
            A resource group allows you to define a set of resources that can 
            be loaded / unloaded as a unit. For example, it might be all the 
            resources used for the level of a game. There is always one predefined
            resource group called "General", which is typically used to hold all
            resources which do not need to be unloaded until shutdown. You can 
            create additional ones so that you can control the life of your
            resources in whichever way you wish.
        @par
            Once you have defined a resource group, it is advisable to declare
            the resources which you intend to use in a group. That way, you can 
            load them in one call (@see ResourceGroupManager::loadResourceGroup,
            and receive progress callbacks during loading
            (@see ResourceGroupManager::addResourceGroupListener). 
            Resources which are loaded in other ways are not subject to 
            progress monitoring.
        @param name The name to give the resource group.
        */
        void createResourceGroup(const String& name);

        /** Loads a resource group.
        @remarks
            This method loads all the resources that have been declared as
            being part of the named resource group. When this method is called, 
            this class will count up the resources being loaded so that it
            can present a running progress of the load to any listeners.
        @param name The name to of the resource group to load.
        */
        void loadResourceGroup(const String& name);

        /** Unloads a resource group.
        @remarks
            This method unloads all the resources that have been declared as
            being part of the named resource group. Note that these resources
            will still exist in their respective ResourceManager classes, but
            will be in an unloaded state. If you want to remove them entirely,
            you should use destroyResourceGroup.
        @param name The name to of the resource group to unload.
        */
        void unloadResourceGroup(const String& name);

        
        /** Destroys a resource group, unloading it first, destroying the resources
            which are part of it, and then removing it from
            the list of resource groups. 
        @param name The name of the resource group to destroy.
        */
        void destroyResourceGroup(const String& name);


        /** Method to add a resource location to for a given resource group. 
        @remarks
            Resource locations are places which are searched to load resource files.
            When you choose to load a file, or to search for valid files to load, 
            the resource locations are used.
        @param name The name of the resource location; probably a directory, zip file, URL etc.
        @param locType The codename for the resource type, which must correspond to the 
            ArchiveEx factory which is providing the implementation.
        @param resGroup The name of the resource group for which this location is
            to apply. "General" is the default group which always exists, and can
            be used for resources which are unlikely to be unloaded until application
            shutdown. Otherwise it must be the name of a group which has already 
            been created with createResourceGroup.
        @param recursive Whether subdirectories will be searched for files which 
            are loaded with no specific path. Note that this can slow down file 
            location speeds if enabled.
        */
        void addResourceLocation(const String& name, const String& locType, 
            const String& resGroup = "General", bool recursive = false);
        /** Removes a resource location from the search path. */ 
        void removeResourceLocation(const String& name, const String& resGroup = "General");

        /** Declares a resource to be a part of a resource group, allowing you 
            to load and unload it as part of the group.
        @remarks
            By declaring resources before you attempt to use them, you can 
            more easily control the loading and unloading of those resources
            by their group. Declaring them also allows them to be enumerated, 
            which means events can be raised to indicate the loading progress
            (@see ResourceGroupListener).
        @param name The resource name. Note that this can either be a single
            resource name, or it can contain a wildcard character, indicating
            all files matching a pattern. The wildcard character is '*'.
        @param resourceType The type of the resource. Ogre comes preconfigured with 
            a number of resource types: 
            <ul>
            <li>Font</li>
            <li>Material</li>
            <li>Mesh</li>
            <li>Overlay</li>
            <li>Skeleton</li>
            <li>WorldGeometry</li>
            </ul>
            .. but more can be added by plugin ResourceManager classes.
        @param groupName The name of the group to which it will belong.
        */
        void declareResource(const String& name, const String& resourceType, 
            const String& groupName = "General");
        /** Undeclare a resource, note that this will not cause it to be unloaded
            if it is already loaded, only unloadResourceGroup will do that. 
        @param name The name of the resource, or wildcarded resource pattern. 
        */
        void undeclareResource(const String& name);

		/** Find a single resource by name and return a DataStream
		 	pointing at the source of the data.
		@param resourceName The name of the resource to locate
		@param groupName The name of the resource group; this determines which 
			locations are searched. 
		@returns Shared pointer to data stream containing the data, will be
			destroyed automatically when no longer referenced
		*/
		DataStreamPtr void _findResource(const String& resourceName, 
			const String& groupName = "General");

		/** Find all resources matching a given pattern (which can contain
			the character '*' as a wildcard, and return a collection of 
			DataStream objects on them.
		@param pattern The pattern to look for
		@param groupName The resource group; this determines which locations
			are searched.
		@returns Shared pointer to a data stream list , will be
			destroyed automatically when no longer referenced
		*/
		DataStreamListPtr void _findResources(const String& pattern, 
			const String& groupName = "General");
		
		/** Adds a ResourceGroupListener which will be called back during 
            resource loading events. 
        */
        void addResourceGroupListener(ResourceGroupListener* l);
        /** Removes a ResourceGroupListener */
        void removeResourceGroupListener(ResourceGroupListener* l);

        /** Internal method for registering a ResourceManager (which should be
            a singleton). Creators of plugins can register new ResourceManagers
            this way if they wish.
        @param resourceType String identifying the resource type, must be unique.
        @param rm Pointer to the ResourceManager instance.
        */
        void _registerResourceManager(const String& resourceType, ResourceManager* rm);

        /** Internal method for unregistering a ResourceManager.
        @param resourceType String identifying the resource type.
        */
        void _unregisterResourceManager(const String& resourceType);



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
        static ResourceGroupManager& getSingleton(void);
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
        static ResourceGroupManager* getSingletonPtr(void);

    };
}

#endif

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
#include "OgreCommon.h"
#include "OgreDataStream.h"

namespace Ogre {

    /** This abstract class defines an interface which is called back during
        resource group loading to indicate the progress of the load. 
	@remarks
		Resource group loading is in 2 phases - creating resources from 
		declarations (which includes parsing scripts), and loading
		resources. Note that you don't necessarily have to have both; it
		is quite possible to just parse all the scripts for a group (see
		ResourceGroupManager::initialiseResourceGroup, but not to 
		load the resource group. 
		The sequence of events is (* signifies a repeating item):
		<ul>
		<li>resourceGroupScriptingStarted</li>
		<li>resourceScript (*)</li>
		<li>resourceGroupScriptingEnded</li>
		<li>resourceGroupLoadStarted</li>
		<li>resourceLoad (*)</li>
		<li>resourceGroupLoadEnded</li>
		</ul>
    */
    class _OgreExport ResourceGroupListener
    {
    public:
		/** This event is fired when a resource group begins parsing scripts.
		@param groupName The name of the group 
		@param scriptCount The number of scripts which will be parsed
		*/
		virtual void resourceGroupScriptingStarted(const String& groupName, size_t scriptCount) = 0;
		/** This event is fired when a script has been fully parsed.
		@param scriptName Name of the script being parsed
		*/
		virtual void resourceScript(const String& scriptName) = 0;
		/** This event is fired when a resource group finished parsing scripts. */
		virtual void resourceGroupScriptingEnded(const String& groupName) = 0;

		/** This event is fired  when a resource group begins loading.
		@param groupName The name of the group being loaded
		@param resourceCount The number of resources which will be loaded
		*/
		virtual void resourceGroupLoadStarted(const String& groupName, size_t resourceCount) = 0;
		/** This event is fired when a resource is finished loading. 
		@param resource Weak reference to the resource loaded.
		*/
		virtual void resourceLoad(const ResourcePtr& resource) = 0;
		/** This event is fired when a resource group finished loading. */
		virtual void resourceGroupLoadEnded(const String& groupName) = 0;

    };
    /** This singleton class manages the list of resource groups, and notifying
        the various resource managers of their obligations to load / unload
        resources in a group. It also provides facilities to monitor resource
        loading per group (to do progress bars etc), provided the resources 
        that are required are pre-registered.
    @par
        Defining new resource groups,  and declaring the resources you intend to
        use in advance is optional, however it is a very useful feature. In addition, 
		if a ResourceManager supports the definition of resources through scripts, 
		then this is the class which drives the locating of the scripts and telling
		the ResourceManager to parse them. 
	@par
		There are several states that a resource can be in (the concept, not the
		object instance in this case):
		<ol>
		<li><b>Undefined</b>. Nobody knows about this resource yet. It might be
		in the filesystem, but Ogre is oblivious to it at the moment - there 
		is no Resource instance. This might be because it's never been declared
		(either in a script, or using ResourceGroupManager::declareResource), or
		it may have previously been a valid Resource instance but has been 
		removed, either individually through ResourceManager::remove or as a group
		through ResourceGroupManager::clearResourceGroup.</li>
		<li><b>Declared</b>. Ogre has some forewarning of this resource, either
		through calling ResourceGroupManager::declareResource, or by declaring
		the resource in a script file which is on one of the resource locations
		which has been defined for a group. There is still no instance of Resource,
		but Ogre will know to create this resource when 
		ResourceGroupManager::initialiseResourceGroup is called.</li>
		<li><b>Unloaded</b>. There is now a Resource instance for this resource, 
		although it is not loaded. This means that code which looks for this
		named resource will find it, but the Resource is not using a lot of memory
		because it is in an unloaded state. A Resource can get into this state
		by having just been created by ResourceGroupManager::initialiseResourceGroup 
		(either from a script, or from a call to declareResource), by 
		being created directly from code (ResourceManager::create), or it may 
		have previously been loaded and has been unloaded, either individually
		through Resource::unload, or as a group through ResourceGroupManager::unloadResourceGroup.</li>
		<li><b>Loaded</b>The Resource instance is fully loaded. This may have
		happened implicitly because something used it, or it may have been 
		loaded as part of a group.</li>
		</ol>
		@see ResourceGroupManager::declareResource
		@see ResourceGroupManager::initialiseResourceGroup
		@see ResourceGroupManager::loadResourceGroup
		@see ResourceGroupManager::unloadResourceGroup
		@see ResourceGroupManager::clearResourceGroup
    */
    class _OgreExport ResourceGroupManager : public Singleton<ResourceGroupManager>
    {
    public:
		/// Default resource group name
		static String DEFAULT_RESOURCE_GROUP_NAME;
        /// Default resource group used for world geometry resources
        static String WORLD_RESOURCE_GROUP_NAME;
        /// Nested struct defining a resource declaration
        struct ResourceDeclaration
        {
            String resourceName;
            String resourceType;
			NameValuePairList parameters;
        };
        /// List of resource declarations
        typedef std::list<ResourceDeclaration> ResourceDeclarationList;
    protected:
		/// Map of resource types (strings) to ResourceManagers, used to notify them to load / unload group contents
		typedef std::map<String, ResourceManager*> ResourceManagerMap;
        ResourceManagerMap mResourceManagerMap;

		/// Map of resource loading order (Real) to ResourceManagers, used to order general tasks e.g. script parsing
		typedef std::multimap<Real, ResourceManager*> ResourceManagerOrderMap;
		ResourceManagerOrderMap mResourceManagerOrderMap;

		typedef std::vector<ResourceGroupListener*> ResourceGroupListenerList;
        ResourceGroupListenerList mResourceGroupListenerList;

		struct ResourceIndexEntry
		{
			/// Full name if recursive was used
			String fullname;
			/// Archive where file was found
			Archive* archive;

		};
        /// Resource index entry, resourcename->location (also maps basename if recursive)
        typedef std::map<String, ResourceIndexEntry> ResourceLocationIndex;

		/// Resource location entry
		struct ResourceLocation
		{
			/// Pointer to the archive which is the destination
			Archive* archive;
			/// Whether this location was added recursively
			bool recursive;
		};
		/// List of possible file locations
		typedef std::list<ResourceLocation*> LocationList;
		/// List of resources which can be loaded / unloaded
		typedef std::list<ResourcePtr> LoadUnloadResourceList;
		/// Resource group entry
		struct ResourceGroup
		{
			/// Group name
			String name;
			/// List of possible locations to search
			LocationList locationList;
			/// Index of resource names to locations, built for speedy access
			ResourceLocationIndex resourceIndex;
			/// Pre-declared resources, ready to be created
			ResourceDeclarationList resourceDeclarations;
			/// Created resources which are ready to be loaded / unloaded
			// Group by loading order of the type (defined by ResourceManager)
			// (e.g. skeletons and materials before meshes)
			typedef std::map<Real, LoadUnloadResourceList*> LoadResourceOrderMap;
			LoadResourceOrderMap loadResourceOrderMap;
		};
        /// Map from resource group names to groups
        typedef std::map<String, ResourceGroup*> ResourceGroupMap;
        ResourceGroupMap mResourceGroupMap;

		/** Parses all the available scripts found in the resource locations
		for the given group, for all ResourceManagers.
		@remarks
			Called as part of initialiseResourceGroup
		*/
		void parseResourceGroupScripts(const String& groupName);
		/** Create all the pre-declared resources.
		@remarks
			Called as part of initialiseResourceGroup
		*/
		void createDeclaredResources(const String& name);
		/** Adds a created resource to a group. */
		void addCreatedResource(ResourcePtr& res, ResourceGroup& group);
		/** Get resource group */
		ResourceGroup* getResourceGroup(const String& name);
		/** Drops contents of a group, leave group there, notify ResourceManagers. */
		void dropGroupContents(ResourceGroup* grp);
		/** Delete a group for shutdown - don't notify ResourceManagers. */
		void deleteGroup(ResourceGroup* grp);
		/// Internal event firing method
		void fireResourceGroupScriptingStarted(const String& groupName, size_t scriptCount);
		/// Internal event firing method
		void fireResourceScript(const String& scriptName);
		/// Internal event firing method
		void fireResourceGroupScriptingEnded(const String& groupName);
		/// Internal event firing method
		void fireResourceGroupLoadStarted(const String& groupName, size_t resourceCount);
		/// Internal event firing method
		void fireResourceLoad(const ResourcePtr& resource);
		/// Internal event firing method
		void fireResourceGroupLoadEnded(const String& groupName);



		/// Stored current group - optimisation for when bulk loading a group
		ResourceGroup* mCurrentGroup;

    public:
        ResourceGroupManager();
        virtual ~ResourceGroupManager();

        /** Create a resource group.
        @remarks
            A resource group allows you to define a set of resources that can 
            be loaded / unloaded as a unit. For example, it might be all the 
            resources used for the level of a game. There is always one predefined
            resource group called ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
			which is typically used to hold all resources which do not need to 
			be unloaded until shutdown. You can create additional ones so that 
			you can control the life of your resources in whichever way you wish.
        @par
            Once you have defined a resource group, resources which will be loaded
			as part of it are defined in one of 3 ways:
			<ol>
			<li>Manually through declareResource(); this is useful for scripted
				declarations since it is entirely generalised, and does not 
				create Resource instances right away</li>
			<li>Through the use of scripts; some ResourceManager subtypes have
				script formats (e.g. .material, .overlay) which can be used
				to declare resources</li>
			<li>By calling ResourceManager::create to create a resource manually.
			This resource will go on the list for it's group and will be loaded
			and unloaded with that group</li>
			</ol>
			You must remember to call initialiseResourceGroup if you intend to use
			the first 2 types.
        @param name The name to give the resource group.
        */
        void createResourceGroup(const String& name);


        /** Initialises a resource group.
		@remarks
			After creating a resource group, adding some resource locations, and
			perhaps pre-declaring some resources using declareResource(), but 
			before you need to use the resources in the group, you 
			should call this method to initialise the group. By calling this,
			you are triggering the following processes:
			<ol>
			<li>Scripts for all resource types which support scripting are
				parsed from the resource locations, and resources within them are
				created (but not loaded yet).</li>
			<li>Creates all the resources which have just pre-declared using
			declareResource (again, these are not loaded yet)</li>
			</ol>
			So what this essentially does is create a bunch of unloaded Resource entries
			in the respective ResourceManagers based on scripts, and resources
			you've pre-declared. That means that code looking for these resources
			will find them, but they won't be taking up much memory yet, until
			they are either used, or they are loaded in bulk using loadResourceGroup.
			Loading the resource group in bulk is entirely optional, but has the 
			advantage of coming with progress reporting as resources are loaded.
		@par
			Failure to call this method means that loadResourceGroup will do 
			nothing, and any resources you define in scripts will not be found.
			Similarly, once you have called this method you won't be able to
			pick up any new scripts or pre-declared resources, unless you
			call clearResourceGroup, set up declared resources, and call this
			method again.
		@param name The name of the resource group to initialise
		*/
		void initialiseResourceGroup(const String& name);

		/** Loads a resource group.
        @remarks
			Loads any created resources which are part of the named group.
			Note that resources must have already been created by calling
			ResourceManager::create, or declared using declareResource() or
			in a script (such as .material and .overlay). The latter requires
			that initialiseResourceGroup has been called. 

			When this method is called, this class will callback any ResourceGroupListeners
			which have been registered to update them on progress. 
        @param name The name to of the resource group to load.
        */
        void loadResourceGroup(const String& name);

        /** Unloads a resource group.
        @remarks
            This method unloads all the resources that have been declared as
            being part of the named resource group. Note that these resources
            will still exist in their respective ResourceManager classes, but
            will be in an unloaded state. If you want to remove them entirely,
            you should use clearResourceGroup or destroyResourceGroup.
        @param name The name to of the resource group to unload.
        */
        void unloadResourceGroup(const String& name);

		/** Clears a resource group. 
		@remarks
			This method unloads all resources in the group, but in addition it
			removes all those resources from their ResourceManagers, and then 
			clears all the members from the list. That means after calling this
			method, there are no resources declared as part of the named group
			any more. Resource locations still persist though.
        @param name The name to of the resource group to clear.
		*/
		void clearResourceGroup(const String& name);
        
        /** Destroys a resource group, clearing it first, destroying the resources
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
            Archive factory which is providing the implementation.
        @param resGroup The name of the resource group for which this location is
            to apply. ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME is the 
			default group which always exists, and can
            be used for resources which are unlikely to be unloaded until application
            shutdown. Otherwise it must be the name of a group which has already 
            been created with createResourceGroup.
        @param recursive Whether subdirectories will be searched for files which 
            are loaded with no specific path. Note that this can slow down file 
            location speeds if enabled.
        */
        void addResourceLocation(const String& name, const String& locType, 
            const String& resGroup = DEFAULT_RESOURCE_GROUP_NAME, bool recursive = false);
        /** Removes a resource location from the search path. */ 
        void removeResourceLocation(const String& name, 
			const String& resGroup = DEFAULT_RESOURCE_GROUP_NAME);

        /** Declares a resource to be a part of a resource group, allowing you 
            to load and unload it as part of the group.
        @remarks
            By declaring resources before you attempt to use them, you can 
            more easily control the loading and unloading of those resources
            by their group. Declaring them also allows them to be enumerated, 
            which means events can be raised to indicate the loading progress
            (@see ResourceGroupListener). Note that another way of declaring
			resources is to use a script specific to the resource type, if
			available (e.g. .material).
		@par
			Declared resources are not created as Resource instances (and thus
			are not available through their ResourceManager) until initialiseResourceGroup
			is called, at which point all declared resources will become created 
			(but unloaded) Resource instances, along with any resources declared
			in scripts in resource locations associated with the group.
        @param name The resource name. 
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
		@param loadParameters A list of name / value pairs which supply custom
			parameters to the resource which will be required before it can 
			be loaded. These are specific to the resource type.
        */
        void declareResource(const String& name, const String& resourceType,
            const String& groupName = DEFAULT_RESOURCE_GROUP_NAME,
			const NameValuePairList& loadParameters = NameValuePairList());
        /** Undeclare a resource.
		@remarks
			Note that this will not cause it to be unloaded
            if it is already loaded, nor will it destroy a resource which has 
			already been created if initialiseResourceGroup has been called already.
			Only unloadResourceGroup / clearResourceGroup / destroyResourceGroup 
			will do that. 
        @param name The name of the resource. 
		@param groupName The name of the group this resource was declared in. 
        */
        void undeclareResource(const String& name, const String& groupName);

		/** Find a single resource by name and return a DataStream
		 	pointing at the source of the data.
		@param resourceName The name of the resource to locate
		@param groupName The name of the resource group; this determines which 
			locations are searched. 
		@returns Shared pointer to data stream containing the data, will be
			destroyed automatically when no longer referenced
		*/
		DataStreamPtr _findResource(const String& resourceName, 
			const String& groupName = DEFAULT_RESOURCE_GROUP_NAME);

		/** Find all resources matching a given pattern (which can contain
			the character '*' as a wildcard, and return a collection of 
			DataStream objects on them.
		@param pattern The pattern to look for
		@param groupName The resource group; this determines which locations
			are searched.
		@returns Shared pointer to a data stream list , will be
			destroyed automatically when no longer referenced
		*/
		DataStreamListPtr _findResources(const String& pattern, 
			const String& groupName = DEFAULT_RESOURCE_GROUP_NAME);
		
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

		/** Internal method for getting a registered ResourceManager.
		@param resourceType String identifying the resource type.
		*/
		ResourceManager* _getResourceManager(const String& resourceType) const;

		/** Internal method called by ResourceManager when a resource is created.
		@param res Weak reference to resource
		*/
		void _notifyResourceCreated(ResourcePtr& res);

		/** Internal method called by ResourceManager when a resource is removed.
		@param res Weak reference to resource
		*/
		void _notifyResourceRemoved(ResourcePtr& res);

		/** Internal method called by ResourceManager when all resources 
			for that manager are removed.
		@param manager Pointer to the manager for which all resources are being removed
		*/
		void _notifyAllResourcesRemoved(ResourceManager* manager);

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

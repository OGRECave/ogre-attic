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
#ifndef _ResourceManager_H__
#define _ResourceManager_H__

#include "OgrePrerequisites.h"

#include "OgreResource.h"
#include "OgreResourceGroupManager.h"
#include "OgreIteratorWrappers.h"
#include "OgreCommon.h"
#include "OgreDataStream.h"
#include "OgreStringVector.h"
#include "OgreScriptLoader.h"

namespace Ogre {

    /** Defines a generic resource handler.
    @remarks
        A resource manager is responsible for managing a pool of
        resources of a particular type. It must index them, look
        them up, load and destroy them. It may also need to stay within
        a defined memory budget, and temporaily unload some resources
        if it needs to to stay within this budget.
    @par
        Resource managers use a priority system to determine what can
        be unloaded, and a Least Recently Used (LRU) policy within
        resources of the same priority.
	@par
		Resources can be loaded using the generalised load interface,
		and they can be unloaded and removed. In addition, each 
		subclass of ResourceManager will likely define custom 'load' methods
		which take explicit parameters depending on the kind of resource
		being created.
	@note
		Resources can be loaded and unloaded through the Resource class, 
		but they can only be removed (and thus eventually destroyed) using
		their parent ResourceManager.
    */
	class _OgreExport ResourceManager : public ScriptLoader
    {
    public:
		OGRE_AUTO_MUTEX // public to allow external locking
        ResourceManager();
        virtual ~ResourceManager();

        /** Creates a new blank resource, but does not immediately load it.
        @remarks
            Resource managers handle disparate types of resources, so if you want
			to get at the detailed interface of this resource, you'll have to 
			cast the result to the subclass you know you're creating. 
		@param name The unique name of the resource
		@param group The name of the resource group to attach this new resource to
		@param isManual Is this resource manually loaded? If so, you should really
			populate the loader parameter in order that the load process
			can call the loader back when loading is required. 
		@param loader Pointer to a ManualLoader implementation which will be called
			when the Resource wishes to load (should be supplied if you set
			isManual to true). You can in fact leave this parameter null 
			if you wish, but the Resource will never be able to reload if 
			anything ever causes it to unload. Therefore provision of a proper
			ManualLoader instance is strongly recommended.
        @param createParams If any parameters are required to create an instance,
            they should be supplied here as name / value pairs
        */
        virtual ResourcePtr create(const String& name, const String& group, 
            bool isManual = false, ManualResourceLoader* loader = 0, 
            const NameValuePairList* createParams = 0);
        /** Set a limit on the amount of memory this resource handler may use.
            @remarks
                If, when asked to load a new resource, the manager believes it will exceed this memory
                budget, it will temporarily unload a resource to make room for the new one. This unloading
                is not permanent and the Resource is not destroyed; it simply needs to be reloaded when
                next used.
        */
        virtual void setMemoryBudget( size_t bytes);

        /** Get the limit on the amount of memory this resource handler may use.
        */
        virtual size_t getMemoryBudget(void) const;

		/** Unloads a single resource by name.
		@remarks
			Unloaded resources are not removed, they simply free up their memory
			as much as they can and wait to be reloaded.
			@see ResourceGroupManager for unloading of resource groups.
		*/
		virtual void unload(const String& name);
		
		/** Unloads a single resource by handle.
		@remarks
			Unloaded resources are not removed, they simply free up their memory
			as much as they can and wait to be reloaded.
			@see ResourceGroupManager for unloading of resource groups.
		*/
		virtual void unload(ResourceHandle handle);

		/** Unloads all resources.
		@remarks
			Unloaded resources are not removed, they simply free up their memory
			as much as they can and wait to be reloaded.
			@see ResourceGroupManager for unloading of resource groups.
		*/
		virtual void unloadAll(void);
		/** Caused all currently loaded resources to be reloaded.
		@remarks
			All resources currently being held in this manager which are also
			marked as currently loaded will be unloaded, then loaded again.
		*/
		virtual void reloadAll(void);

		/** Remove a single resource by name.
		@remarks
			Removes a single resource, meaning it will be removed from the list
			of valid resources in this manager, also causing it to be unloaded. 
		@note
			The word 'Destroy' is not used here, since
			if any other pointers are referring to this resource, it will persist
			until they have finished with it; however to all intents and purposes
			it no longer exists and will likely get destroyed imminently.
		*/
		virtual void remove(const String& name);
		
		/** Remove a single resource by handle.
		@remarks
			Removes a single resource, meaning it will be removed from the list
			of valid resources in this manager, also causing it to be unloaded. 
		@note
			The word 'Destroy' is not used here, since
			if any other pointers are referring to this resource, it will persist
			until they have finished with it; however to all intents and purposes
			it no longer exists and will likely get destroyed imminently.
		*/
		virtual void remove(ResourceHandle handle);
		/** Removes all resources.
		@note
			The word 'Destroy' is not used here, since
			if any other pointers are referring to these resources, they will persist
			until they have been finished with; however to all intents and purposes
			the resources no longer exist and will get destroyed imminently.
        */
        virtual void removeAll(void);

        /** Retrieves a pointer to a resource by name, or null if the resource does not exist.
        */
        virtual ResourcePtr getByName(const String& name);
        /** Retrieves a pointer to a resource by handle, or null if the resource does not exist.
        */
        virtual ResourcePtr getByHandle(ResourceHandle handle);

		/** Notify this manager that a resource which it manages has been 
			'touched', ie used. 
		*/
		virtual void _notifyResourceTouched(Resource* res);

		/** Notify this manager that a resource which it manages has been 
			loaded. 
		*/
		virtual void _notifyResourceLoaded(Resource* res);

		/** Notify this manager that a resource which it manages has been 
			unloaded.
		*/
		virtual void _notifyResourceUnloaded(Resource* res);

		/** Generic load method, used to create a Resource specific to this 
			ResourceManager without using one of the specialised 'load' methods
			(containing per-Resource-type parameters).
		@param name The name of the Resource
		@param group The resource group to which this resource will belong
		@param isManual Is the resource to be manually loaded? If so, you should
			provide a value for the loader parameter
		@param loader The manual loader which is to perform the required actions
			when this resource is loaded; only applicable when you specify true
			for the previous parameter
        @param loadParams Optional pointer to a list of name/value pairs 
            containing loading parameters for this type of resource.
		*/
		virtual ResourcePtr load(const String& name, 
            const String& group, bool isManual = false, 
			ManualResourceLoader* loader = 0, const NameValuePairList* loadParams = 0);

		/** Gets the file patterns which should be used to find scripts for this
			ResourceManager.
		@remarks
			Some resource managers can read script files in order to define
			resources ahead of time. These resources are added to the available
			list inside the manager, but none are loaded initially. This allows
			you to load the items that are used on demand, or to load them all 
			as a group if you wish (through ResourceGroupManager).
		@par
			This method lets you determine the file pattern which will be used
			to identify scripts intended for this manager.
		@returns
			A list of file patterns, in the order they should be searched in.
		@see isScriptingSupported, parseScript
		*/
		virtual const StringVector& getScriptPatterns(void) const { return mScriptPatterns; }

		/** Parse the definition of a set of resources from a script file.
		@remarks
			Some resource managers can read script files in order to define
			resources ahead of time. These resources are added to the available
			list inside the manager, but none are loaded initially. This allows
			you to load the items that are used on demand, or to load them all 
			as a group if you wish (through ResourceGroupManager).
		@param stream Weak reference to a data stream which is the source of the script
		@param groupName The name of the resource group that resources which are
			parsed are to become a member of. If this group is loaded or unloaded, 
			then the resources discovered in this script will be loaded / unloaded
			with it.
		*/
		virtual void parseScript(DataStreamPtr& stream, const String& groupName) {}

		/** Gets the relative loading order of resources of this type.
		@remarks
			There are dependencies between some kinds of resource in terms of loading
			order, and this value enumerates that. Higher values load later during
			bulk loading tasks.
		*/
		virtual Real getLoadingOrder(void) const { return mLoadOrder; }

		/** Gets a string identifying the type of resource this manager handles. */
		const String& getResourceType(void) const { return mResourceType; }

    protected:

        /** Allocates the next handle. */
        ResourceHandle getNextHandle(void);

		/** Create a new resource instance compatible with this manager (no custom 
			parameters are populated at this point). 
		@remarks
			Subclasses must override this method and create a subclass of Resource.
		@param name The unique name of the resource
		@param group The name of the resource group to attach this new resource to
		@param isManual Is this resource manually loaded? If so, you should really
			populate the loader parameter in order that the load process
			can call the loader back when loading is required. 
		@param loader Pointer to a ManualLoader implementation which will be called
			when the Resource wishes to load (should be supplied if you set
			isManual to true). You can in fact leave this parameter null 
			if you wish, but the Resource will never be able to reload if 
			anything ever causes it to unload. Therefore provision of a proper
			ManualLoader instance is strongly recommended.
        @param createParams If any parameters are required to create an instance,
            they should be supplied here as name / value pairs. These do not need 
            to be set on the instance (handled elsewhere), just used if required
            to differentiate which concrete class is created.

		*/
		virtual Resource* createImpl(const String& name, ResourceHandle handle, 
			const String& group, bool isManual, ManualResourceLoader* loader, 
            const NameValuePairList* createParams) = 0;
		/** Add a newly created resource to the manager (note weak reference) */
		virtual void addImpl( ResourcePtr& res );
		/** Remove a resource from this manager; remove it from the lists. */
		virtual void removeImpl( ResourcePtr& res );
		/** Checks memory usage and pages out if required.
		*/
		virtual void checkUsage(void);
		/** Gets the current memory usage, in bytes. */
		virtual size_t getMemoryUsage(void) const { return mMemoryUsage; }


    public:
		typedef HashMap< String, ResourcePtr > ResourceMap;
		typedef std::map<ResourceHandle, ResourcePtr> ResourceHandleMap;
    protected:
        ResourceHandleMap mResourcesByHandle;
        ResourceMap mResources;
        ResourceHandle mNextHandle;
        size_t mMemoryBudget; // In bytes
        size_t mMemoryUsage; // In bytes

		// IMPORTANT - all subclasses must populate the fields below

		/// Patterns to use to look for scripts if supported (e.g. *.overlay)
		StringVector mScriptPatterns; 
		/// Loading order relative to other managers, higher is later
		Real mLoadOrder; 
		/// String identifying the resource type this manager handles
		String mResourceType; 

    public:
        typedef MapIterator<ResourceHandleMap> ResourceMapIterator;
        /** Returns an iterator over all resources in this manager. */
        ResourceMapIterator getResourceIterator(void) 
        {
            return ResourceMapIterator(mResourcesByHandle.begin(), mResourcesByHandle.end());
        }

    

    };

}

#endif

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
#include "OgreStableHeaders.h"
#include "OgreResourceGroupManager.h"
#include "OgreException.h"
#include "OgreArchive.h"
#include "OgreArchiveManager.h"
#include "OgreLogManager.h"
#include "OgreScriptLoader.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    template<> ResourceGroupManager* Singleton<ResourceGroupManager>::ms_Singleton = 0;
    ResourceGroupManager* ResourceGroupManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    ResourceGroupManager& ResourceGroupManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
	String ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME = "General";
    String ResourceGroupManager::WORLD_RESOURCE_GROUP_NAME = "World";
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    ResourceGroupManager::ResourceGroupManager()
        : mAddDefaultLocationsToWorld(true), mCurrentGroup(0)
    {
        // Create the 'General' group
        ResourceGroup* grp = new ResourceGroup();
        grp->name = DEFAULT_RESOURCE_GROUP_NAME;
        mResourceGroupMap.insert(
            ResourceGroupMap::value_type(grp->name, grp));
    }
    //-----------------------------------------------------------------------
    ResourceGroupManager::~ResourceGroupManager()
    {
        // delete all resource groups
        ResourceGroupMap::iterator i, iend;
        iend = mResourceGroupMap.end();
        for (i = mResourceGroupMap.begin(); i != iend; ++i)
        {
			deleteGroup(i->second);
        }
        mResourceGroupMap.clear();
    }
    //-----------------------------------------------------------------------
    void ResourceGroupManager::createResourceGroup(const String& name)
    {
		LogManager::getSingleton().logMessage("Creating resource group " + name);
        if (getResourceGroup(name))
        {
            Except(Exception::ERR_DUPLICATE_ITEM, 
                "Resource group with name '" + name + "' already exists!", 
                "ResourceGroupManager::createResourceGroup");
        }
        ResourceGroup* grp = new ResourceGroup();
        grp->name = name;
        mResourceGroupMap.insert(
            ResourceGroupMap::value_type(name, grp));
    }
	//-----------------------------------------------------------------------
	void ResourceGroupManager::initialiseResourceGroup(const String& name)
	{
		LogManager::getSingleton().logMessage("Initialising resource group " + name);
		parseResourceGroupScripts(name);
		createDeclaredResources(name);
	}
	//-----------------------------------------------------------------------
	void ResourceGroupManager::_initialise(void)
	{
		// Intialise all declared resource groups
		ResourceGroupMap::iterator i, iend;
		iend = mResourceGroupMap.end();
		for (i = mResourceGroupMap.begin(); i != iend; ++i)
		{
			initialiseResourceGroup(i->first);
		}
	}
    //-----------------------------------------------------------------------
    void ResourceGroupManager::loadResourceGroup(const String& name)
    {
		LogManager::getSingleton().logMessage("Loading resource group " + name);
		// load all created resources
		ResourceGroup* grp = getResourceGroup(name);
		if (!grp)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find a group named " + name, 
				"ResourceGroupManager::loadResourceGroup");
		}
		// Set current group
		mCurrentGroup = grp;

		// Count up resources for starting event
		ResourceGroup::LoadResourceOrderMap::iterator oi;
		size_t resourceCount = 0;
		for (oi = grp->loadResourceOrderMap.begin(); oi != grp->loadResourceOrderMap.end(); ++oi)
		{
			resourceCount += oi->second->size();
		}
		fireResourceGroupLoadStarted(name, resourceCount);

		// Now load for real
		for (oi = grp->loadResourceOrderMap.begin(); oi != grp->loadResourceOrderMap.end(); ++oi)
		{
			for (LoadUnloadResourceList::iterator l = oi->second->begin();
				l != oi->second->end(); ++oi)
			{
				(*l)->load();
				// fire event
				fireResourceLoaded(*l);
			}
		}
		fireResourceGroupLoadEnded(name);

		// reset current group
		mCurrentGroup = 0;
		
		LogManager::getSingleton().logMessage("Finished loading resource group " + name);
    }
    //-----------------------------------------------------------------------
    void ResourceGroupManager::unloadResourceGroup(const String& name)
    {
		LogManager::getSingleton().logMessage("Unloading resource group " + name);
		ResourceGroup* grp = getResourceGroup(name);
		if (!grp)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find a group named " + name, 
				"ResourceGroupManager::unloadResourceGroup");
		}
		// Set current group
		mCurrentGroup = grp;

		// Count up resources for starting event
		ResourceGroup::LoadResourceOrderMap::reverse_iterator oi;
		// unload in reverse order
		for (oi = grp->loadResourceOrderMap.rbegin(); oi != grp->loadResourceOrderMap.rend(); ++oi)
		{
			for (LoadUnloadResourceList::iterator l = oi->second->begin();
				l != oi->second->end(); ++oi)
			{
				(*l)->unload();
			}
		}

		// reset current group
		mCurrentGroup = 0;
		LogManager::getSingleton().logMessage("Finished unloading resource group " + name);
    }
	//-----------------------------------------------------------------------
	void ResourceGroupManager::clearResourceGroup(const String& name)
	{
		LogManager::getSingleton().logMessage("Clearing resource group " + name);
		ResourceGroup* grp = getResourceGroup(name);
		if (!grp)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find a group named " + name, 
				"ResourceGroupManager::clearResourceGroup");
		}
		// set current group
		mCurrentGroup = grp;
		dropGroupContents(grp);
		// reset current group
		mCurrentGroup = 0;
		LogManager::getSingleton().logMessage("Finished clearing resource group " + name);
	}
    //-----------------------------------------------------------------------
    void ResourceGroupManager::destroyResourceGroup(const String& name)
    {
		LogManager::getSingleton().logMessage("Destroying resource group " + name);
		ResourceGroup* grp = getResourceGroup(name);
		if (!grp)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find a group named " + name, 
				"ResourceGroupManager::destroyResourceGroup");
		}
		// set current group
		mCurrentGroup = grp;
        unloadResourceGroup(name); // will throw an exception if name not valid
		dropGroupContents(grp);
        mResourceGroupMap.erase(mResourceGroupMap.find(name));
		// reset current group
		mCurrentGroup = 0;
    }
    //-----------------------------------------------------------------------
    void ResourceGroupManager::addResourceLocation(const String& name, 
        const String& locType, const String& resGroup, bool recursive)
    {
        ResourceGroup* grp = getResourceGroup(resGroup);
        if (!grp)
        {
            createResourceGroup(resGroup);
            grp = getResourceGroup(resGroup);
        }

        // Get archive
        Archive* pArch = ArchiveManager::getSingleton().load( name, locType );
        // Add to location list
		ResourceLocation* loc = new ResourceLocation();
		loc->archive = pArch;
		loc->recursive = recursive;
        grp->locationList.push_back(loc);
        // Index resources
        StringVectorPtr vec = pArch->find("*", recursive);
        for( StringVector::iterator it = vec->begin(); it != vec->end(); ++it )
        {
			ResourceIndexEntry resIdx;
			resIdx.archive = pArch;
			resIdx.fullname = (*it);
			// Index under full name
            grp->resourceIndex[(*it)] = resIdx;
            // if recursive, index file under basename too
			if (recursive)
			{
				String baseName, path;
				StringUtil::splitFilename((*it), baseName, path);
				grp->resourceIndex[baseName] = resIdx;
			}
        }
		
		StringUtil::StrStreamType msg;
		msg << "Added resource location '" << name << "' of type '" << locType
			<< "' to resource group '" << resGroup;
		if (recursive)
			msg << " with recursive option";
		LogManager::getSingleton().logMessage(msg.str());

        // Add defaults to world?
        if (mAddDefaultLocationsToWorld && 
            resGroup == DEFAULT_RESOURCE_GROUP_NAME)
        {
            addResourceLocation(name, locType, WORLD_RESOURCE_GROUP_NAME, recursive);
        }
    }
    //-----------------------------------------------------------------------
    void ResourceGroupManager::removeResourceLocation(const String& name, 
        const String& resGroup)
    {
		ResourceGroup* grp = getResourceGroup(resGroup);
		if (!grp)
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot locate a resource group called '" + resGroup + "'", 
                "ResourceGroupManager::addResourceLocation");
        }
        // Remove from location list
		LocationList::iterator li, liend;
		liend = grp->locationList.end();
		for (li = grp->locationList.begin(); li != liend; ++li)
		{
			Archive* pArch = (*li)->archive;
			if (pArch->getName() == name)
			{
				// Delete indexes
				ResourceLocationIndex::iterator rit, ritend;
				ritend = grp->resourceIndex.end();
				for (rit = grp->resourceIndex.begin(); rit != ritend;)
				{
					if (rit->second.archive == pArch)
					{
						ResourceLocationIndex::iterator del = rit++;
						grp->resourceIndex.erase(del);
					}
					else
					{
						++rit;
					}
				}
				// Erase list entry
				grp->locationList.erase(li);
				break;
			}

		}

		LogManager::getSingleton().logMessage("Removed resource location " + name);


    }
    //-----------------------------------------------------------------------
    void ResourceGroupManager::declareResource(const String& name, 
        const String& resourceType, const String& groupName,
		const NameValuePairList& loadParameters)
    {
		ResourceGroup* grp = getResourceGroup(groupName);
		if (!grp)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find a group named " + groupName, 
				"ResourceGroupManager::declareResource");
		}
		
		ResourceDeclaration dcl;
		dcl.parameters = loadParameters;
		dcl.resourceName = name;
		dcl.resourceType = resourceType;
		grp->resourceDeclarations.push_back(dcl);
    }
    //-----------------------------------------------------------------------
    void ResourceGroupManager::undeclareResource(const String& name, 
		const String& groupName)
    {
		ResourceGroup* grp = getResourceGroup(groupName);
		if (!grp)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find a group named " + groupName, 
				"ResourceGroupManager::undeclareResource");
		}

		for (ResourceDeclarationList::iterator i = grp->resourceDeclarations.begin();
			i != grp->resourceDeclarations.end(); ++i)
		{
			if (i->resourceName == name)
			{
				grp->resourceDeclarations.erase(i);
				break;
			}
		}
    }
    //-----------------------------------------------------------------------
    DataStreamPtr ResourceGroupManager::_findResource(
        const String& resourceName, const String& groupName)
    {
		// Try to find in resource index first
		ResourceGroup* grp = getResourceGroup(groupName);
		if (!grp)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot locate a resource group called '" + groupName + "'", 
				"ResourceGroupManager::_findResource");
		}

		Archive* pArch = 0;
		ResourceLocationIndex::iterator rit = grp->resourceIndex.find(resourceName);
		if (rit != grp->resourceIndex.end())
		{
			// Found in the index
			pArch = rit->second.archive;
			// look up under the fullname (this lets us load based on short name
			// if recursive was on)
			return pArch->open(rit->second.fullname);
		}
		else
		{
			// Search the hard way
			LocationList::iterator li, liend;
			liend = grp->locationList.end();
			for (li = grp->locationList.begin(); li != liend; ++li)
			{
				Archive* arch = (*li)->archive;
				DataStreamPtr ptr = arch->open(resourceName);
				if (!ptr.isNull())
				{
					return ptr;
				}
			}
		}

		
		// Not found
		Except(Exception::ERR_FILE_NOT_FOUND, "Cannot locate resource " + 
			resourceName + " in resource group " + groupName + ".", 
			"ResourceGroupManager::_findResource");

		// Keep compiler happy
		return DataStreamPtr();


    }
    //-----------------------------------------------------------------------
    DataStreamListPtr ResourceGroupManager::_findResources(
        const String& pattern, const String& groupName)
    {
		ResourceGroup* grp = getResourceGroup(groupName);
		if (!grp)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot locate a resource group called '" + groupName + "'", 
				"ResourceGroupManager::_findResources");
		}

		// Iterate through all the archives and build up a combined list of
		// streams
		DataStreamListPtr ret = DataStreamListPtr(new DataStreamList());

		LocationList::iterator li, liend;
		liend = grp->locationList.end();
		for (li = grp->locationList.begin(); li != liend; ++li)
		{
			Archive* arch = (*li)->archive;
			// Find all the names based on whether this archive is recursive
			StringVectorPtr names = arch->find(pattern, (*li)->recursive);

			// Iterate over the names and load a stream for each
			for (StringVector::iterator ni = names->begin(); ni != names->end(); ++ni)
			{
				DataStreamPtr ptr = arch->open(*ni);
				if (!ptr.isNull())
				{
					ret->push_back(ptr);
				}
			}
		}
		return ret;
		
    }
    //-----------------------------------------------------------------------
    void ResourceGroupManager::addResourceGroupListener(ResourceGroupListener* l)
    {
		mResourceGroupListenerList.push_back(l);
    }
    //-----------------------------------------------------------------------
    void ResourceGroupManager::removeResourceGroupListener(ResourceGroupListener* l)
    {
		for (ResourceGroupListenerList::iterator i = mResourceGroupListenerList.begin();
			i != mResourceGroupListenerList.end(); ++i)
		{
			if (*i == l)
			{
				mResourceGroupListenerList.erase(i);
				break;
			}
		}
    }
    //-----------------------------------------------------------------------
    void ResourceGroupManager::_registerResourceManager(
        const String& resourceType, ResourceManager* rm)
    {
		LogManager::getSingleton().logMessage(
			"Registering ResourceManager for type " + resourceType);
		mResourceManagerMap[resourceType] = rm;
    }
    //-----------------------------------------------------------------------
    void ResourceGroupManager::_unregisterResourceManager(
        const String& resourceType)
    {
		LogManager::getSingleton().logMessage(
			"Unregistering ResourceManager for type " + resourceType);
		
		ResourceManagerMap::iterator i = mResourceManagerMap.find(resourceType);
		if (i != mResourceManagerMap.end())
		{
			mResourceManagerMap.erase(i);
		}
    }
	//-----------------------------------------------------------------------
    void ResourceGroupManager::_registerScriptLoader(ScriptLoader* su)
	{
		mScriptLoaderOrderMap.insert(
			ScriptLoaderOrderMap::value_type(su->getLoadingOrder(), su));
	}
	//-----------------------------------------------------------------------
    void ResourceGroupManager::_unregisterScriptLoader(ScriptLoader* su)
	{
		Real order = su->getLoadingOrder();
		ScriptLoaderOrderMap::iterator oi = mScriptLoaderOrderMap.find(order);
		while (oi != mScriptLoaderOrderMap.end() && oi->first == order)
		{
			if (oi->second == su)
			{
				// erase does not invalidate on multimap, except current
				ScriptLoaderOrderMap::iterator del = oi++;
				mScriptLoaderOrderMap.erase(del);
			}
			else
			{
				++oi;
			}
		}
	}
	//-----------------------------------------------------------------------
	void ResourceGroupManager::parseResourceGroupScripts(const String& name)
	{
		LogManager::getSingleton().logMessage(
			"Parsing scripts for resource group " + name);

		ResourceGroup* grp = getResourceGroup(name);
		if (!grp)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find a group named " + name, 
				"ResourceGroupManager::parseResourceGroupScripts");
		}
		// Set current group
		mCurrentGroup = grp;

		// Count up the number of scripts we have to parse
        typedef std::list<DataStreamListPtr> StreamListList;
        typedef std::map<ScriptLoader*, StreamListList> ScriptLoaderStreamList;
        ScriptLoaderStreamList scriptLoaderStreamList;
		size_t scriptCount = 0;
		// Iterate over script users in loading order and get streams
		ScriptLoaderOrderMap::iterator oi;
		for (oi = mScriptLoaderOrderMap.begin();
			oi != mScriptLoaderOrderMap.end(); ++oi)
		{
			ScriptLoader* su = oi->second;
            StreamListList streamListList;

			// Get all the patterns and search them
			const StringVector& patterns = su->getScriptPatterns();
			for (StringVector::const_iterator p = patterns.begin(); p != patterns.end(); ++p)
			{
				DataStreamListPtr streamList = _findResources(*p, name);
				scriptCount += streamList->size();
				streamListList.push_back(streamList);
			}
            scriptLoaderStreamList[su] = streamListList;
		}
		// Fire scripting event
		fireResourceGroupScriptingStarted(name, scriptCount);

		// Iterate over scripts and parse
		// Note we respect original ordering
        for (ScriptLoaderStreamList::iterator slsi = scriptLoaderStreamList.begin();
            slsi != scriptLoaderStreamList.end(); ++slsi)
        {
			ScriptLoader* su = slsi->first;
            // Iterate over each list
            for (StreamListList::iterator slli = slsi->second.begin(); slli != slsi->second.end(); ++slli)
            {
			    // Iterate over each item in the list
			    for (DataStreamList::iterator si = (*slli)->begin(); si != (*slli)->end(); ++si)
			    {
                    LogManager::getSingleton().logMessage(
                        "Parsing script " + (*si)->getName());
				    su->parseScript(*si, name);
				    fireScriptParsed((*si)->getName());
			    }
            }
		}

		// Reset current group
		mCurrentGroup = 0;
		fireResourceGroupScriptingEnded(name);
		LogManager::getSingleton().logMessage(
			"Finished parsing scripts for resource group " + name);
	}
	//-----------------------------------------------------------------------
	void ResourceGroupManager::createDeclaredResources(const String& name)
	{
		ResourceGroup* grp = getResourceGroup(name);
		if (!grp)
		{
			Except(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find a group named " + name, 
				"ResourceGroupManager::createDeclaredResources");
		}
		// Set current group
		mCurrentGroup = grp;


		for (ResourceDeclarationList::iterator i = grp->resourceDeclarations.begin();
			i != grp->resourceDeclarations.end(); ++i)
		{
			ResourceDeclaration& dcl = *i;
			// Retrieve the appropriate manager
			ResourceManager* mgr = _getResourceManager(dcl.resourceType);
			// Create the resource
			ResourcePtr res = mgr->create(dcl.resourceName, name);
			// Set custom parameters
			res->setParameterList(dcl.parameters);
			// Add resource to load list
			ResourceGroup::LoadResourceOrderMap::iterator li = 
				grp->loadResourceOrderMap.find(mgr->getLoadingOrder());
			LoadUnloadResourceList* loadList;
			if (li == grp->loadResourceOrderMap.end())
			{
				loadList = new LoadUnloadResourceList();
				grp->loadResourceOrderMap[mgr->getLoadingOrder()] = loadList;
			}
			else
			{
				loadList = li->second;
			}
			loadList->push_back(res);

		}

		// Reset current group
		mCurrentGroup = 0;
	}
    //-----------------------------------------------------------------------
	void ResourceGroupManager::_notifyResourceCreated(ResourcePtr& res)
	{
		if (mCurrentGroup)
		{
			// Use current group (batch loading)
			addCreatedResource(res, *mCurrentGroup);
		}
		else
		{
			// Find group
			ResourceGroup* grp = getResourceGroup(res->getGroup());
			if (grp)
			{
				addCreatedResource(res, *grp);
			}
		}
	}
	//-----------------------------------------------------------------------
	void ResourceGroupManager::_notifyResourceRemoved(ResourcePtr& res)
	{
		if (mCurrentGroup)
		{
			// Do nothing - we're batch unloading so list will be cleared
		}
		else
		{
			// Find group
			ResourceGroup* grp = getResourceGroup(res->getGroup());
			if (grp)
			{
				ResourceGroup::LoadResourceOrderMap::iterator i = 
					grp->loadResourceOrderMap.find(
						res->getCreator()->getLoadingOrder());
				if (i != grp->loadResourceOrderMap.end())
				{
					// Iterate over the resource list and remove
					LoadUnloadResourceList* resList = i->second;
					for (LoadUnloadResourceList::iterator l = resList->begin();
						l != resList->end(); ++ l)
					{
						if ((*l).getPointer() == res.getPointer())
						{
							// this is the one
							resList->erase(l);
							break;
						}
					}
				}
			}
		}
	}
	//-----------------------------------------------------------------------
	void ResourceGroupManager::_notifyAllResourcesRemoved(ResourceManager* manager)
	{
		// Iterate over all groups
		for (ResourceGroupMap::iterator grpi = mResourceGroupMap.begin();
			grpi != mResourceGroupMap.end(); ++grpi)
		{
			// Iterate over all priorities
			for (ResourceGroup::LoadResourceOrderMap::iterator oi = grpi->second->loadResourceOrderMap.begin();
				oi != grpi->second->loadResourceOrderMap.end(); ++oi)
			{
				// Iterate over all resources
				for (LoadUnloadResourceList::iterator l = oi->second->begin();
					l != oi->second->end(); )
				{
					if ((*l)->getCreator() == manager)
					{
						// Increment first since iterator will be invalidated
						LoadUnloadResourceList::iterator del = l++;
						oi->second->erase(del);
					}
					else
					{
						++l;
					}
				}
			}

		}
	}
	//-----------------------------------------------------------------------
	void ResourceGroupManager::addCreatedResource(ResourcePtr& res, ResourceGroup& grp)
	{
		Real order = res->getCreator()->getLoadingOrder();

		ResourceGroup::LoadResourceOrderMap::iterator i = grp.loadResourceOrderMap.find(order);
		LoadUnloadResourceList* loadList;
		if (i == grp.loadResourceOrderMap.end())
		{
			loadList = new LoadUnloadResourceList();
			grp.loadResourceOrderMap[order] = loadList;
		}
		else
		{
			loadList = i->second;
		}
		loadList->push_back(res);
	}
	//-----------------------------------------------------------------------
	ResourceGroupManager::ResourceGroup* ResourceGroupManager::getResourceGroup(const String& name)
	{
		ResourceGroupMap::iterator i = mResourceGroupMap.find(name);
		if (i != mResourceGroupMap.end())
		{
			return i->second;
		}
		return 0;

	}
    //-----------------------------------------------------------------------
    ResourceManager* ResourceGroupManager::_getResourceManager(const String& resourceType) 
    {
        ResourceManagerMap::iterator i = mResourceManagerMap.find(resourceType);
        if (i == mResourceManagerMap.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot locate resource manager for resource type '" +
                resourceType + "'", "ResourceGroupManager::_getResourceManager");
        }
        return i->second;

    }
	//-----------------------------------------------------------------------
	void ResourceGroupManager::dropGroupContents(ResourceGroup* grp)
	{
		// delete all the load list entries
		ResourceGroup::LoadResourceOrderMap::iterator j, jend;
		jend = grp->loadResourceOrderMap.end();
		for (j = grp->loadResourceOrderMap.begin(); j != jend; ++j)
		{
			// Iterate over resources
			for (LoadUnloadResourceList::iterator k = j->second->begin();
				k != j->second->end(); ++k)
			{
				(*k)->getCreator()->remove((*k)->getHandle());
			}
			delete j->second;
		}
	}
	//-----------------------------------------------------------------------
	void ResourceGroupManager::deleteGroup(ResourceGroup* grp)
	{
		// delete all the load list entries
		ResourceGroup::LoadResourceOrderMap::iterator j, jend;
		jend = grp->loadResourceOrderMap.end();
		for (j = grp->loadResourceOrderMap.begin(); j != jend; ++j)
		{
			// Don't iterate over resources to drop with ResourceManager
			// Assume this is being done anyway since this is a shutdown method
			delete j->second;
		}
		// Drop location list
		for (LocationList::iterator ll = grp->locationList.begin();
			ll != grp->locationList.end(); ++ll)
		{
			delete *ll;
		}

		// delete ResourceGroup
		delete grp;
	}
	//-----------------------------------------------------------------------
	void ResourceGroupManager::fireResourceGroupScriptingStarted(const String& groupName, size_t scriptCount)
	{
		for (ResourceGroupListenerList::iterator l = mResourceGroupListenerList.begin();
			l != mResourceGroupListenerList.end(); ++l)
		{
			(*l)->resourceGroupScriptingStarted(groupName, scriptCount);
		}
	}
	//-----------------------------------------------------------------------
	void ResourceGroupManager::fireScriptParsed(const String& scriptName)
	{
		for (ResourceGroupListenerList::iterator l = mResourceGroupListenerList.begin();
			l != mResourceGroupListenerList.end(); ++l)
		{
			(*l)->scriptParsed(scriptName);
		}
	}
	//-----------------------------------------------------------------------
	void ResourceGroupManager::fireResourceGroupScriptingEnded(const String& groupName)
	{
		for (ResourceGroupListenerList::iterator l = mResourceGroupListenerList.begin();
			l != mResourceGroupListenerList.end(); ++l)
		{
			(*l)->resourceGroupScriptingEnded(groupName);
		}
	}
	//-----------------------------------------------------------------------
	void ResourceGroupManager::fireResourceGroupLoadStarted(const String& groupName, size_t resourceCount)
	{
		for (ResourceGroupListenerList::iterator l = mResourceGroupListenerList.begin();
			l != mResourceGroupListenerList.end(); ++l)
		{
			(*l)->resourceGroupLoadStarted(groupName, resourceCount);
		}
	}
	//-----------------------------------------------------------------------
	void ResourceGroupManager::fireResourceLoaded(const ResourcePtr& resource)
	{
		for (ResourceGroupListenerList::iterator l = mResourceGroupListenerList.begin();
			l != mResourceGroupListenerList.end(); ++l)
		{
			(*l)->resourceLoaded(resource);
		}
	}
	//-----------------------------------------------------------------------
	void ResourceGroupManager::fireResourceGroupLoadEnded(const String& groupName)
	{
		for (ResourceGroupListenerList::iterator l = mResourceGroupListenerList.begin();
			l != mResourceGroupListenerList.end(); ++l)
		{
			(*l)->resourceGroupLoadEnded(groupName);
		}
	}
	//-----------------------------------------------------------------------

}

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
#include "OgreResourceManager.h"

#include "OgreException.h"
#include "OgreArchive.h"
#include "OgreArchiveManager.h"
#include "OgreStringVector.h"
#include "OgreStringConverter.h"
#include "OgreResourceGroupManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    ResourceManager::ResourceManager()
		:mMemoryUsage(0), mNextHandle(1), mLoadOrder(0)
    {
        // Init memory limit & usage
        mMemoryBudget = std::numeric_limits<unsigned long>::max();
    }
    //-----------------------------------------------------------------------
    ResourceManager::~ResourceManager()
    {
        removeAll();
    }
	//-----------------------------------------------------------------------
    ResourcePtr ResourceManager::create(const String& name, const String& group, 
		bool isManual, ManualResourceLoader* loader, const NameValuePairList* params)
	{
		// Call creation implementation
		ResourcePtr ret = ResourcePtr(
            createImpl(name, getNextHandle(), group, isManual, loader, params));
        if (params)
            ret->setParameterList(*params);

		addImpl(ret);
		// Tell resource group manager
		ResourceGroupManager::getSingleton()._notifyResourceCreated(ret);
		return ret;

	}
    //-----------------------------------------------------------------------
    ResourcePtr ResourceManager::load(const String& name, 
        const String& group, bool isManual, ManualResourceLoader* loader, 
        const NameValuePairList* loadParams)
    {
        ResourcePtr ret = getByName(name);
        if (ret.isNull())
        {
            ret = create(name, group, isManual, loader, loadParams);
            ret->load();
        }
        return ret;
    }
    //-----------------------------------------------------------------------
    void ResourceManager::addImpl( ResourcePtr& res )
    {
		OGRE_LOCK_AUTO_MUTEX

        std::pair<ResourceMap::iterator, bool> result = 
            mResources.insert( ResourceMap::value_type( res->getName(), res ) );
        if (!result.second)
        {
            Except(Exception::ERR_DUPLICATE_ITEM, "Resource with the name " + res->getName() + 
                " already exists.", "ResourceManager::add");
        }
        std::pair<ResourceHandleMap::iterator, bool> resultHandle = 
            mResourcesByHandle.insert( ResourceHandleMap::value_type( res->getHandle(), res ) );
        if (!result.second)
        {
            Except(Exception::ERR_DUPLICATE_ITEM, "Resource with the handle " + 
                StringConverter::toString(res->getHandle()) + 
                " already exists.", "ResourceManager::add");
        }

    }
	//-----------------------------------------------------------------------
	void ResourceManager::removeImpl( ResourcePtr& res )
	{
		OGRE_LOCK_AUTO_MUTEX

		ResourceMap::iterator nameIt = mResources.find(res->getName());
		if (nameIt != mResources.end())
		{
			mResources.erase(nameIt);
		}

		ResourceHandleMap::iterator handleIt = mResourcesByHandle.find(res->getHandle());
		if (handleIt != mResourcesByHandle.end())
		{
			mResourcesByHandle.erase(handleIt);
		}
		// Tell resource group manager
		ResourceGroupManager::getSingleton()._notifyResourceRemoved(res);
	}
    //-----------------------------------------------------------------------
    void ResourceManager::setMemoryBudget( size_t bytes)
    {
        // Update limit & check usage
        mMemoryBudget = bytes;
        checkUsage();
    }
    //-----------------------------------------------------------------------
    size_t ResourceManager::getMemoryBudget(void) const
    {
        return mMemoryBudget;
    }
	//-----------------------------------------------------------------------
	void ResourceManager::unload(const String& name)
	{
		ResourcePtr res = getByName(name);

		if (!res.isNull())
		{
			// Unload resource
			res->unload();

		}
	}
	//-----------------------------------------------------------------------
	void ResourceManager::unload(ResourceHandle handle)
	{
		ResourcePtr res = getByHandle(handle);

		if (!res.isNull())
		{
			// Unload resource
			res->unload();

		}
	}
	//-----------------------------------------------------------------------
	void ResourceManager::unloadAll(void)
	{
		OGRE_LOCK_AUTO_MUTEX

		ResourceMap::iterator i, iend;
		iend = mResources.end();
		for (i = mResources.begin(); i != iend; ++i)
		{
			i->second->unload();
		}

	}
	//-----------------------------------------------------------------------
	void ResourceManager::reloadAll(void)
	{
		OGRE_LOCK_AUTO_MUTEX

		ResourceMap::iterator i, iend;
		iend = mResources.end();
		for (i = mResources.begin(); i != iend; ++i)
		{
			i->second->reload();
		}

	}
    //-----------------------------------------------------------------------
    void ResourceManager::remove(ResourcePtr& res)
    {
        removeImpl(res);
    }
	//-----------------------------------------------------------------------
	void ResourceManager::remove(const String& name)
	{
		ResourcePtr res = getByName(name);

		if (!res.isNull())
		{
			removeImpl(res);
		}
	}
	//-----------------------------------------------------------------------
	void ResourceManager::remove(ResourceHandle handle)
	{
		ResourcePtr res = getByHandle(handle);

		if (!res.isNull())
		{
			removeImpl(res);
		}
	}
	//-----------------------------------------------------------------------
	void ResourceManager::removeAll(void)
	{
		OGRE_LOCK_AUTO_MUTEX

		mResources.clear();
		mResourcesByHandle.clear();
		// Notify resource group manager
		ResourceGroupManager::getSingleton()._notifyAllResourcesRemoved(this);
	}
    //-----------------------------------------------------------------------
    ResourcePtr ResourceManager::getByName(const String& name)
    {
		OGRE_LOCK_AUTO_MUTEX

        ResourceMap::iterator it = mResources.find(name);

        if( it == mResources.end())
		{
            return ResourcePtr();
		}
        else
        {
            return it->second;
        }
    }
    //-----------------------------------------------------------------------
    ResourcePtr ResourceManager::getByHandle(ResourceHandle handle)
    {
		OGRE_LOCK_AUTO_MUTEX

        ResourceHandleMap::iterator it = mResourcesByHandle.find(handle);
        if (it == mResourcesByHandle.end())
        {
            return ResourcePtr();
        }
        else
        {
            return it->second;
        }
    }
    //-----------------------------------------------------------------------
    ResourceHandle ResourceManager::getNextHandle(void)
    {
		OGRE_LOCK_AUTO_MUTEX

        return mNextHandle++;
    }
    //-----------------------------------------------------------------------
    void ResourceManager::checkUsage(void)
    {
        // TODO Page out here?
    }
	//-----------------------------------------------------------------------
	void ResourceManager::_notifyResourceTouched(Resource* res)
	{
		// TODO
	}
	//-----------------------------------------------------------------------
	void ResourceManager::_notifyResourceLoaded(Resource* res)
	{
		OGRE_LOCK_AUTO_MUTEX

		mMemoryUsage += res->getSize();
	}
	//-----------------------------------------------------------------------
	void ResourceManager::_notifyResourceUnloaded(Resource* res)
	{
		OGRE_LOCK_AUTO_MUTEX

		mMemoryUsage -= res->getSize();
	}
	//-----------------------------------------------------------------------

}




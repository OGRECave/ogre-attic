/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreResourceManager.h"

#include "OgreException.h"
#include "OgreArchiveEx.h"
#include "OgreArchiveManager.h"
#include "OgreStringVector.h"

namespace Ogre {

    std::vector<ArchiveEx*> ResourceManager::mCommonVFS;
    ResourceManager::FileMap ResourceManager::mCommonArchiveFiles;

    //-----------------------------------------------------------------------
    String convertPath( const String &init)
    {
        // Internal method for standardising paths
        // Use forward slashes only, end with slash
        String path = init;

        std::replace( path.begin(), path.end(), '\\', '/' );
        if( path[path.length() - 1] != '/' )
            path += '/';

        return path;
        /*

        int pos;

        pos = path.find("\\",0);
        while (pos != -1)
        {
            // Replace bckslash with forward slash
            path.at(pos) = '/';
            pos = path.find("\\",pos);
        }

        if (path.at(path.length()-1) != '/')
        {
            // Make sure finishes with slash
            path += "/";

        }
        */
    }

    //-----------------------------------------------------------------------
    ResourceManager::ResourceManager()
    {
        // Init memory limit & usage
        mMemoryBudget = std::numeric_limits<unsigned long>::max();
        mMemoryUsage = 0;

    }

    //-----------------------------------------------------------------------
    ResourceManager::~ResourceManager()
    {
        this->unloadAndDestroyAll();
    }

    //-----------------------------------------------------------------------
    void ResourceManager::load(Resource *res, int priority)
    {
        res->load();
        res->touch();

        mResources.insert( ResourceMap::value_type( res->getName(), res ) );
    }

    //-----------------------------------------------------------------------
    void ResourceManager::setMemoryBudget( size_t bytes)
    {
        // Update limit & check usage
        mMemoryBudget = bytes;
        checkUsage();
    }

    //-----------------------------------------------------------------------
    void ResourceManager::unload(Resource* res)
    {
        // Unload resource
        res->unload();

        // Erase entry in map
        mResources.erase( res->getName() );

        // Update memory usage
        mMemoryUsage -= res->getSize();
    }

    //-----------------------------------------------------------------------
    void ResourceManager::unloadAndDestroyAll()
    {
        // Unload & delete resources in turn
        for(
            ResourceMap::iterator it = mResources.begin();
            it != mResources.end();
            ++it)
        {
            it->second->unload();
            it->second->destroy();
        }

        // Empty the list
        mResources.clear();
    }
    //-----------------------------------------------------------------------
    Resource* ResourceManager::getByName(const String& name)
    {
        ResourceMap::iterator it = mResources.find(name);

        if( it == mResources.end() )
            return 0;
        else
            return it->second;
    }

    //-----------------------------------------------------------------------
    void ResourceManager::checkUsage(void)
    {
        // Page out here?
    }

    //-----------------------------------------------------------------------
    void ResourceManager::addSearchPath( const String& path)
    {
        addArchiveEx( convertPath(path), "FileSystem" );
    }

    //-----------------------------------------------------------------------
    void ResourceManager::addCommonSearchPath( const String& path)
    {
        addCommonArchiveEx( convertPath(path), "FileSystem" );
    }

    //-----------------------------------------------------------------------
    void ResourceManager::addArchiveEx( const String& strName, const String& strDriverName )
    {
        ArchiveEx* pArch = ArchiveManager::getSingleton().load( strName, strDriverName );

        StringVector vec = pArch->getAllNamesLike( "", "" );
        for( StringVector::iterator it = vec.begin(); it != vec.end(); ++it )
            mArchiveFiles[(*it)] = pArch;

        mVFS.push_back(pArch);
    }

    //-----------------------------------------------------------------------
    void ResourceManager::addCommonArchiveEx( const String& strName, const String& strDriverName )
    {
        ArchiveEx* pArch = ArchiveManager::getSingleton().load( strName, strDriverName );

        StringVector vec = pArch->getAllNamesLike( "", "" );
        for( StringVector::iterator it = vec.begin(); it != vec.end(); ++it )
            mCommonArchiveFiles[(*it)] = pArch;

        mCommonVFS.push_back(pArch);
    }

    //-----------------------------------------------------------------------
    bool ResourceManager::_findResourceData(
        const String& filename,
        DataChunk& refChunk )
    {
        DataChunk* pChunk = &refChunk;
        // Search file cache first
        // NB don't treat this as definitive, incase ArchiveEx can't list all existing files
        FileMap::const_iterator it;
        if( ( it = mArchiveFiles.find( filename ) ) != mArchiveFiles.end() )
            return it->second->fileRead( filename, &pChunk );
        if( ( it = mCommonArchiveFiles.find( filename ) ) != mCommonArchiveFiles.end() )
            return it->second->fileRead( filename, &pChunk );

        // Not found in cache
        // Look for it the hard way
        std::vector<ArchiveEx*>::iterator j; 
        // Search archives specific to this resource type
        for(j = mVFS.begin(); j != mVFS.end(); ++j )
        {
            if( *j && (*j)->fileTest(filename) )
            {
                return (*j)->fileRead( filename, &pChunk );
            }
        }
        // Search common archives
        for(j = mCommonVFS.begin(); j != mCommonVFS.end(); ++j )                 
        {
            if( *j && (*j)->fileTest(filename) )
            {
                return (*j)->fileRead( filename, &pChunk );
            }
        }
        
        // Not found
        Except(
            Exception::ERR_ITEM_NOT_FOUND,
            "Resource " + filename + " not found.",
            "ResourceManager::_findResourceData" );

        // To keep compiler happy
        return false;
    }
    //-----------------------------------------------------------------------
    std::set<String> ResourceManager::_getAllCommonNamesLike( const String& startPath, const String& extension )
    {
        std::vector<ArchiveEx*>::iterator i;
        StringVector vecFiles;
        std::set<String> retFiles;

        // search common archives
        for (i = mCommonVFS.begin(); i != mCommonVFS.end(); ++i)
        {
            vecFiles = (*i)->getAllNamesLike( "./", extension);
            for (StringVector::iterator si = vecFiles.begin(); si != vecFiles.end(); ++si)
            {
                retFiles.insert(*si);
            }
        }

        return retFiles;
    }
    //-----------------------------------------------------------------------
    bool ResourceManager::_findCommonResourceData( const String& filename, DataChunk& refChunk )
    {
        DataChunk* pChunk = &refChunk;
        // Search file cache first
        // NB don't treat this as definitive, incase ArchiveEx can't list all existing files
        FileMap::const_iterator it;
        if( ( it = mCommonArchiveFiles.find( filename ) ) != mCommonArchiveFiles.end() )
            return it->second->fileRead( filename, &pChunk );

        // Not found in cache
        // Look for it the hard way
        std::vector<ArchiveEx*>::iterator j; 
        // Search common archives
        for(j = mCommonVFS.begin(); j != mCommonVFS.end(); ++j )                 
        {
            if( *j && (*j)->fileTest(filename) )
            {
                return (*j)->fileRead( filename, &pChunk );
            }
        }
        
        // Not found
        Except(
            Exception::ERR_ITEM_NOT_FOUND,
            "Resource " + filename + " not found.",
            "ResourceManager::_findCommonResourceData" );

        // To keep compiler happy
        return false;
    }

}




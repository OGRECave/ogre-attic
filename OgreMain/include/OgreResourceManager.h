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
#ifndef _ResourceManager_H__
#define _ResourceManager_H__

#include "OgrePrerequisites.h"

#include "OgreResource.h"
#include "OgreResourceMap.h"
#include "OgreDataChunk.h"
#include "OgreArchiveEx.h"


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
    */
    class _OgreExport ResourceManager
    {
    public:
        ResourceManager();
        virtual ~ResourceManager();

        /** Set a limit on the amount of memory this resource handler may use.
            @remarks
                If, when asked to load a new resource, the manager believes it will exceed this memory
                budget, it will temporarily unload a resource to make room for the new one. This unloading
                is not permanent and the Resource is not destroyed; it simply needs to be reloaded when
                next used.
        */
        virtual void setMemoryBudget( size_t bytes);

        /** Creates a new blank resource, compatible with this manager.
            @remarks
                Resource managers handle disparate types of resources. This method returns a pointer to a
                valid new instance of the kind of resource managed here. The caller should  complete the
                details of the returned resource and call ResourceManager::load to load the resource. Note
                that it is the CALLERS responsibility to destroy this object when it is no longer required
                (after calling ResourceManager::unload if it had been loaded).
        */
        virtual Resource* create( const String& name ) = 0;

        /** Load a resource. Resources will be subclasses.
        */
        virtual void load( Resource *res, int priority );

        /** Add a resource to this manager; normally only done by subclasses.
        */
        virtual void add( Resource *res );
        /** Unloads a Resource from the managed resources list, calling it's unload() method.
            @remarks
                This method removes a resource from the list maintained by this manager, and unloads it from
                memory. It does NOT destroy the resource itself, although the memory used by it will be largely
                freed up. This would allow you to reload the resource again if you wished. 
            @par
                Permanently destroying the resource is, as mentioned in ResourceManager::create, <b>the library 
                user's responsibility</b>.
        */
        virtual void unload( Resource *res );

        /** Unloads all Resources from memory.
            @remarks
                Note that unlike ResourceManager::unload, Resource objects are DESTROYED as well as unloaded.
                This is because you are unlikely to be managing the deletion of the objects individually in this case.
        */
        virtual void unloadAndDestroyAll(void);

        /** Retrieves a pointer to a resource by name, or null if the resource does not exist.
        */
        virtual Resource* getByName(const String& name);

        /** Adds a relative path to search for resources of this type.
            @remarks
                This method adds the supplied path to the list of relative locations that that will be searched for
                a single type of resource only. Each subclass of ResourceManager will maintain it's own list of
                specific subpaths, which it will append to the current path as it searches for matching files.
        */
        void addSearchPath( const String& path );

        /** Adds a relative search path for resources of ALL types.

            <p>
            This method has the same effect as ResourceManager::addSearchPath, except that the path added
            applies to ALL resources, not just the one managed by the subclass in question.
            </p>
        */
        static void addCommonSearchPath( const String& path );

        /** Adds an archive to the search path for this type of resource.
            @remarks
                Ogre can load resources from archives. This method adds the named archive to the
                search path for the type of resource managed by the specific resource manager. 
                Archives are not searched for themselves so a complete path must be specified 
                here (or relative to the current path). Archives take precedence over files 
                in the filesystem.
        */
        void addArchiveEx( const String& strName, const String& strDriverName );

        /** Adds an archive to the search path for all resources.
            @remarks
                As ResourceManager::addArchive, except this archive is used for all types of resources, not
                just the type managed by the resource manager in question.
        */
        static void addCommonArchiveEx( const String& strName, const String& strDriverName );

        /** Internal method, used for locating resource data in the file system / archives.
            @param
                filename File to find
            @param
                refChunk Reference to a DataChunk object to fill with the data from the file
            @returns
                On success, true is returned
            @par
                On failiure, false is returnec
        */
        bool _findResourceData( const String& filename, DataChunk& refChunk );

        /** Returns a collection of files with the given extension in the common resource paths.
            @remarks
                This is a convenience method to allow non-subclasses to search for files in the common paths.
            @param 
                startPath The path, relative to each common resource start, to search in (use "./" for the root)
            @param 
                extension The extension of file to search for.
            @returns 
                A set of String filenames (it is a set because duplicates will be ignored)
        */
        static std::set<String> _getAllCommonNamesLike( const String& startPath, const String& extension );

        /** Returns a collection of files with the given extension in all resource paths, common and specific to this resource type.
            @param 
                startPath The path, relative to each common resource start, to search in (use "./" for the root)
            @param 
                extension The extension of file to search for.
            @returns 
                A set of String filenames (it is a set because duplicates will be ignored)
        */
        std::set<String> _getAllNamesLike( const String& startPath, const String& extension );

        /** Internal method, used for locating common resource data in the file system / archives.
            @remarks
                This is a static version of _findResourceData specifically designed to only search in the
                common resource archives, and is therefore usable from non-ResourceManager subclasses.
            @param
                filename File to find
            @param
                refChunk Reference to a DataChunk object to fill with the data from the file
            @returns
                On success, true is returned
            @par
                On failiure, false is returned
        */
        static bool _findCommonResourceData( const String& filename, DataChunk& refChunk );

    protected:

        typedef HashMap< String, ArchiveEx *, _StringHash > FileMap;
        static FileMap mCommonArchiveFiles;
        FileMap mArchiveFiles;

        ResourceMap mResources;

        size_t mMemoryBudget; // In bytes
        size_t mMemoryUsage; // In bytes, at last checkUsage() call

        /** Checks memory usage and pages out if required.
        */
        void checkUsage(void);

        /// Collection of searchable ArchiveEx classes (virtual file system) for all resource types.
        static std::vector<ArchiveEx*> mCommonVFS;

        /// Collection of searchable ArchiveEx classes (virtual file system) for this resource type.
        std::vector<ArchiveEx*> mVFS;
    };

}

#endif

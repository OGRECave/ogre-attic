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
#ifndef __MemoryManager_H__
#define __MemoryManager_H__

#include "OgrePlatform.h"
#include "OgreStdHeaders.h"

namespace Ogre
{    
    class _OgreExport MemoryManager
    {
    public:        
        static MemoryManager sMemManager;

    public:
        MemoryManager();
        ~MemoryManager();

        static void setAllocNewInfo( const char *szFile, size_t uLine );
        static void setDeallocNewInfo( const char *szFile, size_t uLine );

        void *allocMem( const char *szFile, size_t uLine, size_t count ) throw ( );
        void *rllocMem( const char *szFile, size_t uLine, void *ptr , size_t count ) throw ( );
        void *cllocMem( const char *szFile, size_t uLine, size_t num, size_t size ) throw ( );
        void dllocMem( const char *szFile, size_t uLine, void *ptr ) throw ( );
    };
}

inline void *operator new( size_t count ) throw( )
{
    return ::Ogre::MemoryManager::sMemManager.allocMem( "", 0, count );
}

inline void *operator new[]( size_t count ) throw( )
{    
    return ::Ogre::MemoryManager::sMemManager.allocMem( "", 0, count );
}

inline void operator delete( void* ptr ) throw( )
{
    ::Ogre::MemoryManager::sMemManager.dllocMem( "", 0, ptr );
}

inline void operator delete[]( void* ptr ) throw( )
{
    ::Ogre::MemoryManager::sMemManager.dllocMem( "", 0, ptr );
}

#define new new
#define delete delete
#define malloc( sz ) ::Ogre::MemoryManager::sMemManager.allocMem( __FILE__, __LINE__, sz )
#define free( ptr ) ::Ogre::MemoryManager::sMemManager.dllocMem( __FILE__, __LINE__, ptr )
#define realloc( ptr, sz ) ::Ogre::MemoryManager::sMemManager.rllocMem( __FILE__, __LINE__, ptr, sz )
#define calloc( cnt, sz ) ::Ogre::MemoryManager::sMemManager.cllocMem( __FILE__, __LINE__, cnt, sz )

#endif

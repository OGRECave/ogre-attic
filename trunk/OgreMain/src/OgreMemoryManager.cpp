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

#include "OgrePrerequisites.h"

#undef new
#undef delete
#undef malloc
#undef calloc
#undef realloc
#undef free

namespace Ogre
{
    MemoryManager MemoryManager::sMemManager;        
    
    MemoryManager::MemoryManager()
    {        
    }

    MemoryManager::~MemoryManager()
    {                
    }

    void MemoryManager::setAllocNewInfo( const char *szFile, size_t uLine )
    {        
    }

    void MemoryManager::setDeallocNewInfo( const char *szFile, size_t uLine )
    {        
    }

    void * MemoryManager::allocMem( const char *szFile, size_t uLine, size_t count )
    {
        void *ptr = malloc( count );
        return ptr;
    }        
    void * MemoryManager::rllocMem( const char *szFile, size_t uLine, void *ptr , size_t count )
    {
        void *nptr = realloc( ptr, count );
        return nptr;
    }
    void * MemoryManager::cllocMem( const char *szFile, size_t uLine, size_t num, size_t size )
    {
        void *ptr = malloc( num * size );

        if( ptr )
        {
            memset( ptr , 0, num * size );
        }
        return ptr;
    }
    void MemoryManager::dllocMem( const char *szFile, size_t uLine, void *ptr )
    {
        free( ptr );
    }

}

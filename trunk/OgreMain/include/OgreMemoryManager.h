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

BEGIN_OGRE_NAMESPACE

/** Class that handles memory (de)allocation requests.
    @remarks
        This class works like a wrapper between the actual C memory allocation 
        functions (*alloc, free) and the memory (de)allocation requests of the
        application.
    @par
        Why would such a class be needed? First of all, because we had some 
        major issues with memory getting misued (read: deleted) over DLL 
        boundaries. One thing this memory manager does is solve the problem by
        allocating all the memory in the OgreMain.dll/so process.
    @par
        Another use would be leak detection and memory misuse detection. With
        a custom memory manager, calls to new/delete and *alloc/free could be
        overseed and logged.
    @par
        Yet another use is the optimization of memory allocation for certain
        object types. One of the most common examples is a small object 
        allocator.
    @note
        The class contains a static member of type MemoryManager. That is 
        because we want the memory manager to be created even before we 
        override the new([])/delete([]) operators.
*/
class _OgreExport MemoryManager
{
public:        
    static MemoryManager sMemManager;

public:
    MemoryManager();
    ~MemoryManager();

    /** Memory allocator - uses plain old malloc.
    */
    void *allocMem( const char *szFile, size_t uLine, size_t count ) throw ( );

    /** Memory re-allocator - uses plain old realloc.
    */
    void *rllocMem( const char *szFile, size_t uLine, void *ptr , size_t count ) throw ( );

    /** Memory allocator - uses plain old calloc.
    */
    void *cllocMem( const char *szFile, size_t uLine, size_t num, size_t size ) throw ( );

    /** Memory de-allocator - uses plain old free.
    */
    void dllocMem( const char *szFile, size_t uLine, void *ptr ) throw ( );
};

END_OGRE_NAMESPACE

//-----------------------------------------------------------------------------
// Overridden global new([])/delete([]) functions
//
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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This header adds the *alloc/free macros, wrapping the C functions
#include "OgreMemoryMacros.h"
//-----------------------------------------------------------------------------

#endif

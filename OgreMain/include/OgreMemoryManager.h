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

namespace Ogre {

#if OGRE_DEBUG_MEMORY_MANAGER

#ifndef __FUNCTION__
#define __FUNCTION__ "???"
#endif

typedef struct tag_au
{
    size_t actualSize;
    size_t reportedSize;

    void *actualAddress;
    void *reportedAddress;

    char sourceFile[40];
    char sourceFunc[40];

    unsigned int sourceLine;
    unsigned int allocationType;

    bool breakOnDealloc;
    bool breakOnRealloc;

    unsigned int allocationNumber;
    unsigned int processID;

    struct tag_au *next;
    struct tag_au *prev;
} sAllocUnit;

typedef struct
{
    unsigned int totalReportedMemory;
    unsigned int totalActualMemory;

    unsigned int peakReportedMemory;
    unsigned int peakActualMemory;

    unsigned int accumulatedReportedMemory;
    unsigned int accumulatedActualMemory;
    unsigned int accumulatedAllocUnitCount;

    unsigned int totalAllocUnitCount;
    unsigned int peakAllocUnitCount;
} sMStats;

// ---------------------------------------------------------------------------------------------------------------------------------
// Defaults for the constants & s in the MemoryManager class
// ---------------------------------------------------------------------------------------------------------------------------------

enum
{
    m_alloc_unknown        = 0,
    m_alloc_new            = 1,
    m_alloc_new_array      = 2,
    m_alloc_malloc         = 3,
    m_alloc_calloc         = 4,
    m_alloc_realloc        = 5,
    m_alloc_delete         = 6,
    m_alloc_delete_array   = 7,
    m_alloc_free           = 8
};

struct _OgreExport MemoryManager
{
    static MemoryManager sMemManager;

    // This is used to set process id's
    unsigned m_uProcessIDs;
    bool m_bDeinitTime;

    MemoryManager();
    ~MemoryManager();

    //-------------------------------------------------------------------------
    // Used by the macros
    //-------------------------------------------------------------------------
    void setOwner(const char *file, const unsigned int line, const char *func);

    //-------------------------------------------------------------------------
    // Allocation breakpoints
    //-------------------------------------------------------------------------
    bool &breakOnRealloc(void *reportedAddress);
    bool &breakOnDealloc( void *reportedAddress );
    void breakOnAlloc( unsigned int count );

    //-------------------------------------------------------------------------
    // The meat of the memory tracking software
    //-------------------------------------------------------------------------

    void * allocMem(
        const char *sourceFile, 
        const unsigned int sourceLine, 
        const char *sourceFunc,
        const unsigned int allocationType, 
        const size_t reportedSize, 
        const unsigned processID );
    void * rllocMem(
        const char *sourceFile, 
        const unsigned int sourceLine, 
        const char *sourceFunc,
        const unsigned int reallocationType, 
        const size_t reportedSize, 
        void *reportedAddress, 
        const unsigned processID );
    void dllocMem(
        const char *sourceFile, 
        const unsigned int sourceLine, 
        const char *sourceFunc,
        const unsigned int deallocationType, 
        const void *reportedAddress, 
        const unsigned processID );

    //-------------------------------------------------------------------------
    // Utilitarian functions
    //-------------------------------------------------------------------------

    bool validateAddr(const void *reportedAddress);
    bool validateAlloc(const sAllocUnit *allocUnit);
    bool validateAllAllocs();

    //-------------------------------------------------------------------------
    // Unused RAM calculations
    //-------------------------------------------------------------------------

    unsigned int calcUnused( const sAllocUnit *allocUnit );
    unsigned int calcAllUnused();

    //-------------------------------------------------------------------------
    // Logging and reporting
    //-------------------------------------------------------------------------

    void dumpAllocUnit( const sAllocUnit *allocUnit, const char *prefix = "" );
    void dumpMemReport( const char *filename = "memreport.log", const bool overwrite = true );
    sMStats getMemStats();    

    unsigned getProcessID()
    {
        return ++m_uProcessIDs;
    }

    //-------------------------------------------------------------------------
    // Wrappers for the new/delete functions
    //-------------------------------------------------------------------------
    void *op_new_sc( size_t reportedSize, unsigned processID );
    void *op_new_vc( size_t reportedSize, unsigned processID );

    void *op_new_sc( size_t reportedSize, const char *sourceFile, int sourceLine, unsigned processID );
    void *op_new_vc( size_t reportedSize, const char *sourceFile, int sourceLine, unsigned processID );

    void op_del_sc( void *reportedAddress, unsigned processID );
    void op_del_vc( void *reportedAddress, unsigned processID );
};

}

static unsigned gProcessID = 0;

//-----------------------------------------------------------------------------
// Overridden global new([])/delete([]) functions
//
inline void *operator new(size_t reportedSize)
{
    if( !gProcessID )
        gProcessID = Ogre::MemoryManager::sMemManager.getProcessID();
    return Ogre::MemoryManager::sMemManager.op_new_sc( reportedSize, gProcessID );
}
inline void *operator new[](size_t reportedSize)
{
    if( !gProcessID )
        gProcessID = Ogre::MemoryManager::sMemManager.getProcessID();
    return Ogre::MemoryManager::sMemManager.op_new_vc( reportedSize, gProcessID );
}

inline void operator delete(void *reportedAddress)
{
    Ogre::MemoryManager::sMemManager.op_del_sc( reportedAddress, gProcessID );    
}
inline void operator delete[](void *reportedAddress)
{
    Ogre::MemoryManager::sMemManager.op_del_vc( reportedAddress, gProcessID );
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This header adds the *alloc/free macros, wrapping the C functions
#include "OgreMemoryMacros.h"
//-----------------------------------------------------------------------------

#else

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
struct _OgreExport MemoryManager
{
    static MemoryManager sMemManager;

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

}

//-----------------------------------------------------------------------------
// Overridden global new([])/delete([]) functions
//
inline void *operator new( size_t count )
{
    return Ogre::MemoryManager::sMemManager.allocMem( "", 0, count );
}

inline void *operator new[]( size_t count )
{    
    return Ogre::MemoryManager::sMemManager.allocMem( "", 0, count );
}

inline void operator delete( void* ptr )
{
    Ogre::MemoryManager::sMemManager.dllocMem( "", 0, ptr );
}

inline void operator delete[]( void* ptr )
{
    Ogre::MemoryManager::sMemManager.dllocMem( "", 0, ptr );
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This header adds the *alloc/free macros, wrapping the C functions
#include "OgreMemoryMacros.h"
//-----------------------------------------------------------------------------

#endif

#endif

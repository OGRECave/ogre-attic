/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
//---- ORIGINAL COPYRIGHT FOLLOWS -------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------------------
// Copyright 2000, Paul Nettle. All rights reserved.
//
// You are free to use this source code in any commercial or non-commercial product.
//
// mmgr.cpp - Memory manager & tracking software
//
// The most recent version of this software can be found at: ftp://ftp.GraphicsPapers.com/pub/ProgrammingTools/MemoryManagers/
//
// [NOTE: Best when viewed with 8-character tabs]
//
// ---------------------------------------------------------------------------------------------------------------------------------
#ifndef __MemoryManager_H__
#define __MemoryManager_H__

#include "OgrePlatform.h"
#include "OgreStdHeaders.h"

//#define OGRE_USE_OLD_MEMMAN

#ifdef OGRE_USE_OLD_MEMMAN // Turn off/on old memory system
namespace Ogre {

    /** @page memory_manager The memory manager information page
	
		@subsection desc Description
			The memory manager is a class that handles memory (de)allocation requests.
        @par
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
		@subsection types Manager types
			There actually are two classes, one is the standard memory manager
			which only addresses memory allocation problems when deallocating
			across processes. 
		@par
			The other is a modified version of the debugging memory manager written 
			by Paul 'MidNight' Nettle (aka. the Fluid Studios Memory Manager). 
			Obviously, the second one should be used only when debugging your 
			application as it adds some visible overhead.
		@par
			You can switch between the two memory managers by setting the value of
			the OGRE_DEBUG_MEMORY_MANAGER macro in OgreConfig.h
        @subsection notes Implementation Note
            The class contains a static member of type MemoryManager. That is 
            because we want the memory manager to be created even before we 
            override the new([])/delete([]) operators.
		@subsection see See also
			<a href="http://www.flipcode.com/cgi-bin/msg.cgi?showThread=12September2000-PresentingAMemoryManager&forum=askmid&id=-1">Paul Nettle's Memory Manager page at flipCode</a> - you can get the original source form here.
    */

#if OGRE_DEBUG_MEMORY_MANAGER && OGRE_DEBUG_MODE

#ifndef __FUNCTION__
#define __FUNCTION__ "???"
#endif

}

//-----------------------------------------------------------------------------
// We have to declare the global new([])/delete([]) operators before declaring 
// the Ogre::MemoryManager class since it lists them as friend functions
void *operator new(size_t reportedSize);
void *operator new[](size_t reportedSize);
void operator delete(void *reportedAddress);
void operator delete[](void *reportedAddress);
//-----------------------------------------------------------------------------

namespace Ogre {

    /** For internal use only.
        \internal.
        @remarks
            This structure holds the allocation tracking information. So,
            for each allocation made, the overhead this memory manager adds
            is the size of this structure, the lengths of the names of the
            file and function in which the allocation was made and the
            padding size (which can be adjusted).
    */
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
        size_t totalReportedMemory;
        size_t totalActualMemory;

        size_t peakReportedMemory;
        size_t peakActualMemory;

        size_t accumulatedReportedMemory;
        size_t accumulatedActualMemory;
        size_t accumulatedAllocUnitCount;

        size_t totalAllocUnitCount;
        size_t peakAllocUnitCount;
    } sMStats;
    
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

	/** See the \ref memory_manager.
	*/
    class _OgreExport MemoryManager
    {
        friend void * ::operator new(size_t);
        friend void * ::operator new[](size_t);
        friend void ::operator delete(void*);
        friend void ::operator delete[](void*);

    public:
        static MemoryManager& instance(void);

    private:
        /// This is used in the process tracking part of the memory manager.
        unsigned m_uProcessIDs;
        /// This is set to true when deinitialization takes place.
        bool m_bDeinitTime;

#ifndef __BORLANDC__
    private:
#else
    public:
#endif
        //-------------------------------------------------------------------------
        // Wrappers for the new/delete functions        
        void *op_new_sc( size_t reportedSize, unsigned processID );
        void *op_new_vc( size_t reportedSize, unsigned processID );

        void *op_new_sc( size_t reportedSize, const char *sourceFile, int sourceLine, unsigned processID );
        void *op_new_vc( size_t reportedSize, const char *sourceFile, int sourceLine, unsigned processID );

        void op_del_sc( void *reportedAddress, unsigned processID );
        void op_del_vc( void *reportedAddress, unsigned processID );
        //-------------------------------------------------------------------------

        /** This function is intended for internal use only.
            \internal
            @remarks
                This function is used to return an unique handle for each process 
                calling it. The returned unsigned int is then passed to the memory
                manager every time a re/de/allocation request is made, in order
                to check that deallocations don't occur in processes other than the 
                ones in which allcations were made and so on.
            @par
                Actually, the problem of re/de/allocating in other processes was
                solved with the addition of the new memory manager, but you may
                want to limit the occurence of such events anyway, and this function
                helps you do just that.
        */
        unsigned _getProcessID();

    public:
        MemoryManager();
        ~MemoryManager();

        //-------------------------------------------------------------------------
        // Used by the macros     
        void setOwner(const char *file, const unsigned int line, const char *func);
        //-------------------------------------------------------------------------

        //-------------------------------------------------------------------------
        // Allocation breakpoints        
        bool &breakOnRealloc(void *reportedAddress);
        bool &breakOnDealloc( void *reportedAddress );
        void breakOnAlloc( unsigned int count );
        //-------------------------------------------------------------------------

        //-------------------------------------------------------------------------
        // The meat & potatoes of the memory tracking software

        /** This function is intended for internal use only.
            \internal
            @remarks
                This function is the actual memory allocator and acts as a bridge
                between OGRE and the C/C++ alloc/calloc functions. 
            @par
                While memory allocation requests are made trough this function, 
                the tracking of memory addresses is possible. Therefore, attempting 
                to deallocate a portion of memory that was not allocated using 
                this function will result in a warning given by the deallocator, 
                dllocMem.
        */
        void * allocMem(
            const char *sourceFile, 
            const unsigned int sourceLine, 
            const char *sourceFunc,
            const unsigned int allocationType, 
            const size_t reportedSize, 
            const unsigned processID );

        /** This function is intended for internal use only.
            \internal
            @remarks
                This function is the actual memory reallocator and acts as a bridge
                between OGRE and the C/C++ realloc function. 
            @par
                While memory reallocation requests are made trough this function, 
                the tracking of memory addresses is possible. Therefore, attempting 
                to deallocate a portion of memory that was not reallocated using 
                this function will result in a warning given by the deallocator, 
                dllocMem. 
            @par
                As well, trying to reallocate memory that was not allocated using
                mallc/calloc will result in a warning.
        */
        void * rllocMem(
            const char *sourceFile, 
            const unsigned int sourceLine, 
            const char *sourceFunc,
            const unsigned int reallocationType, 
            const size_t reportedSize, 
            void *reportedAddress, 
            const unsigned processID );

        /** This function is intended for internal use only.
            \internal
            @remarks
                This function is the actual memory deallocator and acts as a
                bridge between OGRE and the C/C++ free function.
            @par
                While memory deallocation requests are made trough this function, 
                the tracking of memory addresses is possible. Therefore, attempting 
                to deallocate a portion of memory that was not allocated using 
                allocMem or rllocMem, trying to deallocate memory that was
                allocated with malloc using delete (and the corresponding 
                permutations) or trying to deallocate memory allocated from from
                process will result in a warning.
            @note
                Actually, memory can be allocated in one process and deallocated
                in another, since the actual (de)allocation takes place in the
                memory space of the OgreMain library. 
            @par
                Tracking this kind of (possible) errors exists because users may 
                want to write their own memory allocator later on or they'd like 
                to get rid of OGRE's memory allocator.
        */
        void dllocMem(
            const char *sourceFile, 
            const unsigned int sourceLine, 
            const char *sourceFunc,
            const unsigned int deallocationType, 
            const void *reportedAddress, 
            const unsigned processID );
        //-------------------------------------------------------------------------

        //-------------------------------------------------------------------------
        // Utilitarian functions        
        bool validateAddr(const void *reportedAddress);
        bool validateAlloc(const sAllocUnit *allocUnit);
        bool validateAllAllocs();
        //-------------------------------------------------------------------------

        //-------------------------------------------------------------------------
        // Unused RAM calculations        
        unsigned int calcUnused( const sAllocUnit *allocUnit );
        unsigned int calcAllUnused();
        //-------------------------------------------------------------------------

        //-------------------------------------------------------------------------
        // Logging and reporting        
        void dumpAllocUnit( const sAllocUnit *allocUnit, const char *prefix = "" );
        void dumpMemReport( const char *filename = "memreport.log", const bool overwrite = true );
        sMStats getMemStats();            
        //-------------------------------------------------------------------------        
    };
}

/** This variable exists separately in each module that links to the OGRE library
    and is used to track the ID of the current process from the perspective
    of the memory manager.
    @see
        unsigned Ogre::MemoryManager::_getProcessID()
*/
static unsigned gProcessID = 0;

// When compiling in Visual C++ (occuring in VS2005 Express but not for VC 7.1) with
// managed C++, should put the new([])/delete([]) overrides inside unmanaged context,
// otherwise Visual C++ will link with overridden version of new([]) and CRT version
// of delete([]), thus, mess up both of OGRE memory manager and CRT memory manager.
#if defined(__cplusplus_cli)
#pragma managed(push, off)
#endif
//-----------------------------------------------------------------------------
// Overridden global new([])/delete([]) functions
//
inline void *operator new(size_t reportedSize)
{
    if( !gProcessID )
        gProcessID = Ogre::MemoryManager::instance()._getProcessID();
    return Ogre::MemoryManager::instance().op_new_sc( reportedSize, gProcessID );
}
inline void *operator new[](size_t reportedSize)
{
    if( !gProcessID )
        gProcessID = Ogre::MemoryManager::instance()._getProcessID();
    return Ogre::MemoryManager::instance().op_new_vc( reportedSize, gProcessID );
}

inline void operator delete(void *reportedAddress)
{
    Ogre::MemoryManager::instance().op_del_sc( reportedAddress, gProcessID );    
}
inline void operator delete[](void *reportedAddress)
{
    Ogre::MemoryManager::instance().op_del_vc( reportedAddress, gProcessID );
}
//-----------------------------------------------------------------------------
#if defined(__cplusplus_cli)
#pragma managed(pop)
#endif

//-----------------------------------------------------------------------------
// This header adds the *alloc/free macros, wrapping the C functions
#include "OgreMemoryMacros.h"
//-----------------------------------------------------------------------------

#else

	/** See the \ref memory_manager.
	*/
    class _OgreExport MemoryManager
    {
    public:
        static MemoryManager& instance(void);

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

#endif

#else // Turn on/off new memory system


// OK, here we go, I've switched off the old memory system, time to replace it 
// overload new/delete to plug-in our gloabl scheme, maybe do this with macros later on
// for now I just want to direct allocations to the right place



/// overloaded operator new, points back to the 
/// allocation wrapper function
_OgreExport void *operator new(std::size_t size);

/// overloaded operator new[], points back to the 
/// allocation wrapper function
_OgreExport void *operator new[](std::size_t size);

/// overloaded operator delete, points back to the 
/// deallocation wrapper function
_OgreExport void operator delete(void *ptr, std::size_t size);

/// overloaded operator delete[], points back to the 
/// deallocation wrapper function
_OgreExport void operator delete[](void *ptr, std::size_t size);


// single param delete, I can work with this for now untill 
// I can sort out the (void*, size_t) version
// /*
/// overloaded operator delete, points back to the 
/// deallocation wrapper function
_OgreExport void operator delete(void *ptr);

/// overloaded operator delete[], points back to the 
/// deallocation wrapper function
_OgreExport void operator delete[](void *ptr);
// */

namespace Ogre
{
	class Exception;
	class MemProfileManager;

	/**
	 * This internal class forms the heart of the memory manager,
	 * all the various ogre-native allocator types grab there 
	 * memory from here. This is where we manage the process 
	 * virtual address space.
	 */	
    class _OgreExport MemoryManager
    {
    private:
		/// the "magic" value, used to check for corruption
		static const uint32 MAGIC = 0xAAAA;
		
		/// used to mask of the full bit
		static const uint32 MASK  = 0x80000000;
			
		/// min alloc size
		static const uint32 MIN_ALOC_SIZE = 8;
		
		/// size of overhead (at start of block)
		static const uint32 OVERHEAD = 16;	
		
		/// 2^31 max aloc = 4Gig
		/// bin n = 2^(n+2)
		static const uint32 NUM_BINS = 29;
    
		// this struct is added to the head and tail of a block
		// we need to add it to the head to hold allocation info.
		// We need to add it to the tail of a block to allow for
		// auto-coelessing of adjacent empty blocks. Having a 
		// block at the end allows us to detect corruption as well.
		struct MemCtrl
		{
			uint16 magic;  // magic value, for detecting corruption
			uint16 bin_id; // id of the bucket we came from 
			uint32 size;   // 31 bits used for size info, 1 bit empty flag
			
		};
		
		// this struct is used to form a linked list of empty 
		// memory blocks. Only when a block is empty is this 
		// struct used, when a block is full we use MemCtrl
		struct MemFree
		{
			MemFree* next; // next empty block in list, maybe not be adjacent 
			MemFree* prev; // prev empty block in list, maybe not be adjacent
		};
		
		// used to pass about chuncks of memory internally within the manager 
		struct MemBlock
		{
			char*  memory;
			uint32 size;
		};
		
		// bin[n] holds chunks of size 2^(n+3) smallest is 8.
		// Overhead is before and after data
        MemFree* mBin[NUM_BINS];
        
        void* mVmemStart;  // start of process virtual address space
        void* mVmemStop;   // end of process virtual address space
        uint32 mPageSize;   // system memory page size
        uint32 mRegionSize; // size of a virtual address space region (windows only)
        
    public:
        MemoryManager();
        ~MemoryManager();
        
        void init();
        
        /**
         * allocate memory from the free store. This will expand the 
         * process virtual address space and "touch" the resulting 
         * memory, the OS should map some form of phisical storage as
         * a result. Normal paging rules still apply.
         */
        void* allocMem(size_t size);// throw(Exception);
        
        /**
         * return memory to be re-used, note, this may not release the
         * memory back to the system imediatly. The manager caches 
         * some memory to be re-used later, its kinder to the OS memory
         * mapping stuff.
         */
        void purgeMem(void* ptr);
        
        /**
         * this lets us ask the manager how large a block of memory is.
         * The provided pointer should point to the start of the block 
         * and the block must have been allocated via this manager. 
         * @param pointer to stroage
         * @return size of storage or -1 on invlaid pointer
         */
        int sizeOfStorage(const void* ptr);// throw(Exception);
        
        /// @return static instance of MemoryManager
        static inline MemoryManager& getSingleton()
        {
        	return smInstance;
        }
        
    private:
    	MemBlock moreCore(uint32 id, uint32 size);
    	void distributeCore(MemBlock& block);
        static MemoryManager smInstance;	
        
        bool mInited;
        
    };
}

#endif // Turn on/off new memory system

#endif // include protect


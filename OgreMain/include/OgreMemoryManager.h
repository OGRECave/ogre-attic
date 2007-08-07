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

#ifndef __MemoryManager_H__
#define __MemoryManager_H__

#include "OgrePlatform.h"
#include "OgreStdHeaders.h"
#include <memory>

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

    /**
     * This internal class forms the heart of the memory manager,
     * all the various ogre-native allocator types grab there 
     * memory from here. This is where we manage the process 
     * virtual address space.
     */
    class MemoryManager
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
                uint32 size;   // 31 bits used for size info, 1 bit empty flag
                uint16 magic;  // magic value, for detecting corruption
                uint16 bin_id; // id of the bucket we came from
            };

            // this struct is used to form a linked list of empty
            // memory blocks. Only when a block is empty is this
            // struct used, when a block is full we use MemCtrl
            struct MemFree
            {
                uint32 size;   // 31 bits used for size info, 1 bit empty flag
                MemFree* next; // next empty block in list, maybe not be adjacent
                MemFree* prev; // prev empty block in list, maybe not be adjacent
            };

            // used to pass about chuncks of memory internally within the manager
            struct MemBlock
            {
                uint32 size;   // size of memory chunk
                char*  memory; // start of memory chunk
            };

            /// internal class used to house the bin functionality
            class Bin
            {
                private:
                    MemFree* mHeadPtr;
                    uint32   mSize;
                    uint32   mIndex;

                public:
                    inline explicit Bin() // cant use init list, crazy g++ f00 :(
                    //: mHeadPtr(NULL)
                    //, mSize(0)
                    //, mIndex(0)
                    {}

                    inline ~Bin()
                    {}

                    /// @return pop a chunk off the top of the bin
                    inline MemFree* pop()
                    {
                        assert(mHeadPtr);
                        MemFree* tmp = mHeadPtr;
                        mHeadPtr = mHeadPtr->next;

                        if(mHeadPtr)
                            mHeadPtr->prev = NULL;

                        return tmp;
                    }

                    /// push a chunk onto the bin
                    /// @param memFree free memory chunk
                    inline void push(MemFree* memFree)
                    {
                        memFree->size = mSize;
                        memFree->prev = NULL;
                        memFree->next = mHeadPtr;

                        if(mHeadPtr)
                            mHeadPtr->prev = memFree;

                        mHeadPtr = memFree;
                    }

                    /// @return true if the bin is depleated
                    inline bool empty()
                    {
                        return (mHeadPtr==NULL);
                    }

                    /// @param memFree memory chunk to remove from the bin
                    inline void remove(MemFree* memFree)
                    {
                        if (memFree->next)
                            memFree->next->prev = memFree->prev;

                        if (memFree->prev)
                            memFree->prev->next = memFree->next;
                        else
                            mHeadPtr = mHeadPtr->next;
                    }

                    /// @return the bins size (width)
                    inline uint32 size()
                    {
                        return mSize;
                    }

                    /// setup the bin
                    /// @param size the bins size
                    inline void setup(uint32 idx, uint32 size)
                    {
                        mIndex = idx;
                        mSize = size;
                        mHeadPtr = NULL;
                        std::cout << "setup bin " << mIndex << " size " << mSize << std::endl;
                    }

                    /// dump the bins internal list
                    void dumpInternals()
                    {
                        MemFree* tmp = mHeadPtr;
                        std::cout << " Bin index " << mIndex << " size " << mSize << std::endl;
                        while(tmp)
                        {
                            std::cout << " block " << (void*)tmp << " size " << tmp->size << std::endl;
                            tmp=tmp->next;
                        }
                    }
            };

            // bin[n] holds chunks of size 2^(n+3) smallest is 8.
            // Overhead is before and after data
            static Bin      mBin[NUM_BINS];
            static Bin      m24ByteBin;
            void*    mVmemStart;  // start of process virtual address space
            void*    mVmemStop;   // end of process virtual address space
            uint32   mPageSize;   // system memory page size
            uint32   mRegionSize; // size of a virtual address space region (windows only)

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
            void* allocMem ( size_t size ) throw ( std::bad_alloc );

            /**
             * return memory to be re-used, note, this may not release the
             * memory back to the system imediatly. The manager caches 
             * some memory to be re-used later, its kinder to the OS memory
             * mapping stuff.
             */
            void purgeMem ( void* ptr ) throw ( std::bad_alloc );

            /**
             * this lets us ask the manager how large a block of memory is.
             * The provided pointer should point to the start of the block 
             * and the block must have been allocated via this manager. 
             * @param pointer to stroage
             * @return size of storage or -1 on invlaid pointer
             */
            int sizeOfStorage ( const void* ptr ) throw ( std::bad_alloc );

            /// @return static instance of MemoryManager
            static
                    inline MemoryManager& getSingleton()
            {
                return smInstance;
            }

            void dumpInternals();

        private:

            MemBlock moreCore ( uint32 id, uint32 size );
            void distributeCore ( MemBlock& block );

            static MemoryManager smInstance;

            bool mInited;
    };
}

#endif

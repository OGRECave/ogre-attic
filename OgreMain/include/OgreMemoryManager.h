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
#include "OgreMemoryRegion.h"
#include <memory>

namespace Ogre{

    /**
     * This internal class forms the heart of the memory manager,
     * all the various ogre-native allocator types grab there 
     * memory from here. This is where we manage the process 
     * virtual address space.
     */
    class _OgreExport MemoryManager
    {
        private:
            /// min alloc size
            static const uint32 MIN_ALOC_SIZE = 8;
            /// size of overhead (at start of block)
            static const uint32 OVERHEAD = 16;

            void setup() throw (std::exception);

        public:
            inline explicit MemoryManager(){}
            inline ~MemoryManager(){}

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
            size_t sizeOfStorage ( const void* ptr ) throw ( std::bad_alloc );

            /// dump memory manager internal info
            void dumpInternals();

            /// @return static instance of MemoryManager
            static inline MemoryManager& getSingleton()
            {
                return smInstance;
            }

        private:
            static MemoryManager smInstance;
            bool   mSetup;
            uint32 mPageSize;
            uint32 mNumRegions;

            // an array of memory regions
            MemoryRegion** mRegion;
    };
}

_OgreExport void doDelete(void*);
_OgreExport void* doNew(size_t);

/*
void *operator new(std::size_t size);
void *operator new[](std::size_t size);
void operator delete(void *ptr);
void operator delete[](void *ptr);
*/

/*
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
# define new(a) doNew(a)
# define delete(a) doDelete(a)
#endif
*/

#endif


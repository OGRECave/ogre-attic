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

#ifndef OGREOGREMEMORYBIN_H
#define OGREOGREMEMORYBIN_H

#include "OgrePlatform.h"
#include "OgreStdHeaders.h"

#include <iostream>
#include <cstddef>
#include <cassert>
#include <memory>

namespace Ogre
{
    /// This struct is added to the head and tail of a block
    /// we need to add it to the head to hold allovoid cation info.
    /// We need to add it to the tail of a block to allow for
    /// auto-coelessing of adjacent empty blocks. Having a
    /// block at the end allows us to detect corruption as well.
    struct _OgreExport MemCtrl
    {
        size_t size;   // 31 bits used for size info, 1 bit empty flag
        uint16 magic;  // magic value, for detecting corruption
        uint16 reg_id; // index of the region we came from
    };

    /// This struct is used to form a linked list of empty
    /// memory blocks. Only when a block is empty is this
    /// struct used, when a block is full we use MemCtrl
    struct _OgreExport MemFree
    {
        size_t   size; // 31 bits used for size info, 1 bit empty flag
        MemFree* next; // next empty block in list, maybe not be adjacent
        MemFree* prev; // prev empty block in list, maybe not be adjacent
    };

    /// used to pass about chuncks of memory internally within the manager
    struct _OgreExport  MemBlock
    {
        size_t size;   // size of memory chunk
        char*  memory; // start of memory chunk
    };

    /// a memory bin or bucket, this is used by the memory manager to 
    /// maintain free blocks of memory for (re)use.
    class _OgreExport MemoryBin
    {
        private:
            MemFree* mHeadPtr;
            uint32   mSize;
            uint32   mIndex;

        public:
            /// the "magic" value, used to check for corruption
            static const uint32 MAGIC = 0xAAAA;
            /// used to mask of the full bit
            static const uint32 MASK  = 0x80000000;

            inline explicit MemoryBin()
            {}

            inline ~MemoryBin()
            {}

            /// @return pop a chunk off the top of the bin
            inline MemFree* pop()
            {
                assert (mHeadPtr);
                MemFree* tmp = mHeadPtr;
                mHeadPtr = mHeadPtr->next;

                if (mHeadPtr)
                    mHeadPtr->prev = NULL;

                return tmp;
            }

            /// push a chunk onto the bin
            /// @param memFree free memory chunk
            inline void push (MemFree* memFree)
            {
                memFree->size = mSize;
                memFree->prev = NULL;
                memFree->next = mHeadPtr;

                if (mHeadPtr)
                    mHeadPtr->prev = memFree;

                mHeadPtr = memFree;
            }

            /// @return true if the bin is depleated
            inline bool empty()
            {
                return (mHeadPtr == NULL);
            }

            /// @param memFree memory chunk to remove from the bin

            inline void remove (MemFree* memFree)
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
            inline void setup (uint32 idx, uint32 size)
            {
                mIndex = idx;
                mSize = size;
                mHeadPtr = NULL;
            }

            /// dump the bins internal list of chunks
            void dumpInternals();
    };

}

#endif

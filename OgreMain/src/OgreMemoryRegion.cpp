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

#include "OgreStableHeaders.h"
#include "OgreMemoryRegion.h"
#include "OgreMemProfileManager.h"

void * Ogre::MemoryRegion::allocMem(size_t size) throw( std :: bad_alloc )
{
    // find a bin index
    register uint32 idx = 0;      // bins start at 0
    register uint32 shiftVal = 8; // the first bin val = 2^3 = 8

    while (idx < NUM_BINS)
    {
        if (shiftVal >= size && !mBin[idx].empty())
            break;

        shiftVal <<= 1;
        ++idx;
    }

    if(idx == NUM_BINS)
        throw(std::bad_alloc()); // too big, check canSatisfy() first!

    // do allocation
    MemCtrl* ret;
    MemCtrl* tmp;
    MemBlock memBlock;
    memBlock.memory = (char*) mBin[idx].pop();
    memBlock.size = shiftVal;

    if (memBlock.size - size == 8)
        size += 8; // just use the entire thing, we cant make any savings

    // build the header block
    ret = (MemCtrl*) memBlock.memory;
    ret->size   = size ^ MemoryBin::MASK;
    ret->reg_id = mIndex;
    ret->magic  = MemoryBin::MAGIC;

    //TODO: only build tail block in debug
    
    // build the tail block
    tmp = (MemCtrl*) (memBlock.memory + (size - sizeof (MemCtrl) ) );
    tmp->size   = size ^ MemoryBin::MASK;
    tmp->reg_id = mIndex;
    tmp->magic  = MemoryBin::MAGIC;

    // distribute any remainder
    memBlock.size -= size;
    if (memBlock.size)
    {
        memBlock.memory += size;
        distributeCore (memBlock);
    }

    // return the result trimmed of the header
    return (void*) ( (char*) ret + sizeof (MemCtrl) );
}

void Ogre::MemoryRegion::purgeMem(MemCtrl* ptr) throw( std :: bad_alloc )
{
    MemBlock memBlock;
    memBlock.memory = (char*)ptr;
    memBlock.size = ptr->size ^ MemoryBin::MASK; // get the size

    // get the head block magic value
    if(ptr->magic != MemoryBin::MAGIC)
    {
        MemProfileManager::getSingleton() << "Bad pointer passed to purgeMem(), double delete or memory corruption";
        dumpInternals();
        throw std::bad_alloc();
    }

    // get the tail block magic value
    if(((MemCtrl*)(memBlock.memory+(memBlock.size-sizeof(MemCtrl))))->magic != MemoryBin::MAGIC)
    {
        MemProfileManager::getSingleton() << "purgeMemory(), Memory corruption detected";
        dumpInternals();
        throw std::bad_alloc();
    }

    // coalesce memory
    //-----------------------------------------------------------------
    // /*
    MemFree* memFree;
    register uint32 size;

    while (memBlock.memory + memBlock.size != mPool+POOL_SIZE)
    {
        memFree = (MemFree*) (memBlock.memory + memBlock.size);
        size = memFree->size;
        assert(size);

        if (size & MemoryBin::MASK) // the block is full, terminate run
        {
            break;
        }
        else // the block is empty, coalesce it
        {
            memBlock.size += size;

            // stitch up the bin
            if(size == 24)
                m24ByteBin.remove(memFree);
            else
            {
                for(uint32 i =0;i < NUM_BINS;++i)
                {
                    if(mBin[i].size() == size)
                    {
                        mBin[i].remove(memFree);
                        break;
                    }
                }
            }
        }
    }
    // */
    //-----------------------------------------------------------------

    distributeCore(memBlock);
}

void Ogre::MemoryRegion::dumpInternals()
{
    for(uint32 i=0;i<NUM_BINS;++i)
        mBin[i].dumpInternals();
    MemProfileManager::getSingleton() << "\n24 byte bin\n";
    m24ByteBin.dumpInternals();

    MemProfileManager::getSingleton() << "\nDummping Map:-\n";
    MemFree* memFree;
    MemBlock memBlock;
    memBlock.memory=mPool;
    memBlock.size=0;
    uint32 size=0;

    while(memBlock.memory + memBlock.size != mPool+POOL_SIZE)
    {
        memFree = (MemFree*)(memBlock.memory + memBlock.size);
        size = memFree->size;

        if(size & MemoryBin::MASK)
        {
            MemProfileManager::getSingleton() << "1 : " 
                    << (void*)(memBlock.memory + memBlock.size) << " : " << (size^MemoryBin::MASK) << "\n";
            memBlock.size += (size^MemoryBin::MASK);

            MemCtrl* memCtrl = (MemCtrl*)memFree;
            assert(memCtrl->magic == MemoryBin::MAGIC);

            memCtrl = (MemCtrl*)((char*)(memFree)+(size^MemoryBin::MASK) - sizeof(MemCtrl));
            assert(memCtrl->magic == MemoryBin::MAGIC);
        }
        else
        {
            MemProfileManager::getSingleton() << "0 : " 
                    << (void*)(memBlock.memory + memBlock.size) << " : " << size << "\n";
            memBlock.size+=size;
        }
        assert(size);
    }
}

bool Ogre::MemoryRegion::canSatisfy(uint32 size)
{
    assert(size < POOL_SIZE); 
    for(int i=NUM_BINS-1;i>=0;--i)
    {
        if(mBin[i].size() < size) // all the rest are too small
            return false;

        if(!mBin[i].empty()) // size is good and its not empty
            return true;
    }
    return false;
}

void Ogre::MemoryRegion::setup(uint32 index)
{
    // setup the bins
    uint32 sz = (1 << 3);
    for(uint32 i=0;i<NUM_BINS;++i)
    {
        mBin[i].setup(i,sz);
        sz <<= 1;
    }
    m24ByteBin.setup(0,24);

    // prime the bins
    mBin[NUM_BINS-1].push((MemFree*)mPool);

    mIndex=index;
}

void Ogre::MemoryRegion::distributeCore(MemBlock block)
{
    // we should never have to deal with a block this small
    assert (block.size > 8 && "Block size too small");

    uint32 shiftVal = (1 << NUM_BINS + 2);
    int idx = NUM_BINS - 1;

    // while we have memory to work for, distribute it
    while (idx >= 0 && block.size)
    {
        // There is one problem with allowing this algorithm to
        // naturally terminate. In some cases an 8 byte block is
        // created and needs to be stored, unfortunatly we need
        // at least 12 bytes to hold the header of an un-allocated
        // memory block. So I terminate the algorithm early and
        // allow an out of sequence 24 byte bin to hold the block
        // that would otherwise form one 16 and one 8 byte block
        if (block.size == 24)
        {
            m24ByteBin.push((MemFree*)block.memory);
            block.size = 0;
            break;  // induced termination
        }

        // general case distribution
        if (shiftVal <= block.size)
        {
            if (block.size - shiftVal != 8) // force 24 byte handling
            {
                mBin[idx].push((MemFree*)block.memory);
                block.size -= shiftVal;
                block.memory += shiftVal;
            }
        }
        --idx;
        shiftVal >>= 1;
    }

    // there should never be any left over
    assert (!block.size && "Orphan memory chunck");
}


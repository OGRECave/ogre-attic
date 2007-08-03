/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as 
published by the Free Software Foundation; either version 2.1 of the 
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
License for more details.

You should have received a copy of the GNU Lesser General Public License 
along with this library; if not, write to the Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/
#include "OgreStableHeaders.h"
#include "OgreAllocator.h"
#include "OgreLogManager.h"
#include "OgreAllocator.h"
#include "OgreException.h"

#include "OgreMemoryManager.h"

#include <iostream>
#include <cassert>

// just a handy debugging macro
#define DBG_PROBE(a) std::cout << #a << " : " << a << std::endl;

// OS specific includes
#ifdef WIN32
# include <windows.h>
#elif defined(__GNUC__)
# include <unistd.h>    // sysconf(3)
# include <sys/mman.h>      // mmap(2)
# include <errno.h>
# define USE_MMAP 1
#endif

// allocator 
//static Ogre::Allocator<unsigned char>  sAllocator;


// realise static instance member for the init class
Ogre::MemoryManager Ogre::MemoryManager::smInstance;

Ogre::MemoryManager::MemoryManager()
{
	if(!mInited)
    	init();
}

Ogre::MemoryManager::~MemoryManager()
{}

void Ogre::MemoryManager::init()
{
    mInited = true;
    
 #ifdef __GNUC__
    mPageSize = sysconf(_SC_PAGESIZE); //_SC_PAGE_SIZE
    mVmemStart = mVmemStop = sbrk(0);
    mVmemStart=sbrk((uint32)(mVmemStart)%mPageSize);
    MemProfileManager::getSingleton() << "UNIX/Linux (GNU) Page size is " << mPageSize << " bytes " << mVmemStart << "\n";
#endif
    
    // setup bins
    memset(mBin,0,sizeof(MemFree*)*NUM_BINS);
    m24ByteBin = NULL;
}

void* Ogre::MemoryManager::allocMem(size_t size) throw(std::bad_alloc)
{
	if(!mInited)
		init();
	
#ifdef __GNUC__

    MemBlock memBlock;
    MemCtrl* ret;
    MemCtrl* tmp;

    size += OVERHEAD; // pad up to min aloc size
    size += MIN_ALOC_SIZE-(size%MIN_ALOC_SIZE);

    // find a bin index
    register uint32 idx=0;      // bins start at 0
    register uint32 shiftVal=8; // the first bin val = 2^3 = 8
    while(idx<NUM_BINS)
    {
        if(shiftVal>=size)
            break;
        shiftVal <<= 1;
        ++idx;
    }

    // alloc memory from the bin
    if(idx==NUM_BINS)
    {
        //MemProfileManager::getSingleton() << "ERROR: asked to alloc a bad size (sz>4G)!";
        throw std::bad_alloc();
        return NULL;
    }
    else
    {
        if(mBin[idx]==NULL) // add more memory
        {
            memBlock = moreCore(idx,shiftVal);
        }
        else // alloc from bin
        {
		    memBlock.memory = (char*)mBin[idx];
            memBlock.size = shiftVal;
            mBin[idx] = mBin[idx]->next;
            if(mBin[idx])
                mBin[idx]->prev = NULL;
        }

        if(memBlock.size - size ==8)
            size+=8; // just use the entire thing, we cant make any savings

        // build the header block
        ret = (MemCtrl*)memBlock.memory;
        ret->size   = size^MASK;
        ret->bin_id = idx;
        ret->magic  = MAGIC;

        // build the tail block
        tmp = (MemCtrl*)(memBlock.memory + (size - sizeof(MemCtrl)));
        tmp->size   = size^MASK;
        tmp->bin_id = idx;
        tmp->magic  = MAGIC;

        // distribute any remainder
        memBlock.size -= size;
        if(memBlock.size)
        {
            memBlock.memory += size;
            distributeCore(memBlock);
        }

        // return the result trimmed of the header
        register void* value = (void*)((char*)ret+sizeof(MemCtrl));
        assert(value < mVmemStop);
        return value;
    }

#elif defined(WIN32)
    return malloc(size);
#endif
}

void Ogre::MemoryManager::purgeMem(void* ptr) throw(std::bad_alloc)
{
#ifdef __GNUC__
	assert(ptr < mVmemStop && "BAD POINTER");


    MemBlock memBlock;
    memBlock.memory = (char*)ptr-sizeof(MemCtrl);
    memBlock.size = ((MemCtrl*)memBlock.memory)->size^MASK; // get the size

    // get the head block magic value
    if(((MemCtrl*)memBlock.memory)->magic != MAGIC)
    {
        MemProfileManager::getSingleton() << "Bad pointer passed to purgeMem(), double delete or memory corruption";
        throw std::bad_alloc();
    }

    // get the tail block magic value
    if(((MemCtrl*)(memBlock.memory+(memBlock.size-sizeof(MemCtrl))))->magic != MAGIC)
    {
        MemProfileManager::getSingleton() << "purgeMemory(), Memory corruption detected";
        throw std::bad_alloc();
    }

    // coalesce memory
    //-----------------------------------------------------------------
    // /*
    MemFree* memFree;
    register uint32 size;
    while(memBlock.memory + memBlock.size != mVmemStop)
    {   
       assert(memBlock.memory + memBlock.size < mVmemStop && "Over running VAS"); 
       memFree = (MemFree*)(memBlock.memory + memBlock.size);
       size = memFree->size;
       assert(size);
       
       if(size & MASK) // the block is full, terminate run
       {
           break;
       }
       else // the block is empty, coalesce it
       {
           memBlock.size += size;  
           
           // stitch up the bin
           if(memFree->next)
               memFree->next->prev = memFree->prev; 
           if(memFree->prev)
               memFree->prev->next = memFree->next;
           else 
           { 
               // we have depleated a bin so we need to mark it empty
               if(m24ByteBin == memFree)
               {
                   m24ByteBin=NULL;
               }
               else
               {
                   for(uint32 i=0;i<NUM_BINS;++i)
                   {
                       if(mBin[i]==memFree)
                       {
                           mBin[i]=NULL;
                           break;
                       }
                   }
               }
           }
       }
    }
    // */
    //-----------------------------------------------------------------
    
    distributeCore(memBlock);

    // TODO, restore the wilderness

#elif defined(WIN32)

    free(ptr);
#endif
}

void Ogre::MemoryManager::dumpInternals()
{
    MemBlock memBlock;
    memBlock.memory=(char*)mVmemStart;
    memBlock.size=0;
    
    uint32 size;
    MemFree* memFree;
    MemProfileManager::getSingleton() << "\nDummping Internal Table\n" <<
        "-------------------------------------------------------------\n";
    for(uint32 i=0;i<NUM_BINS;++i)
    {
        MemProfileManager::getSingleton() << "Bin index " 
            << i << " size " << (unsigned int)(1<<(i+3)) << "\n";
        memFree = mBin[i];
        while(memFree)
        {
            MemProfileManager::getSingleton() << " block " 
                << (void*)memFree << " size " << memFree->size << "\n";
            memFree = memFree->next;
        }
    }
    MemProfileManager::getSingleton() << "\n24 byte bin\n";
    memFree = m24ByteBin;
    while(memFree)
    {
        MemProfileManager::getSingleton() << " block " 
            << (void*)memFree << " size " << memFree->size << "\n";
        memFree = memFree->next;
    }
    
    std::cout << "\nDummping Memory Map\n" <<
        "-------------------------------------------------------------\n";
    while(memBlock.memory + memBlock.size != mVmemStop)
    {
        memFree = (MemFree*)(memBlock.memory + memBlock.size);
        size = memFree->size;

        if(size & MASK)
        {
            MemProfileManager::getSingleton() << "1 : " 
                << (void*)(memBlock.memory + memBlock.size) << " : " << (size^MASK) <<  "\n";
            memBlock.size += (size^MASK);

            MemCtrl* memCtrl = (MemCtrl*)memFree;
            assert(memCtrl->magic == MAGIC);

            memCtrl = (MemCtrl*)((char*)(memFree)+(size^MASK) - sizeof(MemCtrl));
            assert(memCtrl->magic == MAGIC);

        }
        else
        {
            MemProfileManager::getSingleton() << "0 : " 
                << (void*)(memBlock.memory + memBlock.size) << " : " << size << "\n";
            memBlock.size+=size;
        }
        assert(size);
    }
    MemProfileManager::getSingleton() << "-------------------------------------------------------------\n\n";
}

void Ogre::MemoryManager::distributeCore(MemBlock& block)
{
    // we should never have to deal with a block this small
    assert(block.size > 8 && "Block size too small");

    MemFree* memFree;
    uint32 shiftVal = (1 << NUM_BINS+2);
    int idx = NUM_BINS-1;

    // while we have memory to work for, distribute it
    while(idx>=0 && block.size)
    {
        // There is one problem with allowing this algorithm to
        // naturally terminate. In some cases an 8 byte block is
        // created and needs to be stored, unfortunatly we need
        // at least 12 bytes to hold the header of an un-allocated
        // memory block. So I terminate the algorithm early and
        // allow an out of sequence 24 byte bin to hold the block
        // that would otherwise form one 16 and one 8 byte block
        if(block.size == 24)
        {
            memFree = (MemFree*)block.memory;
            memFree->size = 24;
            memFree->next = m24ByteBin;
            memFree->prev = NULL;

            if(m24ByteBin)
                m24ByteBin->prev = memFree;
            m24ByteBin = memFree;

            block.size = 0;
            break;  // induced termination
        }

        // general case distribution
        if(shiftVal <= block.size) 
        {
            if(block.size - shiftVal != 8) // force 24 byte handling
            {
                memFree = (MemFree*)block.memory;
                memFree->size = shiftVal;
                memFree->next = mBin[idx];
                memFree->prev = NULL;

                if(mBin[idx])
                  mBin[idx]->prev = memFree;
                mBin[idx] = memFree;

                block.size -= shiftVal;
                block.memory += shiftVal;
            }
        }
        --idx;
        shiftVal >>= 1;
    }
    // there should never be any left over
    assert(!block.size && "Orphan memory chunck");
}

Ogre::MemoryManager::MemBlock Ogre::MemoryManager::moreCore(uint32 idx, uint32 size)
{
    MemBlock ret;

    // try to find a bigger bin
    register uint32 shiftVal = size;
    register uint32 i=idx;
    while ( !mBin[i] && i<NUM_BINS )
    {
        ++i;
        shiftVal <<= 1;
    }
    if ( i < NUM_BINS ) // split a bigger bins memory
    {
        assert(shiftVal == mBin[i]->size );
        
        ret.size = shiftVal;
        ret.memory = ( char* ) mBin[i];
        
        mBin[i] = mBin[i]->next;
        if(mBin[i])
            mBin[i]->prev = NULL;
    }
    else // we need to grab more core
    {   
        if(size<mPageSize)
            size = mPageSize;

		//std::cout << "SBRK " << sbrk(0) << " size " << size;
		
        ret.size = size;
#ifndef USE_MMAP
        ret.memory= (char*) sbrk( size );
#else
        ret.memory = (char*) mmap(mVmemStop, size, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
#endif
        
        //std::cout <<  " : " << sbrk(0) << std::endl;
        
        if(ret.memory < 0 )
        {
            MemProfileManager::getSingleton() << "Out of memory (sbrk()<0)";
            throw std::bad_alloc();
        }        
        //mVmemStop=ret.memory+ret.size;
        mVmemStop=( (char*) mVmemStop)+size;
        assert(errno!=ENOMEM);
    }
    return ret;
}


int Ogre::MemoryManager::sizeOfStorage(const void* ptr) throw (std::bad_alloc)
{
    if(!ptr)
    {
        MemProfileManager::getSingleton() <<
            "Bad pointer passed to sizeOfStorage() \"NULL\" ";
        throw std::bad_alloc();
    }

    register MemCtrl* head = (MemCtrl*)(((char*)ptr)-sizeof(MemCtrl));
    if(head->magic != MAGIC)
    {

        MemProfileManager::getSingleton() <<
            "Bad pointer passed to sizeOfStorage() or memory corruption" <<
            (void*)head->magic;
        throw std::bad_alloc();
    }
    return head->size^MASK;
}


_OgreExport void *operator new(std::size_t size)
{
	//assert(size && "0 alloc");
	return Ogre::MemoryManager::getSingleton().allocMem(size);
	//return static_cast<void*>(sAllocator.allocateBytes(size));
}

_OgreExport void *operator new[](std::size_t size)
{
	//assert(size && "0 alloc");
	return Ogre::MemoryManager::getSingleton().allocMem(size);
	//return static_cast<void*>(sAllocator.allocateBytes(size));
}

/*
_OgreExport void operator delete(void *ptr, std::size_t size)
{
	//sAllocator.deallocateBytes(static_cast<unsigned char*>(ptr),size);
}

_OgreExport void operator delete[](void *ptr, std::size_t size)
{
	//sAllocator.deallocateBytes(static_cast<unsigned char*>(ptr),size);
}
*/

// /*
_OgreExport void operator delete(void *ptr)
{
	if(ptr==NULL)
		return;
	//sAllocator.deallocateBytes(static_cast<unsigned char*>(ptr),0);
	Ogre::MemoryManager::getSingleton().purgeMem(ptr);
}

_OgreExport void operator delete[](void *ptr)
{
	if(ptr==NULL)
		return;
	//sAllocator.deallocateBytes(static_cast<unsigned char*>(ptr),0);
	Ogre::MemoryManager::getSingleton().purgeMem(ptr);
}
// */


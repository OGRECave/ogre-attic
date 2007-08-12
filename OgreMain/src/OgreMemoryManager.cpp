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
#include <cstdio>

// just a handy debugging macro
#define DBG_PROBE(a) std::cout << #a << " : " << a << std::endl;

// OS specific includes
#ifdef WIN32
# include <windows.h>
#elif defined(__GNUC__)
# include <unistd.h>    // sysconf(3)
# include <sys/mman.h>  // mmap(2)
# include <errno.h>
# define USE_MMAP 1
#endif

#include "OgreMemProfileManager.h"

// allocator 
static Ogre::Allocator<unsigned char>  sAllocator;


// realise static instance member for the init class
Ogre::MemoryManager      Ogre::MemoryManager::smInstance;

#ifdef WIN32
// win32 POSIX(ish) memory emulation
//--------------------------------------------------------------------

// spin lock
static int gSpinLock;

long getpagesize (void) 
{
    static long gPageSize = 0;
    if (! gPageSize)
    {
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        gPageSize = system_info.dwPageSize;
    }
    return gPageSize;
}

long getregionsize (void) 
{
    static long gRegionSize = 0;
    if (! gRegionSize) 
    {
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        gRegionSize = system_info.dwAllocationGranularity;
    }
    return gRegionSize;
}

void *mmap (void *ptr, long size, long prot, long type, long handle, long arg) 
{
    static long gPageSize;
    static long gRegionSize;

    // Wait for spin lock
    while (InterlockedCompareExchange ((void**) &gSpinLock, (void*) 1, (void*) 0) != 0) 
        Sleep (0);

    // First time initialisation
    if (! gPageSize) 
        gPageSize = getpagesize ();
    if (! gRegionSize) 
        gRegionSize = getregionsize ();

    // Allocate this
    ptr = VirtualAlloc (ptr, size, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);
    if (! ptr) 
        ptr = MMAP_FAILURE;

    // Release spin lock
    InterlockedExchange (&gSpinLock, 0);
    return ptr;
}

long munmap (void *ptr, long size) 
{
    static long gPageSize;
    static long gRegionSize;
    int rc = MUNMAP_FAILURE;
    // Wait for spin lock
    while (InterlockedCompareExchange ((void**) &gSpinLock, (void*) 1, (void*) 0) != 0) 
        Sleep (0);

    // First time initialisation
    if (! g_pagesize) 
        g_pagesize = getpagesize ();
    if (! g_regionsize) 
        g_regionsize = getregionsize ();

    // Free this
    if ( VirtualFree (ptr, 0, MEM_RELEASE))
        rc = 0;

    // Release spin lock
    InterlockedExchange (&gSpinLock, 0);
    return rc;
}
//--------------------------------------------------------------------
#endif

void Ogre::MemoryManager::setup() throw( std :: exception )
{
    if(!mSetup)
    {
        mPageSize = getpagesize();
        mRegion = (MemoryRegion**)mmap(  // grab one page for our region array
                   0, mPageSize, PROT_READ|PROT_WRITE,
                   MAP_PRIVATE|MAP_ANONYMOUS,
                   0, 0);

        memset(mRegion,0,mPageSize);
        mNumRegions=0;

        mSetup=true;
    }
}

void * Ogre::MemoryManager::allocMem(size_t size) throw( std :: bad_alloc )
{
    //TODO: free list regions
    //TODO: use mmap direct for allocs bigger than a region
    
    if(!mSetup)
        setup();

    // pad and round up 
    size += OVERHEAD;
    size += MIN_ALOC_SIZE - (size % MIN_ALOC_SIZE);

    // directly mmap() blocks that are bigger than a region
    if(size >= MemoryRegion::POOL_SIZE)
    {
        MemCtrl* tmp = (MemCtrl*) mmap(0, size, PROT_READ|PROT_WRITE,
                                       MAP_PRIVATE|MAP_ANONYMOUS,
                                       0, 0);

        tmp->size=size; // mark as mmap'd
        tmp->magic=MemoryBin::MAGIC;
        return (void*)(((char*)tmp)+sizeof(MemCtrl));
    }

    // look for one
    for(uint32 i=0;i<mNumRegions;++i)
    {
        if(mRegion[i]->canSatisfy(size))
            return mRegion[i]->allocMem(size);
    }

    // we need a new region
    mRegion[mNumRegions]=(MemoryRegion*) mmap(0, sizeof(MemoryRegion),
                                              PROT_READ|PROT_WRITE,
                                              MAP_PRIVATE|MAP_ANONYMOUS,
                                              0, 0);
    mRegion[mNumRegions]->setup(mNumRegions);
    ++mNumRegions;
    return mRegion[mNumRegions-1]->allocMem(size);
}

void Ogre::MemoryManager::purgeMem(void * ptr) throw( std :: bad_alloc )
{
    MemCtrl* tmp = (MemCtrl*)(((char*)ptr)-sizeof(MemCtrl));
    if(!tmp->magic==MemoryBin::MAGIC)
        throw(std::bad_alloc());

    if(tmp->size & MemoryBin::MASK)
    {
        uint32 idx = tmp->reg_id;
        mRegion[idx]->purgeMem(tmp);
        //if(mRegion[idx]->empty())
        //    munmap(mReg)
    }
    else // its mmap'd
    {
        munmap((void*)tmp,tmp->size);
    }
}

int Ogre::MemoryManager::sizeOfStorage(const void * ptr) throw( std :: bad_alloc )
{
    MemCtrl* tmp = (MemCtrl*)(((char*)ptr)-sizeof(MemCtrl));
    if(tmp->magic!=MemoryBin::MAGIC)
    {
        throw(std::bad_alloc());
    }
    if(tmp->size & MemoryBin::MASK)
        return tmp->size ^ MemoryBin::MASK;
    return tmp->size;
}

void Ogre::MemoryManager::dumpInternals()
{
    for(uint32 i=0;i<mNumRegions;++i)
    {
        MemProfileManager::getSingleton()
          << "--------------------------------------------------------\n"
          << "Dumping Internals for region " << i+1 << " of " << mNumRegions
          << "\n\n";
        if(mRegion[i])
            mRegion[i]->dumpInternals();
        MemProfileManager::getSingleton()
          << "--------------------------------------------------------\n";
    }
}

_OgreExport void *operator new(std::size_t size)
{
    //assert(size && "0 alloc");
    //return Ogre::MemoryManager::getSingleton().allocMem(size);
    return static_cast<void*>(sAllocator.allocateBytes(size));
}

_OgreExport void *operator new[](std::size_t size)
{
    //assert(size && "0 alloc");
    //return Ogre::MemoryManager::getSingleton().allocMem(size);
    return static_cast<void*>(sAllocator.allocateBytes(size));
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
    sAllocator.deallocateBytes(static_cast<unsigned char*>(ptr),0);
    //Ogre::MemoryManager::getSingleton().purgeMem(ptr);
}

_OgreExport void operator delete[](void *ptr)
{
    if(ptr==NULL)
        return;
    sAllocator.deallocateBytes(static_cast<unsigned char*>(ptr),0);
    //Ogre::MemoryManager::getSingleton().purgeMem(ptr);
}
// */


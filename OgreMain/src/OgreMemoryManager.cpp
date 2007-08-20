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
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
# include <windows.h>
#elif defined(__GNUC__)
# include <unistd.h>    // sysconf(3)
# include <sys/mman.h>  // mmap(2)
# include <errno.h>
#endif

#include "OgreMemProfileManager.h"
#include <iostream>



// profile
static Ogre::MemProfiler<unsigned char>*   sProfile = NULL;

// realise static instance member for the init class
Ogre::MemoryManager*      Ogre::MemoryManager::smInstance = NULL;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32

// win32 POSIX(ish) memory emulation
//--------------------------------------------------------------------
long getpagesize (void) 
{  
    SYSTEM_INFO system_info;
    GetSystemInfo (&system_info);
    return system_info.dwPageSize;
}

void *mmap (void *ptr, long size, long prot, long type, long handle, long arg) 
{
    // Allocate this
    //ptr = VirtualAlloc (ptr, size, MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN, PAGE_READWRITE);
    ptr = VirtualAlloc (NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    assert(ptr); 
    
    return ptr;
}

long munmap (void *ptr, size_t size) 
{
    return VirtualFree (ptr, 0, MEM_RELEASE);
}
//--------------------------------------------------------------------
#endif

void Ogre::MemoryManager::setup() throw(std::exception)
{
    // register a profile for the default memory manager
    MemProfileManager::getSingleton().registerProfile(sProfile);
}

Ogre::MemoryManager::MemoryManager()
{
    mPageSize = getpagesize();
    /*
    mRegion = (MemoryRegion**)mmap(  // grab one page for our region array
                0, mPageSize, PROT_READ|PROT_WRITE,
                MAP_PRIVATE|MAP_ANONYMOUS,
                0, 0);
    */
    memset(mRegion,0,mPageSize);
    mNumRegions=0;
    mLastAlloc  = NULL;
    mLastDealloc = NULL;

    void* tmp = malloc(sizeof(MemProfiler<unsigned char> ));
    sProfile = ::new(tmp) MemProfiler<unsigned char>(false);
}

Ogre::MemoryManager::~MemoryManager()
{
    free(sProfile);
}

void * Ogre::MemoryManager::allocMem(size_t size) throw( std :: bad_alloc )
{
    // get a loc on the mutex
#if OGRE_THREAD_SUPPORT
    boost::recursive_mutex::scoped_lock lock(mMutex);
#endif
    
    //TODO: free list regions

    // pad and round up 
    size += OVERHEAD;
    size += MIN_ALOC_SIZE - (size % MIN_ALOC_SIZE);

    // directly mmap() blocks that are bigger than a region
    if(size >= MemoryRegion::POOL_SIZE)
    {
        MemCtrl* tmp = (MemCtrl*) mmap(0, (long)size, PROT_READ|PROT_WRITE,
                                       MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);

        tmp->size=(uint32)size; // mark as mmap'd
        tmp->magic=MemoryBin::MAGIC;
        return (void*)(((char*)tmp)+sizeof(MemCtrl));
    }

    // look for one
    if(mLastAlloc && mLastAlloc->canSatisfy(size))
    {
        return mLastAlloc->allocMem(size);
    }
    else if(mLastDealloc && mLastDealloc->canSatisfy(size))
    {
        mLastAlloc = mLastDealloc;
        return mLastDealloc->allocMem(size);
    }
    else
    {
        for(uint32 i=0;i<mNumRegions;++i)
        {
            if(mRegion[i]->canSatisfy((uint32)size))
            {
                mLastAlloc = mRegion[i];
                return mRegion[i]->allocMem(size);
            }
        }
    }

    // we need a new region
    mRegion[mNumRegions]=(MemoryRegion*) mmap(0, sizeof(MemoryRegion),
                                              PROT_READ|PROT_WRITE,
                                              MAP_PRIVATE|MAP_ANONYMOUS,
                                              0, 0);
    mRegion[mNumRegions]->setup(mNumRegions);
    ++mNumRegions;
    mLastAlloc = mRegion[mNumRegions-1];
    return mRegion[mNumRegions-1]->allocMem(size);
}

void Ogre::MemoryManager::purgeMem(void * ptr) throw( std :: bad_alloc )
{
    if(!ptr)
        return;

    // get a loc on the mutex
#if OGRE_THREAD_SUPPORT
    boost::recursive_mutex::scoped_lock lock(mMutex);
#endif

    MemCtrl* tmp = (MemCtrl*)(((char*)ptr)-sizeof(MemCtrl));
    if(tmp->magic!=MemoryBin::MAGIC)
        throw(std::bad_alloc());

    if(tmp->size & MemoryBin::MASK)
    {
        uint32 idx = tmp->reg_id;
        mRegion[idx]->purgeMem(tmp);
        mLastDealloc = mRegion[idx];
        //if(mRegion[idx]->empty())
        //    munmap(mReg)
    }
    else // its mmap'd
    {
        munmap((void*)tmp,tmp->size);
    }
}

size_t Ogre::MemoryManager::sizeOfStorage(const void * ptr) throw( std :: bad_alloc )
{
    if(!ptr)
        return 0;

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

_OgreExport void* doOpNew(size_t sz)
{
    void* p = Ogre::MemoryManager::getSingleton().allocMem(sz);
#ifdef OGRE_DEBUG
    sProfile->note_allocation( 
        Ogre::MemoryManager::getSingleton().sizeOfStorage( p ), NULL );
#endif
    return p;
}

_OgreExport void doOpDelete(void* ptr)
{
    if(!ptr)
        return;
#ifdef OGRE_DEBUG
    sProfile->note_deallocation( (unsigned char*)ptr,
        Ogre::MemoryManager::getSingleton().sizeOfStorage( ptr ) );
#endif
    Ogre::MemoryManager::getSingleton().purgeMem(ptr);
}



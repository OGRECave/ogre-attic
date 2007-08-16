 
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

#ifndef SMALLALLOCPOLICY_H
#define SMALLALLOCPOLICY_H

#include <limits>
#include <cstddef>
#include <memory>
#include <vector>

#include "OgreMemoryManager.h"

namespace Ogre{

    /**
     * A small object allocation policy, this provides a fast efficiant 
     * allocation method for small objects. It will fall back to ::new()
     * and delete:: if the requested allocation is too large. This allocator
     * policy is based on the allocator pressented in "Modern C++ Desgin" by 
     * Alexander Andrescue
     *
     * It takes the following template parameters:-
     * T : the type we will be allocting, (see rebind later)
     * 
     * CHUNK_SIZE: the size of a memory chunk, memory chunks are
     *  allocated chunk at a time from the global allocator, stored and
     *  dished out as needed, this _MUST_ be a multiple of the system
     *  page size (normally 4096 on a 32bit platform)
     * 
     * THRESHOLD: a size threashold above which allocations are forwarded
     *  to the global allocator.
     * 
     * MIN_SIZE: the minimum size of an allocation, must be a multiple of the
     *  allignment value, i.e. 8 on a 32bit platform. Allocation sizes step up
     *  in multipuls of this value. (8,16,24,32 ..... )
     */
    template
    <
        typename T, 
        unsigned int CHUNK_SIZE  = 2048,
        unsigned int THREASHOLD  = 256,
        unsigned int MIN_SIZE    = 8
    >
    class SmallAllocPolicy
    {
        public :
            /// define our types, as per ISO C++
            typedef T                  value_type;
            typedef value_type*        pointer;
            typedef const value_type*  const_pointer;
            typedef value_type&        reference;
            typedef const value_type&  const_reference;
            typedef std::size_t        size_type;
            typedef std::ptrdiff_t     difference_type;

        public :
            /// convert SmallAllocPolicy<T> to SmallAllocPolicy<U>
            template<typename U>
            struct rebind
            {
                typedef class SmallAllocPolicy<U> other;
            };

        private:
            /**
             * this map is used to accelerate finding a fixed bin to 
             * satisfy the requested size. We dont build bins for
             * sizes that are never requested, so we have to find a bin
             * or build one when a reguest comes in.
             */
            int mMap[THREASHOLD/MIN_SIZE];

            /// a chunk of memory, sub-divides into fixed sized blocks
            struct Chunk
            {
                void  setup(size_type sz);
                void* alloc(size_type sz);
                void  purge(void* ptr, size_type sz);

                unsigned char  mFirstFree;
                unsigned char  mNumFree;
                unsigned char* mData;
            };

            /// a fixed size bin holding chunks configured for the size
            struct FixedBin
            {
                std::vector<Chunk> mChunks;
                void  clean();
                void* alloc();
                void  purge(void* ptr);

                Chunk*    mLastAlloc;  // last chunk that provided an allocation
                Chunk*    mLastDealloc;// last chunk that provided a deAllocation 
                size_type mSize;       // size of the bin
            };

            std::vector<FixedBin> mBins;

            // used to speed up size info
            pointer mLastAlloc;
            uint32  mLastAllocSz;
            pointer mLastDealloc;
            uint32  mLastDeallocSz;

        protected:
            /// ctor
            inline explicit SmallAllocPolicy()
            : mLastAlloc(NULL)
            , mLastAllocSz(0)
            , mLastDealloc(NULL)
            , mLastDeallocSz(0)
            { 
                // setup the map
                for(uint32 i=0;i<THREASHOLD/MIN_SIZE;++i)
                    mMap[i] = -1;
            }

            /// dtor
            inline ~SmallAllocPolicy()
            { 
                // kill em all i reverse order, this is nicer for the general allocator 
                typename std::vector<FixedBin>::reverse_iterator i = mBins.rbegin();
                for(;i!=mBins.rend();++i)
                    i->clean();
            }

            /// copy ctor
            inline explicit SmallAllocPolicy(SmallAllocPolicy const&)
            { }

            /// converstion
            template <typename U>
            inline explicit SmallAllocPolicy(SmallAllocPolicy<U> const&)
            { }

            /**
             * memory allocation
             * @param count number of bytes to allocate
             * (NOT number of elements of T)
             */
            pointer allocate(size_type count, typename std::allocator<void>::const_pointer = 0)
            {
                // NOTE: check size  alignment
                if(count > THREASHOLD) // allocation size over threashold
                {
                    mLastAlloc = (pointer)(::new unsigned char [count]);
                    mLastAllocSz = 0;
                    return reinterpret_cast<pointer>(mLastAlloc);
                }

                register int i = count/MIN_SIZE;
                if(mMap[i] == -1) // new size, new bin
                {
                    mBins.reserve(mBins.size()+1);
                    FixedBin bin;
                    bin.mSize = count;
                    mBins.push_back(bin);
                    mMap[i] = mBins.size()-1; // NOTE: check bounds

                    mLastAlloc = (pointer)(mBins.back().alloc());
                    mLastAllocSz = count;

                    return reinterpret_cast<pointer>(mLastAlloc);
                }
                else // use the bin
                {
#ifdef DEBUG
                    mLastAlloc = (pointer)(mBins.at(mMap[i]).alloc());
                    mLastAllocSz = count;
#else
                    mLastAlloc = mBins[mMap[i]].alloc();
                    mLastAllocSz = count;
#endif
                    return reinterpret_cast<pointer>(mLastAlloc);
                }
            }

            /**
             * memory deallocations
             * @param ptr pointer to memory block
             */
            void deallocate(pointer ptr, size_type sz)
            {
                mLastDealloc = ptr;
                if(sz > THREASHOLD)
                {
                    ::delete(ptr);
                    mLastDeallocSz = 0;
                    return;
                }

                mLastDeallocSz = sz;
                register int i = sz/MIN_SIZE;
                assert(mMap[i]!=-1);
#ifdef DEBUG
                mBins.at(mMap[i]).purge(ptr);
#else
                mBins[mMap[i]].purge(ptr);
#endif
            }

            /// maximum allocation size
            inline size_type max_size() const
            {
                return std::numeric_limits<size_type>::max();
            }

            /**
             * return the size of a memory block allocation
             * @param ptr start of the allocated block
             */
            size_type block_size(typename std::allocator<void>::const_pointer ptr = 0)
            {
                /*
                if(ptr==mLastAlloc)
                {
                    if(mLastAllocSz)
                        return mLastAllocSz;
                    else
                        return MemoryManager::getSingleton().sizeOfStorage(ptr);
                }
                if(ptr==mLastDealloc)
                {
                    if(mLastAllocSz)
                        return mLastDeallocSz;
                    else
                        return MemoryManager::getSingleton().sizeOfStorage(ptr);
                }
                */

                // TODO: exahustive search of all bins for the ptr
                return 0;
            }

        private:
            /// determin equality
            template<typename oT, typename oT2>
            friend bool operator==( SmallAllocPolicy<oT> const&, SmallAllocPolicy<oT2> const&);

            /// determin equality
            template<typename oT, typename OtherAllocator>
            friend bool operator==( SmallAllocPolicy<oT> const&, OtherAllocator const&);
    };

    /// determin equality, can memory from another allocator
    /// be released by this allocator, (ISO C++)
    template<typename T, typename T2>
    inline bool operator==(SmallAllocPolicy<T> const&, SmallAllocPolicy<T2> const&)
    {
        return true;
    }

    /// determin equality, can memory from another allocator
    /// be released by this allocator, (ISO C++)
    template<typename T, typename OtherAllocator>
    inline bool operator==(SmallAllocPolicy<T> const&, OtherAllocator const&)
    {
        return false;
    }

    //-----------------------------------------------------------------
    // chunk methods
    template<typename T, unsigned int CHUNK, unsigned int THRESHOLD, unsigned int MIN>
    void SmallAllocPolicy<T,CHUNK,THRESHOLD,MIN>::Chunk::setup( size_type sz ) 
    {
        unsigned int blocks = CHUNK/sz;
        std::cout << "BLOCKS " << blocks << std::endl;
        assert(blocks<=255);

        mData = new unsigned char[CHUNK];
        mFirstFree = 0;
        mNumFree = blocks;

        // build the list
        unsigned char* p = mData;
        for (unsigned char i = 0; i != blocks; p += sz)
        {
            *p = ++i;
        }
    }

    template<typename T, unsigned int CHUNK, unsigned int THRESHOLD, unsigned int MIN>
    void* SmallAllocPolicy<T,CHUNK,THRESHOLD,MIN>::Chunk::alloc( size_type sz )
    {
        if (!mNumFree) 
            return 0;

        unsigned char* ret = mData + (mFirstFree * sz);

        mFirstFree = *ret;
        --mNumFree;

        assert(ret);
        return ret;
    }

    template<typename T, unsigned int CHUNK, unsigned int THRESHOLD, unsigned int MIN>
    void SmallAllocPolicy<T,CHUNK,THRESHOLD,MIN>::Chunk::purge(void* ptr, size_type sz) 
    {
        assert(ptr >= mData && ptr < (mData + CHUNK));
        unsigned char* free = static_cast<unsigned char*>(ptr);

        // Alignment check
        assert((free - mData) % sz == 0);

        *free = mFirstFree;
        mFirstFree = static_cast<unsigned char>((free - mData) / sz);

        // Truncation check
        assert(mFirstFree == (free - mData) / sz);
        ++mNumFree;
    }
    //-----------------------------------------------------------------

    //-----------------------------------------------------------------
    // fixed bin methods
    template<typename T, unsigned int CHUNK, unsigned int THRESHOLD, unsigned int MIN>
    void SmallAllocPolicy<T,CHUNK,THRESHOLD,MIN>::FixedBin::clean()
    {
        // kill em in reverse order, this is nicer for the general allocator
        typename std::vector<Chunk>::reverse_iterator i=mChunks.rbegin();
        for(;i!=mChunks.rend();++i)
            ::delete(i->mData);
    }

    template<typename T, unsigned int CHUNK, unsigned int THRESHOLD, unsigned int MIN>
    void* SmallAllocPolicy<T,CHUNK,THRESHOLD,MIN>::FixedBin::alloc()
    {
        if (mLastAlloc == 0 || mLastAlloc->mNumFree == 0)
        {
            // No available memory in this chunk
            // Try to find one
            typename std::vector<Chunk>::iterator i = mChunks.begin();
            for (;; ++i)
            {
                if (i == mChunks.end())
                {
                    // All filled up-add a new chunk
                    mChunks.reserve(mChunks.size()+1);
                    Chunk chunk;
                    chunk.setup(mSize);
                    mChunks.push_back(chunk);
                    mLastAlloc = &mChunks.back();
                    mLastDealloc = &mChunks.back();
                    break;
                }
                if (i->mNumFree > 0)
                {
                    // Found a chunk
                    mLastAlloc = &*i;
                    break;
                }
            }
        }
        assert(mLastAlloc != 0);
        assert(mLastAlloc->mNumFree > 0);
        return mLastAlloc->alloc(mSize);
    }

    template<typename T, unsigned int CHUNK, unsigned int THRESHOLD, unsigned int MIN>
    void SmallAllocPolicy<T,CHUNK,THRESHOLD,MIN>::FixedBin::purge(void* ptr)
    {
        // is the pointer within the chunk
        if(ptr >= mLastDealloc->mData && ptr < mLastDealloc->mData + CHUNK)
        {
            mLastDealloc->purge(ptr,mSize);
        }
        else
        {
            // search for it
            mLastDealloc = NULL;
            typename std::vector<Chunk>::iterator         fore;
            typename std::vector<Chunk>::reverse_iterator back;
            while(fore!= mChunks.end() && back!=mChunks.rend())
            {
                // step fore
                if(fore!= mChunks.end())
                {
                    ++fore;
                    if(ptr >= fore->mData && ptr < fore->mData + CHUNK)
                    {
                        mLastDealloc = &*fore;
                        break;
                    }
                }

                // step aft
                if(back!= mChunks.rend())
                {
                    ++back;
                    if(ptr >= back->mData && ptr < back->mData + CHUNK)
                    {
                        mLastDealloc = &*back;
                        break;
                    }
                }
            }
            assert(mLastDealloc); // make sure we found one
            mLastDealloc->purge(ptr,mSize);
        }
    }
    //-----------------------------------------------------------------

}// namesoace Ogre
#endif // STDALLOCPOLICY_H

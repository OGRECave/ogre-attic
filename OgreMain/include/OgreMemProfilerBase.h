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

#ifndef OGREMEMPROFILERBASE_INCLUDED
#define OGREMEMPROFILERBASE_INCLUDED



#include "OgrePrerequisites.h"

#if OGRE_THREAD_SUPPORT
#include <boost/thread/recursive_mutex.hpp>
#endif
#include <cstring> // memset

namespace Ogre{

    /// Base class for memory profiler policies
    class MemProfilerBase
    {
    public:
        /// used to accumulate statisitcis between calls to flush
        struct MemStats
        {
            size_t profileID;
            size_t numBytesAllocated;
            size_t numAllocations;
            size_t numBytesDeallocated;
            size_t numDeallocations;
        };

        inline explicit MemProfilerBase()
        {
            memset(&mStats,0,sizeof(MemStats));
        }

        /// copy ctor, we need to define this to be safe with our mutex
        inline explicit MemProfilerBase( MemProfilerBase const& other )
        { 
            // syncronise other
#if OGRE_THREAD_SUPPORT
            boost::recursive_mutex::scoped_lock scoped_lock(other.mDataMutex);
#endif
            mStats = other.mStats;
        }

        /// Assignment, we need to define this to be safe with our mutex
        const MemProfilerBase& operator=(const MemProfilerBase& other)
        {
            if (this == &other)
                return *this;

            // syncronise both sides
#if OGRE_THREAD_SUPPORT
            boost::recursive_mutex::scoped_lock lock1(&mDataMutex < &other.mDataMutex ? mDataMutex : other.mDataMutex);
            boost::recursive_mutex::scoped_lock lock2(&mDataMutex > &other.mDataMutex ? mDataMutex : other.mDataMutex);
#endif

            mStats=other.mStats;
            return *this;
        }


        /// called once a frame to collect statistics
        inline MemStats flush()
        {
            // get a lock on the mutex
#if OGRE_THREAD_SUPPORT
            boost::recursive_mutex::scoped_lock lock(mDataMutex);
#endif

            // copy then reset the stats
            MemStats tmp = mStats;
            mStats.numBytesAllocated=0;
            mStats.numAllocations=0;
            mStats.numBytesDeallocated=0;
            mStats.numDeallocations=0;
            return tmp;
        }

    protected:
        MemStats mStats; // stats package

        /// we use a mutex to maintain sync between allocations 
        /// updating our information and the manager reading it
        //OGRE_MUTEX(mDataMutex);
#if OGRE_THREAD_SUPPORT
        mutable boost::recursive_mutex mDataMutex;
#endif
    };


} // namesoace Ogre
#endif // OGREMEMPROFILERBASE_INCLUDED

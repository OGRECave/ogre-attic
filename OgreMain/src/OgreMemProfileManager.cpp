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
#include "OgreMemProfileManager.h"
#include <sstream>

// realise statics
Ogre::MemProfileManager Ogre::MemProfileManager::smInstance;

namespace Ogre{

    MemProfileManager::~MemProfileManager()
    {
        update(); // collect any final info
        flush("Shutdown"); // flush a final section
        shutdown(); // flush the gloabl stats
        mReportLog.close();
    }

    MemProfileManager::MemProfileManager()
    : mNumUpdates(0)
    , mNumSectionUpdates(0)
    , mPeakAllocations(0)
    , mPeakUpdate(0)
    , mLargestAllocation(0)
    , mProfileIdTracker(0) 
    {
        // reserve some memory ahead of time
        mProfArray.reserve( 10 );

        // setup our log file
        mReportLog.open("OgreMemoryReport.log");

        // init stats
        memset( &( mGlobalStats  ), 0, sizeof( MemProfilerBase::MemStats ) );
        memset( &( mSectionStats ), 0, sizeof( MemProfilerBase::MemStats ) );
    }

    uint32 MemProfileManager::registerProfile( MemProfilerBase* profile )
    {
        ProfileArray::iterator iter = mProfArray.begin();

        for ( ; iter != mProfArray.end(); ++iter ) // for each profile
        {
            if(( *iter ).mProfile == NULL) // found an empty slot
            {
                ( *iter ).mProfile = profile;
                // NOTE: as this slot is empty, the profile will have been 
                // removed and all counts set to 0, so we dont need to do 
                // it again here.

                // update and return the ID
                ++mProfileIdTracker;
                ( *iter ).mStats.profileID = mProfileIdTracker;
                return mProfileIdTracker;
            }
        }

        // no slots found so build a new one
        ++mProfileIdTracker;
        Profile profStruct;
        profStruct.mProfile = profile;
        profStruct.mStats.numAllocations = 0;
        profStruct.mStats.numDeallocations = 0;
        profStruct.mStats.numBytesAllocated = 0;
        profStruct.mStats.numBytesDeallocated = 0;
        profStruct.mStats.profileID = mProfileIdTracker;
        mProfArray.push_back( profStruct ); // add the profile
        return mProfileIdTracker;
    }

    void MemProfileManager::removeProfile(MemProfilerBase* profile)
    {
        // get a lock on the mutex, so we can't update stats in mid-removal
#if OGRE_THREAD_SUPPORT
        boost::recursive_mutex::scoped_lock lock(mUpdateMutex);
#endif

        ProfileArray::iterator iter = mProfArray.begin();
        MemProfilerBase::MemStats tmpStats = profile->flush();

        for ( ; iter != mProfArray.end(); ++iter ) // for each profile
        {
            // find the stats.
            if( ( *iter ).mStats.profileID == tmpStats.profileID )
            {
                mReportLog << "Removing Memory Profile " << tmpStats.profileID << "\n";
                // do some sanity checking
                uint32 aloc = ( *iter ).mStats.numAllocations + tmpStats.numAllocations;
                uint32 dloc = ( *iter ).mStats.numDeallocations + tmpStats.numDeallocations;
                if(aloc-dloc > 0)
                {
                    uint32 tmp = 
                        (( *iter ).mStats.numBytesAllocated + tmpStats.numBytesAllocated) -
                        (( *iter ).mStats.numBytesDeallocated + tmpStats.numBytesDeallocated) ;

                    mReportLog << "  ***MEMORY ERROR DETECTED***  \n";
                    mReportLog << "removed allocator has outstanding allocations!\n";
                    mReportLog <<  aloc - dloc << " allocations ( "<< tmp << " bytes ) \n";
                }
                else
                {
                    mReportLog << "clean removal, no memory errors detected\n";
                }

                // update the global and section stats, so we keep in sync
                mSectionStats.numAllocations += tmpStats.numAllocations;
                mSectionStats.numBytesAllocated += tmpStats.numBytesAllocated;
                mSectionStats.numDeallocations += tmpStats.numDeallocations;
                mSectionStats.numBytesDeallocated += tmpStats.numBytesDeallocated;

                // update the gloabl stats
                mGlobalStats.numAllocations += tmpStats.numAllocations;
                mGlobalStats.numBytesAllocated += tmpStats.numBytesAllocated;
                mGlobalStats.numDeallocations += tmpStats.numDeallocations;
                mGlobalStats.numBytesDeallocated += tmpStats.numBytesDeallocated;

                // remove it
                ( *iter ).mStats.numAllocations = 0;
                ( *iter ).mStats.numDeallocations = 0;
                ( *iter ).mStats.numBytesAllocated = 0;
                ( *iter ).mStats.numBytesDeallocated = 0;
                ( *iter ).mStats.profileID = 0;
                ( *iter ).mProfile=NULL;

                mReportLog << "---------------------------------------------------------\n";
                return; // done, bail now
            }
        }

        // if we get to this point somthing is f00-bar
        mReportLog << "  ***INTERNAL ERROR***  \n";
        mReportLog << " Cant find Memory Profile " << tmpStats.profileID << " to remove!\n";
        mReportLog << "---------------------------------------------------------\n";
        return;
    }

    void MemProfileManager::update()
    {
        // get a lock on the mutex, so we can't remove a profile untill
        // we have finished the batch of updates.
#if OGRE_THREAD_SUPPORT
        boost::recursive_mutex::scoped_lock lock(mUpdateMutex);
#endif

        ProfileArray::iterator iter = mProfArray.begin();
        MemProfilerBase::MemStats tmpStats;
        for ( ; iter != mProfArray.end(); ++iter ) // for each profile
        {
            if( ( *iter ).mProfile)
            {
                // collect its stats
                tmpStats = ( *iter ).mProfile->flush();

                // update local stats package
                ( *iter ).mStats.profileID            = tmpStats.profileID;
                ( *iter ).mStats.numAllocations      += tmpStats.numAllocations;
                ( *iter ).mStats.numBytesAllocated   += tmpStats.numBytesAllocated;
                ( *iter ).mStats.numDeallocations    += tmpStats.numDeallocations;
                ( *iter ).mStats.numBytesDeallocated += tmpStats.numBytesDeallocated;

                // update section stats
                mSectionStats.numAllocations      += tmpStats.numAllocations;
                mSectionStats.numBytesAllocated   += tmpStats.numBytesAllocated;
                mSectionStats.numDeallocations    += tmpStats.numDeallocations;
                mSectionStats.numBytesDeallocated += tmpStats.numBytesDeallocated;

                // update global stats packet
                mGlobalStats.numAllocations      += tmpStats.numAllocations;
                mGlobalStats.numBytesAllocated   += tmpStats.numBytesAllocated;
                mGlobalStats.numDeallocations    += tmpStats.numDeallocations;
                mGlobalStats.numBytesDeallocated += tmpStats.numBytesDeallocated;
            }
        }
        ++mNumUpdates;
        ++mNumSectionUpdates;
    }

    void MemProfileManager::flush(String const& message)
    {
        // section over-veiw
        mReportLog << "---------------------------------------------------------\n";
        mReportLog << "Section Flush: " << message << "\n";
        mReportLog << "Memory Profile Over " << mNumSectionUpdates << " updates\n";
        mReportLog << "Num Allocations                :\t" << mSectionStats.numAllocations << "\n";
        mReportLog << "Num Bytes Allocated            :\t" << mSectionStats.numBytesAllocated << "\n";
        mReportLog << "Num Deallocations              :\t" << mSectionStats.numDeallocations << "\n";
        mReportLog << "Num Bytes Deallocated          :\t" << mSectionStats.numBytesDeallocated << "\n";
        mReportLog << "Average Allocations per Update :\t" <<
            (mSectionStats.numAllocations/static_cast<float>(mNumUpdates)) << "\n";
        mReportLog << "Average Bytes per Allocation   :\t" <<
            (mSectionStats.numBytesAllocated/static_cast<float>(mSectionStats.numAllocations)) << "\n";
        mReportLog << "Global Outstanding Allocations :\t" <<
            (mGlobalStats.numAllocations-mGlobalStats.numDeallocations) <<
            " ( " << (mGlobalStats.numBytesAllocated-mGlobalStats.numBytesDeallocated) <<
            " bytes ) \n";
        mReportLog << "---------------------------------------------------------\n";

        // per-allocator stats
        mReportLog << "Per allocator stats :- \n";
        ProfileArray::iterator iter = mProfArray.begin();
        ProfileArray::iterator end = mProfArray.end();
        for ( ; iter != end; ++iter ) // for each profile
        {
            if(( *iter ).mProfile)
            {
                mReportLog << "Allocator "<< ( *iter ).mStats.profileID;
                mReportLog << " Allocs " << ( *iter ).mStats.numAllocations;
                mReportLog << " ( " << ( *iter ).mStats.numBytesAllocated << " )";
                mReportLog << " De-Allocs " << ( *iter ).mStats.numDeallocations;
                mReportLog << " ( " <<( *iter ).mStats.numBytesDeallocated << " ) \n";
            }
        }
        mReportLog << "---------------------------------------------------------\n";

        // zero the counters
        mSectionStats.numAllocations      = 0;
        mSectionStats.numBytesAllocated   = 0;
        mSectionStats.numDeallocations    = 0;
        mSectionStats.numBytesDeallocated = 0;
        mNumSectionUpdates                = 0;
    }

    void MemProfileManager::shutdown()
    {
        // log the info at shutdown
        mReportLog << "Global Stats at Shutdown: \n";
        mReportLog << "Memory Profile Over " << mNumUpdates << " updates\n";
        mReportLog << "Num Allocations                :\t"  << mGlobalStats.numAllocations << "\n";
        mReportLog << "Num Bytes Allocated            :\t"  << mGlobalStats.numBytesAllocated << "\n";
        mReportLog << "Num Deallocations              :\t"  << mGlobalStats.numDeallocations << "\n";
        mReportLog << "Num Bytes Deallocated          :\t"  << mGlobalStats.numBytesDeallocated << "\n";
        mReportLog << "Average Allocations per Update :\t" <<
            (mGlobalStats.numAllocations/static_cast<float>(mNumUpdates)) << "\n";
        mReportLog << "Average Bytes per Allocation   :\t" <<
            (mGlobalStats.numBytesAllocated/static_cast<float>(mGlobalStats.numAllocations)) << "\n";

        if(mGlobalStats.numAllocations-mGlobalStats.numDeallocations != 0)
        {
            mReportLog << "      ***LEAKED MEMORY***      \n";
            mReportLog << (mGlobalStats.numBytesAllocated-mGlobalStats.numBytesDeallocated) << " bytes in ";
            mReportLog << (mGlobalStats.numAllocations-mGlobalStats.numDeallocations) << " allocations !\n";
        }
        else
        {
            mReportLog << " No memory leaks detected \n";
        }

        mReportLog << "---------------------------------------------------------\n";

        // zero the counters
        mGlobalStats.numAllocations      = 0;
        mGlobalStats.numBytesAllocated   = 0;
        mGlobalStats.numDeallocations    = 0;
        mGlobalStats.numBytesDeallocated = 0;
        mNumUpdates                      = 0;
    }

}

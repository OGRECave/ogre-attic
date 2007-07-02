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
#include "OgreLogManager.h"
#include <sstream>


namespace Ogre{

    template<> MemProfileManager* Singleton<MemProfileManager>::ms_Singleton = 0;

    MemProfileManager* MemProfileManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }

    MemProfileManager& MemProfileManager::getSingleton(void)
    {
        assert( ms_Singleton );  return ( *ms_Singleton );
    }

    MemProfileManager::~MemProfileManager()
    { 
        flush("Shutdown"); // flush a final section
        shutdown(); // flush the gloabl stats
    }

    MemProfileManager::MemProfileManager()
    : mNumUpdates(0)
    , mNumSectionUpdates(0)
    {
        // reserve some memory ahead of time
        mProfArray.reserve( 10 );
        
        // setup our log file
        mReportLog = LogManager::getSingleton().createLog("OgreMemoryReport.log");

        // init stats
        memset( &( mGlobalStats  ), 0, sizeof( MemProfilerBase::MemStats ) );
        memset( &( mSectionStats ), 0, sizeof( MemProfilerBase::MemStats ) );
    }

    void MemProfileManager::registerProfile( MemProfilerBase* profile )
    {
        Profile profStruct;
        profStruct.mProfile=profile;
        memset( &( profStruct.mStats ), 0, sizeof( MemProfilerBase::MemStats ) );
        mProfArray.push_back( profStruct ); // add the profile
    }

    void MemProfileManager::update()
    {
        ProfileArray::iterator iter, end;

        iter = mProfArray.begin();
        end = mProfArray.end();

        MemProfilerBase::MemStats tmpStats;
        for ( ; iter != end; ++iter ) // for each profile
        {
            // collect its stats
            tmpStats = ( *iter ).mProfile->flush();

            // update local stats package
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
        ++mNumUpdates;
        ++mNumSectionUpdates;
    }

    void MemProfileManager::flush(String const& message)
    {
        // log the info
        std::stringstream builder;

        builder << "Section Flush: " << message << "\n";
        builder << "---------------------------------------------------------\n";
        builder << "Memory Profile Over " << mNumSectionUpdates << " updates\n";
        builder << "Num Allocations               :\t"  << mSectionStats.numAllocations << "\n";
        builder << "Num Bytes Allocated           :\t"  << mSectionStats.numBytesAllocated << "\n";
        builder << "Num Deallocations             :\t"  << mSectionStats.numDeallocations << "\n";
        builder << "Num Bytes Deallocations       :\t"  << mSectionStats.numBytesDeallocated << "\n";
        builder << "Average Allocations per Update:\t" <<
            (mSectionStats.numAllocations/static_cast<float>(mNumUpdates)) << "\n";
        builder << "Average Bytes per Allocation  :\t" <<
            (mSectionStats.numBytesAllocated/static_cast<float>(mSectionStats.numAllocations)) << "\n";
        builder << "Outstanding Allocations       :\t" <<
            (mSectionStats.numAllocations-mSectionStats.numDeallocations) <<
            " ( " << (mSectionStats.numBytesAllocated-mSectionStats.numBytesDeallocated) <<
            " bytes ) \n";
        builder << "---------------------------------------------------------";
        mReportLog->logMessage(builder.str());

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
        std::stringstream builder;

        builder << "Global Stats at Shutdown: \n";
        builder << "---------------------------------------------------------\n";
        builder << "Memory Profile Over " << mNumUpdates << " updates\n";
        builder << "Num Allocations               :\t"  << mGlobalStats.numAllocations << "\n";
        builder << "Num Bytes Allocated           :\t"  << mGlobalStats.numBytesAllocated << "\n";
        builder << "Num Deallocations             :\t"  << mGlobalStats.numDeallocations << "\n";
        builder << "Num Bytes Deallocations       :\t"  << mGlobalStats.numBytesDeallocated << "\n";
        builder << "Average Allocations per Update:\t" <<
            (mGlobalStats.numAllocations/static_cast<float>(mNumUpdates)) << "\n";
        builder << "Average Bytes per Allocation  :\t" <<
            (mGlobalStats.numBytesAllocated/static_cast<float>(mGlobalStats.numAllocations)) << "\n";
        
        if(mGlobalStats.numAllocations-mGlobalStats.numDeallocations != 0)
        {    
        	builder << "      ***LEAKED MEMORY***     :\n";
        	builder << (mGlobalStats.numBytesAllocated-mGlobalStats.numBytesDeallocated) << " bytes in ";
        	builder << (mGlobalStats.numAllocations-mGlobalStats.numDeallocations) << " allocations !\n";
        }
        else
        {
        	builder << " No memory leaks detected \n";
        }
            
        builder << "---------------------------------------------------------";
        mReportLog->logMessage(builder.str());

        // zero the counters
        mGlobalStats.numAllocations      = 0;
        mGlobalStats.numBytesAllocated   = 0;
        mGlobalStats.numDeallocations    = 0;
        mGlobalStats.numBytesDeallocated = 0;
        mNumUpdates    	                 = 0;
    }

}

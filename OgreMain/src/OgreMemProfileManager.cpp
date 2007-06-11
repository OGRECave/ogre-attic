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
#include <iostream>

namespace Ogre{

    MemProfileManager::MemProfileManager()
    {
        // reserve some memory ahead of time
        mProfArray.reserve( 10 );

        // setup our log file
        mReportLog = LogManager::getSingleton().createLog("MemoryReport.log");
    }

    void MemProfileManager::registerProfile( MemProfilerBase& profile )
    {
        Profile prof;
        memset( &( prof.mStats ), 0, sizeof( MemProfilerBase::MemStats ) );
        mProfArray.push_back( prof ); // add the profile

        //std::cout << "PROBE: added profile" << std::endl;
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
            ( *iter ).mStats.numAllocations += tmpStats.numAllocations;
            ( *iter ).mStats.numBytesAllocated += tmpStats.numBytesAllocated;
            ( *iter ).mStats.numDeallocations += tmpStats.numDeallocations;
            ( *iter ).mStats.numBytesDeallocated += tmpStats.numBytesDeallocated;

            // update global stats packet
            mGlobalStats.numAllocations += tmpStats.numAllocations;
            mGlobalStats.numBytesAllocated += tmpStats.numBytesAllocated;
            mGlobalStats.numDeallocations += tmpStats.numDeallocations;
            mGlobalStats.numBytesDeallocated += tmpStats.numBytesDeallocated;
        }
    }

}

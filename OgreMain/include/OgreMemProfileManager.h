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

#ifndef MEMPROFILEMANAGER_H
#define MEMPROFILEMANAGER_H

#include "OgrePrerequisites.h"

#include "OgreSingleton.h"
#include "OgreMemProfilerBase.h"
#include <vector>

namespace Ogre{

    /**
    * This is the profile manager for the memory system
    * memory profile policies hook into this via a call to
    * registerProfile. This class collects information from
    * all registered profiles and builds an information report
    * see OgreMemProfiler.h
    */
    class _OgreExport MemProfileManager : public Singleton<MemProfileManager>
    {
    public:
        explicit MemProfileManager();

        //virtual
        ~MemProfileManager();

        /**
        * Register a memory profile with the manager
        * @param profile the memory profile.
        * @return the profiles ID
        */
        uint32 registerProfile(MemProfilerBase* profile);
        
        /**
         * Remove a registered profile 
         * @param the profile ptr
         */
        void removeProfile(MemProfilerBase* profile);

        /**
        * Update, called once a frame to collect profile stats
        * from all registered profiles and build the global
        * profile information.
        */
        void update();

        /**
        * flush the stats to our log file, this records the
        * stats for all allocations since the last call to flush
        * this collection of stats is reffered to as a section. The
        * section stats are zeroed after flushing, note that this
        * will not zero the global stats that hold details on all
        * allocations within the liftime of the memory system.
        *
        * @param message a message that will appear in the stats log
        * to identify the section.
        */
        void flush(String const& message);

    /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static MemProfileManager& getSingleton(void);
        /** Override standard Singleton retrieval.
        @remarks
        Why do we do this? Well, it's because the Singleton
        implementation is in a .h file, which means it gets compiled
        into anybody who includes it. This is needed for the
        Singleton template to work, but we actually only want it
        compiled into the implementation of the class based on the
        Singleton, not all of them. If we don't change this, we get
        link errors when trying to use the Singleton-based class from
        an outside dll.
        @par
        This method just delegates to the template version anyway,
        but the implementation stays in this single compilation unit,
        preventing link errors.
        */
        static MemProfileManager* getSingletonPtr(void);

    protected:
    	// shutdown the memory profiler, unregisters any memory 
    	// profilers and flushes the global memory stats. Any outstanding
    	// allocations at this point are reguarded as memory leaks
    	void shutdown();
    
    
        /// used to hold a profile and pair it with a stats packet
        struct Profile
        {
            MemProfilerBase*          mProfile;
            MemProfilerBase::MemStats mStats;
        };

        // prfile array type
        typedef std::vector<Profile> ProfileArray;

        uint32                      mNumUpdates;        // total number of updates
        uint32                      mNumSectionUpdates; // number of updates since last flush
        ProfileArray                mProfArray;         // registered profiles
        MemProfilerBase::MemStats   mSectionStats;      // section stats
        MemProfilerBase::MemStats   mGlobalStats;       // global stats
        Log*                        mReportLog;         // log for holding the profile info
        
        // all this is per-section, i.e. scopped by calls to flush()
        uint32 mPeakAllocations;    // largest number of allocations occuring in a single update
        uint32 mPeakUpdate;         // update that had the largest number of allocations
        uint32 mLargestAllocation;  // largest single memory allocation
        
        uint32 mProfileIdTracker; // used to hold the highest ID

    private:
    };
}


#endif // MEMPROFILEMANAGER_H

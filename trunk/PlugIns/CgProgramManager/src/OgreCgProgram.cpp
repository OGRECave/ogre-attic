/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreCgProgram.h"
#include "OgreGpuProgramManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    void CgProgram::selectProfile()
    {
        mSelectedProfile = "";
        mSelectedCgProfile = CG_PROFILE_UNKNOWN;

        StringVector::iterator i, iend;
        iend = mProfiles.end();
        GpuProgramManager& gpuMgr = GpuProgramManager::getSingleton();
        for (i = mProfiles.begin(); i != iend; ++i)
        {
            if (gpuMgr.isSyntaxSupported(*i))
            {
                mSelectedProfile = *i;
                mSelectedCgProfile = cgGetProfile(mSelectedProfile);
                // Check for errors
                checkForCgError("CgProgram::selectProfile", 
                    "Unable to find CG profile enum for program " + mName + ": ", mCgContext);
                break;
            }
        }
    }
    //-----------------------------------------------------------------------
    void CgProgram::loadFromSource(void)
    {
        // Create Cg Program
        selectProfile();
        mCgProgram = cgCreateProgram(mCgContext, CG_SOURCE, mSource.c_str(), 
            mSelectedCgProfile, mEntryPoint.c_str(), NULL);
        // Check for errors
        checkForCgError("CgProgram::loadFromSource", 
            "Unable to compile Cg program " + mName + ": ", mCgContext);

    }
    //-----------------------------------------------------------------------
    void CgProgram::createLowLevelImpl(void)
    {
        // TODO
    }
    //-----------------------------------------------------------------------
    void CgProgram::unloadImpl(void)
    {
        // TODO
    }
    //-----------------------------------------------------------------------
    void CgProgram::populateParameterNames(GpuProgramParametersSharedPtr params)
    {
        // TODO
    }
    //-----------------------------------------------------------------------
    CgProgram::CgProgram(const String& name, GpuProgramType gpType, 
        const String& language, CGcontext context)
        : HighLevelGpuProgram(name, gpType, language), mCgContext(context)
    {
        
    }
    //-----------------------------------------------------------------------
    CgProgram::~CgProgram()
    {
    }
    //-----------------------------------------------------------------------

}

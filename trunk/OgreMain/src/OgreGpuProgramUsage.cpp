
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
#include "OgreStableHeaders.h"
#include "OgreGpuProgramUsage.h"
#include "OgreGpuProgramManager.h"

namespace Ogre
{
    //-----------------------------------------------------------------------------
    GpuProgramUsage::GpuProgramUsage(GpuProgramType gptype, const String& programName, 
        bool validateImmediately) :
        mType(gptype), mProgramName(programName), 
        mDeferValidation(!validateImmediately), mProgram(NULL)
    {
		// Create a set of parameters incase we don't want to share the params
		mLowLevelParams = GpuProgramManager::getSingleton().createParameters();
    }
	//-----------------------------------------------------------------------------
	GpuProgramUsage::GpuProgramUsage(const GpuProgramUsage& oth)
	{
		mType = oth.mType;
		mProgramName = oth.mProgramName;
		mDeferValidation = oth.mDeferValidation;
		mProgram = oth.mProgram;
		mLowLevelParams =  oth.mLowLevelParams;
	}
	//-----------------------------------------------------------------------------
	void GpuProgramUsage::setProgramName(const String& name)
	{
		mProgramName = name;
		if (!mDeferValidation)
		{
			validateName();
		}
	}
	//-----------------------------------------------------------------------------
    void GpuProgramUsage::enableValidation(void)
    {
		if (mDeferValidation)
		{
			// Get a handle to the program
			validateName();
			validateNamedParameters();
		}
        mDeferValidation = false;
    }
    //-----------------------------------------------------------------------------
    void GpuProgramUsage::setParameters(GpuProgramParametersSharedPtr params)
    {
        mLowLevelParams = params;
    }
    //-----------------------------------------------------------------------------
    GpuProgramParametersSharedPtr GpuProgramUsage::getParameters(void)
    {
        return mLowLevelParams;
    }
    //-----------------------------------------------------------------------------
	void GpuProgramUsage::validateName(void)
	{
		mProgram = static_cast<GpuProgram*>(
			GpuProgramManager::getSingleton().getByName(mProgramName));
	}
    //-----------------------------------------------------------------------------
	void GpuProgramUsage::validateNamedParameters(void)
	{
		// TODO
	}
    //-----------------------------------------------------------------------------
	GpuProgram* GpuProgramUsage::getProgram(void)
	{
		assert(mProgram && "Usage not yet validated!");
		return mProgram;
	}
}

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
#include "OgreGpuProgram.h"
#include "OgreGpuProgramManager.h"

namespace Ogre
{
	//-----------------------------------------------------------------------------
	GpuProgram::GpuProgram(const String& name, GpuProgramType gptype) 
		: mType(gptype), mLoadFromFile(true)
	{
		mName = name;
	}
	//-----------------------------------------------------------------------------
    void GpuProgram::setSource(const String& source)
    {
        mSource = source;
        mLoadFromFile = false;
	}

	//-----------------------------------------------------------------------------
	void GpuProgram::load(void)
	{
        if (mIsLoaded)
        {
            unload();
        }
        if (mLoadFromFile)
        {
            // find & load source code
            SDDataChunk chunk;
            GpuProgramManager::getSingleton()._findResourceData(mName, chunk);
            mSource = chunk.getAsString();
        }

        // Call polymorphic load
        loadFromSource();

        mIsLoaded = true;
    }
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::setConstant(size_t index, Real val)
	{
		setConstant(index, &val, 1);
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::setConstant(size_t index, int val)
	{
		setConstant(index, &val, 1);
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::setConstant(size_t index, const Vector4& vec)
	{
		setConstant(index++, &vec.x, 1);
		setConstant(index++, &vec.y, 1);
		setConstant(index++, &vec.z, 1);
		setConstant(index++, &vec.w, 1);
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::setConstant(size_t index, const Vector3& vec)
	{
		setConstant(index++, &vec.x, 1);
		setConstant(index++, &vec.y, 1);
		setConstant(index++, &vec.z, 1);
	}
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const Real *val, size_t count)
    {
        // Expand if required
        if (mRealConstants.size() < index + count)
        	mRealConstants.resize(index + count);

        // Copy directly in since vector is a contiguous container
        memcpy(&mRealConstants[index], val, sizeof(Real)*count);

    }
	//-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const int *val, size_t count)
    {
        // Expand if required
        if (mIntConstants.size() < index + count)
			mIntConstants.resize(index + count);

        // Copy directly in since vector is a contiguous container
        memcpy(&mIntConstants[index], val, sizeof(int)*count);

    }
	//-----------------------------------------------------------------------------
    void GpuProgramParameters::setAutoConstant(AutoConstantType acType, size_t index, size_t extraInfo)
    {
        mAutoConstants.push_back(AutoConstantEntry(acType, index, extraInfo));
    }
	//-----------------------------------------------------------------------------
    void GpuProgramParameters::clearAutoConstants(void)
    {
        mAutoConstants.clear();
    }
	//-----------------------------------------------------------------------------
    GpuProgramParameters::AutoConstantIterator GpuProgramParameters::getAutoConstantIterator(void)
    {
        return AutoConstantIterator(mAutoConstants.begin(), mAutoConstants.end());
    }

}

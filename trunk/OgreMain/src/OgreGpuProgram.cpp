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
#include "OgreSDDataChunk.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreAutoParamDataSource.h"
#include "OgreLight.h"

namespace Ogre
{
	//-----------------------------------------------------------------------------
	GpuProgram::GpuProgram(const String& name, GpuProgramType gptype, const String& syntaxCode) 
		: mType(gptype), mLoadFromFile(true), mSyntaxCode(syntaxCode)
	{
		mName = name;
	}
	//-----------------------------------------------------------------------------
	void GpuProgram::setSourceFile(const String& filename)
	{
		mFilename = filename;
		mSource = "";
		mLoadFromFile = true;
	}
	//-----------------------------------------------------------------------------
    void GpuProgram::setSource(const String& source)
    {
        mSource = source;
		mFilename = "";
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
            GpuProgramManager::getSingleton()._findResourceData(mFilename, chunk);
            mSource = chunk.getAsString();
        }

        // Call polymorphic load
        loadFromSource();

        mIsLoaded = true;
    }
	//-----------------------------------------------------------------------------
    bool GpuProgram::isSupported(void)
    {
        return GpuProgramManager::getSingleton().isSyntaxSupported(mSyntaxCode);
    }
	//-----------------------------------------------------------------------------
    GpuProgramParametersSharedPtr GpuProgram::createParameters(void)
    {
        // Default implementation simply returns standard parameters.
        return GpuProgramManager::getSingleton().createParameters();
    }
	//-----------------------------------------------------------------------------
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
	void GpuProgramParameters::setConstant(size_t index, const Matrix4& m)
    {
        // set as 4x 4-element floats
        // Turns out in vertex programs, D3D uses the 'right' matrix layout
        // so no need to convert matrix, we can use the same for both
        GpuProgramParameters::setConstant(index, m[0], 4);
        GpuProgramParameters::setConstant(index+4, m[1], 4);
        GpuProgramParameters::setConstant(index+8, m[2], 4);
        GpuProgramParameters::setConstant(index+12, m[3], 4);
    }
	//-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const ColourValue& colour)
    {
        setConstant(index++, &colour.r, 1);
        setConstant(index++, &colour.g, 1);
        setConstant(index++, &colour.b, 1);
        setConstant(index++, &colour.a, 1);
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
    void GpuProgramParameters::setAutoConstant(size_t index, AutoConstantType acType, size_t extraInfo)
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
	//-----------------------------------------------------------------------------
    void GpuProgramParameters::_updateAutoParams(const AutoParamDataSource& source)
    {
        if (!hasAutoConstants()) return; // abort early if no autos
        Vector3 vec3;
        Vector4 vec4;

        AutoConstantList::const_iterator i, iend;
        iend = mAutoConstants.end();
        for (i = mAutoConstants.begin(); i != iend; ++i)
        {
            switch(i->paramType)
            {
            case ACT_WORLD_MATRIX:
                setConstant(i->index, source.getWorldMatrix());
                break;
            case ACT_VIEW_MATRIX:
                setConstant(i->index, source.getViewMatrix());
                break;
            case ACT_PROJECTION_MATRIX:
                setConstant(i->index, source.getProjectionMatrix());
                break;
            case ACT_WORLDVIEW_MATRIX:
                setConstant(i->index, source.getWorldViewMatrix());
                break;
            case ACT_WORLDVIEWPROJ_MATRIX:
                setConstant(i->index, source.getWorldViewProjMatrix());
                break;
            case ACT_INVERSE_WORLD_MATRIX:
                setConstant(i->index, source.getInverseWorldMatrix());
                break;
            case ACT_INVERSE_WORLDVIEW_MATRIX:
                setConstant(i->index, source.getInverseWorldViewMatrix());
                break;
            case ACT_LIGHT_DIFFUSE_COLOUR:
                setConstant(i->index, source.getLight(i->data).getDiffuseColour());
                break;
            case ACT_LIGHT_SPECULAR_COLOUR:
                setConstant(i->index, source.getLight(i->data).getSpecularColour());
                break;
            case ACT_LIGHT_POSITION_OBJECT_SPACE:
                setConstant(i->index, 
                    source.getInverseWorldMatrix() * source.getLight(i->data).getDerivedPosition());
                break;
            case ACT_LIGHT_DIRECTION_OBJECT_SPACE:
                vec3 = source.getInverseWorldMatrix() * 
                    source.getLight(i->data).getDerivedDirection();
                vec3.normalise();
                // Set as 4D vector for compatibility
                setConstant(i->index, Vector4(vec3.x, vec3.y, vec3.z, 1.0f));
                break;
            case ACT_CAMERA_POSITION_OBJECT_SPACE:
                setConstant(i->index, source.getCameraPositionObjectSpace());
                break;
			case ACT_AMBIENT_LIGHT_COLOUR:
				setConstant(i->index, source.getAmbientLightColour());
				break;
            case ACT_LIGHT_ATTENUATION:
                // range, const, linear, quad
                const Light& l = source.getLight(i->data);
                vec4.x = l.getAttenuationRange();
                vec4.y = l.getAttenuationConstant();
                vec4.z = l.getAttenuationLinear();
                vec4.w = l.getAttenuationQuadric();
                setConstant(i->index, vec4);
                break;
            }
        }
    }
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::_align(size_t intAlignment, size_t realAlignment)	
	{
		size_t rem = mIntConstants.size() % intAlignment;
        if (rem != 0 )
			mIntConstants.resize(mIntConstants.size() + rem);
		
		rem = mRealConstants.size() % realAlignment;
        if (rem != 0 )
			mRealConstants.resize(mRealConstants.size() + rem);
			
	}
    //---------------------------------------------------------------------------
    void GpuProgramParameters::_mapParameterNameToIndex(const String& name, 
        size_t index)
    {
        mParamNameMap[name] = index;
    }
    //---------------------------------------------------------------------------
    size_t GpuProgramParameters::getParamIndex(const String& name)
    {
        ParamNameMap::iterator i = mParamNameMap.find(name);
        if (i == mParamNameMap.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, "Cannot find a parameter named " + name,
                "GpuProgramParameters::getParamIndex");
        }
        return i->second;
    }
    //---------------------------------------------------------------------------
	void GpuProgramParameters::setNamedConstant(const String& name, Real val)
    {
        setConstant(getParamIndex(name), val);
    }
    //---------------------------------------------------------------------------
	void GpuProgramParameters::setNamedConstant(const String& name, int val)
    {
        setConstant(getParamIndex(name), val);
    }
    //---------------------------------------------------------------------------
	void GpuProgramParameters::setNamedConstant(const String& name, const Vector4& vec)
    {
        setConstant(getParamIndex(name), vec);
    }
    //---------------------------------------------------------------------------
	void GpuProgramParameters::setNamedConstant(const String& name, const Vector3& vec)
    {
        setConstant(getParamIndex(name), vec);
    }
    //---------------------------------------------------------------------------
	void GpuProgramParameters::setNamedConstant(const String& name, const Matrix4& m)
    {
        setConstant(getParamIndex(name), m);
    }
    //---------------------------------------------------------------------------
	void GpuProgramParameters::setNamedConstant(const String& name, const Real *val, size_t count)
    {
        setConstant(getParamIndex(name), val, count);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstant(const String& name, const ColourValue& colour)
    {
        setConstant(getParamIndex(name), colour);
    }
    //---------------------------------------------------------------------------
	void GpuProgramParameters::setNamedConstant(const String& name, const int *val, size_t count)
    {
        setConstant(getParamIndex(name), val, count);
    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedAutoConstant(const String& name, AutoConstantType acType, size_t extraInfo)
    {
        setAutoConstant(getParamIndex(name), acType, extraInfo);
    }
}

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
#include "OgreControllerManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"

namespace Ogre
{
	//-----------------------------------------------------------------------------
	GpuProgram::GpuProgram(const String& name, GpuProgramType gptype, const String& syntaxCode) 
		: mType(gptype), mLoadFromFile(true), mSyntaxCode(syntaxCode), mSkeletalAnimation(false)
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
    bool GpuProgram::isSupported(void) const
    {
		// If skeletal animation is being done, we need support for UBYTE4
		if (isSkeletalAnimationIncluded() && 
			!Root::getSingleton().getRenderSystem()->getCapabilities()
				->hasCapability(RSC_VERTEX_FORMAT_UBYTE4))
		{
			return false;
		}
        return GpuProgramManager::getSingleton().isSyntaxSupported(mSyntaxCode);
    }
    //-----------------------------------------------------------------------------
    GpuProgramParametersSharedPtr GpuProgram::createParameters(void)
    {
        // Default implementation simply returns standard parameters.
        return GpuProgramManager::getSingleton().createParameters();
    }
    //-----------------------------------------------------------------------------
	GpuProgramParameters::GpuProgramParameters()
        : mTransposeMatrices(false)
    {
    }
    //-----------------------------------------------------------------------------
	void GpuProgramParameters::setConstant(size_t index, const Vector4& vec)
	{
		setConstant(index, vec.val, 1);
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::setConstant(size_t index, const Vector3& vec)
	{
        setConstant(index, Vector4(vec.x, vec.y, vec.z, 1.0f));
	}
	//-----------------------------------------------------------------------------
	void GpuProgramParameters::setConstant(size_t index, const Matrix4& m)
    {
        // set as 4x 4-element floats
        if (mTransposeMatrices)
        {
            Matrix4 t = m.transpose();
            GpuProgramParameters::setConstant(index++, t[0], 1);
            GpuProgramParameters::setConstant(index++, t[1], 1);
            GpuProgramParameters::setConstant(index++, t[2], 1);
            GpuProgramParameters::setConstant(index, t[3], 1);
        }
        else
        {
            GpuProgramParameters::setConstant(index++, m[0], 1);
            GpuProgramParameters::setConstant(index++, m[1], 1);
            GpuProgramParameters::setConstant(index++, m[2], 1);
            GpuProgramParameters::setConstant(index, m[3], 1);
        }
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const Matrix4* pMatrix, 
        size_t numEntries)
    {
        for (size_t i = 0; i < numEntries; ++i)
        {
            const Matrix4& m = pMatrix[i];

            if (mTransposeMatrices)
            {
                Matrix4 t = m.transpose();
                GpuProgramParameters::setConstant(index++, t[0], 1);
                GpuProgramParameters::setConstant(index++, t[1], 1);
                GpuProgramParameters::setConstant(index++, t[2], 1);
                GpuProgramParameters::setConstant(index++, t[3], 1);
            }
            else
            {
                GpuProgramParameters::setConstant(index++, m[0], 1);
                GpuProgramParameters::setConstant(index++, m[1], 1);
                GpuProgramParameters::setConstant(index++, m[2], 1);
                GpuProgramParameters::setConstant(index++, m[3], 1);
            }
        }
    }
	//-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const ColourValue& colour)
    {
        setConstant(index, colour.val, 1);
    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const Real *val, size_t count)
    {
        // Expand if required
        if (mRealConstants.size() < index + count)
        	mRealConstants.resize(index + count);

        // Copy in chunks of 4
        while (count--)
        {
            RealConstantEntry* e = &(mRealConstants[index++]);
            e->isSet = true;
            memcpy(e->val, val, sizeof(Real) * 4);
            val += 4;
        }

    }
	//-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const int *val, size_t count)
    {
        // Expand if required
        if (mIntConstants.size() < index + count)
        	mIntConstants.resize(index + count);

        // Copy in chunks of 4
        while (count--)
        {
            IntConstantEntry* e = &(mIntConstants[index++]);
            e->isSet = true;
            memcpy(e->val, val, sizeof(int) * 4);
            val += 4;
        }
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
    void GpuProgramParameters::_updateAutoParamsNoLights(const AutoParamDataSource& source)
    {
        if (!hasAutoConstants()) return; // abort early if no autos
        Vector3 vec3;
        Vector4 vec4;
        size_t index;
        size_t numMatrices;
        const Matrix4* pMatrix;
        size_t m;

        AutoConstantList::const_iterator i, iend;
        iend = mAutoConstants.end();
        for (i = mAutoConstants.begin(); i != iend; ++i)
        {
            switch(i->paramType)
            {
            case ACT_WORLD_MATRIX:
                setConstant(i->index, source.getWorldMatrix());
                break;
            case ACT_WORLD_MATRIX_ARRAY:
                setConstant(i->index, source.getWorldMatrixArray(), 
                    source.getWorldMatrixCount());
                break;
            case ACT_WORLD_MATRIX_ARRAY_3x4:
                // Loop over matrices
                pMatrix = source.getWorldMatrixArray();
                numMatrices = source.getWorldMatrixCount();
                index = i->index;
                for (m = 0; m < numMatrices; ++m)
                {
                    GpuProgramParameters::setConstant(index++, (*pMatrix)[0], 1);
                    GpuProgramParameters::setConstant(index++, (*pMatrix)[1], 1);
                    GpuProgramParameters::setConstant(index++, (*pMatrix)[2], 1);
                    ++pMatrix;
                }
                
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
            case ACT_VIEWPROJ_MATRIX:
                setConstant(i->index, source.getViewProjectionMatrix());
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
            case ACT_CAMERA_POSITION_OBJECT_SPACE:
                setConstant(i->index, source.getCameraPositionObjectSpace());
                break;
            // NB ambient light still here because it's not related to a specific light
			case ACT_AMBIENT_LIGHT_COLOUR: 
				setConstant(i->index, source.getAmbientLightColour());
				break;
            case ACT_TEXTURE_VIEWPROJ_MATRIX:
                setConstant(i->index, source.getTextureViewProjMatrix());
                break;

            }
        }
    }
	//-----------------------------------------------------------------------------
    void GpuProgramParameters::_updateAutoParamsLightsOnly(const AutoParamDataSource& source)
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
            case ACT_LIGHT_DIFFUSE_COLOUR:
                setConstant(i->index, source.getLight(i->data).getDiffuseColour());
                break;
            case ACT_LIGHT_SPECULAR_COLOUR:
                setConstant(i->index, source.getLight(i->data).getSpecularColour());
                break;
            case ACT_LIGHT_POSITION:
				// Get as 4D vector, works for directional lights too
                setConstant(i->index, 
                    source.getLight(i->data).getAs4DVector());
                break;
            case ACT_LIGHT_DIRECTION:
                vec3 = source.getLight(i->data).getDerivedDirection();
                // Set as 4D vector for compatibility
                setConstant(i->index, Vector4(vec3.x, vec3.y, vec3.z, 1.0f));
                break;
            case ACT_LIGHT_POSITION_OBJECT_SPACE:
                setConstant(i->index, 
                    source.getInverseWorldMatrix() * source.getLight(i->data).getAs4DVector());
                break;
            case ACT_LIGHT_DIRECTION_OBJECT_SPACE:
                vec3 = source.getInverseWorldMatrix() * 
                    source.getLight(i->data).getDerivedDirection();
                vec3.normalise();
                // Set as 4D vector for compatibility
                setConstant(i->index, Vector4(vec3.x, vec3.y, vec3.z, 1.0f));
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
    //---------------------------------------------------------------------------
    void GpuProgramParameters::_mapParameterNameToIndex(const String& name, 
        size_t index)
    {
        mParamNameMap[name] = index;
    }
    //---------------------------------------------------------------------------
    size_t GpuProgramParameters::getParamIndex(const String& name) const
    {
        ParamNameMap::const_iterator i = mParamNameMap.find(name);
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
    void GpuProgramParameters::setNamedConstant(const String& name, const Matrix4* m, 
        size_t numEntries)
    {
        setConstant(getParamIndex(name), m, numEntries);
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
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setConstantFromTime(size_t index, Real factor)
    {
        // Create controller
        ControllerManager::getSingleton().createGpuProgramTimerParam(this, index, factor);

    }
    //---------------------------------------------------------------------------
    void GpuProgramParameters::setNamedConstantFromTime(const String& name, Real factor)
    {
        setConstantFromTime(getParamIndex(name), factor);
    }
    //---------------------------------------------------------------------------
    GpuProgramParameters::RealConstantIterator GpuProgramParameters::getRealConstantIterator(void)
    {
        return RealConstantIterator(mRealConstants.begin(), mRealConstants.end());
    }
    //---------------------------------------------------------------------------
    GpuProgramParameters::IntConstantIterator GpuProgramParameters::getIntConstantIterator(void)
    {
        return IntConstantIterator(mIntConstants.begin(), mIntConstants.end());
    }

}

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright (c) 2000-2005 The OGRE Team
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
#include "OgreHighLevelGpuProgram.h"
#include "OgreGpuProgramManager.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreAutoParamDataSource.h"
#include "OgreLight.h"
#include "OgreControllerManager.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreStringConverter.h"

namespace Ogre
{
	//-----------------------------------------------------------------------------
	GpuProgram::CmdType GpuProgram::msTypeCmd;
	GpuProgram::CmdSyntax GpuProgram::msSyntaxCmd;
	GpuProgram::CmdSkeletal GpuProgram::msSkeletalCmd;

	//-----------------------------------------------------------------------------
	GpuProgram::GpuProgram(ResourceManager* creator, const String& name, ResourceHandle handle,
		const String& group, bool isManual, ManualResourceLoader* loader) 
		:Resource(creator, name, handle, group, isManual, loader),
		mType(GPT_VERTEX_PROGRAM), mLoadFromFile(true), mSkeletalAnimation(false),
		mPassSurfaceAndLightStates(false)
	{
	}
	//-----------------------------------------------------------------------------
	void GpuProgram::setType(GpuProgramType t)
	{
		mType = t;
	}
	//-----------------------------------------------------------------------------
	void GpuProgram::setSyntaxCode(const String& syntax)
	{
		mSyntaxCode = syntax;
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
	void GpuProgram::loadImpl(void)
	{
        if (mLoadFromFile)
        {
            // find & load source code
            DataStreamPtr stream = 
				ResourceGroupManager::getSingleton().openResource(mFilename, mGroup);
            mSource = stream->getAsString();
        }

        // Call polymorphic load
        loadFromSource();

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
        GpuProgramParametersSharedPtr ret = 
			GpuProgramManager::getSingleton().createParameters();
		// Copy in default parameters if present
		if (!mDefaultParams.isNull())
			ret->copyConstantsFrom(*(mDefaultParams.get()));
		
		return ret;
    }
    //-----------------------------------------------------------------------------
	GpuProgramParametersSharedPtr GpuProgram::getDefaultParameters(void)
	{
		if (mDefaultParams.isNull())
		{
			mDefaultParams = createParameters();
		}
		return mDefaultParams;
	}
	//-----------------------------------------------------------------------------
	void GpuProgram::setupBaseParamDictionary(void)
	{
		ParamDictionary* dict = getParamDictionary();

		dict->addParameter(
			ParameterDef("type", "'vertex_program' or 'fragment_program'",
				PT_STRING), &msTypeCmd);
		dict->addParameter(
			ParameterDef("syntax", "Syntax code, e.g. vs_1_1", PT_STRING), &msSyntaxCmd);
		dict->addParameter(
			ParameterDef("includes_skeletal_animation", 
			"Whether this vertex program includes skeletal animation", PT_BOOL), 
			&msSkeletalCmd);
	}

    //-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	GpuProgramParameters::GpuProgramParameters()
        : mTransposeMatrices(false), mAutoAddParamName(false)
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
    void GpuProgramParameters::setConstant(size_t index, const float *val, size_t count)
    {
        // Expand if required
        if (mRealConstants.size() < index + count)
        	mRealConstants.resize(index + count);

        // Copy in chunks of 4
        while (count--)
        {
            RealConstantEntry* e = &(mRealConstants[index++]);
            e->isSet = true;
            memcpy(e->val, val, sizeof(float) * 4);
            val += 4;
        }

    }
    //-----------------------------------------------------------------------------
    void GpuProgramParameters::setConstant(size_t index, const double *val, size_t count)
    {
        // Expand if required
        if (mRealConstants.size() < index + count)
        	mRealConstants.resize(index + count);

        // Copy, casting to float
        while (count--)
        {
            RealConstantEntry* e = &(mRealConstants[index++]);
            e->isSet = true;
			e->val[0] = static_cast<float>(val[0]);
			e->val[1] = static_cast<float>(val[1]);
			e->val[2] = static_cast<float>(val[2]);
			e->val[3] = static_cast<float>(val[3]);
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
    GpuProgramParameters::AutoConstantIterator GpuProgramParameters::getAutoConstantIterator(void) const
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
			case ACT_INVERSE_VIEW_MATRIX:
				setConstant(i->index, source.getInverseViewMatrix());
				break;
            case ACT_INVERSE_WORLDVIEW_MATRIX:
                setConstant(i->index, source.getInverseWorldViewMatrix());
                break;
            case ACT_INVERSETRANSPOSE_WORLD_MATRIX:
                setConstant(i->index, source.getInverseTransposeWorldMatrix());
                break;
            case ACT_INVERSETRANSPOSE_WORLDVIEW_MATRIX:
                setConstant(i->index, source.getInverseTransposeWorldViewMatrix());
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
            case ACT_CUSTOM:
                source.getCurrentRenderable()->_updateCustomGpuParameter(*i, this);
                break;
            default:
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
			case ACT_LIGHT_DISTANCE_OBJECT_SPACE:
				vec3 = source.getInverseWorldMatrix() * source.getLight(i->data).getDerivedPosition();
				setConstant(i->index, vec3.length());
				break;
			case ACT_SHADOW_EXTRUSION_DISTANCE:
				setConstant(i->index, source.getShadowExtrusionDistance());
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
    size_t GpuProgramParameters::getParamIndex(const String& name)
    {
        ParamNameMap::const_iterator i = mParamNameMap.find(name);
        if (i == mParamNameMap.end())
        {
			// name not found in map, should it be added to the map?
			if(mAutoAddParamName)
			{
				// determine index
				// don't know which Constants list the name is for
				// so pick the largest index
				size_t index = (mRealConstants.size() > mIntConstants.size()) ?
					mRealConstants.size() : mIntConstants.size();
				// allow for at least one Vector4
        		mRealConstants.resize(index + 1);
        		mIntConstants.resize(index + 1);
				_mapParameterNameToIndex(name, index);
				return index;
			}
			else
			{
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot find a parameter named " + name,
					"GpuProgramParameters::getParamIndex");
			}
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
	void GpuProgramParameters::setNamedConstant(const String& name, const float *val, size_t count)
    {
        setConstant(getParamIndex(name), val, count);
    }
    //---------------------------------------------------------------------------
	void GpuProgramParameters::setNamedConstant(const String& name, const double *val, size_t count)
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
    GpuProgramParameters::RealConstantIterator GpuProgramParameters::getRealConstantIterator(void) const
    {
        return RealConstantIterator(mRealConstants.begin(), mRealConstants.end());
    }
    //---------------------------------------------------------------------------
    GpuProgramParameters::IntConstantIterator GpuProgramParameters::getIntConstantIterator(void) const
    {
		return IntConstantIterator(mIntConstants.begin(), mIntConstants.end());
    }

    //---------------------------------------------------------------------------
	GpuProgramParameters::RealConstantEntry* GpuProgramParameters::getRealConstantEntry(const size_t index)
	{
        if (index < mRealConstants.size())
		{
			return &(mRealConstants[index]);
		}
		else
		{
			return NULL;
		}
	}

    //---------------------------------------------------------------------------
	GpuProgramParameters::IntConstantEntry* GpuProgramParameters::getIntConstantEntry(const size_t index)
	{
        if (index < mIntConstants.size())
		{
			return &(mIntConstants[index]);
		}
		else
		{
			return NULL;
		}
	}

    //---------------------------------------------------------------------------
	GpuProgramParameters::RealConstantEntry* GpuProgramParameters::getNamedRealConstantEntry(const String& name)
	{
		// check if name is found
        ParamNameMap::const_iterator i = mParamNameMap.find(name);

        if (i == mParamNameMap.end())
		{
			// no valid name found
			return NULL;
		}
		else
		{
			// name found: return the entry
			return getRealConstantEntry(i->second);
		}

	}

	//---------------------------------------------------------------------------
	GpuProgramParameters::IntConstantEntry* GpuProgramParameters::getNamedIntConstantEntry(const String& name)
	{
		// check if name is found
        ParamNameMap::const_iterator i = mParamNameMap.find(name);

        if (i == mParamNameMap.end())
		{
			// no valid name found
			return NULL;
		}
		else
		{
			// name found: return the entry
			return getIntConstantEntry(i->second);
		}

	}

	//---------------------------------------------------------------------------
		void GpuProgramParameters::copyConstantsFrom(const GpuProgramParameters& source)
	{
		// Iterate over fixed parameters
		RealConstantIterator ri = source.getRealConstantIterator();
		ushort i = 0;
		while(ri.hasMoreElements())
		{
			RealConstantEntry re = ri.getNext();
			if (re.isSet)
			{
				setConstant(i, re.val, 4);
			}
			++i;

		}
		IntConstantIterator ii = source.getIntConstantIterator();
		i = 0;
		while (ii.hasMoreElements())
		{
			IntConstantEntry ie = ii.getNext();
			if (ie.isSet)
			{
				setConstant(i, ie.val, 4);
			}
			++i;
		}

		// Iterate over auto parameters
		// Clear existing auto constants
		clearAutoConstants();
		AutoConstantIterator ai = source.getAutoConstantIterator();
		while (ai.hasMoreElements())
		{
			AutoConstantEntry ae = ai.getNext();
			setAutoConstant(ae.index, ae.paramType, ae.data);
		}

		// need to copy Parameter names from the source
		mParamNameMap = source.mParamNameMap;
		
	}
	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------
	String GpuProgram::CmdType::doGet(const void* target) const
	{
		const GpuProgram* t = static_cast<const GpuProgram*>(target);
		if (t->getType() == GPT_VERTEX_PROGRAM)
		{
			return "vertex_program";
		}
		else
		{
			return "fragment_program";
		}
	}
	void GpuProgram::CmdType::doSet(void* target, const String& val)
	{
		GpuProgram* t = static_cast<GpuProgram*>(target);
		if (val == "vertex_program")
		{
			t->setType(GPT_VERTEX_PROGRAM);
		}
		else
		{
			t->setType(GPT_FRAGMENT_PROGRAM);
		}
	}
	//-----------------------------------------------------------------------
	String GpuProgram::CmdSyntax::doGet(const void* target) const
	{
		const GpuProgram* t = static_cast<const GpuProgram*>(target);
		return t->getSyntaxCode();
	}
	void GpuProgram::CmdSyntax::doSet(void* target, const String& val)
	{
		GpuProgram* t = static_cast<GpuProgram*>(target);
		t->setSyntaxCode(val);
	}
	//-----------------------------------------------------------------------
	String GpuProgram::CmdSkeletal::doGet(const void* target) const
	{
		const GpuProgram* t = static_cast<const GpuProgram*>(target);
		return StringConverter::toString(t->isSkeletalAnimationIncluded());
	}
	void GpuProgram::CmdSkeletal::doSet(void* target, const String& val)
	{
		GpuProgram* t = static_cast<GpuProgram*>(target);
		t->setSkeletalAnimationIncluded(StringConverter::parseBool(val));
	}
    //-----------------------------------------------------------------------
    GpuProgramPtr& GpuProgramPtr::operator=(const HighLevelGpuProgramPtr& r)
    {
        // Can assign direct
        if (pRep == r.getPointer())
            return *this;
        release();
        pRep = r.getPointer();
        pUseCount = r.useCountPointer();
        if (pUseCount)
        {
            ++(*pUseCount);
        }
        return *this;
    }

}

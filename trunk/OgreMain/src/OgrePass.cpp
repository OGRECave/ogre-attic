/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
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
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#include "OgrePass.h"
#include "OgreTechnique.h"
#include "OgreMaterialManager.h"
#include "OgreException.h"
#include "OgreGpuProgramUsage.h"
#include "OgreTextureUnitState.h"

namespace Ogre {
	
    //-----------------------------------------------------------------------------
	Pass::Pass(Technique* parent, unsigned short index)
        : mParent(parent), mIndex(index)
    {
        // Default to white ambient & diffuse, no specular / emissive
	    mAmbient = mDiffuse = ColourValue::White;
	    mSpecular = mEmissive = ColourValue::Black;
	    mShininess = 0;
        mHash = 0;

	    // No fog
	    mFogOverride = false;

	    // Default blending (overwrite)
	    mSourceBlendFactor = SBF_ONE;
	    mDestBlendFactor = SBF_ZERO;

	    mDepthCheck = true;
	    mDepthWrite = true;
        mColourWrite = true;
	    mDepthFunc = CMPF_LESS_EQUAL;
        mDepthBias = 0;
	    mCullMode = CULL_CLOCKWISE;
	    mManualCullMode = MANUAL_CULL_BACK;
	    mLightingEnabled = true;
        mMaxSimultaneousLights = OGRE_MAX_SIMULTANEOUS_LIGHTS;
		mRunOncePerLight = false;
        mRunOnlyForOneLightType = true;
        mOnlyLightType = Light::LT_POINT;
	    mShadeOptions = SO_GOURAUD;

		mTextureFiltering = MaterialManager::getSingleton().getDefaultTextureFiltering();
		mMaxAniso = MaterialManager::getSingleton().getDefaultAnisotropy();
		mIsDefFiltering = true;
		mIsDefAniso = true;

		mVertexProgramUsage = NULL;
		mFragmentProgramUsage = NULL;
   }
	
    //-----------------------------------------------------------------------------
	Pass::Pass(Technique *parent, unsigned short index, const Pass& oth)
        :mParent(parent), mIndex(index)
    {
        *this = oth;
        mParent = parent;
        mIndex = index;
    }
    //-----------------------------------------------------------------------------
    Pass::~Pass()
    {
        removeAllTextureUnitStates();
    }
    //-----------------------------------------------------------------------------
    Pass& Pass::operator=(const Pass& oth)
    {
	    mAmbient = oth.mAmbient;
        mDiffuse = oth.mDiffuse;
	    mSpecular = oth.mSpecular;
        mEmissive = oth.mEmissive;
	    mShininess = oth.mShininess;

	    // No fog
	    mFogOverride = oth.mFogOverride;

	    // Default blending (overwrite)
	    mSourceBlendFactor = oth.mSourceBlendFactor;
	    mDestBlendFactor = oth.mDestBlendFactor;

	    mDepthCheck = oth.mDepthCheck;
	    mDepthWrite = oth.mDepthWrite;
        mColourWrite = oth.mColourWrite;
	    mDepthFunc = oth.mDepthFunc;
        mDepthBias = oth.mDepthBias;
	    mCullMode = oth.mCullMode;
	    mManualCullMode = oth.mManualCullMode;
	    mLightingEnabled = oth.mLightingEnabled;
        mMaxSimultaneousLights = oth.mMaxSimultaneousLights;
	    mShadeOptions = oth.mShadeOptions;

		mTextureFiltering = oth.mTextureFiltering;
		mMaxAniso = oth.mMaxAniso;
		mIsDefFiltering = oth.mIsDefFiltering;
		mIsDefAniso = oth.mIsDefAniso;

		if (oth.mVertexProgramUsage)
		{
			mVertexProgramUsage = new GpuProgramUsage(*(oth.mVertexProgramUsage));
		}
		else
		{
		    mVertexProgramUsage = NULL;
		}
		if (oth.mFragmentProgramUsage)
		{
		    mFragmentProgramUsage = new GpuProgramUsage(*(oth.mFragmentProgramUsage));
        }
        else
        {
		    mFragmentProgramUsage = NULL;
        }

		// Copy texture units
		removeAllTextureUnitStates();
		TextureUnitStates::const_iterator i, iend;
		iend = oth.mTextureUnitStates.end();
		for (i = oth.mTextureUnitStates.begin(); i != iend; ++i)
		{
			TextureUnitState* t = new TextureUnitState(this, *(*i));
			mTextureUnitStates.push_back(t);
		}


		return *this;
    }
    //-----------------------------------------------------------------------
    void Pass::setAmbient(Real red, Real green, Real blue)
    {
	    mAmbient.r = red;
	    mAmbient.g = green;
	    mAmbient.b = blue;

    }
    //-----------------------------------------------------------------------
    void Pass::setAmbient(const ColourValue& ambient)
    {
	    mAmbient = ambient;
    }
    //-----------------------------------------------------------------------
    void Pass::setDiffuse(Real red, Real green, Real blue)
    {
	    mDiffuse.r = red;
	    mDiffuse.g = green;
	    mDiffuse.b = blue;
    }
    //-----------------------------------------------------------------------
    void Pass::setDiffuse(const ColourValue& diffuse)
    {
	    mDiffuse = diffuse;
    }
    //-----------------------------------------------------------------------
    void Pass::setSpecular(Real red, Real green, Real blue)
    {
	    mSpecular.r = red;
	    mSpecular.g = green;
	    mSpecular.b = blue;
    }
    //-----------------------------------------------------------------------
    void Pass::setSpecular(const ColourValue& specular)
    {
	    mSpecular = specular;
    }
    //-----------------------------------------------------------------------
    void Pass::setShininess(Real val)
    {
	    mShininess = val;
    }
    //-----------------------------------------------------------------------
    void Pass::setSelfIllumination(Real red, Real green, Real blue)
    {
	    mEmissive.r = red;
	    mEmissive.g = green;
	    mEmissive.b = blue;

    }
    //-----------------------------------------------------------------------
    void Pass::setSelfIllumination(const ColourValue& selfIllum)
    {
	    mEmissive = selfIllum;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Pass::getAmbient(void) const
    {
	    return mAmbient;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Pass::getDiffuse(void) const
    {
	    return mDiffuse;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Pass::getSpecular(void) const
    {
	    return mSpecular;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Pass::getSelfIllumination(void) const
    {
	    return mEmissive;
    }
    //-----------------------------------------------------------------------
    Real Pass::getShininess(void) const
    {
	    return mShininess;
    }
    //-----------------------------------------------------------------------
    TextureUnitState* Pass::createTextureUnitState(void)
    {
        TextureUnitState *t = new TextureUnitState(this);
        mTextureUnitStates.push_back(t);
        // Needs recompilation
        mParent->_notifyNeedsRecompile();
	    return t;
    }
    //-----------------------------------------------------------------------
    TextureUnitState* Pass::createTextureUnitState(
        const String& textureName, unsigned short texCoordSet)
    {
        TextureUnitState *t = new TextureUnitState(this);
	    t->setTextureName(textureName);
	    t->setTextureCoordSet(texCoordSet);
        mTextureUnitStates.push_back(t);
        // Needs recompilation
        mParent->_notifyNeedsRecompile();
	    return t;
    }
    //-----------------------------------------------------------------------
	void Pass::addTextureUnitState(TextureUnitState* state)
	{
		mTextureUnitStates.push_back(state);
        // Needs recompilation
        mParent->_notifyNeedsRecompile();
	}
    //-----------------------------------------------------------------------
    TextureUnitState* Pass::getTextureUnitState(unsigned short index) 
    {
        assert (index < mTextureUnitStates.size() && "Index out of bounds");
	    return mTextureUnitStates[index];
    }
    //-----------------------------------------------------------------------
    Pass::TextureUnitStateIterator
        Pass::getTextureUnitStateIterator(void)
    {
        return TextureUnitStateIterator(mTextureUnitStates.begin(), mTextureUnitStates.end());
    }
    //-----------------------------------------------------------------------
    void Pass::removeTextureUnitState(unsigned short index)
    {
        assert (index < mTextureUnitStates.size() && "Index out of bounds");

        TextureUnitStates::iterator i = mTextureUnitStates.begin() + index;
        delete *i;
	    mTextureUnitStates.erase(i);
        // Needs recompilation
        mParent->_notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Pass::removeAllTextureUnitStates(void)
    {
        TextureUnitStates::iterator i, iend;
        iend = mTextureUnitStates.end();
        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            delete *i;
        }
        mTextureUnitStates.clear();
        // Needs recompilation
        mParent->_notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Pass::setSceneBlending(SceneBlendType sbt)
    {
	    // Turn predefined type into blending factors
	    switch (sbt)
	    {
	    case SBT_TRANSPARENT_ALPHA:
		    setSceneBlending(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA);
		    break;
	    case SBT_TRANSPARENT_COLOUR:
		    setSceneBlending(SBF_SOURCE_COLOUR, SBF_ONE_MINUS_SOURCE_COLOUR);
		    break;
	    case SBT_ADD:
		    setSceneBlending(SBF_ONE, SBF_ONE);
		    break;
	    // TODO: more
	    }

    }
    //-----------------------------------------------------------------------
    void Pass::setSceneBlending(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor)
    {
	    mSourceBlendFactor = sourceFactor;
	    mDestBlendFactor = destFactor;
    }
    //-----------------------------------------------------------------------
    SceneBlendFactor Pass::getSourceBlendFactor(void) const
    {
	    return mSourceBlendFactor;
    }
    //-----------------------------------------------------------------------
    SceneBlendFactor Pass::getDestBlendFactor(void) const
    {
	    return mDestBlendFactor;
    }
    //-----------------------------------------------------------------------
    bool Pass::isTransparent(void) const
    {
		// Transparent if any of the destination colour is taken into account
	    if (mDestBlendFactor != SBF_ZERO)
		    return true;
	    else
		    return false;
    }
    //-----------------------------------------------------------------------
    void Pass::setDepthCheckEnabled(bool enabled)
    {
	    mDepthCheck = enabled;
    }
    //-----------------------------------------------------------------------
    bool Pass::getDepthCheckEnabled(void) const
    {
	    return mDepthCheck;
    }
    //-----------------------------------------------------------------------
    void Pass::setDepthWriteEnabled(bool enabled)
    {
	    mDepthWrite = enabled;
    }
    //-----------------------------------------------------------------------
    bool Pass::getDepthWriteEnabled(void) const
    {
	    return mDepthWrite;
    }
    //-----------------------------------------------------------------------
    void Pass::setDepthFunction( CompareFunction func)
    {
	    mDepthFunc = func;
    }
    //-----------------------------------------------------------------------
    CompareFunction Pass::getDepthFunction(void) const
    {
	    return mDepthFunc;
    }
    //-----------------------------------------------------------------------
	void Pass::setColourWriteEnabled(bool enabled)
	{
		mColourWrite = enabled;
	}
    //-----------------------------------------------------------------------
	bool Pass::getColourWriteEnabled(void) const
	{
		return mColourWrite;
	}
    //-----------------------------------------------------------------------
    void Pass::setCullingMode( CullingMode mode)
    {
	    mCullMode = mode;
    }
    //-----------------------------------------------------------------------
    CullingMode Pass::getCullingMode(void) const
    {
	    return mCullMode;
    }
    //-----------------------------------------------------------------------
    void Pass::setLightingEnabled(bool enabled)
    {
	    mLightingEnabled = enabled;
    }
    //-----------------------------------------------------------------------
    bool Pass::getLightingEnabled(void) const
    {
	    return mLightingEnabled;
    }
    //-----------------------------------------------------------------------
    void Pass::setMaxSimultaneousLights(unsigned short maxLights)
    {
        mMaxSimultaneousLights = maxLights;
    }
    //-----------------------------------------------------------------------
    unsigned short Pass::getMaxSimultaneousLights(void) const
    {
        return mMaxSimultaneousLights;
    }
    //-----------------------------------------------------------------------
    void Pass::setRunOncePerLight(bool enabled, 
            bool onlyForOneLightType, Light::LightTypes lightType)
    {
        mRunOncePerLight = enabled;
        mRunOnlyForOneLightType = onlyForOneLightType;
        mOnlyLightType = lightType;
    }
    //-----------------------------------------------------------------------
    void Pass::setShadingMode(ShadeOptions mode)
    {
	    mShadeOptions = mode;
    }
    //-----------------------------------------------------------------------
    ShadeOptions Pass::getShadingMode(void) const
    {
	    return mShadeOptions;
    }
    //-----------------------------------------------------------------------
    void Pass::setManualCullingMode(ManualCullingMode mode)
    {
	    mManualCullMode = mode;
    }
    //-----------------------------------------------------------------------
    ManualCullingMode Pass::getManualCullingMode(void) const
    {
	    return mManualCullMode;
    }
    //-----------------------------------------------------------------------
    void Pass::setFog(bool overrideScene, FogMode mode, const ColourValue& colour, Real density, Real start, Real end)
    {
	    mFogOverride = overrideScene;
	    if (overrideScene)
	    {
		    mFogMode = mode;
		    mFogColour = colour;
		    mFogStart = start;
		    mFogEnd = end;
		    mFogDensity = density;
	    }
    }
    //-----------------------------------------------------------------------
    bool Pass::getFogOverride(void) const
    {
	    return mFogOverride;
    }
    //-----------------------------------------------------------------------
    FogMode Pass::getFogMode(void) const
    {
	    return mFogMode;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Pass::getFogColour(void) const
    {
	    return mFogColour;
    }
    //-----------------------------------------------------------------------
    Real Pass::getFogStart(void) const
    {
	    return mFogStart;
    }
    //-----------------------------------------------------------------------
    Real Pass::getFogEnd(void) const
    {
	    return mFogEnd;
    }
    //-----------------------------------------------------------------------
    Real Pass::getFogDensity(void) const
    {
	    return mFogDensity;
    }
    //-----------------------------------------------------------------------
    void Pass::setDepthBias(ushort bias)
    {
        assert(bias <= 16 && "Depth bias must be between 0 and 16");
        mDepthBias = bias;
    }
    //-----------------------------------------------------------------------
    ushort Pass::getDepthBias(void) const
    {
        return mDepthBias;
    }
    //-----------------------------------------------------------------------
	Pass* Pass::_split(unsigned short numUnits)
	{
		if (mFragmentProgramUsage)
		{
			Except(Exception::ERR_INVALIDPARAMS, "Passes with fragment programs cannot be "
				"automatically split, define a fallback technique instead.",
				"Pass:_split");
		}

		if (mTextureUnitStates.size() > numUnits)
		{
			size_t start = mTextureUnitStates.size() - numUnits;
			
			Pass* newPass = mParent->createPass();

			TextureUnitStates::iterator istart, i, iend;
			iend = mTextureUnitStates.end();
			i = istart = mTextureUnitStates.begin() + start;
			// Set the new pass to fallback using scene blend
			newPass->setSceneBlending(
				(*i)->getColourBlendFallbackSrc(), (*i)->getColourBlendFallbackDest());
			// Add all the other passes
			for (; i != iend; ++i)
			{
				newPass->addTextureUnitState(*i);
			}
			// Now remove texture units from this Pass, we don't need to delete since they've
			// been transferred
			mTextureUnitStates.erase(istart, iend);
			return newPass;
		}
		return NULL;
	}
    //-----------------------------------------------------------------------
	void Pass::_load(void)
	{
		// We assume the Technique only calls this when the material is being
		// loaded

		// Load each TextureUnitState
		TextureUnitStates::iterator i, iend;
		iend = mTextureUnitStates.end();
		for (i = mTextureUnitStates.begin(); i != iend; ++i)
		{
			(*i)->_load();
		}

		// Load programs
		if (mVertexProgramUsage)
		{
			// Load vertex program
            mVertexProgramUsage->_load();
        }
        if (mFragmentProgramUsage)
        {
			// Load fragment program
            mFragmentProgramUsage->_load();
		}

        // Recalculate hash
        _recalculateHash();
		
	}
    //-----------------------------------------------------------------------
	void Pass::_unload(void)
	{
		// Unload each TextureUnitState
		TextureUnitStates::iterator i, iend;
		iend = mTextureUnitStates.end();
		for (i = mTextureUnitStates.begin(); i != iend; ++i)
		{
			(*i)->_unload();
		}

		// Unload programs
		if (mVertexProgramUsage)
		{
			// TODO
		}
        if (mFragmentProgramUsage)
        {
            // TODO
        }
	}
    //-----------------------------------------------------------------------
	void Pass::setVertexProgram(const String& name)
	{
        // Turn off vertex program if name blank
        if (name.empty())
        {
            if (mVertexProgramUsage) delete mVertexProgramUsage;
            mVertexProgramUsage = NULL;
        }
        else
        {
            if (!mVertexProgramUsage)
            {
                mVertexProgramUsage = new GpuProgramUsage(GPT_VERTEX_PROGRAM);
            }
		    mVertexProgramUsage->setProgramName(name);
        }
        // Needs recompilation
        mParent->_notifyNeedsRecompile();
	}
    //-----------------------------------------------------------------------
	void Pass::setVertexProgramParameters(GpuProgramParametersSharedPtr params)
	{
		if (!mVertexProgramUsage)
        {
            Except (Exception::ERR_INVALIDPARAMS, 
                "This pass does not have a vertex program assigned!", 
                "Pass::setVertexProgramParameters");
        }
		mVertexProgramUsage->setParameters(params);
	}
    //-----------------------------------------------------------------------
	void Pass::setFragmentProgram(const String& name)
	{
        // Turn off fragment program if name blank
        if (name.empty())
        {
            if (mFragmentProgramUsage) delete mFragmentProgramUsage;
            mFragmentProgramUsage = NULL;
        }
        else
        {
            if (!mFragmentProgramUsage)
            {
                mFragmentProgramUsage = new GpuProgramUsage(GPT_FRAGMENT_PROGRAM);
            }
		    mFragmentProgramUsage->setProgramName(name);
        }
        // Needs recompilation
        mParent->_notifyNeedsRecompile();
	}
    //-----------------------------------------------------------------------
	void Pass::setFragmentProgramParameters(GpuProgramParametersSharedPtr params)
	{
		if (!mFragmentProgramUsage)
        {
            Except (Exception::ERR_INVALIDPARAMS, 
                "This pass does not have a fragment program assigned!", 
                "Pass::setFragmentProgramParameters");
        }
		mFragmentProgramUsage->setParameters(params);
	}
	//-----------------------------------------------------------------------
	const String& Pass::getVertexProgramName(void)
	{
        if (!mVertexProgramUsage)
            return String::BLANK;
        else
		    return mVertexProgramUsage->getProgramName();
	}
	//-----------------------------------------------------------------------
	GpuProgramParametersSharedPtr Pass::getVertexProgramParameters(void)
	{
		if (!mVertexProgramUsage)
        {
            Except (Exception::ERR_INVALIDPARAMS, 
                "This pass does not have a vertex program assigned!", 
                "Pass::getVertexProgramParameters");
        }
		return mVertexProgramUsage->getParameters();
	}
	//-----------------------------------------------------------------------
	GpuProgram* Pass::getVertexProgram(void)
	{
		return mVertexProgramUsage->getProgram();
	}
	//-----------------------------------------------------------------------
	const String& Pass::getFragmentProgramName(void)
	{
		return mFragmentProgramUsage->getProgramName();
	}
	//-----------------------------------------------------------------------
	GpuProgramParametersSharedPtr Pass::getFragmentProgramParameters(void)
	{
		return mFragmentProgramUsage->getParameters();
	}
	//-----------------------------------------------------------------------
	GpuProgram* Pass::getFragmentProgram(void)
	{
		return mFragmentProgramUsage->getProgram();
	}
	//-----------------------------------------------------------------------
    bool Pass::isLoaded(void) const
    {
        return mParent->isLoaded();
    }
	//-----------------------------------------------------------------------
    unsigned long Pass::getHash(void) const
    {
        return mHash;
    }
	//-----------------------------------------------------------------------
    void Pass::_recalculateHash(void)
    {
        /* Hash format is 32-bit, divided as follows (high to low bits)
           bits   purpose
            4     Pass index (i.e. max 16 passes!)
           14     Hashed texture name from unit 0
           14     Hashed texture name from unit 1

           Note that at the moment we don't sort on the 3rd texture unit plus
           on the assumption that these are less frequently used; sorting on 
           the first 2 gives us the most benefit for now.
       */
        _StringHash H;
        mHash = (mIndex << 28);
        size_t c = getNumTextureUnitStates();

        if (c)
            mHash += (H(mTextureUnitStates[0]->getTextureName()) % (1 << 14)) << 14;
        if (c > 1)
            mHash += (H(mTextureUnitStates[1]->getTextureName()) % (1 << 14));
    }

    //-----------------------------------------------------------------------
    void Pass::_notifyNeedsRecompile(void)
    {
        mParent->_notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Pass::setTextureFiltering(TextureFilterOptions filterType)
    {
        TextureUnitStates::iterator i, iend;
        iend = mTextureUnitStates.end();
        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            (*i)->setTextureFiltering(filterType);
        }
    }
    // --------------------------------------------------------------------
    void Pass::setTextureAnisotropy(int maxAniso)
    {
        TextureUnitStates::iterator i, iend;
        iend = mTextureUnitStates.end();
        for (i = mTextureUnitStates.begin(); i != iend; ++i)
        {
            (*i)->setTextureAnisotropy(maxAniso);
        }
    }
    //-----------------------------------------------------------------------
    void Pass::_updateAutoParamsNoLights(const AutoParamDataSource& source)
    {
        if (hasVertexProgram())
        {
            // Update vertex program auto params
            mVertexProgramUsage->getParameters()->_updateAutoParamsNoLights(source);
        }

        if (hasFragmentProgram())
        {
            // Update fragment program auto params
            mFragmentProgramUsage->getParameters()->_updateAutoParamsNoLights(source);
        }
    }
    //-----------------------------------------------------------------------
    void Pass::_updateAutoParamsLightsOnly(const AutoParamDataSource& source)
    {
        if (hasVertexProgram())
        {
            // Update vertex program auto params
            mVertexProgramUsage->getParameters()->_updateAutoParamsLightsOnly(source);
        }

        if (hasFragmentProgram())
        {
            // Update fragment program auto params
            mFragmentProgramUsage->getParameters()->_updateAutoParamsLightsOnly(source);
        }
    }

}

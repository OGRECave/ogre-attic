/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2003 The OGRE Team
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
	Pass::PassSet Pass::msDirtyHashList;
    Pass::PassSet Pass::msPassGraveyard;
    //-----------------------------------------------------------------------------
	Pass::Pass(Technique* parent, unsigned short index)
        : mParent(parent), mIndex(index)
    {
        // Default to white ambient & diffuse, no specular / emissive
	    mAmbient = mDiffuse = ColourValue::White;
	    mSpecular = mEmissive = ColourValue::Black;
	    mShininess = 0;
        mHash = 0;

        // By default, don't override the scene's fog settings
        mFogOverride = false;
        mFogMode = FOG_NONE;
        mFogColour = ColourValue::White;
        mFogStart = 0.0;
        mFogEnd = 1.0;
        mFogDensity = 0.001;

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

		mVertexProgramUsage = NULL;
        mShadowCasterVertexProgramUsage = NULL;
        mShadowReceiverVertexProgramUsage = NULL;
		mFragmentProgramUsage = NULL;

        mQueuedForDeletion = false;

        _dirtyHash();
   }
	
    //-----------------------------------------------------------------------------
	Pass::Pass(Technique *parent, unsigned short index, const Pass& oth)
        :mParent(parent), mIndex(index)
    {
        *this = oth;
        mParent = parent;
        mIndex = index;
        mQueuedForDeletion = false;
        _dirtyHash();
    }
    //-----------------------------------------------------------------------------
    Pass::~Pass()
    {

    }
    //-----------------------------------------------------------------------------
    Pass& Pass::operator=(const Pass& oth)
    {
	    mAmbient = oth.mAmbient;
        mDiffuse = oth.mDiffuse;
	    mSpecular = oth.mSpecular;
        mEmissive = oth.mEmissive;
	    mShininess = oth.mShininess;

        // Copy fog parameters
        mFogOverride = oth.mFogOverride;
        mFogMode = oth.mFogMode;
        mFogColour = oth.mFogColour;
        mFogStart = oth.mFogStart;
        mFogEnd = oth.mFogEnd;
        mFogDensity = oth.mFogDensity;

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
		mRunOncePerLight = oth.mRunOncePerLight;
        mRunOnlyForOneLightType = oth.mRunOnlyForOneLightType;
        mOnlyLightType = oth.mOnlyLightType;
	    mShadeOptions = oth.mShadeOptions;

		if (oth.mVertexProgramUsage)
		{
			mVertexProgramUsage = new GpuProgramUsage(*(oth.mVertexProgramUsage));
		}
		else
		{
		    mVertexProgramUsage = NULL;
		}
        if (oth.mShadowCasterVertexProgramUsage)
        {
            mShadowCasterVertexProgramUsage = new GpuProgramUsage(*(oth.mShadowCasterVertexProgramUsage));
        }
        else
        {
            mShadowCasterVertexProgramUsage = NULL;
        }
        if (oth.mShadowReceiverVertexProgramUsage)
        {
            mShadowReceiverVertexProgramUsage = new GpuProgramUsage(*(oth.mShadowReceiverVertexProgramUsage));
        }
        else
        {
            mShadowReceiverVertexProgramUsage = NULL;
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

        _dirtyHash();

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
    void Pass::setDiffuse(Real red, Real green, Real blue, Real alpha)
    {
	    mDiffuse.r = red;
	    mDiffuse.g = green;
	    mDiffuse.b = blue;
		mDiffuse.a = alpha;
    }
    //-----------------------------------------------------------------------
    void Pass::setDiffuse(const ColourValue& diffuse)
    {
	    mDiffuse = diffuse;
    }
    //-----------------------------------------------------------------------
    void Pass::setSpecular(Real red, Real green, Real blue, Real alpha)
    {
	    mSpecular.r = red;
	    mSpecular.g = green;
	    mSpecular.b = blue;
		mSpecular.a = alpha;
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
        _dirtyHash();
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
        _dirtyHash();
	    return t;
    }
    //-----------------------------------------------------------------------
	void Pass::addTextureUnitState(TextureUnitState* state)
	{
		mTextureUnitStates.push_back(state);
        // Needs recompilation
        mParent->_notifyNeedsRecompile();
        _dirtyHash();
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
        if (!mQueuedForDeletion)
        {
            // Needs recompilation
            mParent->_notifyNeedsRecompile();
        }
        _dirtyHash();
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
        if (!mQueuedForDeletion)
        {        
            // Needs recompilation
            mParent->_notifyNeedsRecompile();
        }
        _dirtyHash();
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
        if (mShadowCasterVertexProgramUsage)
        {
            // Load vertex program
            mShadowCasterVertexProgramUsage->_load();
        }
        if (mShadowReceiverVertexProgramUsage)
        {
            // Load vertex program
            mShadowReceiverVertexProgramUsage->_load();
        }

        if (mFragmentProgramUsage)
        {
			// Load fragment program
            mFragmentProgramUsage->_load();
		}

        // Recalculate hash
        _dirtyHash();
		
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
	void Pass::setVertexProgram(const String& name, bool resetParams)
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
		    mVertexProgramUsage->setProgramName(name, resetParams);
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
	void Pass::setFragmentProgram(const String& name, bool resetParams)
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
		    mFragmentProgramUsage->setProgramName(name, resetParams);
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
	const String& Pass::getVertexProgramName(void) const
	{
        if (!mVertexProgramUsage)
            return StringUtil::BLANK;
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
	const GpuProgramPtr& Pass::getVertexProgram(void)
	{
		return mVertexProgramUsage->getProgram();
	}
	//-----------------------------------------------------------------------
	const String& Pass::getFragmentProgramName(void) const
	{
		return mFragmentProgramUsage->getProgramName();
	}
	//-----------------------------------------------------------------------
	GpuProgramParametersSharedPtr Pass::getFragmentProgramParameters(void)
	{
		return mFragmentProgramUsage->getParameters();
	}
	//-----------------------------------------------------------------------
	const GpuProgramPtr& Pass::getFragmentProgram(void)
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

        if (c && !mTextureUnitStates[0]->isBlank())
            mHash += (H(mTextureUnitStates[0]->getTextureName()) % (1 << 14)) << 14;
        if (c > 1 && !mTextureUnitStates[1]->isBlank())
            mHash += (H(mTextureUnitStates[1]->getTextureName()) % (1 << 14));
    }
    //-----------------------------------------------------------------------
	void Pass::_dirtyHash(void)
	{
		// Mark this hash as for follow up
		msDirtyHashList.insert(this);
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
    void Pass::setTextureAnisotropy(unsigned int maxAniso)
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
    //-----------------------------------------------------------------------
    void Pass::processPendingPassUpdates(void)
    {
        // Delete items in the graveyard
        PassSet::iterator i, iend;
        iend = msPassGraveyard.end();
        for (i = msPassGraveyard.begin(); i != iend; ++i)
        {
            delete *i;
        }
        msPassGraveyard.clear();

        // The dirty ones will have been removed from the groups above using the old hash now
        iend = msDirtyHashList.end();
        for (i = msDirtyHashList.begin(); i != iend; ++i)
        {
            Pass* p = *i;
            p->_recalculateHash();
        }
        // Clear the dirty list
        msDirtyHashList.clear();
    }
    //-----------------------------------------------------------------------
    void Pass::queueForDeletion(void)
    {
        mQueuedForDeletion = true;

        removeAllTextureUnitStates();
        if (mVertexProgramUsage)
        {
            delete mVertexProgramUsage;
            mVertexProgramUsage = 0;
        }
        if (mShadowCasterVertexProgramUsage)
        {
            delete mShadowCasterVertexProgramUsage;
            mShadowCasterVertexProgramUsage = 0;
        }
        if (mShadowReceiverVertexProgramUsage)
        {
            delete mShadowReceiverVertexProgramUsage;
            mShadowReceiverVertexProgramUsage = 0;
        }
        if (mFragmentProgramUsage)
        {
            delete mFragmentProgramUsage;
            mFragmentProgramUsage = 0;
        }
        // remove from dirty list, if there
        msDirtyHashList.erase(this);

        msPassGraveyard.insert(this);
    }
    //-----------------------------------------------------------------------
    bool Pass::isAmbientOnly(void) const
    {
        // treat as ambient if lighting is off, or colour write is off, 
        // or all non-ambient (& emissive) colours are black
        // NB a vertex program could override this, but passes using vertex
        // programs are expected to indicate they are ambient only by 
        // setting the state so it matches one of the conditions above, even 
        // though this state is not used in rendering.
        return (!mLightingEnabled || !mColourWrite ||
            (mDiffuse == ColourValue::Black && 
             mSpecular == ColourValue::Black));
    }
    //-----------------------------------------------------------------------
    void Pass::setShadowCasterVertexProgram(const String& name)
    {
        // Turn off vertex program if name blank
        if (name.empty())
        {
            if (mShadowCasterVertexProgramUsage) delete mShadowCasterVertexProgramUsage;
            mShadowCasterVertexProgramUsage = NULL;
        }
        else
        {
            if (!mShadowCasterVertexProgramUsage)
            {
                mShadowCasterVertexProgramUsage = new GpuProgramUsage(GPT_VERTEX_PROGRAM);
            }
            mShadowCasterVertexProgramUsage->setProgramName(name);
        }
        // Needs recompilation
        mParent->_notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Pass::setShadowCasterVertexProgramParameters(GpuProgramParametersSharedPtr params)
    {
        if (!mShadowCasterVertexProgramUsage)
        {
            Except (Exception::ERR_INVALIDPARAMS, 
                "This pass does not have a shadow caster vertex program assigned!", 
                "Pass::setShadowCasterVertexProgramParameters");
        }
        mShadowCasterVertexProgramUsage->setParameters(params);
    }
    //-----------------------------------------------------------------------
    const String& Pass::getShadowCasterVertexProgramName(void) const
    {
        if (!mShadowCasterVertexProgramUsage)
            return StringUtil::BLANK;
        else
            return mShadowCasterVertexProgramUsage->getProgramName();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getShadowCasterVertexProgramParameters(void)
    {
        if (!mShadowCasterVertexProgramUsage)
        {
            Except (Exception::ERR_INVALIDPARAMS, 
                "This pass does not have a shadow caster vertex program assigned!", 
                "Pass::getShadowCasterVertexProgramParameters");
        }
        return mShadowCasterVertexProgramUsage->getParameters();
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getShadowCasterVertexProgram(void)
    {
        return mShadowCasterVertexProgramUsage->getProgram();
    }
    //-----------------------------------------------------------------------
    void Pass::setShadowReceiverVertexProgram(const String& name)
    {
        // Turn off vertex program if name blank
        if (name.empty())
        {
            if (mShadowReceiverVertexProgramUsage) delete mShadowReceiverVertexProgramUsage;
            mShadowReceiverVertexProgramUsage = NULL;
        }
        else
        {
            if (!mShadowReceiverVertexProgramUsage)
            {
                mShadowReceiverVertexProgramUsage = new GpuProgramUsage(GPT_VERTEX_PROGRAM);
            }
            mShadowReceiverVertexProgramUsage->setProgramName(name);
        }
        // Needs recompilation
        mParent->_notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    void Pass::setShadowReceiverVertexProgramParameters(GpuProgramParametersSharedPtr params)
    {
        if (!mShadowReceiverVertexProgramUsage)
        {
            Except (Exception::ERR_INVALIDPARAMS, 
                "This pass does not have a shadow receiver vertex program assigned!", 
                "Pass::setShadowReceiverVertexProgramParameters");
        }
        mShadowReceiverVertexProgramUsage->setParameters(params);
    }
    //-----------------------------------------------------------------------
    const String& Pass::getShadowReceiverVertexProgramName(void) const
    {
        if (!mShadowReceiverVertexProgramUsage)
            return StringUtil::BLANK;
        else
            return mShadowReceiverVertexProgramUsage->getProgramName();
    }
    //-----------------------------------------------------------------------
    GpuProgramParametersSharedPtr Pass::getShadowReceiverVertexProgramParameters(void)
    {
        if (!mShadowReceiverVertexProgramUsage)
        {
            Except (Exception::ERR_INVALIDPARAMS, 
                "This pass does not have a shadow receiver vertex program assigned!", 
                "Pass::getShadowReceiverVertexProgramParameters");
        }
        return mShadowReceiverVertexProgramUsage->getParameters();
    }
    //-----------------------------------------------------------------------
    const GpuProgramPtr& Pass::getShadowReceiverVertexProgram(void)
    {
        return mShadowReceiverVertexProgramUsage->getProgram();
    }
    //-----------------------------------------------------------------------
	const String& Pass::getResourceGroup(void) const
	{
		return mParent->getResourceGroup();
	}

}

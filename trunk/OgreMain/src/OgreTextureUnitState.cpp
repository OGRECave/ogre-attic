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

#include "OgreTextureUnitState.h"
#include "OgrePass.h"
#include "OgreMaterialManager.h"
#include "OgreControllerManager.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreTextureManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    TextureUnitState::TextureUnitState(Pass* parent)
        : mParent(parent)
    {
        mIsBlank = true;
        colourBlendMode.blendType = LBT_COLOUR;
        setColourOperation(LBO_MODULATE);
        setTextureAddressingMode(TAM_WRAP);

        alphaBlendMode.operation = LBX_MODULATE;
        alphaBlendMode.blendType = LBT_ALPHA;
        alphaBlendMode.source1 = LBS_TEXTURE;
        alphaBlendMode.source2 = LBS_CURRENT;
		
		//default filtering
		mMinFilter = FO_LINEAR;
		mMagFilter = FO_LINEAR;
		mMipFilter = FO_POINT;
		mMaxAniso = MaterialManager::getSingleton().getDefaultAnisotropy();
        mIsDefaultAniso = true;
        mIsDefaultFiltering = true;

		mUMod = mVMod = 0;
        mUScale = mVScale = 1;
        mRotate = 0;
        mTexModMatrix = Matrix4::IDENTITY;
        mRecalcTexMatrix = false;
        mAlphaRejectFunc = CMPF_ALWAYS_PASS;
        mAlphaRejectVal = 0;

        mNumFrames = 0;
        mAnimDuration = 0;
        mAnimController = 0;
        mCubic = false;
        mTextureType = TEX_TYPE_2D;
        mTextureCoordSetIndex = 0;

        mFrames[0] = StringUtil::BLANK;
        mCurrentFrame = 0;

        mParent->_dirtyHash();

    }

    //-----------------------------------------------------------------------
    TextureUnitState::TextureUnitState(Pass* parent, const TextureUnitState& oth )
    {
        mParent = parent;
        *this = oth;
    }

    //-----------------------------------------------------------------------
    TextureUnitState::TextureUnitState( Pass* parent, const String& texName, unsigned int texCoordSet)
        :mParent(parent)
    {
        mIsBlank = true;
        colourBlendMode.blendType = LBT_COLOUR;
        setColourOperation(LBO_MODULATE);
        setTextureAddressingMode(TAM_WRAP);

        alphaBlendMode.operation = LBX_MODULATE;
        alphaBlendMode.blendType = LBT_ALPHA;
        alphaBlendMode.source1 = LBS_TEXTURE;
        alphaBlendMode.source2 = LBS_CURRENT;

		//default filtering && anisotropy
		mMinFilter = FO_LINEAR;
		mMagFilter = FO_LINEAR;
		mMipFilter = FO_POINT;
		mMaxAniso = MaterialManager::getSingleton().getDefaultAnisotropy();
        mIsDefaultAniso = true;
        mIsDefaultFiltering = true;

		mUMod = mVMod = 0;
        mUScale = mVScale = 1;
        mRotate = 0;
        mAnimDuration = 0;
        mAnimController = 0;
        mTexModMatrix = Matrix4::IDENTITY;
        mRecalcTexMatrix = false;
        mAlphaRejectFunc = CMPF_ALWAYS_PASS;
        mAlphaRejectVal = 0;

        mCubic = false;
        mTextureType = TEX_TYPE_2D;
        mTextureCoordSetIndex = 0;

        setTextureName(texName);
        setTextureCoordSet(texCoordSet);

        mParent->_dirtyHash();

    }
    //-----------------------------------------------------------------------
    TextureUnitState::~TextureUnitState()
    {
        // Destroy controllers
        if (mAnimController)
        {
            ControllerManager::getSingleton().destroyController(mAnimController);
        }
        // Destroy effect controllers
        for (EffectMap::iterator i = mEffects.begin(); i != mEffects.end(); ++i)
        {
            if (i->second.controller)
            {
                ControllerManager::getSingleton().destroyController(i->second.controller);
            }

        }
        // Don't unload textures. may be used elsewhere

    }
    //-----------------------------------------------------------------------
    TextureUnitState & TextureUnitState::operator = ( 
        const TextureUnitState &oth )
    {
        // copy basic members (int's, real's)
        memcpy( this, &oth, (uchar *)(&oth.mFrames[0]) - (uchar *)(&oth) );

        // copy complex members
        for( ushort i = 0; i<mNumFrames; i++ )
            mFrames[i] = oth.mFrames[i];

        mEffects = oth.mEffects;

        mParent->_dirtyHash();

        return *this;
    }
    //-----------------------------------------------------------------------
    const String& TextureUnitState::getTextureName(void) const
    {
        // Return name of current frame
        return mFrames[mCurrentFrame];
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureName( const String& name, TextureType texType)
    {
        if (texType == TEX_TYPE_CUBE_MAP)
        {
            // delegate to cubic texture implementation
            setCubicTextureName(name, true);
        }
        else
        {
            mFrames[0] = name;
            mNumFrames = 1;
            mCurrentFrame = 0;
            mCubic = false;
            mTextureType = texType;

            if (name == "")
            {
                mIsBlank = true;
                return;
            }
            
            // Load immediately ?
            if (isLoaded())
            {
                _load(); // reload
                // Tell parent to recalculate hash
                mParent->_dirtyHash();
            }
        }

    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setCubicTextureName( const String& name, bool forUVW)
    {
        if (forUVW)
        {
            setCubicTextureName(&name, forUVW);
        }
        else
        {
            String ext;
            String suffixes[6] = {"_fr", "_bk", "_lf", "_rt", "_up", "_dn"};
            String baseName;
            String fullNames[6];


            size_t pos = name.find_last_of(".");
            baseName = name.substr(0, pos);
            ext = name.substr(pos);

            for (int i = 0; i < 6; ++i)
            {
                fullNames[i] = baseName + suffixes[i] + ext;
            }

            setCubicTextureName(fullNames, forUVW);
        }
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setCubicTextureName(const String* const names, bool forUVW)
    {
        mNumFrames = forUVW ? 1 : 6;
        mCurrentFrame = 0;
        mCubic = true;
        mTextureType = forUVW ? TEX_TYPE_CUBE_MAP : TEX_TYPE_2D;

        for (unsigned int i = 0; i < mNumFrames; ++i)
        {
            mFrames[i] = names[i];
        }
        // Tell parent we need recompiling, will cause reload too
        mParent->_notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    bool TextureUnitState::isCubic(void) const
    {
        return mCubic;
    }
    //-----------------------------------------------------------------------
    bool TextureUnitState::is3D(void) const
    {
        return mTextureType == TEX_TYPE_CUBE_MAP;
    }
    //-----------------------------------------------------------------------
    TextureType TextureUnitState::getTextureType(void) const
    {
        return mTextureType;

    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setAnimatedTextureName( const String& name, unsigned int numFrames, Real duration)
    {
        String ext;
        String baseName;

        size_t pos = name.find_last_of(".");
        baseName = name.substr(0, pos);
        ext = name.substr(pos);

        if (numFrames > MAX_FRAMES)
        {
            char cmsg[128];
            sprintf(cmsg, "Maximum number of frames is %d.", MAX_FRAMES);
            Except(Exception::ERR_INVALIDPARAMS, cmsg, "TextureUnitState::setAnimatedTextureName");
        }
        mNumFrames = numFrames;
        mAnimDuration = duration;
        mCurrentFrame = 0;
        mCubic = false;

        for (unsigned int i = 0; i < mNumFrames; ++i)
        {
            char suffix[5];
            sprintf(suffix, "_%d", i);

            mFrames[i] = baseName + suffix + ext;
        }

        // Load immediately if Material loaded
        if (isLoaded())
        {
            _load();
            // Tell parent to recalculate hash
            mParent->_dirtyHash();
        }

    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setAnimatedTextureName(const String* const names, unsigned int numFrames, Real duration)
    {
        if (numFrames > MAX_FRAMES)
        {
            char cmsg[128];
            sprintf(cmsg, "Maximum number of frames is %d.", MAX_FRAMES);
            Except(Exception::ERR_INVALIDPARAMS, cmsg, "TextureUnitState::setAnimatedTextureName");
        }
        mNumFrames = numFrames;
        mAnimDuration = duration;
        mCurrentFrame = 0;
        mCubic = false;

        for (unsigned int i = 0; i < mNumFrames; ++i)
        {
            mFrames[i] = names[i];
        }

        // Load immediately if Material loaded
        if (isLoaded())
        {
            _load();
            // Tell parent to recalculate hash
            mParent->_dirtyHash();
        }
    }
    //-----------------------------------------------------------------------
    std::pair< uint, uint > TextureUnitState::getTextureDimensions( unsigned int frame ) const
    {
        Texture *tex = (Texture *)TextureManager::getSingleton().getByName( mFrames[ frame ] );
		if (!tex)
			Except( Exception::ERR_ITEM_NOT_FOUND, "Could not find texture " + mFrames[ frame ],
				"TextureUnitState::getTextureDimensions" );
        return std::pair< uint, uint >( tex->getWidth(), tex->getHeight() );
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setCurrentFrame(unsigned int frameNumber)
    {
        assert(frameNumber < mNumFrames);
        mCurrentFrame = frameNumber;
        // this will affect the hash
        mParent->_dirtyHash();

    }
    //-----------------------------------------------------------------------
    unsigned int TextureUnitState::getCurrentFrame(void) const
    {
        return mCurrentFrame;
    }
    //-----------------------------------------------------------------------
    unsigned int TextureUnitState::getNumFrames(void) const
    {
        return mNumFrames;
    }
    //-----------------------------------------------------------------------
    const String& TextureUnitState::getFrameTextureName(unsigned int frameNumber) const
    {
        assert(frameNumber < mNumFrames);
        return mFrames[frameNumber];
    }
    //-----------------------------------------------------------------------
    unsigned int TextureUnitState::getTextureCoordSet(void) const
    {
        return mTextureCoordSetIndex;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureCoordSet(unsigned int set)
    {
        mTextureCoordSetIndex = set;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setColourOperationEx(LayerBlendOperationEx op,
        LayerBlendSource source1,
        LayerBlendSource source2,
        const ColourValue& arg1,
        const ColourValue& arg2,
        Real manualBlend)
    {
        colourBlendMode.operation = op;
        colourBlendMode.source1 = source1;
        colourBlendMode.source2 = source2;
        colourBlendMode.colourArg1 = arg1;
        colourBlendMode.colourArg2 = arg2;
        colourBlendMode.factor = manualBlend;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setColourOperation(LayerBlendOperation op)
    {
        // Set up the multitexture and multipass blending operations
        switch (op)
        {
        case LBO_REPLACE:
            setColourOperationEx(LBX_SOURCE1, LBS_TEXTURE, LBS_CURRENT);
            setColourOpMultipassFallback(SBF_ONE, SBF_ZERO);
            break;
        case LBO_ADD:
            setColourOperationEx(LBX_ADD, LBS_TEXTURE, LBS_CURRENT);
            setColourOpMultipassFallback(SBF_ONE, SBF_ONE);
            break;
        case LBO_MODULATE:
            setColourOperationEx(LBX_MODULATE, LBS_TEXTURE, LBS_CURRENT);
            setColourOpMultipassFallback(SBF_DEST_COLOUR, SBF_ZERO);
            break;
        case LBO_ALPHA_BLEND:
            setColourOperationEx(LBX_BLEND_TEXTURE_ALPHA, LBS_TEXTURE, LBS_CURRENT);
            setColourOpMultipassFallback(SBF_SOURCE_ALPHA, SBF_ONE_MINUS_SOURCE_ALPHA);
            break;
        }


    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setColourOpMultipassFallback(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor)
    {
        colourBlendFallbackSrc = sourceFactor;
        colourBlendFallbackDest = destFactor;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setAlphaOperation(LayerBlendOperationEx op,
        LayerBlendSource source1,
        LayerBlendSource source2,
        Real arg1,
        Real arg2,
        Real manualBlend)
    {
        alphaBlendMode.operation = op;
        alphaBlendMode.source1 = source1;
        alphaBlendMode.source2 = source2;
        alphaBlendMode.alphaArg1 = arg1;
        alphaBlendMode.alphaArg2 = arg2;
        alphaBlendMode.factor = manualBlend;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::addEffect(TextureEffect& effect)
    {
        // Ensure controller pointer is null
        effect.controller = 0;

        if (effect.type == ET_ENVIRONMENT_MAP || effect.type == ET_SCROLL || effect.type == ET_ROTATE
            || effect.type == ET_PROJECTIVE_TEXTURE)
        {
            // Replace - must be unique
            // Search for existing effect of this type
            EffectMap::iterator i = mEffects.find(effect.type);
            if (i != mEffects.end())
            {
                mEffects.erase(i);
            }
        }

        if (isLoaded())
        {
            // Create controller
            createEffectController(effect);
        }

        // Record new effect
        mEffects.insert(EffectMap::value_type(effect.type, effect));

    }
    //-----------------------------------------------------------------------
    void TextureUnitState::removeAllEffects(void)
    {
        // Iterate over effects to remove controllers
        EffectMap::iterator i, iend;
        iend = mEffects.end();
        for (i = mEffects.begin(); i != iend; ++i)
        {
            if (i->second.controller)
            {
                ControllerManager::getSingleton().destroyController(i->second.controller);
            }
        }

        mEffects.clear();
    }

    //-----------------------------------------------------------------------
    bool TextureUnitState::isBlank(void) const
    {
        return mIsBlank;
    }

    //-----------------------------------------------------------------------
    SceneBlendFactor TextureUnitState::getColourBlendFallbackSrc(void) const
    {
        return colourBlendFallbackSrc;
    }
    //-----------------------------------------------------------------------
    SceneBlendFactor TextureUnitState::getColourBlendFallbackDest(void) const
    {
        return colourBlendFallbackDest;
    }
    //-----------------------------------------------------------------------
    const LayerBlendModeEx& TextureUnitState::getColourBlendMode(void) const
    {
        return colourBlendMode;
    }
    //-----------------------------------------------------------------------
    const LayerBlendModeEx& TextureUnitState::getAlphaBlendMode(void) const
    {
        return alphaBlendMode;
    }
    //-----------------------------------------------------------------------
    TextureUnitState::TextureAddressingMode TextureUnitState::getTextureAddressingMode(void) const
    {
        return mAddressMode;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureAddressingMode(TextureUnitState::TextureAddressingMode tam)
    {
        mAddressMode = tam;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setEnvironmentMap(bool enable, EnvMapType envMapType)
    {
        if (enable)
        {
            TextureEffect eff;
            eff.type = ET_ENVIRONMENT_MAP;

            eff.subtype = envMapType;
            addEffect(eff);
        }
        else
        {
            removeEffect(ET_ENVIRONMENT_MAP);
        }
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::removeEffect(TextureEffectType type)
    {
        // Get range of items matching this effect
        std::pair< EffectMap::iterator, EffectMap::iterator > remPair = 
            mEffects.equal_range( type );
        // Remove controllers
        for (EffectMap::iterator i = remPair.first; i != remPair.second; ++i)
        {
            if (i->second.controller)
            {
                ControllerManager::getSingleton().destroyController(i->second.controller);
            }
        }
        // Erase         
        mEffects.erase( remPair.first, remPair.second );
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setBlank(void)
    {
        mIsBlank = true;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureTransform(const Matrix4& xform)
    {
        mTexModMatrix = xform;
        mRecalcTexMatrix = false;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureScroll(Real u, Real v)
    {
        mUMod = u;
        mVMod = v;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureScale(Real uScale, Real vScale)
    {
        mUScale = uScale;
        mVScale = vScale;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureRotate(const Radian& angle)
    {
        mRotate = angle;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    const Matrix4& TextureUnitState::getTextureTransform()
    {
        if (mRecalcTexMatrix)
            recalcTextureMatrix();
        return mTexModMatrix;

    }
    //-----------------------------------------------------------------------
    void TextureUnitState::recalcTextureMatrix()
    {
        // Assumption: 2D texture coords
        Matrix3 xform, rot;

        xform = Matrix3::IDENTITY;
        if (mUScale || mVScale)
        {
            // Offset to center of texture
            xform[0][0] = 1/mUScale;
            xform[1][1] = 1/mVScale;
            // Skip matrix concat since first matrix update
            xform[0][2] = (-0.5 * xform[0][0]) + 0.5;
            xform[1][2] = (-0.5 * xform[1][1]) + 0.5;

        }

        if (mUMod || mVMod)
        {
            Matrix3 xlate = Matrix3::IDENTITY;

            xlate[0][2] = mUMod;
            xlate[1][2] = mVMod;

            xform = xlate * xform;
        }

        if (mRotate != Radian(0))
        {
            rot = Matrix3::IDENTITY;
            Radian theta ( mRotate );
            Real cosTheta = Math::Cos(theta);
            Real sinTheta = Math::Sin(theta);

            rot[0][0] = cosTheta;
            rot[0][1] = -sinTheta;
            rot[1][0] = sinTheta;
            rot[1][1] = cosTheta;
            // Offset center of rotation to center of texture
            rot[0][2] = 0.5 + ( (-0.5 * cosTheta) - (-0.5 * sinTheta) );
            rot[1][2] = 0.5 + ( (-0.5 * sinTheta) + (-0.5 * cosTheta) );


            xform = rot * xform;
        }

        mTexModMatrix = xform;

    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureUScroll(Real value)
    {
        mUMod = value;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureVScroll(Real value)
    {
        mVMod = value;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureUScale(Real value)
    {
        mUScale = value;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTextureVScale(Real value)
    {
        mVScale = value;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setAlphaRejectSettings(CompareFunction func, unsigned char value)
    {
        mAlphaRejectFunc = func;
        mAlphaRejectVal = value;
    }
    //-----------------------------------------------------------------------
    CompareFunction TextureUnitState::getAlphaRejectFunction(void) const
    {
        return mAlphaRejectFunc;
    }
    //-----------------------------------------------------------------------
    unsigned char TextureUnitState::getAlphaRejectValue(void) const
    {
        return mAlphaRejectVal;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setScrollAnimation(Real uSpeed, Real vSpeed)
    {
        // Remove existing effect
        removeEffect(ET_SCROLL);
        // Create new effect
        TextureEffect eff;
        eff.type = ET_SCROLL;
        eff.arg1 = uSpeed;
        eff.arg2 = vSpeed;
        addEffect(eff);
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setRotateAnimation(Real speed)
    {
        // Remove existing effect
        removeEffect(ET_ROTATE);
        // Create new effect
        TextureEffect eff;
        eff.type = ET_ROTATE;
        eff.arg1 = speed;
        addEffect(eff);
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setTransformAnimation(TextureTransformType ttype,
        WaveformType waveType, Real base, Real frequency, Real phase, Real amplitude)
    {
        // Remove existing effect
        removeEffect(ET_TRANSFORM);
        // Create new effect
        TextureEffect eff;
        eff.type = ET_TRANSFORM;
        eff.subtype = ttype;
        eff.waveType = waveType;
        eff.base = base;
        eff.frequency = frequency;
        eff.phase = phase;
        eff.amplitude = amplitude;
        addEffect(eff);
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::_load(void)
    {
        // Load textures
        for (unsigned int i = 0; i < mNumFrames; ++i)
        {
            if (mFrames[i] != "")
            {
                // Ensure texture is loaded, default MipMaps and priority
                try {

                    TextureManager::getSingleton().load(mFrames[i], mTextureType);
                    mIsBlank = false;
                }
                catch (...) {
                    String msg;
                    msg = msg + "Error loading texture " + mFrames[i]  + ". Texture layer will be blank.";
                    LogManager::getSingleton().logMessage(msg);
                    mIsBlank = true;
                }
            }
        }
        // Animation controller
        if (mAnimDuration != 0)
        {
            createAnimController();
        }
        // Effect controllers
        for (EffectMap::iterator it = mEffects.begin(); it != mEffects.end(); ++it)
        {
            createEffectController(it->second);
        }

    }
    //-----------------------------------------------------------------------
    void TextureUnitState::createAnimController(void)
    {
        mAnimController = ControllerManager::getSingleton().createTextureAnimator(this, mAnimDuration);

    }
    //-----------------------------------------------------------------------
    void TextureUnitState::createEffectController(TextureEffect& effect)
    {
        effect.controller = 0;
        ControllerManager& cMgr = ControllerManager::getSingleton();
        switch (effect.type)
        {
        case ET_SCROLL:
            effect.controller = cMgr.createTextureScroller(this, effect.arg1, effect.arg2);
            break;
        case ET_ROTATE:
            effect.controller = cMgr.createTextureRotater(this, effect.arg1);
            break;
        case ET_TRANSFORM:
            effect.controller = cMgr.createTextureWaveTransformer(this, (TextureUnitState::TextureTransformType)effect.subtype, effect.waveType, effect.base,
                effect.frequency, effect.phase, effect.amplitude);
            break;
        case ET_ENVIRONMENT_MAP:
            break;
        default:
            break;
        }
    }
    //-----------------------------------------------------------------------
	Real TextureUnitState::getTextureUScroll(void) const
    {
		return mUMod;
    }

	//-----------------------------------------------------------------------
	Real TextureUnitState::getTextureVScroll(void) const
    {
		return mVMod;
    }

	//-----------------------------------------------------------------------
	Real TextureUnitState::getTextureUScale(void) const
    {
		return mUScale;
    }

	//-----------------------------------------------------------------------
	Real TextureUnitState::getTextureVScale(void) const
    {
		return mVScale;
    }

	//-----------------------------------------------------------------------
	const Radian& TextureUnitState::getTextureRotate(void) const
    {
		return mRotate;
    }
	
	//-----------------------------------------------------------------------
	Real TextureUnitState::getAnimationDuration(void) const
	{
		return mAnimDuration;
	}

	//-----------------------------------------------------------------------
	std::multimap<TextureUnitState::TextureEffectType, TextureUnitState::TextureEffect> TextureUnitState::getEffects(void) const
	{
		return mEffects;
	}

	//-----------------------------------------------------------------------
	void TextureUnitState::setTextureFiltering(TextureFilterOptions filterType)
	{
        switch (filterType)
        {
        case TFO_NONE:
            setTextureFiltering(FO_POINT, FO_POINT, FO_NONE);
            break;
        case TFO_BILINEAR:
            setTextureFiltering(FO_LINEAR, FO_LINEAR, FO_POINT);
            break;
        case TFO_TRILINEAR:
            setTextureFiltering(FO_LINEAR, FO_LINEAR, FO_LINEAR);
            break;
        case TFO_ANISOTROPIC:
            setTextureFiltering(FO_ANISOTROPIC, FO_ANISOTROPIC, FO_LINEAR);
            break;
        }
        mIsDefaultFiltering = false;
	}
	//-----------------------------------------------------------------------
    void TextureUnitState::setTextureFiltering(FilterType ft, FilterOptions fo)
    {
        switch (ft)
        {
        case FT_MIN:
            mMinFilter = fo;
            break;
        case FT_MAG:
            mMagFilter = fo;
            break;
        case FT_MIP:
            mMipFilter = fo;
            break;
        }
        mIsDefaultFiltering = false;
    }
	//-----------------------------------------------------------------------
    void TextureUnitState::setTextureFiltering(FilterOptions minFilter, 
        FilterOptions magFilter, FilterOptions mipFilter)
    {
        mMinFilter = minFilter;
        mMagFilter = magFilter;
        mMipFilter = mipFilter;
        mIsDefaultFiltering = false;
    }
	//-----------------------------------------------------------------------
	FilterOptions TextureUnitState::getTextureFiltering(FilterType ft) const
	{

        switch (ft)
        {
        case FT_MIN:
            return mIsDefaultFiltering ? 
                MaterialManager::getSingleton().getDefaultTextureFiltering(FT_MIN) : mMinFilter;
        case FT_MAG:
            return mIsDefaultFiltering ? 
                MaterialManager::getSingleton().getDefaultTextureFiltering(FT_MAG) : mMagFilter;
        case FT_MIP:
            return mIsDefaultFiltering ? 
                MaterialManager::getSingleton().getDefaultTextureFiltering(FT_MIP) : mMipFilter;
        }
		// to keep compiler happy
		return mMinFilter;
	}

	//-----------------------------------------------------------------------
	void TextureUnitState::setTextureAnisotropy(unsigned int maxAniso)
	{
		mMaxAniso = maxAniso;
        mIsDefaultAniso = false;
	}
	//-----------------------------------------------------------------------
	unsigned int TextureUnitState::getTextureAnisotropy() const
	{
        return mIsDefaultAniso? MaterialManager::getSingleton().getDefaultAnisotropy() : mMaxAniso;
	}

	//-----------------------------------------------------------------------
    void TextureUnitState::_unload(void)
    {
        // TODO
    }
    //-----------------------------------------------------------------------------
    bool TextureUnitState::isLoaded(void)
    {
        return mParent->isLoaded();
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::_notifyNeedsRecompile(void)
    {
        mParent->_notifyNeedsRecompile();
    }
    //-----------------------------------------------------------------------
    bool TextureUnitState::hasViewRelativeTextureCoordinateGeneration(void)
    {
        // Right now this only returns true for reflection maps

        EffectMap::const_iterator i, iend;
        iend = mEffects.end();
        
        for(i = mEffects.find(ET_ENVIRONMENT_MAP); i != iend; ++i)
        {
            if (i->second.subtype == ENV_REFLECTION)
                return true;
        }
        for(i = mEffects.find(ET_PROJECTIVE_TEXTURE); i != iend; ++i)
        {
            return true;
        }

        return false;
    }
    //-----------------------------------------------------------------------
    void TextureUnitState::setProjectiveTexturing(bool enable, 
        const Frustum* projectionSettings)
    {
        if (enable)
        {
            TextureEffect eff;
            eff.type = ET_PROJECTIVE_TEXTURE;
            eff.frustum = projectionSettings;
            addEffect(eff);
        }
        else
        {
            removeEffect(ET_PROJECTIVE_TEXTURE);
        }

    }

}

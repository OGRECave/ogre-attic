/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreMaterial.h"

#include "OgreLogManager.h"
#include "OgreMath.h"
#include "OgreControllerManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    Material::TextureLayer::TextureLayer(bool deferLoad)
    {
        mIsBlank = true;
        colourBlendMode.blendType = LBT_COLOUR;
        setColourOperation(LBO_MODULATE);
        setTextureAddressingMode(TAM_WRAP);

        alphaBlendMode.operation = LBX_MODULATE;
        alphaBlendMode.blendType = LBT_ALPHA;
        alphaBlendMode.source1 = LBS_TEXTURE;
        alphaBlendMode.source2 = LBS_CURRENT;

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

        mDeferLoad = deferLoad;
    }

    //-----------------------------------------------------------------------
    Material::TextureLayer::TextureLayer( const TextureLayer& oth )
    {
        *this = oth;
    }

    //-----------------------------------------------------------------------
    Material::TextureLayer::TextureLayer( const String& texName, int texCoordSet, bool deferLoad)
    {
        mIsBlank = true;
        colourBlendMode.blendType = LBT_COLOUR;
        setColourOperation(LBO_MODULATE);
        setTextureAddressingMode(TAM_WRAP);

        alphaBlendMode.operation = LBX_MODULATE;
        alphaBlendMode.blendType = LBT_ALPHA;
        alphaBlendMode.source1 = LBS_TEXTURE;
        alphaBlendMode.source2 = LBS_CURRENT;

        mUMod = mVMod = 0;
        mUScale = mVScale = 1;
        mRotate = 0;
        mAnimDuration = 0;
        mAnimController = 0;
        mTexModMatrix = Matrix4::IDENTITY;
        mRecalcTexMatrix = false;
        mAlphaRejectFunc = CMPF_ALWAYS_PASS;
        mAlphaRejectVal = 0;
        mDeferLoad = deferLoad;

        setTextureName(texName);
        setTextureCoordSet(texCoordSet);
    }

    Material::TextureLayer::~TextureLayer()
    {
        /*
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
        */

    }
    //-----------------------------------------------------------------------
    Material::TextureLayer & Material::TextureLayer::operator = ( 
        const Material::TextureLayer &oth )
    {
        // copy basic members (int's, real's)
        memcpy( this, &oth, (uchar *)(&oth.mFrames[0]) - (uchar *)(&oth) );

        // copy complex members
        for( ushort i = 0; i<mNumFrames; i++ )
            mFrames[i] = oth.mFrames[i];

        mEffects = oth.mEffects;

        return *this;
    }
    //-----------------------------------------------------------------------
    const String& Material::TextureLayer::getTextureName(void) const
    {
        // Return name of current frame
        return mFrames[mCurrentFrame];
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setTextureName( const String& name)
    {
        mFrames[0] = name;
        mNumFrames = 1;
        mCurrentFrame = 0;
        mCubic = false;

        if (name == "")
        {
            mIsBlank = true;
        }
        else if (!mDeferLoad)
        {
            // Ensure texture is loaded, default MipMaps and priority
            if( TextureManager::getSingleton().getByName( name ) != NULL )
			{
				mIsBlank = false;
                return;
			}

            try {                
                TextureManager::getSingleton().load(name);
                mIsBlank = false;
            }
            catch (...) {
                String msg;
                msg = msg + "Error loading texture " + name  + ". Texture layer will be blank.";
                LogManager::getSingleton().logMessage(msg);
                mIsBlank = true;
            }
        }

    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setCubicTextureName( const String& name, bool forUVW)
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
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setCubicTextureName(const String* const names, bool forUVW)
    {
        if (forUVW)
        {
            // TODO - build single cubic texture
        }
        else
        {

            mNumFrames = 6;
            mCurrentFrame = 0;
            mCubic = true;

            for (int i = 0; i < 6; ++i)
            {
                mFrames[i] = names[i];
                if (!mDeferLoad)
                {
                    // Ensure texture is loaded, default MipMaps and priority
                    try {

                        TextureManager::getSingleton().load(mFrames[i]);
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

        }
    }
    //-----------------------------------------------------------------------
    bool Material::TextureLayer::isCubic(void) const
    {
        return mCubic;
    }
    //-----------------------------------------------------------------------
    bool Material::TextureLayer::is3D(void) const
    {
        return mCubic && (mNumFrames == 1);

    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setAnimatedTextureName( const String& name, int numFrames, Real duration)
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
            Except(Exception::ERR_INVALIDPARAMS, cmsg, "TextureLayer::setAnimatedTextureName");
        }
        mNumFrames = numFrames;
        mAnimDuration = duration;
        mCurrentFrame = 0;
        mCubic = false;

        for (int i = 0; i < mNumFrames; ++i)
        {
            char suffix[5];
            sprintf(suffix, "_%d", i);

            mFrames[i] = baseName + suffix + ext;
            if (!mDeferLoad)
            {
                // Ensure texture is loaded, default MipMaps and priority
                try {

                    TextureManager::getSingleton().load(mFrames[i]);
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

        // Set up automatic transition
        if (mAnimDuration != 0 && !mDeferLoad)
        {
            createAnimController();
        }
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setAnimatedTextureName(const String* const names, int numFrames, Real duration)
    {
        if (numFrames > MAX_FRAMES)
        {
            char cmsg[128];
            sprintf(cmsg, "Maximum number of frames is %d.", MAX_FRAMES);
            Except(Exception::ERR_INVALIDPARAMS, cmsg, "TextureLayer::setAnimatedTextureName");
        }
        mNumFrames = numFrames;
        mAnimDuration = duration;
        mCurrentFrame = 0;
        mCubic = false;

        for (int i = 0; i < mNumFrames; ++i)
        {
            mFrames[i] = names[i];

            if (!mDeferLoad)
            {
                // Ensure texture is loaded, default MipMaps and priority
                try {

                    TextureManager::getSingleton().load(mFrames[i]);
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
        // Set up automatic transition
        if (mAnimDuration != 0 && !mDeferLoad)
        {
            createAnimController();
        }
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setCurrentFrame(int frameNumber)
    {
        assert(frameNumber < mNumFrames);
        mCurrentFrame = frameNumber;

    }
    //-----------------------------------------------------------------------
    int Material::TextureLayer::getCurrentFrame(void) const
    {
        return mCurrentFrame;
    }
    //-----------------------------------------------------------------------
    int Material::TextureLayer::getNumFrames(void) const
    {
        return mNumFrames;
    }
    //-----------------------------------------------------------------------
    const String& Material::TextureLayer::getFrameTextureName(int frameNumber) const
    {
        assert(frameNumber < mNumFrames);
        return mFrames[frameNumber];
    }
    //-----------------------------------------------------------------------
    int Material::TextureLayer::getTextureCoordSet(void) const
    {
        return textureCoordSetIndex;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setTextureCoordSet(int set)
    {
        textureCoordSetIndex = set;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setColourOperationEx(LayerBlendOperationEx op,
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
    void Material::TextureLayer::setColourOperation(LayerBlendOperation op)
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
    void Material::TextureLayer::setColourOpMultipassFallback(SceneBlendFactor sourceFactor, SceneBlendFactor destFactor)
    {
        colourBlendFallbackSrc = sourceFactor;
        colourBlendFallbackDest = destFactor;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setAlphaOperation(LayerBlendOperationEx op,
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
    void Material::TextureLayer::addEffect(TextureEffect& effect)
    {
        // Ensure controller pointer is null
        effect.controller = 0;

        if (effect.type == ET_ENVIRONMENT_MAP || effect.type == ET_SCROLL || effect.type == ET_ROTATE)
        {
            // Replace - must be unique
            // Search for existing effect of this type
            EffectMap::iterator i = mEffects.find(effect.type);
            if (i != mEffects.end())
            {
                mEffects.erase(i);
            }
        }

        if (!mDeferLoad)
        {
            // Create controller
            createEffectController(effect);
        }

        // Record new effect
        mEffects.insert(EffectMap::value_type(effect.type, effect));

    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::removeAllEffects(void)
    {
        mEffects.clear();
    }

    //-----------------------------------------------------------------------
    bool Material::TextureLayer::isBlank(void) const
    {
        return mIsBlank;
    }

    //-----------------------------------------------------------------------
    SceneBlendFactor Material::TextureLayer::getColourBlendFallbackSrc(void) const
    {
        return colourBlendFallbackSrc;
    }
    //-----------------------------------------------------------------------
    SceneBlendFactor Material::TextureLayer::getColourBlendFallbackDest(void) const
    {
        return colourBlendFallbackDest;
    }
    //-----------------------------------------------------------------------
    LayerBlendModeEx Material::TextureLayer::getColourBlendMode(void) const
    {
        return colourBlendMode;
    }
    //-----------------------------------------------------------------------
    LayerBlendModeEx Material::TextureLayer::getAlphaBlendMode(void) const
    {
        return alphaBlendMode;
    }
    //-----------------------------------------------------------------------
    Material::TextureLayer::TextureAddressingMode Material::TextureLayer::getTextureAddressingMode(void) const
    {
        return mAddressMode;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setTextureAddressingMode(Material::TextureLayer::TextureAddressingMode tam)
    {
        mAddressMode = tam;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setEnvironmentMap(bool enable, bool planar)
    {
        TextureEffect eff;
        eff.type = ET_ENVIRONMENT_MAP;

        if (enable)
        {
            if (planar)
            {
                eff.subtype = ENV_PLANAR;
                addEffect(eff);
            }
            else
            {
                eff.subtype = ENV_CURVED;
                addEffect(eff);
            }
        }
        else
        {
            removeEffect(ET_ENVIRONMENT_MAP);
        }
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::removeEffect(TextureEffectType type)
    {
      // EffectMap::iterator i = mEffects.find(type);
        std::pair< EffectMap::iterator, EffectMap::iterator > remPair = mEffects.equal_range( type );
        mEffects.erase( remPair.first, remPair.second );
        //EffectMap::iterator i = mEffects.find(type);
/*        for (; i != mEffects.end() && i->first == type; ++i)
        {
            // Remove all instances of this type
#ifdef __GNUC__
            assert(0);
            // Why do I get an error here???
            //i = mEffects.erase(i);
#else
            i++;
            EffectMap::iterator j = i;
            i--;
            mEffects.erase( i );
            i = j;
#endif
        }
*/
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setBlank(void)
    {
        mIsBlank = true;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setTextureTransform(const Matrix4& xform)
    {
        mTexModMatrix = xform;
        mRecalcTexMatrix = false;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setTextureScroll(Real u, Real v)
    {
        mUMod = u;
        mVMod = v;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setTextureScale(Real uScale, Real vScale)
    {
        mUScale = uScale;
        mVScale = vScale;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setTextureRotate(Real degrees)
    {
        mRotate = degrees;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    const Matrix4& Material::TextureLayer::getTextureTransform()
    {
        if (mRecalcTexMatrix)
            recalcTextureMatrix();
        return mTexModMatrix;

    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::recalcTextureMatrix()
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

        if (mRotate != 0)
        {
            rot = Matrix3::IDENTITY;
            Real theta = Math::getSingleton().DegreesToRadians(mRotate);
            Real cosTheta = Math::getSingleton().Cos(theta);
            Real sinTheta = Math::getSingleton().Sin(theta);

            rot[0][0] = cosTheta;
            rot[0][1] = -sinTheta;
            rot[1][0] = sinTheta;
            rot[1][1] = cosTheta;
            // Offset center of rotation to center of texture
            Real cosThetaOff = cosTheta * -0.5;
            Real sinThetaOff = sinTheta * -0.5;
            rot[0][2] = cosThetaOff - sinThetaOff;
            rot[1][2] = sinThetaOff + cosThetaOff;


            xform = xform * rot;
        }

        mTexModMatrix = xform;

    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setTextureUScroll(Real value)
    {
        mUMod = value;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setTextureVScroll(Real value)
    {
        mVMod = value;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setTextureUScale(Real value)
    {
        mUScale = value;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setTextureVScale(Real value)
    {
        mVScale = value;
        mRecalcTexMatrix = true;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setAlphaRejectSettings(CompareFunction func, unsigned char value)
    {
        mAlphaRejectFunc = func;
        mAlphaRejectVal = value;
    }
    //-----------------------------------------------------------------------
    CompareFunction Material::TextureLayer::getAlphaRejectFunction(void) const
    {
        return mAlphaRejectFunc;
    }
    //-----------------------------------------------------------------------
    unsigned char Material::TextureLayer::getAlphaRejectValue(void) const
    {
        return mAlphaRejectVal;
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setScrollAnimation(Real uSpeed, Real vSpeed)
    {
        TextureEffect eff;
        eff.type = ET_SCROLL;
        eff.arg1 = uSpeed;
        eff.arg2 = vSpeed;
        addEffect(eff);
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setRotateAnimation(Real speed)
    {
        TextureEffect eff;
        eff.type = ET_ROTATE;
        eff.arg1 = speed;
        addEffect(eff);
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setTransformAnimation(TextureTransformType ttype,
        WaveformType waveType, Real base, Real frequency, Real phase, Real amplitude)
    {
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
    void Material::TextureLayer::_load(void)
    {
        // Load textures
        for (int i = 0; i < mNumFrames; ++i)
        {
            if (mFrames[i] != "")
            {
                // Ensure texture is loaded, default MipMaps and priority
                try {

                    TextureManager::getSingleton().load(mFrames[i]);
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

        mDeferLoad = false;

    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::createAnimController(void)
    {
        mAnimController = ControllerManager::getSingleton().createTextureAnimator(this, mAnimDuration);

    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::createEffectController(TextureEffect& effect)
    {
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
            effect.controller = cMgr.createTextureWaveTransformer(this, (Material::TextureLayer::TextureTransformType)effect.subtype, effect.waveType, effect.base,
                effect.frequency, effect.phase, effect.amplitude);
            break;
	case ET_ENVIRONMENT_MAP:
	case ET_BUMP_MAP:
	    break;
        }
    }
    //-----------------------------------------------------------------------
    void Material::TextureLayer::setDeferredLoad(bool defer)
    {
        mDeferLoad = defer;
    }

}

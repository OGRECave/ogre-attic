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
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"

#include "OgreMaterialSerializer.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreTextureUnitState.h"

namespace Ogre 
{
	MaterialSerializer::MaterialSerializer()
	{
		mBuffer = "";
	}

	void MaterialSerializer::exportMaterial(const Material *pMat, const String &fileName, bool exportDefaults)
	{
		clearQueue();
		mDefaults = exportDefaults;
		writeMaterial(pMat);
		exportQueued(fileName);
	}

	void MaterialSerializer::exportQueued(const String &fileName)
	{
		if (mBuffer == "")
			Except(Exception::ERR_INVALIDPARAMS, "Queue is empty !", "MaterialSerializer::exportQueued");

		LogManager::getSingleton().logMessage("MaterialSerializer : writing material(s) to material script : " + fileName, LML_CRITICAL);
		FILE *fp;
        fp = fopen(fileName.c_str(), "w");
		if (!fp)
            Except(Exception::ERR_CANNOT_WRITE_TO_FILE, "Cannot create material file.",
            "MaterialSerializer::export");

		fputs(mBuffer.c_str(), fp);
        fclose(fp);
		LogManager::getSingleton().logMessage("MaterialSerializer : done.", LML_CRITICAL);
        clearQueue();
	}

	void MaterialSerializer::queueForExport(const Material *pMat, bool clearQueued, bool exportDefaults)
	{
		if (clearQueued)
			clearQueue();

		mDefaults = exportDefaults;
		writeMaterial(pMat);
	}

	void MaterialSerializer::clearQueue()
	{
		mBuffer = "";
	}

	const String &MaterialSerializer::getQueuedAsString() const
	{
		return mBuffer;
	}

	void MaterialSerializer::writeMaterial(const Material *pMat)
    {
		LogManager::getSingleton().logMessage("MaterialSerializer : writing material " + pMat->getName() + " to queue.", LML_CRITICAL);
        // Material name
		mBuffer += "\n";
        mBuffer += "material " + pMat->getName();
		beginSection(0);
		{
            // Iterate over techniques
            Material::TechniqueIterator it = 
                const_cast<Material*>(pMat)->getTechniqueIterator();
            while (it.hasMoreElements())
            {
                writeTechnique(it.getNext());
            }
        }
        endSection(0);
    }

    void MaterialSerializer::writeTechnique(const Technique* pTech)
    {
        // Technique header
		mBuffer += "\n";
        mBuffer += "technique";
        beginSection(1);
        {
            // Iterate over passes
            Technique::PassIterator it = const_cast<Technique*>(pTech)->getPassIterator();
            while (it.hasMoreElements())
            {
                writePass(it.getNext());
            }
        }
        endSection(1);

    }

    void MaterialSerializer::writePass(const Pass* pPass)
    {
		mBuffer += "\n";
        mBuffer += "pass";
        beginSection(2);
        {
			//lighting
			if (mDefaults || 
				pPass->getLightingEnabled() != true)
			{
				writeAttribute(2, "lighting");
				writeValue(pPass->getLightingEnabled() ? "on" : "off");
			}

			if (pPass->getLightingEnabled())
			{
				// Ambient
				if (mDefaults ||
					pPass->getAmbient().r != 1 ||
					pPass->getAmbient().g != 1 ||
					pPass->getAmbient().b != 1)
				{
					writeAttribute(2, "ambient");
					writeColourValue(pPass->getAmbient());
				}

				// Diffuse
				if (mDefaults ||
					pPass->getDiffuse().r != 1 ||
					pPass->getDiffuse().g != 1 ||
					pPass->getDiffuse().b != 1)
				{
					writeAttribute(2, "diffuse");
					writeColourValue(pPass->getDiffuse());
				}

				// Specular
				if (mDefaults ||
					pPass->getSpecular().r != 0 ||
					pPass->getSpecular().g != 0 ||
					pPass->getSpecular().b != 0 ||
					pPass->getSpecular().a != 1)
				{
					writeAttribute(2, "specular");
					writeColourValue(pPass->getSpecular(), true);
				}

				// Emissive
				if (mDefaults ||
					pPass->getSelfIllumination().r != 0 ||
					pPass->getSelfIllumination().g != 0 ||
					pPass->getSelfIllumination().b != 0)
				{
					writeAttribute(2, "emissive");
					writeColourValue(pPass->getSelfIllumination());
				}
			}

			// scene blend factor
			if (mDefaults || 
				pPass->getSourceBlendFactor() != SBF_ONE || 
				pPass->getDestBlendFactor() != SBF_ZERO)
			{
				writeAttribute(2, "scene_blend");
				writeSceneBlendFactor(pPass->getSourceBlendFactor(), pPass->getDestBlendFactor());
			}


			//depth check
			if (mDefaults || 
				pPass->getDepthCheckEnabled() != true)
			{
				writeAttribute(2, "depth_check");
				writeValue(pPass->getDepthCheckEnabled() ? "on" : "off");
			}

			//depth write
			if (mDefaults || 
				pPass->getDepthWriteEnabled() != true)
			{
				writeAttribute(2, "depth_write");
				writeValue(pPass->getDepthWriteEnabled() ? "on" : "off");
			}

			//depth function
			if (mDefaults || 
				pPass->getDepthFunction() != CMPF_LESS_EQUAL)
			{
				writeAttribute(2, "depth_func");
				writeCompareFunction(pPass->getDepthFunction());
			}

			//depth bias
			if (mDefaults || 
				pPass->getDepthBias() != 0)
			{
				writeAttribute(2, "depth_bias");
				writeValue(StringConverter::toString(pPass->getDepthBias()));
			}

			// hardware culling mode
			if (mDefaults || 
				pPass->getCullingMode() != CULL_CLOCKWISE)
			{
				CullingMode hcm = pPass->getCullingMode();
				writeAttribute(2, "cull_hardware");
				switch (hcm)
				{
				case CULL_NONE :
					writeValue("none");
					break;
				case CULL_CLOCKWISE :
					writeValue("clockwise");
					break;
				case CULL_ANTICLOCKWISE :
					writeValue("anticlockwise");
					break;
				}
			}

			// software culling mode
			if (mDefaults || 
				pPass->getManualCullingMode() != MANUAL_CULL_BACK)
			{
				ManualCullingMode scm = pPass->getManualCullingMode();
				writeAttribute(2, "cull_software");
				switch (scm)
				{
				case MANUAL_CULL_NONE :
					writeValue("none");
					break;
				case MANUAL_CULL_BACK :
					writeValue("back");
					break;
				case MANUAL_CULL_FRONT :
					writeValue("front");
					break;
				}
			}

			//shading
			if (mDefaults || 
				pPass->getShadingMode() != SO_GOURAUD)
			{
				writeAttribute(2, "shading");
				switch (pPass->getShadingMode())
				{
				case SO_FLAT:
					writeValue("flat");
					break;
				case SO_GOURAUD:
					writeValue("gouraud");
					break;
				case SO_PHONG:
					writeValue("phong");
					break;
				}
			}


			//fog override
			if (mDefaults || 
				pPass->getFogOverride() != false)
			{
				writeAttribute(2, "fog_override");
				writeValue(pPass->getFogOverride() ? "true" : "false");
				if (pPass->getFogOverride())
				{
					switch (pPass->getFogMode())
					{
					case FOG_NONE:
						writeValue("none");
						break;
					case FOG_LINEAR:
						writeValue("linear");
						break;
					case FOG_EXP2:
						writeValue("exp2");
						break;
					case FOG_EXP:
						writeValue("exp");
						break;
					}

					if (pPass->getFogMode() != FOG_NONE)
					{
						writeColourValue(pPass->getFogColour());
						writeValue(StringConverter::toString(pPass->getFogDensity()));
						writeValue(StringConverter::toString(pPass->getFogStart()));
						writeValue(StringConverter::toString(pPass->getFogEnd()));
					}
				}
			}
						
			// Nested texture layers
            Pass::TextureUnitStateIterator it = const_cast<Pass*>(pPass)->getTextureUnitStateIterator();
            while(it.hasMoreElements())
            {
				writeTextureUnit(it.getNext());
			}
		}
		endSection(2);
		LogManager::getSingleton().logMessage("MaterialSerializer : done.", LML_CRITICAL);
    }

	void MaterialSerializer::writeTextureUnit(const TextureUnitState *pTex)
    {
		LogManager::getSingleton().logMessage("MaterialSerializer : parsing texture layer.", LML_CRITICAL);

		mBuffer += "\n";
		//texture name
		if (pTex->getNumFrames() == 1 && pTex->getTextureName() != "" && !pTex->isCubic())
		{
			writeAttribute(3, "texture");
			writeValue(pTex->getTextureName());
		}

		//anim. texture
		if (pTex->getNumFrames() > 1 && !pTex->isCubic())
		{
			writeAttribute(3, "anim_texture");
			for (int n = 0; n < pTex->getNumFrames(); n++)
				writeValue(pTex->getFrameTextureName(n));
			writeValue(StringConverter::toString(pTex->getAnimationDuration()));
		}

		//cubic texture
		if (pTex->isCubic())
		{
			writeAttribute(3, "cubic_texture");
			for (int n = 0; n < pTex->getNumFrames(); n++)
				writeValue(pTex->getFrameTextureName(n));

			//combinedUVW/separateUW
			if (pTex->is3D())
				writeValue("combinedUVW");
			else
				writeValue("separateUV");
		}
		
		//anisotropy level
		if (mDefaults || 
			pTex->getTextureAnisotropy() != 1)
		{
			writeAttribute(3, "tex_anisotropy");
			writeValue(StringConverter::toString(pTex->getTextureAnisotropy()));
		}

		//texture coordinate set
		if (mDefaults || 
			pTex->getTextureCoordSet() != 0)
		{
			writeAttribute(3, "tex_coord_set");
			writeValue(StringConverter::toString(pTex->getTextureCoordSet()));
		}

		//addressing mode
		if (mDefaults || 
			pTex->getTextureAddressingMode() != Ogre::TextureUnitState::TAM_WRAP)
		{
			writeAttribute(3, "tex_address_mode");
			switch (pTex->getTextureAddressingMode())
			{
			case Ogre::TextureUnitState::TAM_CLAMP:
				writeValue("clamp");
				break;
			case Ogre::TextureUnitState::TAM_MIRROR:
				writeValue("mirror");
				break;
			case Ogre::TextureUnitState::TAM_WRAP:
				writeValue("wrap");
				break;
			}
		}
		
		//filtering
		if (mDefaults || 
			pTex->getTextureFiltering() != TFO_BILINEAR)
		{
			writeAttribute(3, "tex_filtering");
			switch (pTex->getTextureFiltering())
			{
			case TFO_BILINEAR:
				writeValue("bilinear");
				break;
			case TFO_NONE:
				writeValue("none");
				break;
			case TFO_TRILINEAR:
				writeValue("trilinear");
				break;
			case TFO_ANISOTROPIC:
				writeValue("anisotropic");
				break;
			}
		}

		// alpha_rejection
		if (mDefaults || 
			pTex->getAlphaRejectFunction() != CMPF_ALWAYS_PASS ||
			pTex->getAlphaRejectValue() != 0)
		{
			writeAttribute(3, "alpha_rejection");
			writeCompareFunction(pTex->getAlphaRejectFunction());
			writeValue(StringConverter::toString(pTex->getAlphaRejectValue()));
		}

		// colour_op_ex
		if (mDefaults || 
			pTex->getColourBlendMode().operation != LBX_MODULATE ||
			pTex->getColourBlendMode().source1 != LBS_TEXTURE ||
			pTex->getColourBlendMode().source2 != LBS_CURRENT)
		{
			writeAttribute(3, "colour_op_ex");
			writeLayerBlendOperationEx(pTex->getColourBlendMode().operation);
			writeLayerBlendSource(pTex->getColourBlendMode().source1);
			writeLayerBlendSource(pTex->getColourBlendMode().source2);
			if (pTex->getColourBlendMode().operation == LBX_BLEND_MANUAL)
				writeValue(StringConverter::toString(pTex->getColourBlendMode().factor));
			if (pTex->getColourBlendMode().source1 == LBS_MANUAL)
				writeColourValue(pTex->getColourBlendMode().colourArg1, false);
			if (pTex->getColourBlendMode().source2 == LBS_MANUAL)
				writeColourValue(pTex->getColourBlendMode().colourArg2, false);

			//colour_op_multipass_fallback
			writeAttribute(3, "colour_op_multipass_fallback");
			writeSceneBlendFactor(pTex->getColourBlendFallbackSrc());
			writeSceneBlendFactor(pTex->getColourBlendFallbackDest());
		}

		// alpha_op_ex
		if (mDefaults || 
			pTex->getAlphaBlendMode().operation != LBX_MODULATE ||
			pTex->getAlphaBlendMode().source1 != LBS_TEXTURE ||
			pTex->getAlphaBlendMode().source2 != LBS_CURRENT)
		{
			writeAttribute(3, "alpha_op_ex");
			writeLayerBlendOperationEx(pTex->getAlphaBlendMode().operation);
			writeLayerBlendSource(pTex->getAlphaBlendMode().source1);
			writeLayerBlendSource(pTex->getAlphaBlendMode().source2);
			if (pTex->getAlphaBlendMode().operation == LBX_BLEND_MANUAL)
				writeValue(StringConverter::toString(pTex->getAlphaBlendMode().factor));
			else if (pTex->getAlphaBlendMode().source1 == LBS_MANUAL)
				writeValue(StringConverter::toString(pTex->getAlphaBlendMode().alphaArg1));
			else if (pTex->getAlphaBlendMode().source2 == LBS_MANUAL)
				writeValue(StringConverter::toString(pTex->getAlphaBlendMode().alphaArg2));
		}

		// rotate
		if (mDefaults ||
			pTex->getTextureRotate() != 0)
		{
			writeAttribute(3, "rotate");
			writeValue(StringConverter::toString(pTex->getTextureRotate()));
		}

		// scroll
		if (mDefaults ||
			pTex->getTextureUScroll() != 0 || 
			pTex->getTextureVScroll() != 0 )
		{
			writeAttribute(3, "scroll");
			writeValue(StringConverter::toString(pTex->getTextureUScroll()));
			writeValue(StringConverter::toString(pTex->getTextureVScroll()));
		}

		EffectMap m_ef = pTex->getEffects();
		if (!m_ef.empty())
		{
			EffectMap::const_iterator it;
			for (it = m_ef.begin(); it != m_ef.end(); ++it)
			{
				TextureUnitState::TextureEffect ef = it->second;
				switch (ef.type)
				{
				case TextureUnitState::ET_ENVIRONMENT_MAP :
					writeEnvironmentMapEffect(ef, pTex);
					break;
				case TextureUnitState::ET_ROTATE :
					writeRotationEffect(ef, pTex);
					break;
				case TextureUnitState::ET_SCROLL :
					writeScrollEffect(ef, pTex);
					break;
				case TextureUnitState::ET_TRANSFORM :
					writeTransformEffect(ef, pTex);
					break;
				case TextureUnitState::ET_BUMP_MAP :
				default:
					break;
				}
			}
		}
	}

	void MaterialSerializer::writeEnvironmentMapEffect(const TextureUnitState::TextureEffect effect, const TextureUnitState *pTex)
	{
		writeAttribute(3, "env_map");
		switch (effect.subtype)
		{
		case TextureUnitState::ENV_PLANAR:
			writeValue("planar");
			break;
		case TextureUnitState::ENV_CURVED:
			writeValue("spherical");
			break;
		case TextureUnitState::ENV_NORMAL:
			writeValue("cubic_normal");
			break;
		case TextureUnitState::ENV_REFLECTION:
			writeValue("cubic_reflection");
			break;
		}
	}

	void MaterialSerializer::writeRotationEffect(const TextureUnitState::TextureEffect effect, const TextureUnitState *pTex)
	{
		if (effect.arg1)
		{
			writeAttribute(3, "rotate_anim");
			writeValue(StringConverter::toString(effect.arg1));
		}
	}

	void MaterialSerializer::writeTransformEffect(const TextureUnitState::TextureEffect effect, const TextureUnitState *pTex)
	{
		writeAttribute(3, "wave_xform");
		
		switch (effect.type)
		{
		case TextureUnitState::TT_ROTATE:
			writeValue("rotate");
			break;
		case TextureUnitState::TT_SCALE_U:
			writeValue("scale_x");
			break;
		case TextureUnitState::TT_SCALE_V:
			writeValue("scale_u");
			break;
		case TextureUnitState::TT_TRANSLATE_U:
			writeValue("scroll_x");
			break;
		case TextureUnitState::TT_TRANSLATE_V:
			writeValue("scroll_y");
			break;
		}

		switch (effect.waveType)
		{
		case WFT_INVERSE_SAWTOOTH:
			writeValue("inverse_sawtooth");
			break;
		case WFT_SAWTOOTH:
			writeValue("sawtooth");
			break;
		case WFT_SINE:
			writeValue("sine");
			break;
		case WFT_SQUARE:
			writeValue("square");
			break;
		case WFT_TRIANGLE:
			writeValue("triangle");
			break;
		}
		
		writeValue(StringConverter::toString(effect.base));
		writeValue(StringConverter::toString(effect.frequency));
		writeValue(StringConverter::toString(effect.phase));
		writeValue(StringConverter::toString(effect.amplitude));
	}

	void MaterialSerializer::writeScrollEffect(const TextureUnitState::TextureEffect effect, const TextureUnitState *pTex)
	{
		if (effect.arg1 || effect.arg2)
		{
			writeAttribute(3, "scroll_anim");
			writeValue(StringConverter::toString(effect.arg1));
			writeValue(StringConverter::toString(effect.arg2));
		}
	}

	void MaterialSerializer::writeSceneBlendFactor(const SceneBlendFactor sbf)
	{
		switch (sbf)
		{
		case SBF_DEST_ALPHA:
			writeValue("dest_alpha");
			break;
		case SBF_DEST_COLOUR:
			writeValue("dest_colour");
			break;
		case SBF_ONE:
			writeValue("one");
			break;
		case SBF_ONE_MINUS_DEST_ALPHA:
			writeValue("one_minus_dest_alpha");
			break;
		case SBF_ONE_MINUS_DEST_COLOUR:
			writeValue("one_minus_dest_colour");
			break;
		case SBF_ONE_MINUS_SOURCE_ALPHA:
			writeValue("one_minus_src_alpha");
			break;
		case SBF_ONE_MINUS_SOURCE_COLOUR:
			writeValue("one_minus_src_colour");
			break;
		case SBF_SOURCE_ALPHA:
			writeValue("src_alpha");
			break;
		case SBF_SOURCE_COLOUR:
			writeValue("src_colour");
			break;
		case SBF_ZERO:
			writeValue("zero");
			break;
		}
	}

	void MaterialSerializer::writeSceneBlendFactor(const SceneBlendFactor sbf_src, const SceneBlendFactor sbf_dst)
	{
		if (sbf_src == SBF_ONE && sbf_dst == SBF_ONE )
			writeValue("add");
		else if (sbf_src == SBF_SOURCE_COLOUR && sbf_dst == SBF_ONE_MINUS_SOURCE_COLOUR)
			writeValue("modulate");
		else if (sbf_src == SBF_SOURCE_ALPHA && sbf_dst == SBF_ONE_MINUS_SOURCE_ALPHA)
			writeValue("alpha_blend");
		else
		{
			writeSceneBlendFactor(sbf_src);
			writeSceneBlendFactor(sbf_dst);
		}
	}

	void MaterialSerializer::writeCompareFunction(const CompareFunction cf)
	{
		switch (cf)
		{
		case CMPF_ALWAYS_FAIL:
			writeValue("always_fail");
			break;
		case CMPF_ALWAYS_PASS:
			writeValue("always_pass");
			break;
		case CMPF_EQUAL:
			writeValue("equal");
			break;
		case CMPF_GREATER:
			writeValue("greater");
			break;
		case CMPF_GREATER_EQUAL:
			writeValue("greater_equal");
			break;
		case CMPF_LESS:
			writeValue("less");
			break;
		case CMPF_LESS_EQUAL:
			writeValue("less_equal");
			break;
		case CMPF_NOT_EQUAL:
			writeValue("not_equal");
			break;
		}
	}

	void MaterialSerializer::writeColourValue(const ColourValue &colour, bool writeAlpha)
	{
		writeValue(StringConverter::toString(colour.r));
		writeValue(StringConverter::toString(colour.g));
		writeValue(StringConverter::toString(colour.b));
		if (writeAlpha)
			writeValue(StringConverter::toString(colour.a));
	}

	void MaterialSerializer::writeLayerBlendOperationEx(const LayerBlendOperationEx op)
	{
		switch (op)
		{
		case LBX_ADD:
			writeValue("add");
			break;
		case LBX_ADD_SIGNED:
			writeValue("add_signed");
			break;
		case LBX_ADD_SMOOTH:
			writeValue("add_smooth");
			break;
		case LBX_BLEND_CURRENT_ALPHA:
			writeValue("blend_current_alpha");
			break;
		case LBX_BLEND_DIFFUSE_ALPHA:
			writeValue("blend_diffuse_alpha");
			break;
		case LBX_BLEND_MANUAL:
			writeValue("blend_manual");
			break;
		case LBX_BLEND_TEXTURE_ALPHA:
			writeValue("blend_texture_alpha");
			break;
		case LBX_MODULATE:
			writeValue("modulate");
			break;
		case LBX_MODULATE_X2:
			writeValue("modulate_x2");
			break;
		case LBX_MODULATE_X4:
			writeValue("modulate_x4");
			break;
		case LBX_SOURCE1:
			writeValue("source1");
			break;
		case LBX_SOURCE2:
			writeValue("source2");
			break;
		case LBX_SUBTRACT:
			writeValue("subtract");
			break;
		case LBX_DOTPRODUCT:
			writeValue("dotproduct");
			break;
		}
	}

	void MaterialSerializer::writeLayerBlendSource(const LayerBlendSource lbs)
	{
		switch (lbs)
		{
		case LBS_CURRENT:
			writeValue("src_current");
			break;
		case LBS_DIFFUSE:
			writeValue("src_diffuse");
			break;
		case LBS_MANUAL:
			writeValue("src_manual");
			break;
		case LBS_SPECULAR:
			writeValue("src_specular");
			break;
		case LBS_TEXTURE:
			writeValue("src_texture");
			break;
		}
	}
}

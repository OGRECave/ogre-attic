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

#include "OgreMaterialSerializer.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"

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
		LogManager::getSingleton().logMessage("MaterialSerializer : parsing material " + pMat->getName() + " to queue.", LML_CRITICAL);
        // Name
        mBuffer += pMat->getName();
		beginSection();
		{
			//lighting
			if (mDefaults || 
				pMat->getLightingEnabled() != true)
			{
				writeAttribute("lighting");
				writeValue(pMat->getLightingEnabled() ? "on" : "off");
			}

			if (pMat->getLightingEnabled())
			{
				// Ambient
				if (mDefaults ||
					pMat->getAmbient().r != 1 ||
					pMat->getAmbient().g != 1 ||
					pMat->getAmbient().b != 1)
				{
					writeAttribute("ambient");
					writeColourValue(pMat->getAmbient());
				}

				// Diffuse
				if (mDefaults ||
					pMat->getDiffuse().r != 1 ||
					pMat->getDiffuse().g != 1 ||
					pMat->getDiffuse().b != 1)
				{
					writeAttribute("diffuse");
					writeColourValue(pMat->getDiffuse());
				}

				// Specular
				if (mDefaults ||
					pMat->getSpecular().r != 0 ||
					pMat->getSpecular().g != 0 ||
					pMat->getSpecular().b != 0 ||
					pMat->getSpecular().a != 1)
				{
					writeAttribute("specular");
					writeColourValue(pMat->getSpecular(), true);
				}

				// Emissive
				if (mDefaults ||
					pMat->getSelfIllumination().r != 0 ||
					pMat->getSelfIllumination().g != 0 ||
					pMat->getSelfIllumination().b != 0)
				{
					writeAttribute("emissive");
					writeColourValue(pMat->getSelfIllumination());
				}
			}

			// scene blend factor
			if (mDefaults || 
				pMat->getSourceBlendFactor() != SBF_ONE || 
				pMat->getDestBlendFactor() != SBF_ZERO)
			{
				writeAttribute("scene_blend");
				writeSceneBlendFactor(pMat->getSourceBlendFactor(), pMat->getDestBlendFactor());
			}

			//anisotropy level
			if (mDefaults || 
				pMat->getAnisotropy() != 1)
			{
				writeAttribute("anisotropy");
				writeValue(StringConverter::toString(pMat->getAnisotropy()));
			}

			//depth check
			if (mDefaults || 
				pMat->getDepthCheckEnabled() != true)
			{
				writeAttribute("depth_check");
				writeValue(pMat->getDepthCheckEnabled() ? "on" : "off");
			}

			//depth write
			if (mDefaults || 
				pMat->getDepthWriteEnabled() != true)
			{
				writeAttribute("depth_write");
				writeValue(pMat->getDepthWriteEnabled() ? "on" : "off");
			}

			//depth function
			if (mDefaults || 
				pMat->getDepthFunction() != CMPF_LESS_EQUAL)
			{
				writeAttribute("depth_func");
				writeCompareFunction(pMat->getDepthFunction());
			}

			//depth bias
			if (mDefaults || 
				pMat->getDepthBias() != 0)
			{
				writeAttribute("depth_bias");
				writeValue(StringConverter::toString(pMat->getDepthBias()));
			}

			// hardware culling mode
			if (mDefaults || 
				pMat->getCullingMode() != CULL_CLOCKWISE)
			{
				CullingMode hcm = pMat->getCullingMode();
				writeAttribute("cull_hardware");
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
				pMat->getManualCullingMode() != MANUAL_CULL_BACK)
			{
				ManualCullingMode scm = pMat->getManualCullingMode();
				writeAttribute("cull_software");
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
				pMat->getShadingMode() != SO_GOURAUD)
			{
				writeAttribute("shading");
				switch (pMat->getShadingMode())
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

			//filtering
			if (mDefaults || 
				pMat->getTextureFiltering() != TFO_BILINEAR)
			{
				writeAttribute("filtering");
				switch (pMat->getTextureFiltering())
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

			//fog override
			if (mDefaults || 
				pMat->getFogOverride() != false)
			{
				writeAttribute("fog_override");
				writeValue(pMat->getFogOverride() ? "true" : "false");
				if (pMat->getFogOverride())
				{
					switch (pMat->getFogMode())
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

					if (pMat->getFogMode() != FOG_NONE)
					{
						writeColourValue(pMat->getFogColour());
						writeValue(StringConverter::toString(pMat->getFogDensity()));
						writeValue(StringConverter::toString(pMat->getFogStart()));
						writeValue(StringConverter::toString(pMat->getFogEnd()));
					}
				}
			}
						
			// Nested texture layers
			if (pMat->getNumTextureLayers())
			{
				for (int i = 0; i < pMat->getNumTextureLayers(); ++i)
				{
					mBuffer += "\n";
					String tmp = "Texture layer #";
					tmp += StringConverter::toString(i);
					writeComment(tmp);
					beginSubSection();
					writeTextureLayer(pMat->getTextureLayer(i));
					endSubSection();
				}
			}
		}
		endSection();
		LogManager::getSingleton().logMessage("MaterialSerializer : done.", LML_CRITICAL);
    }

	void MaterialSerializer::writeTextureLayer(const Material::TextureLayer *pTex)
    {
		LogManager::getSingleton().logMessage("MaterialSerializer : parsing texture layer.", LML_CRITICAL);

		//texture name
		if (pTex->getNumFrames() == 1 && pTex->getTextureName() != "")
		{
			writeSubAttribute("texture");
			writeValue(pTex->getTextureName());
		}

		//anim. texture
		if (pTex->getNumFrames() > 1 && !pTex->isCubic())
		{
			writeSubAttribute("anim_texture");
			for (int n = 0; n < pTex->getNumFrames(); n++)
				writeValue(pTex->getFrameTextureName(n));
			writeValue(StringConverter::toString(pTex->getAnimationDuration()));
		}

		//cubic texture
		if (pTex->isCubic())
		{
			writeSubAttribute("cubic_texture");
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
			writeAttribute("tex_anisotropy");
			writeValue(StringConverter::toString(pTex->getTextureAnisotropy()));
		}

		//texture coordinate set
		if (mDefaults || 
			pTex->getTextureCoordSet() != 0)
		{
			writeSubAttribute("tex_coord_set");
			writeValue(StringConverter::toString(pTex->getTextureCoordSet()));
		}

		//addressing mode
		if (mDefaults || 
			pTex->getTextureAddressingMode() != Ogre::Material::TextureLayer::TAM_WRAP)
		{
			writeSubAttribute("tex_address_mode");
			switch (pTex->getTextureAddressingMode())
			{
			case Ogre::Material::TextureLayer::TAM_CLAMP:
				writeValue("clamp");
				break;
			case Ogre::Material::TextureLayer::TAM_MIRROR:
				writeValue("mirror");
				break;
			case Ogre::Material::TextureLayer::TAM_WRAP:
				writeValue("wrap");
				break;
			}
		}
		
		//filtering
		if (mDefaults || 
			pTex->getTextureLayerFiltering() != TFO_BILINEAR)
		{
			writeSubAttribute("tex_filtering");
			switch (pTex->getTextureLayerFiltering())
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
			writeSubAttribute("alpha_rejection");
			writeCompareFunction(pTex->getAlphaRejectFunction());
			writeValue(StringConverter::toString(pTex->getAlphaRejectValue()));
		}

		// colour_op_ex
		if (mDefaults || 
			pTex->getColourBlendMode().operation != LBX_MODULATE ||
			pTex->getColourBlendMode().source1 != LBS_TEXTURE ||
			pTex->getColourBlendMode().source2 != LBS_CURRENT)
		{
			writeSubAttribute("colour_op_ex");
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
			writeSubAttribute("colour_op_multipass_fallback");
			writeSceneBlendFactor(pTex->getColourBlendFallbackSrc());
			writeSceneBlendFactor(pTex->getColourBlendFallbackDest());
		}

		// alpha_op_ex
		if (mDefaults || 
			pTex->getAlphaBlendMode().operation != LBX_MODULATE ||
			pTex->getAlphaBlendMode().source1 != LBS_TEXTURE ||
			pTex->getAlphaBlendMode().source2 != LBS_CURRENT)
		{
			writeSubAttribute("alpha_op_ex");
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
			writeSubAttribute("rotate");
			writeValue(StringConverter::toString(pTex->getTextureRotate()));
		}

		// scroll
		if (mDefaults ||
			pTex->getTextureUScroll() != 0 || 
			pTex->getTextureVScroll() != 0 )
		{
			writeSubAttribute("scroll");
			writeValue(StringConverter::toString(pTex->getTextureUScroll()));
			writeValue(StringConverter::toString(pTex->getTextureVScroll()));
		}

		EffectMap m_ef = pTex->getEffects();
		if (!m_ef.empty())
		{
			EffectMap::const_iterator it;
			for (it = m_ef.begin(); it != m_ef.end(); ++it)
			{
				Material::TextureLayer::TextureEffect ef = it->second;
				switch (ef.type)
				{
				case Material::TextureLayer::ET_ENVIRONMENT_MAP :
					writeEnvironmentMapEffect(ef, pTex);
					break;
				case Material::TextureLayer::ET_ROTATE :
					writeRotationEffect(ef, pTex);
					break;
				case Material::TextureLayer::ET_SCROLL :
					writeScrollEffect(ef, pTex);
					break;
				case Material::TextureLayer::ET_TRANSFORM :
					writeTransformEffect(ef, pTex);
					break;
				case Material::TextureLayer::ET_BUMP_MAP :
				default:
					break;
				}
			}
		}
	}

	void MaterialSerializer::writeEnvironmentMapEffect(const Material::TextureLayer::TextureEffect effect, const Material::TextureLayer *pTex)
	{
		writeSubAttribute("env_map");
		if (effect.subtype == Material::TextureLayer::ENV_PLANAR)
			writeValue("planar");
		else
			writeValue("spherical");
	}

	void MaterialSerializer::writeRotationEffect(const Material::TextureLayer::TextureEffect effect, const Material::TextureLayer *pTex)
	{
		if (effect.arg1)
		{
			writeSubAttribute("rotate_anim");
			writeValue(StringConverter::toString(effect.arg1));
		}
	}

	void MaterialSerializer::writeTransformEffect(const Material::TextureLayer::TextureEffect effect, const Material::TextureLayer *pTex)
	{
		writeSubAttribute("wave_xform");
		
		switch (effect.type)
		{
		case Material::TextureLayer::TT_ROTATE:
			writeValue("rotate");
			break;
		case Material::TextureLayer::TT_SCALE_U:
			writeValue("scale_x");
			break;
		case Material::TextureLayer::TT_SCALE_V:
			writeValue("scale_u");
			break;
		case Material::TextureLayer::TT_TRANSLATE_U:
			writeValue("scroll_x");
			break;
		case Material::TextureLayer::TT_TRANSLATE_V:
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

	void MaterialSerializer::writeScrollEffect(const Material::TextureLayer::TextureEffect effect, const Material::TextureLayer *pTex)
	{
		if (effect.arg1 || effect.arg2)
		{
			writeSubAttribute("scroll_anim");
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

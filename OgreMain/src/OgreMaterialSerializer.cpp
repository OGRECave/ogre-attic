/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright (c) 2000-2005 The OGRE Team
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
#include "OgreMaterialManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreExternalTextureSourceManager.h"

namespace Ogre 
{

    //-----------------------------------------------------------------------
    // Internal parser methods
    //-----------------------------------------------------------------------
    void logParseError(const String& error, const MaterialScriptContext& context)
    {
        // log material name only if filename not specified
        if (context.filename.empty() && !context.material.isNull())
        {
            LogManager::getSingleton().logMessage(
                "Error in material " + context.material->getName() + 
                " : " + error);
        }
        else
        {
            if (!context.material.isNull())
            {
                LogManager::getSingleton().logMessage(
                    "Error in material " + context.material->getName() +
                    " at line " + StringConverter::toString(context.lineNo) + 
                    " of " + context.filename + ": " + error);
            }
            else
            {
                LogManager::getSingleton().logMessage(
                    "Error at line " + StringConverter::toString(context.lineNo) + 
                    " of " + context.filename + ": " + error);
            }
        }
    }
    //-----------------------------------------------------------------------
    ColourValue _parseColourValue(StringVector& vecparams)
    {
        return ColourValue(
            StringConverter::parseReal(vecparams[0]) ,
            StringConverter::parseReal(vecparams[1]) ,
            StringConverter::parseReal(vecparams[2]) ,
            (vecparams.size()==4) ? StringConverter::parseReal(vecparams[3]) : 1.0f ) ;
    }
    //-----------------------------------------------------------------------
    FilterOptions convertFiltering(const String& s)
    {
        if (s == "none")
        {
            return FO_NONE;
        }
        else if (s == "point")
        {
            return FO_POINT;
        }
        else if (s == "linear")
        {
            return FO_LINEAR;
        }
        else if (s == "anisotropic")
        {
            return FO_ANISOTROPIC;
        }

        return FO_POINT;
    }
    //-----------------------------------------------------------------------
    bool parseAmbient(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 1, 3 or 4 parameters 
        if (vecparams.size() == 1) {
            if(vecparams[0] == "vertexcolour") {
               context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() | TVC_AMBIENT);
            } else {
                logParseError(
                    "Bad ambient attribute, single parameter flag must be 'vertexcolour'", 
                    context);
            }
        } 
        else if (vecparams.size() == 3 || vecparams.size() == 4)
        {
            context.pass->setAmbient( _parseColourValue(vecparams) );
            context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() & ~TVC_AMBIENT);
        }
        else 
        {
            logParseError(
                "Bad ambient attribute, wrong number of parameters (expected 1, 3 or 4)", 
                context);
        }
        return false;
    }
   //-----------------------------------------------------------------------
    bool parseDiffuse(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 1, 3 or 4 parameters 
        if (vecparams.size() == 1) {
            if(vecparams[0] == "vertexcolour") {
               context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() | TVC_DIFFUSE);
            } else {
                logParseError(
                    "Bad diffuse attribute, single parameter flag must be 'vertexcolour'", 
                    context);
            }
        }
        else if (vecparams.size() == 3 || vecparams.size() == 4)
        {
            context.pass->setDiffuse( _parseColourValue(vecparams) );
            context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() & ~TVC_DIFFUSE);
        }
        else
        {
            logParseError(
                "Bad diffuse attribute, wrong number of parameters (expected 1, 3 or 4)", 
                context);
        }        return false;
    }
    //-----------------------------------------------------------------------
    bool parseSpecular(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 2, 4 or 5 parameters 
        if(vecparams.size() == 2) 
        {   
            if(vecparams[0] == "vertexcolour") {
                context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() | TVC_SPECULAR);
                context.pass->setShininess(StringConverter::parseReal(vecparams[1]) );
            }
            else
            {
                logParseError(
                    "Bad specular attribute, double parameter statement must be 'vertexcolour <shininess>'", 
                    context);
            }
        } 
        else if(vecparams.size() == 4 || vecparams.size() == 5) 
        {
            context.pass->setSpecular(
                StringConverter::parseReal(vecparams[0]), 
                StringConverter::parseReal(vecparams[1]), 
                StringConverter::parseReal(vecparams[2]), 
                vecparams.size() == 5? 
                    StringConverter::parseReal(vecparams[3]) : 1.0f);
            context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() & ~TVC_SPECULAR);
            context.pass->setShininess(
                StringConverter::parseReal(vecparams[vecparams.size() - 1]) );
        }
        else 
        {
            logParseError(
                "Bad specular attribute, wrong number of parameters (expected 2, 4 or 5)",
                context);
        }
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseEmissive(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 1, 3 or 4 parameters 
        if (vecparams.size() == 1) {
            if(vecparams[0] == "vertexcolour") {
               context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() | TVC_EMISSIVE);
            } else {
                logParseError(
                    "Bad emissive attribute, single parameter flag must be 'vertexcolour'", 
                    context);
            }
        }
        else if (vecparams.size() == 3 || vecparams.size() == 4)
        {
            context.pass->setSelfIllumination( _parseColourValue(vecparams) );
            context.pass->setVertexColourTracking(context.pass->getVertexColourTracking() & ~TVC_EMISSIVE);
        }
        else
        {
            logParseError(
                "Bad emissive attribute, wrong number of parameters (expected 1, 3 or 4)", 
                context);
        }
        return false;
    }
    //-----------------------------------------------------------------------
    SceneBlendFactor convertBlendFactor(const String& param)
    {
        if (param == "one")
            return SBF_ONE;
        else if (param == "zero")
            return SBF_ZERO;
        else if (param == "dest_colour")
            return SBF_DEST_COLOUR;
        else if (param == "src_colour")
            return SBF_SOURCE_COLOUR;
        else if (param == "one_minus_dest_colour")
            return SBF_ONE_MINUS_DEST_COLOUR;
        else if (param == "one_minus_src_colour")
            return SBF_ONE_MINUS_SOURCE_COLOUR;
        else if (param == "dest_alpha")
            return SBF_DEST_ALPHA;
        else if (param == "src_alpha")
            return SBF_SOURCE_ALPHA;
        else if (param == "one_minus_dest_alpha")
            return SBF_ONE_MINUS_DEST_ALPHA;
        else if (param == "one_minus_src_alpha")
            return SBF_ONE_MINUS_SOURCE_ALPHA;
        else
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid blend factor.", "convertBlendFactor");
        }


    }
    //-----------------------------------------------------------------------
    bool parseSceneBlend(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        // Should be 1 or 2 params 
        if (vecparams.size() == 1)
        {
            //simple
            SceneBlendType stype;
            if (vecparams[0] == "add")
                stype = SBT_ADD;
            else if (vecparams[0] == "modulate")
                stype = SBT_MODULATE;
			else if (vecparams[0] == "colour_blend")
				stype = SBT_TRANSPARENT_COLOUR;
            else if (vecparams[0] == "alpha_blend")
                stype = SBT_TRANSPARENT_ALPHA;
            else
            {
                logParseError(
                    "Bad scene_blend attribute, unrecognised parameter '" + vecparams[0] + "'",
                    context);
                return false;
            }
            context.pass->setSceneBlending(stype);

        }
        else if (vecparams.size() == 2)
        {
            //src/dest
            SceneBlendFactor src, dest;

            try {
                src = convertBlendFactor(vecparams[0]);
                dest = convertBlendFactor(vecparams[1]);
                context.pass->setSceneBlending(src,dest);
            }
            catch (Exception& e)
            {
                logParseError("Bad scene_blend attribute, " + e.getFullDescription(), context);
            }

        }
        else
        {
            logParseError(
                "Bad scene_blend attribute, wrong number of parameters (expected 1 or 2)", 
                context);
        }

        return false;

    }
    //-----------------------------------------------------------------------
    CompareFunction convertCompareFunction(const String& param)
    {
        if (param == "always_fail")
            return CMPF_ALWAYS_FAIL;
        else if (param == "always_pass")
            return CMPF_ALWAYS_PASS;
        else if (param == "less")
            return CMPF_LESS;
        else if (param == "less_equal")
            return CMPF_LESS_EQUAL;
        else if (param == "equal")
            return CMPF_EQUAL;
        else if (param == "not_equal")
            return CMPF_NOT_EQUAL;
        else if (param == "greater_equal")
            return CMPF_GREATER_EQUAL;
        else if (param == "greater")
            return CMPF_GREATER;
        else
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid compare function", "convertCompareFunction");

    }
    //-----------------------------------------------------------------------
    bool parseDepthCheck(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setDepthCheckEnabled(true);
        else if (params == "off")
            context.pass->setDepthCheckEnabled(false);
        else
            logParseError(
            "Bad depth_check attribute, valid parameters are 'on' or 'off'.", 
            context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseDepthWrite(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setDepthWriteEnabled(true);
        else if (params == "off")
            context.pass->setDepthWriteEnabled(false);
        else
            logParseError(
                "Bad depth_write attribute, valid parameters are 'on' or 'off'.", 
                context);
        return false;
    }

    //-----------------------------------------------------------------------
    bool parseDepthFunc(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        try {
            CompareFunction func = convertCompareFunction(params);
            context.pass->setDepthFunction(func);
        }
        catch (...)
        {
            logParseError("Bad depth_func attribute, invalid function parameter.", context);
        }

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseColourWrite(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.pass->setColourWriteEnabled(true);
        else if (params == "off")
            context.pass->setColourWriteEnabled(false);
        else
            logParseError(
                "Bad colour_write attribute, valid parameters are 'on' or 'off'.", 
                context);
        return false;
    }

    //-----------------------------------------------------------------------
    bool parseCullHardware(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="none")
            context.pass->setCullingMode(CULL_NONE);
        else if (params=="anticlockwise")
            context.pass->setCullingMode(CULL_ANTICLOCKWISE);
        else if (params=="clockwise")
            context.pass->setCullingMode(CULL_CLOCKWISE);
        else
            logParseError(
                "Bad cull_hardware attribute, valid parameters are "
                "'none', 'clockwise' or 'anticlockwise'.", context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseCullSoftware(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="none")
            context.pass->setManualCullingMode(MANUAL_CULL_NONE);
        else if (params=="back")
            context.pass->setManualCullingMode(MANUAL_CULL_BACK);
        else if (params=="front")
            context.pass->setManualCullingMode(MANUAL_CULL_FRONT);
        else
            logParseError(
                "Bad cull_software attribute, valid parameters are 'none', "
                "'front' or 'back'.", context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseLighting(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="on")
            context.pass->setLightingEnabled(true);
        else if (params=="off")
            context.pass->setLightingEnabled(false);
        else
            logParseError(
                "Bad lighting attribute, valid parameters are 'on' or 'off'.", context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseMaxLights(String& params, MaterialScriptContext& context)
    {
		context.pass->setMaxSimultaneousLights(StringConverter::parseInt(params));
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseIteration(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 1 && vecparams.size() != 2)
        {
            logParseError("Bad iteration attribute, expected 1 or 2 parameters.", context);
            return false;
        }

        if (vecparams[0]=="once")
            context.pass->setRunOncePerLight(false);
        else if (vecparams[0]=="once_per_light")
        {
            if (vecparams.size() == 2)
            {
                // Parse light type
                if (vecparams[1] == "directional")
                {
                    context.pass->setRunOncePerLight(true, true, Light::LT_DIRECTIONAL);
                }
                else if (vecparams[1] == "point")
                {
                    context.pass->setRunOncePerLight(true, true, Light::LT_POINT);
                }
                else if (vecparams[1] == "spot")
                {
                    context.pass->setRunOncePerLight(true, true, Light::LT_SPOTLIGHT);
                }
                else
                {
                    logParseError("Bad iteration attribute, valid values for second parameter "
                        "are 'point' or 'directional' or 'spot'.", context);
                }
            }
            else
            {
                context.pass->setRunOncePerLight(true, false);
            }

        }
        else
            logParseError(
                "Bad iteration attribute, valid parameters are 'once' or 'once_per_light'.", context);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseFogging(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams[0]=="true")
        {
            // if true, we need to see if they supplied all arguments, or just the 1... if just the one,
            // Assume they want to disable the default fog from effecting this material.
            if( vecparams.size() == 8 )
            {
                FogMode mFogtype;
                if( vecparams[1] == "none" )
                    mFogtype = FOG_NONE;
                else if( vecparams[1] == "linear" )
                    mFogtype = FOG_LINEAR;
                else if( vecparams[1] == "exp" )
                    mFogtype = FOG_EXP;
                else if( vecparams[1] == "exp2" )
                    mFogtype = FOG_EXP2;
                else
                {
                    logParseError(
                        "Bad fogging attribute, valid parameters are "
                        "'none', 'linear', 'exp', or 'exp2'.", context);
                    return false;
                }

                context.pass->setFog(
                    true,
                    mFogtype,
                    ColourValue(
                    StringConverter::parseReal(vecparams[2]),
                    StringConverter::parseReal(vecparams[3]),
                    StringConverter::parseReal(vecparams[4])),
                    StringConverter::parseReal(vecparams[5]),
                    StringConverter::parseReal(vecparams[6]),
                    StringConverter::parseReal(vecparams[7])
                    );
            }
            else
            {
                context.pass->setFog(true);
            }
        }
        else if (vecparams[0]=="false")
            context.pass->setFog(false);
        else
            logParseError(
                "Bad fog_override attribute, valid parameters are 'true' or 'false'.", 
                context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseShading(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="flat")
            context.pass->setShadingMode(SO_FLAT);
        else if (params=="gouraud")
            context.pass->setShadingMode(SO_GOURAUD);
        else if (params=="phong")
            context.pass->setShadingMode(SO_PHONG);
        else
            logParseError("Bad shading attribute, valid parameters are 'flat', "
                "'gouraud' or 'phong'.", context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseFiltering(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        // Must be 1 or 3 parameters 
        if (vecparams.size() == 1)
        {
            // Simple format
            if (vecparams[0]=="none")
                context.textureUnit->setTextureFiltering(TFO_NONE);
            else if (vecparams[0]=="bilinear")
                context.textureUnit->setTextureFiltering(TFO_BILINEAR);
            else if (vecparams[0]=="trilinear")
                context.textureUnit->setTextureFiltering(TFO_TRILINEAR);
            else if (vecparams[0]=="anisotropic")
                context.textureUnit->setTextureFiltering(TFO_ANISOTROPIC);
            else
            {
                logParseError("Bad filtering attribute, valid parameters for simple format are "
                    "'none', 'bilinear', 'trilinear' or 'anisotropic'.", context);
                return false;
            }
        }
        else if (vecparams.size() == 3)
        {
            // Complex format
            context.textureUnit->setTextureFiltering(
                convertFiltering(vecparams[0]), 
                convertFiltering(vecparams[1]), 
                convertFiltering(vecparams[2]));


        }
        else
        {
            logParseError(
                "Bad filtering attribute, wrong number of parameters (expected 1 or 3)", 
                context);
        }

        return false;
    }
    //-----------------------------------------------------------------------
    // Texture layer attributes
    bool parseTexture(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() > 3)
        {
            logParseError("Invalid texture attribute - expected only 1, 2 or 3 parameters.", 
                context);
        }
        TextureType tt = TEX_TYPE_2D;
		int mips = -1; // When passed to TextureManager::load, this means default to default number of mipmaps
        if (vecparams.size() >= 2)
        {
            StringUtil::toLowerCase(vecparams[1]);
            if (vecparams[1] == "1d")
            {
                tt = TEX_TYPE_1D;
            }
            else if (vecparams[1] == "2d")
            {
                tt = TEX_TYPE_2D;
            }
            else if (vecparams[1] == "3d")
            {
                tt = TEX_TYPE_3D;
            }
            else if (vecparams[1] == "cubic")
            {
                tt = TEX_TYPE_CUBE_MAP;
            }  
			else
			{
				logParseError("Invalid texture type - "+vecparams[1]+".", 
                context);
			}
        }
		if (vecparams.size() >= 3)
		{
			StringUtil::toLowerCase(vecparams[2]);
			if (vecparams[2] == "unlimited")
            {
				mips = MIP_UNLIMITED;
            }
			else
			{
				mips = StringConverter::parseInt(vecparams[2]);
			}
		}
        context.textureUnit->setTextureName(vecparams[0], tt, mips);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseAnimTexture(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        size_t numParams = vecparams.size();
        // Determine which form it is
        // Must have at least 3 params though
        if (numParams < 3)
        {
            logParseError("Bad anim_texture attribute, wrong number of parameters "
                "(expected at least 3)", context);
            return false;
        }
        if (numParams == 3 && StringConverter::parseInt(vecparams[1]) != 0 )
        {
            // First form using base name & number of frames
            context.textureUnit->setAnimatedTextureName(
                vecparams[0], 
                StringConverter::parseInt(vecparams[1]), 
                StringConverter::parseReal(vecparams[2]));
        }
        else
        {
            // Second form using individual names
            context.textureUnit->setAnimatedTextureName(
                (String*)&vecparams[0], 
                numParams-1, 
                StringConverter::parseReal(vecparams[numParams-1]));
        }
        return false;

    }
    //-----------------------------------------------------------------------
    bool parseCubicTexture(String& params, MaterialScriptContext& context)
    {

        StringVector vecparams = StringUtil::split(params, " \t");
        size_t numParams = vecparams.size();

        // Get final param
        bool useUVW;
        String& uvOpt = vecparams[numParams-1];
		StringUtil::toLowerCase(uvOpt);
        if (uvOpt == "combineduvw")
            useUVW = true;
        else if (uvOpt == "separateuv")
            useUVW = false;
        else
        {
            logParseError("Bad cubic_texture attribute, final parameter must be "
                "'combinedUVW' or 'separateUV'.", context);
            return false;
        }
        // Determine which form it is
        if (numParams == 2)
        {
            // First form using base name
            context.textureUnit->setCubicTextureName(vecparams[0], useUVW);
        }
        else if (numParams == 7)
        {
            // Second form using individual names
            // Can use vecparams[0] as array start point
            context.textureUnit->setCubicTextureName((String*)&vecparams[0], useUVW);
        }
        else
        {
            logParseError(
                "Bad cubic_texture attribute, wrong number of parameters (expected 2 or 7)", 
                context);
            return false;
        }

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseTexCoord(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setTextureCoordSet(
            StringConverter::parseInt(params));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseTexAddressMode(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="wrap")
            context.textureUnit->setTextureAddressingMode(TextureUnitState::TAM_WRAP);
        else if (params=="mirror")
            context.textureUnit->setTextureAddressingMode(TextureUnitState::TAM_MIRROR);
        else if (params=="clamp")
            context.textureUnit->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
        else
            logParseError("Bad tex_address_mode attribute, valid parameters are "
                "'wrap', 'clamp' or 'mirror'.", context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseColourOp(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="replace")
            context.textureUnit->setColourOperation(LBO_REPLACE);
        else if (params=="add")
            context.textureUnit->setColourOperation(LBO_ADD);
        else if (params=="modulate")
            context.textureUnit->setColourOperation(LBO_MODULATE);
        else if (params=="alpha_blend")
            context.textureUnit->setColourOperation(LBO_ALPHA_BLEND);
        else
            logParseError("Bad colour_op attribute, valid parameters are "
                "'replace', 'add', 'modulate' or 'alpha_blend'.", context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseAlphaRejection(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError(
                "Bad alpha_rejection attribute, wrong number of parameters (expected 2)", 
                context);
            return false;
        }

        CompareFunction cmp;
        try {
            cmp = convertCompareFunction(vecparams[0]);
        }
        catch (...)
        {
            logParseError("Bad alpha_rejection attribute, invalid compare function.", context);
            return false;
        }

        context.pass->setAlphaRejectSettings(cmp, StringConverter::parseInt(vecparams[1]));

        return false;
    }
    //-----------------------------------------------------------------------
    LayerBlendOperationEx convertBlendOpEx(const String& param)
    {
        if (param == "source1")
            return LBX_SOURCE1;
        else if (param == "source2")
            return LBX_SOURCE2;
        else if (param == "modulate")
            return LBX_MODULATE;
        else if (param == "modulate_x2")
            return LBX_MODULATE_X2;
        else if (param == "modulate_x4")
            return LBX_MODULATE_X4;
        else if (param == "add")
            return LBX_ADD;
        else if (param == "add_signed")
            return LBX_ADD_SIGNED;
        else if (param == "add_smooth")
            return LBX_ADD_SMOOTH;
        else if (param == "subtract")
            return LBX_SUBTRACT;
        else if (param == "blend_diffuse_alpha")
            return LBX_BLEND_DIFFUSE_ALPHA;
        else if (param == "blend_texture_alpha")
            return LBX_BLEND_TEXTURE_ALPHA;
        else if (param == "blend_current_alpha")
            return LBX_BLEND_CURRENT_ALPHA;
        else if (param == "blend_manual")
            return LBX_BLEND_MANUAL;
        else if (param == "dotproduct")
            return LBX_DOTPRODUCT;
        else
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid blend function", "convertBlendOpEx");
    }
    //-----------------------------------------------------------------------
    LayerBlendSource convertBlendSource(const String& param)
    {
        if (param == "src_current")
            return LBS_CURRENT;
        else if (param == "src_texture")
            return LBS_TEXTURE;
        else if (param == "src_diffuse")
            return LBS_DIFFUSE;
        else if (param == "src_specular")
            return LBS_SPECULAR;
        else if (param == "src_manual")
            return LBS_MANUAL;
        else
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid blend source", "convertBlendSource");
    }
    //-----------------------------------------------------------------------
    bool parseColourOpEx(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        size_t numParams = vecparams.size();

        if (numParams < 3 || numParams > 10)
        {
            logParseError(
                "Bad colour_op_ex attribute, wrong number of parameters (expected 3 to 10)", 
                context);
            return false;
        }
        LayerBlendOperationEx op;
        LayerBlendSource src1, src2;
        Real manual = 0.0;
        ColourValue colSrc1 = ColourValue::White;
        ColourValue colSrc2 = ColourValue::White;

        try {
            op = convertBlendOpEx(vecparams[0]);
            src1 = convertBlendSource(vecparams[1]);
            src2 = convertBlendSource(vecparams[2]);

            if (op == LBX_BLEND_MANUAL)
            {
                if (numParams < 4)
                {
                    logParseError("Bad colour_op_ex attribute, wrong number of parameters "
                        "(expected 4 for manual blend)", context);
                    return false;
                }
                manual = StringConverter::parseReal(vecparams[3]);
            }

            if (src1 == LBS_MANUAL)
            {
                unsigned int parIndex = 3;
                if (op == LBX_BLEND_MANUAL)
                    parIndex++;

                if (numParams < parIndex + 3)
                {
                    logParseError("Bad colour_op_ex attribute, wrong number of parameters "
                        "(expected " + StringConverter::toString(parIndex + 3) + ")", context);
                    return false;
                }

                colSrc1.r = StringConverter::parseReal(vecparams[parIndex++]);
                colSrc1.g = StringConverter::parseReal(vecparams[parIndex++]);
                colSrc1.b = StringConverter::parseReal(vecparams[parIndex++]);
                if (numParams > parIndex)
                {
                    colSrc1.a = StringConverter::parseReal(vecparams[parIndex]);
                }
                else
                {
                    colSrc1.a = 1.0f;
                }
            }

            if (src2 == LBS_MANUAL)
            {
                unsigned int parIndex = 3;
                if (op == LBX_BLEND_MANUAL)
                    parIndex++;
                if (src1 == LBS_MANUAL)
                    parIndex += 3;

                if (numParams < parIndex + 3)
                {
                    logParseError("Bad colour_op_ex attribute, wrong number of parameters "
                        "(expected " + StringConverter::toString(parIndex + 3) + ")", context);
                    return false;
                }

                colSrc2.r = StringConverter::parseReal(vecparams[parIndex++]);
                colSrc2.g = StringConverter::parseReal(vecparams[parIndex++]);
                colSrc2.b = StringConverter::parseReal(vecparams[parIndex++]);
                if (numParams > parIndex)
                {
                    colSrc2.a = StringConverter::parseReal(vecparams[parIndex]);
                }
                else
                {
                    colSrc2.a = 1.0f;
                }
            }
        }
        catch (Exception& e)
        {
            logParseError("Bad colour_op_ex attribute, " + e.getFullDescription(), context);
            return false;
        }

        context.textureUnit->setColourOperationEx(op, src1, src2, colSrc1, colSrc2, manual);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseColourOpFallback(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Bad colour_op_multipass_fallback attribute, wrong number "
                "of parameters (expected 2)", context);
            return false;
        }

        //src/dest
        SceneBlendFactor src, dest;

        try {
            src = convertBlendFactor(vecparams[0]);
            dest = convertBlendFactor(vecparams[1]);
            context.textureUnit->setColourOpMultipassFallback(src,dest);
        }
        catch (Exception& e)
        {
            logParseError("Bad colour_op_multipass_fallback attribute, " 
                + e.getFullDescription(), context);
        }
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseAlphaOpEx(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        size_t numParams = vecparams.size();
        if (numParams < 3 || numParams > 6)
        {
            logParseError("Bad alpha_op_ex attribute, wrong number of parameters "
                "(expected 3 to 6)", context);
            return false;
        }
        LayerBlendOperationEx op;
        LayerBlendSource src1, src2;
        Real manual = 0.0;
        Real arg1 = 1.0, arg2 = 1.0;

        try {
            op = convertBlendOpEx(vecparams[0]);
            src1 = convertBlendSource(vecparams[1]);
            src2 = convertBlendSource(vecparams[2]);
            if (op == LBX_BLEND_MANUAL)
            {
                if (numParams != 4)
                {
                    logParseError("Bad alpha_op_ex attribute, wrong number of parameters "
                        "(expected 4 for manual blend)", context);
                    return false;
                }
                manual = StringConverter::parseReal(vecparams[3]);
            }
            if (src1 == LBS_MANUAL)
            {
                unsigned int parIndex = 3;
                if (op == LBX_BLEND_MANUAL)
                    parIndex++;

                if (numParams < parIndex)
                {
                    logParseError(
                        "Bad alpha_op_ex attribute, wrong number of parameters (expected " + 
                        StringConverter::toString(parIndex - 1) + ")", context);
                    return false;
                }

                arg1 = StringConverter::parseReal(vecparams[parIndex]);
            }

            if (src2 == LBS_MANUAL)
            {
                unsigned int parIndex = 3;
                if (op == LBX_BLEND_MANUAL)
                    parIndex++;
                if (src1 == LBS_MANUAL)
                    parIndex++;

                if (numParams < parIndex)
                {
                    logParseError(
                        "Bad alpha_op_ex attribute, wrong number of parameters "
                        "(expected " + StringConverter::toString(parIndex - 1) + ")", context);
                    return false;
                }

                arg2 = StringConverter::parseReal(vecparams[parIndex]);
            }
        }
        catch (Exception& e)
        {
            logParseError("Bad alpha_op_ex attribute, " + e.getFullDescription(), context);
            return false;
        }

        context.textureUnit->setAlphaOperation(op, src1, src2, arg1, arg2, manual);
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseEnvMap(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params=="off")
            context.textureUnit->setEnvironmentMap(false);
        else if (params=="spherical")
            context.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_CURVED);
        else if (params=="planar")
            context.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_PLANAR);
        else if (params=="cubic_reflection")
            context.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_REFLECTION);
        else if (params=="cubic_normal")
            context.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_NORMAL);
        else
            logParseError("Bad env_map attribute, valid parameters are 'off', "
                "'spherical', 'planar', 'cubic_reflection' and 'cubic_normal'.", context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseScroll(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Bad scroll attribute, wrong number of parameters (expected 2)", context);
            return false;
        }
        context.textureUnit->setTextureScroll(
            StringConverter::parseReal(vecparams[0]), 
            StringConverter::parseReal(vecparams[1]));

    
        return false;
    }
    //-----------------------------------------------------------------------
    bool parseScrollAnim(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Bad scroll_anim attribute, wrong number of "
                "parameters (expected 2)", context);
            return false;
        }
        context.textureUnit->setScrollAnimation(
            StringConverter::parseReal(vecparams[0]), 
            StringConverter::parseReal(vecparams[1]));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseRotate(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setTextureRotate(
            StringConverter::parseAngle(params));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseRotateAnim(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setRotateAnimation(
            StringConverter::parseReal(params));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseScale(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2)
        {
            logParseError("Bad scale attribute, wrong number of parameters (expected 2)", context);
            return false;
        }
        context.textureUnit->setTextureScale(
            StringConverter::parseReal(vecparams[0]), 
            StringConverter::parseReal(vecparams[1]));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseWaveXform(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");

        if (vecparams.size() != 6)
        {
            logParseError("Bad wave_xform attribute, wrong number of parameters "
                "(expected 6)", context);
            return false;
        }
        TextureUnitState::TextureTransformType ttype;
        WaveformType waveType;
        // Check transform type
        if (vecparams[0]=="scroll_x")
            ttype = TextureUnitState::TT_TRANSLATE_U;
        else if (vecparams[0]=="scroll_y")
            ttype = TextureUnitState::TT_TRANSLATE_V;
        else if (vecparams[0]=="rotate")
            ttype = TextureUnitState::TT_ROTATE;
        else if (vecparams[0]=="scale_x")
            ttype = TextureUnitState::TT_SCALE_U;
        else if (vecparams[0]=="scale_y")
            ttype = TextureUnitState::TT_SCALE_V;
        else
        {
            logParseError("Bad wave_xform attribute, parameter 1 must be 'scroll_x', "
                "'scroll_y', 'rotate', 'scale_x' or 'scale_y'", context);
            return false;
        }
        // Check wave type
        if (vecparams[1]=="sine")
            waveType = WFT_SINE;
        else if (vecparams[1]=="triangle")
            waveType = WFT_TRIANGLE;
        else if (vecparams[1]=="square")
            waveType = WFT_SQUARE;
        else if (vecparams[1]=="sawtooth")
            waveType = WFT_SAWTOOTH;
        else if (vecparams[1]=="inverse_sawtooth")
            waveType = WFT_INVERSE_SAWTOOTH;
        else
        {
            logParseError("Bad wave_xform attribute, parameter 2 must be 'sine', "
                "'triangle', 'square', 'sawtooth' or 'inverse_sawtooth'", context);
            return false;
        }

        context.textureUnit->setTransformAnimation(
            ttype, 
            waveType, 
            StringConverter::parseReal(vecparams[2]), 
            StringConverter::parseReal(vecparams[3]),
            StringConverter::parseReal(vecparams[4]), 
            StringConverter::parseReal(vecparams[5]) );

        return false;
    }
	//-----------------------------------------------------------------------
	bool parseTransform(String& params, MaterialScriptContext& context)
	{
		StringVector vecparams = StringUtil::split(params, " \t");
		if (vecparams.size() != 16)
		{
			logParseError("Bad transform attribute, wrong number of parameters (expected 16)", context);
			return false;
		}
		Matrix4 xform(
			StringConverter::parseReal(vecparams[0]), 
			StringConverter::parseReal(vecparams[1]), 
			StringConverter::parseReal(vecparams[2]), 
			StringConverter::parseReal(vecparams[3]), 
			StringConverter::parseReal(vecparams[4]), 
			StringConverter::parseReal(vecparams[5]), 
			StringConverter::parseReal(vecparams[6]), 
			StringConverter::parseReal(vecparams[7]), 
			StringConverter::parseReal(vecparams[8]), 
			StringConverter::parseReal(vecparams[9]), 
			StringConverter::parseReal(vecparams[10]), 
			StringConverter::parseReal(vecparams[11]), 
			StringConverter::parseReal(vecparams[12]), 
			StringConverter::parseReal(vecparams[13]), 
			StringConverter::parseReal(vecparams[14]), 
			StringConverter::parseReal(vecparams[15]) );
		context.textureUnit->setTextureTransform(xform);


		return false;
	}
    //-----------------------------------------------------------------------
    bool parseDepthBias(String& params, MaterialScriptContext& context)
    {
        context.pass->setDepthBias(
            StringConverter::parseReal(params));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseAnisotropy(String& params, MaterialScriptContext& context)
    {
        context.textureUnit->setTextureAnisotropy(
            StringConverter::parseInt(params));

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseLodDistances(String& params, MaterialScriptContext& context)
    {
        StringVector vecparams = StringUtil::split(params, " \t");

        // iterate over the parameters and parse distances out of them
        Material::LodDistanceList lodList;
        StringVector::iterator i, iend;
        iend = vecparams.end();
        for (i = vecparams.begin(); i != iend; ++i)
        {
            lodList.push_back(StringConverter::parseReal(*i));
        }

        context.material->setLodLevels(lodList);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseLodIndex(String& params, MaterialScriptContext& context)
    {
        context.technique->setLodIndex(StringConverter::parseInt(params));
        return false;
    }
    //-----------------------------------------------------------------------
    void processManualProgramParam(size_t index, const String& commandname, 
        StringVector& vecparams, MaterialScriptContext& context)
    {
        // NB we assume that the first element of vecparams is taken up with either 
        // the index or the parameter name, which we ignore

        // Determine type
        size_t start, dims, roundedDims, i;
        bool isReal;

        StringUtil::toLowerCase(vecparams[1]);

        if (vecparams[1] == "matrix4x4")
        {
            dims = 16;
            isReal = true;
        }
        else if ((start = vecparams[1].find("float")) != String::npos)
        {
            // find the dimensionality
            start = vecparams[1].find_first_not_of("float");
            // Assume 1 if not specified
            if (start == String::npos)
            {
                dims = 1;
            }
            else
            {
                dims = StringConverter::parseInt(vecparams[1].substr(start));
            }
            isReal = true;
        }
        else if ((start = vecparams[1].find("int")) != String::npos)
        {
            // find the dimensionality
            start = vecparams[1].find_first_not_of("int");
            // Assume 1 if not specified
            if (start == String::npos)
            {
                dims = 1;
            }
            else
            {
                dims = StringConverter::parseInt(vecparams[1].substr(start));
            }
            isReal = false;
        }
        else
        {
            logParseError("Invalid " + commandname + " attribute - unrecognised "
                "parameter type " + vecparams[1], context);
            return;
        }

        if (vecparams.size() != 2 + dims)
        {
            logParseError("Invalid " + commandname + " attribute - you need " +
                StringConverter::toString(2 + dims) + " parameters for a parameter of "
                "type " + vecparams[1], context);
        }

        // Round dims to multiple of 4
        if (dims %4 != 0)
        {
            roundedDims = dims + 4 - (dims % 4);
        }
        else
        {
            roundedDims = dims;
        }

        // Now parse all the values
        if (isReal)
        {
            Real* realBuffer = new Real[roundedDims];
            // Do specified values
            for (i = 0; i < dims; ++i)
            {
                realBuffer[i] = StringConverter::parseReal(vecparams[i+2]);
            }
            // Fill up to multiple of 4 with zero
            for (; i < roundedDims; ++i)
            {
                realBuffer[i] = 0.0f;
            }
            // Set
            context.programParams->setConstant(index, realBuffer, roundedDims * 0.25);
            delete [] realBuffer;
        }
        else
        {
            int* intBuffer = new int[roundedDims];
            // Do specified values
            for (i = 0; i < dims; ++i)
            {
                intBuffer[i] = StringConverter::parseInt(vecparams[i+2]);
            }
            // Fill to multiple of 4 with 0
            for (; i < roundedDims; ++i)
            {
                intBuffer[i] = 0;
            }
            // Set
            context.programParams->setConstant(index, intBuffer, roundedDims * 0.25);
            delete [] intBuffer;
        }
    }
    //-----------------------------------------------------------------------
    void processAutoProgramParam(size_t index, const String& commandname, 
        StringVector& vecparams, MaterialScriptContext& context)
    {
        // NB we assume that the first element of vecparams is taken up with either 
        // the index or the parameter name, which we ignore

        bool extras = false;
		bool float_extras = false;
        GpuProgramParameters::AutoConstantType acType;

        StringUtil::toLowerCase(vecparams[1]);

        if (vecparams[1] == "world_matrix")
        {
            acType = GpuProgramParameters::ACT_WORLD_MATRIX;
        }
        else if (vecparams[1] == "world_matrix_array")
        {
            acType = GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY;
        }
        else if (vecparams[1] == "world_matrix_array_3x4")
        {
            acType = GpuProgramParameters::ACT_WORLD_MATRIX_ARRAY_3x4;
        }
        else if (vecparams[1] == "view_matrix")
        {
            acType = GpuProgramParameters::ACT_VIEW_MATRIX;
        }
		else if (vecparams[1] == "projection_matrix")
		{
			acType = GpuProgramParameters::ACT_PROJECTION_MATRIX;
		}
        else if (vecparams[1] == "viewproj_matrix")
        {
            acType = GpuProgramParameters::ACT_VIEWPROJ_MATRIX;
        }
        else if (vecparams[1] == "worldview_matrix")
        {
            acType = GpuProgramParameters::ACT_WORLDVIEW_MATRIX;
        }
        else if (vecparams[1] == "worldviewproj_matrix")
        {
            acType = GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX;
        }
        else if (vecparams[1] == "inverse_world_matrix")
        {
            acType = GpuProgramParameters::ACT_INVERSE_WORLD_MATRIX;
        }
		else if (vecparams[1] == "inverse_view_matrix")
		{
			acType = GpuProgramParameters::ACT_INVERSE_VIEW_MATRIX;
		}
        else if (vecparams[1] == "inverse_worldview_matrix")
        {
            acType = GpuProgramParameters::ACT_INVERSE_WORLDVIEW_MATRIX;
        }
        else if (vecparams[1] == "inverse_transpose_world_matrix")
        {
            acType = GpuProgramParameters::ACT_INVERSETRANSPOSE_WORLD_MATRIX;
        }
        else if (vecparams[1] == "inverse_transpose_worldview_matrix")
        {
            acType = GpuProgramParameters::ACT_INVERSETRANSPOSE_WORLDVIEW_MATRIX;
        }
		else if (vecparams[1] == "time_0_x") 
		{
			acType = GpuProgramParameters::ACT_TIME_0_X;
			float_extras = true;
		}
		else if (vecparams[1] == "costime_0_x") 
		{
			acType = GpuProgramParameters::ACT_COSTIME_0_X;
			float_extras = true;
		}
		else if (vecparams[1] == "sintime_0_x") 
		{
			acType = GpuProgramParameters::ACT_SINTIME_0_X;
			float_extras = true;
		}
		else if (vecparams[1] == "tantime_0_x") 
		{
			acType = GpuProgramParameters::ACT_TANTIME_0_X;
			float_extras = true;
		}
		else if (vecparams[1] == "time_0_x_packed") 
		{
			acType = GpuProgramParameters::ACT_TIME_0_X_PACKED;
			float_extras = true;
		}
		else if (vecparams[1] == "time_0_1") 
		{
			acType = GpuProgramParameters::ACT_TIME_0_1;
			float_extras = true;
		}
		else if (vecparams[1] == "costime_0_1") 
		{
			acType = GpuProgramParameters::ACT_COSTIME_0_1;
			float_extras = true;
		}
		else if (vecparams[1] == "sintime_0_1") 
		{
			acType = GpuProgramParameters::ACT_SINTIME_0_1;
			float_extras = true;
		}
		else if (vecparams[1] == "tantime_0_1") 
		{
			acType = GpuProgramParameters::ACT_TANTIME_0_1;
			float_extras = true;
		}
		else if (vecparams[1] == "time_0_1_packed") 
		{
			acType = GpuProgramParameters::ACT_TIME_0_1_PACKED;
			float_extras = true;
		}
		else if (vecparams[1] == "time_0_2pi") 
		{
			acType = GpuProgramParameters::ACT_TIME_0_2PI;
			float_extras = true;
		}
		else if (vecparams[1] == "costime_0_2pi") 
		{
			acType = GpuProgramParameters::ACT_COSTIME_0_2PI;
			float_extras = true;
		}
		else if (vecparams[1] == "sintime_0_2pi") 
		{
			acType = GpuProgramParameters::ACT_SINTIME_0_2PI;
			float_extras = true;
		}
		else if (vecparams[1] == "tantime_0_2pi") 
		{
			acType = GpuProgramParameters::ACT_TANTIME_0_2PI;
			float_extras = true;
		}
		else if (vecparams[1] == "time_0_2pi_packed") 
		{
			acType = GpuProgramParameters::ACT_TIME_0_2PI_PACKED;
			float_extras = true;
		}
		else if (vecparams[1] == "fps") 
		{
			acType = GpuProgramParameters::ACT_FPS;
		}
		else if (vecparams[1] == "viewport_width") 
		{
			acType = GpuProgramParameters::ACT_VIEWPORT_WIDTH;
		}
		else if (vecparams[1] == "viewport_height") 
		{
			acType = GpuProgramParameters::ACT_VIEWPORT_HEIGHT;
		}
		else if (vecparams[1] == "inverse_viewport_width") 
		{
			acType = GpuProgramParameters::ACT_INVERSE_VIEWPORT_WIDTH;
		}
		else if (vecparams[1] == "inverse_viewport_height") 
		{
			acType = GpuProgramParameters::ACT_INVERSE_VIEWPORT_HEIGHT;
		}
		else if (vecparams[1] == "view_direction") 
		{
			acType = GpuProgramParameters::ACT_VIEW_DIRECTION;
		}
		else if (vecparams[1] == "view_side_vector") 
		{
			acType = GpuProgramParameters::ACT_VIEW_SIDE_VECTOR;
		}
		else if (vecparams[1] == "view_up_vector") 
		{
			acType = GpuProgramParameters::ACT_VIEW_UP_VECTOR;
		}
		else if (vecparams[1] == "fov") 
		{
			acType = GpuProgramParameters::ACT_FOV;
		}
		else if (vecparams[1] == "near_clip_distance") 
		{
			acType = GpuProgramParameters::ACT_NEAR_CLIP_DISTANCE;
		}
		else if (vecparams[1] == "far_clip_distance") 
		{
			acType = GpuProgramParameters::ACT_FAR_CLIP_DISTANCE;
		}
		else if (vecparams[1] == "inverse_viewproj_matrix") 
		{
			acType = GpuProgramParameters::ACT_INVERSE_VIEWPROJ_MATRIX;
		}
		else if (vecparams[1] == "inverse_transpose_viewproj_matrix") 
		{
			acType = GpuProgramParameters::ACT_INVERSETRANSPOSE_VIEWPROJ_MATRIX;
		}
		else if (vecparams[1] == "transpose_viewproj_matrix") 
		{
			acType = GpuProgramParameters::ACT_TRANSPOSE_VIEWPROJ_MATRIX;
		}
		else if (vecparams[1] == "transpose_view_matrix") 
		{
			acType = GpuProgramParameters::ACT_TRANSPOSE_VIEW_MATRIX;
		}
		else if (vecparams[1] == "inverse_transpose_view_matrix") 
		{
			acType = GpuProgramParameters::ACT_INVERSETRANSPOSE_VIEW_MATRIX;
		}
		else if (vecparams[1] == "projection_matrix") 
		{
			acType = GpuProgramParameters::ACT_PROJECTION_MATRIX;
		}
		else if (vecparams[1] == "transpose_projection_matrix") 
		{
			acType = GpuProgramParameters::ACT_TRANSPOSE_PROJECTION_MATRIX;
		}
		else if (vecparams[1] == "inverse_projection_matrix") 
		{
			acType = GpuProgramParameters::ACT_INVERSE_PROJECTION_MATRIX;
		}
		else if (vecparams[1] == "inverse_transpose_projection_matrix") 
		{
			acType = GpuProgramParameters::ACT_INVERSETRANSPOSE_PROJECTION_MATRIX;
		}
		else if (vecparams[1] == "transpose_worldviewproj_matrix") 
		{
			acType = GpuProgramParameters::ACT_TRANSPOSE_WORLDVIEWPROJ_MATRIX;
		}
		else if (vecparams[1] == "inverse_worldviewproj_matrix") 
		{
			acType = GpuProgramParameters::ACT_INVERSE_WORLDVIEWPROJ_MATRIX;
		}
		else if (vecparams[1] == "inverse_transpose_worldviewproj_matrix") 
		{
			acType = GpuProgramParameters::ACT_INVERSETRANSPOSE_WORLDVIEWPROJ_MATRIX;
		}
		else if (vecparams[1] == "transpose_worldview_matrix") 
		{
			acType = GpuProgramParameters::ACT_TRANSPOSE_WORLDVIEW_MATRIX;
		}
		else if (vecparams[1] == "inverse_transpose_worldview_matrix") 
		{
			acType = GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLDVIEW_MATRIX;
		}
		else if (vecparams[1] == "transpose_world_matrix") 
		{
			acType = GpuProgramParameters::ACT_TRANSPOSE_WORLD_MATRIX;
		}
		else if (vecparams[1] == "inverse_transpose_world_matrix") 
		{
			acType = GpuProgramParameters::ACT_INVERSE_TRANSPOSE_WORLD_MATRIX;
		}
        else if (vecparams[1] == "light_diffuse_colour")
        {
            acType = GpuProgramParameters::ACT_LIGHT_DIFFUSE_COLOUR;
            extras = true;
        }
        else if (vecparams[1] == "light_specular_colour")
        {
            acType = GpuProgramParameters::ACT_LIGHT_SPECULAR_COLOUR;
            extras = true;
        }
        else if (vecparams[1] == "light_attenuation")
        {
            acType = GpuProgramParameters::ACT_LIGHT_ATTENUATION;
            extras = true;
        }
        else if (vecparams[1] == "light_position")
        {
            acType = GpuProgramParameters::ACT_LIGHT_POSITION;
            extras = true;
        }
        else if (vecparams[1] == "light_direction")
        {
            acType = GpuProgramParameters::ACT_LIGHT_DIRECTION;
            extras = true;
        }
        else if (vecparams[1] == "light_position_object_space")
        {
            acType = GpuProgramParameters::ACT_LIGHT_POSITION_OBJECT_SPACE;
            extras = true;
        }
        else if (vecparams[1] == "light_direction_object_space")
        {
            acType = GpuProgramParameters::ACT_LIGHT_DIRECTION_OBJECT_SPACE;
            extras = true;
        }
        else if (vecparams[1] == "ambient_light_colour")
        {
            acType = GpuProgramParameters::ACT_AMBIENT_LIGHT_COLOUR;
        }
        else if (vecparams[1] == "camera_position")
        {
            acType = GpuProgramParameters::ACT_CAMERA_POSITION;
        }
        else if (vecparams[1] == "camera_position_object_space")
        {
            acType = GpuProgramParameters::ACT_CAMERA_POSITION_OBJECT_SPACE;
        }
        else if (vecparams[1] == "texture_viewproj_matrix")
        {
            acType = GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX;
        }
        else if (vecparams[1] == "time")
        {
            // Special case!
            Real factor = 1.0f;
            if (vecparams.size() == 3)
            {
                factor = StringConverter::parseReal(vecparams[2]);
            }
            
            context.programParams->setConstantFromTime(index, factor);
            return;
        }
        else if (vecparams[1] == "custom")
        {
            acType = GpuProgramParameters::ACT_CUSTOM;
            extras = true;
        }
		else
		{
			logParseError("Invalid " + commandname + " attribute - "
				+ vecparams[1], context);
			return;

		}

        // Do we need any extra parameters?
        size_t extraParam = 0;
        if (extras)
        {
            if (vecparams.size() != 3)
            {
                logParseError("Invalid " + commandname + " attribute - "
                    "expected 3 parameters.", context);
                return;
            }
            extraParam = StringConverter::parseInt(vecparams[2]);
        }
		
		
		if (float_extras) {
			Real rData = StringConverter::parseReal(vecparams[2]);
			context.programParams->setAutoConstantReal(index, acType, rData);
		}
		else {
			context.programParams->setAutoConstant(index, acType, extraParam);
		}

    }
    //-----------------------------------------------------------------------
    bool parseParamIndexed(String& params, MaterialScriptContext& context)
    {
        // NB skip this if the program is not supported or could not be found
        if (context.program.isNull() || !context.program->isSupported())
        {
            return false;
        }

        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() < 3)
        {
            logParseError("Invalid param_indexed attribute - expected at least 3 parameters.", 
                context);
            return false;
        }

        // Get start index
        size_t index = StringConverter::parseInt(vecparams[0]);

        processManualProgramParam(index, "param_indexed", vecparams, context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseParamIndexedAuto(String& params, MaterialScriptContext& context)
    {
        // NB skip this if the program is not supported or could not be found
        if (context.program.isNull() || !context.program->isSupported())
        {
            return false;
        }

        StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2 && vecparams.size() != 3)
        {
            logParseError("Invalid param_indexed_auto attribute - expected 2 or 3 parameters.", 
                context);
            return false;
        }

        // Get start index
        size_t index = StringConverter::parseInt(vecparams[0]);

        processAutoProgramParam(index, "param_indexed_auto", vecparams, context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseParamNamed(String& params, MaterialScriptContext& context)
    {
        // NB skip this if the program is not supported or could not be found
        if (context.program.isNull() || !context.program->isSupported())
        {
            return false;
        }

        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() < 3)
        {
            logParseError("Invalid param_named attribute - expected at least 3 parameters.", 
                context);
            return false;
        }

        // Get start index from name
        size_t index;
        try {
            index = context.programParams->getParamIndex(vecparams[0]);
        }
        catch (Exception& e)
        {
            logParseError("Invalid param_named attribute - " + e.getFullDescription(), context);
            return false;
        }

        // TEST
        /*
        LogManager::getSingleton().logMessage("SETTING PARAMETER " + vecparams[0] + " as index " +
            StringConverter::toString(index));
        */
        processManualProgramParam(index, "param_named", vecparams, context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseParamNamedAuto(String& params, MaterialScriptContext& context)
    {
        // NB skip this if the program is not supported or could not be found
        if (context.program.isNull() || !context.program->isSupported())
        {
            return false;
        }

        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 2 && vecparams.size() != 3)
        {
            logParseError("Invalid param_indexed_auto attribute - expected 2 or 3 parameters.", 
                context);
            return false;
        }

        // Get start index from name
        size_t index;
        try {
            index = context.programParams->getParamIndex(vecparams[0]);
        }
        catch (Exception& e)
        {
            logParseError("Invalid param_named_auto attribute - " + e.getFullDescription(), context);
            return false;
        }

        processAutoProgramParam(index, "param_named_auto", vecparams, context);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseMaterial(String& params, MaterialScriptContext& context)
    {
        // Create a brand new material
        context.material = 
			MaterialManager::getSingleton().create(params, context.groupName);
		context.material->_notifyOrigin(context.filename);
        // Remove pre-created technique from defaults
        context.material->removeAllTechniques();

        // update section
        context.section = MSS_MATERIAL;

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parseTechnique(String& params, MaterialScriptContext& context)
    {
        // Create a new technique
        context.technique = context.material->createTechnique();

        // update section
        context.section = MSS_TECHNIQUE;

		//Increase technique level depth
		context.techLev += 1;

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parsePass(String& params, MaterialScriptContext& context)
    {
        // Create a new pass
        context.pass = context.technique->createPass();

        // update section
        context.section = MSS_PASS;

		//Increase pass level depth
		context.passLev += 1;

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parseTextureUnit(String& params, MaterialScriptContext& context)
    {
        // Create a new texture unit
        context.textureUnit = context.pass->createTextureUnitState();

        // update section
        context.section = MSS_TEXTUREUNIT;

		// Increase texture unit depth
		context.stateLev += 1;

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parseVertexProgramRef(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM_REF;

        context.program = GpuProgramManager::getSingleton().getByName(params);
        if (context.program.isNull())
        {
            // Unknown program
            logParseError("Invalid vertex_program_ref entry - vertex program " 
                + params + " has not been defined.", context);
            return true;
        }

        context.isProgramShadowCaster = false;
        context.isProgramShadowReceiver = false;
        
        // Set the vertex program for this pass
        context.pass->setVertexProgram(params);

        // Create params? Skip this if program is not supported
        if (context.program->isSupported())
        {
            context.programParams = context.pass->getVertexProgramParameters();
        }

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parseShadowCasterVertexProgramRef(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM_REF;

        context.program = GpuProgramManager::getSingleton().getByName(params);
        if (context.program.isNull())
        {
            // Unknown program
            logParseError("Invalid vertex_program_ref entry - vertex program " 
                + params + " has not been defined.", context);
            return true;
        }

        context.isProgramShadowCaster = true;
        context.isProgramShadowReceiver = false;

        // Set the vertex program for this pass
        context.pass->setShadowCasterVertexProgram(params);

        // Create params? Skip this if program is not supported
        if (context.program->isSupported())
        {
            context.programParams = context.pass->getShadowCasterVertexProgramParameters();
        }

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parseShadowReceiverVertexProgramRef(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM_REF;

        context.program = GpuProgramManager::getSingleton().getByName(params);
        if (context.program.isNull())
        {
            // Unknown program
            logParseError("Invalid vertex_program_ref entry - vertex program " 
                + params + " has not been defined.", context);
            return true;
        }

        context.isProgramShadowCaster = false;
        context.isProgramShadowReceiver = true;

        // Set the vertex program for this pass
        context.pass->setShadowReceiverVertexProgram(params);

        // Create params? Skip this if program is not supported
        if (context.program->isSupported())
        {
            context.programParams = context.pass->getShadowReceiverVertexProgramParameters();
        }

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parseFragmentProgramRef(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM_REF;

        context.program = GpuProgramManager::getSingleton().getByName(params);
        if (context.program.isNull())
        {
            // Unknown program
            logParseError("Invalid fragment_program_ref entry - fragment program " 
                + params + " has not been defined.", context);
            return true;
        }
        
        // Set the vertex program for this pass
        context.pass->setFragmentProgram(params);

        // Create params? Skip this if program is not supported
        if (context.program->isSupported())
        {
            context.programParams = context.pass->getFragmentProgramParameters();
        }

        // Return TRUE because this must be followed by a {
        return true;
    }
    //-----------------------------------------------------------------------
    bool parseVertexProgram(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM;

		// Create new program definition-in-progress
		context.programDef = new MaterialScriptProgramDefinition();
		context.programDef->progType = GPT_VERTEX_PROGRAM;
        context.programDef->supportsSkeletalAnimation = false;

		// Get name and language code
		StringVector vecparams = StringUtil::split(params, " \t");
		if (vecparams.size() != 2)
		{
            logParseError("Invalid vertex_program entry - expected "
				"2 parameters.", context);
            return true;
		}
		// Name, preserve case
		context.programDef->name = vecparams[0];
		// language code, make lower case
		context.programDef->language = vecparams[1];
		StringUtil::toLowerCase(context.programDef->language);

        // Return TRUE because this must be followed by a {
        return true;
	}
    //-----------------------------------------------------------------------
    bool parseFragmentProgram(String& params, MaterialScriptContext& context)
    {
        // update section
        context.section = MSS_PROGRAM;

		// Create new program definition-in-progress
		context.programDef = new MaterialScriptProgramDefinition();
		context.programDef->progType = GPT_FRAGMENT_PROGRAM;
		context.programDef->supportsSkeletalAnimation = false;

		// Get name and language code
		StringVector vecparams = StringUtil::split(params, " \t");
		if (vecparams.size() != 2)
		{
            logParseError("Invalid fragment_program entry - expected "
				"2 parameters.", context);
            return true;
		}
		// Name, preserve case
		context.programDef->name = vecparams[0];
		// language code, make lower case
		context.programDef->language = vecparams[1];
		StringUtil::toLowerCase(context.programDef->language);

		// Return TRUE because this must be followed by a {
        return true;
	
	}
    //-----------------------------------------------------------------------
    bool parseProgramSource(String& params, MaterialScriptContext& context)
    {
		// Source filename, preserve case
		context.programDef->source = params;

		return false;
	}
    //-----------------------------------------------------------------------
    bool parseProgramSkeletalAnimation(String& params, MaterialScriptContext& context)
    {
        // Source filename, preserve case
        context.programDef->supportsSkeletalAnimation 
            = StringConverter::parseBool(params);

        return false;
    }
    //-----------------------------------------------------------------------
    bool parseProgramSyntax(String& params, MaterialScriptContext& context)
    {
		// Syntax code, make lower case
        StringUtil::toLowerCase(params);
		context.programDef->syntax = params;

		return false;
	}
    //-----------------------------------------------------------------------
    bool parseProgramCustomParameter(String& params, MaterialScriptContext& context)
    {
		// This params object does not have the command stripped
		// Lower case the command, but not the value incase it's relevant
		// Split only up to first delimiter, program deals with the rest
		StringVector vecparams = StringUtil::split(params, " \t", 1);
		if (vecparams.size() != 2)
		{
            logParseError("Invalid custom program parameter entry; "
				"there must be a parameter name and at least one value.", 
				context);
            return false;
		}

		context.programDef->customParameters[vecparams[0]] = vecparams[1];

		return false;
	}

	//-----------------------------------------------------------------------
    bool parseTextureSource(String& params, MaterialScriptContext& context)
    {
		StringUtil::toLowerCase(params);
        StringVector vecparams = StringUtil::split(params, " \t");
        if (vecparams.size() != 1)
			logParseError("Invalid texture source attribute - expected 1 parameter.",                 context);
        //The only param should identify which ExternalTextureSource is needed
		ExternalTextureSourceManager::getSingleton().setCurrentPlugIn( vecparams[0] );

		if(	ExternalTextureSourceManager::getSingleton().getCurrentPlugIn() != 0 )
		{
			String tps;
			tps = StringConverter::toString( context.techLev ) + " "
				+ StringConverter::toString( context.passLev ) + " "
				+ StringConverter::toString( context.stateLev);

			ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->setParameter( "set_T_P_S", tps );
		}
			
        // update section
        context.section = MSS_TEXTURESOURCE;
        // Return TRUE because this must be followed by a {
        return true;
    }

    //-----------------------------------------------------------------------
    bool parseTextureCustomParameter(String& params, MaterialScriptContext& context)
    {
		// This params object does not have the command stripped
		// Split only up to first delimiter, program deals with the rest
		StringVector vecparams = StringUtil::split(params, " \t", 1);
		if (vecparams.size() != 2)
		{
            logParseError("Invalid texture parameter entry; "
				"there must be a parameter name and at least one value.", 
				context);
            return false;
		}
		
		if(	ExternalTextureSourceManager::getSingleton().getCurrentPlugIn() != 0 )
			////First is command, next could be a string with one or more values
			ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->setParameter( vecparams[0], vecparams[1] );
		
		return false;
	}
    //-----------------------------------------------------------------------
    bool parseReceiveShadows(String& params, MaterialScriptContext& context)
    {
        StringUtil::toLowerCase(params);
        if (params == "on")
            context.material->setReceiveShadows(true);
        else if (params == "off")
            context.material->setReceiveShadows(false);
        else
            logParseError(
            "Bad receive_shadows attribute, valid parameters are 'on' or 'off'.", 
            context);

        return false;

    }
    //-----------------------------------------------------------------------
    bool parseDefaultParams(String& params, MaterialScriptContext& context)
    {
        context.section = MSS_DEFAULT_PARAMETERS;
        // Should be a brace next
        return true;
    }

	//-----------------------------------------------------------------------
	bool parseTransparencyCastsShadows(String& params, MaterialScriptContext& context)
	{
        StringUtil::toLowerCase(params);
		if (params == "on")
			context.material->setTransparencyCastsShadows(true);
		else if (params == "off")
			context.material->setTransparencyCastsShadows(false);
		else
			logParseError(
			"Bad transparency_casts_shadows attribute, valid parameters are 'on' or 'off'.", 
			context);

		return false;

	}
	//-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    MaterialSerializer::MaterialSerializer()
    {
        // Set up root attribute parsers
        mRootAttribParsers.insert(AttribParserList::value_type("material", (ATTRIBUTE_PARSER)parseMaterial));
        mRootAttribParsers.insert(AttribParserList::value_type("vertex_program", (ATTRIBUTE_PARSER)parseVertexProgram));
        mRootAttribParsers.insert(AttribParserList::value_type("fragment_program", (ATTRIBUTE_PARSER)parseFragmentProgram));
        // Set up material attribute parsers
        mMaterialAttribParsers.insert(AttribParserList::value_type("lod_distances", (ATTRIBUTE_PARSER)parseLodDistances));
        mMaterialAttribParsers.insert(AttribParserList::value_type("receive_shadows", (ATTRIBUTE_PARSER)parseReceiveShadows));
		mMaterialAttribParsers.insert(AttribParserList::value_type("transparency_casts_shadows", (ATTRIBUTE_PARSER)parseTransparencyCastsShadows));
        mMaterialAttribParsers.insert(AttribParserList::value_type("technique", (ATTRIBUTE_PARSER)parseTechnique));
        // Set up technique attribute parsers
        mTechniqueAttribParsers.insert(AttribParserList::value_type("lod_index", (ATTRIBUTE_PARSER)parseLodIndex));
        mTechniqueAttribParsers.insert(AttribParserList::value_type("pass", (ATTRIBUTE_PARSER)parsePass));
        // Set up pass attribute parsers
        mPassAttribParsers.insert(AttribParserList::value_type("ambient", (ATTRIBUTE_PARSER)parseAmbient));
        mPassAttribParsers.insert(AttribParserList::value_type("diffuse", (ATTRIBUTE_PARSER)parseDiffuse));
        mPassAttribParsers.insert(AttribParserList::value_type("specular", (ATTRIBUTE_PARSER)parseSpecular));
        mPassAttribParsers.insert(AttribParserList::value_type("emissive", (ATTRIBUTE_PARSER)parseEmissive));
        mPassAttribParsers.insert(AttribParserList::value_type("scene_blend", (ATTRIBUTE_PARSER)parseSceneBlend));
        mPassAttribParsers.insert(AttribParserList::value_type("depth_check", (ATTRIBUTE_PARSER)parseDepthCheck));
        mPassAttribParsers.insert(AttribParserList::value_type("depth_write", (ATTRIBUTE_PARSER)parseDepthWrite));
        mPassAttribParsers.insert(AttribParserList::value_type("depth_func", (ATTRIBUTE_PARSER)parseDepthFunc));
		mPassAttribParsers.insert(AttribParserList::value_type("alpha_rejection", (ATTRIBUTE_PARSER)parseAlphaRejection));
        mPassAttribParsers.insert(AttribParserList::value_type("colour_write", (ATTRIBUTE_PARSER)parseColourWrite));
        mPassAttribParsers.insert(AttribParserList::value_type("cull_hardware", (ATTRIBUTE_PARSER)parseCullHardware));
        mPassAttribParsers.insert(AttribParserList::value_type("cull_software", (ATTRIBUTE_PARSER)parseCullSoftware));
        mPassAttribParsers.insert(AttribParserList::value_type("lighting", (ATTRIBUTE_PARSER)parseLighting));
        mPassAttribParsers.insert(AttribParserList::value_type("fog_override", (ATTRIBUTE_PARSER)parseFogging));
        mPassAttribParsers.insert(AttribParserList::value_type("shading", (ATTRIBUTE_PARSER)parseShading));
        mPassAttribParsers.insert(AttribParserList::value_type("depth_bias", (ATTRIBUTE_PARSER)parseDepthBias));
        mPassAttribParsers.insert(AttribParserList::value_type("texture_unit", (ATTRIBUTE_PARSER)parseTextureUnit));
        mPassAttribParsers.insert(AttribParserList::value_type("vertex_program_ref", (ATTRIBUTE_PARSER)parseVertexProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("shadow_caster_vertex_program_ref", (ATTRIBUTE_PARSER)parseShadowCasterVertexProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("shadow_receiver_vertex_program_ref", (ATTRIBUTE_PARSER)parseShadowReceiverVertexProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("fragment_program_ref", (ATTRIBUTE_PARSER)parseFragmentProgramRef));
        mPassAttribParsers.insert(AttribParserList::value_type("max_lights", (ATTRIBUTE_PARSER)parseMaxLights));
        mPassAttribParsers.insert(AttribParserList::value_type("iteration", (ATTRIBUTE_PARSER)parseIteration));
		mTextureUnitAttribParsers.insert(AttribParserList::value_type("texture_source", (ATTRIBUTE_PARSER)parseTextureSource));

        // Set up texture unit attribute parsers
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("texture", (ATTRIBUTE_PARSER)parseTexture));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("anim_texture", (ATTRIBUTE_PARSER)parseAnimTexture));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("cubic_texture", (ATTRIBUTE_PARSER)parseCubicTexture));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("tex_coord_set", (ATTRIBUTE_PARSER)parseTexCoord));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("tex_address_mode", (ATTRIBUTE_PARSER)parseTexAddressMode));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("colour_op", (ATTRIBUTE_PARSER)parseColourOp));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("colour_op_ex", (ATTRIBUTE_PARSER)parseColourOpEx));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("colour_op_multipass_fallback", (ATTRIBUTE_PARSER)parseColourOpFallback));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("alpha_op_ex", (ATTRIBUTE_PARSER)parseAlphaOpEx));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("env_map", (ATTRIBUTE_PARSER)parseEnvMap));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("scroll", (ATTRIBUTE_PARSER)parseScroll));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("scroll_anim", (ATTRIBUTE_PARSER)parseScrollAnim));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("rotate", (ATTRIBUTE_PARSER)parseRotate));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("rotate_anim", (ATTRIBUTE_PARSER)parseRotateAnim));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("scale", (ATTRIBUTE_PARSER)parseScale));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("wave_xform", (ATTRIBUTE_PARSER)parseWaveXform));
		mTextureUnitAttribParsers.insert(AttribParserList::value_type("transform", (ATTRIBUTE_PARSER)parseTransform));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("filtering", (ATTRIBUTE_PARSER)parseFiltering));
        mTextureUnitAttribParsers.insert(AttribParserList::value_type("max_anisotropy", (ATTRIBUTE_PARSER)parseAnisotropy));

        // Set up program reference attribute parsers
        mProgramRefAttribParsers.insert(AttribParserList::value_type("param_indexed", (ATTRIBUTE_PARSER)parseParamIndexed));
        mProgramRefAttribParsers.insert(AttribParserList::value_type("param_indexed_auto", (ATTRIBUTE_PARSER)parseParamIndexedAuto));
        mProgramRefAttribParsers.insert(AttribParserList::value_type("param_named", (ATTRIBUTE_PARSER)parseParamNamed));
        mProgramRefAttribParsers.insert(AttribParserList::value_type("param_named_auto", (ATTRIBUTE_PARSER)parseParamNamedAuto));

        // Set up program definition attribute parsers
        mProgramAttribParsers.insert(AttribParserList::value_type("source", (ATTRIBUTE_PARSER)parseProgramSource));
        mProgramAttribParsers.insert(AttribParserList::value_type("syntax", (ATTRIBUTE_PARSER)parseProgramSyntax));
        mProgramAttribParsers.insert(AttribParserList::value_type("includes_skeletal_animation", (ATTRIBUTE_PARSER)parseProgramSkeletalAnimation));
        mProgramAttribParsers.insert(AttribParserList::value_type("default_params", (ATTRIBUTE_PARSER)parseDefaultParams));
		
        // Set up program default param attribute parsers
        mProgramDefaultParamAttribParsers.insert(AttribParserList::value_type("param_indexed", (ATTRIBUTE_PARSER)parseParamIndexed));
        mProgramDefaultParamAttribParsers.insert(AttribParserList::value_type("param_indexed_auto", (ATTRIBUTE_PARSER)parseParamIndexedAuto));
        mProgramDefaultParamAttribParsers.insert(AttribParserList::value_type("param_named", (ATTRIBUTE_PARSER)parseParamNamed));
        mProgramDefaultParamAttribParsers.insert(AttribParserList::value_type("param_named_auto", (ATTRIBUTE_PARSER)parseParamNamedAuto));

        mScriptContext.section = MSS_NONE;
        mScriptContext.material.setNull();
        mScriptContext.technique = 0;
        mScriptContext.pass = 0;
        mScriptContext.textureUnit = 0;
        mScriptContext.program.setNull();
        mScriptContext.lineNo = 0;
        mScriptContext.filename = "";
		mScriptContext.techLev = -1;
		mScriptContext.passLev = -1;
		mScriptContext.stateLev = -1;

        mBuffer = "";
    }

    //-----------------------------------------------------------------------
    void MaterialSerializer::parseScript(DataStreamPtr& stream, const String& groupName)
    {
        String line;
        bool nextIsOpenBrace = false;

        mScriptContext.section = MSS_NONE;
        mScriptContext.material.setNull();
        mScriptContext.technique = 0;
        mScriptContext.pass = 0;
        mScriptContext.textureUnit = 0;
        mScriptContext.program.setNull();
        mScriptContext.lineNo = 0;
		mScriptContext.techLev = -1;
		mScriptContext.passLev = -1;
		mScriptContext.stateLev = -1;
        mScriptContext.filename = stream->getName();
		mScriptContext.groupName = groupName;
        while(!stream->eof())
        {
            line = stream->getLine();
            mScriptContext.lineNo++;
            
            // DEBUG LINE
            // LogManager::getSingleton().logMessage("About to attempt line(#" + 
            //    StringConverter::toString(mScriptContext.lineNo) + "): " + line);

            // Ignore comments & blanks
            if (!(line.length() == 0 || line.substr(0,2) == "//"))
            {
                if (nextIsOpenBrace)
                {
                    // NB, parser will have changed context already
                    if (line != "{")
                    {
                        logParseError("Expecting '{' but got " +
                            line + " instead.", mScriptContext);
                    }
                    nextIsOpenBrace = false;
                }
                else
                {
                    nextIsOpenBrace = parseScriptLine(line);
                }

            }
        }

        // Check all braces were closed
        if (mScriptContext.section != MSS_NONE)
        {
            logParseError("Unexpected end of file.", mScriptContext);
        }

		// Make sure we invalidate our context shared pointer (don't wanna hold on)
		mScriptContext.material.setNull();

    }
    //-----------------------------------------------------------------------
    bool MaterialSerializer::parseScriptLine(String& line)
    {
        switch(mScriptContext.section)
        {
        case MSS_NONE:
            if (line == "}")
            {
                logParseError("Unexpected terminating brace.", mScriptContext);
                return false;
            }
            else
            {
                // find & invoke a parser
                return invokeParser(line, mRootAttribParsers); 
            }
            break;
        case MSS_MATERIAL:
            if (line == "}")
            {
                // End of material
                mScriptContext.section = MSS_NONE;
                mScriptContext.material.setNull();
				//Reset all levels for next material
				mScriptContext.passLev = -1;
				mScriptContext.stateLev= -1;
				mScriptContext.techLev = -1;
            }
            else
            {
                // find & invoke a parser
                return invokeParser(line, mMaterialAttribParsers); 
            }
            break;
        case MSS_TECHNIQUE:
            if (line == "}")
            {
                // End of technique
                mScriptContext.section = MSS_MATERIAL;
                mScriptContext.technique = NULL;
				mScriptContext.passLev = -1;	//Reset pass level (yes, the pass level)
            }
            else
            {
                // find & invoke a parser
                return invokeParser(line, mTechniqueAttribParsers); 
            }
            break;
        case MSS_PASS:
            if (line == "}")
            {
                // End of pass
                mScriptContext.section = MSS_TECHNIQUE;
                mScriptContext.pass = NULL;
				mScriptContext.stateLev = -1;	//Reset state level (yes, the state level)
            }
            else
            {
                // find & invoke a parser
                return invokeParser(line, mPassAttribParsers); 
            }
            break;
        case MSS_TEXTUREUNIT:
            if (line == "}")
            {
                // End of texture unit
                mScriptContext.section = MSS_PASS;
                mScriptContext.textureUnit = NULL;
            }
            else
            {
                // find & invoke a parser
                return invokeParser(line, mTextureUnitAttribParsers); 
            }
            break;
		case MSS_TEXTURESOURCE:
			if( line == "}" )
			{
				//End texture source section
				//Finish creating texture here
				String sMaterialName = mScriptContext.material->getName();
				if(	ExternalTextureSourceManager::getSingleton().getCurrentPlugIn() != 0)
					ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->
					createDefinedTexture( sMaterialName, mScriptContext.groupName );
				//Revert back to texture unit
				mScriptContext.section = MSS_TEXTUREUNIT;
			}
			else
			{
				// custom texture parameter, use original line
				parseTextureCustomParameter(line, mScriptContext);
			}
			break;
        case MSS_PROGRAM_REF:
            if (line == "}")
            {
                // End of program
                mScriptContext.section = MSS_PASS;
                mScriptContext.program.setNull();
            }
            else
            {
                // find & invoke a parser
                return invokeParser(line, mProgramRefAttribParsers); 
            }
            break;
        case MSS_PROGRAM:
			// Program definitions are slightly different, they are deferred
			// until all the information required is known
            if (line == "}")
            {
                // End of program
				finishProgramDefinition();
                mScriptContext.section = MSS_NONE;
                delete mScriptContext.programDef;
                mScriptContext.defaultParamLines.clear();
                mScriptContext.programDef = NULL;
            }
            else
            {
                // find & invoke a parser
				// do this manually because we want to call a custom
				// routine when the parser is not found
				// First, split line on first divisor only
				StringVector splitCmd = StringUtil::split(line, " \t", 1);
				// Find attribute parser
				AttribParserList::iterator iparser = mProgramAttribParsers.find(splitCmd[0]);
				if (iparser == mProgramAttribParsers.end())
				{
					// custom parameter, use original line
					parseProgramCustomParameter(line, mScriptContext);
				}
				else
				{
                    String cmd = splitCmd.size() >= 2? splitCmd[1]:StringUtil::BLANK;
					// Use parser with remainder
                    return iparser->second(cmd, mScriptContext );
				}
				
            }
            break;
        case MSS_DEFAULT_PARAMETERS:
            if (line == "}")
            {
                // End of default parameters
                mScriptContext.section = MSS_PROGRAM;
            }
            else
            {
                // Save default parameter lines up until we finalise the program
                mScriptContext.defaultParamLines.push_back(line);
            }


            break;
        };

        return false;
    }
    //-----------------------------------------------------------------------
	void MaterialSerializer::finishProgramDefinition(void)
	{
		// Now it is time to create the program and propagate the parameters
		MaterialScriptProgramDefinition* def = mScriptContext.programDef;
        GpuProgramPtr gp;
		if (def->language == "asm")
		{
			// Native assembler
			// Validate
			if (def->source.empty())
			{
				logParseError("Invalid program definition for " + def->name +
					", you must specify a source file.", mScriptContext);
			}
			if (def->syntax.empty())
			{
				logParseError("Invalid program definition for " + def->name +
					", you must specify a syntax code.", mScriptContext);
			}
			// Create
			gp = GpuProgramManager::getSingleton().
				createProgram(def->name, mScriptContext.groupName, def->source, 
                    def->progType, def->syntax);

		}
		else
		{
			// High-level program
			// Validate
			if (def->source.empty())
			{
				logParseError("Invalid program definition for " + def->name +
					", you must specify a source file.", mScriptContext);
			}
			// Create
            try 
            {
			    HighLevelGpuProgramPtr hgp = HighLevelGpuProgramManager::getSingleton().
				    createProgram(def->name, mScriptContext.groupName, 
                        def->language, def->progType);
                // Assign to generalised version
                gp = hgp;
                // Set source file
                hgp->setSourceFile(def->source);

			    // Set custom parameters
			    std::map<String, String>::const_iterator i, iend;
			    iend = def->customParameters.end();
			    for (i = def->customParameters.begin(); i != iend; ++i)
			    {
				    if (!hgp->setParameter(i->first, i->second))
				    {
					    logParseError("Error in program " + def->name + 
						    " parameter " + i->first + " is not valid.", mScriptContext);
				    }
			    }
            }
            catch (Exception& e)
            {
                logParseError("Could not create GPU program '"
                    + def->name + "', error reported was: " + e.getFullDescription(), mScriptContext);
				mScriptContext.program.setNull();
            	mScriptContext.programParams.setNull();
				return;
            }
        }
        // Set skeletal animation option
        gp->setSkeletalAnimationIncluded(def->supportsSkeletalAnimation);
		// set origin
		gp->_notifyOrigin(mScriptContext.filename);

        // Set up to receive default parameters
        if (gp->isSupported() 
            && !mScriptContext.defaultParamLines.empty())
        {
            mScriptContext.programParams = gp->getDefaultParameters();
            mScriptContext.program = gp;
            StringVector::iterator i, iend;
            iend = mScriptContext.defaultParamLines.end();
            for (i = mScriptContext.defaultParamLines.begin();
                i != iend; ++i)
            {
                // find & invoke a parser
                // do this manually because we want to call a custom
                // routine when the parser is not found
                // First, split line on first divisor only
                StringVector splitCmd = StringUtil::split(*i, " \t", 1);
                // Find attribute parser
                AttribParserList::iterator iparser 
                    = mProgramDefaultParamAttribParsers.find(splitCmd[0]);
                if (iparser != mProgramDefaultParamAttribParsers.end())
                {
                    String cmd = splitCmd.size() >= 2? splitCmd[1]:StringUtil::BLANK;
                    // Use parser with remainder
                    iparser->second(cmd, mScriptContext );
                }

            }
            // Reset
            mScriptContext.program.setNull();
            mScriptContext.programParams.setNull();
        }

	}
    //-----------------------------------------------------------------------
	bool MaterialSerializer::invokeParser(String& line, AttribParserList& parsers)
    {
        // First, split line on first divisor only
        StringVector splitCmd(StringUtil::split(line, " \t", 1));

        // Find attribute parser
        AttribParserList::iterator iparser = parsers.find(splitCmd[0]);
        if (iparser == parsers.end())
        {
            // BAD command. BAD!
            logParseError("Unrecognised command: " + splitCmd[0], mScriptContext);
            return false;
        }
        else
        {
            String cmd;
            if(splitCmd.size() >= 2)
                cmd = splitCmd[1];
            // Use parser, make sure we have 2 params before using splitCmd[1]
            return iparser->second( cmd, mScriptContext );
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::exportMaterial(const MaterialPtr& pMat, const String &fileName, bool exportDefaults)
    {
        clearQueue();
        mDefaults = exportDefaults;
        writeMaterial(pMat);
        exportQueued(fileName);
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::exportQueued(const String &fileName)
    {
        if (mBuffer == "")
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Queue is empty !", "MaterialSerializer::exportQueued");

        LogManager::getSingleton().logMessage("MaterialSerializer : writing material(s) to material script : " + fileName, LML_CRITICAL);
        FILE *fp;
        fp = fopen(fileName.c_str(), "w");
        if (!fp)
            OGRE_EXCEPT(Exception::ERR_CANNOT_WRITE_TO_FILE, "Cannot create material file.",
            "MaterialSerializer::export");

        fputs(mBuffer.c_str(), fp);
        fclose(fp);
        LogManager::getSingleton().logMessage("MaterialSerializer : done.", LML_CRITICAL);
        clearQueue();
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::queueForExport(const MaterialPtr& pMat, bool clearQueued, bool exportDefaults)
    {
        if (clearQueued)
            clearQueue();

        mDefaults = exportDefaults;
        writeMaterial(pMat);
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::clearQueue()
    {
        mBuffer = "";
    }
    //-----------------------------------------------------------------------
    const String &MaterialSerializer::getQueuedAsString() const
    {
        return mBuffer;
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeMaterial(const MaterialPtr& pMat)
    {
        LogManager::getSingleton().logMessage("MaterialSerializer : writing material " + pMat->getName() + " to queue.", LML_CRITICAL);
        // Material name
        writeAttribute(0, "material " + pMat->getName());
        beginSection(0);
        {
            // Write LOD information
            Material::LodDistanceIterator distIt = pMat->getLodDistanceIterator();
            // Skip zero value
            if (distIt.hasMoreElements())
                distIt.getNext();
            String attributeVal;
            while (distIt.hasMoreElements())
            {
                Real sqdist = distIt.getNext();
                attributeVal.append(StringConverter::toString(Math::Sqrt(sqdist)));
                if (distIt.hasMoreElements())
                    attributeVal.append(" ");
            }
            if (!attributeVal.empty())
            {
                writeAttribute(1, "lod_distances");
                writeValue(attributeVal);
            }


            // Shadow receive
            if (mDefaults || 
                pMat->getReceiveShadows() != true)
            {
                writeAttribute(1, "receive_shadows");
                writeValue(pMat->getReceiveShadows() ? "on" : "off");
            }

			// When rendering shadows, treat transparent things as opaque?
			if (mDefaults || 
				pMat->getTransparencyCastsShadows() == true)
			{
				writeAttribute(1, "transparency_casts_shadows");
				writeValue(pMat->getTransparencyCastsShadows() ? "on" : "off");
			}

            // Iterate over techniques
            Material::TechniqueIterator it = pMat->getTechniqueIterator();
            while (it.hasMoreElements())
            {
                writeTechnique(it.getNext());
            }
        }
        endSection(0);
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeTechnique(const Technique* pTech)
    {
        // Technique header
        writeAttribute(1, "technique");
        beginSection(1);
        {
			// Lod index
			if (mDefaults || 
				pTech->getLodIndex() != 0)
			{
				writeAttribute(2, "lod_index");
				writeValue(StringConverter::toString(pTech->getLodIndex()));
			}


            // Iterate over passes
            Technique::PassIterator it = const_cast<Technique*>(pTech)->getPassIterator();
            while (it.hasMoreElements())
            {
                writePass(it.getNext());
            }
        }
        endSection(1);

    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writePass(const Pass* pPass)
    {
        writeAttribute(2, "pass");
        beginSection(2);
        {
            //lighting
            if (mDefaults || 
                pPass->getLightingEnabled() != true)
            {
                writeAttribute(3, "lighting");
                writeValue(pPass->getLightingEnabled() ? "on" : "off");
            }
			// max_lights
            if (mDefaults || 
                pPass->getMaxSimultaneousLights() != OGRE_MAX_SIMULTANEOUS_LIGHTS)
            {
                writeAttribute(3, "max_lights");
                writeValue(StringConverter::toString(pPass->getMaxSimultaneousLights()));
            }
			// iteration
            if (mDefaults || 
                pPass->getRunOncePerLight())
            {
                writeAttribute(3, "iteration");
                writeValue(pPass->getRunOncePerLight() ? "once_per_light" : "once");
                if (pPass->getRunOncePerLight() && pPass->getRunOnlyForOneLightType())
                {
                    switch (pPass->getOnlyLightType())
                    {
                    case Light::LT_DIRECTIONAL:
                        writeValue("directional");
                        break;
                    case Light::LT_POINT:
                        writeValue("point");
                        break;
                    case Light::LT_SPOTLIGHT:
                        writeValue("spot");
                        break;
                    };
                }
            }
			

            if (pPass->getLightingEnabled())
            {
                // Ambient
                if (mDefaults ||
                    pPass->getAmbient().r != 1 ||
                    pPass->getAmbient().g != 1 ||
                    pPass->getAmbient().b != 1 ||
                    pPass->getAmbient().a != 1)
                {
                    writeAttribute(3, "ambient");
                    writeColourValue(pPass->getAmbient(), true);
                }

                // Diffuse
                if (mDefaults ||
                    pPass->getDiffuse().r != 1 ||
                    pPass->getDiffuse().g != 1 ||
                    pPass->getDiffuse().b != 1 ||
                    pPass->getDiffuse().a != 1)
                {
                    writeAttribute(3, "diffuse");
                    writeColourValue(pPass->getDiffuse(), true);
                }

                // Specular
                if (mDefaults ||
                    pPass->getSpecular().r != 0 ||
                    pPass->getSpecular().g != 0 ||
                    pPass->getSpecular().b != 0 ||
                    pPass->getSpecular().a != 1 ||
                    pPass->getShininess() != 0)
                {
                    writeAttribute(3, "specular");
                    writeColourValue(pPass->getSpecular(), true);
                    writeValue(StringConverter::toString(pPass->getShininess()));
                }

                // Emissive
                if (mDefaults ||
                    pPass->getSelfIllumination().r != 0 ||
                    pPass->getSelfIllumination().g != 0 ||
                    pPass->getSelfIllumination().b != 0 ||
                    pPass->getSelfIllumination().a != 1)
                {
                    writeAttribute(3, "emissive");
                    writeColourValue(pPass->getSelfIllumination(), true);
                }
            }

            // scene blend factor
            if (mDefaults || 
                pPass->getSourceBlendFactor() != SBF_ONE || 
                pPass->getDestBlendFactor() != SBF_ZERO)
            {
                writeAttribute(3, "scene_blend");
                writeSceneBlendFactor(pPass->getSourceBlendFactor(), pPass->getDestBlendFactor());
            }


            //depth check
            if (mDefaults || 
                pPass->getDepthCheckEnabled() != true)
            {
                writeAttribute(3, "depth_check");
                writeValue(pPass->getDepthCheckEnabled() ? "on" : "off");
            }
			// alpha_rejection
			if (mDefaults || 
				pPass->getAlphaRejectFunction() != CMPF_ALWAYS_PASS ||
				pPass->getAlphaRejectValue() != 0)
			{
				writeAttribute(3, "alpha_rejection");
				writeCompareFunction(pPass->getAlphaRejectFunction());
				writeValue(StringConverter::toString(pPass->getAlphaRejectValue()));
			}


            //depth write
            if (mDefaults || 
                pPass->getDepthWriteEnabled() != true)
            {
                writeAttribute(3, "depth_write");
                writeValue(pPass->getDepthWriteEnabled() ? "on" : "off");
            }

            //depth function
            if (mDefaults || 
                pPass->getDepthFunction() != CMPF_LESS_EQUAL)
            {
                writeAttribute(3, "depth_func");
                writeCompareFunction(pPass->getDepthFunction());
            }

            //depth bias
            if (mDefaults || 
                pPass->getDepthBias() != 0)
            {
                writeAttribute(3, "depth_bias");
                writeValue(StringConverter::toString(pPass->getDepthBias()));
            }

            // hardware culling mode
            if (mDefaults || 
                pPass->getCullingMode() != CULL_CLOCKWISE)
            {
                CullingMode hcm = pPass->getCullingMode();
                writeAttribute(3, "cull_hardware");
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
                writeAttribute(3, "cull_software");
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
                writeAttribute(3, "shading");
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
                writeAttribute(3, "fog_override");
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
    //-----------------------------------------------------------------------
    String MaterialSerializer::convertFiltering(FilterOptions fo)
    {
        switch (fo)
        {
        case FO_NONE:
            return "none";
        case FO_POINT:
            return "point";
        case FO_LINEAR:
            return "linear";
        case FO_ANISOTROPIC:
            return "anisotropic";
        }

        return "point";
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeTextureUnit(const TextureUnitState *pTex)
    {
        LogManager::getSingleton().logMessage("MaterialSerializer : parsing texture layer.", LML_CRITICAL);
        mBuffer += "\n";
        writeAttribute(3, "texture_unit");
        beginSection(3);
        {
            //texture name
            if (pTex->getNumFrames() == 1 && pTex->getTextureName() != "" && !pTex->isCubic())
            {
                writeAttribute(4, "texture");
                writeValue(pTex->getTextureName());
                switch (pTex->getTextureType())
                {
                case TEX_TYPE_1D:
                    writeValue("1d");
                    break;
                case TEX_TYPE_2D:
                    // nothing, this is the default
                    break;
                case TEX_TYPE_3D:
                    writeValue("3d");
                    break;
                case TEX_TYPE_CUBE_MAP:
                    // nothing, deal with this as cubic_texture since it copes with all variants
                    break;
                default:
                    break;
                };
            }

            //anim. texture
            if (pTex->getNumFrames() > 1 && !pTex->isCubic())
            {
                writeAttribute(4, "anim_texture");
                for (unsigned int n = 0; n < pTex->getNumFrames(); n++)
                    writeValue(pTex->getFrameTextureName(n));
                writeValue(StringConverter::toString(pTex->getAnimationDuration()));
            }

            //cubic texture
            if (pTex->isCubic())
            {
                writeAttribute(4, "cubic_texture");
                for (unsigned int n = 0; n < pTex->getNumFrames(); n++)
                    writeValue(pTex->getFrameTextureName(n));

                //combinedUVW/separateUW
                if (pTex->getTextureType() == TEX_TYPE_CUBE_MAP)
                    writeValue("combinedUVW");
                else
                    writeValue("separateUV");
            }

            //anisotropy level
            if (mDefaults || 
                pTex->getTextureAnisotropy() != 1)
            {
                writeAttribute(4, "max_anisotropy");
                writeValue(StringConverter::toString(pTex->getTextureAnisotropy()));
            }

            //texture coordinate set
            if (mDefaults || 
                pTex->getTextureCoordSet() != 0)
            {
                writeAttribute(4, "tex_coord_set");
                writeValue(StringConverter::toString(pTex->getTextureCoordSet()));
            }

            //addressing mode
            if (mDefaults || 
                pTex->getTextureAddressingMode() != Ogre::TextureUnitState::TAM_WRAP)
            {
                writeAttribute(4, "tex_address_mode");
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
                pTex->getTextureFiltering(FT_MIN) != FO_LINEAR ||
                pTex->getTextureFiltering(FT_MAG) != FO_LINEAR ||
                pTex->getTextureFiltering(FT_MIP) != FO_POINT)
            {
                writeAttribute(4, "filtering");
                writeValue(
                    convertFiltering(pTex->getTextureFiltering(FT_MIN))
                    + " "
                    + convertFiltering(pTex->getTextureFiltering(FT_MAG))
                    + " "
                    + convertFiltering(pTex->getTextureFiltering(FT_MIP)));
            }

            // colour_op_ex
            if (mDefaults || 
                pTex->getColourBlendMode().operation != LBX_MODULATE ||
                pTex->getColourBlendMode().source1 != LBS_TEXTURE ||
                pTex->getColourBlendMode().source2 != LBS_CURRENT)
            {
                writeAttribute(4, "colour_op_ex");
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
                writeAttribute(4, "colour_op_multipass_fallback");
                writeSceneBlendFactor(pTex->getColourBlendFallbackSrc());
                writeSceneBlendFactor(pTex->getColourBlendFallbackDest());
            }

            // alpha_op_ex
            if (mDefaults || 
                pTex->getAlphaBlendMode().operation != LBX_MODULATE ||
                pTex->getAlphaBlendMode().source1 != LBS_TEXTURE ||
                pTex->getAlphaBlendMode().source2 != LBS_CURRENT)
            {
                writeAttribute(4, "alpha_op_ex");
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

			bool individualTransformElems = false;
            // rotate
            if (mDefaults ||
                pTex->getTextureRotate() != Radian(0))
            {
                writeAttribute(4, "rotate");
                writeValue(StringConverter::toString(pTex->getTextureRotate().valueDegrees()));
				individualTransformElems = true;
            }

            // scroll
            if (mDefaults ||
                pTex->getTextureUScroll() != 0 || 
                pTex->getTextureVScroll() != 0 )
            {
                writeAttribute(4, "scroll");
                writeValue(StringConverter::toString(pTex->getTextureUScroll()));
                writeValue(StringConverter::toString(pTex->getTextureVScroll()));
				individualTransformElems = true;
            }
            // scale
            if (mDefaults ||
                pTex->getTextureUScale() != 1.0 || 
                pTex->getTextureVScale() != 1.0 )
            {
                writeAttribute(4, "scale");
                writeValue(StringConverter::toString(pTex->getTextureUScale()));
                writeValue(StringConverter::toString(pTex->getTextureVScale()));
				individualTransformElems = true;
            }

			// free transform
			if (!individualTransformElems && 
				(mDefaults ||
				pTex->getTextureTransform() != Matrix4::IDENTITY))
			{
				writeAttribute(4, "transform");
				const Matrix4& xform = pTex->getTextureTransform();
				for (int row = 0; row < 4; ++row)
				{
					for (int col = 0; col < 4; ++col)
					{
						writeValue(StringConverter::toString(xform[row][col]));
					}
				}
			}

            EffectMap m_ef = pTex->getEffects();
            if (!m_ef.empty())
            {
                EffectMap::const_iterator it;
                for (it = m_ef.begin(); it != m_ef.end(); ++it)
                {
                    const TextureUnitState::TextureEffect& ef = it->second;
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
                    default:
                        break;
                    }
                }
            }
        }
        endSection(3);

    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeEnvironmentMapEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex)
    {
        writeAttribute(4, "env_map");
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
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeRotationEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex)
    {
        if (effect.arg1)
        {
            writeAttribute(4, "rotate_anim");
            writeValue(StringConverter::toString(effect.arg1));
        }
    }
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeTransformEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex)
    {
        writeAttribute(4, "wave_xform");

        switch (effect.subtype)
        {
        case TextureUnitState::TT_ROTATE:
            writeValue("rotate");
            break;
        case TextureUnitState::TT_SCALE_U:
            writeValue("scale_x");
            break;
        case TextureUnitState::TT_SCALE_V:
            writeValue("scale_y");
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
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeScrollEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex)
    {
        if (effect.arg1 || effect.arg2)
        {
            writeAttribute(4, "scroll_anim");
            writeValue(StringConverter::toString(effect.arg1));
            writeValue(StringConverter::toString(effect.arg2));
        }
    }
    //-----------------------------------------------------------------------
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
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeSceneBlendFactor(const SceneBlendFactor sbf_src, const SceneBlendFactor sbf_dst)
    {
        if (sbf_src == SBF_ONE && sbf_dst == SBF_ONE )
            writeValue("add");
        else if (sbf_src == SBF_DEST_COLOUR && sbf_dst == SBF_ZERO)
            writeValue("modulate");
        else if (sbf_src == SBF_SOURCE_COLOUR && sbf_dst == SBF_ONE_MINUS_SOURCE_COLOUR)
            writeValue("colour_blend");
        else if (sbf_src == SBF_SOURCE_ALPHA && sbf_dst == SBF_ONE_MINUS_SOURCE_ALPHA)
            writeValue("alpha_blend");
        else
        {
            writeSceneBlendFactor(sbf_src);
            writeSceneBlendFactor(sbf_dst);
        }
    }
    //-----------------------------------------------------------------------
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
    //-----------------------------------------------------------------------
    void MaterialSerializer::writeColourValue(const ColourValue &colour, bool writeAlpha)
    {
        writeValue(StringConverter::toString(colour.r));
        writeValue(StringConverter::toString(colour.g));
        writeValue(StringConverter::toString(colour.b));
        if (writeAlpha)
            writeValue(StringConverter::toString(colour.a));
    }
    //-----------------------------------------------------------------------
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
    //-----------------------------------------------------------------------
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

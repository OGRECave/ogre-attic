/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreMaterialScriptCompiler.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreMaterial.h"
#include "OgreBlendMode.h"
#include "OgreTextureUnitState.h"
#include "OgreGpuProgram.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreMaterialManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreExternalTextureSourceManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    String MaterialScriptCompiler::materialScript_BNF =
        "<Script> ::= {<Script_Properties>}; \n"

        "<Script_Properties> ::= <Material_Def> | <Vertex_Program_Def> | <Fragment_Program_Def>; \n"

        "<Material_Def> ::= 'material' <Label> [<Material_Clone>] -'{' {<Material_Properties>} -'}'; \n"

        "<Material_Properties> ::= <Technique_Def> | <Set_Texture_Alias_Def> | "
        "                          <Lod_Distances_Def> | <Receive_Shadows_Def> | "
        "                          <transparency_casts_shadows_def>; \n"

        "    <Material_Clone> ::= ':' <Label>; \n"
        "    <Set_Texture_Alias_Def> ::= 'set_texture_alias' <Label> <Label>; \n"
        "    <Lod_Distances_Def> ::= 'lod_distances' <#distance> {<#distance>}; \n"
        "    <Receive_Shadows_Def> ::= 'receive_shadows' <On_Off>; \n"
        "    <transparency_casts_shadows_def> ::= 'transparency_casts_shadows' <On_Off>; \n"

        // Technique section rules
        "<Technique_Def> ::= 'technique' [<Label>] -'{' {<Technique_Properties>} -'}'; \n"
        "    <Technique_Properties> ::= <Pass_Def> | <Lod_Index_Def>; \n"
        "    <Lod_Index_Def> ::= 'lod_index' <#value>; \n"

        // Pass section rules
        "    <Pass_Def> ::= 'pass' [<Label>] -'{' {<Pass_Properties>} -'}'; \n"
        "        <Pass_Properties> ::= <Ambient_Def> | <Diffuse_Def> | <Specular_Def> | <Emissive_Def>; \n"
        "                              <Scene_Blend_Def> | <Depth_Check_Def> | <Depth_Write_Def> | "
        "                              <Depth_Func_Def> | <Colour_Write_Def> | <Cull_Hardware_Def> | "
        "                              <Cull_Software_Def> | <Lighting_Def> | <Shading_Def> | "

        "        <Ambient_Def> ::= 'ambient' <Colour_Param_Def> | <Vertexcolour_Def>; \n"
        "        <Diffuse_Def> ::= 'diffuse' <Colour_Param_Def> | <Vertexcolour_Def>; \n"
        "        <Specular_Def> ::= 'specular' <Specular_Params> <#shininess>; \n"
        "            <Specular_Params> ::= <Colour_params> | <Vertexcolour_Def>; \n"
        "        <Emissive_Def> ::= 'emissive' <Colour_params> | <Vertexcolour_Def>; \n"

        "        <Vertexcolour> ::= 'vertexcolour'; \n"

        "        <Scene_Blend_Def> ::= 'scene_blend' <Simple_Blend> | <User_Blend>; \n"
        "          <Simple_Blend> ::= 'add' | 'modulate' | 'colour_blend' | 'alpha_blend'; \n"
        "          <User_Blend> ::= <Blend_Factor> <Blend_Factor>; \n"
        "          <Blend_Factor> ::= 'one' | 'zero' | 'dest_colour' | 'src_colour' | \n"
        "                             'one_minus_dest_colour' | 'one_minus_src_colour' | \n"
        "                             'dest_alpha' | 'src_alpha' | 'one_minus_dest_alpha' | \n"
        "                             'one_minus_src_alpha'; \n"

        "        <Depth_Check_Def> ::= 'depth_check' <On_Off>; \n"
        "        <Depth_Write_Def> ::= 'depth_write' <On_Off>; \n"
        "        <Depth_Func_Def> ::= 'depth_func' <Compare_Func_Def>; \n"
        "        <Depth_Bias_Def> ::= 'depth_bias' <#value>; \n"
        "        <Alpha_Rejection_Def> ::= 'alpha_rejection' <Compare_Func_Def> <value>; \n"
        "          <Compare_Func_Def> ::= 'always_fail' | 'always_pass' | 'less_equal' | 'less' | \n"
        "                                 'equal' | 'not_equal' | 'greater_equal' | 'greater'; \n"
        "        <Colour_Write_Def> ::= 'colour_write' <On_Off>; \n"
        "        <Cull_Hardware_Def> ::= 'cull_hardware' 'clockwise' | 'anticlockwise' | 'none'; \n"
        "        <Cull_Software_Def> ::= 'cull_software' 'back' | 'front' | 'none'; \n"
        "        <Lighting_Def> ::= 'lighting' <On_Off>; \n"
        "        <Shading_Def> ::= 'shading' 'flat' | 'gouraud' | 'phong'; \n"
        "        <Fog_Override_Def> ::= 'fog_override' <fog_false> | <fog_true>; \n"
        "           <fog_false> ::= 'false'; \n"
        "           <fog_true> ::= 'true' [<Fog_parameters>]; \n"
        "               <Fog_parameters> ::= <fog_type> <fog_colour> <#fog_density> <#start> <#end>; \n"
        "                   <fog_type> ::= 'none' | 'linear' | 'exp' | 'exp2'; \n"
        "                   <fog_colour> ::= <#red> <#green> <#blue>; \n"
        "        <Max_Lights_Def> ::= 'max_lights' <#number>; \n"
        "        <Iteration_Def> ::= 'iteration' 'once' | <Iteration_Once_Params> | <Iteration_Counted>; \n"
        "           <Iteration_Once_Params> ::= 'once_per_light' [<light_type>]; \n"
        "           <Iteration_Counted> ::= <#number> [<Per_Light>]; \n"
        "               <Per_Light> ::= 'per_light' <light_type>; \n"
        "           <light_type> ::= 'point' | 'directional' | 'spot'; \n"
        // Texture Unit section rules
        "        <Texture_Unit_Def> ::= 'texture_unit' [<Label>] -'{' {<TUS_Properties>} -'}'; \n"
        " "

        // common rules
        "<On_Off> ::= 'on' | 'off'; \n"
        "<Colour_params> ::= <#red> <#green> <#blue> [<#alpha>]; \n"


        "<Label> ::= <Quoted_Label> | <Unquoted_Label>; \n"
        "<Quoted_Label> ::= '\"' <Character> {<Alphanumeric_Space>} '\"'; \n"
        "<Alphanumeric_Space> ::= <Alphanumeric> | ' '; \n"
        "<Alphanumeric> ::= <Character> | <Number>; \n"
        "<Character> ::= (abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$#%!_*&\\/); \n"
        "<Number> ::= (0123456789); \n"
        
        ;

    //-----------------------------------------------------------------------
    MaterialScriptCompiler::MaterialScriptCompiler(void)
    {

    }
    //-----------------------------------------------------------------------
    MaterialScriptCompiler::~MaterialScriptCompiler(void)
    {

    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::initTokenActions(void)
    {
        addLexemeTokenAction("material", ID_MATERIAL, &MaterialScriptCompiler::parseMaterial);
        addLexemeTokenAction("technique", ID_TECHNIQUE, &MaterialScriptCompiler::parseTechnique);
        addLexemeTokenAction("transparency_casts_shadows", ID_TRANSPARENCY_CASTS_SHADOWS, &MaterialScriptCompiler::parseTransparencyCastsShadows);
        addLexemeTokenAction("receive_shadows", ID_RECEIVE_SHADOWS, &MaterialScriptCompiler::parseReceiveShadows);
        addLexemeTokenAction("ambient", ID_AMBIENT, &MaterialScriptCompiler::parseAmbient);
        addLexemeTokenAction("diffuse", ID_DIFFUSE, &MaterialScriptCompiler::parseDiffuse);
        addLexemeTokenAction("specular", ID_SPECULAR, &MaterialScriptCompiler::parseSpecular);
        addLexemeTokenAction("emissive", ID_EMISSIVE, &MaterialScriptCompiler::parseEmissive);

    }

    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::addLexemeTokenAction(const String& lexeme, const size_t token, const MSC_Action action)
    {
        addLexemeToken(lexeme, token, action != 0);
        mTokenActionMap[token] = action;
    }

    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::executeTokenAction(const size_t tokenID)
    {
        TokenActionIterator action = mTokenActionMap.find(tokenID);
        if (action != mTokenActionMap.end())
            (this->*action->second)();
    }

    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::logParseError(const String& error)
    {
        // log material name only if filename not specified
        if (mScriptContext.filename.empty() && !mScriptContext.material.isNull())
        {
            LogManager::getSingleton().logMessage(
                "Error in material " + mScriptContext.material->getName() + 
                " : " + error);
        }
        else
        {
            if (!mScriptContext.material.isNull())
            {
                LogManager::getSingleton().logMessage(
                    "Error in material " + mScriptContext.material->getName() +
                    " at line " + StringConverter::toString(mScriptContext.lineNo) + 
                    " of " + mScriptContext.filename + ": " + error);
            }
            else
            {
                LogManager::getSingleton().logMessage(
                    "Error at line " + StringConverter::toString(mScriptContext.lineNo) + 
                    " of " + mScriptContext.filename + ": " + error);
            }
        }
    }
	//-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseMaterial(void)
    {

    }
	//-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseTechnique(void)
    {

    }
	//-----------------------------------------------------------------------
    void MaterialScriptCompiler::parsePass(void)
    {

    }
	//-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseTransparencyCastsShadows(void)
	{
        //StringUtil::toLowerCase(params);
        switch (getNextToken().mID)
        {
        case ID_ON:
			mScriptContext.material->setTransparencyCastsShadows(true);
            break;
        case ID_OFF:
			mScriptContext.material->setTransparencyCastsShadows(false);
            break;
        default:
            replaceToken();
			logParseError(
			"Bad transparency_casts_shadows attribute, valid parameters are 'on' or 'off'.");
        }

	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseReceiveShadows(void)
    {
        //StringUtil::toLowerCase(params);
        switch (getNextToken().mID)
        {
        case ID_ON:
            mScriptContext.material->setReceiveShadows(true);
            break;
        case ID_OFF:
            mScriptContext.material->setReceiveShadows(false);
            break;
        default:
            replaceToken();
            logParseError("Bad receive_shadows attribute, valid parameters are 'on' or 'off'.");
        }
    }

    //-----------------------------------------------------------------------
    ColourValue MaterialScriptCompiler::_parseColourValue(void)
    {
        return ColourValue(
            getNextTokenValue() ,
            getNextTokenValue() ,
            getNextTokenValue() ,
            (getTokenQueCount()==1) ? getNextTokenValue() : 1.0f ) ;
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseAmbient(void)
    {
        // Must be 1, 3 or 4 parameters
        const size_t paramCount = getTokenQueCount();
        if (paramCount == 1) {
            if(getNextToken().mID == ID_VERTEXCOLOUR)
            {
                mScriptContext.pass->setVertexColourTracking(mScriptContext.pass->getVertexColourTracking() | TVC_AMBIENT);
            }
            else
            {
                replaceToken();
                logParseError("Bad ambient attribute, single parameter flag must be 'vertexcolour'");
            }
        } 
        else if (paramCount == 3 || paramCount == 4)
        {
            mScriptContext.pass->setAmbient( _parseColourValue() );
            mScriptContext.pass->setVertexColourTracking(mScriptContext.pass->getVertexColourTracking() & ~TVC_AMBIENT);
        }
        else 
        {
            logParseError("Bad ambient attribute, wrong number of parameters (expected 1, 3 or 4)");
        }
    }
   //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseDiffuse(void)
    {
        // Must be 1, 3 or 4 parameters 
        const size_t paramCount = getTokenQueCount();
        if (paramCount == 1) {
            if(getNextToken().mID == ID_VERTEXCOLOUR)
            {
               mScriptContext.pass->setVertexColourTracking(mScriptContext.pass->getVertexColourTracking() | TVC_DIFFUSE);
            }
            else
            {
                replaceToken();
                logParseError("Bad diffuse attribute, single parameter flag must be 'vertexcolour'");
            }
        }
        else if (paramCount == 3 || paramCount == 4)
        {
            mScriptContext.pass->setDiffuse( _parseColourValue() );
            mScriptContext.pass->setVertexColourTracking(mScriptContext.pass->getVertexColourTracking() & ~TVC_DIFFUSE);
        }
        else
        {
            logParseError("Bad diffuse attribute, wrong number of parameters (expected 1, 3 or 4)");
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseSpecular(void)
    {
        // Must be 2, 4 or 5 parameters 
        const size_t paramCount = getTokenQueCount();
        if(paramCount == 2) 
        {   
            if(getNextToken().mID == ID_VERTEXCOLOUR)
            {
                mScriptContext.pass->setVertexColourTracking(mScriptContext.pass->getVertexColourTracking() | TVC_SPECULAR);
                mScriptContext.pass->setShininess(getNextTokenValue());
            }
            else
            {
                replaceToken();
                logParseError("Bad specular attribute, double parameter statement must be 'vertexcolour <shininess>'");
            }
        } 
        else if(paramCount == 4 || paramCount == 5) 
        {
            mScriptContext.pass->setSpecular(
                getNextTokenValue(), 
                getNextTokenValue(), 
                getNextTokenValue(), 
                paramCount == 5 ? getNextTokenValue() : 1.0f);
            mScriptContext.pass->setVertexColourTracking(mScriptContext.pass->getVertexColourTracking() & ~TVC_SPECULAR);
            mScriptContext.pass->setShininess( getNextTokenValue() );
        }
        else 
        {
            logParseError("Bad specular attribute, wrong number of parameters (expected 2, 4 or 5)");
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseEmissive(void)
    {
        // Must be 1, 3 or 4 parameters 
        const size_t paramCount = getTokenQueCount();
        if (paramCount == 1) {
            if(getNextToken().mID == ID_VERTEXCOLOUR)
            {
               mScriptContext.pass->setVertexColourTracking(mScriptContext.pass->getVertexColourTracking() | TVC_EMISSIVE);
            }
            else
            {
                replaceToken();
                logParseError("Bad emissive attribute, single parameter flag must be 'vertexcolour'");
            }
        }
        else if (paramCount == 3 || paramCount == 4)
        {
            mScriptContext.pass->setSelfIllumination( _parseColourValue() );
            mScriptContext.pass->setVertexColourTracking(mScriptContext.pass->getVertexColourTracking() & ~TVC_EMISSIVE);
        }
        else
        {
            logParseError("Bad emissive attribute, wrong number of parameters (expected 1, 3 or 4)");
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseTextureCustomParameter(void)
    {
		// This params object does not have the command stripped
		// Split only up to first delimiter, program deals with the rest

		if (getTokenQueCount() != 2)
		{
            logParseError("Invalid texture parameter entry; "
				"there must be a parameter name and at least one value.");
		}
		
		else if( ExternalTextureSourceManager::getSingleton().getCurrentPlugIn() != 0 )
        {
			////First is command, next could be a string with one or more values
            const String& param1 = getNextTokenLabel();
            const String& param2 = getNextTokenLabel();
			ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->setParameter( param1, param2 );
        }
	}

}

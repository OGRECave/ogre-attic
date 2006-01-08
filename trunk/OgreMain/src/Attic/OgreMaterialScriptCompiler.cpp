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
        "<Script> ::= {<Script_Properties>} \n"

        "<Script_Properties> ::= <Material_Def> | <Vertex_Program_Def> | <Fragment_Program_Def> \n"

        "<Material_Def> ::= 'material' <Label> [<Material_Clone>] '{' {<Material_Properties>} '}' \n"

        "<Material_Properties> ::= <Technique_Def> | <Set_Texture_Alias_Def> | "
        "                          <Lod_Distances_Def> | <Receive_Shadows_Def> | "
        "                          <transparency_casts_shadows_def> \n"

        "    <Material_Clone> ::= ':' <Label> \n"
        "    <Set_Texture_Alias_Def> ::= 'set_texture_alias' <Label> <Label> \n"
        "    <Lod_Distances_Def> ::= 'lod_distances' <#distance> {<#distance>} \n"
        "    <Receive_Shadows_Def> ::= 'receive_shadows' <On_Off> \n"
        "    <transparency_casts_shadows_def> ::= 'transparency_casts_shadows' <On_Off> \n"

        // Technique section rules
        "<Technique_Def> ::= 'technique' [<Label>] '{' {<Technique_Properties>} '}' \n"
        "    <Technique_Properties> ::= <Pass_Def> | <Lod_Index_Def> \n"
        "    <Lod_Index_Def> ::= 'lod_index' <#value> \n"

        // Pass section rules
        "    <Pass_Def> ::= 'pass' [<Label>] '{' {<Pass_Properties>} '}' \n"
        "        <Pass_Properties> ::= <Ambient_Def> | <Diffuse_Def> | <Specular_Def> | <Emissive_Def> \n"
        "                              <Scene_Blend_Def> | <Depth_Check_Def> | <Depth_Write_Def> | "
        "                              <Depth_Func_Def> | <Colour_Write_Def> | <Cull_Hardware_Def> | "
        "                              <Cull_Software_Def> | <Lighting_Def> | <Shading_Def> | "
		"                              <Point_Size_Def> | <Point_Sprites_Def> | <Point_Size_Attenuation_Def> | "
		"                              <Point_Size_Min_Def> | <Point_Size_Max_Def> \n"

        "        <Ambient_Def> ::= 'ambient' <Colour_Param_Def> | <Vertexcolour_Def> \n"
        "        <Diffuse_Def> ::= 'diffuse' <Colour_Param_Def> | <Vertexcolour_Def> \n"
        "        <Specular_Def> ::= 'specular' <Specular_Params> <#shininess> \n"
        "            <Specular_Params> ::= <Colour_params> | <Vertexcolour_Def> \n"
        "        <Emissive_Def> ::= 'emissive' <Colour_params> | <Vertexcolour_Def> \n"

        "        <Vertexcolour> ::= 'vertexcolour' \n"

        "        <Scene_Blend_Def> ::= 'scene_blend' <Simple_Blend> | <User_Blend> \n"
        "          <Simple_Blend> ::= 'add' | 'modulate' | 'colour_blend' | 'alpha_blend' \n"
        "          <User_Blend> ::= <Blend_Factor> <Blend_Factor> \n"
        "          <Blend_Factor> ::= 'one' | 'zero' | 'dest_colour' | 'src_colour' | \n"
        "                             'one_minus_dest_colour' | 'one_minus_src_colour' | \n"
        "                             'dest_alpha' | 'src_alpha' | 'one_minus_dest_alpha' | \n"
        "                             'one_minus_src_alpha' \n"

        "        <Depth_Check_Def> ::= 'depth_check' <On_Off> \n"
        "        <Depth_Write_Def> ::= 'depth_write' <On_Off> \n"
        "        <Depth_Func_Def> ::= 'depth_func' <Compare_Func_Def> \n"
        "        <Depth_Bias_Def> ::= 'depth_bias' <#value> \n"
        "        <Alpha_Rejection_Def> ::= 'alpha_rejection' <Compare_Func_Def> <#value> \n"
        "        <Compare_Func_Def> ::= 'always_fail' | 'always_pass' | 'less_equal' | 'less' | \n"
        "                               'equal' | 'not_equal' | 'greater_equal' | 'greater' \n"
        "        <Colour_Write_Def> ::= 'colour_write' <On_Off> \n"
        "        <Cull_Hardware_Def> ::= 'cull_hardware' 'clockwise' | 'anticlockwise' | 'none' \n"
        "        <Cull_Software_Def> ::= 'cull_software' 'back' | 'front' | 'none' \n"
        "        <Lighting_Def> ::= 'lighting' <On_Off> \n"
        "        <Shading_Def> ::= 'shading' 'flat' | 'gouraud' | 'phong' \n"
		"        <Point_Size_Def> ::= 'point_size' <#size> \n"
		"        <Point_Sprites_Def> ::= 'point_sprites' <On_Off> \n"
		"        <Point_Size_Min_Def> ::= 'point_size_min' <#size> \n"
		"        <Point_Size_Max_Def> ::= 'point_size_max' <#size> \n"
		"        <Point_Size_Attenuation_Def> ::= 'point_size_attenuation' <On_Off> [<Point_Size_Att_Params>] \n"
		"            <Point_Size_Att_Params> ::= <#constant> <#linear> <#quadric> \n"
        "        <Fog_Override_Def> ::= 'fog_override' <fog_false> | <fog_true> \n"
        "           <fog_false> ::= 'false' \n"
        "           <fog_true> ::= 'true' [<Fog_parameters>] \n"
        "               <Fog_parameters> ::= <fog_type> <fog_colour> <#fog_density> <#start> <#end> \n"
        "                   <fog_type> ::= 'none' | 'linear' | 'exp' | 'exp2' \n"
        "                   <fog_colour> ::= <#red> <#green> <#blue> \n"
        "        <Max_Lights_Def> ::= 'max_lights' <#number> \n"
        "        <Iteration_Def> ::= 'iteration' 'once' | <Iteration_Once_Params> | <Iteration_Counted> \n"
        "           <Iteration_Once_Params> ::= 'once_per_light' [<light_type>] \n"
        "           <Iteration_Counted> ::= <#number> [<Per_Light>] \n"
        "               <Per_Light> ::= 'per_light' <light_type> \n"
        "           <light_type> ::= 'point' | 'directional' | 'spot' \n"
        // Texture Unit section rules
        "        <Texture_Unit_Def> ::= 'texture_unit' [<Label>] '{' {<TUS_Properties>} '}' \n"
        " "

        // common rules
        "<On_Off> ::= 'on' | 'off' \n"
        "<Colour_params> ::= <#red> <#green> <#blue> [<#alpha>] \n"


        "<Label> ::= <Quoted_Label> | <Unquoted_Label> \n"
        "<Quoted_Label> ::= '\"' <Character> {<Alphanumeric_Space>} '\"' \n"
        "<Alphanumeric_Space> ::= <Alphanumeric> | ' ' \n"
        "<Alphanumeric> ::= <Character> | <Number> \n"
        "<Character> ::= (abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$#%!_*&\\/) \n"
        "<Number> ::= (0123456789); \n"
        
        ;

    //-----------------------------------------------------------------------
    MaterialScriptCompiler::MaterialScriptCompiler(void)
    {
        initTokenActions();
        setClientBNFGrammer(materialScript_BNF);
    }
    //-----------------------------------------------------------------------
    MaterialScriptCompiler::~MaterialScriptCompiler(void)
    {

    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::initTokenActions(void)
    {
        addLexemeTokenAction("{", ID_OPENBRACE, &MaterialScriptCompiler::parseOpenBrace);
        addLexemeTokenAction("}", ID_CLOSEBRACE, &MaterialScriptCompiler::parseCloseBrace);
        addLexemeTokenAction("vertex_program", ID_VERTEX_PROGRAM, &MaterialScriptCompiler::parseVertexProgram);
        addLexemeTokenAction("fragment_program", ID_FRAGMENT_PROGRAM, &MaterialScriptCompiler::parseFragmentProgram);
        addLexemeTokenAction("material", ID_MATERIAL, &MaterialScriptCompiler::parseMaterial);
            addLexemeTokenAction(":", ID_CLONE);

        // Technique section
        addLexemeTokenAction("technique", ID_TECHNIQUE, &MaterialScriptCompiler::parseTechnique);
            addLexemeTokenAction("transparency_casts_shadows", ID_TRANSPARENCY_CASTS_SHADOWS, &MaterialScriptCompiler::parseTransparencyCastsShadows);
            addLexemeTokenAction("receive_shadows", ID_RECEIVE_SHADOWS, &MaterialScriptCompiler::parseReceiveShadows);

        // Pass section
        addLexemeTokenAction("pass", ID_PASS, &MaterialScriptCompiler::parsePass);
            addLexemeTokenAction("ambient", ID_AMBIENT, &MaterialScriptCompiler::parseAmbient);
            addLexemeTokenAction("diffuse", ID_DIFFUSE, &MaterialScriptCompiler::parseDiffuse);
            addLexemeTokenAction("specular", ID_SPECULAR, &MaterialScriptCompiler::parseSpecular);
            addLexemeTokenAction("emissive", ID_EMISSIVE, &MaterialScriptCompiler::parseEmissive);
            addLexemeTokenAction("depth_check", ID_DEPTH_CHECK, &MaterialScriptCompiler::parseDepthCheck);
            addLexemeTokenAction("depth_write", ID_DEPTH_WRITE, &MaterialScriptCompiler::parseDepthWrite);
            addLexemeTokenAction("depth_func", ID_DEPTH_FUNC, &MaterialScriptCompiler::parseDepthFunc);
                addLexemeTokenAction("always_fail", ID_ALWAYS_FAIL);
                addLexemeTokenAction("always_pass", ID_ALWAYS_PASS);
                addLexemeTokenAction("less_equal", ID_LESS_EQUAL);
                addLexemeTokenAction("less", ID_LESS);
                addLexemeTokenAction("equal", ID_EQUAL);
                addLexemeTokenAction("not_equal", ID_NOT_EQUAL);
                addLexemeTokenAction("greater_equal", ID_GREATER_EQUAL);
                addLexemeTokenAction("greater", ID_GREATER);
            addLexemeTokenAction("colour_write", ID_COLOUR_WRITE, &MaterialScriptCompiler::parseColourWrite);
            addLexemeTokenAction("cull_hardware", ID_CULL_HARDWARE, &MaterialScriptCompiler::parseCullHardware);
                addLexemeTokenAction("clockwise", ID_CLOCKWISE);
                addLexemeTokenAction("anticlockwise", ID_ANTICLOCKWISE);
                addLexemeTokenAction("none", ID_CULL_NONE);
            addLexemeTokenAction("cull_software", ID_CULL_SOFTWARE, &MaterialScriptCompiler::parseCullSoftware);
                addLexemeTokenAction("back", ID_CULL_BACK);
                addLexemeTokenAction("front", ID_CULL_FRONT);
            addLexemeTokenAction("lighting", ID_LIGHTING, &MaterialScriptCompiler::parseLighting);
            addLexemeTokenAction("max_lights", ID_MAX_LIGHTS, &MaterialScriptCompiler::parseMaxLights);
            addLexemeTokenAction("shading", ID_SHADING, &MaterialScriptCompiler::parseShading);
                addLexemeTokenAction("flat", ID_FLAT);
                addLexemeTokenAction("gouraud", ID_GOURAUD);
                addLexemeTokenAction("phong", ID_PHONG);
            addLexemeTokenAction("point_size", ID_POINT_SIZE, &MaterialScriptCompiler::parsePointSize);
            addLexemeTokenAction("point_sprites", ID_POINT_SPRITES, &MaterialScriptCompiler::parsePointSprites);
            addLexemeTokenAction("point_size_attenuation", ID_POINT_SIZE_ATTENUATION, &MaterialScriptCompiler::parsePointSizeAttenuation);
            addLexemeTokenAction("point_size_min", ID_POINT_SIZE_MIN, &MaterialScriptCompiler::parsePointSizeMin);
            addLexemeTokenAction("point_size_max", ID_POINT_SIZE_MAX, &MaterialScriptCompiler::parsePointSizeMax);

        // Texture Unit section
        addLexemeTokenAction("texture_unit", ID_TEXTURE_UNIT, &MaterialScriptCompiler::parseTextureUnit);

        // common section
        addLexemeTokenAction("on", ID_ON);
        addLexemeTokenAction("off", ID_OFF);

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

        if (action == mTokenActionMap.end())
        {
            // BAD command. BAD!
            logParseError("Unrecognised command action");
            return;
        }
        else
        {
            try
            {
                (this->*action->second)();
            }
            catch (Exception& ogreException)
            {
                // an unknown token found or BNF Grammer rule was not successful
                // in finding a valid terminal token to complete the rule expression.
                logParseError(ogreException.getDescription());
            }
        }
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
    void MaterialScriptCompiler::parseOpenBrace(void)
    {

    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseCloseBrace(void)
    {
        switch(mScriptContext.section)
        {
        case MSS_NONE:
            logParseError("Unexpected terminating brace.");
            break;
        case MSS_MATERIAL:
            // End of material
            // if texture aliases were found, pass them to the material
            // to update texture names used in Texture unit states
            if (!mScriptContext.textureAliases.empty())
            {
                // request material to update all texture names in TUS's
                // that use texture aliases in the list
                mScriptContext.material->applyTextureAliases(mScriptContext.textureAliases);
            }

            mScriptContext.section = MSS_NONE;
            mScriptContext.material.setNull();
			//Reset all levels for next material
			mScriptContext.passLev = -1;
			mScriptContext.stateLev= -1;
			mScriptContext.techLev = -1;
            mScriptContext.textureAliases.clear();
            break;
        case MSS_TECHNIQUE:
            // End of technique
            mScriptContext.section = MSS_MATERIAL;
            mScriptContext.technique = NULL;
			mScriptContext.passLev = -1;	//Reset pass level (yes, the pass level)
            break;
        case MSS_PASS:
            // End of pass
            mScriptContext.section = MSS_TECHNIQUE;
            mScriptContext.pass = NULL;
			mScriptContext.stateLev = -1;	//Reset state level (yes, the state level)
            break;
        case MSS_TEXTUREUNIT:
            // End of texture unit
            mScriptContext.section = MSS_PASS;
            mScriptContext.textureUnit = NULL;
            break;
		case MSS_TEXTURESOURCE:
			//End texture source section
			//Finish creating texture here
			
			if(	ExternalTextureSourceManager::getSingleton().getCurrentPlugIn() != 0)
            {
                const String sMaterialName = mScriptContext.material->getName();
				ExternalTextureSourceManager::getSingleton().getCurrentPlugIn()->
				createDefinedTexture( sMaterialName, mScriptContext.groupName );
            }
			//Revert back to texture unit
			mScriptContext.section = MSS_TEXTUREUNIT;
			break;
        case MSS_PROGRAM_REF:
            // End of program
            mScriptContext.section = MSS_PASS;
            mScriptContext.program.setNull();
            break;
        case MSS_PROGRAM:
			// Program definitions are slightly different, they are deferred
			// until all the information required is known
            // End of program
			finishProgramDefinition();
            mScriptContext.section = MSS_NONE;
            delete mScriptContext.programDef;
            mScriptContext.defaultParamLines.clear();
            mScriptContext.programDef = NULL;
            break;
        case MSS_DEFAULT_PARAMETERS:
            // End of default parameters
            mScriptContext.section = MSS_PROGRAM;
            break;
        };
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseVertexProgram(void)
    {
        // update section
        mScriptContext.section = MSS_PROGRAM;

		// Create new program definition-in-progress
		mScriptContext.programDef = new MaterialScriptProgramDefinition();
		mScriptContext.programDef->progType = GPT_VERTEX_PROGRAM;
        mScriptContext.programDef->supportsSkeletalAnimation = false;
		mScriptContext.programDef->supportsMorphAnimation = false;
		mScriptContext.programDef->supportsPoseAnimation = 0;

		// Get name and language code
        const size_t paramCount = getTokenQueCount();
		if (paramCount != 2)
		{
            logParseError("Invalid vertex_program entry - expected "
				"2 parameters.");
            return;
		}
		// Name, preserve case
		mScriptContext.programDef->name = getNextTokenLabel();
		// language code, make lower case
		mScriptContext.programDef->language = getNextTokenLabel();
		StringUtil::toLowerCase(mScriptContext.programDef->language);
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseFragmentProgram(void)
    {
        // update section
        mScriptContext.section = MSS_PROGRAM;

		// Create new program definition-in-progress
		mScriptContext.programDef = new MaterialScriptProgramDefinition();
		mScriptContext.programDef->progType = GPT_FRAGMENT_PROGRAM;
		mScriptContext.programDef->supportsSkeletalAnimation = false;
		mScriptContext.programDef->supportsMorphAnimation = false;
		mScriptContext.programDef->supportsPoseAnimation = 0;

		// Get name and language code
        const size_t paramCount = getTokenQueCount();
		if (paramCount != 2)
		{
            logParseError("Invalid fragment_program entry - expected "
				"2 parameters.");
            return;
		}
		// Name, preserve case
		mScriptContext.programDef->name = getNextTokenLabel();
		// language code, make lower case
		mScriptContext.programDef->language = getNextTokenLabel();
		StringUtil::toLowerCase(mScriptContext.programDef->language);
	}
	//-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseMaterial(void)
    {
        // check params for reference to parent material to copy from
        // syntax: material name : parentMaterialName
        MaterialPtr basematerial;

        const String materialName = getNextTokenLabel();
        // Create a brand new material
        const size_t paramCount = getTokenQueCount();
        if (paramCount == 2)
        {
            getNextToken();
            // if a second parameter exists then assume its the name of the base material
            // that this new material should clone from
            const String parentName = getNextTokenLabel();
            // make sure base material exists
            basematerial = MaterialManager::getSingleton().getByName(parentName);
            // if it doesn't exist then report error in log and just create a new material
            if (basematerial.isNull())
            {
                logParseError("parent material: " + parentName + " not found for new material:"
                    + materialName);
            }
        }

        mScriptContext.material = 
			MaterialManager::getSingleton().create(materialName, mScriptContext.groupName);

        if (!basematerial.isNull())
        {
            // copy parent material details to new material
            basematerial->copyDetailsTo(mScriptContext.material);
        }
        else
        {
            // Remove pre-created technique from defaults
            mScriptContext.material->removeAllTechniques();
        }

		mScriptContext.material->_notifyOrigin(mScriptContext.filename);

        // update section
        mScriptContext.section = MSS_MATERIAL;

    }
	//-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseTechnique(void)
    {
        String techniqueName;
        const size_t paramCount = getTokenQueCount();
        // if params is not empty then see if the technique name already exists
        if ((paramCount > 0) && (mScriptContext.material->getNumTechniques() > 0))
        {
            // find the technique with name = params
            techniqueName = getNextTokenLabel();
            Technique * foundTechnique = mScriptContext.material->getTechnique(techniqueName);
            if (foundTechnique)
            {
                // figure out technique index by iterating through technique container
                // would be nice if each technique remembered its index
                int count = 0;
                Material::TechniqueIterator i = mScriptContext.material->getTechniqueIterator();
                while(i.hasMoreElements())
                {
                    if (foundTechnique == i.peekNext())
                        break;
                    i.moveNext();
                    ++count;
                }

                mScriptContext.techLev = count;
            }
            else
            {
                // name was not found so a new technique is needed
                // position technique level to the end index
                // a new technique will be created later on
                mScriptContext.techLev = mScriptContext.material->getNumTechniques();
            }

        }
        else 
        {
            // no name was given in the script so a new technique will be created
		    // Increase technique level depth
		    ++mScriptContext.techLev;
        }

        // Create a new technique if it doesn't already exist
        if (mScriptContext.material->getNumTechniques() > mScriptContext.techLev)
        {
            mScriptContext.technique = mScriptContext.material->getTechnique(mScriptContext.techLev);
        }
        else
        {
            mScriptContext.technique = mScriptContext.material->createTechnique();
            if (!techniqueName.empty())
                mScriptContext.technique->setName(techniqueName);
        }

        // update section
        mScriptContext.section = MSS_TECHNIQUE;

    }
	//-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseTransparencyCastsShadows(void)
	{
        switch (getNextToken().tokenID)
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
        switch (getNextToken().tokenID)
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
    void MaterialScriptCompiler::parsePass(void)
    {
        String passName;
        const size_t paramCount = getTokenQueCount();
        // if params is not empty then see if the pass name already exists
        if ((paramCount > 0) && (mScriptContext.technique->getNumPasses() > 0))
        {
            passName = getNextTokenLabel();
            // find the pass with name = params
            Pass * foundPass = mScriptContext.technique->getPass(passName);
            if (foundPass)
            {
                mScriptContext.passLev = foundPass->getIndex();
            }
            else
            {
                // name was not found so a new pass is needed
                // position pass level to the end index
                // a new pass will be created later on
                mScriptContext.passLev = mScriptContext.technique->getNumPasses();
            }

        }
        else
        {
		    //Increase pass level depth
		    ++mScriptContext.passLev;
        }

        if (mScriptContext.technique->getNumPasses() > mScriptContext.passLev)
        {
            mScriptContext.pass = mScriptContext.technique->getPass(mScriptContext.passLev);
        }
        else
        {
            // Create a new pass
            mScriptContext.pass = mScriptContext.technique->createPass();
            if (!passName.empty())
                mScriptContext.pass->setName(passName);
        }

        // update section
        mScriptContext.section = MSS_PASS;
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
            if(getNextToken().tokenID == ID_VERTEXCOLOUR)
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
            if(getNextToken().tokenID == ID_VERTEXCOLOUR)
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
            if(getNextToken().tokenID == ID_VERTEXCOLOUR)
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
            if(getNextToken().tokenID == ID_VERTEXCOLOUR)
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
    void MaterialScriptCompiler::parseDepthCheck(void)
    {
        switch (getNextToken().tokenID)
        {
        case ID_ON:
            mScriptContext.pass->setDepthCheckEnabled(true);
            break;
        case ID_OFF:
            mScriptContext.pass->setDepthCheckEnabled(false);
            break;
        default:
            logParseError("Bad depth_check attribute, valid parameters are 'on' or 'off'.");
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseDepthWrite(void)
    {
        switch (getNextToken().tokenID)
        {
        case ID_ON:
            mScriptContext.pass->setDepthWriteEnabled(true);
            break;
        case ID_OFF:
            mScriptContext.pass->setDepthWriteEnabled(false);
            break;
        default:
            logParseError("Bad depth_write attribute, valid parameters are 'on' or 'off'.");
        }
    }
    //-----------------------------------------------------------------------
    CompareFunction MaterialScriptCompiler::convertCompareFunction(void)
    {
        switch (getNextToken().tokenID)
        {
        case ID_ALWAYS_FAIL:
            return CMPF_ALWAYS_FAIL;
        case ID_ALWAYS_PASS:
            return CMPF_ALWAYS_PASS;
        case ID_LESS:
            return CMPF_LESS;
        case ID_LESS_EQUAL:
            return CMPF_LESS_EQUAL;
        case ID_EQUAL:
            return CMPF_EQUAL;
        case ID_NOT_EQUAL:
            return CMPF_NOT_EQUAL;
        case ID_GREATER_EQUAL:
            return CMPF_GREATER_EQUAL;
        case ID_GREATER:
            return CMPF_GREATER;
        }

        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid compare function", "convertCompareFunction");
    }

    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseDepthFunc(void)
    {
        const CompareFunction func = convertCompareFunction();
        mScriptContext.pass->setDepthFunction(func);
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseAlphaRejection(void)
    {
        const size_t paramCount = getTokenQueCount();
        if (paramCount != 2)
        {
            logParseError("Bad alpha_rejection attribute, wrong number of parameters (expected 2)");
            return;
        }

        const CompareFunction cmp = convertCompareFunction();
        mScriptContext.pass->setAlphaRejectSettings(cmp, getNextTokenValue());
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseColourWrite(void)
    {
        switch (getNextToken().tokenID)
        {
        case ID_ON:
            mScriptContext.pass->setColourWriteEnabled(true);
            break;
        case ID_OFF:
            mScriptContext.pass->setColourWriteEnabled(false);
            break;
        default:
            logParseError("Bad colour_write attribute, valid parameters are 'on' or 'off'.");
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseCullHardware(void)
    {
        switch (getNextToken().tokenID)
        {
        case ID_CULL_NONE:
            mScriptContext.pass->setCullingMode(CULL_NONE);
            break;
        case ID_ANTICLOCKWISE:
            mScriptContext.pass->setCullingMode(CULL_ANTICLOCKWISE);
            break;
        case ID_CLOCKWISE:
            mScriptContext.pass->setCullingMode(CULL_CLOCKWISE);
            break;
        default:
            logParseError(
                "Bad cull_hardware attribute, valid parameters are "
                "'none', 'clockwise' or 'anticlockwise'.");
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseCullSoftware(void)
    {
        switch (getNextToken().tokenID)
        {
        case ID_CULL_NONE:
            mScriptContext.pass->setManualCullingMode(MANUAL_CULL_NONE);
            break;
        case ID_CULL_BACK:
            mScriptContext.pass->setManualCullingMode(MANUAL_CULL_BACK);
            break;
        case ID_CULL_FRONT:
            mScriptContext.pass->setManualCullingMode(MANUAL_CULL_FRONT);
            break;
        default:
            logParseError(
                "Bad cull_software attribute, valid parameters are 'none', "
                "'front' or 'back'.");
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseLighting(void)
    {
        switch (getNextToken().tokenID)
        {
        case ID_ON:
            mScriptContext.pass->setLightingEnabled(true);
            break;
        case ID_OFF:
            mScriptContext.pass->setLightingEnabled(false);
            break;
        default:
            logParseError("Bad lighting attribute, valid parameters are 'on' or 'off'.");
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseMaxLights(void)
    {
		mScriptContext.pass->setMaxSimultaneousLights(static_cast<int>(getNextTokenValue()));
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseShading(void)
    {
        switch (getNextToken().tokenID)
        {
        case ID_FLAT:
            mScriptContext.pass->setShadingMode(SO_FLAT);
            break;
        case ID_GOURAUD:
            mScriptContext.pass->setShadingMode(SO_GOURAUD);
            break;
        case ID_PHONG:
            mScriptContext.pass->setShadingMode(SO_PHONG);
            break;
        default:
            logParseError("Bad shading attribute, valid parameters are 'flat', "
                "'gouraud' or 'phong'.");
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parsePointSize(void)
    {
        mScriptContext.pass->setPointSize(getNextTokenValue());
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parsePointSprites(void)
	{
        switch (getNextToken().tokenID)
        {
        case ID_ON:
            mScriptContext.pass->setPointSpritesEnabled(true);
            break;
        case ID_OFF:
            mScriptContext.pass->setPointSpritesEnabled(false);
            break;
        default:
            logParseError("Bad point_sprites attribute, valid parameters are 'on' or 'off'.");
        }
	}
    //-----------------------------------------------------------------------
	void MaterialScriptCompiler::parsePointSizeMin(void)
	{
        mScriptContext.pass->setPointMinSize(getNextTokenValue());
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parsePointSizeMax(void)
	{
        mScriptContext.pass->setPointMaxSize(getNextTokenValue());
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parsePointSizeAttenuation(void)
	{
        const size_t paramCount = getTokenQueCount();
        if (paramCount != 1 && paramCount != 4)
        {
            logParseError("Bad point_size_attenuation attribute, wrong number of parameters (expected 1 or 4)");
            return;
        }
        switch (getNextToken().tokenID)
        {
        case ID_ON:
			if (paramCount == 4)
			{
				Real constant = getNextTokenValue();
				Real linear = getNextTokenValue();
				Real quadric = getNextTokenValue();
	            mScriptContext.pass->setPointAttenuation(true, constant, linear, quadric);
			}
			else
			{
				mScriptContext.pass->setPointAttenuation(true);
			}
			
            break;
        case ID_OFF:
            mScriptContext.pass->setPointAttenuation(false);
            break;
        default:
            logParseError("Bad point_size_attenuation attribute, valid values are 'on' or 'off'.");
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
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseTextureUnit(void)
    {
        String tusName;
        const size_t paramCount = getTokenQueCount();
        // if params is a name then see if that texture unit exists
        // if not then log the warning and just move on to the next TU from current
        if ((paramCount > 0) && (mScriptContext.pass->getNumTextureUnitStates() > 0))
        {
            // specifying a TUS name in the script for a TU means that a specific TU is being requested
            // try to get the specific TU
            // if the index requested is not valid, just creat a new TU
            // find the TUS with name
            tusName = getNextTokenLabel();
            TextureUnitState * foundTUS = mScriptContext.pass->getTextureUnitState(tusName);
            if (foundTUS)
            {
                mScriptContext.stateLev = mScriptContext.pass->getTextureUnitStateIndex(foundTUS);
            }
            else
            {
                // name was not found so a new TUS is needed
                // position TUS level to the end index
                // a new TUS will be created later on
                mScriptContext.stateLev = static_cast<uint>(mScriptContext.pass->getNumTextureUnitStates());
            }
        }
        else
        {
		    //Increase Texture Unit State level depth
		    ++mScriptContext.stateLev;
        }

        if (mScriptContext.pass->getNumTextureUnitStates() > mScriptContext.stateLev)
        {
            mScriptContext.textureUnit = mScriptContext.pass->getTextureUnitState(mScriptContext.stateLev);
        }
        else
        {
            // Create a new texture unit
            mScriptContext.textureUnit = mScriptContext.pass->createTextureUnitState();
            if (!tusName.empty())
                mScriptContext.textureUnit->setName(tusName);
        }
        // update section
        mScriptContext.section = MSS_TEXTUREUNIT;
    }
    //-----------------------------------------------------------------------
	void MaterialScriptCompiler::finishProgramDefinition(void)
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
					", you must specify a source file.");
			}
			if (def->syntax.empty())
			{
				logParseError("Invalid program definition for " + def->name +
					", you must specify a syntax code.");
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
					", you must specify a source file.");
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
						    " parameter " + i->first + " is not valid.");
				    }
			    }
            }
            catch (Exception& e)
            {
                logParseError("Could not create GPU program '"
                    + def->name + "', error reported was: " + e.getFullDescription());
				mScriptContext.program.setNull();
            	mScriptContext.programParams.setNull();
				return;
            }
        }
        // Set skeletal animation option
        gp->setSkeletalAnimationIncluded(def->supportsSkeletalAnimation);
		// Set morph animation option
		gp->setMorphAnimationIncluded(def->supportsMorphAnimation);
		// Set pose animation option
		gp->setPoseAnimationIncluded(def->supportsPoseAnimation);
		// set origin
		gp->_notifyOrigin(mScriptContext.filename);

        // Set up to receive default parameters
        if (gp->isSupported() 
            && !mScriptContext.defaultParamLines.empty())
        {
            mScriptContext.programParams = gp->getDefaultParameters();
			mScriptContext.numAnimationParametrics = 0;
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
                //******************** FIX THIS
                //AttribParserList::iterator iparser 
                //    = mProgramDefaultParamAttribParsers.find(splitCmd[0]);
                //if (iparser != mProgramDefaultParamAttribParsers.end())
                //{
                //    String cmd = splitCmd.size() >= 2? splitCmd[1]:StringUtil::BLANK;
                //    // Use parser with remainder
                //    iparser->second(cmd, mScriptContext );
                //}

            }
            // Reset
            mScriptContext.program.setNull();
            mScriptContext.programParams.setNull();
        }

	}


}

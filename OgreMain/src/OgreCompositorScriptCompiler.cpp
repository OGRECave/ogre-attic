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
#include "OgreCompositorScriptCompiler.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreCompositorManager.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionPass.h"

namespace Ogre {

	//-----------------------------------------------------------------------
    // Static definitions
    //-----------------------------------------------------------------------
    CompositorScriptCompiler::TokenActionMap CompositorScriptCompiler::mTokenActionMap;

	String CompositorScriptCompiler::compositorScript_BNF =
		// Top level rule
		"<Script> ::= {<Script_Properties>} \n"
		"<Script_Properties> ::= {<CompositorDef>} \n"
		"<CompositorDef> ::= 'compositor' <Label> '{' <TechniqueDef> '}' \n"
		// Technique
		"<TechniqueDef> ::= 'technique' '{' {<TextureDef>} {<TargetDef>} <TargetOutputDef> '}' \n"
		"<TextureDef> ::= 'texture' <Label> <WidthOptionDef> <HeightOptionDef> <PixelFormatDef> \n"
		"<WidthOptionDef> ::= 'target_width' | <#width> \n"
		"<HeightOptionDef> ::= 'target_height' | <#height> \n"
		"<PixelFormatDef> ::= 'PF_A8R8G8B8' | 'PF_R8G8B8A8' | 'PF_R8G8B8' | 'PF_FLOAT16_R' | 'PF_FLOAT16_RGB' | 'PF_FLOAT16_RGBA' | 'PF_FLOAT32_R' | 'PF_FLOAT32_RGB' | 'PF_FLOAT32_RGBA' \n"
		// Target
		"<TargetDef> ::= 'target ' <Label> '{' {<TargetOptionsDef>} {<PassDef>} '}' \n"
	    "<TargetOptionsDef> ::=	[<TargetInputDef>] | [<OnlyInitialDef>]  | [<VisibilityMaskDef>] | [<LodBiasDef>] | [<MaterialSchemeDef>] \n"
		"<TargetInputDef> ::= 'input' <TargetInputOptionsDef> \n"
		"<TargetInputOptionsDef> ::= 'none' | 'previous' \n"
		"<OnlyInitialDef> ::= 'only_initial' <On_Off> \n"
		"<VisibilityMaskDef> ::= 'visibility_mask' <#mask> \n"
		"<LodBiasDef> ::= 'lod_bias' <#lodbias> \n"
		"<MaterialSchemeDef> ::= 'material_scheme' <Label> \n"
		"<TargetOutputDef> ::= 'target_output' '{' [<TargetInputDef>] {<PassDef>} '}' \n"
		// Pass
		"<PassDef> ::= 'pass' <PassTypeDef> '{' <PassOptionsDef> '}' \n"
		"<PassTypeDef> ::= 'render_quad' | 'clear' | 'stencil' | 'render_scene' \n"
		"<PassOptionsDef> ::= [<PassMaterialDef>] {<PassInputDef>} \n"
		"<PassMaterialDef> ::= 'material' <Label> \n"
		"<PassInputDef> ::= 'input' <#id> <Label> \n"
		// common rules
		"<On_Off> ::= 'on' | 'off' \n"
		"<Label> ::= <Unquoted_Label> | <Quoted_Label> \n"
		"<Quoted_Label> ::= '\"' <Character> {<Alphanumeric_Space>} '\"' \n"
        "<Unquoted_Label> ::= <Character> {<Alphanumeric>} \n"
		"<Alphanumeric_Space> ::= <Alphanumeric> | <Space> \n"
		"<Alphanumeric> ::= <Character> | <Number> \n"
		"<Character> ::= (abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$#%!_*&\\/) \n"
		"<Number> ::= (0123456789) \n"
		"<Space> ::= ( ) \n"

		;
	//-----------------------------------------------------------------------
	CompositorScriptCompiler::CompositorScriptCompiler(void)
	{
	}
	//-----------------------------------------------------------------------
	CompositorScriptCompiler::~CompositorScriptCompiler(void)
	{

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::setupTokenDefinitions(void)
	{
		addLexemeTokenAction("{", ID_OPENBRACE, &CompositorScriptCompiler::parseOpenBrace);
		addLexemeTokenAction("}", ID_CLOSEBRACE, &CompositorScriptCompiler::parseCloseBrace);
		addLexemeTokenAction("compositor", ID_COMPOSITOR, &CompositorScriptCompiler::parseCompositor);

		// Technique section
		addLexemeTokenAction("technique", ID_TECHNIQUE, &CompositorScriptCompiler::parseTechnique);
		addLexemeTokenAction("texture", ID_TEXTURE, &CompositorScriptCompiler::parseTexture);
		addLexemeTokenAction("target_width", ID_TARGET_WIDTH);
		addLexemeTokenAction("target_height", ID_TARGET_HEIGHT);
		addLexemeTokenAction("PF_A8R8G8B8", ID_PF_A8R8G8B8);
		addLexemeTokenAction("PF_R8G8B8A8", ID_PF_R8G8B8A8);
		addLexemeTokenAction("PF_R8G8B8", ID_PF_R8G8B8);
		addLexemeTokenAction("PF_FLOAT16_R", ID_PF_FLOAT16_R);
		addLexemeTokenAction("PF_FLOAT16_RGB", ID_PF_FLOAT16_RGB);
		addLexemeTokenAction("PF_FLOAT16_RGBA", ID_PF_FLOAT16_RGBA);
		addLexemeTokenAction("PF_FLOAT32_R", ID_PF_FLOAT32_R);
		addLexemeTokenAction("PF_FLOAT32_RGB", ID_PF_FLOAT32_RGB);
		addLexemeTokenAction("PF_FLOAT32_RGBA", ID_PF_FLOAT32_RGBA);

		// Target section
		addLexemeTokenAction("target ", ID_TARGET, &CompositorScriptCompiler::parseTarget);
		addLexemeTokenAction("input", ID_INPUT, &CompositorScriptCompiler::parseInput);
		addLexemeTokenAction("none", ID_NONE);
		addLexemeTokenAction("previous", ID_PREVIOUS);
		addLexemeTokenAction("target_output", ID_TARGET_OUTPUT, &CompositorScriptCompiler::parseTargetOutput);
		addLexemeTokenAction("only_initial", ID_ONLY_INITIAL, &CompositorScriptCompiler::parseOnlyInitial);
		addLexemeTokenAction("visibility_mask", ID_VISIBILITY_MASK, &CompositorScriptCompiler::parseVisibilityMask);
		addLexemeTokenAction("lod_bias", ID_LOD_BIAS, &CompositorScriptCompiler::parseLodBias);
		addLexemeTokenAction("material_scheme", ID_MATERIAL_SCHEME, &CompositorScriptCompiler::parseMaterialScheme);

		// pass section
		addLexemeTokenAction("pass", ID_PASS, &CompositorScriptCompiler::parsePass);
		// input defined above
		addLexemeTokenAction("render_quad", ID_RENDER_QUAD);
		addLexemeTokenAction("clear", ID_CLEAR);
		addLexemeTokenAction("stencil", ID_STENCIL);
		addLexemeTokenAction("render_scene", ID_RENDER_SCENE);
		addLexemeTokenAction("material", ID_MATERIAL, &CompositorScriptCompiler::parseMaterial);

		// common section
		addLexemeTokenAction("on", ID_ON);
		addLexemeTokenAction("off", ID_OFF);

	}

	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::addLexemeTokenAction(const String& lexeme,
		const size_t token, const CSC_Action action)
	{
		addLexemeToken(lexeme, token, action != 0);
		mTokenActionMap[token] = action;
	}

	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::executeTokenAction(const size_t tokenID)
	{
		TokenActionIterator action = mTokenActionMap.find(tokenID);

		if (action == mTokenActionMap.end())
		{
			// BAD command. BAD!
			logParseError("Unrecognised compositor script command action");
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
	void CompositorScriptCompiler::logParseError(const String& error)
	{
		// log material name only if filename not specified
		if (mSourceName.empty() && !mScriptContext.compositor.isNull())
		{
			LogManager::getSingleton().logMessage(
				"Error in compositor " + mScriptContext.compositor->getName() +
				" : " + error);
		}
		else
		{
			if (!mScriptContext.compositor.isNull())
			{
				LogManager::getSingleton().logMessage(
					"Error in compositor " + mScriptContext.compositor->getName() +
					" at line " + StringConverter::toString(mCurrentLine) +
					" of " + mSourceName + ": " + error);
			}
			else
			{
				LogManager::getSingleton().logMessage(
					"Error at line " + StringConverter::toString(mCurrentLine) +
					" of " + mSourceName + ": " + error);
			}
		}
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseOpenBrace(void)
	{

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseCloseBrace(void)
	{
		switch(mScriptContext.section)
		{
		case CSS_NONE:
			logParseError("Unexpected terminating brace.");
			break;
		case CSS_COMPOSITOR:
			// End of compositor
			mScriptContext.section = CSS_NONE;
			mScriptContext.compositor.setNull();
			break;
		case CSS_TECHNIQUE:
			// End of technique
			mScriptContext.section = CSS_COMPOSITOR;
			mScriptContext.technique = NULL;
			break;
		case CSS_TARGET:
			// End of target
			mScriptContext.section = CSS_TECHNIQUE;
			mScriptContext.target = NULL;
			break;
		case CSS_PASS:
			// End of pass
			mScriptContext.section = CSS_TARGET;
			mScriptContext.pass = NULL;
			break;
		};
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseCompositor(void)
	{
		//logParseError("parseCompositor");
		const String compositorName = getNextTokenLabel();
		mScriptContext.compositor = CompositorManager::getSingleton().create(
            compositorName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
		mScriptContext.section = CSS_COMPOSITOR;

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseTechnique(void)
	{
		//logParseError("parseTechnique");

		mScriptContext.technique = mScriptContext.compositor->createTechnique();
		mScriptContext.section = CSS_TECHNIQUE;

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseTexture(void)
	{
		//logParseError("parseTexture");
		const String textureName = getNextTokenLabel();
        CompositionTechnique::TextureDefinition* textureDef = mScriptContext.technique->createTextureDefinition(textureName);
        // ********* needs fixing ie get params from token stream
        // if peek next token is target_width then get token and use 0 for width
        // determine width parameter
        if (testNextTokenID(ID_TARGET_WIDTH))
        {
            getNextToken();
            // a value of zero causes texture to be size of render target
            textureDef->width = 0;
        }
        else
        {
            textureDef->width = static_cast<size_t>(getNextTokenValue());
        }
        // determine height parameter
        if (testNextTokenID(ID_TARGET_HEIGHT))
        {
            getNextToken();
            // a value of zero causes texture to be size of render target
            textureDef->height = 0;
        }
        else
        {
            textureDef->height = static_cast<size_t>(getNextTokenValue());
        }
        // get pixel factor
        switch (getNextToken().tokenID)
        {
        case ID_PF_A8R8G8B8:
            textureDef->format = PF_A8R8G8B8;
            break;

        case ID_PF_R8G8B8A8:
            textureDef->format = PF_R8G8B8A8;
            break;

        case ID_PF_R8G8B8:
            textureDef->format = PF_R8G8B8;
            break;
		case ID_PF_FLOAT16_R:
            textureDef->format = PF_FLOAT16_R;
            break;
		case ID_PF_FLOAT16_RGB:
            textureDef->format = PF_FLOAT16_RGB;
            break;
		case ID_PF_FLOAT16_RGBA:
            textureDef->format = PF_FLOAT16_RGBA;
            break;
		case ID_PF_FLOAT32_R:
            textureDef->format = PF_FLOAT32_R;
            break;
		case ID_PF_FLOAT32_RGB:
            textureDef->format = PF_FLOAT32_RGB;
            break;
		case ID_PF_FLOAT32_RGBA:
            textureDef->format = PF_FLOAT32_RGBA;
            break;

        default:
            // should never get here?
            break;
        }
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseTarget(void)
	{
		//logParseError("parseTarget");
		mScriptContext.section = CSS_TARGET;
        assert(mScriptContext.technique);
		const String targetName = getNextTokenLabel();

        mScriptContext.target = mScriptContext.technique->createTargetPass();
        mScriptContext.target->setOutputName(targetName);

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseInput(void)
	{
		//logParseError("parseInput");
		// input parameters depends on context either target or pass
		if (mScriptContext.section == CSS_TARGET)
		{
		    // for input in target, there is only one parameter
		    assert(mScriptContext.target);
		    if (testNextTokenID(ID_PREVIOUS))
                mScriptContext.target->setInputMode(CompositionTargetPass::IM_PREVIOUS);
            else
                mScriptContext.target->setInputMode(CompositionTargetPass::IM_NONE);
		}
		else // assume for pass section context
		{
		    // for input in pass, there are two parameters
		    uint32 id = static_cast<uint32>(getNextTokenValue());
		    const String& textureName = getNextTokenLabel();
		    assert(mScriptContext.pass);
		    mScriptContext.pass->setInput(id, textureName);
		}

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseTargetOutput(void)
	{
		//logParseError("parseTargetOutput");
		assert(mScriptContext.technique);
		mScriptContext.target = mScriptContext.technique->getOutputTargetPass();
		mScriptContext.section = CSS_TARGET;
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseOnlyInitial(void)
	{
		//logParseError("parseOnlyInitial");
		if (testNextTokenID(ID_ON))
		{
            assert(mScriptContext.target);
            mScriptContext.target->setOnlyInitial(true);
		}

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseVisibilityMask(void)
	{
		uint32 mask = static_cast<uint32>(getNextTokenValue());
        assert(mScriptContext.target);
        mScriptContext.target->setVisibilityMask(mask);
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseLodBias(void)
	{
		Real val = getNextTokenValue();
        assert(mScriptContext.target);
        mScriptContext.target->setLodBias(val);
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseMaterialScheme(void)
	{
		assert(mScriptContext.target);
		mScriptContext.target->setMaterialScheme(getNextTokenLabel());
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parsePass(void)
	{
		//logParseError("parsePass");
		assert(mScriptContext.target);
        mScriptContext.pass = mScriptContext.target->createPass();
        CompositionPass::PassType passType = CompositionPass::PT_RENDERQUAD;
        switch (getNextToken().tokenID)
        {
        case ID_RENDER_QUAD:
            passType = CompositionPass::PT_RENDERQUAD;
            break;

        case ID_CLEAR:
            passType = CompositionPass::PT_CLEAR;
            break;

        case ID_STENCIL:
            passType = CompositionPass::PT_STENCIL;
            break;

        case ID_RENDER_SCENE:
            passType = CompositionPass::PT_RENDERSCENE;
            break;

        default:
            break;
        }

        mScriptContext.pass->setType(passType);

		mScriptContext.section = CSS_PASS;

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseMaterial(void)
	{
		//logParseError("parseMaterial");
        mScriptContext.pass->setMaterialName(getNextTokenLabel());

	}


}

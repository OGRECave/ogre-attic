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
		"<PixelFormatDef> ::= 'PF_A8R8G8B8' | 'PF_R8G8B8A8' | 'PF_R8G8B8' \n"
		// Target
		"<TargetDef> ::= 'target ' <Label> '{' [<TargetInputDef>] [<OnlyInitialDef>] {<PassDef>} '}' \n"
		"<TargetInputDef> ::= 'input' <TargetInputOptionsDef> \n"
		"<TargetInputOptionsDef> ::= 'none' | 'previous' \n"
		"<OnlyInitialDef> ::= 'only_initial' <On_Off> \n"
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
		"<Alphanumeric_Space> ::= <Alphanumeric> | ' ' \n"
		"<Alphanumeric> ::= <Character> | <Number> \n"
		"<Character> ::= (abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$#%!_*&\\/) \n"
		"<Number> ::= (0123456789) \n"

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

		// Target section
		addLexemeTokenAction("target", ID_TARGET, &CompositorScriptCompiler::parseTarget);
		addLexemeTokenAction("input", ID_INPUT, &CompositorScriptCompiler::parseInput);
		addLexemeTokenAction("none", ID_NONE);
		addLexemeTokenAction("previous", ID_PREVIOUS);
		addLexemeTokenAction("target_output", ID_TARGET_OUTPUT, &CompositorScriptCompiler::parseTargetOutput);
		addLexemeTokenAction("only_inital", ID_ONLY_INITIAL, &CompositorScriptCompiler::parseOnlyInitial);

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
		if (mScriptContext.filename.empty() && !mScriptContext.compositor.isNull())
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
					" of " + mScriptContext.filename + ": " + error);
			}
			else
			{
				LogManager::getSingleton().logMessage(
					"Error at line " + StringConverter::toString(mCurrentLine) +
					" of " + mScriptContext.filename + ": " + error);
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
		case CSS_TARGET_OUTPUT:
			// End of target
			mScriptContext.section = CSS_TECHNIQUE;
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
		logParseError("parseCompositor");
		const String compositorName = getNextTokenLabel();
		mScriptContext.compositor = CompositorManager::getSingleton().create(
            compositorName, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
			);
		mScriptContext.section = CSS_COMPOSITOR;

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseTechnique(void)
	{
		logParseError("parseTechnique");

		mScriptContext.technique = mScriptContext.compositor->createTechnique();
		mScriptContext.section = CSS_TECHNIQUE;

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseTexture(void)
	{
		logParseError("parseTexture");
		const String textureName = getNextTokenLabel();
        CompositionTechnique::TextureDefinition* textureDef = mScriptContext.technique->createTextureDefinition(textureName);
        // ********* needs fixing ie get params from token stream
        textureDef->width = 128;
        textureDef->height = 128;
        textureDef->format = PF_A8R8G8B8;
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseTarget(void)
	{
		logParseError("parseTarget");
		mScriptContext.section = CSS_TARGET;

		const String targetName = getNextTokenLabel();
        mScriptContext.target = mScriptContext.technique->createTargetPass();
        // ********* needs fixing ie get params from token stream
        //mScriptContext.target->setInputMode(CompositionTargetPass::IM_PREVIOUS);
        mScriptContext.target->setOutputName(targetName);

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseInput(void)
	{
		logParseError("parseInput");

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseTargetOutput(void)
	{
		logParseError("parseTargetOutput");
		mScriptContext.section = CSS_TARGET_OUTPUT;
	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseOnlyInitial(void)
	{
		logParseError("parseOnlyInitial");

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parsePass(void)
	{
		logParseError("parsePass");
        mScriptContext.pass = mScriptContext.target->createPass();
        // ********* needs fixing ie get params from token stream
        mScriptContext.pass->setType(CompositionPass::PT_RENDERQUAD);
        //mScriptContext.pass->setInput(0, "rt0");
		mScriptContext.section = CSS_PASS;

	}
	//-----------------------------------------------------------------------
	void CompositorScriptCompiler::parseMaterial(void)
	{
		logParseError("parseMaterial");
        mScriptContext.pass->setMaterialName(getNextTokenLabel());

	}


}

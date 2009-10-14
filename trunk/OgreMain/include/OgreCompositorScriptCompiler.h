/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef __CompositorScriptScompiler_H__
#define __CompositorScriptScompiler_H__

#include "OgrePrerequisites.h"
#include "OgreCompiler2Pass.h"
#include "OgreCompositor.h"
#include "OgreRenderSystem.h"


namespace Ogre {
	/** \addtogroup Core
	*  @{
	*/
	/** \addtogroup General
	*  @{
	*/

	/** Compiler for parsing & lexing .compositor scripts */
	class _OgreExport CompositorScriptCompiler : public Compiler2Pass
	{

	public:
		CompositorScriptCompiler(void);
		~CompositorScriptCompiler(void);

        /** gets BNF Grammar for Compositor script.
        */
        virtual const String& getClientBNFGrammer(void) const;

        /** get the name of the Compositor script BNF grammar.
        */
        virtual const String& getClientGrammerName(void) const;

        /** Compile a compositor script from a data stream using a specific resource group name.
        @param stream Weak reference to a data stream which is the source of the material script
        @param groupName The name of the resource group that resources which are
			parsed are to become a member of. If this group is loaded or unloaded,
			then the resources discovered in this script will be loaded / unloaded
			with it.
        */
        void parseScript(DataStreamPtr& stream, const String& groupName)
        {
            mScriptContext.groupName = groupName;
            Compiler2Pass::compile(stream->getAsString(),  stream->getName());
        }

	protected:
		// Token ID enumeration
		enum TokenID {
			// Terminal Tokens section
			ID_UNKOWN = 0,
			// Techniques
			ID_TARGET_WIDTH, ID_TARGET_HEIGHT,
			ID_TARGET_WIDTH_SCALED, ID_TARGET_HEIGHT_SCALED,
			ID_PF_A8R8G8B8, ID_PF_R8G8B8A8, ID_PF_R8G8B8, 
			ID_PF_FLOAT16_R, ID_PF_FLOAT16_RGB, ID_PF_FLOAT16_RGBA,
			ID_PF_FLOAT32_R, ID_PF_FLOAT32_RGB, ID_PF_FLOAT32_RGBA,
			ID_PF_FLOAT16_GR, ID_PF_FLOAT32_GR,
			ID_POOLED, ID_GAMMA, ID_NO_FSAA,
			ID_SCOPE_LOCAL, ID_SCOPE_CHAIN, ID_SCOPE_GLOBAL,
			// Targets
			ID_PREVIOUS, ID_NONE,
			// Passes
			ID_RENDER_QUAD, ID_CLEAR, ID_STENCIL, ID_RENDER_SCENE, ID_RENDER_CUSTOM,
			// Clear section
			ID_CLR_COLOUR, ID_CLR_DEPTH,
			// Stencil section

			// compare functions
            ID_ST_ALWAYS_FAIL, ID_ST_ALWAYS_PASS, ID_ST_LESS,
            ID_ST_LESS_EQUAL, ID_ST_EQUAL, ID_ST_NOT_EQUAL,
            ID_ST_GREATER_EQUAL, ID_ST_GREATER,

            // stencil operations
            ID_ST_KEEP, ID_ST_ZERO, ID_ST_REPLACE, ID_ST_INCREMENT,
            ID_ST_DECREMENT, ID_ST_INCREMENT_WRAP, ID_ST_DECREMENT_WRAP,
            ID_ST_INVERT,

			// general
			ID_ON, ID_OFF, ID_TRUE, ID_FALSE,
            // where auto generated tokens start so do not remove
            ID_AUTOTOKENSTART
		};

		/** Enum to identify compositor sections. */
		enum CompositorScriptSection
		{
			CSS_NONE,
			CSS_COMPOSITOR,
			CSS_TECHNIQUE,
			CSS_TARGET,
			CSS_PASS
		};
		/** Struct for holding the script context while parsing. */
		struct CompositorScriptContext
		{
			CompositorScriptSection section;
		    String groupName;
			CompositorPtr compositor;
			CompositionTechnique* technique;
			CompositionTargetPass* target;
			CompositionPass* pass;
		};

		CompositorScriptContext mScriptContext;

		typedef void (CompositorScriptCompiler::* CSC_Action)(void);
		typedef map<size_t, CSC_Action>::type TokenActionMap;
		typedef TokenActionMap::iterator TokenActionIterator;
		/** Map of Token value as key to an Action.  An Action converts tokens into
		the final format.
            All instances use the same Token Action Map.
		*/
		static TokenActionMap mTokenActionMap;

		/** Execute an Action associated with a token.  Gets called when the compiler finishes tokenizing a
		section of the source that has been parsed.
		**/
		virtual void executeTokenAction(const size_t tokenID);
        /** Get the start position of auto generated token IDs.
        */
        virtual size_t getAutoTokenIDStart() const {return ID_AUTOTOKENSTART;}
		/** Associate all the lexemes used in a material script with their corresponding tokens and actions.
		**/
        virtual void setupTokenDefinitions(void);
		void addLexemeTokenAction(const String& lexeme, const size_t token, const CSC_Action action = 0);
        void addLexemeAction(const String& lexeme, const CSC_Action action) { addLexemeTokenAction(lexeme, 0, action); }

		void logParseError(const String& error);

		// Token Actions which get called when tokens are created during parsing.
		void parseOpenBrace(void);
		void parseCloseBrace(void);
		void parseCompositor(void);
		void parseTechnique(void);
		void parseTexture(void);
		void parseTextureRef(void);
		void parseCompositorLogic(void);
		void parseScheme(void);
		void parseTarget(void);
		void parseInput(void);
		void parseTargetOutput(void);
		void parseOnlyInitial(void);
		void parseVisibilityMask(void);
		void parseLodBias(void);
		void parseMaterialScheme(void);
		void parseShadowsEnabled(void);
		void parsePass(void);
		void parseMaterial(void);
		void parseFirstRenderQueue(void);
		void parseLastRenderQueue(void);
		void parseIdentifier(void);
		void parseClearBuffers(void);
		void parseClearColourValue(void);
		void parseClearDepthValue(void);
		void parseClearStencilValue(void);
		void parseStencilCheck(void);
		void parseStencilFunc(void);
		void parseStencilRefVal(void);
		void parseStencilMask(void);
		void parseStencilFailOp(void);
		void parseStencilDepthFailOp(void);
		void parseStencilPassOp(void);
		void parseStencilTwoSided(void);
		StencilOperation extractStencilOp(void);
        CompareFunction extractCompareFunc(void);
	};
	/** @} */
	/** @} */
}

#endif

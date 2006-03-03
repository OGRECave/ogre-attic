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

#ifndef __CompositorScriptScompiler_H__
#define __CompositorScriptScompiler_H__

#include "OgreCompiler2Pass.h"
#include "OgrePrerequisites.h"
#include "OgreCompositor.h"

namespace Ogre {

	/** Compiler for parsing & lexing .compositor scripts */
	class _OgreExport CompositorScriptCompiler : public Compiler2Pass
	{

	public:
		CompositorScriptCompiler(void);
		~CompositorScriptCompiler(void);

        /** gets BNF Grammer for Compositor script.
        */
        virtual const String& getClientBNFGrammer(void) { return compositorScript_BNF; }

        /** get the name of the Compositor script BNF grammer.
        */
        virtual const String& getClientGrammerName(void) { static const String grammerName("Compositor Script"); return grammerName; }

	protected:
		// Token ID enumeration
		enum TokenID {
			// Terminal Tokens section
			ID_UNKOWN = 0, ID_OPENBRACE, ID_CLOSEBRACE,
			// Top level
			ID_COMPOSITOR,
			// Techniques
			ID_TECHNIQUE, ID_TEXTURE, ID_TARGET_WIDTH, ID_TARGET_HEIGHT,
			ID_PF_A8R8G8B8, ID_PF_R8G8B8A8, ID_PF_R8G8B8,
			// Targets
			ID_TARGET, ID_INPUT, ID_TARGET_OUTPUT, ID_ONLY_INITIAL,
			ID_PREVIOUS, ID_NONE,
			// Passes
			ID_PASS,
			ID_MATERIAL,
			ID_RENDER_QUAD, ID_CLEAR, ID_STENCIL, ID_RENDER_SCENE,

			// general
			ID_ON, ID_OFF, ID_TRUE, ID_FALSE
		};

		/** Enum to identify compositor sections. */
		enum CompositorScriptSection
		{
			CSS_NONE,
			CSS_COMPOSITOR,
			CSS_TECHNIQUE,
			CSS_TARGET,
			CSS_TARGET_OUTPUT,
			CSS_PASS
		};
		/** Struct for holding the script context while parsing. */
		struct CompositorScriptContext
		{
			CompositorScriptSection section;
			CompositorPtr compositor;
			CompositionTechnique* technique;
			CompositionTargetPass* target;
			CompositionPass* pass;
			// Error reporting state
			size_t lineNo;
			String filename;
		};

		CompositorScriptContext mScriptContext;

		// static library database for tokens and BNF rules
		static TokenRule compositorScript_RulePath[];
		// simplified Backus - Naur Form (BNF) grammer for compositor scripts
		static String compositorScript_BNF;

		typedef void (CompositorScriptCompiler::* CSC_Action)(void);
		typedef std::map<size_t, CSC_Action> TokenActionMap;
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
		/** Associate all the lexemes used in a material script with their corresponding tokens and actions.
		**/
        virtual void setupTokenDefinitions(void);
		void addLexemeTokenAction(const String& lexeme, const size_t token, const CSC_Action action = 0);

		void logParseError(const String& error);

		// Token Actions which get called when tokens are created during parsing.
		void parseOpenBrace(void);
		void parseCloseBrace(void);
		void parseCompositor(void);
		void parseTechnique(void);
		void parseTexture(void);
		void parseTarget(void);
		void parseInput(void);
		void parseTargetOutput(void);
		void parseOnlyInitial(void);
		void parsePass(void);
		void parseMaterial(void);

	};
}

#endif

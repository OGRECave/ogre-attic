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

#ifndef __MaterialScriptScompiler_H__
#define __MaterialScriptScompiler_H__

#include "OgreCompiler2Pass.h"
#include "OgrePrerequisites.h"
//#include "OgreMaterial.h"
//#include "OgreBlendMode.h"
//#include "OgreTextureUnitState.h"
//#include "OgreGpuProgram.h"

namespace Ogre {

    class _OgreExport MaterialScriptCompiler : public Compiler2Pass
    {

    public:
        MaterialScriptCompiler(void);
        ~MaterialScriptCompiler(void);
        /** gets BNF Grammer for Compositor script.
        */
        virtual const String& getClientBNFGrammer(void) { return materialScript_BNF; }

        /** get the name of the BNF grammer.
        */
        virtual const String& getClientGrammerName(void) { static const String grammerName("Material Script"); return grammerName; }

    protected:
	    // Token ID enumeration
	    enum TokenID {
		    // Terminal Tokens section
            ID_UNKOWN = 0, ID_OPENBRACE, ID_CLOSEBRACE,
            ID_VERTEX_PROGRAM, ID_FRAGMENT_PROGRAM,
            // material
            ID_MATERIAL, ID_CLONE, ID_LOD_DISTANCES, ID_RECEIVE_SHADOWS,
            ID_TRANSPARENCY_CASTS_SHADOWS, ID_SET_TEXTURE_ALIAS,
            // technique
            ID_TECHNIQUE, ID_SCHEME, ID_LOD_INDEX,
            // pass
            ID_PASS, ID_AMBIENT, ID_DIFFUSE, ID_SPECULAR, ID_EMISSIVE,
            ID_VERTEXCOLOUR, ID_SCENE_BLEND, ID_BLEND_ADD, ID_BLEND_MODULATE, ID_COLOUR_BLEND, ID_ALPHA_BLEND,
            ID_BLEND_ONE, ID_BLEND_ZERO, ID_BLEND_DEST_COLOUR,
            ID_BLEND_SRC_COLOUR, ID_BLEND_ONCE_MINUS_DEST_COLOUR, ID_BLEND_ONE_MINUS_SRC_COLOUR,
            ID_BLEND_DEST_ALPHA, ID_BLEND_SRC_ALPHA, ID_BLEND_ONE_MINUS_DEST_ALPHA, ID_BLEND_ONE_MINUS_SRC_ALPHA,
            ID_DEPTH_CHECK, ID_DEPTH_WRITE, ID_ALPHA_REJECTION, ID_DEPTH_FUNC, ID_ALWAYS_FAIL, ID_ALWAYS_PASS,
            ID_LESS_EQUAL, ID_LESS, ID_EQUAL, ID_NOT_EQUAL, ID_GREATER_EQUAL, ID_GREATER,

            ID_COLOUR_WRITE, ID_CULL_HARDWARE, ID_CLOCKWISE, ID_ANTICLOCKWISE, ID_CULL_NONE,
            ID_CULL_SOFTWARE, ID_CULL_BACK, ID_CULL_FRONT,
            ID_SHADING, ID_FLAT, ID_GOURAUD, ID_PHONG,
            ID_LIGHTING, ID_MAX_LIGHTS, ID_FOG_OVERRIDE,
			ID_POINT_SIZE, ID_POINT_SPRITES, ID_POINT_SIZE_ATTENUATION,
			ID_POINT_SIZE_MIN, ID_POINT_SIZE_MAX,

            // texture unit state
            ID_TEXTURE_UNIT,

            // general
            ID_ON, ID_OFF, ID_TRUE, ID_FALSE
        };

        /** Enum to identify material sections. */
        enum MaterialScriptSection
        {
            MSS_NONE,
            MSS_MATERIAL,
            MSS_TECHNIQUE,
            MSS_PASS,
            MSS_TEXTUREUNIT,
            MSS_PROGRAM_REF,
		    MSS_PROGRAM,
            MSS_DEFAULT_PARAMETERS,
		    MSS_TEXTURESOURCE
        };
	    /** Struct for holding a program definition which is in progress. */
	    struct MaterialScriptProgramDefinition
	    {
		    String name;
		    GpuProgramType progType;
            String language;
		    String source;
		    String syntax;
            bool supportsSkeletalAnimation;
		    bool supportsMorphAnimation;
		    ushort supportsPoseAnimation; // number of simultaneous poses supported
		    std::map<String, String> customParameters;
	    };
        /** Struct for holding the script context while parsing. */
        struct MaterialScriptContext
        {
            MaterialScriptSection section;
		    String groupName;
            MaterialPtr material;
            Technique* technique;
            Pass* pass;
            TextureUnitState* textureUnit;
            GpuProgramPtr program; // used when referencing a program, not when defining it
            bool isProgramShadowCaster; // when referencing, are we in context of shadow caster
            bool isProgramShadowReceiver; // when referencing, are we in context of shadow caster
            GpuProgramParametersSharedPtr programParams;
			ushort numAnimationParametrics;
		    MaterialScriptProgramDefinition* programDef; // this is used while defining a program

		    int techLev,	//Keep track of what tech, pass, and state level we are in
			    passLev,
			    stateLev;
            StringVector defaultParamLines;

            AliasTextureNamePairList textureAliases;
        };

        MaterialScriptContext mScriptContext;

	    // static library database for tokens and BNF rules
	    static TokenRule materialScript_RulePath[];
        // simplified Backus - Naur Form (BNF) grammer for material scripts
        static String materialScript_BNF;

        typedef void (MaterialScriptCompiler::* MSC_Action)(void);
        typedef std::map<size_t, MSC_Action> TokenActionMap;
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
        void addLexemeTokenAction(const String& lexeme, const size_t token, const MSC_Action action = 0);

        void logParseError(const String& error);

        // support methods that convert tokens
        ColourValue _parseColourValue(void);
        CompareFunction convertCompareFunction(void);

        // Token Actions which get called when tokens are created during parsing.
        void parseOpenBrace(void);
        void parseCloseBrace(void);
        void parseVertexProgram(void);
        void parseFragmentProgram(void);
        // material section Actions
        void parseMaterial(void);
        void parseLodDistances(void);
        void parseReceiveShadows(void);
        void parseTransparencyCastsShadows(void);
        void parseSetTextureAlias(void);
        // Technique related actions
        void parseTechnique(void);
        void parseScheme(void);
        void parseLodIndex(void);
        // Pass related Actions
        void parsePass(void);
        void parseAmbient(void);
        void parseDiffuse(void);
        void parseSpecular(void);
        void parseEmissive(void);
        void parseDepthCheck(void);
        void parseDepthWrite(void);
        void parseDepthFunc(void);
        void parseAlphaRejection(void);
        void parseColourWrite(void);
        void parseCullHardware(void);
        void parseCullSoftware(void);
        void parseLighting(void);
        void parseMaxLights(void);
        void parseShading(void);
        void parsePointSize(void);
        void parsePointSprites(void);
        void parsePointSizeMin(void);
        void parsePointSizeMax(void);
        void parsePointSizeAttenuation(void);

        void parseTextureUnit(void);
        void parseTextureCustomParameter(void);
    	void finishProgramDefinition(void);

    };
}

#endif

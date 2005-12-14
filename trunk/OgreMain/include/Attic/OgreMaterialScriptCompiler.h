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

    class MaterialScriptCompiler : public Compiler2Pass
    {

    public:
        MaterialScriptCompiler(void);
        ~MaterialScriptCompiler(void);

    protected:
	    // Token ID enumeration
	    enum TokenID {
		    // Terminal Tokens section
            ID_UNKOWN = 0, ID_OPENBRACE, ID_CLOSEBRACE,
            ID_VERTEX_PROGRAM, ID_FRAGMENT_PROGRAM,
            // material
            ID_MATERIAL, ID_CLONE, ID_TECHNIQUE, ID_SET_TEXTURE_ALIAS, ID_LOD_DISTANCES,
            ID_RECEIVE_SHADOWS, ID_TRANSPARENCY_CASTS_SHADOWS, ID_LOD_INDEX,
            
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
            ID_LIGHTING, ID_FOG_OVERRIDE,
            ID_TEXTURE_UNIT,

            // general
            ID_ON, ID_OFF, ID_TRUE, ID_FALSE
        };

        /* deprecated: rules are now written in BNF text format and will be compiled from string into the rule base


    		// non-terminal tokens section
            mid_SCRIPT, mid_SCRIPT_OPTIONS, mid_SCRIPT_PROPERTIES, mid_MATERIAL_DEF, mid_VERTEX_PROGRAM_DEF,
            mid_FRAGMENT_PROGRAM_DEF, mid_MATERIAL_CLONE, mid_MATERIAL_OPTIONS, mid_MATERIAL_PROPERTIES,

            mid_TECHNIQUE_DEF, mid_SET_TEXTURE_ALIAS_DEF, mid_LOD_DISTANCES_DEF, mid_RECEIVE_SHADOWS_DEF,
            mid_TRANSPARENCY_CASTS_SHADOWS_DEF, mid_TECHNIQUE_OPTIONS, mid_TECHNIQUE_PROPERTIES,

            mid_PASS_DEF, mid_PASS_OPTIONS, mid_LOD_INDEX_DEF, mid_PASS_PROPERTIES, mid_AMBIENT_DEF, mid_DIFFUSE_DEF, 
			mid_EMISSIVE_DEF, mid_SPECULAR_DEF, mid_SPECULAR_PARAMS, mid_VERTEXCOLOUR_DEF, mid_COLOUR_PARAM_DEF,
            mid_SCENE_BLEND_DEF, mid_SIMPLE_BLEND, mid_USER_BLEND, mid_BLEND_FACTOR,
            mid_DEPTH_CHECK_DEF, mid_DEPTH_WRITE_DEF, mid_DEPTH_FUNC_DEF,
            mid_ALPHA_REJECTION_DEF, mid_COMPARE_FUNC_DEF, mid_FOG_PARAMETERS_DEF,
            mid_COLOUR_WRITE_DEF, mid_CULL_HARDWARE_DEF, mid_CULL_SOFTWARE_DEF, 
            mid_LIGHTING_DEF, mid_FOG_OVERRIDE_DEF, mid_SHADING_DEF, mid_DEPTH_BIAS_DEF, mid_TEXTURE_UNIT_DEF, 
            mid_TUS_OPTIONS, mid_TUS_PROPERTIES, mid_VERTEX_PROGRAM_REF_DEF, mid_SHADOW_CASTER_VERTEX_PROGRAM_REF_DEF,
            mid_SHADOW_RECEIVER_VERTEX_PROGRAM_REF_DEF, mid_FRAGMENT_PROGRAM_REF_DEF,
            mid_MAX_LIGHTS_DEF, mid_ITERATION_DEF, 

            mid_ON_OFF, mid_TRUE_FALSE_DEF,
            mid_LABEL, mid_QUOTED_LABEL, mid_UNQUOTED_LABEL, mid_ALPHANUMERIC_SPACE,
            mid_ALPHANUMERIC, mid_CHARACTER, mid_NUMBER,

            // system token base
            mid_SYSTEM
        };
        */

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
		    MaterialScriptProgramDefinition* programDef; // this is used while defining a program

		    uint techLev,	//Keep track of what tech, pass, and state level we are in
			    passLev,
			    stateLev;
            StringVector defaultParamLines;

		    // Error reporting state
            size_t lineNo;
            String filename;
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
        */
        TokenActionMap mTokenActionMap;

        /** Execute an Action associated with a token.  Gets called when the compiler finishes tokenizing a
            section of the source that has been parsed.
        **/
        virtual void executeTokenAction(const size_t tokenID);
        /** Associate all the lexemes used in a material script with their corresponding tokens and actions.
        **/
        void initTokenActions(void);
        void addLexemeTokenAction(const String& lexeme, const size_t token, const MSC_Action action = 0);

        void logParseError(const String& error);

        // Token Actions which get called when tokens are created during parsing.
        void parseOpenBrace(void);
        void parseCloseBrace(void);
        void parseVertexProgram(void);
        void parseFragmentProgram(void);
        void parseMaterial(void);
        // Technique related actions
        void parseTechnique(void);
        void parseTransparencyCastsShadows(void);
        void parseReceiveShadows(void);
        // Pass related Actions
        void parsePass(void);
        ColourValue _parseColourValue(void);
        void parseAmbient(void);
        void parseDiffuse(void);
        void parseSpecular(void);
        void parseEmissive(void);

        void parseTextureUnit(void);
        void parseTextureCustomParameter(void);
    	void finishProgramDefinition(void);

    };
}

#endif

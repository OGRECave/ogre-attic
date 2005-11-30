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

namespace Ogre {

    //-----------------------------------------------------------------------
    String MaterialScriptCompiler::materialScript_BNF =
        "<Script> ::= {<Script_Properties>}; \n"

        "<Script_Properties> ::= <Material_Def> | <Vertex_Program_Def> | <Fragment_Program_Def>; \n"

        "<Material_Def> ::= 'material' <Label> [<Material_Clone>] '{' {<Material_Properties>} '}'; \n"

        "<Material_Properties> ::= <Technique_Def> | <Set_Texture_Alias_Def> | "
        "                          <Lod_Distances_Def> | <Receive_Shadows_Def> | "
        "                          <transparency_casts_shadows_def>; \n"

        "    <Material_Clone> ::= ':' <Label>; \n"
        "    <Set_Texture_Alias_Def> ::= 'set_texture_alias' <Label> <Label>; \n"
        "    <Lod_Distances_Def> ::= 'lod_distances' <value> {<value>}; \n"
        "    <Receive_Shadows_Def> ::= 'receive_shadows' <On_Off>; \n"
        "    <transparency_casts_shadows_def> ::= 'transparency_casts_shadows' <On_Off>; \n"

        // Technique section rules
        "<Technique_Def> ::= 'technique' [<Label>] '{' {<Technique_Properties>} '}'; \n"
        "    <Technique_Properties> ::= <Pass_Def> | <Lod_Index_Def>; \n"
        "    <Lod_Index_Def> ::= 'lod_index' <value>; \n"

            // Pass section rules
        "    <Pass_Def> ::= 'pass' [<Label>] '{' {<Pass_Properties>} '}'; \n"
        "        <Pass_Properties> ::= <Ambient_Def> | <Diffuse_Def> | <Specular_Def> | <Emissive_Def>; \n"
        "                              <Scene_Blend_Def> | <Depth_Check_Def> | <Depth_Write_Def> | "
        "                              <Depth_Func_Def> | <Colour_Write_Def> | <Cull_Hardware_Def> | "
        "                              <Cull_Software_Def> | <Lighting_Def> | <Shading_Def> | "

        "        <Ambient_Def> ::= 'ambient' <Colour_Param_Def> | <Vertexcolour_Def>; \n"
        "        <Diffuse_Def> ::= 'diffuse' <Colour_Param_Def> | <Vertexcolour_Def>; \n"
        "        <Specular_Def> ::= 'specular' <Specular_Params> | <Vertexcolour_Def> <value>; \n"
        "            <Specular_Params> ::= <value> <value> <value> <value> [<value>]; \n"
        "        <Emissive_Def> ::= 'emissive' <Colour_params> | <Vertexcolour_Def>; \n"

        "        <Vertexcolour> ::= 'vertexcolour'; \n"

        "        <Scene_Blend_Def> ::= 'scene_blend' <Simple_Blend> | <User_Blend>; \n"
        "          <Simple_Blend> ::= 'add' | 'modulate' | 'colour_blend' | 'alpha_blend'; \n"
        "          <Blend_Factor> ::= 'one' | 'zero' | 'dest_colour' | 'src_colour' | \n"
        "                             'one_minus_dest_colour' | 'one_minus_src_colour' | \n"
        "                             'dest_alpha' | 'src_alpha' | 'one_minus_dest_alpha' | \n"
        "                             'one_minus_src_alpha'; \n"


        "<Label> ::= <Quoted_Label> | <Unquoted_Label>; \n"
        "<Quoted_Label> ::= '\"' <Character> {<Alphanumeric_Space>} '\"'; \n"
        "<Alphanumeric_Space> ::= <Alphanumeric> | ' '; \n"
        "<Alphanumeric> ::= <Character> | <Number>; \n"
        "<Character> ::= (abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$#%!_*&\\/); \n"
        "<Number> ::= (0123456789); \n"
        ;
    /* deprecated: rules are now written in BNF text format and will be compiled from string into the rule base
    MaterialScriptCompiler::TokenRule MaterialScriptCompiler::materialScript_RulePath[] = {

        // <Script> ::= {<Script_Properties>}
        _rule_ mid_SCRIPT, "<Script>"
	        _repeat_ mid_SCRIPT_PROPERTIES _nt_
	        _end_
        	
        // <Script_Properties> ::= <Material_Def> | <Vertex_Program_Def> | <Fragment_Program_Def>
        _rule_ mid_SCRIPT_PROPERTIES, "<Script_Properties>"
	        _is_ mid_MATERIAL_DEF _nt_
	        _or_ mid_VERTEX_PROGRAM_DEF _nt_
	        _or_ mid_FRAGMENT_PROGRAM_DEF _nt_
	        _end_
        	
        // <Material_Def> ::= material <Label> [<Material_Clone>] "{" {<Material_Properties>} "}" 
        _rule_ mid_MATERIAL_DEF, "<Material_Def>"
	        _is_ mid_MATERIAL, "material"
	        _and_ mid_LABEL _nt_
	        _optional_ mid_MATERIAL_CLONE _nt_
	        _and_ _no_token_, "{"
            _repeat_ mid_MATERIAL_PROPERTIES _nt_
	        _and_ _no_token_, "}"
	        _end_

        // <Material_Properties> ::= <Technique_Def> | <Set_Texture_Alias_Def> |
        //                           <Lod_Distances_Def> | <Receive_Shadows_Def> |
        //                           <transparency_casts_shadows_def>
        _rule_ mid_MATERIAL_PROPERTIES, "<Material_Properties>"
	        _is_ mid_TECHNIQUE_DEF _nt_
	        _or_ mid_SET_TEXTURE_ALIAS_DEF _nt_
            _or_ mid_LOD_DISTANCES_DEF _nt_
            _or_ mid_RECEIVE_SHADOWS_DEF _nt_
            _or_ mid_TRANSPARENCY_CASTS_SHADOWS_DEF _nt_
            _end_

        // <Material_Clone> ::= ":" <Label>
        _rule_ mid_MATERIAL_CLONE, "<Material_Clone>"
	        _is_ mid_CLONE, ":"
	        _and_ mid_LABEL _nt_
	        _end_
        	
        // <Set_Texture_Alias_Def> ::= "set_texture_alias"  <Label> <Label>
        _rule_ mid_SET_TEXTURE_ALIAS_DEF, "<Set_Texture_Alias_Def>"
            _is_ mid_SET_TEXTURE_ALIAS, "set_texture_alias"
            _and_ mid_LABEL _nt_
            _and_ mid_LABEL _nt_
            _end_

        // <Lod_Distances_Def> ::= "lod_distances" <value> {<value>}
        _rule_ mid_LOD_DISTANCES_DEF, "<Lod_Distances_Def>"
            _is_ mid_LOD_DISTANCES, "lod_distances"
            _and_ _value_, ""
            _repeat_ _value_, ""
            _end_

        // <Receive_Shadows_Def> ::= "receive_shadows" <On_Off>
        _rule_ mid_RECEIVE_SHADOWS_DEF, "<Receive_Shadows_Def>"
            _is_ mid_RECEIVE_SHADOWS, "receive_shadows"
            _and_ mid_ON_OFF _nt_
            _end_

        // <transparency_casts_shadows_def> ::= "transparency_casts_shadows" <On_Off>
        _rule_ mid_TRANSPARENCY_CASTS_SHADOWS_DEF, "<transparency_casts_shadows_def>"
            _is_ mid_TRANSPARENCY_CASTS_SHADOWS, "transparency_casts_shadows"
            _and_ mid_ON_OFF _nt_
            _end_

        // <Technique_Def> ::= "technique" [<Label>] "{" {<Technique_Properties>} "}"
        _rule_ mid_TECHNIQUE_DEF, "<Technique Def>"
	        _is_ mid_TECHNIQUE, "technique"
	        _optional_ mid_LABEL _nt_
	        _and_ _no_token_, "{"
	        _repeat_ mid_TECHNIQUE_PROPERTIES _nt_
	        _and_ _no_token_, "}"
	        _end_
        	
        // <Technique_Properties> ::= <Pass_Def> | <Lod_Index_Def>
        _rule_ mid_TECHNIQUE_PROPERTIES, "<Technique Properties>"
            _is_ mid_PASS_DEF _nt_
            _or_ mid_LOD_INDEX_DEF _nt_
            _end_

        // <Lod_Index_Def> ::= lod_index <value>
        _rule_ mid_LOD_INDEX_DEF, "<Lod_Index_Def>"
            _is_ mid_LOD_INDEX, "lod_index"
            _and_ _value_, ""
            _end_

        // <Pass_Def> ::= "pass" [<Label>] "{" {<Pass_Properties>} "}"
        _rule_ mid_PASS_DEF, "<Pass_Def>"
	        _is_ mid_PASS, "pass"
	        _optional_ mid_LABEL _nt_
	        _and_ _no_token_, "{"
	        _repeat_ mid_PASS_PROPERTIES _nt_
	        _and_ _no_token_, "}"
	        _end_
        	
        // <Pass_Properties> ::= <Ambient_Def> | <Diffuse_Def> | <Specular_Def> | <Emissive_Def> |
        //                       <Scene_Blend_Def> | <Depth_Check_Def> | <Depth_Write_Def> | 
        //                       <Depth_Func_Def> | <Colour_Write_Def> | <Cull_Hardware_Def> |
        //                       <Cull_Software_Def> | <Lighting_Def> | <Shading_Def> | 
        _rule_ mid_PASS_PROPERTIES, "<Pass_Properties>"
            _is_ mid_AMBIENT_DEF _nt_
            _or_ mid_DIFFUSE_DEF _nt_
            _or_ mid_SPECULAR_DEF _nt_
            _or_ mid_EMISSIVE_DEF _nt_
            _or_ mid_SCENE_BLEND_DEF _nt_
            _or_ mid_DEPTH_CHECK_DEF _nt_
            _or_ mid_DEPTH_WRITE_DEF _nt_
            _or_ mid_DEPTH_FUNC_DEF _nt_
            _or_ mid_ALPHA_REJECTION_DEF _nt_
            _or_ mid_COLOUR_WRITE_DEF _nt_
            _or_ mid_CULL_HARDWARE_DEF _nt_
            _or_ mid_CULL_SOFTWARE_DEF _nt_
            _or_ mid_LIGHTING_DEF _nt_
            _or_ mid_FOG_OVERRIDE_DEF _nt_
            _or_ mid_SHADING_DEF _nt_
            _or_ mid_DEPTH_BIAS_DEF _nt_
            _or_ mid_TEXTURE_UNIT_DEF _nt_
            _or_ mid_VERTEX_PROGRAM_REF_DEF _nt_
            _or_ mid_SHADOW_CASTER_VERTEX_PROGRAM_REF_DEF _nt_
            _or_ mid_SHADOW_RECEIVER_VERTEX_PROGRAM_REF_DEF _nt_
            _or_ mid_FRAGMENT_PROGRAM_REF_DEF _nt_
            _or_ mid_MAX_LIGHTS_DEF _nt_
            _or_ mid_ITERATION_DEF _nt_
            _end_

        // <Ambient_Def> ::= "ambient" <Colour_Param_Def> | <Vertexcolour_Def>
        _rule_ mid_AMBIENT_DEF, "<Ambient_Def>"
            _is_ mid_AMBIENT, "ambient"
            _and_ mid_COLOUR_PARAM_DEF _nt_
            _or_ mid_VERTEXCOLOUR_DEF _nt_
            _end_

        // <Diffuse_Def> ::= "diffuse" <Colour_Param_Def> | <Vertexcolour_Def>
        _rule_ mid_DIFFUSE_DEF, "<Diffuse_Def>"
            _is_ mid_DIFFUSE, "diffuse"
            _and_ mid_COLOUR_PARAM_DEF _nt_
            _or_ mid_VERTEXCOLOUR_DEF _nt_
            _end_

        // <Specular_Def> ::= "specular" <Specular_Params> | <Vertexcolour_Def> <value>
        _rule_ mid_SPECULAR_DEF, "<Specular_Def>"
            _is_ mid_SPECULAR, "specular"
            _and_ mid_SPECULAR_PARAMS _nt_
            _or_ mid_VERTEXCOLOUR_DEF _nt_
            _and_ _value_, ""
            _end_

        // <Specular_Params> ::= <value> <value> <value> <value> [<value>]
        _rule_ mid_SPECULAR_PARAMS, "<Specular_Params>"
            _is_ _value_, ""
            _and_ _value_, ""
            _and_ _value_, ""
            _and_ _value_, ""
            _or_ _value_, ""
            _end_

        // <Emissive_Def> ::= "emissive" <Colour_params> | <Vertexcolour_Def>
        _rule_ mid_EMISSIVE_DEF, "<Emissive_Def>"
            _is_ mid_EMISSIVE, "emissive"
            _and_ mid_COLOUR_PARAM_DEF _nt_
            _or_ mid_VERTEXCOLOUR_DEF _nt_
            _end_


        // <Vertexcolour> ::= "vertexcolour"
        _rule_ mid_VERTEXCOLOUR_DEF, "<Vertexcolour_Def>"
            _is_ mid_VERTEXCOLOUR, "vertexcolour"
            _end_

        // <Scene_Blend_Def> ::= "scene_blend" <Simple_Blend> | <User_Blend>
        _rule_ mid_SCENE_BLEND_DEF, "<Scene_Blend_Def>"
            _is_ mid_SCENE_BLEND, "scene_blend"
            _and_ mid_SIMPLE_BLEND _nt_
            _or_ mid_USER_BLEND _nt_
            _end_

        // <Simple_Blend> ::= "add" | "modulate" | "colour_blend" | "alpha_blend"
        _rule_ mid_SIMPLE_BLEND, "<Simple_Blend>"
            _is_ mid_BLEND_ADD, "add"
            _or_ mid_BLEND_MODULATE, "modulate"
            _or_ mid_COLOUR_BLEND, "colour_blend"
            _or_ mid_ALPHA_BLEND, "alpha_blend"
            _end_

        // <User_Blend> ::= <Blend_Factor> <Blend_Factor>
        _rule_ mid_USER_BLEND, "<User_Blend>"
            _is_ mid_BLEND_FACTOR _nt_
            _and_ mid_BLEND_FACTOR _nt_
            _end_

        // <Blend_Factor> ::= "one" | "zero" | "dest_colour" | "src_colour" |
        //                   "one_minus_dest_colour" | "one_minus_src_colour" |
        //                   "dest_alpha" | "src_alpha" | "one_minus_dest_alpha" |
        //                   "one_minus_src_alpha"
        _rule_ mid_BLEND_FACTOR, "<Blend_Factor>"
            _is_ mid_BLEND_ONE, "one"
            _or_ mid_BLEND_ZERO, "zero"
            _or_ mid_BLEND_DEST_COLOUR, "dest_colour"
            _or_ mid_BLEND_SRC_COLOUR, "src_colour"
            _or_ mid_BLEND_ONCE_MINUS_DEST_COLOUR, "one_minus_dest_colour"
            _or_ mid_BLEND_ONE_MINUS_SRC_COLOUR, "one_minus_src_colour"
            _or_ mid_BLEND_DEST_ALPHA, "dest_alpha"
            _or_ mid_BLEND_SRC_ALPHA, "src_alpha"
            _or_ mid_BLEND_ONE_MINUS_DEST_ALPHA, "one_minus_dest_alpha"
            _or_ mid_BLEND_ONE_MINUS_SRC_ALPHA, "one_minus_src_alpha"
            _end_

        // <Depth_Check_Def> ::= "depth_check" <On_Off>
        _rule_ mid_DEPTH_CHECK_DEF, "<Depth_Check_Def>"
            _is_ mid_DEPTH_CHECK, "depth_check"
            _and_ mid_ON_OFF _nt_
            _end_

        // <Depth_Write_Def> ::= "depth_write" <On_Off>
        _rule_ mid_DEPTH_WRITE_DEF, "<Depth_Write_Def>"
            _is_ mid_DEPTH_WRITE, "depth_write"
            _and_ mid_ON_OFF _nt_
            _end_

        // <Depth_Func_Def> ::= "depth_func" <Compare_Func_Def>
        _rule_ mid_DEPTH_FUNC_DEF, "<Depth_Func_Def>"
            _is_ mid_DEPTH_FUNC, "depth_func"
            _and_ mid_COMPARE_FUNC_DEF _nt_
            _end_
        
        // <Alpha_Rejection_Def> ::= "alpha_rejection" <Compare_Func_Def> <value>
        _rule_ mid_ALPHA_REJECTION_DEF, "<Alpha_Rejection_Def>"
            _is_ mid_ALPHA_REJECTION, "alpha_rejection"
            _and_ mid_COMPARE_FUNC_DEF _nt_
            _and_ _value_, ""
            _end_

        // <Compare_Func_Def> ::= "always_fail" | "always_pass" | "less_equal" | "less" |
        //                      "equal" | "not_equal" | "greater_equal" | "greater"
        _rule_ mid_COMPARE_FUNC_DEF, "<Compare_Func_Def>"
            _is_ mid_ALWAYS_FAIL, "always_fail"
            _or_ mid_ALWAYS_PASS, "always_pass"
            _or_ mid_LESS_EQUAL, "less_equal"
            _or_ mid_LESS, "less"
            _or_ mid_EQUAL, "equal"
            _or_ mid_NOT_EQUAL, "not_equal"
            _or_ mid_GREATER_EQUAL, "greater_equal"
            _or_ mid_GREATER, "greater"
            _end_

        // <Colour_Write_Def> ::= "colour_write" <On_Off>
        _rule_ mid_COLOUR_WRITE_DEF, "<Colour_Write_Def>"
            _is_ mid_COLOUR_WRITE, "colour_write"
            _and_ mid_ON_OFF _nt_
            _end_

        // <Cull_Hardware_Def> ::= "cull_hardware" "clockwise" | "anticlockwise" | "none"
        _rule_ mid_CULL_HARDWARE_DEF, "<Cull_Hardware_Def>"
            _is_ mid_CULL_HARDWARE, "cull_hardware"
            _and_ mid_CLOCKWISE, "clockwise"
            _or_ mid_ANTICLOCKWISE, "anticlockwise"
            _or_ mid_CULL_NONE, "none"
            _end_

        // <Cull_Software_Def> ::= "cull_software" "back" | "front" | "none"
        _rule_ mid_CULL_SOFTWARE_DEF, "<Cull_Software_Def>"
            _is_ mid_CULL_SOFTWARE, "cull_software"
            _and_ mid_CULL_BACK, "back"
            _or_ mid_CULL_FRONT, "front"
            _or_ mid_CULL_NONE, "none"
            _end_

        // <Lighting_Def> ::= "lighting" <On_Off>
        _rule_ mid_LIGHTING_DEF, "<Lighting_Def>"
            _is_ mid_LIGHTING, "lighting"
            _and_ mid_ON_OFF _nt_
            _end_

        // <Shading_Def> ::= "shading" "flat" | "gouraud" | "phong"
        _rule_ mid_SHADING_DEF, "<Shading_Def>"
            _is_ mid_SHADING, "shading"
            _and_ mid_FLAT, "flat"
            _or_ mid_GOURAUD, "gouraud"
            _or_ mid_PHONG, "phong"
            _end_

        // <Fog_Override_Def> ::= "fog_override" <True_False> [<Fog_parameters>]
        _rule_ mid_FOG_OVERRIDE_DEF, "<Fog_Override_Def>"
            _is_ mid_FOG_OVERRIDE, "fog_override"
            _and_ mid_TRUE_FALSE_DEF _nt_
            _optional_ mid_FOG_PARAMETERS_DEF _nt_
            _end_

        // <Fog_parameters> ::= <type> <colour> <density> <start> <end>

        // <True_False> ::= "true" | "false"
        _rule_ mid_TRUE_FALSE_DEF, "<True_False>"
            _is_ mid_TRUE, "true"
            _or_ mid_FALSE, "false"
            _end_

        // <On_Off> ::= "on" | "off"
        _rule_ mid_ON_OFF, "<On_Off>"
            _is_ mid_ON, "on"
            _or_ mid_OFF, "off"
            _end_

        // <Colour_params> ::= <value> <value> <value> [<value>]
        _rule_ mid_COLOUR_PARAM_DEF, "<Colour_Param_Def>"
            _is_ _value_, ""
            _and_ _value_, ""
            _and_ _value_, ""
            _optional_ _value_, ""
            _end_

        // <Texture_Unit_Def> ::= "texture_unit" [<Label>] "{" {<TUS_Properties>} "}"
        _rule_ mid_TEXTURE_UNIT_DEF, "<Texture_Unit_Def>"
            _is_ mid_TEXTURE_UNIT, "texture_unit"
            _optional_ mid_LABEL _nt_
            _and_ _no_token_, "{"
            _repeat_ mid_TUS_PROPERTIES _nt_
            _and_ _no_token_, "}"
            _end_

        _rule_ mid_TUS_PROPERTIES, "<TUS_Properties>"
            _end_

        // <Label> ::= <Quoted_Label> | <Unquoted_Label>
        _rule_ mid_LABEL, "<Label>"
	        _is_ mid_QUOTED_LABEL _nt_
	        _or_ mid_UNQUOTED_LABEL _nt_
	        _end_

        // <Quoted_Label> ::= """ <Character> {<Alphanumeric_Space>} """
        _rule_ mid_QUOTED_LABEL, "<Quoted_Label>"
	        _is_ _no_token_, "\""
            _and_ mid_CHARACTER _nt_
	        _repeat_ mid_ALPHANUMERIC_SPACE
	        _and_ _no_token_, "\""
	        _end_

        // <Unquoted_Label> ::= <Character> {<Alphanumeric>}
        _rule_ mid_UNQUOTED_LABEL, "<Unquoted_Label>"
            _is_ mid_CHARACTER _nt_
	        _repeat_ mid_ALPHANUMERIC
	        _end_

        // <Alphanumeric_Space> ::= <Alphanumeric> | " "
        _rule_ mid_ALPHANUMERIC_SPACE, "<Alphanumeric_Space>"
	        _is_ mid_ALPHANUMERIC _nt_
	        _or_ _character_, " "
	        _end_
        	
        // <Alphanumeric> ::= <Character> | <Number>
        _rule_ mid_ALPHANUMERIC, "<Alphanumeric>"
	        _is_ mid_CHARACTER _nt_
	        _or_ mid_NUMBER _nt_
	        _end_
        	
        // <Character> ::= [abcdefghijklmnopqrstuvwxyz] | [ABCDEFGHIJKLMNOPQRSTUVWXYZ] | [$#%!_*&\/]
        _rule_ mid_CHARACTER, "<Character>"
	        _is_ _character_, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$#%!_*&\\/"
	        _end_
     
        // <Number> ::= (0123456789)
        _rule_ mid_NUMBER, "<Number>"
	        _is_ _character_, "0123456789"
	        _end_

    };
    */
    //-----------------------------------------------------------------------

}

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreStableHeaders.h"
#include "OgreMaterialScriptCompiler.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreBlendMode.h"
#include "OgreGpuProgram.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreMaterialManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreExternalTextureSourceManager.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    // Static definitions
    //-----------------------------------------------------------------------
    MaterialScriptCompiler::TokenActionMap MaterialScriptCompiler::mTokenActionMap;

    //-----------------------------------------------------------------------
    const String& MaterialScriptCompiler::getClientBNFGrammer(void) const
    {
        // simplified Backus - Naur Form (BNF) grammer for material scripts
        static const String materialScript_BNF =
        "<Script> ::= {<Script_Properties>} \n"

        "<Script_Properties> ::= <Material> | <Vertex_Program> | <Fragment_Program> \n"

        "<Material> ::= 'material' <Flex_Label> [<Material_Clone>] '{' {<Material_Properties>} '}' \n"

        "<Material_Properties> ::= <Technique> | <Set_Texture_Alias> | "
        "                          <Lod_Distances> | <Receive_Shadows> | "
        "                          <Transparency_Casts_Shadows> \n"

        "    <Material_Clone> ::= ':' <Flex_Label> \n"
        "    <Set_Texture_Alias> ::= 'set_texture_alias' <Label> [<Seperator>] <Label> \n"
        "    <Lod_Distances> ::= 'lod_distances' <#distance> {<#distance>} \n"
        "    <Receive_Shadows> ::= 'receive_shadows' <On_Off> \n"
        "    <Transparency_Casts_Shadows> ::= 'transparency_casts_shadows' <On_Off> \n"

        // Technique section rules
        "<Technique> ::= 'technique' [<Label>] '{' {<Technique_Properties>} '}' \n"
        "    <Technique_Properties> ::= <Pass> | <Lod_Index> | <Scheme> \n"
        "    <Lod_Index> ::= 'lod_index' <#value> \n"
        "    <Scheme> ::= 'scheme' <Label> \n"

        // Pass section rules
        "    <Pass> ::= 'pass' [<Label>] '{' {<Pass_Properties>} '}' \n"
        "        <Pass_Properties> ::= <Ambient> | <Diffuse> | <Specular> | <Emissive> | \n"
        "                              <Scene_Blend> | <Depth_Check> | <Depth_Write> | \n"
        "                              <Texture_Unit> | \n"
        "                              <Depth_Func> | <Depth_Bias> | <Alpha_Rejection> | \n"
        "                              <Cull_Hardware> | <Cull_Software> | <Lighting> | \n"
        "                              <GPU_Program_Ref> | \n"
        "                              <Shading> | <PolygonMode> | <Fog_Override> | <Colour_Write> | \n"
		"                              <Max_Lights> | <Start_Light> | <Iteration> | \n"
		"                              <Point_Sprites> | <Point_Size_Attenuation> | \n"
		"                              <Point_Size_Min> | <Point_Size_Max> | <Point_Size> \n"

        "        <Ambient> ::= 'ambient' <ColourOptions> \n"
        "        <Diffuse> ::= 'diffuse' <ColourOptions> \n"
        "        <Specular> ::= 'specular' <SpecularOptions> \n"
        "           <SpecularOptions> ::= <Specular_Colour_Params> | <Specular_Vertex> \n"
        "           <Specular_Colour_Params> ::= <#red> <#green> <#blue> <#val> [<#val>] \n"
        "           <Specular_Vertex> ::= 'vertexcolour' <#shininess> \n"
        "        <Emissive> ::= 'emissive' <ColourOptions> \n"

        "        <ColourOptions> ::= <Colour_Params> | 'vertexcolour' \n"

        "        <Scene_Blend> ::= 'scene_blend' <SceneBlend_Options> \n"
        "          <SceneBlend_Options> ::= <Simple_Blend> | <User_Blend> \n"
        "            <Simple_Blend> ::= <Base_Blend> | 'colour_blend' \n"
        "            <Base_Blend> ::= 'alpha_blend' | 'modulate' | 'add' \n"
        "            <User_Blend> ::= <Blend_Factor> <Blend_Factor> \n"
        "            <Blend_Factor> ::= 'dest_colour' | 'src_colour' | \n"
        "                               'one_minus_dest_colour' | 'one_minus_src_colour' | \n"
        "                               'dest_alpha' | 'src_alpha' | 'one_minus_dest_alpha' | \n"
        "                               'one_minus_src_alpha' | 'one' | 'zero' \n"

        "        <Depth_Check> ::= 'depth_check' <On_Off> \n"
        "        <Depth_Write> ::= 'depth_write' <On_Off> \n"
        "        <Depth_Func> ::= 'depth_func' <Compare_Func> \n"
		"        <Depth_Bias> ::= 'depth_bias' <#constant> [<#slopescale>] \n"
        "        <Alpha_Rejection> ::= 'alpha_rejection' <Compare_Func> <#value> \n"
        "        <Compare_Func> ::= 'always_fail' | 'always_pass' | 'less_equal' | 'less' | \n"
        "                           'equal' | 'not_equal' | 'greater_equal' | 'greater' \n"
        "        <Cull_Hardware> ::= 'cull_hardware' <Cull_Hardware_Otions> \n"
        "           <Cull_Hardware_Otions> ::= 'clockwise' | 'anticlockwise' | 'none' \n"
        "        <Cull_Software> ::= 'cull_software' <Cull_Software_Otions> \n"
        "           <Cull_Software_Otions> ::= 'back' | 'front' | 'none' \n"
        "        <Lighting> ::= 'lighting' <On_Off> \n"
        "        <Shading> ::= 'shading' <Shading_Options> \n"
        "           <Shading_Options> ::= 'flat' | 'gouraud' | 'phong' \n"
        "        <PolygonMode> ::= 'polygon_mode' <PolygonMode_Options> \n"
        "           <PolygonMode_Options> ::= 'solid' | 'wireframe' | 'points' \n"
        "        <Colour_Write> ::= 'colour_write' <On_Off> \n"
		"        <Point_Size> ::= 'point_size' <#size> \n"
		"        <Point_Sprites> ::= 'point_sprites' <On_Off> \n"
		"        <Point_Size_Min> ::= 'point_size_min' <#size> \n"
		"        <Point_Size_Max> ::= 'point_size_max' <#size> \n"
		"        <Point_Size_Attenuation> ::= 'point_size_attenuation' <On_Off> [<Point_Size_Att_Params>] \n"
		"            <Point_Size_Att_Params> ::= <#constant> <#linear> <#quadric> \n"
        "        <Fog_Override> ::= 'fog_override' <Fog_Override_Options> \n"
        "           <Fog_Override_Options> ::= 'false' | <fog_true> \n"
        "             <fog_true> ::= 'true' [<Fog_True_Params>] \n"
        "               <Fog_True_Params> ::= 'none' | <fog_True_Param_Option> \n"
        "                   <fog_True_Param_Option> ::= <fog_type> <#red> <#green> <#blue> <#fog_density> <#start> <#end> \n"
        "                       <fog_type> ::= 'linear' | 'exp2' | 'exp' \n"
        "        <Max_Lights> ::= 'max_lights' <#number> \n"
		"        <Start_Light> ::= 'start_light' <#number> \n"
        "        <Iteration> ::= 'iteration' <Iteration_Options> \n"
        "           <Iteration_Options> ::= <Iteration_Once_Params> | 'once' | <Iteration_Counted> \n"
        "             <Iteration_Once_Params> ::= 'once_per_light' [<light_type>] \n"
        "             <Iteration_Counted> ::= <#number> [<Per_Light_Options>] \n"
		"               <Per_Light_Options> ::= <Per_Light> | <Per_N_Lights> \n"
		"                 <Per_Light> ::= 'per_light' [<light_type>] \n"
		"                 <Per_N_Lights> ::= 'per_n_lights' <#num_lights> [<light_type>] \n"
        "             <light_type> ::= 'point' | 'directional' | 'spot' \n"
        // Texture Unit section rules
        "        <Texture_Unit> ::= 'texture_unit' [<Label>] '{' {<TUS_Properties>} '}' \n"
        "        <TUS_Properties> ::= <Texture_Alias> | <Texture> | <Anim_Texture> | <Cubic_Texture> | \n"
        "                             <Tex_Coord_Set> | <Tex_Address_Mode> | <Tex_Border_Colour> | <Filtering> | \n"
        "                             <Max_Anisotropy> | <MipMap_Bias> | <Colour_Op_Ex> | <Colour_Op_Multipass_Fallback> | <Colour_Op> | \n"
        "                             <Alpha_Op_Ex> | <Env_Map> | <Scroll_Anim> | <Scroll> | <Rotate_Anim> | <Rotate> | \n"
		"                             <Scale> | <Wave_Xform> | <Transform> | <Binding_Type> | <Content_Type> \n"
        "           <Texture_Alias> ::= 'texture_alias' <Label> \n"
        "           <Texture> ::= 'texture' <Label> {<Texture_Properties>} \n"
        "           <Texture_Properties> ::= '1d' | '2d' | '3d' | 'cubic' | 'unlimited' | 'alpha' | <#mipmap> \n"
        "                                    | " + PixelUtil::getBNFExpressionOfPixelFormats(true) + " \n"
        "           <Anim_Texture> ::= 'anim_texture' <Label> <Anim_Texture_Properties> \n"
        "               <Anim_Texture_Properties> ::= <Numbered_Anim_Texture> | <Seperate_Anim_Textures> \n"
        "               <Numbered_Anim_Texture> ::= <#frames> <#duration> \n"
        "               <Seperate_Anim_Textures> ::= <anim_frame> {<anim_frame>} \n"
        "                   <anim_frame> ::= (?!<TUS_Terminators>) <Label> [<Seperator>] \n"
        "           <TUS_Terminators> ::= '}' | 'texture_alias' | 'texture' | 'anim_texture' | 'cubic_texture' | \n"
        "                                 'tex_coord_set' | 'tex_address_mode' | 'tex_border_colour' | \n"
        "                                 'filtering' | 'max_anisotropy' | 'mipmap_bias' | 'colour_op' | 'colour_op_ex' | \n"
        "                                 'colour_op_multipass_fallback' | 'alpha_op_ex' | 'env_map' | \n"
        "                                 'scroll' | 'rotate' | 'scale' | 'wave_xform' | 'transform' | 'binding_type' \n"
        "           <Cubic_Texture> ::= 'cubic_texture' <Label> <Cubic_Texture_Options> \n"
        "               <Cubic_Texture_Options> ::= 'combineduvw' | 'separateuv' | <Cubic_Seperate> \n"
        "               <Cubic_Seperate> ::= <Label> [<Seperator>] <Label> [<Seperator>] <Label> \n"
        "                                    [<Seperator>] <Label> [<Seperator>] <Label> 'separateuv' \n"
        "           <Tex_Coord_Set> ::= 'tex_coord_set' <#set_num> \n"
        "           <Tex_Address_Mode> ::= 'tex_address_mode' <UVW_Mode> [<UVW_Mode>] [<UVW_Mode>] \n"
        "               <UVW_Mode> ::= 'wrap' | 'clamp' | 'mirror' | 'border' \n"
        "           <Tex_Border_Colour> ::= 'tex_border_colour' <Colour_Params> \n"
        "           <Filtering> ::= 'filtering' <Filtering_Options>"
        "               <Filtering_Options> ::= <Simple_Filter> | <Complex_Filter> \n"
        "                 <Simple_Filter> ::= 'bilinear' | 'trilinear' | 'anisotropic' | 'none' \n"
        "                 <Complex_Filter> ::= <MinMagFilter> <MinMagFilter> <MipFilter> \n"
        "                   <MinMagFilter> ::= 'linear' | 'point' | 'anisotropic' \n"
        "                   <MipFilter> ::= 'linear' | 'point' | 'none' \n"
        "           <Max_Anisotropy> ::= 'max_anisotropy' <#val> \n"
        "           <MipMap_Bias> ::= 'mipmap_bias' <#val> \n"
        "           <Colour_Op> ::= 'colour_op' <Colour_Op_Options> \n"
        "               <Colour_Op_Options> ::= <Base_Blend> | 'replace' \n"
        "           <Colour_Op_Ex> ::= 'colour_op_ex' <Combine_Operation> <Source_Option> <Source_Option> {<#val>} \n"
        "               <Combine_Operation> ::= 'source1' | 'source2' | 'modulate_x2' | 'modulate_x4' | \n"
        "                                       'modulate' | 'add_signed' | 'add_smooth' | 'add' | \n"
        "                                       'subtract' | 'blend_diffuse_alpha' | 'blend_texture_alpha' | \n"
        "                                       'blend_current_alpha' | 'blend_manual' | 'dotproduct' | \n"
        "                                       'blend_diffuse_colour' \n"
        "               <Source_Option> ::= 'src_current' | 'src_texture' | 'src_diffuse' | \n"
        "                                   'src_specular' | 'src_manual' \n"
        "           <Colour_Op_Multipass_Fallback> ::= 'colour_op_multipass_fallback' <Blend_Factor> <Blend_Factor> \n"
        "           <Alpha_Op_Ex> ::= 'alpha_op_ex' <Combine_Operation> <Source_Option> <Source_Option> {<#val>} \n"
        "           <Env_Map> ::= 'env_map' <Env_Map_Option> \n"
        "               <Env_Map_Option> ::= 'spherical' | 'planar' | 'cubic_reflection' | 'cubic_normal' | 'off' \n"
        "           <Scroll> ::= 'scroll' <#x> <#y> \n"
        "           <Scroll_Anim> ::= 'scroll_anim' <#xspeed> <#yspeed> \n"
        "           <Rotate> ::= 'rotate' <#angle> \n"
        "           <Rotate_Anim> ::= 'rotate_anim' <#revs_per_second> \n"
        "           <Scale> ::= 'scale' <#x> <#y> \n"
        "           <Wave_Xform> ::= 'wave_xform' <Xform_Type> <Wave_Type> <#base> <#frequency> <#phase> <#amplitude> \n"
        "               <Xform_Type> ::= 'scroll_x' | 'scroll_y' | 'rotate' | 'scale_x' | 'scale_y' \n"
        "               <Wave_Type> ::= 'sine' | 'triangle' | 'square' | 'sawtooth' | 'inverse_sawtooth' \n"
        "           <Transform> ::= 'transform' <#m00> <#m01> <#m02> <#m03> <#m10> <#m11> <#m12> <#m13> <#m20> <#m21> <#m22> <#m23> \n"
        "                           <#m30> <#m31> <#m32> <#m33> \n"
        "           <Binding_Type> ::= 'binding_type' <Program_Type_Options> \n"
        "           <Program_Type_Options> ::= 'vertex' | 'fragment' \n"
		"			<Content_Type> ::= 'content_type' <Content_Type_Options> \n"
		"           <Content_Type_Options> ::= 'named' | 'shadow' \n"
        // GPU Programs
        " \n"
        "<Vertex_Program> ::= 'vertex_program' <Label> [<Seperator>] <Label> '{' {<Vertex_Program_Option>} '}' \n"
        "   <Vertex_Program_Option> ::= <Vertex_Program_Animation> | <Vertex_Texture_Fetch> | <GPU_Program_Options> \n"
        "   <Vertex_Program_Animation> ::= <Skeletal_Animation> | <Morph_Animation> | <Pose_Animation> \n"
        "       <Skeletal_Animation> ::= 'includes_skeletal_animation' <True_False> \n"
        "       <Morph_Animation> ::= 'includes_morph_animation' <True_False> \n"
        "       <Pose_Animation> ::= 'includes_pose_animation' <#val> \n"
        "       <Vertex_Texture_Fetch> ::= 'uses_vertex_texture_fetch' <True_False> \n"
        "<Fragment_Program> ::= 'fragment_program' <Label> [<Seperator>] <Label> '{' {<GPU_Program_Options>}'}' \n"
        // do custom parameters last since it will consume everything on the line in the source
        "   <GPU_Program_Options> ::= <Program_Source> | <Syntax> | <Default_Params> | <Custom_Parameter> \n"
        "       <Program_Source> ::= 'source' <Label> \n"
        "       <Syntax> ::= 'syntax' <Label> \n"
        "       <Default_Params> ::= 'default_params' '{' {<GPUParams_Option>} '}' \n"
        "       <Custom_Parameter> ::= 'custom_parameter' : <Unquoted_Label> [<Seperator>] <Spaced_Label> \n"

        "   <GPU_Program_Ref> ::= <GPU_Program_Ref_Type> [<Flex_Label>] '{' {<GPUParams_Option>} '}' \n"
        "       <GPU_Program_Ref_Type> ::= 'vertex_program_ref' | 'fragment_program_ref' | \n"
        "                                  'shadow_caster_vertex_program_ref' | \n"
        "                                  'shadow_receiver_vertex_program_ref' | \n"
        "                                  'shadow_receiver_fragment_program_ref' \n"

        "   <GPUParams_Option> ::= <Param_Named_Auto> | <Param_Named> | <Param_Indexed_Auto> | <Param_Indexed> \n"
        "       <Param_Named_Auto> ::= 'param_named_auto' <Unquoted_Label> [<Seperator>] <Unquoted_Label> [<#val>] \n"
        "       <Param_Named> ::= 'param_named' <Unquoted_Label> [<Seperator>] <Param_Value_Option> \n"
        "       <Param_Indexed_Auto> ::= 'param_indexed_auto' <#index> <Unquoted_Label> [<#val>] \n"
        "       <Param_Indexed> ::= 'param_indexed' <#index> <Param_Value_Option> \n"
        "       <Param_Value_Option> ::= <Unquoted_Label> {<#val>} \n"

        // common rules
        "<On_Off> ::= 'on' | 'off' \n"
        "<True_False> ::= 'true' | 'false' \n"
        "<Colour_Params> ::= <#red> <#green> <#blue> [<#alpha>] \n"
        "<Seperator> ::= -' ' \n"

        "<Labels_1_N> ::= <Label> [<Seperator>] {<More_Labels>} \n"
        "<More_Labels> ::=  <Label> [<Seperator>] \n"
		"<Label> ::= <Quoted_Label> | <Unquoted_Label> \n"
		"<Flex_Label> ::= <Quoted_Label> | <Spaced_Label> \n"
		"<Quoted_Label> ::= -'\"' <Spaced_Label> -'\"' \n"
		"<Spaced_Label> ::= <Spaced_Label_Illegals> {<Spaced_Label_Illegals>} \n"
        "<Unquoted_Label> ::= <Unquoted_Label_Illegals> {<Unquoted_Label_Illegals>} \n"
		"<Spaced_Label_Illegals> ::= (!,:\n\r\t{}\") \n"
		"<Unquoted_Label_Illegals> ::= (! :\n\r\t{}\") \n"

        ;

        return materialScript_BNF;
    }
    //-----------------------------------------------------------------------
    const String& MaterialScriptCompiler::getClientGrammerName(void) const
    {
        static const String grammerName = "Material Script";
        return grammerName;
    }
    //-----------------------------------------------------------------------
    MaterialScriptCompiler::MaterialScriptCompiler(void)
    {
        // set default group resource name
        mScriptContext.groupName = ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME;
    }
    //-----------------------------------------------------------------------
    MaterialScriptCompiler::~MaterialScriptCompiler(void)
    {

    }

    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseScript(DataStreamPtr& stream, const String& groupName, const bool allowOverride)
    {
        mScriptContext.groupName = groupName;
        mScriptContext.allowOverride = allowOverride;
        Compiler2Pass::compile(stream->getAsString(),  stream->getName());
    }

    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::setupTokenDefinitions(void)
    {
        addLexemeAction("{", &MaterialScriptCompiler::parseOpenBrace);
        addLexemeAction("}", &MaterialScriptCompiler::parseCloseBrace);
        addLexemeTokenAction("vertex_program", ID_VERTEX_PROGRAM, &MaterialScriptCompiler::parseGPUProgram);
            addLexemeAction("includes_skeletal_animation", &MaterialScriptCompiler::parseProgramSkeletalAnimation);
            addLexemeAction("includes_morph_animation", &MaterialScriptCompiler::parseProgramMorphAnimation);
            addLexemeAction("includes_pose_animation", &MaterialScriptCompiler::parseProgramPoseAnimation);
            addLexemeAction("uses_vertex_texture_fetch", &MaterialScriptCompiler::parseProgramVertexTextureFetch);

        addLexemeTokenAction("fragment_program", ID_FRAGMENT_PROGRAM, &MaterialScriptCompiler::parseGPUProgram);

            addLexemeAction("source", &MaterialScriptCompiler::parseProgramSource);
            addLexemeAction("syntax", &MaterialScriptCompiler::parseProgramSyntax);
            addLexemeAction("default_params", &MaterialScriptCompiler::parseDefaultParams);
            addLexemeAction("param_indexed", &MaterialScriptCompiler::parseParamIndexed);
            addLexemeAction("param_indexed_auto", &MaterialScriptCompiler::parseParamIndexedAuto);
            addLexemeAction("param_named", &MaterialScriptCompiler::parseParamNamed);
            addLexemeAction("param_named_auto", &MaterialScriptCompiler::parseParamNamedAuto);
            addLexemeAction("custom_parameter", &MaterialScriptCompiler::parseProgramCustomParameter);

        addLexemeAction("material", &MaterialScriptCompiler::parseMaterial);
            addLexemeToken(":", ID_CLONE);
            addLexemeAction("lod_distances", &MaterialScriptCompiler::parseLodDistances);
            addLexemeAction("receive_shadows", &MaterialScriptCompiler::parseReceiveShadows);
            addLexemeAction("transparency_casts_shadows", &MaterialScriptCompiler::parseTransparencyCastsShadows);
            addLexemeAction("set_texture_alias", &MaterialScriptCompiler::parseSetTextureAlias);

        // Technique section
        addLexemeAction("technique", &MaterialScriptCompiler::parseTechnique);
            addLexemeAction("scheme", &MaterialScriptCompiler::parseScheme);
            addLexemeAction("lod_index", &MaterialScriptCompiler::parseLodIndex);


        // Pass section
        addLexemeAction("pass", &MaterialScriptCompiler::parsePass);
            addLexemeAction("ambient", &MaterialScriptCompiler::parseAmbient);
            addLexemeAction("diffuse", &MaterialScriptCompiler::parseDiffuse);
            addLexemeAction("specular", &MaterialScriptCompiler::parseSpecular);
            addLexemeAction("emissive", &MaterialScriptCompiler::parseEmissive);
            addLexemeToken("vertexcolour", ID_VERTEXCOLOUR);

            addLexemeAction("scene_blend", &MaterialScriptCompiler::parseSceneBlend);
                addLexemeToken("colour_blend", ID_COLOUR_BLEND);
                addLexemeToken("dest_colour", ID_DEST_COLOUR);
                addLexemeToken("src_colour", ID_SRC_COLOUR);
                addLexemeToken("one_minus_dest_colour", ID_ONE_MINUS_DEST_COLOUR);
                addLexemeToken("one_minus_src_colour", ID_ONE_MINUS_SRC_COLOUR);
                addLexemeToken("dest_alpha", ID_DEST_ALPHA);
                addLexemeToken("src_alpha", ID_SRC_ALPHA);
                addLexemeToken("one_minus_dest_alpha", ID_ONE_MINUS_DEST_ALPHA);
                addLexemeToken("one_minus_src_alpha", ID_ONE_MINUS_SRC_ALPHA);

            addLexemeAction("depth_check", &MaterialScriptCompiler::parseDepthCheck);
            addLexemeAction("depth_write", &MaterialScriptCompiler::parseDepthWrite);
            addLexemeAction("depth_func", &MaterialScriptCompiler::parseDepthFunc);
            addLexemeAction("depth_bias", &MaterialScriptCompiler::parseDepthBias);
                addLexemeToken("always_fail", ID_ALWAYS_FAIL);
                addLexemeToken("always_pass", ID_ALWAYS_PASS);
                addLexemeToken("less_equal", ID_LESS_EQUAL);
                addLexemeToken("less", ID_LESS);
                addLexemeToken("equal", ID_EQUAL);
                addLexemeToken("not_equal", ID_NOT_EQUAL);
                addLexemeToken("greater_equal", ID_GREATER_EQUAL);
                addLexemeToken("greater", ID_GREATER);
            addLexemeAction("alpha_rejection", &MaterialScriptCompiler::parseAlphaRejection);
            addLexemeAction("cull_hardware", &MaterialScriptCompiler::parseCullHardware);
                addLexemeToken("clockwise", ID_CLOCKWISE);
                addLexemeToken("anticlockwise", ID_ANTICLOCKWISE);
            addLexemeAction("cull_software", &MaterialScriptCompiler::parseCullSoftware);
                addLexemeToken("back", ID_CULL_BACK);
                addLexemeToken("front", ID_CULL_FRONT);
            addLexemeAction("lighting", &MaterialScriptCompiler::parseLighting);
            addLexemeAction("shading", &MaterialScriptCompiler::parseShading);
                addLexemeTokenAction("flat", ID_FLAT);
                addLexemeToken("gouraud", ID_GOURAUD);
                addLexemeToken("phong", ID_PHONG);
            addLexemeAction("polygon_mode", &MaterialScriptCompiler::parsePolygonMode);
                addLexemeTokenAction("solid", ID_SOLID);
                addLexemeToken("wireframe", ID_WIREFRAME);
                addLexemeToken("points", ID_POINTS);
            addLexemeAction("fog_override", &MaterialScriptCompiler::parseFogOverride);
                addLexemeToken("exp", ID_EXP);
                addLexemeToken("exp2", ID_EXP2);
            addLexemeAction("colour_write", &MaterialScriptCompiler::parseColourWrite);
            addLexemeAction("max_lights", &MaterialScriptCompiler::parseMaxLights);
			addLexemeAction("start_light", &MaterialScriptCompiler::parseStartLight);
            addLexemeAction("iteration", &MaterialScriptCompiler::parseIteration);
                addLexemeToken("once", ID_ONCE);
                addLexemeToken("once_per_light", ID_ONCE_PER_LIGHT);
                addLexemeToken("per_light", ID_PER_LIGHT);
				addLexemeToken("per_n_lights", ID_PER_N_LIGHTS);
                addLexemeToken("directional", ID_DIRECTIONAL);
                addLexemeToken("spot", ID_SPOT);
            addLexemeAction("point_size", &MaterialScriptCompiler::parsePointSize);
            addLexemeAction("point_sprites", &MaterialScriptCompiler::parsePointSprites);
            addLexemeAction("point_size_attenuation",  &MaterialScriptCompiler::parsePointSizeAttenuation);
            addLexemeAction("point_size_min", &MaterialScriptCompiler::parsePointSizeMin);
            addLexemeAction("point_size_max", &MaterialScriptCompiler::parsePointSizeMax);

        // Texture Unit section
        addLexemeAction("texture_unit", &MaterialScriptCompiler::parseTextureUnit);
        addLexemeAction("texture_alias", &MaterialScriptCompiler::parseTextureAlias);
        addLexemeAction("texture", &MaterialScriptCompiler::parseTexture);
            addLexemeToken("1d", ID_1D);
            addLexemeToken("2d", ID_2D);
            addLexemeToken("3d", ID_3D);
            addLexemeToken("cubic", ID_CUBIC);
            addLexemeToken("unlimited", ID_UNLIMITED);
            addLexemeToken("alpha", ID_ALPHA);
        addLexemeAction("anim_texture", &MaterialScriptCompiler::parseAnimTexture);
        addLexemeAction("cubic_texture", &MaterialScriptCompiler::parseCubicTexture);
            addLexemeToken("separateuv", ID_SEPARATE_UV);
            addLexemeToken("combineduvw", ID_COMBINED_UVW);
        addLexemeAction("tex_coord_set", &MaterialScriptCompiler::parseTexCoord);
        addLexemeAction("tex_address_mode", &MaterialScriptCompiler::parseTexAddressMode);
            addLexemeToken("wrap", ID_WRAP);
            addLexemeToken("clamp", ID_CLAMP);
            addLexemeToken("mirror", ID_MIRROR);
            addLexemeToken("border", ID_BORDER);
        addLexemeAction("tex_border_colour", &MaterialScriptCompiler::parseTexBorderColour);
        addLexemeAction("filtering", &MaterialScriptCompiler::parseFiltering);
            addLexemeToken("bilinear", ID_BILINEAR);
            addLexemeToken("trilinear", ID_TRILINEAR);
            addLexemeToken("anisotropic", ID_ANISOTROPIC);
        addLexemeAction("max_anisotropy", &MaterialScriptCompiler::parseMaxAnisotropy);
        addLexemeAction("mipmap_bias", &MaterialScriptCompiler::parseMipMapBias);
        addLexemeAction("colour_op", &MaterialScriptCompiler::parseColourOp);
            addLexemeToken("replace", ID_REPLACE);
        addLexemeAction("colour_op_ex", &MaterialScriptCompiler::parseColourOpEx);
            addLexemeToken("source1", ID_SOURCE1);
            addLexemeToken("source2", ID_SOURCE2);
            addLexemeToken("modulate_x2", ID_MODULATE_X2);
            addLexemeToken("modulate_x4", ID_MODULATE_X4);
            addLexemeToken("add_signed", ID_ADD_SIGNED);
            addLexemeToken("add_smooth", ID_ADD_SMOOTH);
            addLexemeToken("subtract", ID_SUBTRACT);
            addLexemeToken("blend_diffuse_colour", ID_BLEND_DIFFUSE_COLOUR);
            addLexemeToken("blend_diffuse_alpha", ID_BLEND_DIFFUSE_ALPHA);
            addLexemeToken("blend_texture_alpha", ID_BLEND_TEXTURE_ALPHA);
            addLexemeToken("blend_current_alpha", ID_BLEND_CURRENT_ALPHA);
            addLexemeToken("blend_manual", ID_BLEND_MANUAL);
            addLexemeToken("dotproduct", ID_DOTPRODUCT);
            addLexemeToken("src_current", ID_SRC_CURRENT);
            addLexemeToken("src_texture", ID_SRC_TEXTURE);
            addLexemeToken("src_diffuse", ID_SRC_DIFFUSE);
            addLexemeToken("src_specular", ID_SRC_SPECULAR);
            addLexemeToken("src_manual", ID_SRC_MANUAL);
        addLexemeAction("colour_op_multipass_fallback", &MaterialScriptCompiler::parseColourOpMultipassFallback);
        addLexemeAction("alpha_op_ex", &MaterialScriptCompiler::parseAlphaOpEx);
        addLexemeAction("env_map", &MaterialScriptCompiler::parseEnvMap);
            addLexemeToken("spherical", ID_SPHERICAL);
            addLexemeToken("planar", ID_PLANAR);
            addLexemeToken("cubic_reflection", ID_CUBIC_REFLECTION);
            addLexemeToken("cubic_normal", ID_CUBIC_NORMAL);
        addLexemeAction("scroll", &MaterialScriptCompiler::parseScroll);
        addLexemeAction("scroll_anim", &MaterialScriptCompiler::parseScrollAnim);
        addLexemeTokenAction("rotate", ID_ROTATE, &MaterialScriptCompiler::parseRotate);
        addLexemeAction("rotate_anim", &MaterialScriptCompiler::parseRotateAnim);
        addLexemeAction("scale", &MaterialScriptCompiler::parseScale);
        addLexemeAction("wave_xform", &MaterialScriptCompiler::parseWaveXform);
            addLexemeToken("scroll_x", ID_SCROLL_X);
            addLexemeToken("scroll_y", ID_SCROLL_Y);
            addLexemeToken("scale_x", ID_SCALE_X);
            addLexemeToken("scale_y", ID_SCALE_Y);
            addLexemeToken("sine", ID_SINE);
            addLexemeToken("triangle", ID_TRIANGLE);
            addLexemeToken("square", ID_SQUARE);
            addLexemeToken("sawtooth", ID_SAWTOOTH);
            addLexemeToken("inverse_sawtooth", ID_INVERSE_SAWTOOTH);
        addLexemeAction("transform", &MaterialScriptCompiler::parseTransform);
        addLexemeAction("binding_type", &MaterialScriptCompiler::parseBindingType);
		addLexemeAction("content_type", &MaterialScriptCompiler::parseContentType);
			addLexemeToken("named", ID_NAMED);
			addLexemeToken("shadow", ID_SHADOW);
        // GPU program reference
        addLexemeAction("vertex_program_ref", &MaterialScriptCompiler::parseVertexProgramRef);
        addLexemeAction("fragment_program_ref", &MaterialScriptCompiler::parseFragmentProgramRef);
        addLexemeAction("shadow_caster_vertex_program_ref", &MaterialScriptCompiler::parseShadowCasterVertexProgramRef);
        addLexemeAction("shadow_receiver_vertex_program_ref", &MaterialScriptCompiler::parseShadowReceiverVertexProgramRef);
        addLexemeAction("shadow_receiver_fragment_program_ref", &MaterialScriptCompiler::parseShadowReceiverFragmentProgramRef);

        // common section
        addLexemeToken("on", ID_ON);
        addLexemeToken("off", ID_OFF);
        addLexemeToken("true", ID_TRUE);
        addLexemeToken("false", ID_FALSE);
        addLexemeToken("none", ID_NONE);
        addLexemeToken("point", ID_POINT);
        addLexemeToken("linear", ID_LINEAR);
        addLexemeToken("add", ID_ADD);
        addLexemeToken("modulate", ID_MODULATE);
        addLexemeToken("alpha_blend", ID_ALPHA_BLEND);
        addLexemeToken("one", ID_ONE);
        addLexemeToken("zero", ID_ZERO);
        addLexemeToken("vertex", ID_VERTEX);
        addLexemeToken("fragment", ID_FRAGMENT);


    }

    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::addLexemeTokenAction(const String& lexeme, const size_t token, const MSC_Action action)
    {
        size_t newTokenID = addLexemeToken(lexeme, token, action != 0);
        if (action)
            mTokenActionMap[newTokenID] = action;
    }

    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::executeTokenAction(const size_t tokenID)
    {
        TokenActionIterator action = mTokenActionMap.find(tokenID);

        if (action == mTokenActionMap.end())
        {
            // BAD command. BAD!
            logParseError("Unrecognised Material Script command action");
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
                if (ogreException.getNumber() == Exception::ERR_DUPLICATE_ITEM)
                {
                    // an exception for duplicate item was thrown.
                    // this means that either a material name or gpu program name conflict exists.
                    // Since it wasn't trapped at a lower level then re-throw in order to terminate parsing
                    logParseError("Duplicate Item Exception trapped. Parsing terminated for this material script.");
                    throw;
                }
                else
                {
                    // an unknown token found or BNF Grammer rule was not successful
                    // in finding a valid terminal token to complete the rule expression.
                    // don't relog the exception but do log the material script being parsed and the line number
                    logParseError("Exception trapped, attempting to continue parsing");
                }
            }
        }
    }

    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::logParseError(const String& error)
    {
        // log material name only if filename not specified
        if (mSourceName.empty() && !mScriptContext.material.isNull())
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
            mScriptContext.pendingDefaultParams.clear();
            mScriptContext.programDef = NULL;
            break;
        case MSS_DEFAULT_PARAMETERS:
            // End of default parameters
            mScriptContext.section = MSS_PROGRAM;
            break;
        };
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseGPUProgram(void)
    {
        // update section
        mScriptContext.section = MSS_PROGRAM;

		// Create new program definition-in-progress
		mScriptContext.programDef = new MaterialScriptProgramDefinition();
		mScriptContext.programDef->progType =
            (getCurrentTokenID() == ID_VERTEX_PROGRAM) ? GPT_VERTEX_PROGRAM : GPT_FRAGMENT_PROGRAM;
        mScriptContext.programDef->supportsSkeletalAnimation = false;
		mScriptContext.programDef->supportsMorphAnimation = false;
		mScriptContext.programDef->supportsPoseAnimation = 0;
        mScriptContext.programDef->usesVertexTextureFetch = false;

		// Get name and language code
		// Name, preserve case
		mScriptContext.programDef->name = getNextTokenLabel();
		// trim trailing white space only
		StringUtil::trim(mScriptContext.programDef->name);
		// language code
		mScriptContext.programDef->language = getNextTokenLabel();
		// make sure language is lower case
		StringUtil::toLowerCase(mScriptContext.programDef->language);
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseProgramSource(void)
    {
        assert(mScriptContext.programDef);
		mScriptContext.programDef->source = getNextTokenLabel();
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseProgramSyntax(void)
    {
        assert(mScriptContext.programDef);
		mScriptContext.programDef->syntax = getNextTokenLabel();
		// make sure language is lower case
		StringUtil::toLowerCase(mScriptContext.programDef->syntax);
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseProgramSkeletalAnimation(void)
    {
        assert(mScriptContext.programDef);
        mScriptContext.programDef->supportsSkeletalAnimation = testNextTokenID(ID_TRUE);
    }
	//-----------------------------------------------------------------------
	void MaterialScriptCompiler::parseProgramMorphAnimation(void)
	{
        assert(mScriptContext.programDef);
		mScriptContext.programDef->supportsMorphAnimation = testNextTokenID(ID_TRUE);
	}
	//-----------------------------------------------------------------------
	void MaterialScriptCompiler::parseProgramPoseAnimation(void)
	{
        assert(mScriptContext.programDef);
		mScriptContext.programDef->supportsPoseAnimation = static_cast<ushort>(getNextTokenValue());
	}
	//-----------------------------------------------------------------------
	void MaterialScriptCompiler::parseProgramVertexTextureFetch(void)
	{
        assert(mScriptContext.programDef);
		mScriptContext.programDef->usesVertexTextureFetch = testNextTokenID(ID_TRUE);
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseProgramCustomParameter(void)
    {
        assert(mScriptContext.programDef);

        String command = getNextTokenLabel();
		StringUtil::toLowerCase(command);
        String params = getNextTokenLabel();
        StringUtil::trim(params);
		mScriptContext.programDef->customParameters.push_back(
			std::pair<String, String>(command, params));
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseDefaultParams(void)
    {
        mScriptContext.section = MSS_DEFAULT_PARAMETERS;
    }
	//-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseMaterial(void)
    {
        // check params for reference to parent material to copy from
        // syntax: material name : parentMaterialName
        MaterialPtr basematerial;

        String materialName = getNextTokenLabel();
        StringUtil::trim(materialName);
        // Create a brand new material
        const size_t paramCount = getRemainingTokensForAction();
        if (paramCount == 2)
        {
            // this gets the ':' token which we need to consume to get to the label
            getNextToken();
            // if a second parameter exists then assume its the name of the base material
            // that this new material should clone from
            String parentName = getNextTokenLabel();
            StringUtil::trim(parentName);
            // make sure base material exists
            basematerial = MaterialManager::getSingleton().getByName(parentName);
            // if it doesn't exist then report error in log and just create a new material
            if (basematerial.isNull())
            {
                logParseError("parent material: " + parentName + " not found for new material:"
                    + materialName);
            }
        }

        /* attempt to create the material.  If the material name is already in use
           then MaterialManager throws an exception.
        */
        try
        {
            mScriptContext.material =
                MaterialManager::getSingleton().create(materialName, mScriptContext.groupName);
        }
        catch (Exception& e)
        {
            bool exceptionHandled = false;

            if (mScriptContext.allowOverride && (e.getNumber() == Exception::ERR_DUPLICATE_ITEM))
            {
                /* the material already exists log a warning message about it being modified by
                 another material script
                */
                mScriptContext.material = MaterialManager::getSingleton().getByName(materialName);
                if ( !mScriptContext.material.isNull())
                {
                    logParseError("material " + materialName +
                        ", defined in " + mScriptContext.material->getOrigin() +
                        ", was overwritten by current material being parsed with same name");
                    // update group ownership since the original material might have been in a different resource group
                    mScriptContext.material->changeGroupOwnership(mScriptContext.groupName);
                    exceptionHandled = true;
                }
            }
            if (!exceptionHandled)
            {
                logParseError("material " + materialName +
                        " was previously defined and can not override.\n"
                        "Material Manager script override was not enabled." );

                throw;
            }
        }

        if (!basematerial.isNull())
        {
            // copy parent material details to new or pre-existing material
            basematerial->copyDetailsTo(mScriptContext.material);
        }
        else
        {
            // Remove pre-created technique from defaults or original material
            mScriptContext.material->removeAllTechniques();
        }

		mScriptContext.material->_notifyOrigin(mSourceName);

        // update section
        mScriptContext.section = MSS_MATERIAL;
        //Reset all levels since this is the start of the material definition
        mScriptContext.passLev = -1;
        mScriptContext.stateLev= -1;
        mScriptContext.techLev = -1;

    }
    //-----------------------------------------------------------------------
    // material Section Actions
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseLodDistances(void)
    {
        // iterate over the parameters and parse distances out of them
        Material::LodDistanceList lodList;
		while (getRemainingTokensForAction() > 0)
		{
            lodList.push_back(getNextTokenValue());
        }

        mScriptContext.material->setLodLevels(lodList);
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseReceiveShadows(void)
    {
        mScriptContext.material->setReceiveShadows(testNextTokenID(ID_ON));
    }
	//-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseTransparencyCastsShadows(void)
	{
		mScriptContext.material->setTransparencyCastsShadows(testNextTokenID(ID_ON));
	}
	//-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseSetTextureAlias(void)
	{
	    const String& aliasName = getNextTokenLabel();
	    const String& textureName = getNextTokenLabel();
		mScriptContext.textureAliases[aliasName] = textureName;
	}
	//-----------------------------------------------------------------------
	// Technique section Actions
	//-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseTechnique(void)
    {
        String techniqueName;
        if (getRemainingTokensForAction() > 0)
            techniqueName = getNextTokenLabel();
        // if params is not empty then see if the technique name already exists
        if (!techniqueName.empty() && (mScriptContext.material->getNumTechniques() > 0))
        {
            Technique* foundTechnique = mScriptContext.material->getTechnique(techniqueName);
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
	void MaterialScriptCompiler::parseScheme(void)
	{
	    assert(mScriptContext.technique);
		mScriptContext.technique->setSchemeName(getNextTokenLabel());
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseLodIndex(void)
    {
	    assert(mScriptContext.technique);
        mScriptContext.technique->setLodIndex(static_cast<uint>(getNextTokenValue()));
    }
	//-----------------------------------------------------------------------
	// Pass Section Actions
	//-----------------------------------------------------------------------
    void MaterialScriptCompiler::parsePass(void)
    {
        String passName;
        if (getRemainingTokensForAction() > 0)
            passName = getNextTokenLabel();
        // if params is not empty then see if the pass name already exists
        if (!passName.empty() && (mScriptContext.technique->getNumPasses() > 0))
        {
            Pass* foundPass = mScriptContext.technique->getPass(passName);
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
        Real r = getNextTokenValue();
        Real g = getNextTokenValue();
        Real b = getNextTokenValue();
        Real a = getRemainingTokensForAction() == 1 ? getNextTokenValue() : 1.0f;
        return ColourValue(r, g, b, a);
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseAmbient(void)
    {
        // Must be 1, 3 or 4 parameters
        assert(mScriptContext.pass);
        const size_t paramCount = getRemainingTokensForAction();
        if (paramCount == 1) {
            if(testNextTokenID(ID_VERTEXCOLOUR))
            {
                mScriptContext.pass->setVertexColourTracking(mScriptContext.pass->getVertexColourTracking() | TVC_AMBIENT);
            }
            else
            {
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
        assert(mScriptContext.pass);
        const size_t paramCount = getRemainingTokensForAction();
        if (paramCount == 1) {
            if(testNextTokenID(ID_VERTEXCOLOUR))
            {
               mScriptContext.pass->setVertexColourTracking(mScriptContext.pass->getVertexColourTracking() | TVC_DIFFUSE);
            }
            else
            {
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
        assert(mScriptContext.pass);
        const size_t paramCount = getRemainingTokensForAction();
        if(paramCount == 2)
        {
            if(getNextTokenID() == ID_VERTEXCOLOUR)
            {
                mScriptContext.pass->setVertexColourTracking(mScriptContext.pass->getVertexColourTracking() | TVC_SPECULAR);
                mScriptContext.pass->setShininess(getNextTokenValue());
            }
            else
            {
                logParseError("Bad specular attribute, double parameter statement must be 'vertexcolour <shininess>'");
            }
        }
        else if(paramCount == 4 || paramCount == 5)
        {
            Real r = getNextTokenValue();
            Real g = getNextTokenValue();
            Real b = getNextTokenValue();
            Real a = paramCount == 5 ? getNextTokenValue() : 1.0f;
            mScriptContext.pass->setSpecular(r, g, b, a);
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
        assert(mScriptContext.pass);
        const size_t paramCount = getRemainingTokensForAction();
        if (paramCount == 1) {
            if(testNextTokenID(ID_VERTEXCOLOUR))
            {
               mScriptContext.pass->setVertexColourTracking(mScriptContext.pass->getVertexColourTracking() | TVC_EMISSIVE);
            }
            else
            {
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
    SceneBlendFactor MaterialScriptCompiler::convertBlendFactor(void)
    {
        switch(getNextTokenID())
        {
        case ID_ONE:
            return SBF_ONE;
        case ID_ZERO:
            return SBF_ZERO;
        case ID_DEST_COLOUR:
            return SBF_DEST_COLOUR;
        case ID_SRC_COLOUR:
            return SBF_SOURCE_COLOUR;
        case ID_ONE_MINUS_DEST_COLOUR:
            return SBF_ONE_MINUS_DEST_COLOUR;
        case ID_ONE_MINUS_SRC_COLOUR:
            return SBF_ONE_MINUS_SOURCE_COLOUR;
        case ID_DEST_ALPHA:
            return SBF_DEST_ALPHA;
        case ID_SRC_ALPHA:
            return SBF_SOURCE_ALPHA;
        case ID_ONE_MINUS_DEST_ALPHA:
            return SBF_ONE_MINUS_DEST_ALPHA;
        case ID_ONE_MINUS_SRC_ALPHA:
            return SBF_ONE_MINUS_SOURCE_ALPHA;
        default:
            return SBF_ONE;
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseSceneBlend(void)
    {
        assert(mScriptContext.pass);
        const size_t paramCount = getRemainingTokensForAction();
        // Should be 1 or 2 params
        if (paramCount == 1)
        {
            //simple blend types
            SceneBlendType sbtype = SBT_REPLACE;
            switch(getNextTokenID())
            {
            case ID_ADD:
                sbtype = SBT_ADD;
                break;
            case ID_MODULATE:
                sbtype = SBT_MODULATE;
                break;
			case ID_COLOUR_BLEND:
				sbtype = SBT_TRANSPARENT_COLOUR;
				break;
            case ID_ALPHA_BLEND:
                sbtype = SBT_TRANSPARENT_ALPHA;
                break;
            default:
                break;
            }
            mScriptContext.pass->setSceneBlending(sbtype);

        }
        else if (paramCount == 2)
        {
            const SceneBlendFactor src = convertBlendFactor();
            const SceneBlendFactor dest = convertBlendFactor();
            mScriptContext.pass->setSceneBlending(src,dest);
        }
        else
        {
            logParseError(
                "Bad scene_blend attribute, wrong number of parameters (expected 1 or 2)");
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseDepthCheck(void)
    {
        assert(mScriptContext.pass);
        mScriptContext.pass->setDepthCheckEnabled(testNextTokenID(ID_ON));
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseDepthWrite(void)
    {
        assert(mScriptContext.pass);
        mScriptContext.pass->setDepthWriteEnabled(testNextTokenID(ID_ON));
    }
    //-----------------------------------------------------------------------
    CompareFunction MaterialScriptCompiler::convertCompareFunction(void)
    {
        switch (getNextTokenID())
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
        default:
            return CMPF_LESS_EQUAL;
            break;
        }
    }

    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseDepthFunc(void)
    {
        assert(mScriptContext.pass);
        mScriptContext.pass->setDepthFunction(convertCompareFunction());
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseDepthBias(void)
    {
        assert(mScriptContext.pass);
		float constantBias = static_cast<float>(getNextTokenValue());
		float slopeScaleBias = 0.0f;
		if (getRemainingTokensForAction() == 1)
		{
			slopeScaleBias = static_cast<float>(getNextTokenValue());
		}

        mScriptContext.pass->setDepthBias(constantBias, slopeScaleBias);
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseAlphaRejection(void)
    {
        assert(mScriptContext.pass);
        const CompareFunction cmp = convertCompareFunction();
        mScriptContext.pass->setAlphaRejectSettings(cmp, static_cast<unsigned char>(getNextTokenValue()));
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseCullHardware(void)
    {
        assert(mScriptContext.pass);
        switch (getNextTokenID())
        {
        case ID_NONE:
            mScriptContext.pass->setCullingMode(CULL_NONE);
            break;
        case ID_ANTICLOCKWISE:
            mScriptContext.pass->setCullingMode(CULL_ANTICLOCKWISE);
            break;
        case ID_CLOCKWISE:
            mScriptContext.pass->setCullingMode(CULL_CLOCKWISE);
            break;
        default:
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseCullSoftware(void)
    {
        assert(mScriptContext.pass);
        switch (getNextTokenID())
        {
        case ID_NONE:
            mScriptContext.pass->setManualCullingMode(MANUAL_CULL_NONE);
            break;
        case ID_CULL_BACK:
            mScriptContext.pass->setManualCullingMode(MANUAL_CULL_BACK);
            break;
        case ID_CULL_FRONT:
            mScriptContext.pass->setManualCullingMode(MANUAL_CULL_FRONT);
            break;
        default:
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseLighting(void)
    {
        assert(mScriptContext.pass);
        mScriptContext.pass->setLightingEnabled(testNextTokenID(ID_ON));
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseShading(void)
    {
        assert(mScriptContext.pass);
        switch (getNextTokenID())
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
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parsePolygonMode(void)
    {
        assert(mScriptContext.pass);
        switch (getNextTokenID())
        {
        case ID_SOLID:
            mScriptContext.pass->setPolygonMode(PM_SOLID);
            break;
        case ID_WIREFRAME:
            mScriptContext.pass->setPolygonMode(PM_WIREFRAME);
            break;
        case ID_POINTS:
            mScriptContext.pass->setPolygonMode(PM_POINTS);
            break;
        default:
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseFogOverride(void)
    {
        assert(mScriptContext.pass);
        if (getNextTokenID() == ID_TRUE)
        {
            // if true, we need to see if they supplied all arguments, or just the 1... if just the one,
            // Assume they want to disable the default fog from effecting this material.
            const size_t paramCount = getRemainingTokensForAction();
            if( paramCount == 7 )
            {
                FogMode fogtype;
                switch (getNextTokenID())
                {
                case ID_LINEAR:
                    fogtype = FOG_LINEAR;
                case ID_EXP:
                    fogtype = FOG_EXP;
                case ID_EXP2:
                    fogtype = FOG_EXP2;
                case ID_NONE:
                default:
                    fogtype = FOG_NONE;
                    break;
                }

                const Real red = getNextTokenValue();
                const Real green = getNextTokenValue();
                const Real blue = getNextTokenValue();
                const Real density = getNextTokenValue();
                const Real start = getNextTokenValue();
                const Real end = getNextTokenValue();

                mScriptContext.pass->setFog(
                    true,
                    fogtype,
                    ColourValue(red, green, blue),
                    density, start, end
                    );
            }
            else
            {
                mScriptContext.pass->setFog(true);
            }
        }
        else
            mScriptContext.pass->setFog(false);

    }
   //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseColourWrite(void)
    {
        assert(mScriptContext.pass);
        mScriptContext.pass->setColourWriteEnabled(testNextTokenID(ID_ON));
    }
     //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseMaxLights(void)
    {
        assert(mScriptContext.pass);
		mScriptContext.pass->setMaxSimultaneousLights(static_cast<int>(getNextTokenValue()));
    }
	void MaterialScriptCompiler::parseStartLight(void)
	{
		assert(mScriptContext.pass);
		mScriptContext.pass->setStartLight(static_cast<int>(getNextTokenValue()));
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseIterationLightTypes(void)
    {
        assert(mScriptContext.pass);
        // Parse light type
        switch(getNextTokenID())
        {
        case ID_DIRECTIONAL:
            mScriptContext.pass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
            break;
        case ID_POINT:
            mScriptContext.pass->setIteratePerLight(true, true, Light::LT_POINT);
            break;
        case ID_SPOT:
            mScriptContext.pass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseIteration(void)
    {
        assert(mScriptContext.pass);
        // we could have more than one parameter
        /** combinations could be:
            iteration once
            iteration once_per_light [light type]
            iteration <number>
            iteration <number> [per_light] [light type]
			iteration <number> [per_n_lights] <number> [light type]
        */
        if (testNextTokenID(ID_ONCE))
            mScriptContext.pass->setIteratePerLight(false, false);
        else if (testNextTokenID(ID_ONCE_PER_LIGHT))
        {
            getNextToken();
            if (getRemainingTokensForAction() == 1)
            {
                parseIterationLightTypes();
            }
            else
            {
                mScriptContext.pass->setIteratePerLight(true, false);
            }

        }
        else // could be using form: <number> [per_light] [light type]
        {
            uint passIterationCount = static_cast<uint>(getNextTokenValue());
            if (passIterationCount > 0)
            {
                mScriptContext.pass->setPassIterationCount(passIterationCount);
                if (getRemainingTokensForAction() > 1)
                {
                    const size_t tokenID = getNextTokenID();
                    if (tokenID == ID_PER_LIGHT)
                    {
                        if (getRemainingTokensForAction() == 1)
                        {
                            parseIterationLightTypes();
                        }
                        else
                        {
                            mScriptContext.pass->setIteratePerLight(true, false);
                        }
                    }
					else if (tokenID == ID_PER_N_LIGHTS)
					{
						// Number of lights per iteration
						mScriptContext.pass->setLightCountPerIteration(static_cast<short unsigned int>(getNextTokenValue()));
						if (getRemainingTokensForAction() == 1)
						{
							parseIterationLightTypes();
						}
						else
						{
							mScriptContext.pass->setIteratePerLight(true, false);
						}
					}
                    else
                        logParseError(
                            "Bad iteration attribute, valid parameters are <number> [per_light|per_n_lights <num_lights>] [light type].");
                }
            }
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
        assert(mScriptContext.pass);
        mScriptContext.pass->setPointSpritesEnabled(testNextTokenID(ID_ON));
	}
    //-----------------------------------------------------------------------
	void MaterialScriptCompiler::parsePointSizeMin(void)
	{
        assert(mScriptContext.pass);
        mScriptContext.pass->setPointMinSize(getNextTokenValue());
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parsePointSizeMax(void)
	{
        assert(mScriptContext.pass);
        mScriptContext.pass->setPointMaxSize(getNextTokenValue());
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parsePointSizeAttenuation(void)
	{
        assert(mScriptContext.pass);
        const size_t paramCount = getRemainingTokensForAction();
        if (paramCount != 1 && paramCount != 4)
        {
            logParseError("Bad point_size_attenuation attribute, wrong number of parameters (expected 1 or 4)");
            return;
        }
        switch (getNextTokenID())
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

		if (getRemainingTokensForAction() != 2)
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
        if (getRemainingTokensForAction() > 0)
            tusName = getNextTokenLabel();
        // if params is a name then see if that texture unit exists
        // if not then log the warning and just move on to the next TU from current
        if (!tusName.empty() && (mScriptContext.pass->getNumTextureUnitStates() > 0))
        {
            // specifying a TUS name in the script for a TU means that a specific TU is being requested
            // try to get the specific TU
            // if the index requested is not valid, just creat a new TU
            // find the TUS with name
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

        if (mScriptContext.pass->getNumTextureUnitStates() > static_cast<size_t>(mScriptContext.stateLev))
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
    void MaterialScriptCompiler::parseTextureAlias(void)
    {
        assert(mScriptContext.textureUnit);
        mScriptContext.textureUnit->setTextureNameAlias(getNextTokenLabel());
    }
    //-----------------------------------------------------------------------
    // Texture layer attributes
    void MaterialScriptCompiler::parseTexture(void)
    {
        assert(mScriptContext.textureUnit);
        TextureType tt = TEX_TYPE_2D;
		int mipmaps = MIP_DEFAULT; // When passed to TextureManager::load, this means default to default number of mipmaps
        bool isAlpha = false;
        PixelFormat desiredFormat = PF_UNKNOWN;
        const String& textureName = getNextTokenLabel();

		while (getRemainingTokensForAction() > 0)
		{
            switch(getNextTokenID())
            {
            case ID_1D:
                tt = TEX_TYPE_1D;
                break;
            case ID_2D:
                tt = TEX_TYPE_2D;
                break;
            case ID_3D:
                tt = TEX_TYPE_3D;
                break;
            case ID_CUBIC:
                tt = TEX_TYPE_CUBE_MAP;
                break;
            case ID_UNLIMITED:
				mipmaps = MIP_UNLIMITED;
                break;
            case ID_ALPHA:
                isAlpha = true;
                break;
            case _value_:
                replaceToken();
                mipmaps = static_cast<int>(getNextTokenValue());
                break;
            default:
                desiredFormat = PixelUtil::getFormatFromName(getCurrentTokenLexeme(), true);
                break;
            }
		}
        mScriptContext.textureUnit->setTextureName(textureName, tt);
        mScriptContext.textureUnit->setNumMipmaps(mipmaps);
        mScriptContext.textureUnit->setIsAlpha(isAlpha);
        mScriptContext.textureUnit->setDesiredFormat(desiredFormat);
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseAnimTexture(void)
    {
        assert(mScriptContext.textureUnit);
        StringVector vecparams;
        // first token will be a label
        vecparams.push_back(getNextTokenLabel());
        // Determine which form it is
        // if next token is a value then no more labels to be processed
        if (testNextTokenID(_value_))
        {
            // First form using base name & number of frames
            unsigned int frameCount = static_cast<unsigned int>(getNextTokenValue());
            mScriptContext.textureUnit->setAnimatedTextureName(
                vecparams[0],
                frameCount,
                getNextTokenValue());
        }
        else
        {
            unsigned int numParams = 1;
            while (getRemainingTokensForAction() > 1)
            {
                vecparams.push_back(getNextTokenLabel());
                ++numParams;
            }
            // the last label should be a number so convert string label to number
            // Second form using individual names
            mScriptContext.textureUnit->setAnimatedTextureName(
                (String*)&vecparams[0],
                numParams,
                StringConverter::parseReal(getNextTokenLabel()));
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseCubicTexture(void)
    {
        assert(mScriptContext.textureUnit);
        StringVector vecparams;

        // first token will be a label
        vecparams.push_back(getNextTokenLabel());
        // Determine which form it is
        // if next token is a label then 5 more labels to be processed
        if (testNextTokenID(_character_))
        {
            // get next five texture names
            for (int i = 0; i < 5; ++i)
                vecparams.push_back(getNextTokenLabel());
        }

        bool useUVW = testNextTokenID(ID_COMBINED_UVW);

        if (vecparams.size() == 1)
        {
            mScriptContext.textureUnit->setCubicTextureName(vecparams[0], useUVW);
        }
        else
        {
            // Second form using individual names
            // Can use vecparams[0] as array start point
            mScriptContext.textureUnit->setCubicTextureName((String*)&vecparams[0], useUVW);
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseTexCoord(void)
    {
        assert(mScriptContext.textureUnit);
        mScriptContext.textureUnit->setTextureCoordSet(
            static_cast<unsigned int>(getNextTokenValue()));
    }
    //-----------------------------------------------------------------------
	TextureUnitState::TextureAddressingMode MaterialScriptCompiler::convTexAddressMode(void)
	{
	    switch (getNextTokenID())
	    {
		case ID_WRAP:
			return TextureUnitState::TAM_WRAP;
		case ID_CLAMP:
			return TextureUnitState::TAM_CLAMP;
		case ID_MIRROR:
			return TextureUnitState::TAM_MIRROR;
		case ID_BORDER:
			return TextureUnitState::TAM_BORDER;
		default:
            return TextureUnitState::TAM_WRAP;
	    }
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseTexAddressMode(void)
    {
        assert(mScriptContext.textureUnit);
        const size_t paramCount = getRemainingTokensForAction();

		if (paramCount == 1)
		{
			// Single-parameter option
			mScriptContext.textureUnit->setTextureAddressingMode(
				convTexAddressMode());
		}
		else
		{
			// 2-3 parameter option
			TextureUnitState::UVWAddressingMode uvw;
			uvw.u = convTexAddressMode();
			uvw.v = convTexAddressMode();

			if (paramCount == 3)
			{
				// w
				uvw.w = convTexAddressMode();
			}
			else
			{
				uvw.w = TextureUnitState::TAM_WRAP;
			}
			mScriptContext.textureUnit->setTextureAddressingMode(uvw);
		}
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseTexBorderColour(void)
    {
        assert(mScriptContext.textureUnit);
        mScriptContext.textureUnit->setTextureBorderColour( _parseColourValue() );
    }
    //-----------------------------------------------------------------------
    FilterOptions MaterialScriptCompiler::convertFiltering()
    {
        switch (getNextTokenID())
        {
        case ID_NONE:
            return FO_NONE;
        case ID_POINT:
            return FO_POINT;
        case ID_LINEAR:
            return FO_LINEAR;
        case ID_ANISOTROPIC:
            return FO_ANISOTROPIC;
        default:
            return FO_POINT;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseFiltering(void)
    {
        assert(mScriptContext.textureUnit);
        // Must be 1 or 3 parameters
        const size_t paramCount = getRemainingTokensForAction();
        if (paramCount == 1)
        {
            // Simple format
            switch (getNextTokenID())
            {
            case ID_BILINEAR:
                mScriptContext.textureUnit->setTextureFiltering(TFO_BILINEAR);
                break;
            case ID_TRILINEAR:
                mScriptContext.textureUnit->setTextureFiltering(TFO_TRILINEAR);
                break;
            case ID_ANISOTROPIC:
                mScriptContext.textureUnit->setTextureFiltering(TFO_ANISOTROPIC);
                break;
            case ID_NONE:
                mScriptContext.textureUnit->setTextureFiltering(TFO_NONE);
                break;
            }
        }
        else
        {
            // Complex format
            const FilterOptions minFO = convertFiltering();
            const FilterOptions magFO = convertFiltering();
            const FilterOptions mipFO = convertFiltering();
            mScriptContext.textureUnit->setTextureFiltering(minFO, magFO, mipFO);
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseMaxAnisotropy(void)
    {
        assert(mScriptContext.textureUnit);
        mScriptContext.textureUnit->setTextureAnisotropy(
            static_cast<unsigned int>(getNextTokenValue()));
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseMipMapBias(void)
    {
        assert(mScriptContext.textureUnit);
        mScriptContext.textureUnit->setTextureMipmapBias(getNextTokenValue());
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseColourOp(void)
    {
        assert(mScriptContext.textureUnit);
        switch (getNextTokenID())
        {
        case ID_REPLACE:
            mScriptContext.textureUnit->setColourOperation(LBO_REPLACE);
            break;
        case ID_ADD:
            mScriptContext.textureUnit->setColourOperation(LBO_ADD);
            break;
        case ID_MODULATE:
            mScriptContext.textureUnit->setColourOperation(LBO_MODULATE);
            break;
        case ID_ALPHA_BLEND:
            mScriptContext.textureUnit->setColourOperation(LBO_ALPHA_BLEND);
            break;
        }
    }
    //-----------------------------------------------------------------------
    LayerBlendOperationEx MaterialScriptCompiler::convertBlendOpEx(void)
    {
        switch(getNextTokenID())
        {
        case ID_SOURCE1:
            return LBX_SOURCE1;
        case ID_SOURCE2:
            return LBX_SOURCE2;
        case ID_MODULATE:
            return LBX_MODULATE;
        case ID_MODULATE_X2:
            return LBX_MODULATE_X2;
        case ID_MODULATE_X4:
            return LBX_MODULATE_X4;
        case ID_ADD:
            return LBX_ADD;
        case ID_ADD_SIGNED:
            return LBX_ADD_SIGNED;
        case ID_ADD_SMOOTH:
            return LBX_ADD_SMOOTH;
        case ID_SUBTRACT:
            return LBX_SUBTRACT;
        case ID_BLEND_DIFFUSE_COLOUR:
            return LBX_BLEND_DIFFUSE_COLOUR;
        case ID_BLEND_DIFFUSE_ALPHA:
            return LBX_BLEND_DIFFUSE_ALPHA;
        case ID_BLEND_TEXTURE_ALPHA:
            return LBX_BLEND_TEXTURE_ALPHA;
        case ID_BLEND_CURRENT_ALPHA:
            return LBX_BLEND_CURRENT_ALPHA;
        case ID_BLEND_MANUAL:
            return LBX_BLEND_MANUAL;
        case ID_DOTPRODUCT:
            return LBX_DOTPRODUCT;
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid blend function", "convertBlendOpEx");
        }
    }
    //-----------------------------------------------------------------------
    LayerBlendSource MaterialScriptCompiler::convertBlendSource(void)
    {
        switch(getNextTokenID())
        {
        case ID_SRC_CURRENT:
            return LBS_CURRENT;
        case ID_SRC_TEXTURE:
            return LBS_TEXTURE;
        case ID_SRC_DIFFUSE:
            return LBS_DIFFUSE;
        case ID_SRC_SPECULAR:
            return LBS_SPECULAR;
        case ID_SRC_MANUAL:
            return LBS_MANUAL;
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid blend source", "convertBlendSource");
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseColourOpEx(void)
    {

        assert(mScriptContext.textureUnit);

        LayerBlendOperationEx op;
        LayerBlendSource src1, src2;
        Real manual = 0.0;
        ColourValue colSrc1 = ColourValue::White;
        ColourValue colSrc2 = ColourValue::White;

        try {
            op = convertBlendOpEx();
            src1 = convertBlendSource();
            src2 = convertBlendSource();

            if (op == LBX_BLEND_MANUAL)
                manual = getNextTokenValue();

            if (src1 == LBS_MANUAL)
                colSrc1 = _parseColourValue();

            if (src2 == LBS_MANUAL)
                colSrc2 = _parseColourValue();
        }
        catch (Exception& e)
        {
            logParseError("Bad colour_op_ex attribute, " + e.getDescription());
            return;
        }

        mScriptContext.textureUnit->setColourOperationEx(op, src1, src2, colSrc1, colSrc2, manual);
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseColourOpMultipassFallback(void)
    {
        assert(mScriptContext.textureUnit);

        SceneBlendFactor src = convertBlendFactor();
        SceneBlendFactor dest = convertBlendFactor();
        mScriptContext.textureUnit->setColourOpMultipassFallback(src,dest);
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseAlphaOpEx(void)
    {
        assert(mScriptContext.textureUnit);

        LayerBlendOperationEx op;
        LayerBlendSource src1, src2;
        Real manual = 0.0;
        Real arg1 = 1.0, arg2 = 1.0;

        try {
            op = convertBlendOpEx();
            src1 = convertBlendSource();
            src2 = convertBlendSource();

            if (op == LBX_BLEND_MANUAL)
                manual = getNextTokenValue();

            if (src1 == LBS_MANUAL)
                arg1 = getNextTokenValue();

            if (src2 == LBS_MANUAL)
                arg2 = getNextTokenValue();
        }
        catch (Exception& e)
        {
            logParseError("Bad alpha_op_ex attribute, " + e.getDescription());
            return;
        }

        mScriptContext.textureUnit->setAlphaOperation(op, src1, src2, arg1, arg2, manual);
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseEnvMap(void)
    {
        assert(mScriptContext.textureUnit);

        switch (getNextTokenID())
        {
        case ID_OFF:
            mScriptContext.textureUnit->setEnvironmentMap(false);
            break;
        case ID_SPHERICAL:
            mScriptContext.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_CURVED);
            break;
        case ID_PLANAR:
            mScriptContext.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_PLANAR);
            break;
        case ID_CUBIC_REFLECTION:
            mScriptContext.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_REFLECTION);
            break;
        case ID_CUBIC_NORMAL:
            mScriptContext.textureUnit->setEnvironmentMap(true, TextureUnitState::ENV_NORMAL);
            break;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseScroll(void)
    {
        assert(mScriptContext.textureUnit);

        const Real x = getNextTokenValue();
        const Real y = getNextTokenValue();

        mScriptContext.textureUnit->setTextureScroll(x, y);
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseScrollAnim(void)
    {
        assert(mScriptContext.textureUnit);

        const Real xspeed = getNextTokenValue();
        const Real yspeed = getNextTokenValue();

        mScriptContext.textureUnit->setScrollAnimation(xspeed, yspeed);
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseRotate(void)
    {
        assert(mScriptContext.textureUnit);
        mScriptContext.textureUnit->setTextureRotate(Angle(getNextTokenValue()));
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseRotateAnim(void)
    {
        assert(mScriptContext.textureUnit);
        mScriptContext.textureUnit->setRotateAnimation(getNextTokenValue());
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseScale(void)
    {
        assert(mScriptContext.textureUnit);
        const Real xscale = getNextTokenValue();
        const Real yscale = getNextTokenValue();
        mScriptContext.textureUnit->setTextureScale(xscale, yscale);
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseWaveXform(void)
    {

        assert(mScriptContext.textureUnit);

        TextureUnitState::TextureTransformType ttype;
        WaveformType waveType;
        // Check transform type
        switch (getNextTokenID())
        {
		default:
        case ID_SCROLL_X:
            ttype = TextureUnitState::TT_TRANSLATE_U;
            break;
        case ID_SCROLL_Y:
            ttype = TextureUnitState::TT_TRANSLATE_V;
            break;
        case ID_ROTATE:
            ttype = TextureUnitState::TT_ROTATE;
            break;
        case ID_SCALE_X:
            ttype = TextureUnitState::TT_SCALE_U;
            break;
        case ID_SCALE_Y:
            ttype = TextureUnitState::TT_SCALE_V;
            break;
        }
        // Check wave type
        switch (getNextTokenID())
        {
		default:
        case ID_SINE:
            waveType = WFT_SINE;
            break;
        case ID_TRIANGLE:
            waveType = WFT_TRIANGLE;
            break;
        case ID_SQUARE:
            waveType = WFT_SQUARE;
            break;
        case ID_SAWTOOTH:
            waveType = WFT_SAWTOOTH;
            break;
        case ID_INVERSE_SAWTOOTH:
            waveType = WFT_INVERSE_SAWTOOTH;
            break;
        }

        const Real base = getNextTokenValue();
        const Real frequency = getNextTokenValue();
        const Real phase = getNextTokenValue();
        const Real amplitude = getNextTokenValue();

        mScriptContext.textureUnit->setTransformAnimation(
            ttype,
            waveType,
            base,
            frequency,
            phase,
            amplitude );
    }
	//-----------------------------------------------------------------------
	void MaterialScriptCompiler::parseTransform(void)
	{
        assert(mScriptContext.textureUnit);

        Real matrixArray[16];

        for (size_t i = 0; i < 16; ++i)
        {
            matrixArray[i] = getNextTokenValue();
        }

		Matrix4 xform(
			matrixArray[0],
			matrixArray[1],
			matrixArray[2],
			matrixArray[3],
			matrixArray[4],
			matrixArray[5],
			matrixArray[6],
			matrixArray[7],
			matrixArray[8],
			matrixArray[9],
			matrixArray[10],
			matrixArray[11],
			matrixArray[12],
			matrixArray[13],
			matrixArray[14],
			matrixArray[15]);

		mScriptContext.textureUnit->setTextureTransform(xform);
	}
	//-----------------------------------------------------------------------
	void MaterialScriptCompiler::parseBindingType(void)
	{
        assert(mScriptContext.textureUnit);
        switch (getNextTokenID())
        {
        case ID_VERTEX:
            mScriptContext.textureUnit->setBindingType(TextureUnitState::BT_VERTEX);
            break;
        case ID_FRAGMENT:
            mScriptContext.textureUnit->setBindingType(TextureUnitState::BT_FRAGMENT);
            break;
        }
    }
	//-----------------------------------------------------------------------
	void MaterialScriptCompiler::parseContentType(void)
	{
		assert(mScriptContext.textureUnit);
		switch (getNextTokenID())
		{
		case ID_NAMED:
			mScriptContext.textureUnit->setContentType(TextureUnitState::CONTENT_NAMED);
			break;
		case ID_SHADOW:
			mScriptContext.textureUnit->setContentType(TextureUnitState::CONTENT_SHADOW);
			break;
		}
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseVertexProgramRef(void)
    {
        assert(mScriptContext.pass);
        // update section
        mScriptContext.section = MSS_PROGRAM_REF;
        String name;

        // get the name of the program definition if it was set
        if (getRemainingTokensForAction() == 1)
        {
            name = getNextTokenLabel();
            StringUtil::trim(name);
        }

        // check if pass has a vertex program already
        if (mScriptContext.pass->hasVertexProgram())
        {
            // if existing pass vertex program has same name as params
            // or params is empty then use current vertex program
            if (name.empty() || (mScriptContext.pass->getVertexProgramName() == name))
            {
                mScriptContext.program = mScriptContext.pass->getVertexProgram();
            }
        }

        // if context.program was not set then try to get the vertex program using the name
        // passed in params
        if (mScriptContext.program.isNull())
        {
            mScriptContext.program = GpuProgramManager::getSingleton().getByName(name);
            if (mScriptContext.program.isNull())
            {
                // Unknown program
                logParseError("Invalid vertex_program_ref entry - vertex program "
                    + name + " has not been defined.");
                return;
            }

            // Set the vertex program for this pass
            mScriptContext.pass->setVertexProgram(name);
        }

        mScriptContext.isProgramShadowCaster = false;
        mScriptContext.isVertexProgramShadowReceiver = false;
        mScriptContext.isFragmentProgramShadowReceiver = false;

        // Create params? Skip this if program is not supported
        if (mScriptContext.program->isSupported())
        {
            mScriptContext.programParams = mScriptContext.pass->getVertexProgramParameters();
			mScriptContext.numAnimationParametrics = 0;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseShadowCasterVertexProgramRef(void)
    {
        assert(mScriptContext.pass);
        // update section
        mScriptContext.section = MSS_PROGRAM_REF;
        String name;

        // get the name of the program definition if it was set
        if (getRemainingTokensForAction() == 1)
        {
            name = getNextTokenLabel();
            StringUtil::trim(name);
        }

        // check if pass has a shadow caster vertex program already
        if (mScriptContext.pass->hasShadowCasterVertexProgram())
        {
            // if existing pass vertex program has same name as params
            // or params is empty then use current vertex program
            if (name.empty() || (mScriptContext.pass->getShadowCasterVertexProgramName() == name))
            {
                mScriptContext.program = mScriptContext.pass->getShadowCasterVertexProgram();
            }
        }

        // if context.program was not set then try to get the vertex program using the name
        // passed in params
        if (mScriptContext.program.isNull())
        {
            mScriptContext.program = GpuProgramManager::getSingleton().getByName(name);
            if (mScriptContext.program.isNull())
            {
                // Unknown program
                logParseError("Invalid shadow_caster_vertex_program_ref entry - vertex program "
                    + name + " has not been defined.");
                return;
            }

            // Set the vertex program for this pass
            mScriptContext.pass->setShadowCasterVertexProgram(name);
        }

        mScriptContext.isProgramShadowCaster = true;
        mScriptContext.isVertexProgramShadowReceiver = false;
		mScriptContext.isFragmentProgramShadowReceiver = false;

        // Create params? Skip this if program is not supported
        if (mScriptContext.program->isSupported())
        {
            mScriptContext.programParams = mScriptContext.pass->getShadowCasterVertexProgramParameters();
			mScriptContext.numAnimationParametrics = 0;
        }
    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseShadowReceiverVertexProgramRef(void)
    {
        assert(mScriptContext.pass);
        // update section
        mScriptContext.section = MSS_PROGRAM_REF;

        String name;

        // get the name of the program definition if it was set
        if (getRemainingTokensForAction() == 1)
        {
            name = getNextTokenLabel();
            StringUtil::trim(name);
        }

        // check if pass has a shadow caster vertex program already
        if (mScriptContext.pass->hasShadowReceiverVertexProgram())
        {
            // if existing pass vertex program has same name as params
            // or params is empty then use current vertex program
            if (name.empty() || (mScriptContext.pass->getShadowReceiverVertexProgramName() == name))
            {
                mScriptContext.program = mScriptContext.pass->getShadowReceiverVertexProgram();
            }
        }

        // if context.program was not set then try to get the vertex program using the name
        // passed in params
        if (mScriptContext.program.isNull())
        {
            mScriptContext.program = GpuProgramManager::getSingleton().getByName(name);
            if (mScriptContext.program.isNull())
            {
                // Unknown program
                logParseError("Invalid shadow_receiver_vertex_program_ref entry - vertex program "
                    + name + " has not been defined.");
                return;
            }

            // Set the vertex program for this pass
            mScriptContext.pass->setShadowReceiverVertexProgram(name);
        }

        mScriptContext.isProgramShadowCaster = false;
        mScriptContext.isVertexProgramShadowReceiver = true;
		mScriptContext.isFragmentProgramShadowReceiver = false;

        // Create params? Skip this if program is not supported
        if (mScriptContext.program->isSupported())
        {
            mScriptContext.programParams = mScriptContext.pass->getShadowReceiverVertexProgramParameters();
			mScriptContext.numAnimationParametrics = 0;
        }
    }
	//-----------------------------------------------------------------------
	void MaterialScriptCompiler::parseShadowReceiverFragmentProgramRef(void)
	{
        assert(mScriptContext.pass);
		// update section
		mScriptContext.section = MSS_PROGRAM_REF;

        String name;

        // get the name of the program definition if it was set
        if (getRemainingTokensForAction() == 1)
        {
            name = getNextTokenLabel();
            StringUtil::trim(name);
        }
        // check if pass has a fragment program already
        if (mScriptContext.pass->hasShadowReceiverFragmentProgram())
        {
            // if existing pass fragment program has same name as params
            // or params is empty then use current fragment program
            if (name.empty() || (mScriptContext.pass->getShadowReceiverFragmentProgramName() == name))
            {
                mScriptContext.program = mScriptContext.pass->getShadowReceiverFragmentProgram();
            }
        }

        // if context.program was not set then try to get the fragment program using the name
        // passed in
        if (mScriptContext.program.isNull())
        {
            mScriptContext.program = GpuProgramManager::getSingleton().getByName(name);
            if (mScriptContext.program.isNull())
            {
                // Unknown program
                logParseError("Invalid shadow_receiver_fragment_program_ref entry - fragment program "
                    + name + " has not been defined.");
                return;
            }

            // Set the vertex program for this pass
            mScriptContext.pass->setShadowReceiverFragmentProgram(name);
        }

        mScriptContext.isProgramShadowCaster = false;
        mScriptContext.isVertexProgramShadowReceiver = false;
        mScriptContext.isFragmentProgramShadowReceiver = true;

		// Create params? Skip this if program is not supported
		if (mScriptContext.program->isSupported())
		{
			mScriptContext.programParams = mScriptContext.pass->getShadowReceiverFragmentProgramParameters();
			mScriptContext.numAnimationParametrics = 0;
		}
	}
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseFragmentProgramRef(void)
    {
        assert(mScriptContext.pass);
        // update section
        mScriptContext.section = MSS_PROGRAM_REF;

        String name;

        // get the name of the program definition if it was set
        if (getRemainingTokensForAction() == 1)
        {
            name = getNextTokenLabel();
            StringUtil::trim(name);
        }
        // check if pass has a fragment program already
        if (mScriptContext.pass->hasFragmentProgram())
        {
            // if existing pass fragment program has same name as params
            // or params is empty then use current fragment program
            if (name.empty() || (mScriptContext.pass->getFragmentProgramName() == name))
            {
                mScriptContext.program = mScriptContext.pass->getFragmentProgram();
            }
        }

        // if context.program was not set then try to get the fragment program using the name
        // passed in params
        if (mScriptContext.program.isNull())
        {
            mScriptContext.program = GpuProgramManager::getSingleton().getByName(name);
            if (mScriptContext.program.isNull())
            {
                // Unknown program
                logParseError("Invalid fragment_program_ref entry - fragment program "
                    + name + " has not been defined.");
                return;
            }

            // Set the vertex program for this pass
            mScriptContext.pass->setFragmentProgram(name);
        }

        // Create params? Skip this if program is not supported
        if (mScriptContext.program->isSupported())
        {
            mScriptContext.programParams = mScriptContext.pass->getFragmentProgramParameters();
			mScriptContext.numAnimationParametrics = 0;
        }
    }
    //-----------------------------------------------------------------------
	void MaterialScriptCompiler::processManualProgramParam(bool isNamed, 
		const String commandname, size_t index, const String& paramName)
    {
        // NB we assume that the first element of vecparams is taken up with either
        // the index or the parameter name, which we ignore

        // Determine type
        size_t start, dims, roundedDims, i;
        bool isReal;
        bool isMatrix4x4 = false;
        String param(getNextTokenLabel());

        StringUtil::toLowerCase(param);

        if (param == "matrix4x4")
        {
            dims = 16;
            isReal = true;
            isMatrix4x4 = true;
        }
        else if ((start = param.find("float")) != String::npos)
        {
            // find the dimensionality
            start = param.find_first_not_of("float");
            // Assume 1 if not specified
            if (start == String::npos)
            {
                dims = 1;
            }
            else
            {
                dims = StringConverter::parseInt(param.substr(start));
            }
            isReal = true;
        }
        else if ((start = param.find("int")) != String::npos)
        {
            // find the dimensionality
            start = param.find_first_not_of("int");
            // Assume 1 if not specified
            if (start == String::npos)
            {
                dims = 1;
            }
            else
            {
                dims = StringConverter::parseInt(param.substr(start));
            }
            isReal = false;
        }
        else
        {
            logParseError("Invalid " + commandname + " attribute - unrecognised "
                "parameter type " + param);
            return;
        }

        if (getRemainingTokensForAction() != dims)
        {
            logParseError("Invalid " + commandname + " attribute - you need " +
                StringConverter::toString(2 + dims) + " parameters for a parameter of "
                "type " + param);
        }

		// clear any auto parameter bound to this constant, it would override this setting
		// can cause problems overriding materials or changing default params
		if (isNamed)
			mScriptContext.programParams->clearNamedAutoConstant(paramName);
		else
			mScriptContext.programParams->clearAutoConstant(index);

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
                realBuffer[i] = getNextTokenValue();
            }
			// Fill up to multiple of 4 with zero
			for (; i < roundedDims; ++i)
			{
				realBuffer[i] = 0.0f;

			}

            if (isMatrix4x4)
            {
                // its a Matrix4x4 so pass as a Matrix4
                // use specialized setConstant that takes a matrix so matrix is transposed if required
                Matrix4 m4x4(
                    realBuffer[0],  realBuffer[1],  realBuffer[2],  realBuffer[3],
                    realBuffer[4],  realBuffer[5],  realBuffer[6],  realBuffer[7],
                    realBuffer[8],  realBuffer[9],  realBuffer[10], realBuffer[11],
                    realBuffer[12], realBuffer[13], realBuffer[14], realBuffer[15]
                    );
				if (isNamed)
					mScriptContext.programParams->setNamedConstant(paramName, m4x4);
				else
					mScriptContext.programParams->setConstant(index, m4x4);
            }
            else
            {
                // Set
				if (isNamed)
				{
					// For named, only set up to the precise number of elements
					// (no rounding to 4 elements)
					// GLSL can support sub-float4 elements and we support that
					// in the buffer now. Note how we set the 'multiple' param to 1
					mScriptContext.programParams->setNamedConstant(paramName, 
						realBuffer, dims, 1);
				}
				else
				{
	                mScriptContext.programParams->setConstant(index, 
						realBuffer, static_cast<size_t>(roundedDims * 0.25));
				}

            }


            delete [] realBuffer;
        }
        else
        {
            int* intBuffer = new int[roundedDims];
            // Do specified values
            for (i = 0; i < dims; ++i)
            {
                intBuffer[i] = static_cast<int>(getNextTokenValue());
            }
			// Fill to multiple of 4 with 0
			for (; i < roundedDims; ++i)
			{
				intBuffer[i] = 0;
			}
            // Set
			if (isNamed)
			{
				// For named, only set up to the precise number of elements
				// (no rounding to 4 elements)
				// GLSL can support sub-float4 elements and we support that
				// in the buffer now. Note how we set the 'multiple' param to 1
				mScriptContext.programParams->setNamedConstant(paramName, intBuffer, 
					dims, 1);
			}
			else
			{
				mScriptContext.programParams->setConstant(index, intBuffer, 
					static_cast<size_t>(roundedDims * 0.25));
			}
            delete [] intBuffer;
        }
    }
    //-----------------------------------------------------------------------
	void MaterialScriptCompiler::processAutoProgramParam(bool isNamed, const String commandname, 
		size_t index, const String& paramName)
    {

        String autoConstantName(getNextTokenLabel());
        // make sure param is in lower case
        StringUtil::toLowerCase(autoConstantName);

        // lookup the param to see if its a valid auto constant
        const GpuProgramParameters::AutoConstantDefinition* autoConstantDef =
            mScriptContext.programParams->getAutoConstantDefinition(autoConstantName);

        // exit with error msg if the auto constant definition wasn't found
        if (!autoConstantDef)
		{
			logParseError("Invalid " + commandname + " attribute - "
				+ autoConstantName);
			return;
		}

        // add AutoConstant based on the type of data it uses
        switch (autoConstantDef->dataType)
        {
        case GpuProgramParameters::ACDT_NONE:
			if (isNamed)
				mScriptContext.programParams->setNamedAutoConstant(paramName, autoConstantDef->acType, 0);
			else
				mScriptContext.programParams->setAutoConstant(index, autoConstantDef->acType, 0);
            break;

        case GpuProgramParameters::ACDT_INT:
            {
				// Special case animation_parametric, we need to keep track of number of times used
				if (autoConstantDef->acType == GpuProgramParameters::ACT_ANIMATION_PARAMETRIC)
				{
					if (isNamed)
						mScriptContext.programParams->setNamedAutoConstant(
							paramName, autoConstantDef->acType, mScriptContext.numAnimationParametrics++);
					else
						mScriptContext.programParams->setAutoConstant(
							index, autoConstantDef->acType, mScriptContext.numAnimationParametrics++);
				}
				// Special case texture projector - assume 0 if data not specified
				else if (autoConstantDef->acType == GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX
					&& getRemainingTokensForAction() == 0)
				{
					if (isNamed)
						mScriptContext.programParams->setNamedAutoConstant(
							paramName, autoConstantDef->acType, 0);
					else
						mScriptContext.programParams->setAutoConstant(
							index, autoConstantDef->acType, 0);

				}
				else
				{

					if (getRemainingTokensForAction() != 1)
					{
						logParseError("Invalid " + commandname + " attribute - "
							"expected 3 parameters.");
						return;
					}

					size_t extraParam = static_cast<size_t>(getNextTokenValue());
					if (isNamed)
						mScriptContext.programParams->setNamedAutoConstant(
							paramName, autoConstantDef->acType, extraParam);
					else
						mScriptContext.programParams->setAutoConstant(
							index, autoConstantDef->acType, extraParam);
				}
            }
            break;

        case GpuProgramParameters::ACDT_REAL:
            {
                // special handling for time
                if (autoConstantDef->acType == GpuProgramParameters::ACT_TIME ||
                    autoConstantDef->acType == GpuProgramParameters::ACT_FRAME_TIME)
                {
                    Real factor = 1.0f;
                    if (getRemainingTokensForAction() == 1)
                    {
                        factor = getNextTokenValue();
                    }

					if (isNamed)
						mScriptContext.programParams->setNamedAutoConstantReal(
							paramName, autoConstantDef->acType, factor);
					else
						mScriptContext.programParams->setAutoConstantReal(
							index, autoConstantDef->acType, factor);
                }
                else // normal processing for auto constants that take an extra real value
                {
                    if (getRemainingTokensForAction() != 1)
                    {
                        logParseError("Invalid " + commandname + " attribute - "
                            "expected 3 parameters.");
                        return;
                    }

			        const Real rData = getNextTokenValue();
					if (isNamed)
						mScriptContext.programParams->setNamedAutoConstantReal(
							paramName, autoConstantDef->acType, rData);
					else
						mScriptContext.programParams->setAutoConstantReal(
							index, autoConstantDef->acType, rData);
                }
            }
            break;

        } // end switch


    }

    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseParamIndexed(void)
    {
        if (mScriptContext.section == MSS_DEFAULT_PARAMETERS)
        {
            // save the pass2 token que position for later processing
            mScriptContext.pendingDefaultParams.push_back(getPass2TokenQuePosition());
            return;
        }

        // NB skip this if the program is not supported or could not be found
        if (mScriptContext.program.isNull() || !mScriptContext.program->isSupported())
        {
            return;
        }

        // Get start index
        const size_t index = static_cast<size_t>(getNextTokenValue());

        processManualProgramParam(false, "param_indexed", index);

    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseParamIndexedAuto(void)
    {
        if (mScriptContext.section == MSS_DEFAULT_PARAMETERS)
        {
            // save the pass2 token que position for later processing
            mScriptContext.pendingDefaultParams.push_back(getPass2TokenQuePosition());
            return;
        }
        // NB skip this if the program is not supported or could not be found
        if (mScriptContext.program.isNull() || !mScriptContext.program->isSupported())
        {
            return;
        }
        // Get start index
        const size_t index = static_cast<size_t>(getNextTokenValue());

        processAutoProgramParam(false, "param_indexed_auto", index);

    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseParamNamed(void)
    {
        if (mScriptContext.section == MSS_DEFAULT_PARAMETERS)
        {
            // save the pass2 token que position for later processing
            mScriptContext.pendingDefaultParams.push_back(getPass2TokenQuePosition());
            return;
        }
        // NB skip this if the program is not supported or could not be found
        if (mScriptContext.program.isNull() || !mScriptContext.program->isSupported())
        {
            return;
        }

        // Get start index from name
        const String& paramName = getNextTokenLabel();
        try {
            const GpuConstantDefinition& def =
				mScriptContext.programParams->getConstantDefinition(paramName);
        }
        catch (Exception& e)
        {
            logParseError("Invalid param_named attribute - " + e.getDescription());
            return;
        }

        processManualProgramParam(true, "param_named", 0, paramName);

    }
    //-----------------------------------------------------------------------
    void MaterialScriptCompiler::parseParamNamedAuto(void)
    {
        if (mScriptContext.section == MSS_DEFAULT_PARAMETERS)
        {
            // save the pass2 token que position for later processing
            mScriptContext.pendingDefaultParams.push_back(getPass2TokenQuePosition());
            return;
        }
        // NB skip this if the program is not supported or could not be found
        if (mScriptContext.program.isNull() || !mScriptContext.program->isSupported())
        {
            return;
        }

        // Get start index from name
        const String& paramName = getNextTokenLabel();
        try {
			const GpuConstantDefinition& def =
				mScriptContext.programParams->getConstantDefinition(paramName);
        }
        catch (Exception& e)
        {
            logParseError("Invalid param_named_auto attribute - " + e.getDescription());
            return;
        }

        processAutoProgramParam(true, "param_named_auto", 0, paramName);
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

			/*  attempt to create the gpu program.  This could fail if the gpu program name is
                already being used so must catch the exception and see if its possible to override
                the existing gpu program.
            */
			try
			{
                gp = GpuProgramManager::getSingleton().
                    createProgram(def->name, mScriptContext.groupName, def->source,
                        def->progType, def->syntax);
			}
			catch (Exception& e)
			{
			    bool exceptionHandled = false;

			    // attempt recovery if exception was caused by duplicate program name found
                if (mScriptContext.allowOverride && (e.getNumber() == Exception::ERR_DUPLICATE_ITEM))
                {
                    // the gpu program already exists so unload it and then change source and syntax
                    // the syntax code is not checked here
                    gp = GpuProgramManager::getSingleton().getByName(def->name);
                    if (!gp.isNull())
                    {
                        if (gp->getType() == def->progType)
                        {
                            logParseError("gpu asm program: " + def->name +
                                ", defined in " + gp->getOrigin() +
                                ", was overwritten by current gpu program definition being parsed with same name");
                            gp->unload();
                            /* updat group ownership since the original gpu program
                               might have been in a different resource group
                            */
                            gp->changeGroupOwnership(mScriptContext.groupName);
                            gp->setSyntaxCode(def->syntax);
                            gp->setSource(def->source);
                            // Need to do something about the existing default parameters
                            gp->load();
                            exceptionHandled = true;
                        }
                        else
                        {
                            // Don't create or override the gpu program due to incompitble program type so ignore the new definition
                            logParseError("gpu asm program: " + def->name + " program type conflict with current gpu program definition being parsed with same name");
                        }
                    }
                }

                if (!exceptionHandled)
                {
                    // exception message already logged so no sense logging them in here and making two entries in the log
                    logParseError("gpu asm program: " + def->name +
                        " was previously defined and can not be overridden.\n"
                        "Material Manager script override was not enabled." );
                    mScriptContext.program.setNull();
                    mScriptContext.programParams.setNull();
                }
			}
		}
		else
		{
			// High-level program
			// Validate
			if (def->source.empty() && def->language != "unified")
			{
				logParseError("Invalid program definition for " + def->name +
					", you must specify a source file.");
			}
			/*  attempt to create the high level gpu program.  This could fail if the gpu program name is
                already being used so must catch the exception and see if its possible to override
                the existing gpu program.
            */
            HighLevelGpuProgramPtr hgp;
            try
            {
                hgp = HighLevelGpuProgramManager::getSingleton().
                    createProgram(def->name, mScriptContext.groupName,
                        def->language, def->progType);
            }
            catch (Exception& e)
            {
			    bool exceptionHandled = false;
			    // attempt recovery if exception was caused by duplicate gpu program name found
                if (mScriptContext.allowOverride && (e.getNumber() == Exception::ERR_DUPLICATE_ITEM))
                {
                    hgp = HighLevelGpuProgramManager::getSingleton().getByName(def->name);
                    if (!hgp.isNull())
                    {
                        // the language and program type must still be the same otherwise reuse is not possible
                        if ((hgp->getLanguage() != def->language) || (hgp->getType() != def->progType))
                        {
                            logParseError("gpu high level program: " + def->name +
                                " already exists and can not be redefined with the language or program type defined in the material script");
                        }
                        else
                        {
                            logParseError("gpu high level program: " + def->name +
                                ", defined in " + hgp->getOrigin() +
                                ", was overwritten by current gpu program definition being parsed with same name");
                            // unload the previous source that was compiled
                            hgp->unload();
                            /* update group ownership since the original gpu program
                               might have been in a different resource group
                            */
                            hgp->changeGroupOwnership(mScriptContext.groupName);
                            exceptionHandled = true;
                        }
                    }
                }

                if (!exceptionHandled)
                {
                    // exception message already logged so no sense logging them in here and making two entries in the log
                    logParseError("gpu high level program: " + def->name +
                        " was previously defined and can not be overridden.\n"
                        "Material Manager script override was not enabled." );
                    mScriptContext.program.setNull();
                    mScriptContext.programParams.setNull();
                }
            }
            // if the high level gpu program dosn't exist then don't continue
            if (hgp.isNull())
                return;

            // Assign to generalised version
            gp = hgp;

            // Set source file
            hgp->setSourceFile(def->source);

            // Set custom parameters
			std::vector<std::pair<String, String> >::const_iterator i, iend;
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

        // don't continue processing gpu program if it doesn't exist
        if (gp.isNull())
            return;

        // Set skeletal animation option
        gp->setSkeletalAnimationIncluded(def->supportsSkeletalAnimation);
		// Set morph animation option
		gp->setMorphAnimationIncluded(def->supportsMorphAnimation);
		// Set pose animation option
		gp->setPoseAnimationIncluded(def->supportsPoseAnimation);
        // Set vertex texture usage
        gp->setVertexTextureFetchRequired(def->usesVertexTextureFetch);

		// set origin
		gp->_notifyOrigin(mSourceName);

        // Set up to receive default parameters
        if (gp->isSupported()
            && !mScriptContext.pendingDefaultParams.empty())
        {
            mScriptContext.programParams = gp->getDefaultParameters();
			mScriptContext.numAnimationParametrics = 0;
            mScriptContext.program = gp;
            size_t i, iend;
            iend = mScriptContext.pendingDefaultParams.size();
            for (i = 0; i < iend; ++i)
            {
                // invoke the action for the pending default param in the token que
                setPass2TokenQuePosition(mScriptContext.pendingDefaultParams[i], true);
            }
            // Reset
            mScriptContext.program.setNull();
            mScriptContext.programParams.setNull();
        }

	}


}

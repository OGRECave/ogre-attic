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
        //                           <Lod_Distances_Def> | <transparency_casts_shadows_def>
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

        // <Pass_Def> ::= "pass" [<Label>] "{" {<Pass_Properties>} "}"
        _rule_ mid_PASS_DEF, "<Pass_Def>"
	        _is_ mid_PASS, "pass"
	        _optional_ mid_LABEL _nt_
	        _and_ _no_token_, "{"
	        _repeat_ mid_PASS_PROPERTIES _nt_
	        _and_ _no_token_, "}"
	        _end_
        	
        // <Pass_Properties> ::= <Ambient_Def> | <Diffuse_Def> | <Specular_Def> | <Emmisive_Def> |
        //                       <Scene_Blend_Def> | <Depth_Check_Def> | <Depth_Write_Def> | 
        _rule_ mid_PASS_PROPERTIES, "<Pass_Properties>"
            _is_ mid_AMBIENT_DEF _nt_
            _or_ mid_DIFFUSE_DEF _nt_
            _or_ mid_SPECULAR_DEF _nt_
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
        	
        // <Character> ::= [abcdefghijklmnopqrstuvwxyz] | [ABCDEFGHIJKLMNOPQRSTUVWXYZ] | [$#%!_*&]
        _rule_ mid_CHARACTER, "<Character>"
	        _is_ _character_, "abcdefghijklmnopqrstuvwxyz"
	        _or_ _character_, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	        _or_ _character_, "$#%!_*&"
	        _end_
     
        // <Number> ::= [0123456789]
        _rule_ mid_NUMBER, "<Number>"
	        _is_ _character_, "0123456789"
	        _end_

    };
    //-----------------------------------------------------------------------

}
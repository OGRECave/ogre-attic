/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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
-----------------------------------------------------------------------------
*/

#include "OgreStableHeaders.h"
#include "OgreShadowVolumeExtrudeProgram.h"
#include "OgreString.h"

namespace Ogre {

    String ShadowVolumeExtrudeProgram::mPointArbvp1 = 
        "!!ARBvp1.0\n"
        "PARAM c5 = { 0, 0, 0, 0 };\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0.xyz, v16.xyzx, -c4.xyzx;\n"
        "MOV R0.w, c5.x;\n"
        "MAD R0, v24.x, c4, R0;\n"
        "DP4 result.position.x, c0[0], R0;\n"
        "DP4 result.position.y, c0[1], R0;\n"
        "DP4 result.position.z, c0[2], R0;\n"
        "DP4 result.position.w, c0[3], R0;\n"
        "END\n";

    String ShadowVolumeExtrudeProgram::mPointVs_1_1 = 
        "vs_1_1\n"
        "def c5, 0, 0, 0, 0\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0.xyz, v0.xyz, -c4.xyz\n"
        "mov r0.w, c5.x\n"
        "mad r0, v7.x, c4, r0\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";

    String ShadowVolumeExtrudeProgram::mDirArbvp1 = 
        "!!ARBvp1.0\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0, v16, c4;\n"
        "MAD R0, v24.x, R0, -c4;\n"
        "DP4 result.position.x, c0[0], R0;\n"
        "DP4 result.position.y, c0[1], R0;\n"
        "DP4 result.position.z, c0[2], R0;\n"
        "DP4 result.position.w, c0[3], R0;\n"
        "END\n";

    String ShadowVolumeExtrudeProgram::mDirVs_1_1 = 
        "vs_1_1\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0, v0, c4\n"
        "mad r0, v7.x, r0, -c4\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n";


    String ShadowVolumeExtrudeProgram::mPointArbvp1Debug = 
        "!!ARBvp1.0\n"
        "PARAM c5 = { 0, 0, 0, 0 };\n"
        "PARAM c6 = { 1, 1, 1, 1 };\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0.xyz, v16.xyzx, -c4.xyzx;\n"
        "MOV R0.w, c5.x;\n"
        "MAD R0, v24.x, c4, R0;\n"
        "DP4 result.position.x, c0[0], R0;\n"
        "DP4 result.position.y, c0[1], R0;\n"
        "DP4 result.position.z, c0[2], R0;\n"
        "DP4 result.position.w, c0[3], R0;\n"
        "MOV result.color.front.primary, c6.x;\n"
        "END\n";

    String ShadowVolumeExtrudeProgram::mPointVs_1_1Debug = 
        "vs_1_1\n"
        "def c5, 0, 0, 0, 0\n"
        "def c6, 1, 1, 1, 1\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0.xyz, v0.xyz, -c4.xyz\n"
        "mov r0.w, c5.x\n"
        "mad r0, v7.x, c4, r0\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n"
        "mov oD0, c6.x\n";

    String ShadowVolumeExtrudeProgram::mDirArbvp1Debug = 
        "!!ARBvp1.0\n"
        "PARAM c5 = { 1, 1, 1, 1};\n"
        "TEMP R0;\n"
        "ATTRIB v24 = vertex.texcoord[0];\n"
        "ATTRIB v16 = vertex.position;\n"
        "PARAM c0[4] = { program.local[0..3] };\n"
        "PARAM c4 = program.local[4];\n"
        "ADD R0, v16, c4;\n"
        "MAD R0, v24.x, R0, -c4;\n"
        "DP4 result.position.x, c0[0], R0;\n"
        "DP4 result.position.y, c0[1], R0;\n"
        "DP4 result.position.z, c0[2], R0;\n"
        "DP4 result.position.w, c0[3], R0;\n"
        "MOV result.color.front.primary, c5.x;"
        "END\n";

    String ShadowVolumeExtrudeProgram::mDirVs_1_1Debug = 
        "vs_1_1\n"
        "def c5, 1, 1, 1, 1\n"
        "dcl_texcoord0 v7\n"
        "dcl_position v0\n"
        "add r0, v0, c4\n"
        "mad r0, v7.x, r0, -c4\n"
        "dp4 oPos.x, c0, r0\n"
        "dp4 oPos.y, c1, r0\n"
        "dp4 oPos.z, c2, r0\n"
        "dp4 oPos.w, c3, r0\n"
        "mov oD0, c5.x\n";


}

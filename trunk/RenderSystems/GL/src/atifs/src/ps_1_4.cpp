/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2004 The OGRE Team
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

 
 //---------------------------------------------------------------------------
#include "ps_1_4.h"
#include "OgreGLATIFSInit.h"

//---------------------------------------------------------------------------

/* ********************* START OF PS_1_4 CLASS STATIC DATA ********************************* */

PS_1_4::TokenInstType PS_1_4::InstTypeLib[] = {

 {"ANY", id_ANY},
 {"REGISTER", id_REGISTER},
 {"CONSTANT", id_CONSTANT},
 {"COLOR", id_COLOR},
 {"TEXTURE", id_TEXTURE},
 {"OPINST", id_OPINST},
 {"MASK", id_MASK},
 {"TEXSWIZZLE", id_TEXSWIZZLE},
 {"DSTMOD", id_DSTMOD},
 {"ARGMOD", id_ARGMOD},
 {"NUMVAL", id_NUMVAL},
 {"SEPERATOR", id_SEPERATOR}
};

// library of built in symbol types

PS_1_4::ASMSymbolDef PS_1_4::PS_1_4_ASMSymbolTypeLib[] = {
	// pixel shader versions supported
	{id_ANY, sid_PS_1_4, GL_NONE, 0, rwa_NONE, 0, id_ANY, id_OPINST, ckp_PS_BASE, ckp_PS_1_4},
	{id_ANY, sid_PS_1_1, GL_NONE, 0, rwa_NONE, 0, id_ANY, id_OPINST, ckp_PS_BASE, ckp_PS_1_1},
	{id_ANY, sid_PS_1_2, GL_NONE, 0, rwa_NONE, 0, id_ANY, id_OPINST, ckp_PS_BASE, ckp_PS_1_2 + ckp_PS_1_1},
	{id_ANY, sid_PS_1_3, GL_NONE, 0, rwa_NONE, 0, id_ANY, id_OPINST, ckp_PS_BASE, ckp_PS_1_3 + ckp_PS_1_2 + ckp_PS_1_1},

	// PS_BASE

	// constants
	{id_CONSTANT, sid_C0, GL_CON_0_ATI, 0, rwa_READ, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_BASE},
	{id_CONSTANT, sid_C1, GL_CON_1_ATI, 0, rwa_READ, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_BASE},
	{id_CONSTANT, sid_C2, GL_CON_2_ATI, 0, rwa_READ, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_BASE},
	{id_CONSTANT, sid_C3, GL_CON_3_ATI, 0, rwa_READ, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_BASE},
	{id_CONSTANT, sid_C4, GL_CON_4_ATI, 0, rwa_READ, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_BASE},
	{id_CONSTANT, sid_C5, GL_CON_5_ATI, 0, rwa_READ, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_BASE},
	{id_CONSTANT, sid_C6, GL_CON_6_ATI, 0, rwa_READ, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_BASE},
	{id_CONSTANT, sid_C7, GL_CON_7_ATI, 0, rwa_READ, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_BASE},

	// colour
	{id_COLOR, sid_V0, GL_PRIMARY_COLOR_ARB, 0, rwa_READ, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_BASE},
	{id_COLOR, sid_V1, GL_SECONDARY_INTERPOLATOR_ATI, 0, rwa_READ, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_BASE},

	// instruction ops
	{id_OPINST, sid_ADD, GL_ADD_ATI, 3, rwa_NONE, 0, id_OPLEFT, id_DSTMOD | id_TEMPREGISTERS, ckp_PS_BASE},
	{id_OPINST, sid_SUB, GL_SUB_ATI, 3, rwa_NONE, 0, id_OPLEFT, id_DSTMOD | id_TEMPREGISTERS, ckp_PS_BASE},
	{id_OPINST, sid_MUL, GL_MUL_ATI, 3, rwa_NONE, 0, id_OPLEFT, id_DSTMOD | id_TEMPREGISTERS, ckp_PS_BASE},
	{id_OPINST, sid_MAD, GL_MAD_ATI, 4, rwa_NONE, 0, id_OPLEFT, id_DSTMOD | id_TEMPREGISTERS, ckp_PS_BASE},
	{id_OPINST, sid_LRP, GL_LERP_ATI, 4, rwa_NONE, 0, id_OPLEFT, id_DSTMOD | id_TEMPREGISTERS, ckp_PS_BASE},
	{id_OPINST, sid_MOV, GL_MOV_ATI, 2, rwa_NONE, 0, id_OPLEFT, id_DSTMOD | id_TEMPREGISTERS, ckp_PS_BASE},
	{id_OPINST, sid_CMP, GL_CND0_ATI, 4, rwa_NONE, 0, id_OPLEFT, id_DSTMOD | id_TEMPREGISTERS, ckp_PS_BASE},
	{id_OPINST, sid_CND, GL_CND_ATI, 4, rwa_NONE, 0, id_OPLEFT, id_DSTMOD | id_TEMPREGISTERS, ckp_PS_BASE},
	{id_OPINST, sid_DP3, GL_DOT3_ATI, 3, rwa_NONE, 0, id_OPLEFT, id_DSTMOD | id_TEMPREGISTERS, ckp_PS_BASE},
	{id_OPINST, sid_DP4, GL_DOT4_ATI, 3, rwa_NONE, 0, id_OPLEFT, id_DSTMOD | id_TEMPREGISTERS, ckp_PS_BASE},

	{id_OPINST, sid_DEF, GL_NONE, 5, rwa_NONE, 0, id_OPLEFT, id_CONSTANT, ckp_PS_BASE},

	// Masks
	{id_MASK, sid_R, GL_RED_BIT_ATI, 0, rwa_NONE, 0, id_MASKREPLEFT, id_MASKREPRIGHT, ckp_PS_BASE},
	{id_MASK, sid_RA, GL_RED_BIT_ATI | ALPHA_BIT, 0, rwa_NONE, 0, id_REGISTER | id_ARGMOD, id_SEPERATOR | id_DSTMOD, ckp_PS_BASE},
	{id_MASK, sid_G, GL_GREEN_BIT_ATI, 0, rwa_NONE, 0, id_MASKREPLEFT, id_MASKREPRIGHT, ckp_PS_BASE},
	{id_MASK, sid_GA, GL_GREEN_BIT_ATI | ALPHA_BIT, 0, rwa_NONE, 0, id_REGISTER | id_ARGMOD, id_SEPERATOR | id_OPINST | id_DSTMOD, ckp_PS_BASE},
	{id_MASK, sid_B, GL_BLUE_BIT_ATI, 0, rwa_NONE, 0, id_MASKREPLEFT, id_MASKREPRIGHT, ckp_PS_BASE},
	{id_MASK, sid_BA, GL_BLUE_BIT_ATI | ALPHA_BIT, 0, rwa_NONE, 0, id_TEMPREGISTERS | id_ARGMOD, id_SEPERATOR, ckp_PS_BASE},
	{id_MASK, sid_A, ALPHA_BIT, 0, rwa_NONE, 0, id_MASKREPLEFT, id_MASKREPRIGHT, ckp_PS_BASE},
	{id_MASK, sid_RGBA, RGB_BITS | ALPHA_BIT, 0, rwa_NONE, 0, id_TEMPREGISTERS | id_ARGMOD, id_SEPERATOR, ckp_PS_BASE},
	{id_MASK, sid_RGB, RGB_BITS, 0, rwa_NONE, 0, id_TEMPREGISTERS | id_ARGMOD, id_SEPERATOR, ckp_PS_BASE},
	{id_MASK, sid_RG, GL_RED_BIT_ATI | GL_GREEN_BIT_ATI, 0, rwa_NONE, 0, id_TEMPREGISTERS | id_ARGMOD, id_SEPERATOR, ckp_PS_BASE},
	{id_MASK, sid_RGA, GL_RED_BIT_ATI | GL_GREEN_BIT_ATI | ALPHA_BIT, 0, rwa_NONE, 0, id_REGISTER | id_ARGMOD, id_SEPERATOR, ckp_PS_BASE},
	{id_MASK, sid_RB, GL_RED_BIT_ATI | GL_BLUE_BIT_ATI, 0, rwa_NONE, 0, id_TEMPREGISTERS | id_ARGMOD, id_SEPERATOR, ckp_PS_BASE},
	{id_MASK, sid_RBA, GL_RED_BIT_ATI | GL_BLUE_BIT_ATI | ALPHA_BIT, 0, rwa_NONE, 0, id_TEMPREGISTERS | id_ARGMOD, id_SEPERATOR, ckp_PS_BASE},
	{id_MASK, sid_GB, GL_GREEN_BIT_ATI | GL_BLUE_BIT_ATI, 0, rwa_NONE, 0, id_TEMPREGISTERS | id_ARGMOD, id_SEPERATOR, ckp_PS_BASE},
	{id_MASK, sid_GBA, GL_GREEN_BIT_ATI | GL_BLUE_BIT_ATI | ALPHA_BIT, 0, rwa_NONE, 0, id_TEMPREGISTERS | id_ARGMOD, id_SEPERATOR, ckp_PS_BASE},


	// modifiers
	{id_DSTMOD, sid_X2, GL_2X_BIT_ATI, 0, rwa_NONE, 0, id_OPINST | id_ARGUMENT | id_MASK, id_SEPERATOR | id_REGISTER | id_OPINST, ckp_PS_BASE},
	{id_DSTMOD, sid_X4, GL_4X_BIT_ATI, 0, rwa_NONE, 0, id_OPINST, id_TEMPREGISTERS, ckp_PS_BASE},
	{id_DSTMOD, sid_D2, GL_HALF_BIT_ATI, 0, rwa_NONE, 0, id_OPINST, id_SEPERATOR | id_TEMPREGISTERS , ckp_PS_BASE},
	{id_DSTMOD, sid_SAT, GL_SATURATE_BIT_ATI, 0, rwa_NONE, 0, id_OPINST | id_DSTMOD, id_TEMPREGISTERS, ckp_PS_BASE},

	// argument modifiers
	{id_ARGMOD, sid_BIAS, GL_BIAS_BIT_ATI, 0, rwa_NONE, 0, id_ARGUMENT | id_ARGMOD | id_MASK, id_ANY, ckp_PS_BASE},
	{id_ARGMOD, sid_INVERT, GL_COMP_BIT_ATI, 0, rwa_NONE, 0, id_SEPERATOR | id_ARGMOD, id_ARGUMENT | id_ARGMOD, ckp_PS_BASE},
	{id_ARGMOD, sid_NEGATE, GL_NEGATE_BIT_ATI, 0, rwa_NONE, 0, id_SEPERATOR, id_ARGUMENT | id_ARGMOD, ckp_PS_BASE},
	{id_ARGMOD, sid_BX2, GL_2X_BIT_ATI | GL_BIAS_BIT_ATI, 0, rwa_NONE, 0, id_ARGUMENT | id_ARGMOD | id_MASK, id_ANY, ckp_PS_BASE, ckp_PS_BASE},

	// seperator characters
	{id_SEPERATOR, sid_COMMA, GL_NONE, 0, rwa_NONE, 0, id_ARGUMENT | id_ARGMOD | id_NUMVAL | id_MASK, id_ARGUMENT | id_ARGMOD | id_NUMVAL, ckp_PS_BASE},
	{id_NUMVAL, sid_VALUE, GL_NONE, 0, rwa_NONE, 0, id_CONSTANT | id_SEPERATOR, id_SEPERATOR | id_OPINST, ckp_PS_BASE},

	// PS_1_4
	// temp R/W registers
	{id_REGISTER, sid_R0, GL_REG_0_ATI, 0, rwa_READ | rwa_WRITE, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_1_4},
	{id_REGISTER, sid_R1, GL_REG_1_ATI, 0, rwa_READ | rwa_WRITE, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_1_4},
	{id_REGISTER, sid_R2, GL_REG_2_ATI, 0, rwa_READ | rwa_WRITE, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_1_4},
	{id_REGISTER, sid_R3, GL_REG_3_ATI, 0, rwa_READ | rwa_WRITE, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_1_4},
	{id_REGISTER, sid_R4, GL_REG_4_ATI, 0, rwa_READ | rwa_WRITE, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_1_4},
	{id_REGISTER, sid_R5, GL_REG_5_ATI, 0, rwa_READ | rwa_WRITE, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_1_4},

	// textures
	{id_TEXTURE, sid_T0, GL_TEXTURE0_ARB, 0, rwa_READ, 0,id_SEPERATOR, id_TEXSWIZZLE | id_OPINST, ckp_PS_1_4},
	{id_TEXTURE, sid_T1, GL_TEXTURE1_ARB, 0, rwa_READ, 0,id_SEPERATOR, id_TEXSWIZZLE | id_OPINST, ckp_PS_1_4},
	{id_TEXTURE, sid_T2, GL_TEXTURE2_ARB, 0, rwa_READ, 0,id_SEPERATOR, id_TEXSWIZZLE | id_OPINST, ckp_PS_1_4},
	{id_TEXTURE, sid_T3, GL_TEXTURE3_ARB, 0, rwa_READ, 0,id_SEPERATOR, id_TEXSWIZZLE | id_OPINST, ckp_PS_1_4},
	{id_TEXTURE, sid_T4, GL_TEXTURE4_ARB, 0, rwa_READ, 0,id_SEPERATOR, id_TEXSWIZZLE | id_OPINST, ckp_PS_1_4},
	{id_TEXTURE, sid_T5, GL_TEXTURE5_ARB, 0, rwa_READ, 0,id_SEPERATOR, id_TEXSWIZZLE | id_OPINST, ckp_PS_1_4},

	{id_OPINST, sid_DP2ADD, GL_DOT2_ADD_ATI, 4, rwa_NONE, 0, id_OPLEFT, id_DSTMOD | id_REGISTER, ckp_PS_1_4},
	// modifiers
	{id_DSTMOD, sid_X8, GL_8X_BIT_ATI, 0, rwa_NONE, 0, id_OPINST, id_REGISTER, ckp_PS_1_4},
	{id_DSTMOD, sid_D8, GL_EIGHTH_BIT_ATI, 0, rwa_NONE, 0, id_OPINST, id_REGISTER, ckp_PS_1_4},
	{id_DSTMOD, sid_D4, GL_QUARTER_BIT_ATI, 0, rwa_NONE, 0, id_OPINST, id_TEMPREGISTERS, ckp_PS_1_4},

	// instructions
	{id_OPINST, sid_TEXCRD, GL_NONE, 2, rwa_NONE, 0, id_OPLEFT, id_DSTMOD | id_REGISTER, ckp_PS_1_4},
	{id_OPINST, sid_TEXLD, GL_NONE, 2, rwa_NONE, 0, id_OPLEFT, id_DSTMOD | id_REGISTER, ckp_PS_1_4},

	// texture swizzlers
	{id_TEXSWIZZLE, sid_STRDR, GL_SWIZZLE_STR_DR_ATI, 0, rwa_NONE, 0, id_TEXTURE, id_OPINST, ckp_PS_1_4},
	{id_TEXSWIZZLE, sid_STQDQ, GL_SWIZZLE_STQ_DQ_ATI, 0, rwa_NONE, 0, id_TEXTURE, id_OPINST, ckp_PS_1_4},

	{id_ANY, sid_PHASE, GL_NONE, 0, rwa_NONE, 0, id_ANY, id_OPINST, ckp_PS_1_4},

	// PS_1_1 
	// temp R/W registers
	// r0, r1 are mapped to r4, r5
	// t0 to t3 are mapped to r0 to r3
	{id_REGISTER, sid_1R0, GL_REG_4_ATI, 0, rwa_READ | rwa_WRITE, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_1_1},
	{id_REGISTER, sid_1R1, GL_REG_5_ATI, 0, rwa_READ | rwa_WRITE, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_1_1},
	{id_TEXREGISTER, sid_1T0, GL_REG_0_ATI, 0, rwa_READ | rwa_WRITE, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_1_1},
	{id_TEXREGISTER, sid_1T1, GL_REG_1_ATI, 0, rwa_READ | rwa_WRITE, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_1_1},
	{id_TEXREGISTER, sid_1T2, GL_REG_2_ATI, 0, rwa_READ | rwa_WRITE, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_1_1},
	{id_TEXREGISTER, sid_1T3, GL_REG_3_ATI, 0, rwa_READ | rwa_WRITE, 0, id_PARMETERLEFT, id_PARMETERRIGHT, ckp_PS_1_1},

	// instructions common to PS_1_1, PS_1_2, PS_1_3
	{id_OPINST, sid_TEX, GL_NONE, 1, rwa_NONE, 0, id_OPLEFT, id_TEXREGISTER, ckp_PS_1_1},
	{id_OPINST, sid_TEXCOORD, GL_NONE, 1, rwa_NONE, 0, id_OPLEFT, id_TEXREGISTER, ckp_PS_1_1},
	{id_OPINST, sid_TEXM3X2PAD, GL_NONE, 2, rwa_NONE, 0, id_OPLEFT, id_TEXREGISTER, ckp_PS_1_1},
	{id_OPINST, sid_TEXM3X2TEX, GL_NONE, 2, rwa_NONE, 0, id_OPLEFT, id_TEXREGISTER, ckp_PS_1_1},
	{id_OPINST, sid_TEXM3X3PAD, GL_NONE, 2, rwa_NONE, 0, id_OPLEFT, id_TEXREGISTER, ckp_PS_1_1},
	{id_OPINST, sid_TEXM3X3TEX, GL_NONE, 2, rwa_NONE, 0, id_OPLEFT, id_TEXREGISTER, ckp_PS_1_1},
	{id_OPINST, sid_TEXM3X3SPEC, GL_NONE, 3, rwa_NONE, 0, id_OPLEFT, id_TEXREGISTER, ckp_PS_1_1},
	{id_OPINST, sid_TEXM3X3VSPEC, GL_NONE, 3, rwa_NONE, 0, id_OPLEFT, id_TEXREGISTER, ckp_PS_1_1},
	{id_OPINST, sid_TEXREG2AR, GL_NONE, 2, rwa_NONE, 0, id_OPLEFT, id_TEXREGISTER, ckp_PS_1_2},
	{id_OPINST, sid_TEXREG2GB, GL_NONE, 2, rwa_NONE, 0, id_OPLEFT, id_TEXREGISTER, ckp_PS_1_2},

	// PS_1_2 & PS_1_3
	{id_OPINST, sid_TEXREG2RGB, GL_NONE, 2, rwa_NONE, 0, id_OPLEFT, id_TEXREGISTER, ckp_PS_1_2},
	{id_OPINST, sid_TEXDP3, GL_NONE, 2, rwa_NONE, 0, id_OPLEFT, id_TEXREGISTER, ckp_PS_1_1},
	{id_OPINST, sid_TEXDP3TEX, GL_NONE, 2, rwa_NONE, 0, id_OPLEFT, id_TEXREGISTER, ckp_PS_1_1},
	

	// Common
	{id_ANY, sid_SKIP, GL_NONE, 0, rwa_NONE, 0, id_ANY, id_OPINST, ckp_PS_BASE},

};

// library of all the text aliases for each symbol type
PS_1_4::ASMSymbolText PS_1_4::PS_1_4_ASMSymbolTextLib[] = {
	{"ps.1.4", sid_PS_1_4}, {"ps.1.1", sid_PS_1_1}, {"ps.1.2", sid_PS_1_2}, {"ps.1.3", sid_PS_1_3},

	// symbols common to all ps flavors
	{"c0", sid_C0}, {"c1", sid_C1}, {"c2", sid_C2}, {"c3", sid_C3}, {"c4", sid_C4},
	{"c5", sid_C5}, {"c6", sid_C6}, {"c7", sid_C7},
	{"v0", sid_V0}, {"v1", sid_V1},

	{"add", sid_ADD}, {"sub", sid_SUB}, {"mul", sid_MUL}, {"mad", sid_MAD}, {"lrp", sid_LRP},
	{"mov", sid_MOV}, {"cmp", sid_CMP}, {"cnd", sid_CND},  {"dp3", sid_DP3},
	{"dp4", sid_DP4},

	{"def", sid_DEF},

	{"_x2", sid_X2}, {"_x4", sid_X4},
	{"_d2", sid_D2}, {"_d4", sid_D4},
	{"_sat", sid_SAT},

	{"_bias", sid_BIAS}, {"1-", sid_INVERT}, {"1 -", sid_INVERT}, {"-", sid_NEGATE}, {"_bx2", sid_BX2},
	{",", sid_COMMA},
	{"value", sid_VALUE},
	{"+", sid_SKIP}, 

	// PS_1_4 symbols
	{"r0", sid_R0}, {"r1", sid_R1}, {"r2", sid_R2}, {"r3", sid_R3}, {"r4", sid_R4},
	{"r5", sid_R5},
	{"t0", sid_T0}, {"t1", sid_T1}, {"t2", sid_T2}, {"t3", sid_T3}, {"t4", sid_T4},
	{"t5", sid_T5},
	{"texcrd", sid_TEXCRD}, {"texld", sid_TEXLD},
	{"dp2", sid_DP2ADD},
	{".r", sid_R}, {".x", sid_R}, {".s", sid_R}, {".ra", sid_RA}, {".xw", sid_RA}, {".sq", sid_RA},
	{".g", sid_G}, {".y", sid_G}, {".t", sid_G}, {".ga", sid_GA}, {".yw", sid_GA}, {".tq", sid_GA},
	{".b", sid_B}, {".z", sid_B}, {".r", sid_B}, {".ba", sid_BA}, {".zw", sid_BA}, {".rw", sid_BA},
	{".a", sid_A}, {".w", sid_A}, {".q", sid_A}, {".rgba", sid_RGBA}, {".xyzw", sid_RGBA},
	{".strq", sid_RGBA}, {".rgb", sid_RGB}, {".xyz", sid_RGB}, {".str", sid_RGB}, {".rg", sid_RG},
	{".xy", sid_RG}, {".st", sid_RG}, {".rga", sid_RGA}, {".xyw", sid_RGA},  {".stq", sid_RGA},
	{".rb", sid_RB}, {".xz", sid_RB}, {".sr", sid_RB}, {".rba", sid_RBA}, {".xzw", sid_RBA},
	{".srq", sid_RBA}, {".gb", sid_GB}, {".yz", sid_GB}, {".tr", sid_GB}, {".gba", sid_GBA},
	{".yzw", sid_GBA}, {".trq", sid_GBA}, {".str_dr", sid_STRDR}, {".xyz_dz", sid_STRDR}, {".rgb_db", sid_STRDR},
	{".stq_dq", sid_STQDQ}, {".xyw_dw", sid_STQDQ}, {".rga_da", sid_STQDQ},
	{"_x8", sid_X8}, 
	{"_d8", sid_D8},
	{"phase", sid_PHASE},

	// PS_1_1 symbols
	{"r0", sid_1R0}, {"r1", sid_1R1},
	{"t0", sid_1T0}, {"t1", sid_1T1}, {"t2", sid_1T2}, {"t3", sid_1T3},
	{"tex", sid_TEX}, {"texcoord", sid_TEXCOORD}, {"texdp3", sid_TEXDP3}, {"texdp3tex", sid_TEXDP3TEX}, {"texm3x2pad", sid_TEXM3X2PAD},
	{"texm3x2tex", sid_TEXM3X2TEX}, {"texm3x3pad", sid_TEXM3X3PAD}, {"texm3x3tex", sid_TEXM3X3TEX},
	{"texm3x3spec", sid_TEXM3X3SPEC}, {"texm3x3vspec", sid_TEXM3X3VSPEC},

	// PS_1_2 symbols
	{"texreg2rgb", sid_TEXREG2RGB}, {"texreg2ar", sid_TEXREG2AR}, {"texreg2gb", sid_TEXREG2GB},

};


// macro token expansion for ps_1_2 instruction: texreg2ar
PS_1_4::TokenInst PS_1_4::texreg2ar[] = {
	// mov r(x).r, r(y).a
	{sid_MOV}, {sid_R1}, {sid_R,}, {sid_COMMA}, {sid_R0}, {sid_A},
	// mov r(x).g, r(y).r
	{sid_MOV}, {sid_R1}, {sid_G}, {sid_COMMA}, {sid_R0}, {sid_R},
	// texld r(x), r(x)
	{sid_TEXLD}, {sid_R1}, {sid_COMMA}, {sid_R1},
};

PS_1_4::RegModOffset PS_1_4::texreg2xx_RegMods[] = {
	{1, R_BASE, 0},
	{7, R_BASE, 0},
	{13, R_BASE, 0},
	{15, R_BASE, 0},
	{4, R_BASE, 1},
	{10, R_BASE, 1},

};

PS_1_4::MacroRegModify PS_1_4::texreg2ar_MacroMods = {
	texreg2ar, ARRAYSIZE(texreg2ar),
	texreg2xx_RegMods, ARRAYSIZE(texreg2xx_RegMods)
};

// macro token expansion for ps_1_2 instruction: texreg2gb
PS_1_4::TokenInst PS_1_4::texreg2gb[] = {
	// mov r(x).r, r(y).g
	{sid_MOV}, {sid_R1}, {sid_R}, {sid_COMMA}, {sid_R0}, {sid_G},
	// mov r(x).g, r(y).b
	{sid_MOV}, {sid_R1}, {sid_G}, {sid_COMMA}, {sid_R0}, {sid_B},
	// texld r(x), r(x)
	{sid_TEXLD}, {sid_R1}, {sid_COMMA}, {sid_R1},
};

PS_1_4::MacroRegModify PS_1_4::texreg2gb_MacroMods = {
	texreg2gb, ARRAYSIZE(texreg2gb),
	texreg2xx_RegMods, ARRAYSIZE(texreg2xx_RegMods)
};


// macro token expansion for ps_1_1 instruction: texdp3
PS_1_4::TokenInst PS_1_4::texdp3[] = {
	// texcoord t(x)
	{sid_TEXCOORD}, {sid_1T1},
	// dp3 r(x), r(x), r(y)
	{sid_DP3}, {sid_R1}, {sid_COMMA}, {sid_R1}, {sid_COMMA}, {sid_R0},
};

PS_1_4::RegModOffset PS_1_4::texdp3_RegMods[] = {
	{1, T_BASE, 0},
	{3, R_BASE, 0},
	{5, R_BASE, 0},
	{7, R_BASE, 1},

};

PS_1_4::MacroRegModify PS_1_4::texdp3_MacroMods = {
	texdp3, ARRAYSIZE(texdp3),
	texdp3_RegMods, ARRAYSIZE(texdp3_RegMods)
};

// macro token expansion for ps_1_1 instruction: texdp3tex
PS_1_4::TokenInst PS_1_4::texdp3tex[] = {
	// texcoord t(x)
	{sid_TEXCOORD}, {sid_1T1},
	// dp3 r1, r(x), r(y)
	{sid_DP3}, {sid_R1}, {sid_COMMA}, {sid_R1}, {sid_COMMA}, {sid_R0},
	// texld r(x), r(x)
	{sid_TEXLD}, {sid_R1}, {sid_COMMA}, {sid_R1},
};

PS_1_4::RegModOffset PS_1_4::texdp3tex_RegMods[] = {
	{1, T_BASE, 0},
	{3, R_BASE, 0},
	{5, R_BASE, 0},
	{7, R_BASE, 1},
	{9, R_BASE, 1},
	{11, R_BASE, 1},

};

PS_1_4::MacroRegModify PS_1_4::texdp3tex_MacroMods = {
	texdp3tex, ARRAYSIZE(texdp3tex),
	texdp3tex_RegMods, ARRAYSIZE(texdp3tex_RegMods)
};

// macro token expansion for ps_1_1 instruction: texm3x2pad
PS_1_4::TokenInst PS_1_4::texm3x2pad[] = {
	// texcoord t(x)
	{sid_TEXCOORD}, {sid_1T0},
	// dp3 r4.r, r(x), r(y)
	{sid_DP3}, {sid_R4}, {sid_R}, {sid_COMMA}, {sid_R1}, {sid_COMMA}, {sid_R0},

};

PS_1_4::RegModOffset PS_1_4::texm3xxpad_RegMods[] = {
	{1, T_BASE, 0},
	{6, R_BASE, 0},
	{8, R_BASE, 1},
};

PS_1_4::MacroRegModify PS_1_4::texm3x2pad_MacroMods = {
	texm3x2pad, ARRAYSIZE(texm3x2pad),
	texm3xxpad_RegMods, ARRAYSIZE(texm3xxpad_RegMods)
};


// macro token expansion for ps_1_1 instruction: texm3x2tex
PS_1_4::TokenInst PS_1_4::texm3x2tex[] = {
	// texcoord t(x)
	{sid_TEXCOORD}, {sid_1T1},
	// dp3 r4.g, r(x), r(y)
	{sid_DP3}, {sid_R4}, {sid_G}, {sid_COMMA}, {sid_R1}, {sid_COMMA}, {sid_R0},
	// texld r(x), r4
	{sid_TEXLD}, {sid_R1}, {sid_COMMA}, {sid_R4}

};

PS_1_4::RegModOffset PS_1_4::texm3xxtex_RegMods[] = {
	{1, T_BASE, 0},
	{6, R_BASE, 0},
	{8, R_BASE, 1},
	{10, R_BASE, 0}
};

PS_1_4::MacroRegModify PS_1_4::texm3x2tex_MacroMods = {
	texm3x2tex, ARRAYSIZE(texm3x2tex),
	texm3xxtex_RegMods, ARRAYSIZE(texm3xxtex_RegMods)
};

// macro token expansion for ps_1_1 instruction: texm3x3pad
PS_1_4::TokenInst PS_1_4::texm3x3pad[] = {
	// texcoord t(x)
	{sid_TEXCOORD}, {sid_1T1},
	// dp3 r4.r, r(x), r(y)
	{sid_DP3}, {sid_R4}, {sid_R}, {sid_COMMA}, {sid_R1}, {sid_COMMA}, {sid_R0},
	// texld r1, r4
	{sid_TEXLD}, {sid_R1}, {sid_COMMA}, {sid_R4}
};


PS_1_4::MacroRegModify PS_1_4::texm3x3pad_MacroMods = {
	texm3x3pad, ARRAYSIZE(texm3x3pad),
	texm3xxpad_RegMods, ARRAYSIZE(texm3xxpad_RegMods)
};

// macro token expansion for ps_1_1 instruction: texm3x3tex
PS_1_4::TokenInst PS_1_4::texm3x3tex[] = {
	// texcoord t(x)
	{sid_TEXCOORD}, {sid_1T0},
	// dp3 r4.b, r(x), r(y)
	{sid_DP3}, {sid_R4}, {sid_B}, {sid_COMMA}, {sid_R1}, {sid_COMMA}, {sid_R0},

};


PS_1_4::MacroRegModify PS_1_4::texm3x3tex_MacroMods = {
	texm3x3tex, ARRAYSIZE(texm3x3tex),
	texm3xxtex_RegMods, ARRAYSIZE(texm3xxtex_RegMods)
};

// macro token expansion for ps_1_1 instruction: texm3x3spec
PS_1_4::TokenInst PS_1_4::texm3x3spec[] = {
	// texcoord t(x)
	{sid_TEXCOORD}, {sid_1T3},
	// dp3 r4.b, r3, r(x)
	{sid_DP3}, {sid_R4}, {sid_B}, {sid_COMMA}, {sid_R3}, {sid_COMMA}, {sid_R0},
	// dp3_x2 r3, r4, c(x)
	{sid_DP3}, {sid_X2}, {sid_R3}, {sid_COMMA}, {sid_R4}, {sid_COMMA}, {sid_C0},
	// mul r3, r3, c(x)
	{sid_MUL}, {sid_R3}, {sid_COMMA}, {sid_R3}, {sid_COMMA}, {sid_C0},
	// dp3 r2, r4, r4
	{sid_DP3}, {sid_R2}, {sid_COMMA}, {sid_R4}, {sid_COMMA}, {sid_R4},
	// mad r4.rgb, 1-c(x), r2, r3
	{sid_MAD}, {sid_R4}, {sid_RGB}, {sid_COMMA}, {sid_INVERT}, {sid_C0}, {sid_COMMA}, {sid_R2}, {sid_COMMA}, {sid_R3},
	// + mov r4.a, r2.r
	{sid_MOV}, {sid_R4}, {sid_A}, {sid_COMMA}, {sid_R2}, {sid_R},
	// texld r3, r4.xyz_dz
	{sid_TEXLD}, {sid_R3}, {sid_COMMA}, {sid_R4}, {sid_STRDR}

};

PS_1_4::RegModOffset PS_1_4::texm3x3spec_RegMods[] = {
	{8, R_BASE, 1},
	{15, R_BASE, 2},
	{21, C_BASE, 2},
	{33, C_BASE, 2},

};

PS_1_4::MacroRegModify PS_1_4::texm3x3spec_MacroMods = {
	texm3x3spec, ARRAYSIZE(texm3x3spec),
	texm3x3spec_RegMods, ARRAYSIZE(texm3x3spec_RegMods)
};


/* ********************* END OF CLASS STATIC DATA ********************************* */

PS_1_4::PS_1_4()
{
	// allocate enough room for a large pixel shader
	mPhase1TEX_mi.reserve(50);
	mPhase2TEX_mi.reserve(30);
	mPhase1ALU_mi.reserve(100);
	mPhase2ALU_mi.reserve(100);


	mASMSymbolTextLib = PS_1_4_ASMSymbolTextLib;
	mASMSymbolTextLibCnt = ARRAYSIZE(PS_1_4_ASMSymbolTextLib);
	mASMSymbolTypeLib = PS_1_4_ASMSymbolTypeLib;
	mASMSymbolTypeLibCnt = ARRAYSIZE(PS_1_4_ASMSymbolTypeLib);
	// tell compiler what the symbol id is for a numeric value
	mValueID = sid_VALUE;
	// The type library must have text definitions initialized
	// before compiler is invoked

	mPhaseMarkerFound = false;
	InitTypeLibText();

}


bool PS_1_4::bindMachineInstInPassToFragmentShader(const MachineInstContainer & PassMachineInstructions)
{
  int instIDX = 0;
  int instCount = PassMachineInstructions.size();
  bool error = false;

  while ((instIDX < instCount) && !error) {
    switch(PassMachineInstructions[instIDX]) {
      case mi_COLOROP1:
        if((instIDX+7) < instCount)
          glColorFragmentOp1ATI_ptr(PassMachineInstructions[instIDX+1], // op
            PassMachineInstructions[instIDX+2], // dst
            PassMachineInstructions[instIDX+3], // dstMask
            PassMachineInstructions[instIDX+4], // dstMod
            PassMachineInstructions[instIDX+5],   // arg1
            PassMachineInstructions[instIDX+6],   // arg1Rep
            PassMachineInstructions[instIDX+7]);  // arg1Mod
        instIDX += 8;
        break;

      case mi_COLOROP2:
        if((instIDX+10) < instCount)
          glColorFragmentOp2ATI_ptr(PassMachineInstructions[instIDX+1], // op
            PassMachineInstructions[instIDX+2], // dst
            PassMachineInstructions[instIDX+3], // dstMask
            PassMachineInstructions[instIDX+4], // dstMod
            PassMachineInstructions[instIDX+5],   // arg1
            PassMachineInstructions[instIDX+6],   // arg1Rep
            PassMachineInstructions[instIDX+7],  // arg1Mod
            PassMachineInstructions[instIDX+8],   // arg2
            PassMachineInstructions[instIDX+9],   // arg2Rep
            PassMachineInstructions[instIDX+10]);  // arg2Mod
        instIDX += 11;
        break;

      case mi_COLOROP3:
        if((instIDX+13) < instCount)
          glColorFragmentOp3ATI_ptr(PassMachineInstructions[instIDX+1], // op
            PassMachineInstructions[instIDX+2], // dst
            PassMachineInstructions[instIDX+3], // dstMask
            PassMachineInstructions[instIDX+4], // dstMod
            PassMachineInstructions[instIDX+5],   // arg1
            PassMachineInstructions[instIDX+6],   // arg1Rep
            PassMachineInstructions[instIDX+7],  // arg1Mod
            PassMachineInstructions[instIDX+8],   // arg2
            PassMachineInstructions[instIDX+9],   // arg2Rep
            PassMachineInstructions[instIDX+10],  // arg2Mod
            PassMachineInstructions[instIDX+11],   // arg2
            PassMachineInstructions[instIDX+12],   // arg2Rep
            PassMachineInstructions[instIDX+13]);  // arg2Mod
        instIDX += 14;
        break;

      case mi_ALPHAOP1:
        if((instIDX+6) < instCount)
          glAlphaFragmentOp1ATI_ptr(PassMachineInstructions[instIDX+1], // op
            PassMachineInstructions[instIDX+2],   // dst
            PassMachineInstructions[instIDX+3],   // dstMod
            PassMachineInstructions[instIDX+4],   // arg1
            PassMachineInstructions[instIDX+5],   // arg1Rep
            PassMachineInstructions[instIDX+6]);  // arg1Mod
        instIDX += 7;
        break;

      case mi_ALPHAOP2:
        if((instIDX+9) < instCount)
          glAlphaFragmentOp2ATI_ptr(PassMachineInstructions[instIDX+1], // op
            PassMachineInstructions[instIDX+2],   // dst
            PassMachineInstructions[instIDX+3],   // dstMod
            PassMachineInstructions[instIDX+4],   // arg1
            PassMachineInstructions[instIDX+5],   // arg1Rep
            PassMachineInstructions[instIDX+6],   // arg1Mod
            PassMachineInstructions[instIDX+7],   // arg2
            PassMachineInstructions[instIDX+8],   // arg2Rep
            PassMachineInstructions[instIDX+9]);  // arg2Mod
        instIDX += 10;
        break;

      case mi_ALPHAOP3:
        if((instIDX+12) < instCount)
          glAlphaFragmentOp3ATI_ptr(PassMachineInstructions[instIDX+1], // op
            PassMachineInstructions[instIDX+2],   // dst
            PassMachineInstructions[instIDX+3],   // dstMod
            PassMachineInstructions[instIDX+4],   // arg1
            PassMachineInstructions[instIDX+5],   // arg1Rep
            PassMachineInstructions[instIDX+6],   // arg1Mod
            PassMachineInstructions[instIDX+7],   // arg2
            PassMachineInstructions[instIDX+8],   // arg2Rep
            PassMachineInstructions[instIDX+9],   // arg2Mod
            PassMachineInstructions[instIDX+10],  // arg2
            PassMachineInstructions[instIDX+11],  // arg2Rep
            PassMachineInstructions[instIDX+12]); // arg2Mod
        instIDX += 13;
        break;

      case mi_SETCONSTANTS:
        if((instIDX+2) < instCount)
          glSetFragmentShaderConstantATI_ptr(PassMachineInstructions[instIDX+1], // dst
            &mConstants[PassMachineInstructions[instIDX+2]]);
        instIDX += 3;
        break;

      case mi_PASSTEXCOORD:
        if((instIDX+3) < instCount)
          glPassTexCoordATI_ptr(PassMachineInstructions[instIDX+1], // dst
            PassMachineInstructions[instIDX+2], // coord
            PassMachineInstructions[instIDX+3]); // swizzle
        instIDX += 4;
        break;

      case mi_SAMPLEMAP:
        if((instIDX+3) < instCount)
          glSampleMapATI_ptr(PassMachineInstructions[instIDX+1], // dst
            PassMachineInstructions[instIDX+2], // interp
            PassMachineInstructions[instIDX+3]); // swizzle
        instIDX += 4;
        break;

	  default:
		instIDX = instCount;
		// should generate an error since an unknown instruction was found
		// instead for now the bind process is terminated and the fragment program may still function
		// but its output may not be what was programmed

    } // end of switch

    error = (glGetError() != GL_NO_ERROR);
  }// end of while

  return !error;

}


uint PS_1_4::getMachineInst(uint Idx)
{
	if (Idx < mPhase1TEX_mi.size()) {
		return mPhase1TEX_mi[Idx];
	}
	else {
		Idx -= mPhase1TEX_mi.size();
		if (Idx < mPhase1ALU_mi.size()) {
			return mPhase1ALU_mi[Idx];
		}
		else {
			Idx -= mPhase1ALU_mi.size();
			if (Idx < mPhase2TEX_mi.size()) {
			//MessageBox(NULL, "ptPHASE2TEX instruction index", "ATI fs compiler", MB_OK);
				return mPhase2TEX_mi[Idx];
			}
			else {
				Idx -= mPhase2TEX_mi.size();
				if (Idx < mPhase2ALU_mi.size()) {
					return mPhase2ALU_mi[Idx];
				}

			}

		}

	}

	return 0;

}


void PS_1_4::addMachineInst(PhaseType phase, uint inst)
{
	switch(phase) {

		case ptPHASE1TEX:
			mPhase1TEX_mi.push_back(inst);
			break;

		case ptPHASE1ALU:
			mPhase1ALU_mi.push_back(inst);
			break;

		case ptPHASE2TEX:
			mPhase2TEX_mi.push_back(inst);
			//MessageBox(NULL, "ptPHASE2TEX instruction", "ATI fs compiler", MB_OK);

			break;

		case ptPHASE2ALU:
			mPhase2ALU_mi.push_back(inst);
			break;


	} // end switch(phase)

}

uint PS_1_4::getMachineInstCount()
{

	return (mPhase1TEX_mi.size() + mPhase1ALU_mi.size() + mPhase2TEX_mi.size() + mPhase2ALU_mi.size());
}


bool PS_1_4::bindAllMachineInstToFragmentShader()
{
	bool passed;

	// there are 4 machine instruction ques to pass to the ATI fragment shader
	passed = bindMachineInstInPassToFragmentShader(mPhase1TEX_mi);
	passed &= bindMachineInstInPassToFragmentShader(mPhase1ALU_mi);
	passed &= bindMachineInstInPassToFragmentShader(mPhase2TEX_mi);
	passed &= bindMachineInstInPassToFragmentShader(mPhase2ALU_mi);
	return passed;

}


bool PS_1_4::expandMacro(MacroRegModify & MacroMod)
{

	RegModOffset * regmod;

	// set source and destination registers in macro expansion
	for (uint i = 0; i < MacroMod.RegModSize; i++) {
		regmod = &MacroMod.RegMods[i];
		MacroMod.Macro[regmod->MacroOffset].mID = regmod->RegisterBase + mOpParrams[regmod->OpParramsIndex].Arg;

	}

	// turn macro support on so that ps.1.4 ALU instructions get put in phase 1 alu instruction sequence container
	mMacroOn = true;
	// pass macro tokens on to be turned into machine instructions
	// expand macro to ps.1.4 by doing recursive call to doPass2
	bool passed = Pass2scan(MacroMod.Macro, MacroMod.MacroSize);
	mMacroOn = false;

	return passed;
}


bool PS_1_4::BuildMachineInst()
{
	bool passed = false;

	// check the states to see if a machine instruction can be assembled
	if(mOpInst != sid_INVALID) {

		// only build the machine instruction if all arguments have been set up

		if(mArgCnt == (mNumArgs - 1)) {
			bool doalpha = false;
			PhaseType instruction_phase;
			MachineInstID alphaoptype;
			GLuint glopinst;
			int srccnt = mNumArgs - 2;
			passed = true; // assume everything will go okay untill proven otherwise

			// start with machine NOP instuction
			// this is used after the switch to see if an instruction was set up
			// determine which MachineInstID is required based on the op instruction
			mOpType = mi_NOP;

			switch(mOpInst) {
				case sid_ADD:
				case sid_SUB:
				case sid_MUL:
				case sid_MAD:
				case sid_LRP:
				case sid_MOV:
				case sid_CMP:
				case sid_CND:
				case sid_DP2ADD:
				case sid_DP3:
				case sid_DP4:
					glopinst = mASMSymbolTypeLib[mOpInst].mPass2Data;
					mOpType = (MachineInstID)(mi_COLOROP1 + srccnt);

					// if context is ps.1.x and Macro not on or a phase marker was found then put all ALU ops in phase 2 ALU container
					if (((mActiveContexts & ckp_PS_1_1) && !mMacroOn) || mPhaseMarkerFound) instruction_phase = ptPHASE2ALU;
					else instruction_phase = ptPHASE1ALU;
					// check for alpha op in destination register which is OpParrams[0]
					// if no Mask for destination then make it .rgba
					if(mOpParrams[0].MaskRep == 0) mOpParrams[0].MaskRep =
					GL_RED_BIT_ATI | GL_GREEN_BIT_ATI | GL_BLUE_BIT_ATI | ALPHA_BIT;
					if (mOpParrams[0].MaskRep & ALPHA_BIT) {
						doalpha = true;
						mOpParrams[0].MaskRep -= ALPHA_BIT;
						if(mOpParrams[0].MaskRep == 0) mOpType = mi_NOP; // only do alpha op
						alphaoptype = (MachineInstID)(mi_ALPHAOP1 + srccnt);
					}
					passed = checkSourceParramsReplicate();
					break;

				case sid_TEXCRD:
					mOpType = mi_PASSTEXCOORD;
					if (mPhaseMarkerFound) instruction_phase = ptPHASE2TEX;
					else instruction_phase = ptPHASE1TEX;
					break;

				case sid_TEXLD:
					mOpType = mi_SAMPLEMAP;
					if (mPhaseMarkerFound) instruction_phase = ptPHASE2TEX;
					else instruction_phase = ptPHASE1TEX;
					break;

				case sid_TEX: // PS_1_1 emulation
					mOpType = mi_TEX;
					instruction_phase = ptPHASE1TEX;
					break;

				case sid_TEXCOORD: // PS_1_1 emulation
					mOpType = mi_TEXCOORD;
					instruction_phase = ptPHASE1TEX;
					break;

				case sid_TEXREG2AR:
					expandMacro(texreg2ar_MacroMods);
					break;

				case sid_TEXREG2GB:
					expandMacro(texreg2gb_MacroMods);
					break;

				case sid_TEXDP3:
					expandMacro(texdp3_MacroMods);
					break;

				case sid_TEXDP3TEX:
					expandMacro(texdp3tex_MacroMods);
					break;

				case sid_TEXM3X2PAD:
					expandMacro(texm3x2pad_MacroMods);
					break;

				case sid_TEXM3X2TEX:
					expandMacro(texm3x2tex_MacroMods);
					break;

				case sid_TEXM3X3PAD:
					// only 2 texm3x3pad instructions allowed
					// use count to modify macro to select which mask to use
					if(mTexm3x3padCount<2) {
						texm3x3pad[4].mID = sid_R + mTexm3x3padCount;
						mTexm3x3padCount++;
						expandMacro(texm3x3pad_MacroMods);

					}
					else passed = false;

					break;

				case sid_TEXM3X3TEX:
					expandMacro(texm3x3tex_MacroMods);
					break;

				case sid_DEF:
					mOpType = mi_SETCONSTANTS;
					instruction_phase = ptPHASE1TEX;
					break;

				case sid_PHASE: // PS_1_4 only
					mPhaseMarkerFound = true;
					break;

			} // end of switch
			// push instructions onto MachineInstructions container
			// assume that an instruction will be built
			if (mOpType != mi_NOP) {

				// a machine instruction will be built
				// this is currently the last one being built so keep track of it
				if (instruction_phase == ptPHASE2ALU) { 
					mSecondLastInstructionPos = mLastInstructionPos;
					mLastInstructionPos = mPhase2ALU_mi.size();
				}

				switch (mOpType) {
					case mi_COLOROP1:
					case mi_COLOROP2:
					case mi_COLOROP3:
						{
							addMachineInst(instruction_phase, mOpType);
							addMachineInst(instruction_phase, glopinst);
							// send all parameters to machine inst container
							for(int i=0; i<=mArgCnt; i++) {
								addMachineInst(instruction_phase, mOpParrams[i].Arg);
								addMachineInst(instruction_phase, mOpParrams[i].MaskRep);
								addMachineInst(instruction_phase, mOpParrams[i].Mod);
							}
						}
						break;

					case mi_SETCONSTANTS:
						addMachineInst(instruction_phase, mOpType);
						addMachineInst(instruction_phase, mOpParrams[0].Arg); // dst
						addMachineInst(instruction_phase, mConstantsPos); // index into constants array
						break;

					case mi_PASSTEXCOORD:
					case mi_SAMPLEMAP:
						// if source is a temp register than place instruction in phase 2 Texture ops
						if ((mOpParrams[1].Arg >= GL_REG_0_ATI) && (mOpParrams[1].Arg <= GL_REG_5_ATI)) {
							instruction_phase = ptPHASE2TEX;
						}
						addMachineInst(instruction_phase, mOpType);
						addMachineInst(instruction_phase, mOpParrams[0].Arg); // dst
						addMachineInst(instruction_phase, mOpParrams[1].Arg); // coord
						addMachineInst(instruction_phase, mOpParrams[1].MaskRep + GL_SWIZZLE_STR_ATI); // swizzle
						break;

					case mi_TEX: // PS_1_1 emulation - turn CISC into RISC - phase 1
						addMachineInst(instruction_phase, mi_SAMPLEMAP);
						addMachineInst(instruction_phase, mOpParrams[0].Arg); // dst
						// tex tx becomes texld rx, tx with x: 0 - 3
						addMachineInst(instruction_phase, mOpParrams[0].Arg - GL_REG_0_ATI + GL_TEXTURE0_ARB); // interp
						// default to str which fills rgb of destination register
						addMachineInst(instruction_phase, GL_SWIZZLE_STR_ATI); // swizzle
						break;

					case mi_TEXCOORD: // PS_1_1 emulation - turn CISC into RISC - phase 1
						addMachineInst(instruction_phase, mi_PASSTEXCOORD);
						addMachineInst(instruction_phase, mOpParrams[0].Arg); // dst
						// texcoord tx becomes texcrd rx, tx with x: 0 - 3
						addMachineInst(instruction_phase, mOpParrams[0].Arg - GL_REG_0_ATI + GL_TEXTURE0_ARB); // interp
						// default to str which fills rgb of destination register
						addMachineInst(instruction_phase, GL_SWIZZLE_STR_ATI); // swizzle
						break;

			


				} // end of switch (mOpType)
			} // end of if (mOpType != mi_NOP)

			if(doalpha) { // process alpha channel
				//
				// a scaler machine instruction will be built
				// this is currently the last one being built so keep track of it
				if (instruction_phase == ptPHASE2ALU) { 
					mSecondLastInstructionPos = mLastInstructionPos;
					mLastInstructionPos = mPhase2ALU_mi.size();
				}

				addMachineInst(instruction_phase, alphaoptype);
				addMachineInst(instruction_phase, glopinst);
				// put all parameters in instruction que
				for(int i=0; i<=mArgCnt; i++) {
					addMachineInst(instruction_phase, mOpParrams[i].Arg);
					// destination parameter has no mask since it is the alpha channel
					// don't push mask for parrameter 0 (dst)
					if(i>0) addMachineInst(instruction_phase, mOpParrams[i].MaskRep);
					addMachineInst(instruction_phase, mOpParrams[i].Mod);
				}
			}
			// instruction passed on to machine instruction so clear the pipe
			clearMachineInstState();

		}// end if (NumArgs == ArgCnt)

	}// end if (OpInst != sid_INVALID_)

	return passed;
}

void PS_1_4::optimize()
{
	// perform some optimizations on ps.1.1 machine instructions
	if (mActiveContexts & ckp_PS_1_1) {
		// need to check last few instructions to make sure r0 is set
		// ps.1.1 emulation uses r4 for r0 so last couple of instructions will probably require
		// changine destination register back to r0
		if (mLastInstructionPos < mPhase2ALU_mi.size()) {
			// first argument at mLastInstructionPos + 2 is destination register for all ps.1.1 ALU instructions
			mPhase2ALU_mi[mLastInstructionPos + 2] = GL_REG_0_ATI; 
			// if was an alpha op only then modify second last instruction destination register
			if ((mPhase2ALU_mi[mLastInstructionPos] == mi_ALPHAOP1) ||
				(mPhase2ALU_mi[mLastInstructionPos] == mi_ALPHAOP2) ||
				(mPhase2ALU_mi[mLastInstructionPos] == mi_ALPHAOP3)
				
				) {

				mPhase2ALU_mi[mSecondLastInstructionPos + 2] = GL_REG_0_ATI; 
			}

		}// end if (mLastInstructionPos < mMachineInstructions.size())

	}// end if (mActiveContexts & ckp_PS_1_1)

}

bool PS_1_4::checkSourceParramsReplicate()
{
  // make sure that all source params have proper replicators ie: .r .g .b .a or equivalent
  bool passed = true;
  for(int i=1; i<=mArgCnt; i++) {
    switch(mOpParrams[i].MaskRep) {
      case GL_RED_BIT_ATI:
        mOpParrams[i].MaskRep = GL_RED;
        break;
      case GL_GREEN_BIT_ATI:
        mOpParrams[i].MaskRep = GL_GREEN;
        break;
      case GL_BLUE_BIT_ATI:
        mOpParrams[i].MaskRep = GL_BLUE;
        break;
      case ALPHA_BIT:
        mOpParrams[i].MaskRep = GL_ALPHA;
        break;

      default:
        passed = false;
    }// end of switch

  } // end of for

  return passed;

}


void PS_1_4::clearMachineInstState()
{
  // set current Machine Instruction State to baseline
  mOpType = mi_NOP;
  mOpInst = sid_INVALID;
  mNumArgs = 0;
  mArgCnt = 0;
  for(int i=0; i<MAXOPPARRAMS; i++) {
    mOpParrams[i].Arg = GL_NONE;
    mOpParrams[i].Filled = false;
    mOpParrams[i].MaskRep = GL_NONE;
    mOpParrams[i].Mod = GL_NONE;
  }

}


void PS_1_4::clearAllMachineInst()
{

	mPhase1TEX_mi.clear();
	mPhase1ALU_mi.clear();
	mPhase2TEX_mi.clear();
	mPhase2ALU_mi.clear();


}

bool PS_1_4::doPass2()
{
	clearAllMachineInst();
	mConstantsPos = -4;
	// keep track of the last instruction built
	// this info is used at the end of pass 2 to optimize the machine code
	mLastInstructionPos = 0;
	mSecondLastInstructionPos = 0;
	mMacroOn = false;  // macro's off at the beginning
	mTexm3x3padCount = 0;
	// if pass 2 was successful, optimize the machine instructions
	bool passed = Pass2scan(&mTokenInstructions[0], mTokenInstructions.size());
	if (passed) optimize();  

	return passed;

}


bool PS_1_4::Pass2scan(TokenInst * Tokens, uint size)
{
	//if(Tokens == texreg2ar)  MessageBox(NULL, "starting texreg2ar", "ATI fs compiler", MB_OK);

	// execute TokenInstructions to build MachineInstructions
	bool passed = true;
	//GLuint insttype;
	ASMSymbolDef* cursymboldef;

	clearMachineInstState();


	// iterate through all the tokens and build machine instruction
	// for each machine instruction need: optype, opinst, and up to 5 parameters
	for(uint i = 0; i < size; i++) {
		// lookup instruction type in library

		cursymboldef = &mASMSymbolTypeLib[Tokens[i].mID];
		mCurrentLine = Tokens[i].mLine;
		mCharPos = Tokens[i].mPos;

		switch(cursymboldef->mInstType) {
			case id_REGISTER:
			case id_TEXREGISTER:
			case id_CONSTANT:
				// registars can be used for read and write so they can be used for dst and arg
				passed = setOpParram(cursymboldef);
				break;

			case id_COLOR:
			case id_TEXTURE:
				if(mArgCnt>0) passed = setOpParram(cursymboldef);
				else passed = false;
				break;

			case id_OPINST:
				// if the last instruction has not been passed on then do it now
				// make sure the pipe is clear for a new instruction
				BuildMachineInst();
				if(mOpInst == sid_INVALID) {
					// set op instruction, num of parameters required
					mOpInst = cursymboldef->mID;
					mNumArgs = cursymboldef->mParrameters;
				}
				else passed = false;
				break;

			case id_MASK:
				// could be a dst mask or a arg replicator
				// if dst mask and alpha included then make up a alpha instruction: maybe best to wait until instruction args completed
				mOpParrams[mArgCnt].MaskRep = cursymboldef->mPass2Data;
				break;

			case id_TEXSWIZZLE:
				// to be completed
				// only applicable to ps.1.4 texld and sampltex instructions
				if (mArgCnt > 0) mOpParrams[mArgCnt].Mod = cursymboldef->mPass2Data;
				else passed = false;
				break;

			case id_DSTMOD:
				if((mArgCnt == 0) || (cursymboldef->mID == sid_X2)) mOpParrams[mArgCnt].Mod |= cursymboldef->mPass2Data;
				else passed = false;
				break;

			case id_ARGMOD:
				// can only be applied to ArgCnt>0 : arguments other than destination
				//if(ArgCnt>0)
				mOpParrams[mArgCnt].Mod |= cursymboldef->mPass2Data;
				//else passed = false;
				break;

			case id_NUMVAL:
				if(mArgCnt>0) {
				passed = setOpParram(cursymboldef);
				// keep track of how many values are used
				// update Constants array position
				mConstantsPos++;
				}
				else passed = false;
				break;

			case id_SEPERATOR:
				if(cursymboldef->mID == sid_COMMA) mArgCnt++;
				break;
		} // end of switch

		if(!passed) break;
	}// end of for: i<TokenInstCnt

	// check to see if there is still an instruction left in the pipe
	if(passed) {
		BuildMachineInst();
		// if there are no more instructions in the pipe than OpInst should be invalid
		if(mOpInst != sid_INVALID) passed = false;
	}


	return passed;
}



bool PS_1_4::setOpParram(ASMSymbolDef* symboldef)
{
  bool success = true;
  if(mArgCnt<MAXOPPARRAMS) {
    if(mOpParrams[mArgCnt].Filled) mArgCnt++;
  }
  if (mArgCnt<MAXOPPARRAMS) {
    mOpParrams[mArgCnt].Filled = true;
    mOpParrams[mArgCnt].Arg = symboldef->mPass2Data;
  }
  else success = false;

  return success;
}




// *********************************************************************************
//  this is where the tests are carried out to make sure the PS_1_4 compiler works
#ifdef _DEBUG
// check the functionality of functions in PS_1_4: each test will print to the output file PASSED of FAILED
void PS_1_4::test()
{
  

  struct test1result{
    char character;
    int line;
  };

  struct testfloatresult{
    char *teststr;
    float fvalue;
    int charsize;
  };

  char TestStr1[] = "   \n\r  //c  \n\r// test\n\r  \t  c   - \n\r ,  e";
  test1result test1results[] = {
    {'c', 4},
    {'-', 4},
    {',', 5},
    {'e', 5}
  };

  testfloatresult testfloatresults[] = {
    {"1 test", 1.0f, 1},
    {"2.3f test", 2.3f, 3},
    {"-0.5 test", -0.5f, 4},
    {" 23.6 test", 23.6f, 5},
    {"  -0.021 test", -0.021f, 8},
    {"12 test", 12.0f, 2},
    {"3test", 3.0f, 1}
  };


  char TestStr2[] = "ps.1.4\nmov r0.xzw, c1, -r1.x_bias \nmul r3, r2, c3";
  SymbolID test2result[] = {sid_PS_1_4, sid_MOV, sid_R0, sid_RBA, sid_COMMA, sid_C1, sid_COMMA,
                            sid_NEGATE, sid_R1, sid_R, sid_BIAS, sid_MUL, sid_R3, sid_COMMA,
                            sid_R2, sid_COMMA, sid_C3};
  SymbolID test3result[] = { sid_MOV, sid_COMMA, sid_MUL, sid_ADD, sid_NEGATE, sid_T0
  };

  #define PART2INST 17
  char TestStr3[] = "mov r0,c1";
  char TestSymbols[] = "mov";
  char passed[] = "PASSED\n";
  char failed[] = "***** FAILED *****\n";

  int resultID = 0;

  // loop variable used in for loops
  int i;
  fp = fopen("ASMTests.txt", "wt");
  //MessageBox(NULL, "starting compiler tests", "ATI fs compiler", MB_OK);

// **************************************************************
  // first test: see if positionToNextSymbol can find a valid Symbol
  fprintf(fp, "Testing: positionToNextSymbol\n");

  mSource = TestStr1;
  mCharPos = 0;
  mCurrentLine = 1;
  mEndOfSource = strlen(mSource);
  while (positionToNextSymbol()) {
    fprintf(fp,"  character found: [%c]   Line:%d  : " , mSource[mCharPos], mCurrentLine);
    if( (mSource[mCharPos] == test1results[resultID].character) && (mCurrentLine==test1results[resultID].line)) fprintf(fp, passed);
    else fprintf(fp, failed);
    resultID++;
    mCharPos++;
  }
  fprintf(fp, "finished testing: positionToNextSymbol\n");
// **************************************************************
  // Second Test
  // did the type lib get initialized properly with a default name index
  fprintf(fp, "\nTesting: getTypeDefText\n");
  char* resultstr = getTypeDefText(sid_MOV);
  fprintf(fp, "  default name of mov is: [%s]: %s", resultstr, (strcmp("mov", resultstr)==0)?passed:failed);
  fprintf(fp, "finished testing: getTypeDefText\n");
// **************************************************************
// **************************************************************
  // fourth test - does isSymbol work correctly
  fprintf(fp, "\nTesting: isSymbol\n");
  mSource = TestStr3;
  mCharPos = 0;
  fprintf(fp, "  before: [%s]\n", mSource + mCharPos);
  fprintf(fp, "  symbol to find: [%s]\n", TestSymbols);
  if(isSymbol(TestSymbols, resultID)) {
    fprintf(fp, "  after: [%s] : %s", mSource + resultID + 1, (mSource[resultID + 1] == 'r')? passed:failed);
  }
  else fprintf(fp, failed);
  fprintf(fp,"  symbol size: %d\n", resultID);
  fprintf(fp, "finished testing: isSymbol\n");

// **************************************************************
  fprintf(fp, "\nTesting: isFloatValue\n");
  float fvalue = 0;
  int charsize = 0;
  char teststrfloat1[] = "1 test";
  mCharPos = 0;
  int testsize = ARRAYSIZE(testfloatresults);
  for(i=0; i<testsize; i++) {
    mSource = testfloatresults[i].teststr;
    fprintf(fp, "  test string [%s]\n", mSource);
    isFloatValue(fvalue, charsize);
    fprintf(fp, "   value is: %f should be %f: %s", fvalue, testfloatresults[i].fvalue, (fvalue == testfloatresults[i].fvalue)?passed:failed);
    fprintf(fp, "   char size is: %d should be %d: %s", charsize, testfloatresults[i].charsize, (charsize == testfloatresults[i].charsize)?passed:failed);
  }

  fprintf(fp, "finished testing: isFloatValue\n");

// **************************************************************
  // fifth test
  fprintf(fp, "\nTesting: checkTokenSemantics\n");
  int testarraysize = ARRAYSIZE(test2result) - 1;
  fprintf(fp, "  Semantic checks that should pass:\n");
  for(i=0;i<testarraysize; i++) {
    fprintf(fp, "  [%s] [%s] : %s", getTypeDefText(test2result[i]),
    getTypeDefText(test2result[i+1]), checkTokenSemantics(test2result[i],test2result[i+1])?passed:failed);
  }

  fprintf(fp, "\n  Semantic checks that should fail:\n");
  testarraysize = ARRAYSIZE(test3result) - 1;
  for(i=0;i<testarraysize; i++) {
    fprintf(fp, "  [%s] [%s] : %s", getTypeDefText(test3result[i]),
    getTypeDefText(test3result[i+1]), checkTokenSemantics(test3result[i],test3result[i+1])?passed:failed);
  }
  fprintf(fp, "finished testing: checkTokenSemantics\n");

// **************************************************************
  fprintf(fp, "\nTesting: Tokenize & context check\n");
  setActiveContexts(ckp_PS_BASE | ckp_PS_1_4);
  mSource = TestStr2;
  mCharPos = 0;
  fprintf(fp, "Part 1:\n");
  fprintf(fp, "  before: [%s]\n", mSource);
  resultID = Tokenize();
  fprintf(fp, "\n  after: [%s] : %s\n", mSource + mCharPos, (resultID == sid_PS_1_4)?passed:failed);

  mCharPos = 13;
  fprintf(fp,"\nPart 2: checking similar names\n");
  fprintf(fp, "  before: [%s]\n", mSource + mCharPos);
  resultID = Tokenize();
  if(resultID<sid_INVALID) fprintf(fp, "Symbol found: [%s]\n",getTypeDefText((SymbolID)resultID));
  if(resultID == sid_RBA) {
    fprintf(fp, "\n  after: [%s] : %s", mSource + mCharPos, passed);
  }
  else {
    fprintf(fp, failed);
  }

  fprintf(fp, "finished testing: Tokenize\n");


// **************************************************************
  // sixth test:
  fprintf(fp, "\nTesting: Compile\n");
  fprintf(fp, "Part 1 Compile: Check Pass 1 of \n\n%s\n\n", TestStr2);

  bool compiled = compile(TestStr2);
  fprintf(fp, "  Lines scaned: %d, instructions produced: %d out of %d: %s",
    mCurrentLine, mTokenInstructions.size(), PART2INST,
    (mTokenInstructions.size() == PART2INST)? passed : failed);
  //bool semcheck = checkTokenInstructionsForSemantics();
  //fprintf(fp, "  semantic check: %s",semcheck ? passed : failed);
  //if(!semcheck) {
  //  fprintf(fp, "error at: [%s]\n\n",&TestStr2[mCharPos]);
  //}
  for(i = 0; i<(int)mTokenInstructions.size(); i++) {
    fprintf(fp,"  [%s] : %s", getTypeDefText(mTokenInstructions[i].mID),
      (mTokenInstructions[i].mID == (uint)test2result[i])?passed:failed);
  }

  fprintf(fp, "finished testing: Compile\n");
// **************************************************************
  // test to see if PS_1_4 compile pass 2 generates the proper machine instructions
	char TestStr4[] = "ps.1.4\ndef c0,1.0,2.0,3.0,4.0\nmov_x8 r1,v0\nmov r0,r1";

	SymbolID test4result[] = {
		sid_PS_1_4, sid_DEF, sid_C0, sid_COMMA, sid_VALUE, sid_COMMA,
		sid_VALUE, sid_COMMA, sid_VALUE, sid_COMMA, sid_VALUE, sid_MOV, sid_X8, sid_R1, sid_COMMA,
		sid_V0, sid_MOV, sid_R0, sid_COMMA, sid_R1
	};

	GLuint test4MachinInstResults[] = {
		mi_SETCONSTANTS, GL_CON_0_ATI, 0, mi_COLOROP1, GL_MOV_ATI, GL_REG_1_ATI, RGB_BITS, GL_8X_BIT_ATI,
		GL_PRIMARY_COLOR_ARB, GL_NONE, GL_NONE,mi_ALPHAOP1, GL_MOV_ATI, GL_REG_1_ATI, GL_8X_BIT_ATI,
		GL_PRIMARY_COLOR_ARB, GL_NONE, GL_NONE, mi_COLOROP1, GL_MOV_ATI, GL_REG_0_ATI, RGB_BITS, GL_NONE,GL_REG_1_ATI, GL_NONE, GL_NONE,
		mi_ALPHAOP1, GL_MOV_ATI, GL_REG_0_ATI, GL_NONE,GL_REG_1_ATI, GL_NONE, GL_NONE,
	};


	testCompile("PS_1_4", TestStr4, test4result, ARRAYSIZE(test4result), test4MachinInstResults, ARRAYSIZE(test4MachinInstResults));

// **************************************************************
// test to see if a PS_1_1 can be compiled - pass 1 and pass 2 are checked

	char TestStr5[] = "ps.1.1\ndef c0,1.0,2.0,3.0,4.0\ntex t0\n// test\ntex t1\ndp3 t0.rgb, t0_bx2, t1_bx2\nmov r0,1 - t0";

	SymbolID test5result[] = {
		sid_PS_1_1, sid_DEF, sid_C0, sid_COMMA, sid_VALUE, sid_COMMA,
		sid_VALUE, sid_COMMA, sid_VALUE, sid_COMMA, sid_VALUE, sid_TEX, sid_1T0, sid_TEX, sid_1T1,
		sid_DP3, sid_1T0, sid_RGB, sid_COMMA, sid_1T0, sid_BX2, sid_COMMA, sid_1T1, sid_BX2,
		
		sid_MOV, sid_1R0, sid_COMMA, sid_INVERT, sid_1T0
	};

	GLuint test5MachinInstResults[] = {
		mi_SETCONSTANTS, GL_CON_0_ATI, 0, mi_SAMPLEMAP, GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI, mi_SAMPLEMAP, GL_REG_1_ATI, GL_TEXTURE1_ARB, GL_SWIZZLE_STR_ATI,
		mi_COLOROP2, GL_DOT3_ATI, GL_REG_0_ATI, RGB_BITS, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_2X_BIT_ATI | GL_BIAS_BIT_ATI,
		GL_REG_1_ATI, GL_NONE, GL_2X_BIT_ATI | GL_BIAS_BIT_ATI,
		mi_COLOROP1, GL_MOV_ATI, GL_REG_0_ATI, RGB_BITS, GL_NONE, GL_REG_0_ATI, GL_NONE, GL_COMP_BIT_ATI, mi_ALPHAOP1, GL_MOV_ATI, GL_REG_0_ATI,
		GL_NONE, GL_REG_0_ATI, GL_NONE, GL_COMP_BIT_ATI,
	};


	testCompile("PS_1_1", TestStr5, test5result, ARRAYSIZE(test5result), test5MachinInstResults, ARRAYSIZE(test5MachinInstResults));


// **************************************************************
// test to see if a PS_1_2 CISC instructions can be compiled - pass 1 and pass 2 are checked

	char TestStr6[] = "ps.1.2\ndef c0,1.0,2.0,3.0,4.0\ntex t0\n// test\ntexreg2ar t1, t0";

	SymbolID test6result[] = {
		sid_PS_1_2, sid_DEF, sid_C0, sid_COMMA, sid_VALUE, sid_COMMA,
		sid_VALUE, sid_COMMA, sid_VALUE, sid_COMMA, sid_VALUE,
		sid_TEX, sid_1T0,
		sid_TEXREG2AR, sid_1T1, sid_COMMA, sid_1T0
	};

	GLuint test6MachinInstResults[] = {
		// def c0
		mi_SETCONSTANTS, GL_CON_0_ATI, 0,
		// texld r0, t0.str
		mi_SAMPLEMAP, GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI,
		// mov r1.r, r0.a 
		mi_COLOROP1, GL_MOV_ATI, GL_REG_1_ATI, GL_RED_BIT_ATI, GL_NONE, GL_REG_0_ATI, GL_ALPHA, GL_NONE,
		// mov r1.g, r0.r
		mi_COLOROP1, GL_MOV_ATI, GL_REG_1_ATI, GL_GREEN_BIT_ATI, GL_NONE, GL_REG_0_ATI, GL_RED, GL_NONE,
		// texld r1, r1
		mi_SAMPLEMAP, GL_REG_1_ATI, GL_REG_1_ATI, GL_SWIZZLE_STR_ATI,
	};


	testCompile("PS_1_2 CISC instructions", TestStr6, test6result, ARRAYSIZE(test6result), test6MachinInstResults, ARRAYSIZE(test6MachinInstResults));


	fclose(fp);
	fp = NULL;

	//MessageBox(NULL, "finished PS compiler tests", "ATI fs compiler", MB_OK);

// **************************************************************
}

void PS_1_4::testCompile(char* testname, char* teststr, SymbolID* testresult, uint testresultsize, GLuint* MachinInstResults, uint MachinInstResultsSize)
{

	char passed[] = "PASSED\n";
	char failed[] = "***** FAILED ****\n";

	fprintf(fp, "\n*** TESTING: %s Compile: Check Pass 1 and 2\n", testname);
	fprintf(fp, "  source to compile:\n[%s]\n", teststr);
	bool compiled = compile(teststr);
	fprintf(fp, "  Pass 1 Lines scaned: %d, instructions produced: %d out of %d: %s",
		mCurrentLine, mTokenInstructions.size(), testresultsize,
		(mTokenInstructions.size() == (uint)testresultsize) ? passed : failed);

	fprintf(fp, "\n  Checking Pass 2:\n");

	fprintf(fp, "  Pass 2 Machine Instructions generated: %d out of %d: %s", getMachineInstCount(),
		MachinInstResultsSize, (getMachineInstCount() == MachinInstResultsSize) ? passed : failed);

	if(compiled) {
		fprintf(fp, "\n  Tokens:\n");
		for(uint i = 0; i<(testresultsize); i++) {
			fprintf(fp,"    [%s] : %s", getTypeDefText(mTokenInstructions[i].mID),
				(mTokenInstructions[i].mID == (uint)testresult[i]) ? passed : failed);
		}

		fprintf(fp, "\n  Machine Instructions:\n");

		for(i = 0; i<MachinInstResultsSize; i++) {
			fprintf(fp,"    instruction[%d] = 0x%x : 0x%x : %s", i, getMachineInst(i), MachinInstResults[i], (getMachineInst(i) == MachinInstResults[i]) ? passed : failed);
		}

		fprintf(fp, "\n  Constants:\n");
		for(i=0; i<4; i++) {
			fprintf(fp, "    Constants[%d] = %f : %s", i, mConstants[i], (mConstants[i] == (1.0f+i)) ? passed : failed);
		}


	}

	else {
		fprintf(fp, failed);
	}

	fprintf(fp, "\nfinished testing: %s Compile: Check Pass 2\n\n", testname);

}

void PS_1_4::testbinder()
{
  FILE* fp;
  char BindTestStr[] = "mov_x8 r0,v0";
  char passed[] = "PASSED\n";
  char failed[] = "FAILED\n";
  #define BinderInstCnt 8
  GLuint BindMachinInst[BinderInstCnt] = {mi_COLOROP1, GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_8X_BIT_ATI,
   GL_PRIMARY_COLOR_ARB, GL_NONE, GL_NONE};

  fp = fopen("ASMTests.txt", "at");
  fprintf(fp,"Testing: bindMachineInstToFragmentShader\n");
  // fill Machin instruction container with predefined code
  clearAllMachineInst();
  for(int i=0; i<BinderInstCnt; i++) {
    mPhase2ALU_mi.push_back(BindMachinInst[i]);
  }
  fprintf(fp,"bindMachineInstToFragmentShader succes: %s\n",bindAllMachineInstToFragmentShader()?passed:failed);
  fprintf(fp,"finished testing: bindAllMachineInstToFragmentShader\n");
  fclose(fp);
}

#endif




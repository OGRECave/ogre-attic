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


/**
	A number of invaluable references were used to put together this ps.1.x compiler for ATI_fragment_shader execution

	References:
		1. MSDN: DirectX 8.1 Reference
		2. Wolfgang F. Engel "Fundamentals of Pixel Shaders - Introduction to Shader Programming Part III" on gamedev.net
		3. Martin Ecker - XEngine
		4. Shawn Kirst - ps14toATIfs
		5. Jason L. Mitchell "Real-Time 3D Graphics With Pixel Shaders" 
		6. Jason L. Mitchell "1.4 Pixel Shaders"
		7. Jason L. Mitchell and Evan Hart "Hardware Shading with EXT_vertex_shader and ATI_fragment_shader"
		6. ATI 8500 SDK
		7. GL_ATI_fragment_shader extension reference

*/
//---------------------------------------------------------------------------
#ifndef ps_1_4H
#define ps_1_4H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "OgreGLPrerequisites.h"
#include "Compiler2Pass.h"


//---------------------------------------------------------------------------
#define ARRAYSIZE(array) (sizeof(array)/sizeof(array[0]))
// Rule Symbol group types
#define id_ANY          0xffff
#define id_REGISTER     0x1
#define id_CONSTANT     0x2
#define id_COLOR        0x4
#define id_TEXTURE      0x8
#define id_OPINST       0x10
#define id_MASK         0x20
#define id_TEXSWIZZLE   0x40
#define id_DSTMOD       0x80
#define id_ARGMOD       0x100
#define id_NUMVAL       0x200
#define id_SEPERATOR    0x400
#define id_TEXREGISTER  0x800

//Combined semantic rules to make bit fields used for semantic checks on each Token instruction
#define id_PARMETERLEFT  (id_OPINST | id_SEPERATOR | id_DSTMOD | id_MASK | id_ARGMOD)
#define id_PARMETERRIGHT  (id_SEPERATOR | id_ARGMOD | id_MASK | id_OPINST | id_DSTMOD)
#define id_ARGUMENT (id_REGISTER | id_CONSTANT | id_COLOR | id_TEXREGISTER)
#define id_MASKREPLEFT (id_ARGUMENT | id_ARGMOD)
#define id_MASKREPRIGHT (id_ARGMOD | id_SEPERATOR | id_OPINST | id_DSTMOD)
#define id_OPLEFT (id_ARGUMENT | id_DSTMOD | id_MASK | id_ARGMOD | id_NUMVAL | id_TEXSWIZZLE | id_TEXTURE)
#define id_TEMPREGISTERS (id_REGISTER | id_TEXREGISTER)

#define ALPHA_BIT 0x08
#define RGB_BITS 0x07

// Context key patterns
#define ckp_PS_BASE 0x1
#define ckp_PS_1_1  0x2
#define ckp_PS_1_2  0x4
#define ckp_PS_1_3  0x8
#define ckp_PS_1_4  0x10

#define ckp_PS_1_4_BASE (ckp_PS_BASE + ckp_PS_1_4)




/** Subclasses Compiler2Pass to provide a ps_1_x compiler that takes DirectX pixel shader assembly
	and converts it to a form that can be used by ATI_fragment_shader OpenGL API
@remarks
	all ps_1_1, ps_1_2, ps_1_3, ps_1_4 assembly instructions are recognized but not all are passed
	on to ATI_fragment_shader.	ATI_fragment_shader does not have an equivelant directive for
	texkill or texdepth instructions.

	The user must provide the GL binding interfaces.

	A Test method is provided to verify the basic operation of the compiler which outputs the test
	results to a file.


*/
class PS_1_4 : public Compiler2Pass{
private:
	enum RWAflags {rwa_NONE = 0, rwa_READ = 1, rwa_WRITE = 2};

	enum MachineInstID {mi_COLOROP1, mi_COLOROP2, mi_COLOROP3, mi_ALPHAOP1, mi_ALPHAOP2,
						mi_ALPHAOP3, mi_SETCONSTANTS, mi_PASSTEXCOORD, mi_SAMPLEMAP, mi_TEX,
						mi_TEXCOORD, mi_TEXREG2RGB, mi_NOP
	};

	struct  TokenInstType{
	  char* Name;
	  GLuint ID;

	};

	// Token ID enumeration
	enum SymbolID {
		// DirectX pixel shader source formats 
		sid_PS_1_4, sid_PS_1_1, sid_PS_1_2, sid_PS_1_3,
					
		// PS_BASE
		sid_C0, sid_C1, sid_C2, sid_C3, sid_C4, sid_C5, sid_C6, sid_C7,
		sid_V0, sid_V1,
		sid_ADD, sid_SUB, sid_MUL, sid_MAD, sid_LRP, sid_MOV, sid_CMP, sid_CND,
		sid_DP3, sid_DP4, sid_DEF,
		sid_R, sid_RA, sid_G, sid_GA, sid_B, sid_BA, sid_A, sid_RGBA, sid_RGB,
		sid_RG, sid_RGA, sid_RB, sid_RBA, sid_GB, sid_GBA,
		sid_X2, sid_X4, sid_D2, sid_SAT,
		sid_BIAS, sid_INVERT, sid_NEGATE, sid_BX2,
		sid_COMMA, sid_VALUE,

		//PS_1_4 sid
		sid_R0, sid_R1, sid_R2, sid_R3, sid_R4, sid_R5,
		sid_T0, sid_T1, sid_T2, sid_T3, sid_T4, sid_T5,
		sid_DP2ADD,
		sid_X8, sid_D8, sid_D4,
		sid_TEXCRD, sid_TEXLD,
		sid_STRDR, sid_STQDQ,
		sid_PHASE,

		//PS_1_1 sid
		sid_1R0, sid_1R1, sid_1T0, sid_1T1, sid_1T2, sid_1T3,
		sid_TEX, sid_TEXCOORD, sid_TEXM3X2PAD,
		sid_TEXM3X2TEX, sid_TEXM3X3PAD, sid_TEXM3X3TEX, sid_TEXM3X3SPEC, sid_TEXM3X3VSPEC,
		sid_TEXREG2AR, sid_TEXREG2GB,
		
		//PS_1_2 side
		sid_TEXREG2RGB, sid_TEXDP3, sid_TEXDP3TEX,

		// common
		sid_SKIP,
		sid_INVALID = BAD_TOKEN // must be last in enumeration
	};

	/// structure used to keep track of arguments and instruction parameters
	struct OpParram {
	  GLuint Arg;		// type of argument
	  bool Filled;		// has it been filled yet
	  GLuint MaskRep;	// Mask/Replicator flags
	  GLuint Mod;		// argument modifier
	};

	typedef std::vector<uint> MachineInstContainer;
	//typedef MachineInstContainer::iterator MachineInstIterator;


	// there are 2 phases with 2 subphases each
	enum PhaseType {ptPHASE1TEX, ptPHASE1ALU, ptPHASE2TEX, ptPHASE2ALU };

	struct RegModOffset {
		uint MacroOffset;
		uint RegisterBase;
		uint OpParramsIndex;
	};

	struct MacroRegModify {
		TokenInst *		Macro;
		uint			MacroSize;
		RegModOffset *	RegMods;
		uint			RegModSize;

	};

	#define R_BASE  (sid_R0 - GL_REG_0_ATI)
	#define C_BASE  (sid_C0 - GL_CON_0_ATI)
	#define T_BASE  (sid_1T0 - GL_REG_0_ATI)

	// Static Macro database for ps.1.1 ps.1.2 ps.1.3 instructions

	static TokenInstType InstTypeLib[];
	static ASMSymbolDef PS_1_4_ASMSymbolTypeLib[];
	static ASMSymbolText PS_1_4_ASMSymbolTextLib[];

	static TokenInst texreg2ar[];
	static RegModOffset texreg2xx_RegMods[];
	static MacroRegModify texreg2ar_MacroMods;

	static TokenInst texreg2gb[];
	static MacroRegModify texreg2gb_MacroMods;

	static TokenInst texdp3[];
	static RegModOffset texdp3_RegMods[];
	static MacroRegModify texdp3_MacroMods;

	static TokenInst texdp3tex[];
	static RegModOffset texdp3tex_RegMods[];
	static MacroRegModify texdp3tex_MacroMods;

	static TokenInst texm3x2pad[];
	static RegModOffset texm3xxpad_RegMods[];
	static MacroRegModify texm3x2pad_MacroMods;

	static TokenInst texm3x2tex[];
	static RegModOffset texm3xxtex_RegMods[];
	static MacroRegModify texm3x2tex_MacroMods;

	static TokenInst texm3x3pad[];
	static MacroRegModify texm3x3pad_MacroMods;

	static TokenInst texm3x3tex[];
	static MacroRegModify texm3x3tex_MacroMods;

	static TokenInst texm3x3spec[];
	static RegModOffset texm3x3spec_RegMods[];
	static MacroRegModify texm3x3spec_MacroMods;

	static TokenInst texm3x3vspec[];
	static RegModOffset texm3x3vspec_RegMods[];
	static MacroRegModify texm3x3vspec_MacroMods;


	MachineInstContainer mPhase1TEX_mi; /// machine instructions for phase one texture section
	MachineInstContainer mPhase1ALU_mi; /// machine instructions for phase one ALU section
	MachineInstContainer mPhase2TEX_mi; /// machine instructions for phase two texture section
	MachineInstContainer mPhase2ALU_mi; /// machine instructions for phase two ALU section

	MachineInstContainer* mActivePhaseMachineInstructions;
	// vars used during pass 2
	MachineInstID mOpType;
	uint mOpInst;
	int mNumArgs;
	int mArgCnt;

	#define MAXOPPARRAMS 5 // max number of parrams bound to an instruction
	
	OpParram mOpParrams[MAXOPPARRAMS];

	bool mMacroOn; // if true then put all ALU instructions in phase 1

	uint mTexm3x3padCount; // keep track of how many texm3x3pad instructions are used so know which mask to use

	uint mLastInstructionPos; // keep track of last phase 2 ALU instruction to check for R0 setting
	uint mSecondLastInstructionPos;

	// keep track of phase marker found: determines which phase ALU instructions go into
	bool mPhaseMarkerFound; 

#ifdef _DEBUG
	FILE* fp;
	// full compiler test with output results going to a text file
	void testCompile(char* testname, char* teststr, SymbolID* testresult,
		uint testresultsize, GLuint* MachinInstResults, uint MachinInstResultsSize);
#endif // _DEBUG


	// attempt to build a machine instruction using current tokens
	bool BuildMachineInst();
	
	void clearMachineInstState();

	bool checkSourceParramsReplicate();

	bool setOpParram(ASMSymbolDef* symboldef);

	// optimizes machine instructions depending on pixel shader context
	// only applies to ps.1.1 ps.1.2 and ps.1.3 since they use CISC instructions
	// that must be transformed into RISC instructions
	void optimize();

	// the method is expected to be recursive to allow for inline expansion of instructions if required
	bool Pass2scan(TokenInst * Tokens, uint size);
	// supply virtual functions for Compiler2Pass
	bool doPass2();

	bool bindMachineInstInPassToFragmentShader(const MachineInstContainer & PassMachineInstructions);

	bool expandMacro(MacroRegModify & MacroMod);

	// mainly used by tests - to slow for use in binding
	uint getMachineInst(uint Idx);
	uint getMachineInstCount();
	void addMachineInst(PhaseType phase, uint inst);
	void clearAllMachineInst();

public:

	/// constructor
	PS_1_4();
	/// binds machine instructions generated in Pass 2 to the ATI GL fragment shader
	bool bindAllMachineInstToFragmentShader();

#ifdef _DEBUG
	/// perform compiler tests - only available in _DEBUG mode
	void test();
	void testbinder();

#endif
};


#endif


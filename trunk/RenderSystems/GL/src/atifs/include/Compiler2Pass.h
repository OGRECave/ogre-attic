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


#ifndef COMPILER2PASS_H
#define COMPILER2PASS_H

#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

// FIX ME - should not be hard coded
#define BAD_TOKEN 999
#define MAX_CONSTANTS 50

typedef unsigned int uint; 



/** Compiler2Pass is a generic compiler/assembler
@remarks
	provides a tokenizer in pass 1 and relies on the subclass to provide the virtual method for pass 2 

	PASS 1 - tokenize source: this is a simple brute force lexical scanner/analyzer that also parses
			 the formed token for proper semantics and context in one pass
			 it uses Look Ahead Left-Right (LALR) ruling for semantic checking and also performs
			 context checking

	PASS 2 - generate application specific instructions ie native instructions

@par
	this class must be subclassed with the subclass providing implementation for Pass 2.  The subclass
	is responsible for setting up the token libraries along with defining the language syntax.

*/
class Compiler2Pass {

protected:

	/** structure used to build Symbol Type library */
	struct ASMSymbolDef {
	  uint mInstType;			// Instruction group type
	  uint mID;					// Token ID which is the index into the Token Type library
	  uint mPass2Data;			// data used by pass 2 to build native instructions
	  int mParrameters;			// number of parramters associated with instruction
	  int mRWAccess;			// type of access required
	  int mDefTextID;			// index into text table for default name : set at runtime
	  uint mLeftRules;			// semantic rules for left hand side
	  uint mRightRules;			// semantic rules for right hand side
	  uint mContextKey;			// context key to fit the Active Context 
	  uint mContextPatternSet;	// new pattern to set for Active Context bits
	  uint mContextPatternClear;// Contexts bits to clear Active Context bits

	};

	/** structure used to build Symbol Text library */
	struct ASMSymbolText {
	  char* mName;				// text
	  uint mID;					// Token ID associated with the text
	};

	/** structure for Token instructions */
	struct TokenInst {
	  uint mID;					// Token ID
	  int mLine;				// line number in source code where Token was found
	  int mPos;					// Character position in source where Token was found

	};

	typedef std::vector<TokenInst> TokenInstContainer;
	//typedef TokenInstContainer::iterator TokenInstIterator;

	/// container for Tokens extracted from source
	TokenInstContainer mTokenInstructions;

	/// pointer to the source to be compiled
	const char* mSource;
	int mEndOfSource;

	/// pointers to Text and Token Type libraries setup by subclass
	ASMSymbolText* mASMSymbolTextLib;
	ASMSymbolDef* mASMSymbolTypeLib;

	/// number of entries in Text and Token Type libraries
	int mASMSymbolTextLibCnt;
	int mASMSymbolTypeLibCnt;

	/// mVauleID needs to be initialized by the subclass before compiling occurs
	/// it defines the token ID used in the symbol type library
	uint mValueID;
	/// number of Constants defined 
	int mConstantsPos;
	float mConstantFloatValue;

	/// storage array for constants defined in source
	float mConstants[MAX_CONSTANTS];

	/// Active Contexts pattern used in pass 1 to determine which tokens are valid for a certain context
	uint mActiveContexts;

	/** check token semantics between ID1 and ID2 using left/right semantic data in Token Type library
	@param ID1 token ID on the left 
	@param ID2 token ID on the right
	@return
		true if both will bind to each other
		false if either fails the semantic bind test

	*/
	bool checkTokenSemantics(uint ID1, uint ID2);

	/** perform pass 1 of compile process
		scans source for symbols that can be tokenized and then
		performs general semantic and context verification on each symbol before it is tokenized.
		A tokenized instruction list is built to be used by Pass 2.

	*/
	bool doPass1();

	/** get the text symbol for this token
	@remark
		mainly used for debugging and in test routines
	@param sid is the token ID
	@return a pointer to the string text
	*/
	char* getTypeDefText(uint sid);

	/** check to see if the text is in the symbol text library
	@param symbol points to begining of text where a symbol token might exist
	@param symbolsize reference that will receive the size value of the symbol found
	@return
		true if a matching token could be found in the token type library
		false if could not be tokenized
	*/
	bool isSymbol(char* symbol, int & symbolsize);

	/** check to see if the text at the present position in the source is a numerical constant
	@param fvalue is a reference that will receive the float value that is in the source
	@param charsize reference to receive number of characters that make of the value in the source
	@return
		true if characters form a valid float representation
		false if a number value could not be extracted
	*/
	bool isFloatValue(float & fvalue, int & charsize);

	/// position to the next possible valid sysmbol
	bool positionToNextSymbol();

	/// comment specifiers are hard coded : // ; #
	void skipComments();
	void skipEOL();
	void findEOL();
	void skipWhiteSpace();


	uint Tokenize();

	// setup ActiveContexts - should be called by subclass to setup initial language contexts
	void setActiveContexts(uint contexts){ mActiveContexts = contexts; }

	/** pure virtual method that must be set up by subclass to perform Pass 2 of compile process
	@remark
		Pass 2 is for the subclass to take the token instructions generated in Pass 1 and
		build the application specific instructions along with verifying
		symantic and context rules that could not be checked in Pass 1
		
	*/
	virtual bool doPass2() = 0;

public:
	// ** these probably should not be public 
	int mCurrentLine;
	int mCharPos;

	/** Initialize the type library with matching symbol text found in symbol text library
		called by subclass after libraries setup
	*/
	void InitTypeLibText();

	/// constructor
	Compiler2Pass();

	/** compile the source - performs 2 passes
		first pass is to tokinize, check semantics and context
		second pass is performed by subclass and converts tokens to application specific instructions
	@remark
		Pass 2 only gets executed if Pass 1 has no errors
	@param source a pointer to the source text to be compiled
	@return
		true if Pass 1 and Pass 2 are successfull
		false if any errors occur in Pass 1 or Pass 2
	*/
	bool compile(const char* source);

};

#endif


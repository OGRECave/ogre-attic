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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Compiler2Pass.h"

Compiler2Pass::Compiler2Pass()
{
	mTokenInstructions.reserve(100);
	// default contexts allows all contexts
	// subclass should change it to fit the language being compiled
	mActiveContexts = 0xffffffff;

}



void Compiler2Pass::InitTypeLibText()
{

  // find a default text for all Symbol Types in library
  for(int i=0; i < mASMSymbolTypeLibCnt; i++) {
    // search for the first occurrence of text with same type
    for(int j=0; j < mASMSymbolTextLibCnt; j++) {
      if( mASMSymbolTextLib[j].mID == mASMSymbolTypeLib[i].mID) {
        mASMSymbolTypeLib[i].mDefTextID = j;
        break;
      }
    }// end of for:  j<ASMSymbolTextLibCnt
  }// end of for: i<ASMSymbolTypeLibCnt

}


bool Compiler2Pass::checkTokenSemantics(uint ID1, uint ID2)
{
  ASMSymbolDef* tokenleft = &mASMSymbolTypeLib[ID1];
  ASMSymbolDef* tokenright = &mASMSymbolTypeLib[ID2];
  // both left and right results must be true for semantic check to pass
  return (tokenleft->mRightRules & tokenright->mInstType) && (tokenright->mLeftRules & tokenleft->mInstType);

}


bool Compiler2Pass::compile(const char* source)
{
  bool Passed;

  mCurrentLine = 1;
  mCharPos = 0;
  // reset position in Constants array
  mConstantsPos = 0;
  mSource = source;
  mEndOfSource = strlen(mSource);

  // start compiling
  Passed = doPass1();
  if(Passed) {
    Passed = doPass2();
  }

  return Passed;
}


bool Compiler2Pass::doPass1()
{
	// scan through Source string and build a token list using TokenInstructions
	// this is a simple brute force lexical scanner/analyzer that also parses the formed
	// token for proper semantics and context in one pass
	bool success = true;
	bool finished = false;
	uint tokenfound;
	bool semantics_ok;
	TokenInst newtoken;

#ifdef _DEBUG
  //MessageBox(NULL, "starting pass 1", "ATI fs compiler", MB_OK);
  //MessageBox(NULL, mSource, "ATI fs compiler", MB_OK);
#endif
	// start with a clean slate
	mTokenInstructions.clear();
	// tokenize and check semantics untill an error occurs or end of source is reached
	while(success && !finished) {
		// assume going to fail
		success = false;

		if(positionToNextSymbol()) {
			tokenfound = Tokenize();
			if(tokenfound != BAD_TOKEN) {
				// a token has been found
				// check for correct semantics if there is a token to the left
				if(mTokenInstructions.size() > 0) {
					semantics_ok = checkTokenSemantics(mTokenInstructions[mTokenInstructions.size() - 1].mID, tokenfound);
				}
				else {
					semantics_ok = true;
				}


				if(semantics_ok) {
					newtoken.mID = tokenfound;
					newtoken.mLine = mCurrentLine;
					newtoken.mPos = mCharPos;
					mTokenInstructions.push_back(newtoken);
					// if the token is a constant value then put value in Constants container
					if (tokenfound == mValueID) {
						mConstants[mConstantsPos] = mConstantFloatValue;
						mConstantsPos++;
					}
					// allow token instruction to change the ActiveContexts
					// token contexts pattern clear ActiveContexts pattern bits
					mActiveContexts &= ~mASMSymbolTypeLib[tokenfound].mContextPatternClear;
					// token contexts pattern set ActiveContexts pattern bits
					mActiveContexts |= mASMSymbolTypeLib[tokenfound].mContextPatternSet;
					success = true;
				}// end of if(tokenok)
			}
		}
		else {
			// check for end of source
			if(mCharPos == mEndOfSource) {
				finished = true;
				success = true;
			}

		}
	}
	return success;
}


char* Compiler2Pass::getTypeDefText(uint sid)
{
	return mASMSymbolTextLib[mASMSymbolTypeLib[sid].mDefTextID].mName;
}


bool Compiler2Pass::isFloatValue(float& fvalue, int& charsize)
{
	// check to see if it is a numeric float value
	bool valuefound = false;

	const char* startptr = mSource + mCharPos;
	char* endptr = NULL;

	fvalue = (float)strtod(startptr, &endptr);
	// if a valid float was found then endptr will have the pointer to the first invalid character
	if(endptr) {
		if(endptr>startptr) {
			// a valid value was found so process it
			charsize = endptr - startptr;
			valuefound = true;
		}
	}

	return valuefound;
}


bool Compiler2Pass::isSymbol(char* symbol, int& symbolsize)
{
	// compare text at source+charpos with the symbol : limit testing to symbolsize
	bool symbolfound = false;
	symbolsize = strlen(symbol);
	if(strncmp(mSource + mCharPos, symbol, symbolsize)==0) {
		symbolfound = true;
	}

	return symbolfound;
}


bool Compiler2Pass::positionToNextSymbol()
{
	bool validsymbolfound = false;
	bool endofsource = false;
	while(!validsymbolfound && !endofsource) {
		skipWhiteSpace();
		// skip newline and return character 10,13
		// keep track of which line we are on
		skipEOL();
		skipComments();
		// have we reached the end of the string?
		if (mCharPos == mEndOfSource) endofsource = true;
		else
			// if ASCII > space then assume valid character is found
			if (mSource[mCharPos] > ' ') validsymbolfound = true;
			else mCharPos++;
	}// end of while

	return validsymbolfound;
}



void Compiler2Pass::skipComments()
{
  // if current char and next are // then search for EOL
	if(mCharPos < mEndOfSource) {
		if( ((mSource[mCharPos] == '/') && (mSource[mCharPos + 1] == '/')) ||
			(mSource[mCharPos] == ';') ||
			(mSource[mCharPos] == '#') ) findEOL();
	}
}


void Compiler2Pass::findEOL()
{
	// find eol charter and move to this position
	char* newpos = strchr(&mSource[mCharPos], '\n');
	if(newpos) {
		mCharPos += newpos - &mSource[mCharPos];
	}
	// couldn't find end of line so skip to the end
	else mCharPos = mEndOfSource - 1;

}


void Compiler2Pass::skipEOL()
{
	if ((mSource[mCharPos] == '\n') || (mSource[mCharPos] == '\r')) {
		mCurrentLine++;
		mCharPos++;
		if ((mSource[mCharPos] == '\n') || (mSource[mCharPos] == '\r')) {
		  mCharPos++;
		}
	}
}


void Compiler2Pass::skipWhiteSpace()
{
	// FIX - this method kinda slow
	while((mSource[mCharPos] == ' ') || (mSource[mCharPos] == '\t')) mCharPos++; // find first non white space character
}



uint Compiler2Pass::Tokenize()
{
	// assume that CharPos is at an alpha numeric character that may represent a symbol in the library
	// iterate through the library until a symbol is found
	// the same symbol could be used in different contexts and may have different meanings
	// each symbol has a context key
	// the same symbols in the library are assumed to have a unique context key so that symbol conflicts can be resolved in Pass 1
	// the symbol context key is tested in the ActiveContext pattern to see if it fits
	// if the symbol context key fits in the ActiveContext pattern then the symbol can be used as a token
	// if can't be tokenized then return BAD_TOKEN
	uint tokenID = BAD_TOKEN;
	int tokenlength = 0;
	int length;

	// scan the whole library and use the token with the longest name
	// that fits the context
	// use straight linear scan for now
	// should move to a HASH map
	for(int i= 0; i < mASMSymbolTextLibCnt; i++) {
		if(isSymbol(mASMSymbolTextLib[i].mName, length)) {
			if(length >= tokenlength) {
				// check token context key fits in Active Context
				if (mASMSymbolTypeLib[mASMSymbolTextLib[i].mID].mContextKey & mActiveContexts) {
			
					tokenID = mASMSymbolTextLib[i].mID;
					tokenlength = length;
				}

			}
		}

		// if no symbol found then check if its a numerical constant
		if (tokenID == BAD_TOKEN) {
			if(isFloatValue(mConstantFloatValue, tokenlength)) tokenID = mValueID;
		}
	}

	if(tokenID != BAD_TOKEN) mCharPos += tokenlength;

	return tokenID;

}


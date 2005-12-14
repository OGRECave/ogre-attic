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
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "OgreCompiler2Pass.h"

namespace Ogre {
    // boot strap for parsing BNF texts
    Compiler2Pass::TokenRule Compiler2Pass::BNF_RulePath[] = {
        // <syntax>     ::=  { rule }
        _rule_ BNF_SYNTAX, "<syntax>"
            _repeat_ BNF_RULE _nt_
            _end_

        // <rule>       ::=  <identifier>  "::="  <expression> ";"
        _rule_ BNF_RULE, "<rule>"
            _is_ BNF_IDENTIFIER _nt_
            _and_ BNF_SET_RULE, "::="
            _and_ BNF_EXPRESSION _nt_
            _and_ _no_token_, ";"
            _end_

        // <expression> ::=  <term> { "|" <term> }
        // <term>       ::=  <factor> { <factor> }
        // <factor>     ::=  <identifier> | <terminal_symbol> | <repeat_expression> | <optional_expression> | 

        // <repeat_expression> ::=  "{"  <identifier>  "}"
        _rule_ BNF_REPEAT_EXPRESSION, "<repeat_expression>"
            _is_ BNF_REPEAT, "{"
            _and_ BNF_IDENTIFIER _nt_
            _and_ _no_token_, "}"
            _end_

        // <optional_expression> ::= "["  <identifier>  "]"
        _rule_ BNF_OPTIONAL_EXPRESSION, "<optional_expression>"
            _is_ BNF_OPTIONAL, "["
            _and_ BNF_IDENTIFIER _nt_
            _and_ _no_token_, "]"
            _end_

        // <identifier> ::=  "<" <letter> {<letter_number>} ">"
        _rule_ BNF_IDENTIFIER, "<identifier>"
            _is_ _no_token_, "<"
            _and_ BNF_LETTER _nt_
            _repeat_ BNF_LETTER_DIGIT _nt_
            _and_ _no_token_, ">"
            _end_
        // <terminal_symbol> ::= ["-"] "'" { <any_character> } "'"

        // <letter_digit> ::= <letter> | <digit>
        _rule_ BNF_LETTER_DIGIT, "<letter_digit>"
            _is_ BNF_LETTER _nt_
            _or_ BNF_DIGIT _nt_
            _end_

        // <letter> ::= [abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ]
        _rule_ BNF_LETTER, "<letter>"
            _is_ _character_, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
            _end_

        // <digit> ::= [0123456789]
        _rule_ BNF_DIGIT, "<digit>"
            _is_ _character_, "0123456789"
            _end_

    };


    Compiler2Pass::Compiler2Pass()
	    // default contexts allows all contexts
	    // subclass should change it to fit the language being compiled
        : mActiveContexts(0xffffffff)
    {
	    // reserve some memory space in the containers being used
	    mTokenInstructions.reserve(100);
    }

    //-----------------------------------------------------------------------
    void Compiler2Pass::InitSymbolTypeLib()
    {
	    uint token_ID;
	    // find a default text for all Symbol Types in library

	    // scan through all the rules and initialize TypeLib with index to text and index to rules for non-terminal tokens
	    for (size_t i = 0; i < mRulePathLibCnt; i++) {
		    token_ID = mRootRulePath[i].mTokenID;
		    // make sure SymbolTypeLib holds valid token
		    assert(mSymbolTypeLib[token_ID].mID == token_ID);
		    switch(mRootRulePath[i].mOperation)
		    {
		    case otRULE:
			    // if operation is a rule then update typelib
			    mSymbolTypeLib[token_ID].mRuleID = i;

		    case otAND:
		    case otOR:
		    case otOPTIONAL:
			    // update text index in typelib
			    if (mRootRulePath[i].mSymbol != NULL) mSymbolTypeLib[token_ID].mDefTextID = i;
			    break;
		    }
	    }

    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::compile(const char* source)
    {
	    bool Passed = false;

	    mSource = source;
	    // start compiling if there is a rule base to work with
	    if (mRootRulePath != NULL)
	    {
		    Passed = doPass1();

		    if (Passed)
		    {
			    Passed = doPass2();
		    }
	    }

	    return Passed;
    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::doPass1()
    {
	    // scan through Source string and build a token list using TokenInstructions
	    // this is a simple brute force lexical scanner/analyzer that also parses the formed
	    // token for proper semantics and context in one pass

	    mCurrentLine = 1;
	    mCharPos = 0;
	    // reset position in Constants container
	    mConstants.clear();
        mLabels.clear();
        // there is no active label when first starting pass 1
        mLabelIsActive = false;
        mActiveLabelKey = 0;
	    mEndOfSource = strlen(mSource);

	    // start with a clean slate
	    mTokenInstructions.clear();
	    // tokenize and check semantics untill an error occurs or end of source is reached
	    // assume RootRulePath has pointer to rules so start at index + 1 for first rule path
	    // first rule token would be a rule definition so skip over it
	    bool passed = processRulePath(0);
	    // if a symbol in source still exists then the end of source was not reached and there was a problem some where
	    if (positionToNextSymbol()) passed = false;
    	
	    return passed;

    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::doPass2()
    {
        bool passed = true;
        // step through tokens container and execute until end found or error occurs

        return passed;
    }

    //-----------------------------------------------------------------------
    const Compiler2Pass::TokenInst& Compiler2Pass::getNextToken(void)
    {
        static TokenInst badToken;
        // get the current token instruction and advance instruction que index by one
        return badToken;
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::replaceToken(void)
    {
        // move instruction que index back one position
    }
    //-----------------------------------------------------------------------
    float Compiler2Pass::getNextTokenValue(void)
    {
        // get float value from current token instruction
        // if no value associated then return 0.0f
        return 0.0f;
    }
    //-----------------------------------------------------------------------
    const String& Compiler2Pass::getNextTokenLabel(void)
    {
        static String emptyString;
        // get label from current token instruction
        // if token has no label then return empty string
        return emptyString;
    }
    //-----------------------------------------------------------------------
    size_t Compiler2Pass::getTokenQueCount(void)
    {
        // calculate number of tokens between current token instruction and next token with action
        return 0;
    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::processRulePath( size_t rulepathIDX)
    {
	    // rule path determines what tokens and therefore what symbols are acceptable from the source
	    // it is assumed that the tokens with the longest similar symbols are arranged first so
	    // if a match is found it is accepted and no further searching is done

	    // record position of last token in container
	    // to be used as the rollback position if a valid token is not found
	    const size_t TokenContainerOldSize = mTokenInstructions.size();
	    const size_t OldCharPos = mCharPos;
	    const size_t OldLinePos = mCurrentLine;
	    //const size_t OldConstantsSize = mConstants.size();

	    // keep track of what non-terminal token activated the rule
	    uint ActiveNTTRule = mRootRulePath[rulepathIDX].mTokenID;
	    // start rule path at next position for definition
	    rulepathIDX++;

	    // assume the rule will pass
	    bool Passed = true;
	    bool EndFound = false;
        bool ExecutePass2 = false;

	    // keep following rulepath until the end is reached
	    while (!EndFound)
	    {
		    switch (mRootRulePath[rulepathIDX].mOperation)
		    {

		    case otAND:
			    // only validate if the previous rule passed
			    if (Passed)
				    Passed = ValidateToken(rulepathIDX, ActiveNTTRule); 
			    break;

		    case otOR:
			    // only validate if the previous rule failed
			    if ( Passed == false )
			    {
				    // clear previous tokens from entry and try again
				    mTokenInstructions.resize(TokenContainerOldSize);
				    Passed = ValidateToken(rulepathIDX, ActiveNTTRule);
			    }
			    else
			    {
				    // path passed up to this point therefore finished so pretend end marker found
				    EndFound = true;
			    }
			    break;

		    case otOPTIONAL:
			    // if previous passed then try this rule but it does not effect succes of rule since its optional
			    if(Passed) ValidateToken(rulepathIDX, ActiveNTTRule); 
			    break;

		    case otREPEAT:
			    // repeat until no called rule fails or cursor does not advance
			    // repeat is 0 or more times
			    if (Passed)
			    {
				    // keep calling until failure or no change in cursor position
            	    size_t prevPos = mCharPos;
				    while ( ValidateToken(rulepathIDX, ActiveNTTRule))
				    {
                        if (mCharPos > prevPos)
                        {
                            prevPos = mCharPos;
                        }
                        else
                        {
                            // repeat failed to advance the cursor position so time to quit since the repeating rule
                            // path isn't finding anything
                            // this can happen if the rule being called only has _optional_ rules
                            // this checking of the cursor positions prevents infinite loop from occuring
                            break;
                        }
				    }
			    }
			    break;

		    case otEND:
            case otEND_EXECUTE:
			    // end of rule found so time to return
			    EndFound = true;
			    if (Passed == false)
			    {
				    // the rule did not validate so get rid of tokens decoded
				    // roll back the token container end position to what it was when rule started
				    // this will get rid of all tokens that had been pushed on the container while
				    // trying to validating this rule
				    mTokenInstructions.resize(TokenContainerOldSize);
				    //mConstants.resize(OldConstantsSize);
				    mCharPos = OldCharPos;
				    mCurrentLine = OldLinePos;
			    }
                else
                {
                    // set execute pass 2 flag if rule requests it
                    ExecutePass2 = mRootRulePath[rulepathIDX].mOperation == otEND_EXECUTE;
                }
			    break;

		    default:
			    // an exception should be raised since the code should never get here
			    Passed = false;
			    EndFound = true;
			    break;

		    }


		    // move on to the next rule in the path
		    rulepathIDX++;
	    }

        if (ExecutePass2) doPass2();
	    return Passed;

    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::ValidateToken(const size_t rulepathIDX, const uint activeRuleID)
    {
	    size_t tokenlength = 0;
	    // assume the test is going to fail
	    bool Passed = false;
	    uint TokenID = mRootRulePath[rulepathIDX].mTokenID;
	    // only validate token if context is correct
	    if (mSymbolTypeLib[TokenID].mContextKey & mActiveContexts)
	    {
		    // if terminal token then compare text of symbol with what is in source
		    if ( mSymbolTypeLib[TokenID].mRuleID == 0)
		    {
                // position cursur to next token (non space) in script
			    if (positionToNextSymbol())
			    {
				    // if Token is supposed to be a number then check if its a numerical constant
				    if (TokenID == _value_)
				    {
        			    float constantvalue = 0.0f;
					    if (Passed = isFloatValue(constantvalue, tokenlength))
					    {
                            // key is the next instruction index
                            mConstants[mTokenInstructions.size()] = constantvalue;
					    }
				    }
                    else // check if user label or valid keyword token
                    {
                        if (TokenID == _character_)
                        {
                            // token string is list of valid single characters
                            // compare character at current cursor position in script to characters in list for a match
                            // if match found then add character to active label
                            if ( strchr(mRootRulePath[rulepathIDX].mSymbol, mSource[mCharPos]))
                            {
                                // is a new label starting?
                                // if mLabelIsActive is false then starting a new label so need a new mActiveLabelKey
                                if (!mLabelIsActive)
                                {
                                    // mActiveLabelKey will be the end of the instruction container ie the size of mTokenInstructions
                                    mActiveLabelKey = mTokenInstructions.size();
                                    mLabelIsActive = true;
                                }
                                // add the single character to the end of the active label
                                mLabels[mActiveLabelKey] += mSource[mCharPos];
                                // only one character was processed
                                tokenlength = 1;
                            }
                        }
                        else
                        {
				            // compare token symbol text with source text
				            Passed = isSymbol(mRootRulePath[rulepathIDX].mSymbol, tokenlength);
                        }
                    }

                    // turn off label processing if token ID was not for _character_
                    if (TokenID != _character_)
                    {
                        mLabelIsActive = false;
                    }
                    else // _character_ token being processed
                    {
                        // turn off generation of a new token instruction if this is not
                        // the first _character_ in a sequence of _character_ terminal tokens.
                        // Only want one _character_ token which Identifies a label

                        if (mTokenInstructions.size() > mActiveLabelKey)
                        {
                            // this token is not the first _character_ in the label sequence
                            // so turn off the token by turning TokenID into _no_token_
                            TokenID = _no_token_;
                        }
                    }

                    // if valid terminal token was found then add it to the instruction container for pass 2 processing
				    if (Passed)
				    {
                        if (TokenID != _no_token_)
                        {
					        TokenInst newtoken;
					        // push token onto end of container
					        newtoken.mID = TokenID;
					        newtoken.mNTTRuleID = activeRuleID;
					        newtoken.mLine = mCurrentLine;
					        newtoken.mPos = mCharPos;

					        mTokenInstructions.push_back(newtoken);
                        }

                        // update source position
					    mCharPos += tokenlength;

					    // allow token instruction to change the ActiveContexts
					    // use token contexts pattern to clear ActiveContexts pattern bits
					    mActiveContexts &= ~mSymbolTypeLib[TokenID].mContextPatternClear;
					    // use token contexts pattern to set ActiveContexts pattern bits
					    mActiveContexts |= mSymbolTypeLib[TokenID].mContextPatternSet;
				    }
			    }

		    }
		    // else a non terminal token was found
		    else
		    {

			    // execute rule for non-terminal
			    // get rule_ID for index into  rulepath to be called
			    Passed = processRulePath(mSymbolTypeLib[TokenID].mRuleID);
		    }
	    }


	    return Passed;

    }

    //-----------------------------------------------------------------------
    char* Compiler2Pass::getTypeDefText(const uint sid)
    {
	    return mRootRulePath[mSymbolTypeLib[sid].mDefTextID].mSymbol;
    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::isFloatValue(float& fvalue, size_t& charsize)
    {
	    // check to see if it is a numeric float value
	    bool valuefound = false;

	    const char* startptr = mSource + mCharPos;
	    char* endptr = NULL;

	    fvalue = (float)strtod(startptr, &endptr);
	    // if a valid float was found then endptr will have the pointer to the first invalid character
	    if (endptr)
	    {
		    if (endptr>startptr)
		    {
			    // a valid value was found so process it
			    charsize = endptr - startptr;
			    valuefound = true;
		    }
	    }

	    return valuefound;
    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::isSymbol(const char* symbol, size_t& symbolsize)
    {
	    // compare text at source+charpos with the symbol : limit testing to symbolsize
	    bool symbolfound = false;
	    symbolsize = strlen(symbol);
	    if (strncmp(mSource + mCharPos, symbol, symbolsize)==0)
	    {
		    symbolfound = true;
	    }

	    return symbolfound;
    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::positionToNextSymbol()
    {
	    bool validsymbolfound = false;
	    bool endofsource = false;
	    while (!validsymbolfound && !endofsource)
	    {
		    skipWhiteSpace();
		    skipEOL();
		    skipComments();
		    // have we reached the end of the string?
		    if (mCharPos == mEndOfSource)
			    endofsource = true;
		    else
		    {
			    // if ASCII > space then assume valid character is found
			    if (mSource[mCharPos] > ' ') validsymbolfound = true;
		    }
	    }// end of while

	    return validsymbolfound;
    }


    //-----------------------------------------------------------------------
    void Compiler2Pass::skipComments()
    {
    // if current char and next are // then search for EOL
	    if (mCharPos < mEndOfSource)
	    {
		    if ( ((mSource[mCharPos] == '/') && (mSource[mCharPos + 1] == '/')) ||
			    (mSource[mCharPos] == ';') ||
			    (mSource[mCharPos] == '#') )
		    {
			    findEOL();
		    }
	    }
    }


    //-----------------------------------------------------------------------
    void Compiler2Pass::findEOL()
    {
	    // find eol charter and move to this position
	    const char* newpos = strchr(&mSource[mCharPos], '\n');
	    if(newpos) {
		    mCharPos += newpos - &mSource[mCharPos];
	    }
	    // couldn't find end of line so skip to the end
	    else
		    mCharPos = mEndOfSource - 1;

    }


    //-----------------------------------------------------------------------
    void Compiler2Pass::skipEOL()
    {
	    if ((mSource[mCharPos] == '\n') || (mSource[mCharPos] == '\r'))
	    {
		    mCurrentLine++;
		    mCharPos++;
		    if ((mSource[mCharPos] == '\n') || (mSource[mCharPos] == '\r'))
		    {
			    mCharPos++;
		    }
	    }
    }


    //-----------------------------------------------------------------------
    void Compiler2Pass::skipWhiteSpace()
    {
	    // FIX - this method kinda slow
	    while ((mSource[mCharPos] == ' ') || (mSource[mCharPos] == '\t'))
		    mCharPos++; // find first non white space character
    }

    //-----------------------------------------------------------------------
    void Compiler2Pass::addLexemeToken(const String& name, const size_t key, const bool hasAction)
    {
        mLexemeTokenMap[name] = TokenDef(key, hasAction);
    }


}
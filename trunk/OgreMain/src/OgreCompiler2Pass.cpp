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
    //-----------------------------------------------------------------------
    // instantiate static members
    Compiler2Pass::TokenState Compiler2Pass::mBNFTokenState;
    //-----------------------------------------------------------------------
    Compiler2Pass::Compiler2Pass()
        : mActiveTokenState(&mBNFTokenState)
        , mSource(0)
    {
	    // reserve some memory space in the containers being used
	    mClientTokenState.mTokenQue.reserve(100);
        mClientTokenState.mLexemeTokenDefinitions.reserve(100);
	    mBNFTokenState.mTokenQue.reserve(100);
        mBNFTokenState.mLexemeTokenDefinitions.reserve(50);

        initBNFCompiler();
    }
    //-----------------------------------------------------------------------
    void Compiler2Pass::initBNFCompiler(void)
    {
        if (mBNFTokenState.mLexemeTokenDefinitions.empty())
        {
            #define TOKEN_HAS_ACTION true

            addLexemeToken("UNKNOWN", BNF_UNKOWN);
            addLexemeToken("<syntax>", BNF_SYNTAX);
            addLexemeToken("<rule>", BNF_RULE);
            addLexemeToken("<identifier>", BNF_IDENTIFIER);
            addLexemeToken("<", BNF_ID_BEGIN);
            addLexemeToken(">", BNF_ID_END);
            addLexemeToken("::=", BNF_SET_RULE);
            addLexemeToken("<expression>", BNF_EXPRESSION);
            addLexemeToken("<and_term>", BNF_AND_TERM);
            addLexemeToken("<or_term>", BNF_OR_TERM);
            addLexemeToken("<term>", BNF_TERM);
            addLexemeToken("|", BNF_OR);
            addLexemeToken("<terminal_symbol>", BNF_TERMINAL_SYMBOL);
            addLexemeToken("<repeat_expression>", BNF_REPEAT_EXPRESSION);
            addLexemeToken("{", BNF_REPEAT_BEGIN);
            addLexemeToken("}", BNF_REPEAT_END);
            addLexemeToken("<optional_expression>", BNF_OPTIONAL_EXPRESSION);
            addLexemeToken("[", BNF_OPTIONAL_BEGIN);
            addLexemeToken("]", BNF_OPTIONAL_END);
            addLexemeToken("'", BNF_SINGLEQUOTE);
            addLexemeToken("<any_character>", BNF_ANY_CHARACTER);
            addLexemeToken("<special_characters>", BNF_SPECIAL_CHARACTERS);

            addLexemeToken("<letter>", BNF_LETTER);
            addLexemeToken("<letter_digit>", BNF_LETTER_DIGIT);
            addLexemeToken("<digit>", BNF_DIGIT);
            addLexemeToken("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", BNF_ALPHA_SET);
            addLexemeToken("0123456789", BNF_NUMBER_SET);
            addLexemeToken("`~!@#$%^&*()-_=+\\|[]{}:;\"'<>,.?/", BNF_SPECIAL_CHARACTER_SET);
        }

        if (mBNFTokenState.mRootRulePath.empty())
        {
            //  used by bootstrap BNF text parser
            //  <>	- non-terminal token
            //  ()  - set of 
            // ::=	- rule definition
            #define _rule_(id)	    mBNFTokenState.mRootRulePath.push_back(TokenRule(otRULE, id));
            #define _is_(id)	    mBNFTokenState.mRootRulePath.push_back(TokenRule(otAND, id));
            //     - blank space is an implied "AND" meaning the token is required
            #define _and_(id)		mBNFTokenState.mRootRulePath.push_back(TokenRule(otAND, id));
            // |	- or
            #define _or_(id)		mBNFTokenState.mRootRulePath.push_back(TokenRule(otOR,	id));
            // []	- optional
            #define _optional_(id)	mBNFTokenState.mRootRulePath.push_back(TokenRule(otOPTIONAL, id));
            // {}	- repeat 0 or more times until fail or rule does not progress
            #define _repeat_(id)	mBNFTokenState.mRootRulePath.push_back(TokenRule(otREPEAT,	id));
            #define _data_(id)      mBNFTokenState.mRootRulePath.push_back(TokenRule(otDATA, id));
            #define _end_   		mBNFTokenState.mRootRulePath.push_back(TokenRule(otEND,0));

            // <syntax>     ::=  { rule }
            _rule_(BNF_SYNTAX) _repeat_(BNF_RULE) _end_

            // <rule>       ::=  <identifier>  "::="  <expression>
            _rule_(BNF_RULE)
                _is_(BNF_IDENTIFIER)
                _and_(BNF_SET_RULE)
                _and_(BNF_EXPRESSION)
            _end_

            // <expression> ::=  <and_term> { <or_term> }
            _rule_(BNF_EXPRESSION)
                _is_(BNF_AND_TERM)
                _repeat_(BNF_OR_TERM)
            _end_
            // <or_term>    ::=  "|" <and_term>
            _rule_(BNF_OR_TERM)
                _is_(BNF_OR)
                _and_(BNF_AND_TERM)
            _end_
            // <and_term>   ::=  <term> { <term> }
            _rule_(BNF_AND_TERM)
                _is_(BNF_TERM)
                _repeat_(BNF_TERM)
            _end_
            // <term>       ::=  <identifier> | <terminal_symbol> | <repeat_expression> | <optional_expression>
            _rule_(BNF_TERM)
                _is_(BNF_IDENTIFIER)
                _or_(BNF_TERMINAL_SYMBOL)
                _or_(BNF_REPEAT_EXPRESSION)
                _or_(BNF_OPTIONAL_EXPRESSION)
            _end_
            // <repeat_expression> ::=  "{"  <identifier>  "}"
            _rule_(BNF_REPEAT_EXPRESSION)
                _is_(BNF_REPEAT_BEGIN)
                _and_(BNF_IDENTIFIER)
                _and_(BNF_REPEAT_END)
            _end_

            // <optional_expression> ::= "["  <identifier>  "]"
            _rule_(BNF_OPTIONAL_EXPRESSION)
                _is_(BNF_OPTIONAL_BEGIN)
                _and_(BNF_IDENTIFIER)
                _and_(BNF_OPTIONAL_END)
            _end_

            // <identifier> ::=  "<" <letter> {<letter_number>} ">"
            _rule_(BNF_IDENTIFIER)
                _is_(BNF_ID_BEGIN)
                _and_(BNF_LETTER)
                _repeat_(BNF_LETTER_DIGIT)
                _and_(BNF_ID_END)
            _end_

            // <terminal_symbol> ::= "'" { <any_character> } "'"
            _rule_(BNF_TERMINAL_SYMBOL)
                _is_(BNF_SINGLEQUOTE)
                _repeat_(BNF_ANY_CHARACTER)
                _and_(BNF_SINGLEQUOTE)
            _end_

            // <any_character> ::= <letter_digit> | <special_characters>
            _rule_(BNF_ANY_CHARACTER)
                _is_(BNF_LETTER_DIGIT)
                _or_(BNF_SPECIAL_CHARACTERS)
            _end_

            // <letter_digit> ::= <letter> | <digit>
            _rule_(BNF_LETTER_DIGIT)
                _is_(BNF_LETTER)
                _or_(BNF_DIGIT)
            _end_

            // <letter> ::= (abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ)
            _rule_(BNF_LETTER)
                _is_(_character_)
                _data_(BNF_ALPHA_SET)// "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
            _end_

            // <digit> ::= (0123456789)
            _rule_(BNF_DIGIT)
                _is_(_character_)
                _data_(BNF_NUMBER_SET)
            _end_

            // <special_characters> ::= (`~!@#$%^&*()-_=+\|[]{}:;"'<>,.?/)
            _rule_(BNF_SPECIAL_CHARACTERS)
                _is_(_character_)
                _data_(BNF_SPECIAL_CHARACTER_SET)
            _end_

            // now that all the rules are added, update token definitions with rule links
            verifyTokenRuleLinks();
        }
        // switch to client state
        mActiveTokenState = &mClientTokenState;
    }

    //-----------------------------------------------------------------------
    void Compiler2Pass::verifyTokenRuleLinks()
    {
	    size_t token_ID;

	    // scan through all the rules and initialize index to rules for non-terminal tokens
        const size_t ruleCount = mActiveTokenState->mRootRulePath.size();
        for (size_t i = 0; i < ruleCount; ++i)
        {
		   // make sure token definition holds valid token
		   if (mActiveTokenState->mRootRulePath[i].mOperation == otRULE)
		   {
    		   token_ID = mActiveTokenState->mRootRulePath[i].mTokenID;
               assert(token_ID < mActiveTokenState->mLexemeTokenDefinitions.size());
        	   assert(mActiveTokenState->mLexemeTokenDefinitions[token_ID].mID == token_ID);
			   // if operation is a rule then update token definition
               mActiveTokenState->mLexemeTokenDefinitions[token_ID].mRuleID = i;
               mActiveTokenState->mLexemeTokenDefinitions[token_ID].mIsNonTerminal = true;
		   }
	    }

    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::compile(const char* source)
    {
	    bool Passed = false;

	    mSource = source;
	    // start compiling if there is a rule base to work with
        if (!mActiveTokenState->mRootRulePath.empty())
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
	    mActiveTokenState->mTokenQue.clear();
	    // tokenize and check semantics untill an error occurs or end of source is reached
	    // assume RootRulePath has pointer to rules so start at index + 1 for first rule path
	    // first rule token would be a rule definition so skip over it
	    bool passed = processRulePath(0);
	    // if a lexeme in source still exists then the end of source was not reached and there was a problem some where
	    if (positionToNextLexeme()) passed = false;
    	
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
    void Compiler2Pass::setClientBNFGrammer(const String& bnfGrammer)
    {
        // switch to internal BNF Containers
        mActiveTokenState = &mBNFTokenState;
        // clear client containers
        mClientTokenState.mTokenQue.clear();
        mClientTokenState.mLexemeTokenDefinitions.clear();
        mClientTokenState.mRootRulePath.clear();

        // attempt to compile the grammer into a rule base
        // change token state to client data after compiling grammer
        mActiveTokenState = &mClientTokenState;

    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::processRulePath( size_t rulepathIDX)
    {
	    // rule path determines what tokens and therefore what lexemes are acceptable from the source
	    // it is assumed that the tokens with the longest similar lexemes are arranged first so
	    // if a match is found it is accepted and no further searching is done

	    // record position of last token in container
	    // to be used as the rollback position if a valid token is not found
        const size_t TokenContainerOldSize = mActiveTokenState->mTokenQue.size();
	    const size_t OldCharPos = mCharPos;
	    const size_t OldLinePos = mCurrentLine;
	    //const size_t OldConstantsSize = mConstants.size();

	    // keep track of what non-terminal token activated the rule
	    size_t ActiveNTTRule = mActiveTokenState->mRootRulePath[rulepathIDX].mTokenID;
	    // start rule path at next position for definition
	    ++rulepathIDX;

	    // assume the rule will pass
	    bool Passed = true;
	    bool EndFound = false;

	    // keep following rulepath until the end is reached
	    while (!EndFound)
	    {
		    switch (mActiveTokenState->mRootRulePath[rulepathIDX].mOperation)
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
				    mActiveTokenState->mTokenQue.resize(TokenContainerOldSize);
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

            case otDATA:
                // skip it, should have been handled by previous operation.
                break;

		    case otEND:
			    // end of rule found so time to return
			    EndFound = true;
			    if (Passed == false)
			    {
				    // the rule did not validate so get rid of tokens decoded
				    // roll back the token container end position to what it was when rule started
				    // this will get rid of all tokens that had been pushed on the container while
				    // trying to validating this rule
				    mActiveTokenState->mTokenQue.resize(TokenContainerOldSize);
				    //mConstants.resize(OldConstantsSize);
				    mCharPos = OldCharPos;
				    mCurrentLine = OldLinePos;
			    }
			    break;

		    default:
			    // an exception should be raised since the code should never get here
			    Passed = false;
			    EndFound = true;
			    break;

		    }


		    // move on to the next rule in the path
		    ++rulepathIDX;
	    }

	    return Passed;

    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::isCharacterLabel(const size_t rulepathIDX)
    {
	    // assume the test is going to fail
	    bool Passed = false;

        if (mActiveTokenState->mRootRulePath[rulepathIDX].mTokenID == _character_)
        {
            // get token from next rule operation
            // token string is list of valid single characters
            // compare character at current cursor position in script to characters in list for a match
            // if match found then add character to active label
            // _character_ will not have  a token definition but the next rule operation should be 
            // DATA and have the token ID required to get the character set.
            const TokenRule& rule = mActiveTokenState->mRootRulePath[rulepathIDX + 1];
            if (rule.mOperation == otDATA)
            {
	            const size_t TokenID = rule.mTokenID;
                if ( strchr(mActiveTokenState->mLexemeTokenDefinitions[TokenID].mLexeme.c_str(), mSource[mCharPos]))
                {
                    // is a new label starting?
                    // if mLabelIsActive is false then starting a new label so need a new mActiveLabelKey
                    if (!mLabelIsActive)
                    {
                        // mActiveLabelKey will be the end of the instruction container ie the size of mTokenInstructions
                        mActiveLabelKey = mActiveTokenState->mTokenQue.size();
                        mLabelIsActive = true;
                    }
                    // add the single character to the end of the active label
                    mLabels[mActiveLabelKey] += mSource[mCharPos];
                    Passed = true;
                }
            }
        }

        return Passed;
    }
    //-----------------------------------------------------------------------
    bool Compiler2Pass::ValidateToken(const size_t rulepathIDX, const size_t activeRuleID)
    {
	    size_t tokenlength = 0;
	    // assume the test is going to fail
	    bool Passed = false;
	    size_t TokenID = mActiveTokenState->mRootRulePath[rulepathIDX].mTokenID;
	    // if terminal token then compare text of lexeme with what is in source
	    if ( !mActiveTokenState->mLexemeTokenDefinitions[TokenID].mIsNonTerminal )
	    {
            // position cursur to next token (non space) in script
		    if (positionToNextLexeme())
		    {
			    // if Token is supposed to be a number then check if its a numerical constant
			    if (TokenID == _value_)
			    {
    			    float constantvalue = 0.0f;
				    if (Passed = isFloatValue(constantvalue, tokenlength))
				    {
                        // key is the next instruction index
                        mConstants[mActiveTokenState->mTokenQue.size()] = constantvalue;
				    }
			    }
                else // check if user label or valid keyword token
                {
                    if (isCharacterLabel(rulepathIDX))
                    {
                        // only one character was processed
                        tokenlength = 1;
                    }
                    else
                    {
			            // compare token lexeme text with source text
                        Passed = isLexemeMatch(mActiveTokenState->mLexemeTokenDefinitions[TokenID].mLexeme.c_str(), tokenlength);
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

                    if (mActiveTokenState->mTokenQue.size() > mActiveLabelKey)
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
				        newtoken.mTokenID = TokenID;
				        newtoken.mNTTRuleID = activeRuleID;
				        newtoken.mLine = mCurrentLine;
				        newtoken.mPos = mCharPos;
                        newtoken.mFound = true;

				        mActiveTokenState->mTokenQue.push_back(newtoken);
                    }

                    // update source position
				    mCharPos += tokenlength;
			    }
		    }

	    }
	    // else a non terminal token was found
	    else
	    {
		    // execute rule for non-terminal
		    // get rule_ID for index into  rulepath to be called
		    Passed = processRulePath(mActiveTokenState->mLexemeTokenDefinitions[TokenID].mRuleID);
	    }

	    return Passed;

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
    bool Compiler2Pass::isLexemeMatch(const char* lexeme, size_t& lexemesize)
    {
	    // compare text at source+charpos with the lexeme : limit testing to lexemesize
	    bool lexemefound = false;
	    lexemesize = strlen(lexeme);
	    if (strncmp(mSource + mCharPos, lexeme, lexemesize)==0)
	    {
		    lexemefound = true;
	    }

	    return lexemefound;
    }

    //-----------------------------------------------------------------------
    bool Compiler2Pass::positionToNextLexeme()
    {
	    bool validlexemefound = false;
	    bool endofsource = false;
	    while (!validlexemefound && !endofsource)
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
			    if (mSource[mCharPos] > ' ') validlexemefound = true;
		    }
	    }// end of while

	    return validlexemefound;
    }


    //-----------------------------------------------------------------------
    void Compiler2Pass::skipComments()
    {
    // if current char and next are // then search for EOL
	    if (mCharPos < mEndOfSource)
	    {
		    if ( ((mSource[mCharPos] == '/') && (mSource[mCharPos + 1] == '/'))/* ||
			    (mSource[mCharPos] == ';') ||
			    (mSource[mCharPos] == '#')*/ )
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
    void Compiler2Pass::addLexemeToken(const String& lexeme, const size_t token, const bool hasAction)
    {
        if (token >= mActiveTokenState->mLexemeTokenDefinitions.size())
            mActiveTokenState->mLexemeTokenDefinitions.resize(token + 1);
        // since resizing guarentees the token definition will exist, just assign values to members
        LexemeTokenDef& tokenDef = mActiveTokenState->mLexemeTokenDefinitions[token];
        assert(tokenDef.mID == 0);
        tokenDef.mID = token;
        tokenDef.mLexeme = lexeme;
        tokenDef.mHasAction = hasAction;

        mActiveTokenState->mLexemeTokenMap[lexeme] = token;
    }


}
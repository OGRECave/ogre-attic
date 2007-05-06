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
#include "OgreScriptLexer.h"

namespace Ogre{

	ScriptLexer::ScriptLexer()
		:mIgnoreRepeatedNewlines(false)
	{
	}

	void ScriptLexer::setTokenId(const String &lexeme, uint32 id)
	{
		mUserTokens[lexeme] = id;
	}

	void ScriptLexer::removeTokenId(const String &lexeme)
	{
		mUserTokens.erase(lexeme);
	}

	void ScriptLexer::clearTokenIds()
	{
		mUserTokens.clear();
	}

	void ScriptLexer::setIgnoreRepeatedNewlines(bool b)
	{
		mIgnoreRepeatedNewlines = b;
	}

	bool ScriptLexer::getIgnoreRepeatedNewlines() const
	{
		return mIgnoreRepeatedNewlines;
	}

	ScriptTokenListPtr ScriptLexer::tokenize(const String &source)
	{
		// State enums
		enum{ READY = 0, INCOMMENT, INTOKEN, INQUOTE, INDOUBLEQUOTE, INVAR };

		// Set up some constant characters of interest
#if OGRE_WCHAR_T_STRINGS
		const wchar_t space = L' ', newline = L'\n', tab = L'\t', linefeed = L'\r',
			varopener = L'$', quote = L'\'', doublequote = L'\"', slash = L'/', backslash = L'\\';
		wchar_t c = 0, lastc = 0;
#else
		const wchar_t space = ' ', newline = '\n', tab = '\t', linefeed = '\r',
			varopener = '$', quote = '\'', doublequote = '\"', slash = '/', backslash = '\\';
		char c = 0, lastc = 0;
#endif

		String lexeme;
		uint32 line = 0, state = READY;
		ScriptTokenListPtr tokens(new ScriptTokenList());
		bool newlineAgain = false;

		// Iterate over the input
		String::const_iterator i = source.begin(), end = source.end();
		while(i != end)
		{
			lastc = c;
			c = *i;

			switch(state)
			{
			case READY:
				if(c == slash && lastc == slash)
				{
					// Comment start, clear out the lexeme
					lexeme = "";
					state = INCOMMENT;
				}
				else if(c == quote)
				{
					// Clear out the lexeme ready to be filled with quotes!
					lexeme = c;
					state = INQUOTE;
				}
				else if(c == doublequote)
				{
					lexeme = c;
					state = INDOUBLEQUOTE;
				}
				else if(c == varopener)
				{
					// Set up to read in a variable
					lexeme = c;
					state = INVAR;
				}
				else if(isNewline(c))
				{
					if(!newlineAgain || !mIgnoreRepeatedNewlines)
					{
						lexeme = c;
						setToken(lexeme, line, tokens);
						newlineAgain = true;
					}
				}
				else if(!isWhitespace(c))
				{
					lexeme = c;
					state = INTOKEN;
				}
				break;
			case INCOMMENT:
				// This newline happens to be ignored automatically
				if(isNewline(c))
					state = READY;
				break;
			case INTOKEN:
				if(isNewline(c))
				{
					setToken(lexeme, line, tokens);
					if(!newlineAgain || !mIgnoreRepeatedNewlines)
					{
						lexeme = c;
						setToken(lexeme, line, tokens);
						state = READY;
						newlineAgain = true;
					}
				}
				else if(isWhitespace(c))
				{
					newlineAgain = false;
					setToken(lexeme, line, tokens);
					state = READY;
				}
				else
				{
					lexeme += c;
				}
				break;
			case INQUOTE:
				if(c != backslash)
				{
					if(c == quote && lastc == backslash)
					{
						lexeme += c;
					}
					else if(c == quote)
					{
						lexeme += c;
						newlineAgain = false;
						setToken(lexeme, line, tokens);
						state = READY;
					}
					else
					{
						if(lastc == backslash)
							lexeme = lexeme + "\\" + c;
						else
							lexeme += c;
					}
				}
				break;
			case INDOUBLEQUOTE:
				if(c != backslash)
				{
					if(c == doublequote && lastc == backslash)
					{
						lexeme += c;
					}
					else if(c == doublequote)
					{
						lexeme += c;
						newlineAgain = false;
						setToken(lexeme, line, tokens);
						state = READY;
					}
					else
					{
						if(lastc == backslash)
							lexeme = lexeme + "\\" + c;
						else
							lexeme += c;
					}
				}
				break;
			case INVAR:
				if(isNewline(c))
				{
					setToken(lexeme, line, tokens);
					if(!newlineAgain || !mIgnoreRepeatedNewlines)
					{
						lexeme = c;
						setToken(lexeme, line, tokens);
						state = READY;
						newlineAgain = true;
					}
				}
				else if(isWhitespace(c))
				{
					newlineAgain = false;
					setToken(lexeme, line, tokens);
					state = READY;
				}
				else
				{
					lexeme += c;
				}
				break;
			}

			// Separate check for newlines just to track line numbers
			if(isNewline(c))
				line++;
			
			i++;
		}

		// Check for valid exit states
		if(state == INTOKEN || state == INVAR)
		{
			if(!lexeme.empty())
				setToken(lexeme, line, tokens);
		}
		else
		{

		}

		return tokens;
	}

	ScriptTokenListPtr ScriptLexer::tokenize(DataStreamPtr stream)
	{
		assert(!stream.isNull());
		return tokenize(stream->getAsString());
	}

	void ScriptLexer::setToken(const Ogre::String &lexeme, Ogre::uint32 line, Ogre::ScriptTokenListPtr &tokens)
	{
#if OGRE_WCHAR_T_STRINGS
			const wchar_t newline = L'\n', openBracket = L'{', closeBracket = L'}', colon = L':', 
				*import = L"import", quote = L'\'', doubleQuote = L'\"', var = L'$';
#else
			const char newline = '\n', openBracket = '{', closeBracket = '}', colon = ':', 
				*import = "import", quote = '\'', doubleQuote = '\"', var = '$';
#endif

			ScriptTokenPtr token(new ScriptToken());
			token->lexeme = lexeme;
			token->line = line;

			// Check the user token map first
			UserTokenMap::iterator i = mUserTokens.find(lexeme);
			if(i != mUserTokens.end())
				token->type = i->second;
			else if(lexeme.empty()) // Block out empty tokens first
				token->type = TID_UNKNOWN;
			else if(lexeme.size() == 1 && lexeme[0] == newline)
				token->type = TID_NEWLINE;
			else if(lexeme.size() == 1 && lexeme[0] == openBracket)
				token->type = TID_LBRACKET;
			else if(lexeme.size() == 1 && lexeme[0] == closeBracket)
				token->type = TID_RBRACKET;
			else if(lexeme.size() == 1 && lexeme[0] == colon)
				token->type = TID_COLON;
			else if(lexeme == import)
				token->type = TID_IMPORT;
			else if(lexeme[0] == var)
				token->type = TID_VARIABLE;
			else
			{
				// This is either a non-zero length phrase or quoted phrase
				if((lexeme[0] == quote && lexeme[lexeme.size() - 1] == quote) ||
					(lexeme[0] == doubleQuote && lexeme[lexeme.size() - 1] == doubleQuote))
				{
					token->type = TID_QUOTE;
				}
				else
				{
					token->type = TID_WORD;
				}
			}

			tokens->push_back(token);
	}

	bool ScriptLexer::isWhitespace(Ogre::String::value_type c) const
	{
#ifdef OGRE_WCHAR_T_STRINGS
		return c == L' ' || c == L'\r' || c == L'\t';
#else
		return c == ' ' || c == '\r' || c == '\t';
#endif
	}

	bool ScriptLexer::isNewline(Ogre::String::value_type c) const
	{
#ifdef OGRE_WCHAR_T_STRINGS
		return c == L'\n';
#else
		return c == '\n';
#endif
	}

}


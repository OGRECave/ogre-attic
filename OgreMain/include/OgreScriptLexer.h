/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#ifndef __ScriptLexer_H_
#define __ScriptLexer_H_

#include "OgrePrerequisites.h"
#include "OgreStdHeaders.h"
#include "OgreSharedPtr.h"
#include "OgreDataStream.h"

namespace Ogre {

	/** These codes represent token IDs which are numerical translations of
		specific lexemes. Specific compilers using the lexer can register their
		own token IDs which are given precedence over these built-in ones.
	*/
	enum{
		TID_LBRACKET = 0, // {
		TID_RBRACKET, // }
		TID_COLON, // :
		TID_IMPORT, // import
		TID_VARIABLE, // $...
		TID_WORD, // *
		TID_QUOTE, // "*" or '*'
		TID_NEWLINE, // \n
		TID_UNKNOWN,
		TID_END
	};

	/** This struct represents a token, which is an ID'd lexeme from the
		parsing input stream.
	*/
	struct ScriptToken
	{
		/// This is the lexeme for this token
		String lexeme;
		/// This is the id associated with the lexeme, which comes from a lexeme-token id mapping
		uint32 type;
		/// This holds the line number of the input stream where the token was found.
		uint32 line;
	};
	typedef SharedPtr<ScriptToken> ScriptTokenPtr;
	typedef std::list<ScriptTokenPtr> ScriptTokenList;
	typedef SharedPtr<ScriptTokenList> ScriptTokenListPtr;

	class _OgreExport ScriptLexer
	{
	public:
		ScriptLexer();

		/** Sets the given lexeme to be assigned the given token id value */
		void setTokenId(const String &lexeme, uint32 id);
		/** Removes the lexeme to id mapping */
		void removeTokenId(const String &lexeme);
		/** Clears all lexeme to id mappings */
		void clearTokenIds();

		/** Sets whether repeated newlines are tokenized, or ignored */
		void setIgnoreRepeatedNewlines(bool b);
		/** Returns whether repeated newlines are tokenized or ignored */
		bool getIgnoreRepeatedNewlines() const;

		/** Tokenizes the given input and returns the list of tokens found */
		ScriptTokenListPtr tokenize(const String &source);
		/** Tokenizes the string read from the given DataStream */
		ScriptTokenListPtr tokenize(DataStreamPtr stream);
	private: // Private utility operations
		void setToken(const String &lexeme, uint32 line, ScriptTokenListPtr &tokens);
		bool isWhitespace(Ogre::String::value_type c) const;
		bool isNewline(Ogre::String::value_type c) const;
	private:
		// This flag sets whether repeated newlines are ignored or stored
		bool mIgnoreRepeatedNewlines;
		// This maps lexemes to specific user ids.
		typedef std::map<String,uint32> UserTokenMap;
		// This map supercedes the built-in token ids and allows you assign your own ids to specific lexemes.
		UserTokenMap mUserTokens;
	};

}

#endif

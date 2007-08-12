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

#ifndef __SCRIPTPARSER_H_
#define __SCRIPTPARSER_H_

#include "OgrePrerequisites.h"
#include "OgreSharedPtr.h"
#include "OgreScriptLexer.h"

namespace Ogre {

	struct ScriptNode;
	typedef SharedPtr<ScriptNode> ScriptNodePtr;

	/** A list of nodes is the base structure of the AST */
	typedef std::list<ScriptNodePtr> ScriptNodeList;
	typedef SharedPtr<ScriptNodeList> ScriptNodeListPtr;

	/** This is the basic unit of the Abstract Syntax Tree.
		It holds information about the token that is parsed,
		as well as extra information for error handling.
	*/
	struct ScriptNode
	{
		String token, file;
		uint32 type, wordID;
		int line, column;
		Real data;
		bool isProperty, isObject;
		ScriptNodeList children;
		ScriptNode *parent;
	};

	/** This enum contains identifiers for the possible parsing errors */
	enum ParseError
	{
		PE_RESERVED,
		PE_OPENBRACEEXPECTED,
		PE_CLOSEBRACEEXPECTED,
		PE_IMPORTPATHEXPECTED,
		PE_IMPORTTARGETEXPECTED,
		PE_ENDQUOTEEXPECTED,
		PE_FROMEXPECTED,
		PE_PARENTOBJECTEXPECTED,
		PE_VARIABLEEXPECTED,
		PE_VARIABLEVALUEEXPECTED,
		PE_UNKNOWN
	};

	/** This enum contains type IDs for identified built-in types */
	enum
	{
		SNT_VARIABLE,
		SNT_VARIABLE_ASSIGN,
		SNT_WORD,
		SNT_NUMBER,
		SNT_IMPORT,
		SNT_QUOTE,
		SNT_LBRACE,
		SNT_RBRACE,
		SNT_COLON,
		SNT_NEWLINE,
		SNT_END_TYPES
	};

	/** This exception type stores information about a parsing error.
		It identified where in the file the error occurs and may
		include additional information about the nature of the error.
	*/
	class _OgreExport ParseErrorException : public std::exception
	{
	private:
		String mFile;
		mutable String mErrorStr;
		int mLine, mColumn;
		ParseError mError;
	public:
		ParseErrorException(const Ogre::String &file, int line, int column, ParseError err);
		
		const char *what() const;
		const Ogre::String &getFile() const;
		int getLine() const;
		ParseError getError() const;
	private:
		// Converts the error code to a string
		String getErrorString() const;
	};

	/** This is the free parse function. It takes the input and parses it into
		an AST, returning it in a ScriptNodeListPtr. If there is a parse error
		then it will throw a ParseErrorException.

		@param script This is the code for the script file
		@param source This is the source of the code for the script, for instance the file
	*/
	ScriptNodeListPtr _OgreExport parse(const String &script, const String &source);
}

#endif

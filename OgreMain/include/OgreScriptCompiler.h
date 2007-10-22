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

#ifndef __SCRIPTCOMPILER_H_
#define __SCRIPTCOMPILER_H_

#include "OgreSharedPtr.h"

namespace Ogre
{
	/** These enums hold the types of the concrete parsed nodes */
	enum ConcreteNodeType
	{
		CNT_VARIABLE,
		CNT_VARIABLE_ASSIGN,
		CNT_WORD,
		CNT_IMPORT,
		CNT_QUOTE,
		CNT_LBRACE,
		CNT_RBRACE,
		CNT_COLON,
		CNT_NEWLINE
	};

	/** The ConcreteNode is the struct that holds an un-conditioned sub-tree of parsed input */
	struct ConcreteNode;
	typedef SharedPtr<ConcreteNode> ConcreteNodePtr;
	typedef std::list<ConcreteNodePtr> ConcreteNodeList;
	typedef SharedPtr<ConcreteNodeList> ConcreteNodeListPtr;
	struct ConcreteNode
	{
		String token, file;
		unsigned int line, column;
		ConcreteNodeType type;
		ConcreteNodeList children;
		ConcreteNode *parent;
	};

	/** This enum holds the types of the possible abstract nodes */
	enum AbstractNodeType
	{
		ANT_ROOT,
		ANT_ATOM,
		ANT_OBJECT,
		ANT_PROPERTY,
		ANT_IMPORT,
		ANT_VARIABLE_SET,
		ANT_VARIABLE_ACCESS
	};
	class AbstractNode;
	typedef SharedPtr<AbstractNode> AbstractNodePtr;
	typedef std::vector<AbstractNodePtr> AbstractNodeList;
	typedef SharedPtr<AbstractNodeList> AbstractNodeListPtr;
	class AbstractNode
	{
	public:
		String file;
		unsigned int line, column;
		AbstractNodeType type;
	public:
		virtual AbstractNode *clone() const = 0; 
	};

	/** This is an abstract node which cannot be broken down further */
	class AtomAbstractNode : public AbstractNode
	{
	private:
		mutable bool mIsNumber, mNumberTest;
		mutable Real mNum;
	public:
		String value;
	public:
		AtomAbstractNode();
		AbstractNode *clone() const;
		bool isNumber() const;
		Real getNumber() const;
	};

	/** This specific abstract node represents a script object */
	class ObjectAbstractNode : public AbstractNode
	{
	public:
		String name, cls;
		bool abstract;
		AbstractNodeList children;
	public:
		ObjectAbstractNode();
		AbstractNode *clone() const;
	};

	/** This abstract node represents a script property */
	class PropertyAbstractNode : public AbstractNode
	{
	public:
		String name;
		AbstractNodeList values;
	public:
		PropertyAbstractNode();
		AbstractNode *clone() const;
	};

	/** This abstract node represents an import statement */
	class ImportAbstractNode : public AbstractNode
	{
	public:
		String target, source;
	public:
		ImportAbstractNode();
		AbstractNode *clone() const;
	};

	/** This abstract node represents a variable assignment */
	class VariableAssignAbstractNode : public AbstractNode
	{
	public:
		String name, value;
	public:
		VariableAssignAbstractNode();
		AbstractNode *clone() const;
	};

	/** This abstract node represents a variable assignment */
	class VariableAccessAbstractNode : public AbstractNode
	{
	public:
		String name;
	public:
		VariableAccessAbstractNode();
		AbstractNode *clone() const;
	};

	/** This is a listener for the compiler. The compiler can be customized with
		this listener. It lets you listen in on events occuring during compilation,
		hook them, and change the behavior.
	*/
	class _OgreExport ScriptCompilerListener
	{
	public:
		ScriptCompilerListener();

		/// Must return the requested material
		virtual Material *allocateMaterial(const String &name, const String &group);
	};

	/** This is the main class for the compiler. It calls the parser
		and processes the CST into an AST and then uses translators
		to translate the AST into the final resources.
	*/
	class _OgreExport ScriptCompiler
	{
	public:
		ScriptCompiler();

		bool compile(const String &str, const String &source, const String &group);
	};
}

#endif

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
		CNT_COLON
	};

	/** The ConcreteNode is the struct that holds an un-conditioned sub-tree of parsed input */
	struct ConcreteNode;
	typedef SharedPtr<ConcreteNode> ConcreteNodePtr;
	typedef std::list<ConcreteNodePtr> ConcreteNodeList;
	typedef SharedPtr<ConcreteNodeList> ConcreteNodeListPtr;
	struct ConcreteNode
	{
		String token, file;
		unsigned int line;
		ConcreteNodeType type;
		ConcreteNodeList children;
		ConcreteNode *parent;
	};

	/** This enum holds the types of the possible abstract nodes */
	enum AbstractNodeType
	{
		ANT_UNKNOWN,
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
		unsigned int line;
		AbstractNodeType type;
		AbstractNode *parent;
	public:
		AbstractNode(AbstractNode *ptr);
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
		uint32 id;
	public:
		AtomAbstractNode(AbstractNode *ptr);
		AbstractNode *clone() const;
		bool isNumber() const;
		Real getNumber() const;
	private:
		void parseNumber() const;
	};

	/** This specific abstract node represents a script object */
	class ObjectAbstractNode : public AbstractNode
	{
	private:
		std::map<String,String> mEnv;
	public:
		String name, cls, base;
		uint32 id;
		bool abstract;
		AbstractNodeList children;
	public:
		ObjectAbstractNode(AbstractNode *ptr);
		AbstractNode *clone() const;

		void addVariable(const String &name);
		void setVariable(const String &name, const String &value);
		String getVariable(const String &name) const;
	};

	/** This abstract node represents a script property */
	class PropertyAbstractNode : public AbstractNode
	{
	public:
		String name;
		uint32 id;
		AbstractNodeList values;
	public:
		PropertyAbstractNode(AbstractNode *ptr);
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
	class VariableAccessAbstractNode : public AbstractNode
	{
	public:
		String name;
	public:
		VariableAccessAbstractNode(AbstractNode *ptr);
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

		/// Returns the concrete node list from the given file
		virtual ConcreteNodeListPtr importFile(const String &name);
		/// Must return the requested material
		virtual Material *allocateMaterial(const String &name, const String &group);
	};

	/** This is the main class for the compiler. It calls the parser
		and processes the CST into an AST and then uses translators
		to translate the AST into the final resources.
	*/
	class _OgreExport ScriptCompiler
	{
	public: // Externally accessible types
		typedef std::map<String,uint32> IdMap;

		// The container for errors
		struct Error
		{
			String file;
			int line;
			uint32 code;
		};
		typedef SharedPtr<Error> ErrorPtr;
		typedef std::list<ErrorPtr> ErrorList;

		// These are the built-in error codes
		enum{
			CE_STRINGEXPECTED,
			CE_NUMBEREXPECTED,
			CE_FEWERPARAMETERSEXPECTED,
			CE_VARIABLEEXPECTED
		};
	public:
		ScriptCompiler();

		/// Takes in a string of script code and compiles it into resources
		/**
		 * @param str The script code
		 * @param source The source of the script code (e.g. a script file)
		 * @param group The resource group to place the compiled resources into
		 */
		bool compile(const String &str, const String &source, const String &group);
		/// Compiles resources from the given concrete node list
		bool compile(const ConcreteNodeListPtr &nodes, const String &group);
		/// Adds the given error to the compiler's list of errors
		void addError(uint32 code, const String &file, int line);
		/// Sets the listener used by the compiler
		void setListener(ScriptCompilerListener *listener);
	private: // Tree processing
		AbstractNodeListPtr convertToAST(const ConcreteNodeListPtr &nodes);
		/// This built-in function processes import nodes
		void processImports(AbstractNodeListPtr &nodes);
		/// Loads the requested script and converts it to an AST
		AbstractNodeListPtr loadImportPath(const String &name);
		/// Returns the abstract nodes from the given tree which represent the target
		AbstractNodeListPtr locateTarget(AbstractNodeList &nodes, const String &target);
		/// Handles object inheritance and variable expansion
		void processObjects(AbstractNodeList &nodes, const AbstractNodeListPtr &top);
		/// This function overlays the given object on the destination object following inheritance rules
		void overlayObject(const AbstractNodePtr &source, ObjectAbstractNode *dest);
	private:
		// Resource group
		String mGroup;
		// The word -> id conversion table
		IdMap mIds;
		// This is an environment map
		typedef std::map<String,String> Environment;
		Environment mEnv;

		typedef std::map<String,AbstractNodeListPtr> ImportCacheMap;
		ImportCacheMap mImports; // The set of imported scripts to avoid circular dependencies
		typedef std::multimap<String,String> ImportRequestMap;
		ImportRequestMap mImportRequests; // This holds the target objects for each script to be imported

		// This stores the imports of the scripts, so they are separated and can be treated specially
		AbstractNodeList mImportTable;

		// Error list
		ErrorList mErrors;

		// The listener
		ScriptCompilerListener *mListener;
	private: // Internal helper classes and processors
		class AbstractTreeBuilder
		{
		private:
			AbstractNodeListPtr mNodes;
			AbstractNode *mCurrent;
			ScriptCompiler *mCompiler;
		public:
			AbstractTreeBuilder(ScriptCompiler *compiler);
			const AbstractNodeListPtr &getResult() const;
			void visit(ConcreteNode *node);
			static void visit(AbstractTreeBuilder *visitor, const ConcreteNodeList &nodes);
		};
		friend class AbstractTreeBuilder;
	};
}

#endif

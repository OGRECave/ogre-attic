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

#include "OgreScriptParser.h"

namespace Ogre{

	/** This class is the base for all compiler listeners.
		The base class provides a mechanism for overriding the script importing
		behavior.
	*/
	class _OgreExport ScriptCompilerListener
	{
	protected:
		std::set<String> mObjectTypeNames, mPropertyNames;
	public:
		virtual ScriptNodeListPtr importFile(const String &name);
		virtual const std::set<String> &getObjectTypeNames() const;
		virtual const std::set<String> &getPropertyNames() const;
	};

	enum
	{
		CE_OPENBRACEEXPECTED
	};

	/** This struct stores semantic error information. It is information
		about errors which may occur during the script compilation phase.
		This is different than a script parsing error (syntax error).
	*/
	struct ScriptCompilerError
	{
		uint32 error;
		String file;
		int line, column;
	};
	typedef SharedPtr<ScriptCompilerError> ScriptCompilerErrorPtr;
	typedef std::list<ScriptCompilerErrorPtr> ScriptCompilerErrorList;

	/** This class acts as a base class for all compilers of Ogre scripts.
		Each script target type (e.g. Materials, Particle Systems, etc.) has
		its own derived compiler.

		The base class defines a few convenient entry points for the compilation
		process. It then pre-processes the input before handing the final
		abstract syntax tree over to the derived class for translation into
		the target form.
	*/
	class _OgreExport ScriptCompiler
	{
	public:
		ScriptCompiler();

		/** This function accepts the script text and the source of the script
			and then delegates the compilation to the derived compiler.

			@param text This is the source code of the script being compiled
			@param source This is the source of the script (usually a file or resource name)
			@param group The resource group the final resources belong to
		*/
		bool compile(const String &text, const String &group, const String &source);
		/** This function takes in a list of ScriptNodePtr objects. Essentially, this
			is an unprocessed abstract syntax tree, as it would be returned from the
			parse function. It then processes the nodes and delegates final compilation
			to the derived compiler.

			@param nodes The abstract syntax tree representing the source code to compile
			@param group The resource group the final resources belong to
		*/
		bool compile(ScriptNodeListPtr nodes, const String &group);
	protected: // Operations
		/// This is the overridable function for base classes to compile the AST
		virtual bool compileImpl(ScriptNodeListPtr nodes) = 0;
		/** This function handles inheritance expansions.

			Since these functions depend on varying features of scripts, each compiler
			must override this method and provide its own implementation.
		*/
		virtual void processObjects(ScriptNodeList &nodes, const ScriptNodeListPtr &top) = 0;
		/// This built-in function processes import nodes
		void processImports(ScriptNodeListPtr &nodes);
		/** This should be overridden by the given compiler. It is meant to load the given
			script and turn it into an AST.

			This base version uses the ResourceGroupManager to load the script
			and uses the parse function to turn it into an un-processed AST.
			The calling compiler should further process this AST if it uses
			this base version.
		*/
		virtual ScriptNodeListPtr loadImportPath(const String &name);
		/** This should be overridden by the given compiler. It located the import
			target within the AST and returns only the tree for that target object.
		*/
		virtual ScriptNodeListPtr locateImportTarget(ScriptNodeListPtr nodes, const String &target) = 0;
		/** This function is intended to do a search within the given level of the tree
			for an object of the same name. If the object exists, it should return true, otherwise
			it returns false.

			This should be overridden by derived compilers.
		*/
		virtual bool containsObject(const ScriptNodeList &nodes, const String &name) = 0;
		/// This is a utility function to provide random access to a sequential list of nodes
		ScriptNodePtr getNodeAt(const ScriptNodeList &nodes, int index) const;
		/// This is a utility for getting random access based on current iterator position
		ScriptNodePtr getNodeAt(ScriptNodeList::const_iterator from, ScriptNodeList::const_iterator end, int index) const;
		// This is a utility for locating the position of the given token in the range provided
		ScriptNodeList::const_iterator findNode(ScriptNodeList::const_iterator from, ScriptNodeList::const_iterator to, const String &token) const;
		// This is a utility for locating the position of the given token in the range provided
		ScriptNodeList::const_iterator findNode(ScriptNodeList::const_iterator from, ScriptNodeList::const_iterator to, uint32 type) const;
		// This is a utility for locating the position of the given token in the range provided
		ScriptNodeList::iterator findNode(ScriptNodeList::iterator from, ScriptNodeList::iterator to, const String &token) const;
		// This is a utility for locating the position of the given token in the range provided
		ScriptNodeList::iterator findNode(ScriptNodeList::iterator from, ScriptNodeList::iterator to, uint32 type) const;
		/// This registers a new error
		void addError(uint32 error, const String &file, int line, int col);
	protected:
		// Compiler context data
		String mGroup; // The resource group of the resultant resources
		
		typedef std::map<String,ScriptNodeListPtr> ImportCacheMap;
		ImportCacheMap mImports; // The set of imported scripts to avoid circular dependencies
		typedef std::multimap<String,String> ImportRequestMap;
		ImportRequestMap mImportRequests; // This holds the target objects for each script to be imported

		// Error information
		ScriptCompilerErrorList mErrors;
	};

}

#endif

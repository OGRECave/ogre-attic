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

#include "OgreStableHeaders.h"
#include "OgreScriptCompiler.h"
#include "OgreResourceGroupManager.h"

namespace Ogre{

	// ScriptCompilerListener
	ScriptNodeListPtr ScriptCompilerListener::importFile(const String &name)
	{
		return ScriptNodeListPtr();
	}

	// ScriptCompiler
	ScriptCompiler::ScriptCompiler()
		:mAllowNontypedObjects(false)
	{
	}

	bool ScriptCompiler::compile(const String &text, const String &group, const String &source)
	{
		// Delegate to the other overload of this function
		return compile(parse(text, source), group);
	}

	bool ScriptCompiler::compile(ScriptNodeListPtr nodes, const String &group)
	{
		// Set up the context
		mGroup = group;

		// Set up the import mechanisms
		mImportTable.clear();
		mImports.clear();
		if(!nodes->empty())
			mImports.insert(std::make_pair(nodes->front()->file, ScriptNodeListPtr()));

		// Clear errors
		mErrors.clear();

		// Set up the global scope, the top level
		mStack.clear();
		pushScope();

		processImports(nodes);
		processObjects(*nodes.get(), nodes);
		processVariables(*nodes.get(), nodes);
		return compileImpl(nodes);
	}

	bool ScriptCompiler::compile(DataStreamPtr &stream, const String &group)
	{
		return compile(stream->getAsString(), group, stream->getName());
	}

	const String &ScriptCompiler::getGroup() const
	{
		return mGroup;
	}

	const ScriptCompilerErrorList &ScriptCompiler::getErrors() const
	{
		return mErrors;
	}

	void ScriptCompiler::processVariables(ScriptNodeList &nodes, const ScriptNodeListPtr &top)
	{
		ScriptNodeList::iterator i = nodes.begin(), end = nodes.end();
		while(i != end)
		{
			ScriptNodeList::iterator cur = i++;

			if((*cur)->token == "set")
			{
				// The next token should be a var
				ScriptNodePtr next1 = getNodeAt(cur, nodes.end(), 1);
				if(next1.isNull())
					addError(CE_VARIABLEEXPECTED, (*cur)->file, (*cur)->line, (*cur)->column);
				else if(next1->type != SNT_VARIABLE)
					addError(CE_VARIABLEEXPECTED, next1->file, next1->line, next1->column);
				else
				{
					// Now we need a quote or a word as the variable value
					ScriptNodePtr next2 = getNodeAt(cur, nodes.end(), 2);
					if(next2.isNull())
						addError(CE_VARIABLEVALUEEXPECTED, next1->file, next1->line, next1->column);
					else if(next2->type != SNT_STRING && next2->type != SNT_QUOTE)
						addError(CE_VARIABLEVALUEEXPECTED, next2->file, next2->line, next2->column);
					else
					{
						// If it is a quote, we need to remove the first and last characters
						String value = next2->token;
						if(next2->type == SNT_QUOTE)
						{
							value.erase(0, 1);
							value.erase(value.size() - 1, 1);
						}

						// Insert this string into the current scope
						setVariable(next1->token, value);

						// We have used up 3 nodes now, so push the iterator forward 2 more slots
						// past the variable and the value, then remove all 3 from the node list
						// entirely
						++i; // past the variable
						++i; // past the value

						ScriptNodeList::iterator curPlus1 = cur;
						curPlus1++;
						ScriptNodeList::iterator curPlus2 = cur;
						curPlus2++;
						curPlus2++;

						nodes.erase(cur);
						nodes.erase(curPlus1);
						nodes.erase(curPlus2);
					}
				}
			}
		}

		i = nodes.begin(), end = nodes.end();
		while(i != end)
		{
			ScriptNodeList::iterator cur = i++;

			// Handle the use of variables
			if((*cur)->type == SNT_VARIABLE)
			{
				// Look up the value of the variable in the current scope
				std::pair<bool,String> var = findVariable((*cur)->token);

				// If the variable is defined, then process its value
				if(var.first)
				{
					// 1. parse, 2. inheritance, 3. variables
					ScriptNodeListPtr newNodes = parse(var.second, (*cur)->file);
					if(!newNodes.isNull() && !newNodes->empty())
					{
						processObjects(*newNodes.get(), top);
						processVariables(*newNodes.get(), top);
					}

					// Insert the AST in place of the variable usage
					if(!newNodes.isNull() && !newNodes->empty())
						nodes.insert(cur, newNodes->begin(), newNodes->end());

					// Remove the current node
					nodes.erase(cur);
				}
				else
					addError(CE_UNDEFINEDVARIABLE, (*cur)->file, (*cur)->line, (*cur)->column);
			}
			else if((*cur)->type == SNT_LBRACE)
			{
				pushScope();
				if(!(*cur)->children.empty())
					processVariables((*cur)->children, top);
			}
			else if((*cur)->type == SNT_RBRACE)
			{
				popScope();
			}
		}
	}

	void ScriptCompiler::processObjects(ScriptNodeList &nodes, const ScriptNodeListPtr &top)
	{
		ScriptNodeList::iterator i = nodes.begin(), end = nodes.end();
		while(i != end)
		{
			ScriptNodeList::iterator cur = i++;

			// Recurse so that children can take advantage of inheritance too
			if(!(*cur)->children.empty())
				processObjects((*cur)->children, top);

			if((*cur)->token == ":") // This is the indicator that inheritance is occuring
			{
				// The next token is the name of the inherited
				// The one after that should be a '{'
				ScriptNodePtr next1 = getNodeAt(cur, end, 1), next2 = getNodeAt(cur, end, 2);
				if(next2->token != "{")
					addError(CE_OPENBRACEEXPECTED, next2->file, next2->line, next2->column);
				else
				{
					// We are removing the next token too, so increment again
					ScriptNodeList::iterator next = i++;

					// next points to the name of the import target, which we need to remove
					// to avoid any name clashes during a search for it
					nodes.erase(cur);
					nodes.erase(next);

					// Use the locateImportTarget function to extract the AST of the inherited object
					// Check the top-level section of the script first
					ScriptNodeListPtr newNodes = locateTarget(*top.get(), next1->token);
					// Check the import table if nothing was found
					if(newNodes.isNull() || newNodes->empty())
						newNodes = locateTarget(mImportTable, next1->token);
					if(!newNodes.isNull() && !newNodes->empty())
					{
						// Find the '{', since we only copy the content of the inherited object
						ScriptNodeList::iterator braceIter = findNode(newNodes->begin(), newNodes->end(), SNT_LBRACE);
						if(braceIter != newNodes->end())
						{
							next2->children.insert(next2->children.begin(), 
								(*braceIter)->children.begin(), (*braceIter)->children.end());
						}
					}
				}
			}
		}
	}

	void ScriptCompiler::processImports(ScriptNodeListPtr &nodes)
	{
		// We only need to iterate over the top-level of nodes
		ScriptNodeList::iterator i = nodes->begin(), last = nodes->end();
		while(i != last)
		{
			// We move to the next node here and save the current one.
			// If any replacement happens, then we are still assured that
			// i points to the node *after* the replaced nodes, no matter
			// how many insertions and deletions may happen
			ScriptNodeList::iterator cur = i++;
			if((*cur)->type == SNT_IMPORT)
			{
				// There should be two nodes below this one
				ScriptNodePtr importTarget = getNodeAt((*cur)->children, 0),
					importPath = getNodeAt((*cur)->children, 1);

				// Remove this import statement
				nodes->erase(cur);

				// Only process if the file's contents haven't been loaded
				if(mImports.find(importPath->token) == mImports.end())
				{
					// Load the script
					ScriptNodeListPtr importedNodes = loadImportPath(importPath->token);
					if(!importedNodes.isNull() && !importedNodes->empty())
					{
						processImports(importedNodes);
						processObjects(*importedNodes.get(), importedNodes);
					}
					if(!importedNodes.isNull() && !importedNodes->empty())
						mImports.insert(std::make_pair(importPath->token, importedNodes));
				}

				// Handle the target request now
				// If it is a '*' import we remove all previous requests and just use the '*'
				// Otherwise, ensre '*' isn't already registered and register our request
				if(importTarget->token == "*")
				{
					mImportRequests.erase(mImportRequests.lower_bound(importPath->token),
						mImportRequests.upper_bound(importPath->token));
					mImportRequests.insert(std::make_pair(importPath->token, "*"));
				}
				else
				{
					ImportRequestMap::iterator iter = mImportRequests.lower_bound(importPath->token),
						end = mImportRequests.upper_bound(importPath->token);
					if(iter == end || iter->second != "*")
					{
						mImportRequests.insert(std::make_pair(importPath->token, importTarget->token));
					}
				}
			}
		}

		// All import nodes are removed
		// We have cached the code blocks from all the imported scripts
		// We can process all import requests now
		for(ImportCacheMap::iterator i = mImports.begin(); i != mImports.end(); ++i)
		{
			ImportRequestMap::iterator j = mImportRequests.lower_bound(i->first),
				end = mImportRequests.upper_bound(i->first);
			if(j != end)
			{
				if(j->second == "*")
				{
					// Insert the entire AST into the import table
					mImportTable.insert(mImportTable.begin(), i->second->begin(), i->second->end());
					continue; // Skip ahead to the next file
				}
				else
				{
					for(; j != end; ++j)
					{
						// Locate this target and insert it into the import table
						ScriptNodeListPtr newNodes = locateTarget(*(i->second.get()), j->second);
						if(!newNodes.isNull() && !newNodes->empty())
							mImportTable.insert(mImportTable.begin(), newNodes->begin(), newNodes->end());
					}
				}
			}
		}
	}

	ScriptNodeListPtr ScriptCompiler::loadImportPath(const Ogre::String &name)
	{
		ScriptNodeListPtr nodes;

		if(ResourceGroupManager::getSingletonPtr())
		{
			DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(name, mGroup);
			if(!stream.isNull())
			{
				nodes = parse(stream->getAsString(), name);
			}
		}

		return nodes;
	}

	ScriptNodeListPtr ScriptCompiler::locateTarget(ScriptNodeList &nodes, const String &target)
	{
		ScriptNodeList::iterator first = nodes.end(), last = nodes.end();
		/*
		for(ScriptNodeList::iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			// Handle the abstract case first
			if((*i)->token == "abstract")
			{
				// Check the next node
				ScriptNodePtr next1 = getNodeAt(i, nodes.end(), 1);
				if(!next1.isNull())
				{
					// Typed object check
					if(mObjectTypes.find(next1->token) != mObjectTypes.end())
					{
						// We need to check one more node for the name now
						ScriptNodePtr next2 = getNodeAt(i, nodes.end(), 2);
						if(!next2.isNull() && next2->token == target) // We found it!
						{
							first = i;
							break;
						}
					}
					// Non-typed object check
					else if(next1->token == target && mAllowNontypedObjects) // We found it!
					{
						first = i;
						break;
					}
				}
			}
			// Handle the typed object non-abstract case
			else if(mObjectTypes.find((*i)->token) != mObjectTypes.end())
			{
				// The next node should be the name
				ScriptNodePtr next1 = getNodeAt(i, nodes.end(), 1);
				if(!next1.isNull() && next1->token == target)
				{
					first = i;
					break;
				}
			}
			else if((*i)->token == target && mAllowNontypedObjects) // We found it!
			{
				first = i;
				break;
			}
		}
		*/

		// Search from the beginning for '{'
		ScriptNodeList::iterator i = findNode(nodes.begin(), nodes.end(), SNT_LBRACE);
		while(first == nodes.end() && i != nodes.end())
		{
			// From here, search backwards for the name
			ScriptNodeList::iterator j = i++;
			--j;

			while(j != nodes.begin() && ((*j)->type == SNT_NUMBER || ((*j)->type == SNT_STRING && (*j)->token != target)))
				--j;

			// Something made us stop, check if it is what we've been looking for
			if((*j)->type == SNT_STRING && (*j)->token == target)
				first = j;
			else
				i = findNode(i, nodes.end(), SNT_LBRACE);
		}

		ScriptNodeListPtr newNodes(new ScriptNodeList());

		if(first != nodes.end())
		{
			// Attempt to search for the ending brace for this object
			last = findNode(first, nodes.end(), SNT_RBRACE);
			if(last != nodes.end())
			{
				// We want it inclusive, so...
				++last;
				newNodes->insert(newNodes->begin(), first, last);
			}
		}
		return newNodes;
	}

	bool ScriptCompiler::containsObject(const ScriptNodeList &nodes, const String &name)
	{
		// Search from the beginning for '{'
		ScriptNodeList::const_iterator i = findNode(nodes.begin(), nodes.end(), SNT_LBRACE);
		while(i != nodes.end())
		{
			// From here, search backwards for the name
			ScriptNodeList::const_iterator j = i++;
			--j;

			while(j != nodes.begin() && ((*j)->type == SNT_NUMBER || ((*j)->type == SNT_STRING && (*j)->token != name)))
				--j;

			// Something made us stop, check if it is what we've been looking for
			if((*j)->type == SNT_STRING && (*j)->token == name)
				return true;
			else
				i = findNode(i, nodes.end(), SNT_LBRACE);
		}

		return false;
	}

	ScriptNodePtr ScriptCompiler::getNodeAt(const ScriptNodeList &nodes, int index) const
	{
		if(index >= nodes.size())
			return ScriptNodePtr();

		int n = 0;
		if(index >= 0)
		{
			for(ScriptNodeList::const_iterator i = nodes.begin(); i != nodes.end(); ++i, ++n)
			{
				if(n == index)
					return *i;
			}
		}
		else
		{
			for(ScriptNodeList::const_reverse_iterator i = nodes.rbegin(); i != nodes.rend(); --i, --n)
			{
				if(n == index)
					return *i;
			}
		}
		return ScriptNodePtr();
	}

	ScriptNodePtr ScriptCompiler::getNodeAt(ScriptNodeList::const_iterator from, ScriptNodeList::const_iterator end, int index) const
	{
		int n = 0;
		for(ScriptNodeList::const_iterator i = from; i != end; ++i, ++n)
		{
			if(n == index)
				return *i;
		}
		return ScriptNodePtr();
	}

	bool ScriptCompiler::getNextNode(ScriptNodeList::iterator &iter, ScriptNodeList::iterator end, Ogre::uint32 type) const
	{
		ScriptNodeList::iterator i = iter;
		++i;
		if(i == end || (*i)->type != type)
			return false;
		++iter;
		return true;
	}

	bool ScriptCompiler::nodeExists(ScriptNodeList::iterator &iter, ScriptNodeList::iterator end, uint32 index)
	{
		ScriptNodeList::iterator i = iter;
		uint32 n = 0;
		while(n != index && i != end)
		{
			++i;
			++n;
		}

		if(n == index)
			return true;
		return false;
	}

	ScriptNodeList::const_iterator ScriptCompiler::findNode(ScriptNodeList::const_iterator from, ScriptNodeList::const_iterator to, const String &token) const
	{
		ScriptNodeList::const_iterator rslt;
		for(ScriptNodeList::const_iterator i = from; i != to; ++i)
		{
			if((*i)->token == token)
			{
				rslt = i;
				break;
			}
		}
		return rslt;
	}

	ScriptNodeList::const_iterator ScriptCompiler::findNode(ScriptNodeList::const_iterator from, ScriptNodeList::const_iterator to, uint32 type) const
	{
		ScriptNodeList::const_iterator rslt = to;
		for(ScriptNodeList::const_iterator i = from; i != to; ++i)
		{
			if((*i)->type == type)
			{
				rslt = i;
				break;
			}
		}
		return rslt;
	}

	ScriptNodeList::iterator ScriptCompiler::findNode(ScriptNodeList::iterator from, ScriptNodeList::iterator to, const String &token) const
	{
		ScriptNodeList::iterator rslt;
		for(ScriptNodeList::iterator i = from; i != to; ++i)
		{
			if((*i)->token == token)
			{
				rslt = i;
				break;
			}
		}
		return rslt;
	}

	ScriptNodeList::iterator ScriptCompiler::findNode(ScriptNodeList::iterator from, ScriptNodeList::iterator to, uint32 type) const
	{
		ScriptNodeList::iterator rslt = to;
		for(ScriptNodeList::iterator i = from; i != to; ++i)
		{
			if((*i)->type == type)
			{
				rslt = i;
				break;
			}
		}
		return rslt;
	}

	bool ScriptCompiler::isTruthValue(const String &value) const
	{
		return value == "1" || value == "true" || value == "yes" || value == "on";
	}

	void ScriptCompiler::addError(uint32 error, const String &file, int line, int col)
	{
		ScriptCompilerErrorPtr err(new ScriptCompilerError());
		err->error = error;
		err->file = file;
		err->line = line;
		err->column = col;

		mErrors.push_back(err);
	}

	void ScriptCompiler::pushScope()
	{
		LexicalScopePtr scope(new LexicalScope());

		// If there are scopes on the stack, copy the top one's contents into the new scope
		if(!mStack.empty())
			scope->vars = mStack.front()->vars;

		// Push the new scope onto the stack
		mStack.push_front(scope);
	}

	void ScriptCompiler::popScope()
	{
		if(!mStack.empty())
			mStack.pop_front();
	}

	std::pair<bool,String> ScriptCompiler::findVariable(const Ogre::String &name)
	{
		std::pair<bool,String> ret;
		ret.first = false;

		if(!mStack.empty())
		{
			// Grab the top stack and do a search on its variable map
			LexicalScopePtr &scope = mStack.front();
			ScopedVariableMap::iterator i = scope->vars.find(name);
			ret.first = i != scope->vars.end();
			if(ret.first)
				ret.second = i->second;
		}

		return ret;
	}

	void ScriptCompiler::setVariable(const String &name, const String &value)
	{
		if(!mStack.empty())
			mStack.front()->vars[name] = value;
	}
}

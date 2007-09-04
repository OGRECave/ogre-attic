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
	ScriptNodeListPtr ScriptCompiler::Listener::importFile(const String &name)
	{
		return ScriptNodeListPtr();
	}

	void ScriptCompiler::Listener::preParse(Ogre::WordIDMap &ids)
	{

	}

	bool ScriptCompiler::Listener::errorRaised(const ScriptCompiler::ErrorPtr &error)
	{
		return true;
	}

	// ScriptCompiler
	ScriptCompiler::ScriptCompiler()
		:mAllowNontypedObjects(false), mListener(0)
	{
		mWordIDs["on"] = ID_ON;
		mWordIDs["off"] = ID_OFF;
		mWordIDs["true"] = ID_TRUE;
		mWordIDs["false"] = ID_FALSE;
		mWordIDs["yes"] = ID_YES;
		mWordIDs["no"] = ID_NO;
	}

	bool ScriptCompiler::compile(const String &text, const String &group, const String &source)
	{
		// Before parsing, allow a listener to modify the word id map
		preParse();

		// Delegate to the other overload of this function
		return compile(parse(text, source, mWordIDs), group);
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

	const ScriptCompiler::ErrorList &ScriptCompiler::getErrors() const
	{
		return mErrors;
	}

	const WordIDMap &ScriptCompiler::getWordIDs() const
	{
		return mWordIDs;
	}

	void ScriptCompiler::setListener(Listener *listener)
	{
		mListener = listener;
	}

	bool ScriptCompiler::compileImpl(const ScriptNodeListPtr &nodes)
	{
		return mErrors.empty();
	}

	void ScriptCompiler::processVariables(ScriptNodeList &nodes, const ScriptNodeListPtr &top)
	{
		// First we process the current level for variable assignments
		ScriptNodeList::iterator i = nodes.begin(), end = nodes.end(), cur;
		while(i != end)
		{
			cur = i++;
			if((*cur)->type == SNT_VARIABLE_ASSIGN)
			{
				// There should be two children, name and value
				ScriptNodePtr next1 = getNodeAt((*cur)->children.begin(), (*cur)->children.end(), 0),
					next2 = getNodeAt((*cur)->children.begin(), (*cur)->children.end(), 1);
				if(next1.isNull())
				{
					addError(CE_VARIABLEEXPECTED, (*cur)->file, (*cur)->line, (*cur)->column);
					continue;
				}
				else if(next1->type != SNT_VARIABLE)
				{
					addError(CE_VARIABLEEXPECTED, next1->file, next1->line, next1->column);
					continue;
				}
				if(next2.isNull())
				{
					addError(CE_VARIABLEVALUEEXPECTED, (*cur)->file, (*cur)->line, (*cur)->column);
					continue;
				}
				else if(next2->type != SNT_QUOTE)
				{
					addError(CE_VARIABLEVALUEEXPECTED, next1->file, next1->line, next1->column);
					continue;
				}

				// Insert this string into the current scope
				setVariable(next1->token, next2->token);

				// Remove the variable assignment node
				nodes.erase(cur);
			}
		}

		// Now handle variable references
		i = nodes.begin(), end = nodes.end();
		while(i != end)
		{
			ScriptNodeList::iterator cur = i++;

			// Handle the use of variables
			bool currentValid = true;
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
					currentValid = false;
				}
				else
					addError(CE_UNDEFINEDVARIABLE, (*cur)->file, (*cur)->line, (*cur)->column);
			}
			else if((*cur)->type == SNT_LBRACE)
			{
				pushScope();
			}
			else if((*cur)->type == SNT_RBRACE)
			{
				popScope();
			}

			// Descend into children first
			if(currentValid && !(*cur)->children.empty())
				processVariables((*cur)->children, top);
		}
	}

	void ScriptCompiler::processObjects(ScriptNodeList &nodes, const ScriptNodeListPtr &top)
	{
		ScriptNodeList::iterator i = nodes.begin();
		while(i != nodes.end())
		{
			ScriptNodeList::iterator cur = i++;

			// Recurse so that children can take advantage of inheritance too
			if(!(*cur)->children.empty())
				processObjects((*cur)->children, top);

			if((*cur)->type == SNT_COLON) // This is the indicator that inheritance is occuring
			{
				// The child of this node is the name of the inherited object
				// The next node should be a '{'
				ScriptNodePtr target = getNodeAt((*cur)->children.begin(), (*cur)->children.end(), 0), 
					brace = getNodeAt(cur, nodes.end(), 1);

				if(target.isNull())
				{
					addError(CE_OBJECTNAMEEXPECTED, (*cur)->file, (*cur)->line, -1);
					continue;
				}
				else if(target->type != SNT_WORD)
				{
					addError(CE_OBJECTNAMEEXPECTED, target->file, target->line, target->column);
					continue;
				}
				if(brace.isNull())
				{
					addError(CE_OBJECTNAMEEXPECTED, (*cur)->file, (*cur)->line, -1);
					continue;
				}
				else if(brace->type != SNT_LBRACE)
				{
					addError(CE_OPENBRACEEXPECTED, brace->file, brace->line, brace->column);
					continue;
				}

				// Remove the ':', along with the inherited object name
				nodes.erase(cur);

				// Use the locateImportTarget function to extract the AST of the inherited object
				// Check the top-level section of the script first
				ScriptNodeListPtr newNodes = locateTarget(*top.get(), target->token);
				// Check the import table if nothing was found
				if(newNodes.isNull() || newNodes->empty())
					newNodes = locateTarget(mImportTable, target->token);
				if(!newNodes.isNull() && !newNodes->empty())
				{
					// Find the '{', since we only copy the content of the inherited object
					ScriptNodePtr obj = *newNodes->begin();
					ScriptNodeList::iterator braceIter = 
						findNode(obj->children.begin(), obj->children.end(), SNT_LBRACE);
					if(braceIter !=  obj->children.end())
					{
						brace->children.insert(brace->children.begin(), 
							(*braceIter)->children.begin(), (*braceIter)->children.end());
					}
				}
			}
		}
	}

	void ScriptCompiler::processImports(ScriptNodeListPtr &nodes)
	{
		// We only need to iterate over the top-level of nodes
		ScriptNodeList::iterator i = nodes->begin(), last = nodes->end();
		while(i != nodes->end())
		{
			// We move to the next node here and save the current one.
			// If any replacement happens, then we are still assured that
			// i points to the node *after* the replaced nodes, no matter
			// how many insertions and deletions may happen
			ScriptNodeList::iterator cur = i++;
			if((*cur)->type == SNT_IMPORT)
			{
				// There should be two nodes below this one
				ScriptNodePtr importTarget = getNodeAt((*cur)->children.begin(), (*cur)->children.end(), 0),
					importPath = getNodeAt((*cur)->children.begin(), (*cur)->children.end(), 1);

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
				// Otherwise, ensure '*' isn't already registered and register our request
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
		ScriptNodeList::iterator iter = nodes.end();
	
		// Search for a top-level object node
		for(ScriptNodeList::iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			if((*i)->isObject)
			{
				// If only typed objects are allowed, the first child is the name
				// Otherwise, search this node and its first child
				if(mAllowNontypedObjects)
				{
					if((*i)->token == "abstract")
					{
						// It could be the first or second child
						ScriptNodeList::iterator j = (*i)->children.begin();
						if(j != (*i)->children.end() && (*j)->token == target)
						{
							iter = i;
							break;
						}

						++j;
						if(j != (*i)->children.end() && (*j)->token == target)
						{
							iter = i;
							break;
						}
					}
					else
					{
						if((*i)->token == target)
						{
							iter = i;
							break;
						}
						else
						{
							if(!(*i)->children.empty() && (*(*i)->children.begin())->token == target)
							{
								iter = i;
								break;
							}
						}
					}
				}
				else
				{
					if((*i)->token == "abstract")
					{
						// We're looking for the second child now
						ScriptNodeList::iterator j = (*i)->children.begin();
						if(j != (*i)->children.end())
							++j;
						if(j != (*i)->children.end() && (*j)->token == target)
						{
							iter = i;
							break;
						}
					}
					else
					{
						if(!(*i)->children.empty() && (*(*i)->children.begin())->token == target)
						{
							iter = i;
							break;
						}
					}
				}
			}
		}

		ScriptNodeListPtr newNodes(new ScriptNodeList());
		if(iter != nodes.end())
		{
			newNodes->push_back(*iter);
		}
		return newNodes;
	}

	void ScriptCompiler::preParse()
	{
		
	}

	bool ScriptCompiler::errorRaised(const ErrorPtr &err)
	{
		return true;
	}	

	bool ScriptCompiler::containsObject(const ScriptNodeList &nodes, const String &name)
	{
		// Search for a top-level object node
		for(ScriptNodeList::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			if((*i)->isObject)
			{
				// If only typed objects are allowed, the first child is the name
				// Otherwise, search this node and its first child
				if(mAllowNontypedObjects)
				{
					if((*i)->token == name)
					{
						return true;
					}
					else
					{
						if(!(*i)->children.empty() && (*(*i)->children.begin())->token == name)
						{
							return true;
						}
					}
				}
				else
				{
					if(!(*i)->children.empty() && (*(*i)->children.begin())->token == name)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	ScriptNodePtr ScriptCompiler::getNodeAt(ScriptNodeList::const_iterator &from, ScriptNodeList::const_iterator &end, int index) const
	{
		int n = 0;
		for(ScriptNodeList::const_iterator i = from; i != end; ++i, ++n)
		{
			if(n == index)
				return *i;
		}
		return ScriptNodePtr();
	}

	ScriptNodeList::iterator ScriptCompiler::findNode(ScriptNodeList::iterator &from, ScriptNodeList::iterator &to, uint32 type) const
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

	ScriptNodeList::const_iterator ScriptCompiler::findNode(ScriptNodeList::const_iterator &from, ScriptNodeList::const_iterator &to, uint32 type) const
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

	bool ScriptCompiler::verifyNextNodeType(ScriptNodeList::const_iterator &i, ScriptNodeList::const_iterator &end, uint32 type) const
	{
		ScriptNodeList::const_iterator j = i;
		++j;
		if(j == end)
			return false;
		return (*j)->type == type;
	}

	bool ScriptCompiler::getTruthValue(const ScriptNodePtr &node, bool &val) const
	{
		bool retval = true;
		switch(node->wordID)
		{
		case ID_ON:
			val = true;
			break;
		case ID_OFF:
			val = false;
			break;
		case ID_TRUE:
			val = true;
			break;
		case ID_FALSE:
			val = false;
			break;
		case ID_YES:
			val = true;
			break;
		case ID_NO:
			val = false;
			break;
		default:
			retval = false;
		}
		return retval;
	}

	void ScriptCompiler::addError(uint32 error, const String &file, int line, int col)
	{
		ScriptCompiler::ErrorPtr err(new ScriptCompiler::Error());
		err->error = error;
		err->file = file;
		err->line = line;
		err->column = col;

		// Allow a derived class to override the addition of this error
		if(errorRaised(err))
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

	// ScriptCompilerManager
	ScriptCompilerManager::ScriptCompilerManager()
		:mListener(0)
	{
		OGRE_THREAD_POINTER_SET(mCompiler, new ScriptCompiler());

		mScriptPatterns.push_back("*.os");
		ResourceGroupManager::getSingleton()._registerScriptLoader(this);
	}

	ScriptCompilerManager::~ScriptCompilerManager()
	{
	}

	template<> ScriptCompilerManager* Singleton<ScriptCompilerManager>::ms_Singleton = 0;
    ScriptCompilerManager* ScriptCompilerManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    ScriptCompilerManager& ScriptCompilerManager::getSingleton(void)
    {
        assert( ms_Singleton );  return ( *ms_Singleton );
    }

	const StringVector &ScriptCompilerManager::getScriptPatterns() const
	{
		return mScriptPatterns;
	}

	void ScriptCompilerManager::parseScript(DataStreamPtr &stream, const String &groupName)
	{
#if OGRE_THREAD_SUPPORT
		if(!mCompiler.get())
			mCompiler.reset(new ScriptCompiler());
#endif

		mCompiler->setListener(mListener);
		mCompiler->compile(stream, groupName);
	}

	Real ScriptCompilerManager::getLoadingOrder() const
	{
		return 100.0f;
	}
}

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

	const std::set<String> &ScriptCompilerListener::getObjectTypeNames() const
	{
		return mObjectTypeNames;
	}

	const std::set<String> &ScriptCompilerListener::getPropertyNames() const
	{
		return mPropertyNames;
	}

	// ScriptCompiler
	ScriptCompiler::ScriptCompiler()
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
		mImports.clear();
		if(!nodes->empty())
			mImports.insert(std::make_pair(nodes->front()->file, ScriptNodeListPtr()));

		// Clear errors
		mErrors.clear();

		processImports(nodes);
		processObjects(*nodes.get(), nodes);
		return compileImpl(nodes);
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
			if((*cur)->type == TOK_IMPORT)
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
					// Insert the entire AST into the top of the currently processing node tree
					nodes->insert(nodes->begin(), i->second->begin(), i->second->end());
					continue; // Skip ahead to the next file
				}
				else
				{
					for(; j != end; ++j)
					{
						// Locate this target and insert it into our AST
						ScriptNodeListPtr newNodes = locateImportTarget(i->second, j->second);
						if(!newNodes.isNull() && !newNodes->empty())
							nodes->insert(nodes->begin(), newNodes->begin(), newNodes->end());
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

	void ScriptCompiler::addError(uint32 error, const String &file, int line, int col)
	{
		ScriptCompilerErrorPtr err(new ScriptCompilerError());
		err->error = error;
		err->file = file;
		err->line = line;
		err->column = col;

		mErrors.push_back(err);
	}

}

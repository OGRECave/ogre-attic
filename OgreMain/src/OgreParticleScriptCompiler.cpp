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
#include "OgreParticleScriptCompiler.h"

namespace Ogre{

	// ParticleScriptCompilerListener
	ParticleScriptCompilerListener::ParticleScriptCompilerListener()
	{
	}

	// ParticleScriptCompiler
	ParticleScriptCompiler::ParticleScriptCompiler()
		:mListener(0)
	{
	}

	void ParticleScriptCompiler::setListener(ParticleScriptCompilerListener *listener)
	{
		mListener = listener;
	}

	bool ParticleScriptCompiler::compileImpl(ScriptNodeListPtr nodes)
	{
		return false;
	}

	void ParticleScriptCompiler::processObjects(ScriptNodeList &nodes, const ScriptNodeListPtr &top)
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
					// Use the locateImportTarget function to extract the AST of the inherited object
					ScriptNodeListPtr newNodes = locateImportTarget(top, next1->token);
					if(!newNodes.isNull() && !newNodes->empty())
					{
						// Find the '{', since we only copy the content of the inherited object
						ScriptNodeList::iterator braceIter = findNode(newNodes->begin(), newNodes->end(), TOK_LBRACE);
						if(braceIter != newNodes->end())
							next2->children.insert(next2->children.begin(), (*braceIter)->children.begin(), (*braceIter)->children.end());
					}

					// We are removing the next token too, so increment again
					ScriptNodeList::iterator next = i++;

					nodes.erase(cur);
					nodes.erase(next);
				}
			}
		}
	}

	ScriptNodeListPtr ParticleScriptCompiler::loadImportPath(const Ogre::String &name)
	{
		ScriptNodeListPtr nodes;

		// Try the listener
		if(mListener)
			nodes = mListener->importFile(name);

		// Try the base version
		if(nodes.isNull())
			nodes = ScriptCompiler::loadImportPath(name);

		// If we got any AST loaded, do the necessary pre-processing steps
		if(!nodes.isNull())
		{
			// Expand all imports
			processImports(nodes);
		}

		return nodes;
	}

	ScriptNodeListPtr ParticleScriptCompiler::locateImportTarget(ScriptNodeListPtr nodes, const String &target)
	{
		// Particle scripts can support a number of target forms:
		// abstract <Name>
		// abstract <Type> <Name>
		// <Name>
		// The types can be "emitter" and "affector" and so are invalid as names
		// We only search the top-level because inheritance from top-level objects
		// is all that is supported

		ScriptNodeList::iterator first = nodes->end(), last = nodes->end();
		for(ScriptNodeList::iterator i = nodes->begin(); i != nodes->end(); ++i)
		{
			// Handle the abstract case first
			if((*i)->token == "abstract")
			{
				// Check the next node
				ScriptNodePtr next1 = getNodeAt(i, nodes->end(), 1);
				if(!next1.isNull())
				{
					if(next1->token == "emitter" || next1->token == "affector" ||
						(mListener && mListener->getObjectTypeNames().find(next1->token) != mListener->getObjectTypeNames().end()))
					{
						// We need to check one more node for the name now
						ScriptNodePtr next2 = getNodeAt(i, nodes->end(), 2);
						if(!next2.isNull() && next2->token == target) // We found it!
						{
							first = i;
							break;
						}
					}
					else if(next1->token == target) // We found it!
					{
						first = i;
						break;
					}
				}
			}
			else if((*i)->token == target) // We found it!
			{
				first = i;
				break;
			}
		}

		ScriptNodeListPtr newNodes(new ScriptNodeList());

		if(first != nodes->end())
		{
			// Attempt to search for the ending brace for this object
			last = findNode(first, nodes->end(), TOK_RBRACE);
			if(last != nodes->end())
			{
				// We want it inclusive, so...
				++last;
				newNodes->insert(newNodes->begin(), first, last);
			}
		}
		return newNodes;
	}

	bool ParticleScriptCompiler::containsObject(const ScriptNodeList &nodes, const String &name)
	{
		// Follows the same rules as the function above for determining objects
		for(ScriptNodeList::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			// Handle the abstract case first
			if((*i)->token == "abstract")
			{
				// Check the next node
				ScriptNodePtr next1 = getNodeAt(i, nodes.end(), 1);
				if(!next1.isNull())
				{
					if(next1->token == "emitter" || next1->token == "affector" ||
						(mListener && mListener->getObjectTypeNames().find(next1->token) != mListener->getObjectTypeNames().end()))
					{
						// We need to check one more node for the name now
						ScriptNodePtr next2 = getNodeAt(i, nodes.end(), 2);
						if(!next2.isNull() && next2->token == name) // We found it!
						{
							return true;
						}
					}
					else if(next1->token == name) // We found it!
					{
						return true;
					}
				}
			}
			else if((*i)->token == name) // We found it!
			{
				return true;
			}
		}

		return false;
	}
}
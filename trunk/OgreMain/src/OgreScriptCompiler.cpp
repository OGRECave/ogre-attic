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
#include "OgreScriptLexer.h"
#include "OgreScriptParser.h"
#include "OgreLogManager.h"
#include "OgreMaterialManager.h"

namespace Ogre
{
	// AbstractNode
	AbstractNode::AbstractNode(AbstractNode *ptr)
		:parent(ptr), type(ANT_UNKNOWN), line(0)
	{}

	// AtomAbstractNode
	AtomAbstractNode::AtomAbstractNode(AbstractNode *ptr)
		:AbstractNode(ptr), mIsNumber(false), mNumberTest(false)
	{
		type = ANT_ATOM;
	}

	AbstractNode *AtomAbstractNode::clone() const
	{
		AtomAbstractNode *node = new AtomAbstractNode(parent);
		node->file = file;
		node->line = line;
		node->id = id;
		node->type = type;
		node->value = value;
		node->mIsNumber = false;
		node->mNumberTest = false;
		node->mNum = 0.0f;
		return node;
	}

	bool AtomAbstractNode::isNumber() const
	{
		if(mNumberTest)
			return mIsNumber;
		else
		{
			parseNumber();
			mNumberTest = true;
		}
		return mIsNumber;
	}

	Real AtomAbstractNode::getNumber() const
	{
		if(mNumberTest)
			return mNum;
		else
		{
			parseNumber();
			mNumberTest = true;
		}
		return mNum;
	}

	void AtomAbstractNode::parseNumber() const
	{
		if(StringConverter::isNumber(value))
		{
			mNum = StringConverter::parseReal(value);
			mNumberTest = true;
		}
	}

	// ObjectAbstractNode
	ObjectAbstractNode::ObjectAbstractNode(AbstractNode *ptr)
		:AbstractNode(ptr), abstract(false)
	{
		type = ANT_OBJECT;
	}

	AbstractNode *ObjectAbstractNode::clone() const
	{
		ObjectAbstractNode *node = new ObjectAbstractNode(parent);
		node->file = file;
		node->line = line;
		node->type = type;
		node->name = name;
		node->cls = cls;
		node->id = id;
		node->abstract = abstract;
		for(AbstractNodeList::const_iterator i = children.begin(); i != children.end(); ++i)
			node->children.push_back(AbstractNodePtr((*i)->clone()));
		node->mEnv = mEnv;
		return node;
	}

	void ObjectAbstractNode::addVariable(const Ogre::String &name)
	{
		mEnv.insert(std::make_pair(name, ""));
	}

	void ObjectAbstractNode::setVariable(const Ogre::String &name, const Ogre::String &value)
	{
		mEnv[name] = value;
	}

	std::pair<bool,String> ObjectAbstractNode::getVariable(const String &name) const
	{
		std::map<String,String>::const_iterator i = mEnv.find(name);
		if(i != mEnv.end())
			return std::make_pair(true, i->second);

		ObjectAbstractNode *parent = (ObjectAbstractNode*)this->parent;
		while(parent)
		{
			i = parent->mEnv.find(name);
			if(i != mEnv.end())
				return std::make_pair(true, i->second);
			parent = (ObjectAbstractNode*)parent->parent;
		}
		return std::make_pair(false, "");
	}

	// PropertyAbstractNode
	PropertyAbstractNode::PropertyAbstractNode(AbstractNode *ptr)
		:AbstractNode(ptr)
	{
		type = ANT_PROPERTY;
	}

	AbstractNode *PropertyAbstractNode::clone() const
	{
		PropertyAbstractNode *node = new PropertyAbstractNode(parent);
		node->file = file;
		node->line = line;
		node->type = type;
		node->name = name;
		node->id = id;
		for(AbstractNodeList::const_iterator i = values.begin(); i != values.end(); ++i)
			node->values.push_back(AbstractNodePtr((*i)->clone()));
		return node;
	}

	// ImportAbstractNode
	ImportAbstractNode::ImportAbstractNode()
		:AbstractNode(0)
	{
		type = ANT_IMPORT;
	}

	AbstractNode *ImportAbstractNode::clone() const
	{
		ImportAbstractNode *node = new ImportAbstractNode();
		node->file = file;
		node->line = line;
		node->type = type;
		node->target = target;
		node->source = source;
		return node;
	}

	// VariableAccessAbstractNode
	VariableAccessAbstractNode::VariableAccessAbstractNode(AbstractNode *ptr)
		:AbstractNode(ptr)
	{
		type = ANT_VARIABLE_ACCESS;
	}

	AbstractNode *VariableAccessAbstractNode::clone() const
	{
		VariableAccessAbstractNode *node = new VariableAccessAbstractNode(parent);
		node->file = file;
		node->line = line;
		node->type = type;
		node->name = name;
		return node;
	}

	// ScriptCompilerListener
	ScriptCompilerListener::ScriptCompilerListener()
	{
	}

	ConcreteNodeListPtr ScriptCompilerListener::importFile(const String &name)
	{
		return ConcreteNodeListPtr();
	}

	Material *ScriptCompilerListener::allocateMaterial(const String &name, const String &group)
	{
		return (Material*)MaterialManager::getSingleton().create(name, group).get();
	}

	// ScriptCompiler
	ScriptCompiler::ScriptCompiler()
	{
	}

	bool ScriptCompiler::compile(const String &str, const String &source, const String &group)
	{
		ScriptLexer lexer;
		ScriptParser parser;
		ConcreteNodeListPtr nodes = parser.parse(lexer.tokenize(str, source));
		return compile(nodes, group);
	}

	bool ScriptCompiler::compile(const ConcreteNodeListPtr &nodes, const String &group)
	{
		// Set up the compilation context
		mGroup = group;

		// Clear the past errors
		mErrors.clear();

		// Clear the environment
		mEnv.clear();

		// Convert our nodes to an AST
		AbstractNodeListPtr ast = convertToAST(nodes);
		// Processes the imports for this script
		processImports(ast);
		// Process object inheritance
		processObjects(ast.get(), ast);
		// Process variable expansion
		processVariables(ast.get());
		return true;
	}

	void ScriptCompiler::addError(uint32 code, const Ogre::String &file, int line)
	{
		ErrorPtr err(new Error());
		err->code = code;
		err->file = file;
		err->line = line;
		mErrors.push_back(err);
	}

	void ScriptCompiler::setListener(ScriptCompilerListener *listener)
	{
		mListener = listener;
	}

	AbstractNodeListPtr ScriptCompiler::convertToAST(const Ogre::ConcreteNodeListPtr &nodes)
	{
		AbstractTreeBuilder builder(this);
		AbstractTreeBuilder::visit(&builder, *nodes.get());
		return builder.getResult();
	}

	void ScriptCompiler::processImports(Ogre::AbstractNodeListPtr &nodes)
	{
		// We only need to iterate over the top-level of nodes
		AbstractNodeList::iterator i = nodes->begin(), last = nodes->end();
		while(i != nodes->end())
		{
			// We move to the next node here and save the current one.
			// If any replacement happens, then we are still assured that
			// i points to the node *after* the replaced nodes, no matter
			// how many insertions and deletions may happen
			AbstractNodeList::iterator cur = i++;
			if((*cur)->type == ANT_IMPORT)
			{
				ImportAbstractNode *import = (ImportAbstractNode*)(*cur).get();
				// Only process if the file's contents haven't been loaded
				if(mImports.find(import->source) == mImports.end())
				{
					// Load the script
					AbstractNodeListPtr importedNodes = loadImportPath(import->source);
					if(!importedNodes.isNull() && !importedNodes->empty())
					{
						processImports(importedNodes);
						processObjects(importedNodes.get(), importedNodes);
					}
					if(!importedNodes.isNull() && !importedNodes->empty())
						mImports.insert(std::make_pair(import->source, importedNodes));
				}

				// Handle the target request now
				// If it is a '*' import we remove all previous requests and just use the '*'
				// Otherwise, ensure '*' isn't already registered and register our request
				if(import->target == "*")
				{
					mImportRequests.erase(mImportRequests.lower_bound(import->source),
						mImportRequests.upper_bound(import->source));
					mImportRequests.insert(std::make_pair(import->source, "*"));
				}
				else
				{
					ImportRequestMap::iterator iter = mImportRequests.lower_bound(import->source),
						end = mImportRequests.upper_bound(import->source);
					if(iter == end || iter->second != "*")
					{
						mImportRequests.insert(std::make_pair(import->source, import->target));
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
						AbstractNodeListPtr newNodes = locateTarget(i->second.get(), j->second);
						if(!newNodes.isNull() && !newNodes->empty())
							mImportTable.insert(mImportTable.begin(), newNodes->begin(), newNodes->end());
					}
				}
			}
		}
	}

	AbstractNodeListPtr ScriptCompiler::loadImportPath(const Ogre::String &name)
	{
		AbstractNodeListPtr retval;
		ConcreteNodeListPtr nodes;

		if(mListener)
			nodes = mListener->importFile(name);

		if(nodes.isNull() && ResourceGroupManager::getSingletonPtr())
		{
			DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(name, mGroup);
			if(!stream.isNull())
			{
				ScriptLexer lexer;
				ScriptTokenListPtr tokens = lexer.tokenize(stream->getAsString(), name);
				ScriptParser parser;
				nodes = parser.parse(tokens);
			}
		}

		if(!nodes.isNull())
			retval = convertToAST(nodes);

		return retval;
	}

	AbstractNodeListPtr ScriptCompiler::locateTarget(AbstractNodeList *nodes, const Ogre::String &target)
	{
		AbstractNodeList::iterator iter = nodes->end();
	
		// Search for a top-level object node
		for(AbstractNodeList::iterator i = nodes->begin(); i != nodes->end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				ObjectAbstractNode *impl = (ObjectAbstractNode*)(*i).get();
				if(impl->name == target)
					iter = i;
			}
		}

		AbstractNodeListPtr newNodes(new AbstractNodeList());
		if(iter != nodes->end())
		{
			newNodes->push_back(*iter);
		}
		return newNodes;
	}

	void ScriptCompiler::processObjects(Ogre::AbstractNodeList *nodes, const Ogre::AbstractNodeListPtr &top)
	{
		for(AbstractNodeList::iterator i = nodes->begin(); i != nodes->end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				ObjectAbstractNode *obj = (ObjectAbstractNode*)(*i).get();

				// Check if it is inheriting anything
				if(!obj->base.empty())
				{
					// Check the top level first, then check the import table
					AbstractNodeListPtr newNodes = locateTarget(top.get(), obj->base);
					if(newNodes->empty())
						newNodes = locateTarget(&mImportTable, obj->base);

					if(!newNodes->empty())
					{
						for(AbstractNodeList::iterator j = newNodes->begin(); j != newNodes->end(); ++j)
							overlayObject(*j, obj);
					}
				}

				// Recurse into children
				processObjects(&obj->children, top);
			}
		}
	}

	void ScriptCompiler::overlayObject(const AbstractNodePtr &source, ObjectAbstractNode *dest)
	{
		if(source->type == ANT_OBJECT)
		{
			ObjectAbstractNode *src = (ObjectAbstractNode*)source.get();
			
			// Queue up all transfers
			std::list<AbstractNodePtr> queue;
			queue.insert(queue.begin(), src->children.begin(), src->children.end());

			// Index each source node type
			std::map<String,int> srcIndexMap;

			std::list<AbstractNodePtr>::iterator i = queue.begin();
			while(i != queue.end())
			{
				// Move forward and store current position
				std::list<AbstractNodePtr>::iterator cur = i;
				i++;

				// Only process if it is an object
				if((*cur)->type == ANT_OBJECT)
				{
					ObjectAbstractNode *srcObj = (ObjectAbstractNode*)(*cur).get();
					srcIndexMap[srcObj->cls]++;
					int srcIndex = srcIndexMap[srcObj->cls];

					// Search through destination for an object of this type at this index
					AbstractNodeList::iterator dst_iter = dest->children.end();
					int dstIndex = 0;
					for(AbstractNodeList::iterator j = dest->children.begin(); j != dest->children.end(); ++j)
					{
						if((*j)->type == ANT_OBJECT)
						{
							ObjectAbstractNode *dstObj = (ObjectAbstractNode*)(*j).get();
							if(dstObj->cls == srcObj->cls)
							{
								// Found the right object, so store our searched results
								dstIndex++;
								dst_iter = j;
								if(dstIndex == srcIndex)
									break; // Break if we've reached our goals of the n-th object of the source type
							}
						}
					}

					if(srcIndex == dstIndex && dst_iter != dest->children.end())
					{
						// Overlay the source on the destination node
						overlayObject(*cur, (ObjectAbstractNode*)(*dst_iter).get());
						// Remove the node from the source queue
						queue.erase(cur);
					}
					else if(dst_iter != dest->children.end())
					{
						// This means we found nodes of the right type, but not enough. Copy our after the one we did find.
						AbstractNodeList::iterator next = dst_iter;
						next++;

						AbstractNodePtr newNode = AbstractNodePtr(srcObj->clone());
						newNode->parent = dest;
						dest->children.insert(next, newNode);
						// Remove the node from the source queue
						queue.erase(cur);
					}
				}
			}

			// Insert the remainder into the front of the destination
			AbstractNodeList::iterator k = dest->children.begin();
			for(std::list<AbstractNodePtr>::iterator j = queue.begin(); j != queue.end(); ++j)
			{
				AbstractNodePtr newNode((*j)->clone());
				newNode->parent = dest;
				dest->children.insert(k, newNode);
			}
		}
	}

	void ScriptCompiler::processVariables(Ogre::AbstractNodeList *nodes)
	{
		AbstractNodeList::iterator i = nodes->begin();
		while(i != nodes->end())
		{
			AbstractNodeList::iterator cur = i;
			++i;

			if((*cur)->type == ANT_OBJECT)
			{
				// Only process if this object is not abstract
				ObjectAbstractNode *obj = (ObjectAbstractNode*)(*cur).get();
				if(!obj->abstract)
					processVariables(&obj->children);
			}
			else if((*cur)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = (PropertyAbstractNode*)(*cur).get();
				processVariables(&prop->values);
			}
			else if((*cur)->type == ANT_VARIABLE_ACCESS)
			{
				VariableAccessAbstractNode *var = (VariableAccessAbstractNode*)(*cur).get();

				// Look up the enclosing scope
				ObjectAbstractNode *scope = 0;
				AbstractNode *temp = var->parent;
				while(temp)
				{
					if(temp->type == ANT_OBJECT)
					{
						scope = (ObjectAbstractNode*)temp;
						break;
					}
					temp = temp->parent;
				}

				// Look up the variable in the environment
				std::pair<bool,String> varAccess;
				if(scope)
					varAccess = scope->getVariable(var->name);
				if(!scope || !varAccess.first)
				{
					std::map<String,String>::iterator k = mEnv.find(var->name);
					varAccess.first = k != mEnv.end();
					if(varAccess.first)
						varAccess.second = k->second;
				}

				if(varAccess.first)
				{
					// Found the variable, so process it and insert it into the tree
					ScriptLexer lexer;
					ScriptTokenListPtr tokens = lexer.tokenize(varAccess.second, var->file);
					ScriptParser parser;
					ConcreteNodeListPtr cst = parser.parseChunk(tokens);
					AbstractNodeListPtr ast = convertToAST(cst);

					// Set up ownership for these nodes
					for(AbstractNodeList::iterator j = ast->begin(); j != ast->end(); ++j)
						(*j)->parent = var->parent;

					// Recursively handle variable accesses within the variable expansion
					processVariables(ast.get());

					// Insert the nodes in place of the variable
					nodes->insert(cur, ast->begin(), ast->end());
				}
				else
				{
					// Error
					addError(CE_UNDEFINEDVARIABLE, var->file, var->line);
				}

				// Remove the variable node
				nodes->erase(cur);
			}
		}
	}

	// AbstractTreeeBuilder
	ScriptCompiler::AbstractTreeBuilder::AbstractTreeBuilder(ScriptCompiler *compiler)
		:mCurrent(0), mNodes(new AbstractNodeList()), mCompiler(compiler)
	{
	}

	const AbstractNodeListPtr &ScriptCompiler::AbstractTreeBuilder::getResult() const
	{
		return mNodes;
	}

	void ScriptCompiler::AbstractTreeBuilder::visit(ConcreteNode *node)
	{
		AbstractNodePtr asn;

		// Import = "import" >> 2 children, mCurrent == null
		if(node->type == CNT_IMPORT && mCurrent == 0)
		{
			if(node->children.size() > 2)
			{
				mCompiler->addError(CE_FEWERPARAMETERSEXPECTED, node->file, node->line);
				goto fail;
			}
			if(node->children.size() < 2)
			{
				mCompiler->addError(CE_STRINGEXPECTED, node->file, node->line);
				goto fail;
			}

			ImportAbstractNode *impl = new ImportAbstractNode();
			impl->line = node->line;
			impl->file = node->file;
			
			ConcreteNodeList::iterator iter = node->children.begin();
			impl->target = (*iter)->token;

			iter++;
			impl->source = (*iter)->token;

			asn = AbstractNodePtr(impl);
		}
		// variable set = "set" >> 2 children, children[0] == variable
		else if(node->type == CNT_VARIABLE_ASSIGN)
		{
			if(node->children.size() > 2)
			{
				mCompiler->addError(CE_FEWERPARAMETERSEXPECTED, node->file, node->line);
				goto fail;
			}
			if(node->children.size() < 2)
			{
				mCompiler->addError(CE_STRINGEXPECTED, node->file, node->line);
				goto fail;
			}
			if(node->children.front()->type != CNT_VARIABLE)
			{
				mCompiler->addError(CE_VARIABLEEXPECTED, node->children.front()->file, node->children.front()->line);
				goto fail;
			}

			ConcreteNodeList::iterator i = node->children.begin();
			String name = (*i)->token;

			++i;
			String value = (*i)->token;

			if(mCurrent && mCurrent->type == ANT_OBJECT)
			{
				ObjectAbstractNode *ptr = (ObjectAbstractNode*)mCurrent;
				ptr->setVariable(name, value);
			}
			else
			{
				mCompiler->mEnv.insert(std::make_pair(name, value));
			}
		}
		// variable = $*, no children
		else if(node->type == CNT_VARIABLE)
		{
			if(!node->children.empty())
			{
				mCompiler->addError(CE_FEWERPARAMETERSEXPECTED, node->file, node->line);
				goto fail;
			}

			VariableAccessAbstractNode *impl = new VariableAccessAbstractNode(mCurrent);
			impl->line = node->line;
			impl->file = node->file;
			impl->name = node->token;

			asn = AbstractNodePtr(impl);
		}
		// Handle properties and objects here
		else if(!node->children.empty())
		{
			// Grab the last two nodes
			ConcreteNodePtr temp1, temp2;
			ConcreteNodeList::const_reverse_iterator riter = node->children.rbegin();
			if(riter != node->children.rend())
			{
				temp1 = *riter;
				riter++;
			}
			if(riter != node->children.rend())
				temp2 = *riter;

			// object = last 2 children == { and }
			if(!temp1.isNull() && !temp2.isNull() &&
				temp1->type == CNT_RBRACE && temp2->type == CNT_LBRACE)
			{
				if(node->children.size() < 2)
				{
					mCompiler->addError(CE_STRINGEXPECTED, node->file, node->line);
					goto fail;
				}

				ObjectAbstractNode *impl = new ObjectAbstractNode(mCurrent);
				impl->line = node->line;
				impl->file = node->file;
				impl->abstract = false;

				// Create a temporary detail list
				std::list<ConcreteNode*> temp;
				if(node->token == "abstract")
				{
					impl->abstract = true;
					for(ConcreteNodeList::const_iterator i = node->children.begin(); i != node->children.end(); ++i)
						temp.push_back((*i).get());
				}
				else
				{
					temp.push_back(node);
					for(ConcreteNodeList::const_iterator i = node->children.begin(); i != node->children.end(); ++i)
						temp.push_back((*i).get());
				}

				// Get the type of object
				std::list<ConcreteNode*>::const_iterator iter = temp.begin();
				impl->cls = (*iter)->token;
				++iter;

				// Get the name
				if(iter != temp.end() && ((*iter)->type == CNT_WORD || (*iter)->type == CNT_QUOTE))
				{
					impl->name = (*iter)->token;
					++iter;
				}

				// Find the base
				if(iter != temp.end() && (*iter)->type == CNT_COLON)
				{
					if((*iter)->children.empty())
					{
						mCompiler->addError(CE_STRINGEXPECTED, (*iter)->file, (*iter)->line);
						goto fail;
					}
					impl->base = (*iter)->children.front()->token;
				}

				// Finally try to map the cls to an id
				ScriptCompiler::IdMap::const_iterator iter2 = mCompiler->mIds.find(impl->cls);
				if(iter2 != mCompiler->mIds.end())
					impl->id = iter2->second;

				asn = AbstractNodePtr(impl);
				mCurrent = impl;

				// Visit the children of the {
				AbstractTreeBuilder::visit(this, temp2->children);

				// Go back up the stack
				mCurrent = impl->parent;
			}
			// Otherwise, it is a property
			else
			{
				PropertyAbstractNode *impl = new PropertyAbstractNode(mCurrent);
				impl->line = node->line;
				impl->file = node->file;
				impl->name = node->token;

				ScriptCompiler::IdMap::const_iterator iter2 = mCompiler->mIds.find(impl->name);
				if(iter2 != mCompiler->mIds.end())
					impl->id = iter2->second;

				asn = AbstractNodePtr(impl);
				mCurrent = impl;

				// Visit the children of the {
				AbstractTreeBuilder::visit(this, node->children);

				// Go back up the stack
				mCurrent = impl->parent;
			}
		}
		// Otherwise, it is a standard atom
		else
		{
			AtomAbstractNode *impl = new AtomAbstractNode(mCurrent);
			impl->line = node->line;
			impl->file = node->file;
			impl->value = node->token;

			ScriptCompiler::IdMap::const_iterator iter2 = mCompiler->mIds.find(impl->value);
			if(iter2 != mCompiler->mIds.end())
				impl->id = iter2->second;

			asn = AbstractNodePtr(impl);
		}

		// Here, we must insert the node into the tree
		if(!asn.isNull())
		{
			if(mCurrent)
			{
				if(mCurrent->type == ANT_PROPERTY)
				{
					PropertyAbstractNode *impl = reinterpret_cast<PropertyAbstractNode*>(mCurrent);
					impl->values.push_back(asn);
				}
				else
				{
					ObjectAbstractNode *impl = reinterpret_cast<ObjectAbstractNode*>(mCurrent);
					impl->children.push_back(asn);
				}
			}
			else
			{
				mNodes->push_back(asn);
			}
		}

		// Failure point
		fail:;
	}

	void ScriptCompiler::AbstractTreeBuilder::visit(AbstractTreeBuilder *visitor, const ConcreteNodeList &nodes)
	{
		for(ConcreteNodeList::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
			visitor->visit((*i).get());
	}
}
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
		return node;
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

	// VariableAssignAbstractNode
	VariableAssignAbstractNode::VariableAssignAbstractNode(AbstractNode *ptr)
		:AbstractNode(ptr)
	{
		type = ANT_VARIABLE_SET;
	}

	AbstractNode *VariableAssignAbstractNode::clone() const
	{
		VariableAssignAbstractNode *node = new VariableAssignAbstractNode(parent);
		node->file = file;
		node->line = line;
		node->type = type;
		node->name = name;
		node->value = value;
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

		// Convert our nodes to an AST
		AbstractNodeListPtr ast = convertToAST(nodes);
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

	AbstractNodeListPtr ScriptCompiler::convertToAST(const Ogre::ConcreteNodeListPtr &nodes)
	{
		AbstractTreeBuilder builder(this);
		AbstractTreeBuilder::visit(&builder, *nodes.get());
		return builder.getResult();
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

			VariableAssignAbstractNode *impl = new VariableAssignAbstractNode(mCurrent);
			impl->line = node->line;
			impl->file = node->file;
			
			ConcreteNodeList::iterator iter = node->children.begin();
			impl->name = (*iter)->token;

			iter++;
			impl->value = (*iter)->token;

			asn = AbstractNodePtr(impl);
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
		else if(mCurrent != 0)
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
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
#include "OgreLogManager.h"
#include "OgreMaterialManager.h"

namespace Ogre
{
	extern ConcreteNodeListPtr parse(const String &script, const String &source);
	bool parseNumber(const String &script, Ogre::Real &num);

	// AtomAbstractNode
	AtomAbstractNode::AtomAbstractNode()
	{
	}

	AbstractNode *AtomAbstractNode::clone() const
	{
		AtomAbstractNode *node = new AtomAbstractNode();
		node->file = file;
		node->line = line;
		node->column = column;
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
			mIsNumber = parseNumber(value, mNum);
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
			mIsNumber = parseNumber(value, mNum);
			mNumberTest = true;
		}
		return mNum;
	}

	// ObjectAbstractNode
	ObjectAbstractNode::ObjectAbstractNode()
		:abstract(false)
	{
	}

	AbstractNode *ObjectAbstractNode::clone() const
	{
		ObjectAbstractNode *node = new ObjectAbstractNode();
		node->file = file;
		node->line = line;
		node->column = column;
		node->type = type;
		node->name = name;
		node->cls = cls;
		node->abstract = abstract;
		for(AbstractNodeList::const_iterator i = children.begin(); i != children.end(); ++i)
			node->children.push_back(AbstractNodePtr((*i)->clone()));
		return node;
	}

	// PropertyAbstractNode
	PropertyAbstractNode::PropertyAbstractNode()
	{
	}

	AbstractNode *PropertyAbstractNode::clone() const
	{
		PropertyAbstractNode *node = new PropertyAbstractNode();
		node->file = file;
		node->line = line;
		node->column = column;
		node->type = type;
		node->name = name;
		for(AbstractNodeList::const_iterator i = values.begin(); i != values.end(); ++i)
			node->values.push_back(AbstractNodePtr((*i)->clone()));
		return node;
	}

	// ImportAbstractNode
	ImportAbstractNode::ImportAbstractNode()
	{
	}

	AbstractNode *ImportAbstractNode::clone() const
	{
		ImportAbstractNode *node = new ImportAbstractNode();
		node->file = file;
		node->line = line;
		node->column = column;
		node->type = type;
		node->target = target;
		node->source = source;
		return node;
	}

	// VariableAssignAbstractNode
	VariableAssignAbstractNode::VariableAssignAbstractNode()
	{
	}

	AbstractNode *VariableAssignAbstractNode::clone() const
	{
		VariableAssignAbstractNode *node = new VariableAssignAbstractNode();
		node->file = file;
		node->line = line;
		node->column = column;
		node->type = type;
		node->name = name;
		node->value = value;
		return node;
	}

	// VariableAccessAbstractNode
	VariableAccessAbstractNode::VariableAccessAbstractNode()
	{
	}

	AbstractNode *VariableAccessAbstractNode::clone() const
	{
		VariableAccessAbstractNode *node = new VariableAccessAbstractNode();
		node->file = file;
		node->line = line;
		node->column = column;
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

	static void printConcreteNodes(const ConcreteNodeList &nodes, int tab)
	{
		String tabs = "";
		for(int i = 0; i < tab; ++i)
			tabs += "\t";
		for(ConcreteNodeList::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			LogManager::getSingleton().logMessage(tabs + (*i)->token);
			printConcreteNodes((*i)->children, tab + 1);
		}
	}

	bool ScriptCompiler::compile(const String &str, const String &source, const String &group)
	{
		ConcreteNodeListPtr nodes = parse(str, source);
		printConcreteNodes(*nodes.get(), 0);
		return true;
	}
}
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
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreParticleSystemManager.h"
#include "OgreParticleSystemRenderer.h"
#include "OgreParticleEmitter.h"
#include "OgreParticleAffector.h"
#include "OgreCompositorManager.h"
#include "OgreCompositionTechnique.h"
#include "OgreCompositionTargetPass.h"
#include "OgreCompositionPass.h"

#define OBJ_ERROR(err)	getCompiler()->addError((err), obj->file, obj->line)
#define PROP_ERROR(err) getCompiler()->addError((err), prop->file, prop->line)

namespace Ogre
{
	// This function converts a compile error code to a string description
	static String convertErrorCode(uint32 code)
	{
		switch(code)
		{
		case ScriptCompiler::CE_STRINGEXPECTED:
			return "string expected";
		case ScriptCompiler::CE_NUMBEREXPECTED:
			return "number expected";
		case ScriptCompiler::CE_FEWERPARAMETERSEXPECTED:
			return "fewer parameters expected";
		case ScriptCompiler::CE_VARIABLEEXPECTED:
			return "variable expected";
		case ScriptCompiler::CE_UNDEFINEDVARIABLE:
			return "undefined variable";
		case ScriptCompiler::CE_OBJECTNAMEEXPECTED:
			return "object name expected";
		case ScriptCompiler::CE_OBJECTALLOCATIONERROR:
			return "object allocation error";
		case ScriptCompiler::CE_INVALIDPARAMETERS:
			return "invalid parameters";
		default:
			return "unknown error";
		}
	}

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
			mIsNumber = true;
		}
	}

	// ObjectAbstractNode
	ObjectAbstractNode::ObjectAbstractNode(AbstractNode *ptr)
		:AbstractNode(ptr), abstract(false), id(0)
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
		{
			AbstractNodePtr newNode = AbstractNodePtr((*i)->clone());
			newNode->parent = node;
			node->children.push_back(newNode);
		}
		for(AbstractNodeList::const_iterator i = values.begin(); i != values.end(); ++i)
		{
			AbstractNodePtr newNode = AbstractNodePtr((*i)->clone());
			newNode->parent = node;
			node->values.push_back(newNode);
		}
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
			if(i != parent->mEnv.end())
				return std::make_pair(true, i->second);
			parent = (ObjectAbstractNode*)parent->parent;
		}
		return std::make_pair(false, "");
	}

	const std::map<String,String> &ObjectAbstractNode::getVariables() const
	{
		return mEnv;
	}

	// PropertyAbstractNode
	PropertyAbstractNode::PropertyAbstractNode(AbstractNode *ptr)
		:AbstractNode(ptr), id(0)
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
		{
			AbstractNodePtr newNode = AbstractNodePtr((*i)->clone());
			newNode->parent = node;
			node->values.push_back(newNode);
		}
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

	void ScriptCompilerListener::preASTConversion(ConcreteNodeListPtr nodes, ScriptCompiler::IdMap *ids)
	{
		
	}

	std::pair<bool,ScriptCompiler::Translator*> ScriptCompilerListener::preObjectTranslation(ObjectAbstractNode *obj)
	{
		return std::make_pair(false, (ScriptCompiler::Translator*)0);
	}

	std::pair<bool,ScriptCompiler::Translator*> ScriptCompilerListener::prePropertyTranslation(PropertyAbstractNode *prop)
	{
		return std::make_pair(false, (ScriptCompiler::Translator*)0);
	}

	void ScriptCompilerListener::error(const ScriptCompiler::ErrorPtr &err)
	{
		Ogre::String msg = "Compiler error: ";
		msg = msg + convertErrorCode(err->code) + " in " + err->file + " " +
			Ogre::StringConverter::toString(err->line);
		Ogre::LogManager::getSingleton().logMessage(msg);
	}

	MaterialPtr ScriptCompilerListener::createMaterial(const String &name, const String &group)
	{
		return (MaterialPtr)MaterialManager::getSingleton().create(name, group);
	}

	void ScriptCompilerListener::preApplyTextureAliases(AliasTextureNamePairList *aliases)
	{
	}

	void ScriptCompilerListener::getTextureNames(Ogre::String *names, int count)
	{
	}

	void ScriptCompilerListener::getGpuProgramName(String *name)
	{
	}

	GpuProgramPtr ScriptCompilerListener::createGpuProgram(const String &name, const String &group, const String &source, GpuProgramType type, const String &syntax)
	{
		GpuProgramPtr prog = (GpuProgramPtr)GpuProgramManager::getSingleton().createProgram(name, group, source, type, syntax);
		return prog;
	}

	HighLevelGpuProgramPtr ScriptCompilerListener::createHighLevelGpuProgram(const String &name, const String &group, const String &language, GpuProgramType type, const String &source)
	{
		HighLevelGpuProgramPtr prog = (HighLevelGpuProgramPtr)HighLevelGpuProgramManager::getSingleton().createProgram(name, group, language, type);
		prog->setSourceFile(source);
		return prog;
	}

	ParticleSystem *ScriptCompilerListener::createParticleSystem(const String &name, const String &group)
	{
		ParticleSystem *system = (ParticleSystem*)ParticleSystemManager::getSingleton().createTemplate(name, group);
		return system;
	}

	void ScriptCompilerListener::getMaterialName(Ogre::String *name)
	{
	}

	CompositorPtr ScriptCompilerListener::createCompositor(const Ogre::String &name, const Ogre::String &group)
	{
		return CompositorManager::getSingleton().create(name, group);
	}

	// ScriptCompiler
	ScriptCompiler::ScriptCompiler()
		:mListener(0)
	{
		initWordMap();
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

		// Translate the nodes
		for(AbstractNodeList::iterator i = ast->begin(); i != ast->end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				ObjectAbstractNode *obj = (ObjectAbstractNode*)(*i).get();
				switch(obj->id)
				{
				case ID_MATERIAL:
					{	
						MaterialTranslator translator;
						Translator::translate(&translator, *i, this);
					}
					break;
				case ID_PARTICLE_SYSTEM:
					{
						ParticleSystemTranslator translator;
						Translator::translate(&translator, *i, this);
					}
					break;
				case ID_COMPOSITOR:
					{
						CompositorTranslator translator;
						Translator::translate(&translator, *i, this);
					}
					break;
				case ID_VERTEX_PROGRAM:
				case ID_FRAGMENT_PROGRAM:
					{
						if(!obj->values.empty())
						{
							if(obj->values.front()->type == ANT_ATOM)
							{
								String language = ((AtomAbstractNode*)obj->values.front().get())->value;
								if(language == "asm")
								{
									GpuProgramTranslator translator;
									Translator::translate(&translator, *i, this);
								}
								else if(language == "unified")
								{
									UnifiedGpuProgramTranslator translator;
									Translator::translate(&translator, *i, this);
								}
								else
								{
									HighLevelGpuProgramTranslator translator;
									Translator::translate(&translator, *i, this);
								}
							}
							else
							{
								addError(CE_INVALIDPARAMETERS, obj->file, obj->line);
							}
						}
						else
						{
							addError(CE_STRINGEXPECTED, obj->file, obj->line);
						}
					}
					break;
				default:
					Translator::translate((Translator*)0, *i, this);
				}
			}
		}

		return mErrors.empty();
	}

	void ScriptCompiler::addError(uint32 code, const Ogre::String &file, int line)
	{
		ErrorPtr err(new Error());
		err->code = code;
		err->file = file;
		err->line = line;

		if(mListener)
		{
			mListener->error(err);
		}
		else
		{
			Ogre::String msg = "Compiler error: ";
			msg = msg + convertErrorCode(code) + " in " + file + " " +
				Ogre::StringConverter::toString(line);
			Ogre::LogManager::getSingleton().logMessage(msg);
		}

		mErrors.push_back(err);
	}

	void ScriptCompiler::setListener(ScriptCompilerListener *listener)
	{
		mListener = listener;
	}

	const String &ScriptCompiler::getResourceGroup() const
	{
		return mGroup;
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

				nodes->erase(cur);
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

			// Overlay the environment of one on top the other first
			for(std::map<String,String>::const_iterator i = src->getVariables().begin(); i != src->getVariables().end(); ++i)
			{
				std::pair<bool,String> var = dest->getVariable(i->first);
				if(!var.first)
					dest->setVariable(i->first, i->second);
			}
			
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
				{
					processVariables(&obj->children);
					processVariables(&obj->values);
				}
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

	void ScriptCompiler::initWordMap()
	{
		mIds["on"] = ID_ON;
		mIds["off"] = ID_OFF;
		mIds["true"] = ID_TRUE;
		mIds["false"] = ID_FALSE;
		mIds["yes"] = ID_YES;
		mIds["no"] = ID_NO;

		// Material ids
		mIds["material"] = ID_MATERIAL;
		mIds["vertex_program"] = ID_VERTEX_PROGRAM;
		mIds["fragment_program"] = ID_FRAGMENT_PROGRAM;
		mIds["technique"] = ID_TECHNIQUE;
		mIds["pass"] = ID_PASS;
		mIds["texture_unit"] = ID_TEXTURE_UNIT;
		mIds["vertex_program_ref"] = ID_VERTEX_PROGRAM_REF;
		mIds["fragment_program_ref"] = ID_FRAGMENT_PROGRAM_REF;
		mIds["shadow_caster_vertex_program_ref"] = ID_SHADOW_CASTER_VERTEX_PROGRAM_REF;
		mIds["shadow_receiver_vertex_program_ref"] = ID_SHADOW_RECEIVER_VERTEX_PROGRAM_REF;
		mIds["shadow_receiver_fragment_program_ref"] = ID_SHADOW_RECEIVER_FRAGMENT_PROGRAM_REF;

		mIds["lod_distances"] = ID_LOD_DISTANCES;
		mIds["receive_shadows"] = ID_RECEIVE_SHADOWS;
		mIds["transparency_casts_shadows"] = ID_TRANSPARENCY_CASTS_SHADOWS;
		mIds["set_texture_alias"] = ID_SET_TEXTURE_ALIAS;

		mIds["source"] = ID_SOURCE;
		mIds["syntax"] = ID_SYNTAX;
		mIds["default_params"] = ID_DEFAULT_PARAMS;
		mIds["param_indexed"] = ID_PARAM_INDEXED;
		mIds["param_named"] = ID_PARAM_NAMED;
		mIds["param_indexed_auto"] = ID_PARAM_INDEXED_AUTO;
		mIds["param_named_auto"] = ID_PARAM_NAMED_AUTO;

		mIds["scheme"] = ID_SCHEME;
		mIds["lod_index"] = ID_LOD_INDEX;
		mIds["shadow_caster_material_name"] = ID_SHADOW_CASTER_MATERIAL_NAME;
		mIds["shadow_receiver_material_name"] = ID_SHADOW_RECEIVER_MATERIAL_NAME;

		mIds["ambient"] = ID_AMBIENT;
		mIds["diffuse"] = ID_DIFFUSE;
		mIds["specular"] = ID_SPECULAR;
		mIds["emissive"] = ID_EMISSIVE;
			mIds["vertex_colour"] = ID_VERTEX_COLOUR;
		mIds["scene_blend"] = ID_SCENE_BLEND;
		mIds["colour_blend"] = ID_COLOUR_BLEND;
			mIds["one"] = ID_ONE;
			mIds["zero"] = ID_ZERO;
			mIds["dest_colour"] = ID_DEST_COLOUR;
			mIds["src_colour"] = ID_SRC_COLOUR;
			mIds["one_minus_src_colour"] = ID_ONE_MINUS_SRC_COLOUR;
			mIds["one_minus_dest_colour"] = ID_ONE_MINUS_DEST_COLOUR;
			mIds["dest_alpha"] = ID_DEST_ALPHA;
			mIds["src_alpha"] = ID_SRC_ALPHA;
			mIds["one_minus_dest_alpha"] = ID_ONE_MINUS_DEST_ALPHA;
			mIds["one_minus_src_alpha"] = ID_ONE_MINUS_SRC_ALPHA;
		mIds["separate_scene_blend"] = ID_SEPARATE_SCENE_BLEND;
		mIds["depth_check"] = ID_DEPTH_CHECK;
		mIds["depth_write"] = ID_DEPTH_WRITE;
		mIds["depth_func"] = ID_DEPTH_FUNC;
		mIds["depth_bias"] = ID_DEPTH_BIAS;
		mIds["iteration_depth_bias"] = ID_ITERATION_DEPTH_BIAS;
			mIds["always_fail"] = ID_ALWAYS_FAIL;
			mIds["always_pass"] = ID_ALWAYS_PASS;
			mIds["less_equal"] = ID_LESS_EQUAL;
			mIds["less"] = ID_LESS;
			mIds["equal"] = ID_EQUAL;
			mIds["not_equal"] = ID_NOT_EQUAL;
			mIds["greater_equal"] = ID_GREATER_EQUAL;
			mIds["greater"] = ID_GREATER;
		mIds["alpha_rejection"] = ID_ALPHA_REJECTION;
		mIds["light_scissor"] = ID_LIGHT_SCISSOR;
		mIds["light_clip_planes"] = ID_LIGHT_CLIP_PLANES;
		mIds["transparent_sorting"] = ID_TRANSPARENT_SORTING;
		mIds["illumination_stage"] = ID_ILLUMINATION_STAGE;
			mIds["decal"] = ID_DECAL;
		mIds["cull_hardware"] = ID_CULL_HARDWARE;
			mIds["clockwise"] = ID_CLOCKWISE;
			mIds["anticlockwise"] = ID_ANTICLOCKWISE;
		mIds["cull_software"] = ID_CULL_SOFTWARE;
			mIds["back"] = ID_BACK;
			mIds["front"] = ID_FRONT;
		mIds["normalise_normals"] = ID_NORMALISE_NORMALS;
		mIds["lighting"] = ID_LIGHTING;
		mIds["shading"] = ID_SHADING;
			mIds["flat"] = ID_FLAT;
			mIds["gouraud"] = ID_GOURAUD;
			mIds["phong"] = ID_PHONG;
		mIds["polygon_mode"] = ID_POLYGON_MODE;
		mIds["polygon_mode_overrideable"] = ID_POLYGON_MODE_OVERRIDEABLE;
		mIds["fog_override"] = ID_FOG_OVERRIDE;
			mIds["none"] = ID_NONE;
			mIds["linear"] = ID_LINEAR;
			mIds["exp"] = ID_EXP;
			mIds["exp2"] = ID_EXP2;
		mIds["colour_write"] = ID_COLOUR_WRITE;
		mIds["max_lights"] = ID_MAX_LIGHTS;
		mIds["start_light"] = ID_START_LIGHT;
		mIds["iteration"] = ID_ITERATION;
			mIds["once"] = ID_ONCE;
			mIds["once_per_light"] = ID_ONCE_PER_LIGHT;
			mIds["per_n_lights"] = ID_PER_N_LIGHTS;
			mIds["per_light"] = ID_PER_LIGHT;
			mIds["point"] = ID_POINT;
			mIds["spot"] = ID_SPOT;
			mIds["directional"] = ID_DIRECTIONAL;
		mIds["point_size"] = ID_POINT_SIZE;
		mIds["point_sprites"] = ID_POINT_SPRITES;
		mIds["point_size_min"] = ID_POINT_SIZE_MIN;
		mIds["point_size_max"] = ID_POINT_SIZE_MAX;

		mIds["texture_alias"] = ID_TEXTURE_ALIAS;
		mIds["texture"] = ID_TEXTURE;
			mIds["1d"] = ID_1D;
			mIds["2d"] = ID_2D;
			mIds["3d"] = ID_3D;
			mIds["cubic"] = ID_CUBIC;
			mIds["unlimited"] = ID_UNLIMITED;
			mIds["alpha"] = ID_ALPHA;
		mIds["anim_texture"] = ID_ANIM_TEXTURE;
		mIds["cubic_texture"] = ID_CUBIC_TEXTURE;
			mIds["separateUV"] = ID_SEPARATE_UV;
			mIds["combinedUVW"] = ID_COMBINED_UVW;
		mIds["tex_coord_set"] = ID_TEX_COORD_SET;
		mIds["tex_address_mode"] = ID_TEX_ADDRESS_MODE;
			mIds["wrap"] = ID_WRAP;
			mIds["clamp"] = ID_CLAMP;
			mIds["mirror"] = ID_MIRROR;
			mIds["border"] = ID_BORDER;
		mIds["filtering"] = ID_FILTERING;
			mIds["bilinear"] = ID_BILINEAR;
			mIds["trilinear"] = ID_TRILINEAR;
			mIds["anisotropic"] = ID_ANISOTROPIC;
		mIds["max_anisotropy"] = ID_MAX_ANISOTROPY;
		mIds["mipmap_bias"] = ID_MIPMAP_BIAS;
		mIds["colour_op"] = ID_COLOUR_OP;
			mIds["replace"] = ID_REPLACE;
			mIds["add"] = ID_ADD;
			mIds["modulate"] = ID_MODULATE;
			mIds["alpha_blend"] = ID_ALPHA_BLEND;
		mIds["colour_op_ex"] = ID_COLOUR_OP_EX;
			mIds["source1"] = ID_SOURCE1;
			mIds["source2"] = ID_SOURCE2;
			mIds["modulate"] = ID_MODULATE;
			mIds["modulate_x2"] = ID_MODULATE_X2;
			mIds["modulate_x4"] = ID_MODULATE_X4;
			mIds["add_signed"] = ID_ADD_SIGNED;
			mIds["add_smooth"] = ID_ADD_SMOOTH;
			mIds["blend_diffuse_alpha"] = ID_BLEND_DIFFUSE_ALPHA;
			mIds["blend_texture_alpha"] = ID_BLEND_TEXTURE_ALPHA;
			mIds["blend_current_alpha"] = ID_BLEND_CURRENT_ALPHA;
			mIds["blend_manual"] = ID_BLEND_MANUAL;
			mIds["dotproduct"] = ID_DOT_PRODUCT;
			mIds["blend_diffuse_colour"] = ID_BLEND_DIFFUSE_COLOUR;
			mIds["src_current"] = ID_SRC_CURRENT;
			mIds["src_texture"] = ID_SRC_TEXTURE;
			mIds["src_diffuse"] = ID_SRC_DIFFUSE;
			mIds["src_specular"] = ID_SRC_SPECULAR;
			mIds["src_manual"] = ID_SRC_MANUAL;
		mIds["colour_op_multipass_fallback"] = ID_COLOUR_OP_MULTIPASS_FALLBACK;
		mIds["alpha_op_ex"] = ID_ALPHA_OP_EX;
		mIds["env_map"] = ID_ENV_MAP;
			mIds["spherical"] = ID_SPHERICAL;
			mIds["planar"] = ID_PLANAR;
			mIds["cubic_reflection"] = ID_CUBIC_REFLECTION;
			mIds["cubic_normal"] = ID_CUBIC_NORMAL;
		mIds["scroll"] = ID_SCROLL;
		mIds["scroll_anim"] = ID_SCROLL_ANIM;
		mIds["rotate"] = ID_ROTATE;
		mIds["rotate_anim"] = ID_ROTATE_ANIM;
		mIds["scale"] = ID_SCALE;
		mIds["wave_xform"] = ID_WAVE_XFORM;
			mIds["scroll_x"] = ID_SCROLL_X;
			mIds["scroll_y"] = ID_SCROLL_Y;
			mIds["scale_x"] = ID_SCALE_X;
			mIds["scale_y"] = ID_SCALE_Y;
			mIds["sine"] = ID_SINE;
			mIds["triangle"] = ID_TRIANGLE;
			mIds["sawtooth"] = ID_SAWTOOTH;
			mIds["square"] = ID_SQUARE;
			mIds["inverse_sawtooth"] = ID_INVERSE_SAWTOOTH;
		mIds["transform"] = ID_TRANSFORM;
		mIds["binding_type"] = ID_BINDING_TYPE;
			mIds["vertex"] = ID_VERTEX;
			mIds["fragment"] = ID_FRAGMENT;
		mIds["content_type"] = ID_CONTENT_TYPE;
			mIds["named"] = ID_NAMED;
			mIds["shadow"] = ID_SHADOW;

		// Particle system
		mIds["particle_system"] = ID_PARTICLE_SYSTEM;
		mIds["emitter"] = ID_EMITTER;
		mIds["affector"] = ID_AFFECTOR;

		// Compositor
		mIds["compositor"] = ID_COMPOSITOR;
		mIds["target"] = ID_TARGET;
		mIds["target_output"] = ID_TARGET_OUTPUT;

		mIds["input"] = ID_INPUT;
			mIds["none"] = ID_NONE;
			mIds["previous"] = ID_PREVIOUS;
			mIds["target_width"] = ID_TARGET_WIDTH;
			mIds["target_height"] = ID_TARGET_HEIGHT;
			mIds["target_width_scaled"] = ID_TARGET_WIDTH_SCALED;
			mIds["target_height_scaled"] = ID_TARGET_HEIGHT_SCALED;
		mIds["only_initial"] = ID_ONLY_INITIAL;
		mIds["visibility_mask"] = ID_VISIBILITY_MASK;
		mIds["lod_bias"] = ID_LOD_BIAS;
		mIds["material_scheme"] = ID_MATERIAL_SCHEME;

		mIds["clear"] = ID_CLEAR;
		mIds["stencil"] = ID_STENCIL;
		mIds["render_scene"] = ID_RENDER_SCENE;
		mIds["render_quad"] = ID_RENDER_QUAD;
		mIds["identifier"] = ID_IDENTIFIER;
		mIds["first_render_queue"] = ID_FIRST_RENDER_QUEUE;
		mIds["last_render_queue"] = ID_LAST_RENDER_QUEUE;

		mIds["buffers"] = ID_BUFFERS;
			mIds["colour"] = ID_COLOUR;
			mIds["depth"] = ID_DEPTH;
		mIds["colour_value"] = ID_COLOUR_VALUE;
		mIds["depth_value"] = ID_DEPTH_VALUE;
		mIds["stencil_value"] = ID_STENCIL_VALUE;

		mIds["check"] = ID_CHECK;
		mIds["comp_func"] = ID_COMP_FUNC;
		mIds["ref_value"] = ID_REF_VALUE;
		mIds["mask"] = ID_MASK;
		mIds["fail_op"] = ID_FAIL_OP;
			mIds["keep"] = ID_KEEP;
			mIds["increment"] = ID_INCREMENT;
			mIds["decrement"] = ID_DECREMENT;
			mIds["increment_wrap"] = ID_INCREMENT_WRAP;
			mIds["decrement_wrap"] = ID_DECREMENT_WRAP;
			mIds["invert"] = ID_INVERT;
		mIds["depth_fail_op"] = ID_DEPTH_FAIL_OP;
		mIds["pass_op"] = ID_PASS_OP;
		mIds["two_sided"] = ID_TWO_SIDED;
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
			ConcreteNodeList::reverse_iterator riter = node->children.rbegin();
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

				// Everything up until the colon is a "value" of this object
				while(iter != temp.end() && (*iter)->type != CNT_COLON && (*iter)->type != CNT_LBRACE)
				{
					if((*iter)->type == CNT_VARIABLE)
					{
						VariableAccessAbstractNode *var = new VariableAccessAbstractNode(impl);
						var->file = (*iter)->file;
						var->line = (*iter)->line;
						var->type = ANT_VARIABLE_ACCESS;
						var->name = (*iter)->token;
						impl->values.push_back(AbstractNodePtr(var));
					}
					else
					{
						AtomAbstractNode *atom = new AtomAbstractNode(impl);
						atom->file = (*iter)->file;
						atom->line = (*iter)->line;
						atom->type = ANT_ATOM;
						atom->value = (*iter)->token;
						impl->values.push_back(AbstractNodePtr(atom));
					}
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

	// Translator
	ScriptCompiler::Translator::Translator()
		:mCompiler(0)
	{
	}

	void ScriptCompiler::Translator::translate(Translator *translator, const AbstractNodePtr &node, ScriptCompiler *compiler)
	{
		// If it an abstract object it is completely skipped
		if(node->type == ANT_OBJECT && ((ObjectAbstractNode*)node.get())->abstract)
			return;

		// First check if the compiler listener will override this node
		bool process = true;
		if(compiler && compiler->mListener)
		{
			std::pair<bool,ScriptCompiler::Translator*> p;
			if(node->type == ANT_OBJECT)
				p = compiler->mListener->preObjectTranslation((ObjectAbstractNode*)node.get());
			else if(node->type == ANT_PROPERTY)
				p = compiler->mListener->prePropertyTranslation((PropertyAbstractNode*)node.get());
			else
				p.first = false;
			if(p.first && p.second)
			{
				p.second->mCompiler = compiler;
				// Call the returned translator
				if(node->type == ANT_OBJECT)
					p.second->processObject((ObjectAbstractNode*)node.get());
				else if(node->type == ANT_PROPERTY)
					p.second->processProperty((PropertyAbstractNode*)node.get());
				process = false;
			}
		}

		// Call the suggested translator
		// Or ignore the node if no translator is given
		if(process && translator)
		{
			translator->mCompiler = compiler;
			if(node->type == ANT_OBJECT)
				translator->processObject((ObjectAbstractNode*)node.get());
			else if(node->type == ANT_PROPERTY)
				translator->processProperty((PropertyAbstractNode*)node.get());
		}
	}

	ScriptCompiler *ScriptCompiler::Translator::getCompiler()
	{
		return mCompiler;
	}

	ScriptCompilerListener *ScriptCompiler::Translator::getCompilerListener()
	{
		return mCompiler->mListener;
	}

	AbstractNodeList::const_iterator ScriptCompiler::Translator::getNodeAt(const AbstractNodeList &nodes, int index)
	{
		AbstractNodeList::const_iterator i = nodes.begin();
		int n = 0;
		while(i != nodes.end())
		{
			if(n == index)
				return i;
			++i;
			++n;
		}
		return nodes.end();
	}

	bool ScriptCompiler::Translator::getBoolean(const AbstractNodePtr &node, bool *result)
	{
		if(node->type != ANT_ATOM)
		{
			mCompiler->addError(CE_INVALIDPARAMETERS, node->file, node->line);
			return false;
		}
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		if(atom->id != 1 && atom->id != 2)
		{
			mCompiler->addError(CE_INVALIDPARAMETERS, node->file, node->line);
			return false;
		}
		
		*result = atom->id == 1 ? true : false;
		return true;
	}

	bool ScriptCompiler::Translator::getString(const AbstractNodePtr &node, String *result)
	{
		if(node->type != ANT_ATOM)
		{
			mCompiler->addError(CE_INVALIDPARAMETERS, node->file, node->line);
			return false;
		}
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();		
		*result = atom->value;
		return true;
	}

	bool ScriptCompiler::Translator::getNumber(const Ogre::AbstractNodePtr &node, Ogre::Real *result)
	{
		if(node->type != ANT_ATOM)
		{
			mCompiler->addError(CE_INVALIDPARAMETERS, node->file, node->line);
			return false;
		}
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		if(!atom->isNumber())
		{
			mCompiler->addError(CE_INVALIDPARAMETERS, node->file, node->line);
			return false;
		}
		*result = atom->getNumber();
		return true;
	}

	bool ScriptCompiler::Translator::getColour(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, ColourValue *result)
	{
		Real vals[4];
		memset(vals, 0, sizeof(Real)*4);

		int n = 0;
		while(i != end && n < 4)
		{
			if((*i)->type == ANT_ATOM && ((AtomAbstractNode*)(*i).get())->isNumber())
			{
				vals[n] = ((AtomAbstractNode*)(*i).get())->getNumber();
			}
			else
			{
				return false;
			}
			++n;
			++i;
		}

		result->r = vals[0];
		result->g = vals[1];
		result->b = vals[2];
		result->a = vals[3];
		return true;
	}

	bool ScriptCompiler::Translator::getSceneBlendFactor(const Ogre::AbstractNodePtr &node, Ogre::SceneBlendFactor *sbf)
	{
		if(node->type != ANT_ATOM)
		{
			mCompiler->addError(CE_INVALIDPARAMETERS, node->file, node->line);
			return false;
		}
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		switch(atom->id)
		{
		case ID_ONE:
			*sbf = SBF_ONE;
			break;
		case ID_ZERO:
			*sbf = SBF_ZERO;
			break;
		case ID_DEST_COLOUR:
			*sbf = SBF_DEST_COLOUR;
			break;
		case ID_DEST_ALPHA:
			*sbf = SBF_DEST_ALPHA;
			break;
		case ID_SRC_ALPHA:
			*sbf = SBF_SOURCE_ALPHA;
			break;
		case ID_SRC_COLOUR:
			*sbf = SBF_SOURCE_COLOUR;
			break;
		case ID_ONE_MINUS_DEST_COLOUR:
			*sbf = SBF_ONE_MINUS_DEST_COLOUR;
			break;
		case ID_ONE_MINUS_SRC_COLOUR:
			*sbf = SBF_ONE_MINUS_SOURCE_COLOUR;
			break;
		case ID_ONE_MINUS_DEST_ALPHA:
			*sbf = SBF_ONE_MINUS_DEST_ALPHA;
			break;
		case ID_ONE_MINUS_SRC_ALPHA:
			*sbf = SBF_ONE_MINUS_SOURCE_ALPHA;
			break;
		default:
			return false;
		}
		return true;
	}

	bool ScriptCompiler::Translator::getCompareFunction(const AbstractNodePtr &node, CompareFunction *func)
	{
		if(node->type != ANT_ATOM)
		{
			mCompiler->addError(CE_INVALIDPARAMETERS, node->file, node->line);
			return false;
		}
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		switch(atom->id)
		{
		case ID_ALWAYS_FAIL:
			*func = CMPF_ALWAYS_FAIL;
			break;
		case ID_ALWAYS_PASS:
			*func = CMPF_ALWAYS_PASS;
			break;
		case ID_LESS:
			*func = CMPF_LESS;
			break;
		case ID_LESS_EQUAL:
			*func = CMPF_LESS_EQUAL;
			break;
		case ID_EQUAL:
			*func = CMPF_EQUAL;
			break;
		case ID_NOT_EQUAL:
			*func = CMPF_NOT_EQUAL;
			break;
		case ID_GREATER_EQUAL:
			*func = CMPF_GREATER_EQUAL;
			break;
		case ID_GREATER:
			*func = CMPF_GREATER;
			break;
		default:
			return false;
		}
		return true;
	}

	bool ScriptCompiler::Translator::getMatrix4(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, Matrix4 *m)
	{
		int n = 0;
		while(i != end && n < 16)
		{
			if(i != end)
			{
				if((*i)->type == ANT_ATOM && ((AtomAbstractNode*)(*i).get())->isNumber())
					(*m)[n%4][n/4] = ((AtomAbstractNode*)(*i).get())->getNumber();
				else
					return false;
			}
			else
			{
				return false;
			}
		}
		return true;
	}

	bool ScriptCompiler::Translator::getInts(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, int *vals, int count)
	{
		bool success = true;
		int n = 0;
		while(n < count)
		{
			if(i != end)
			{
				if((*i)->type == ANT_ATOM && ((AtomAbstractNode*)(*i).get())->isNumber())
					vals[n] = ((AtomAbstractNode*)(*i).get())->getNumber();
				else
					break;
				++i;
			}
			else
				vals[n] = 0;
			++n;
		}

		if(n < count)
			success = false;

		return success;
	}

	bool ScriptCompiler::Translator::getFloats(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, float *vals, int count)
	{
		bool success = true;
		int n = 0;
		while(n < count)
		{
			if(i != end)
			{
				if((*i)->type == ANT_ATOM && ((AtomAbstractNode*)(*i).get())->isNumber())
					vals[n] = ((AtomAbstractNode*)(*i).get())->getNumber();
				else
					break;
				++i;
			}
			else
				vals[n] = 0;
			++n;
		}

		if(n < count)
			success = false;

		return success;
	}

	bool ScriptCompiler::Translator::getStencilOp(const Ogre::AbstractNodePtr &node, Ogre::StencilOperation *op)
	{
		if(node->type != ANT_ATOM)
		{
			mCompiler->addError(CE_INVALIDPARAMETERS, node->file, node->line);
			return false;
		}
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		switch(atom->id)
		{
		case ID_KEEP:
			*op = SOP_KEEP;
			break;
		case ID_ZERO:
			*op = SOP_ZERO;
			break;
		case ID_REPLACE:
			*op = SOP_REPLACE;
			break;
		case ID_INCREMENT:
			*op = SOP_INCREMENT;
			break;
		case ID_DECREMENT:
			*op = SOP_DECREMENT;
			break;
		case ID_INCREMENT_WRAP:
			*op = SOP_INCREMENT_WRAP;
			break;
		case ID_DECREMENT_WRAP:
			*op = SOP_DECREMENT_WRAP;
			break;
		case ID_INVERT:
			*op = SOP_INVERT;
			break;
		default:
			return false;
		}
		return true;
	}

	// MaterialTranslator
	ScriptCompiler::MaterialTranslator::MaterialTranslator()
	{
	}

	void ScriptCompiler::MaterialTranslator::processObject(ObjectAbstractNode *obj)
	{
		if(obj->name.empty())
			getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);

		// Create a material with the given name
		if(getCompilerListener())
			mMaterial = getCompilerListener()->createMaterial(obj->name, getCompiler()->getResourceGroup());
		else
			mMaterial = MaterialManager::getSingleton().create(obj->name, getCompiler()->getResourceGroup());

		if(mMaterial.isNull())
		{
			getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
			return;
		}

		mMaterial->removeAllTechniques();
		obj->context = Any(mMaterial.get());

		// Set the properties for the material
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				Translator::translate(this, *i, getCompiler());
			}
			else if((*i)->type == ANT_OBJECT)
			{

				ObjectAbstractNode *obj = (ObjectAbstractNode*)(*i).get();
				if(obj->id == ID_TECHNIQUE)
				{
					// Compile the technique
					Technique *tec = mMaterial->createTechnique();
					TechniqueTranslator translator(tec);
					Translator::translate(&translator, *i, getCompiler());
				}
			}
		}

		// Apply the texture aliases
		if(getCompilerListener())
			getCompilerListener()->preApplyTextureAliases(&mTextureAliases);
		mMaterial->applyTextureAliases(mTextureAliases);
	}

	void ScriptCompiler::MaterialTranslator::processProperty(Ogre::PropertyAbstractNode *prop)
	{
		switch(prop->id)
		{
		case ID_LOD_DISTANCES:
			{
				Material::LodDistanceList lods;
				for(AbstractNodeList::iterator j = prop->values.begin(); j != prop->values.end(); ++j)
				{
					if((*j)->type == ANT_ATOM && ((AtomAbstractNode*)(*j).get())->isNumber())
						lods.push_back(((AtomAbstractNode*)(*j).get())->getNumber());
					else
						getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
				}
				mMaterial->setLodLevels(lods);
			}
			break;
		case ID_RECEIVE_SHADOWS:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				bool val = true;
				if(getBoolean(prop->values.front(), &val))
					mMaterial->setReceiveShadows(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_TRANSPARENCY_CASTS_SHADOWS:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				bool val = true;
				if(getBoolean(prop->values.front(), &val))
					mMaterial->setTransparencyCastsShadows(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_SET_TEXTURE_ALIAS:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 3)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
				String name, value;
				if(getString(*i0, &name) && getString(*i1, &value))
					mTextureAliases.insert(std::make_pair(name, value));
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		}
	}

	// TechniqueTranslator
	ScriptCompiler::TechniqueTranslator::TechniqueTranslator(Technique *technique)
		:mTechnique(technique)
	{
	}

	void ScriptCompiler::TechniqueTranslator::processObject(ObjectAbstractNode *obj)
	{
		obj->context = Any(mTechnique);

		// Get the name of the technique
		if(!obj->name.empty())
			mTechnique->setName(obj->name);

		// Set the properties for the material
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				Translator::translate(this, *i, getCompiler());
			}
			else if((*i)->type == ANT_OBJECT)
			{
				ObjectAbstractNode *obj = (ObjectAbstractNode*)(*i).get();
				if(obj->id == ID_PASS)
				{
					// Create a pass and compile it
					Pass *pass = mTechnique->createPass();
					PassTranslator translator(pass);
					Translator::translate(&translator, *i, getCompiler());
				}
			}
		}
	}

	void ScriptCompiler::TechniqueTranslator::processProperty(PropertyAbstractNode *prop)
	{
		switch(prop->id)
		{
		case ID_SCHEME:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 3)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
				String scheme;
				if(getString(*i0, &scheme))
					mTechnique->setSchemeName(scheme);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_LOD_INDEX:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 3)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
				Real val;
				if(getNumber(*i0, &val))
					mTechnique->setLodIndex(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_SHADOW_CASTER_MATERIAL_NAME:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 3)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
				String matName;
				if(getString(*i0, &matName))
					mTechnique->setShadowCasterMaterial(matName);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_SHADOW_RECEIVER_MATERIAL_NAME:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 3)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
				String matName;
				if(getString(*i0, &matName))
					mTechnique->setShadowReceiverMaterial(matName);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		}
	}

	// PassTranslator
	ScriptCompiler::PassTranslator::PassTranslator(Ogre::Pass *pass)
		:mPass(pass)
	{
	}

	void ScriptCompiler::PassTranslator::processObject(ObjectAbstractNode *obj)
	{
		obj->context = Any(mPass);

		// Get the name of the technique
		if(!obj->name.empty())
			mPass->setName(obj->name);

		// Set the properties for the material
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				Translator::translate(this, *i, getCompiler());
			}
			else if((*i)->type == ANT_OBJECT)
			{
				ObjectAbstractNode *obj = (ObjectAbstractNode*)(*i).get();
				switch(obj->id)
				{
				case ID_TEXTURE_UNIT:
					{
						TextureUnitState *unit = mPass->createTextureUnitState();
						TextureUnitTranslator translator(unit);
						translate(&translator, *i, getCompiler());
					}
					break;
				case ID_VERTEX_PROGRAM_REF:
					{
						// This object needs to be named
						if(!obj->name.empty())
						{
							String name = obj->name;
							if(getCompilerListener())
								getCompilerListener()->getGpuProgramName(&name);

							GpuProgram *prog = 0;
							if(mPass->hasVertexProgram() && (mPass->getVertexProgramName() == name || name.empty()))
							{
								prog = mPass->getVertexProgram().get();
							}
							else
							{
								prog = (GpuProgram*)GpuProgramManager::getSingleton().getByName(name).get();
								if(prog)
									mPass->setVertexProgram(name);
							}

							if(prog)
							{
								if(prog->isSupported())
								{
									GpuProgramParametersSharedPtr params = mPass->getVertexProgramParameters();
									GpuProgramParametersTranslator translator(params);
									translate(&translator, *i, getCompiler());
								}
							}
							else
							{
								getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
							}
						}
						else
						{
							getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
						}
					}
					break;
				case ID_FRAGMENT_PROGRAM_REF:
					{
						// This object needs to be named
						if(!obj->name.empty())
						{
							String name = obj->name;
							if(getCompilerListener())
								getCompilerListener()->getGpuProgramName(&name);

							GpuProgram *prog = 0;
							if(mPass->hasFragmentProgram() && (mPass->getFragmentProgramName() == name || name.empty()))
							{
								prog = mPass->getFragmentProgram().get();
							}
							else
							{
								prog = (GpuProgram*)GpuProgramManager::getSingleton().getByName(name).get();
								if(prog)
									mPass->setFragmentProgram(name);
							}

							if(prog)
							{
								if(prog->isSupported())
								{
									GpuProgramParametersSharedPtr params = mPass->getFragmentProgramParameters();
									GpuProgramParametersTranslator translator(params);
									translate(&translator, *i, getCompiler());
								}
							}
							else
							{
								getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
							}
						}
						else
						{
							getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
						}
					}
					break;
				case ID_SHADOW_CASTER_VERTEX_PROGRAM_REF:
					{
						// This object needs to be named
						if(!obj->name.empty())
						{
							String name = obj->name;
							if(getCompilerListener())
								getCompilerListener()->getGpuProgramName(&name);

							GpuProgram *prog = 0;
							if(mPass->hasShadowCasterVertexProgram() && (mPass->getShadowCasterVertexProgramName() == name || name.empty()))
							{
								prog = mPass->getShadowCasterVertexProgram().get();
							}
							else
							{
								prog = (GpuProgram*)GpuProgramManager::getSingleton().getByName(name).get();
								if(prog)
									mPass->setShadowCasterVertexProgram(name);
							}

							if(prog)
							{
								if(prog->isSupported())
								{
									GpuProgramParametersSharedPtr params = mPass->getShadowCasterVertexProgramParameters();
									GpuProgramParametersTranslator translator(params);
									translate(&translator, *i, getCompiler());
								}
							}
							else
							{
								getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
							}
						}
						else
						{
							getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
						}
					}
					break;
				case ID_SHADOW_RECEIVER_VERTEX_PROGRAM_REF:
					{
						// This object needs to be named
						if(!obj->name.empty())
						{
							String name = obj->name;
							if(getCompilerListener())
								getCompilerListener()->getGpuProgramName(&name);

							GpuProgram *prog = 0;
							if(mPass->hasShadowReceiverVertexProgram() && (mPass->getShadowCasterVertexProgramName() == name || name.empty()))
							{
								prog = mPass->getShadowCasterVertexProgram().get();
							}
							else
							{
								prog = (GpuProgram*)GpuProgramManager::getSingleton().getByName(name).get();
								if(prog)
									mPass->setShadowReceiverVertexProgram(name);
							}

							if(prog)
							{
								if(prog->isSupported())
								{
									GpuProgramParametersSharedPtr params = mPass->getShadowReceiverVertexProgramParameters();
									GpuProgramParametersTranslator translator(params);
									translate(&translator, *i, getCompiler());
								}
							}
							else
							{
								getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
							}
						}
						else
						{
							getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
						}
					}
					break;
				case ID_SHADOW_RECEIVER_FRAGMENT_PROGRAM_REF:
					{
						// This object needs to be named
						if(!obj->name.empty())
						{
							String name = obj->name;
							if(getCompilerListener())
								getCompilerListener()->getGpuProgramName(&name);

							GpuProgram *prog = 0;
							if(mPass->hasShadowReceiverFragmentProgram() && (mPass->getShadowReceiverFragmentProgramName() == name || name.empty()))
							{
								prog = mPass->getShadowReceiverFragmentProgram().get();
							}
							else
							{
								prog = (GpuProgram*)GpuProgramManager::getSingleton().getByName(name).get();
								if(prog)
									mPass->setShadowReceiverFragmentProgram(name);
							}

							if(prog)
							{
								if(prog->isSupported())
								{
									GpuProgramParametersSharedPtr params = mPass->getShadowReceiverFragmentProgramParameters();
									GpuProgramParametersTranslator translator(params);
									translate(&translator, *i, getCompiler());
								}
							}
							else
							{
								getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
							}
						}
						else
						{
							getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
						}
					}
					break;
				default:
					Translator::translate(0, *i, getCompiler());
				}
			}
		}
	}

	void ScriptCompiler::PassTranslator::processProperty(PropertyAbstractNode *prop)
	{
		switch(prop->id)
		{
		case ID_AMBIENT:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 4)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				ColourValue val;
				if(getColour(prop->values.begin(), prop->values.end(), &val))
					mPass->setAmbient(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_DIFFUSE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 4)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				ColourValue val;
				if(getColour(prop->values.begin(), prop->values.end(), &val))
					mPass->setDiffuse(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_SPECULAR:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 5)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				ColourValue val;
				if(getColour(prop->values.begin(), prop->values.end(), &val))
					mPass->setSpecular(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);

				if(prop->values.size() == 5)
				{
					Real val = 0;
					if(getNumber(prop->values.back(), &val))
						mPass->setShininess(val);
					else
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_EMISSIVE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 4)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				ColourValue val;
				if(getColour(prop->values.begin(), prop->values.end(), &val))
					mPass->setSelfIllumination(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_SCENE_BLEND:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 2)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() == 1)
			{
				if(prop->values.front()->type == ANT_ATOM)
				{
					AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
					switch(atom->id)
					{
					case ID_ADD:
						mPass->setSceneBlending(SBT_ADD);
						break;
					case ID_MODULATE:
						mPass->setSceneBlending(SBT_MODULATE);
						break;
					case ID_COLOUR_BLEND:
						mPass->setSceneBlending(SBT_TRANSPARENT_COLOUR);
						break;
					case ID_ALPHA_BLEND:
						mPass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
				SceneBlendFactor sbf0, sbf1;
				if(getSceneBlendFactor(*i0, &sbf0) && getSceneBlendFactor(*i1, &sbf1))
				{
					mPass->setSceneBlending(sbf0, sbf1);
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}				
			}
			break;
		case ID_SEPARATE_SCENE_BLEND:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() == 3)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 4)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() == 2)
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
				if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM)
				{
					AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
					SceneBlendType sbt0, sbt1;
					switch(atom0->id)
					{
					case ID_ADD:
						sbt0 = SBT_ADD;
						break;
					case ID_MODULATE:
						sbt0 = SBT_MODULATE;
						break;
					case ID_COLOUR_BLEND:
						sbt0 = SBT_TRANSPARENT_COLOUR;
						break;
					case ID_ALPHA_BLEND:
						sbt0 = SBT_TRANSPARENT_ALPHA;
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						return;
					}
					
					switch(atom1->id)
					{
					case ID_ADD:
						sbt1 = SBT_ADD;
						break;
					case ID_MODULATE:
						sbt1 = SBT_MODULATE;
						break;
					case ID_COLOUR_BLEND:
						sbt1 = SBT_TRANSPARENT_COLOUR;
						break;
					case ID_ALPHA_BLEND:
						sbt1 = SBT_TRANSPARENT_ALPHA;
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						return;
					}

					mPass->setSeparateSceneBlending(sbt0, sbt1);
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1),
					i2 = getNodeAt(prop->values, 2), i3 = getNodeAt(prop->values, 3);
				if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM && (*i3)->type == ANT_ATOM)
				{
					SceneBlendFactor sbf0, sbf1, sbf2, sbf3;
					if(getSceneBlendFactor(*i0, &sbf0) && getSceneBlendFactor(*i1, &sbf1) && getSceneBlendFactor(*i2, &sbf2) && 
						getSceneBlendFactor(*i3, &sbf3))
					{
						mPass->setSeparateSceneBlending(sbf0, sbf1, sbf2, sbf3);
					}
					else
					{
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_DEPTH_CHECK:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				bool val = true;
				if(getBoolean(prop->values.front(), &val))
					mPass->setDepthCheckEnabled(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_DEPTH_WRITE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				bool val = true;
				if(getBoolean(prop->values.front(), &val))
					mPass->setDepthWriteEnabled(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_DEPTH_BIAS:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 2)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
				Real val0, val1 = 0.0f;
				if(getNumber(*i0, &val0))
				{
					if(i1 != prop->values.end())
						getNumber(*i1, &val1);
					mPass->setDepthBias(val0, val1);
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_DEPTH_FUNC:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				CompareFunction func;
				if(getCompareFunction(prop->values.front(), &func))
					mPass->setDepthFunction(func);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_ITERATION_DEPTH_BIAS:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				Real val = 0.0f;
				if(getNumber(prop->values.front(), &val))
					mPass->setIterationDepthBias(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_ALPHA_REJECTION:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 2)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
				CompareFunction func;
				if(getCompareFunction(*i0, &func))
				{
					if(i1 != prop->values.end())
					{
						Real val = 0.0f;
						if(getNumber(*i1, &val))
							mPass->setAlphaRejectSettings(func, val);
						else
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					else
						mPass->setAlphaRejectFunction(func);
				}
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_LIGHT_SCISSOR:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				bool val = false;
				if(getBoolean(prop->values.front(), &val))
					mPass->setLightScissoringEnabled(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_LIGHT_CLIP_PLANES:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				bool val = false;
				if(getBoolean(prop->values.front(), &val))
					mPass->setLightClipPlanesEnabled(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_TRANSPARENT_SORTING:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				bool val = true;
				if(getBoolean(prop->values.front(), &val))
					mPass->setTransparentSortingEnabled(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_ILLUMINATION_STAGE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				if(prop->values.front()->type == ANT_ATOM)
				{
					AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
					switch(atom->id)
					{
					case ID_AMBIENT:
						mPass->setIlluminationStage(IS_AMBIENT);
						break;
					case ID_PER_LIGHT:
						mPass->setIlluminationStage(IS_PER_LIGHT);
						break;
					case ID_DECAL:
						mPass->setIlluminationStage(IS_DECAL);
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_CULL_HARDWARE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				if(prop->values.front()->type == ANT_ATOM)
				{
					AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
					switch(atom->id)
					{
					case ID_CLOCKWISE:
						mPass->setCullingMode(CULL_CLOCKWISE);
						break;
					case ID_ANTICLOCKWISE:
						mPass->setCullingMode(CULL_ANTICLOCKWISE);
						break;
					case ID_NONE:
						mPass->setCullingMode(CULL_NONE);
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_CULL_SOFTWARE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				if(prop->values.front()->type == ANT_ATOM)
				{
					AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
					switch(atom->id)
					{
					case ID_FRONT:
						mPass->setManualCullingMode(MANUAL_CULL_FRONT);
						break;
					case ID_BACK:
						mPass->setManualCullingMode(MANUAL_CULL_BACK);
						break;
					case ID_NONE:
						mPass->setManualCullingMode(MANUAL_CULL_NONE);
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_NORMALISE_NORMALS:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				bool val = false;
				if(getBoolean(prop->values.front(), &val))
					mPass->setNormaliseNormals(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_LIGHTING:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				bool val = false;
				if(getBoolean(prop->values.front(), &val))
					mPass->setLightingEnabled(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_SHADING:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				if(prop->values.front()->type == ANT_ATOM)
				{
					AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
					switch(atom->id)
					{
					case ID_FLAT:
						mPass->setShadingMode(SO_FLAT);
						break;
					case ID_GOURAUD:
						mPass->setShadingMode(SO_GOURAUD);
						break;
					case ID_PHONG:
						mPass->setShadingMode(SO_PHONG);
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_POLYGON_MODE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				if(prop->values.front()->type == ANT_ATOM)
				{
					AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
					switch(atom->id)
					{
					case ID_SOLID:
						mPass->setPolygonMode(PM_SOLID);
						break;
					case ID_POINTS:
						mPass->setPolygonMode(PM_POINTS);
						break;
					case ID_WIREFRAME:
						mPass->setPolygonMode(PM_WIREFRAME);
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_POLYGON_MODE_OVERRIDEABLE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				bool val = false;
				if(getBoolean(prop->values.front(), &val))
					mPass->setPolygonModeOverrideable(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_FOG_OVERRIDE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 8)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1), i2 = getNodeAt(prop->values, 2);
				bool val = false;
				if(getBoolean(prop->values.front(), &val))
				{
					FogMode mode = FOG_NONE;
					ColourValue clr = ColourValue::White;
					Real dens = 0.001, start = 0.0f, end = 1.0f;

					if(i1 != prop->values.end())
					{
						if((*i1)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)(*i1).get();
							switch(atom->id)
							{
							case ID_NONE:
								mode = FOG_NONE;
								break;
							case ID_LINEAR:
								mode = FOG_LINEAR;
								break;
							case ID_EXP:
								mode = FOG_EXP;
								break;
							case ID_EXP2:
								mode = FOG_EXP2;
								break;
							default:
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
								return;
							}
						}
						else
						{
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
							return;
						}
					}

					if(i2 != prop->values.end())
					{
						if(!getColour(i2, prop->values.end(), &clr))
						{
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
							return;
						}
					}

					if(i2 != prop->values.end())
					{
						if(!getNumber(*i2, &dens))
						{
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
							return;
						}
						++i2;
					}

					if(i2 != prop->values.end())
					{
						if(!getNumber(*i2, &start))
						{
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
							return;
						}
						++i2;
					}

					if(i2 != prop->values.end())
					{
						if(!getNumber(*i2, &end))
						{
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
							return;
						}
						++i2;
					}

					mPass->setFog(val, mode, clr, dens, start, end);
				}
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_COLOUR_WRITE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				bool val = false;
				if(getBoolean(prop->values.front(), &val))
					mPass->setColourWriteEnabled(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_MAX_LIGHTS:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				Real val = 0.0f;
				if(getNumber(prop->values.front(), &val))
					mPass->setMaxSimultaneousLights(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_START_LIGHT:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				Real val = 0.0f;
				if(getNumber(prop->values.front(), &val))
					mPass->setStartLight(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_ITERATION:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0);
				if((*i0)->type == ANT_ATOM)
				{
					AtomAbstractNode *atom = (AtomAbstractNode*)(*i0).get();
					if(atom->id == ID_ONCE)
					{
						mPass->setIteratePerLight(false);
					}
					else if(atom->id == ID_ONCE_PER_LIGHT)
					{
						AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1);
						if(i1 != prop->values.end() && (*i1)->type == ANT_ATOM)
						{
							atom = (AtomAbstractNode*)(*i1).get();
							switch(atom->id)
							{
							case ID_POINT:
								mPass->setIteratePerLight(true);
								break;
							case ID_DIRECTIONAL:
								mPass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
								break;
							case ID_SPOT:
								mPass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
								break;
							default:
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
							}
						}
						else
						{
							mPass->setIteratePerLight(true, false);
						}

					}
					else if(atom->isNumber())
					{
						mPass->setPassIterationCount(atom->getNumber());

						AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1);
						if(i1 != prop->values.end() && (*i1)->type == ANT_ATOM)
						{
							atom = (AtomAbstractNode*)(*i1).get();
							if(atom->id == ID_PER_LIGHT)
							{
								AbstractNodeList::const_iterator i2 = getNodeAt(prop->values, 2);
								if(i2 != prop->values.end() && (*i2)->type == ANT_ATOM)
								{
									atom = (AtomAbstractNode*)(*i2).get();
									switch(atom->id)
									{
									case ID_POINT:
										mPass->setIteratePerLight(true);
										break;
									case ID_DIRECTIONAL:
										mPass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
										break;
									case ID_SPOT:
										mPass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
										break;
									default:
										getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
									}
								}
								else
								{
									mPass->setIteratePerLight(true, false);
								}
							}
							else if(ID_PER_N_LIGHTS)
							{
								AbstractNodeList::const_iterator i2 = getNodeAt(prop->values, 2);
								if(i2 != prop->values.end() && (*i2)->type == ANT_ATOM)
								{
									atom = (AtomAbstractNode*)(*i2).get();
									if(atom->isNumber())
									{
										mPass->setLightCountPerIteration(atom->getNumber());
										
										AbstractNodeList::const_iterator i3 = getNodeAt(prop->values, 3);
										if(i3 != prop->values.end() && (*i3)->type == ANT_ATOM)
										{
											atom = (AtomAbstractNode*)(*i2).get();
											switch(atom->id)
											{
											case ID_POINT:
												mPass->setIteratePerLight(true);
												break;
											case ID_DIRECTIONAL:
												mPass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
												break;
											case ID_SPOT:
												mPass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
												break;
											default:
												getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
											}
										}
										else
										{
											mPass->setIteratePerLight(true, false);
										}
									}
									else
									{
										getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
									}
								}
								else
								{
									getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
								}
							}
						}
					}
					else
					{
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_POINT_SIZE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				Real val = 0.0f;
				if(getNumber(prop->values.front(), &val))
					mPass->setPointSize(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_POINT_SPRITES:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				bool val = false;
				if(getBoolean(prop->values.front(), &val))
					mPass->setPointSpritesEnabled(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_POINT_SIZE_ATTENUATION:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 4)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				bool val = false;
				if(getBoolean(prop->values.front(), &val))
				{
					if(val)
					{
						AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1), i2 = getNodeAt(prop->values, 2), 
							i3 = getNodeAt(prop->values, 3);
						Real constant = 0.0f, linear = 1.0f, quadratic = 0.0f;

						if(i1 != prop->values.end() && (*i1)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)(*i1).get();
							if(atom->isNumber())
								constant = atom->getNumber();
							else
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
						else
						{
							getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
						}

						if(i2 != prop->values.end() && (*i2)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)(*i2).get();
							if(atom->isNumber())
								linear = atom->getNumber();
							else
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
						else
						{
							getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
						}

						if(i3 != prop->values.end() && (*i3)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)(*i3).get();
							if(atom->isNumber())
								quadratic = atom->getNumber();
							else
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
						else
						{
							getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
						}

						mPass->setPointAttenuation(true, constant, linear, quadratic);
					}
					else
					{
						mPass->setPointAttenuation(false);
					}
				}
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_POINT_SIZE_MIN:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				Real val = 0.0f;
				if(getNumber(prop->values.front(), &val))
					mPass->setPointMinSize(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_POINT_SIZE_MAX:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				Real val = 0.0f;
				if(getNumber(prop->values.front(), &val))
					mPass->setPointMaxSize(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		}
	}

	// TextureUnitTranslator
	ScriptCompiler::TextureUnitTranslator::TextureUnitTranslator(TextureUnitState *unit)
		:mUnit(unit)
	{
	}

	void ScriptCompiler::TextureUnitTranslator::processObject(ObjectAbstractNode *obj)
	{
		obj->context = Any(mUnit);

		// Get the name of the technique
		if(!obj->name.empty())
			mUnit->setName(obj->name);

		// Set the properties for the material
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				Translator::translate(this, *i, getCompiler());
				
			}
			else if((*i)->type == ANT_OBJECT)
			{
				Translator::translate(0, *i, getCompiler());
			}
		}
	}

	void ScriptCompiler::TextureUnitTranslator::processProperty(Ogre::PropertyAbstractNode *prop)
	{
		switch(prop->id)
		{
		case ID_TEXTURE_ALIAS:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				String val;
				if(getString(prop->values.front(), &val))
					mUnit->setTextureNameAlias(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_TEXTURE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 5)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator j = prop->values.begin();
				String val;
				if(getString(*j, &val))
				{
					TextureType texType = TEX_TYPE_2D;
					bool isAlpha = false;
					PixelFormat format = PF_UNKNOWN;
					int mipmaps = MIP_DEFAULT;

					++j;
					while(j != prop->values.end())
					{
						if((*j)->type == ANT_ATOM)
						{
							AtomAbstractNode *atom = (AtomAbstractNode*)(*j).get();
							switch(atom->id)
							{
							case ID_1D:
								texType = TEX_TYPE_1D;
								break;
							case ID_2D:
								texType = TEX_TYPE_2D;
								break;
							case ID_3D:
								texType = TEX_TYPE_3D;
								break;
							case ID_CUBIC:
								texType = TEX_TYPE_CUBE_MAP;
								break;
							case ID_UNLIMITED:
								mipmaps = MIP_UNLIMITED;
								break;
							case ID_ALPHA:
								isAlpha = true;
								break;
							default:
								if(atom->isNumber())
									mipmaps = atom->getNumber();
								else
									format = PixelUtil::getFormatFromName(atom->value, true);
							}
						}
						else
						{
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
						++j;
					}

					if(getCompilerListener())
						getCompilerListener()->getTextureNames(&val, 1);

					mUnit->setTextureName(val, texType);
					mUnit->setDesiredFormat(format);
					mUnit->setIsAlpha(isAlpha);
					mUnit->setNumMipmaps(mipmaps);
				}
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_ANIM_TEXTURE:
			if(prop->values.size() < 3)
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i1 = getNodeAt(prop->values, 1);
				if((*i1)->type == ANT_ATOM && ((AtomAbstractNode*)(*i1).get())->isNumber())
				{
					// Short form
					AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i2 = getNodeAt(prop->values, 2);
					if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM)
					{
						AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get(),
							*atom2 = (AtomAbstractNode*)(*i2).get();
						if(atom1->isNumber() && atom2->isNumber())
						{
							mUnit->setAnimatedTextureName(atom0->value, atom1->getNumber(), atom2->getNumber());
						}
						else
						{
							getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
						}
					}
					else
					{
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					// Long form has n number of frames
					AbstractNodeList::const_iterator in = getNodeAt(prop->values, prop->values.size() - 1);
					if((*in)->type == ANT_ATOM && ((AtomAbstractNode*)(*in).get())->isNumber())
					{
						Real duration = ((AtomAbstractNode*)(*in).get())->getNumber();
						String *names = new String[prop->values.size() - 1];
						int n = 0;

						AbstractNodeList::iterator j = prop->values.begin();
						while(j != in)
						{
							if((*j)->type == ANT_ATOM)
								names[n++] = ((AtomAbstractNode*)(*j).get())->value;
							else
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
							++j;
						}

						if(getCompilerListener())
							getCompilerListener()->getTextureNames(names, n);
						mUnit->setAnimatedTextureName(names, n, duration);
					}
					else
					{
						getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
					}
				}
			}
			break;
		case ID_CUBIC_TEXTURE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() == 2)
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
					i1 = getNodeAt(prop->values, 1);
				if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM)
				{	
					AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
					mUnit->setCubicTextureName(atom0->value, atom1->id == ID_COMBINED_UVW);
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			else if(prop->values.size() == 7)
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
					i1 = getNodeAt(prop->values, 1),
					i2 = getNodeAt(prop->values, 2),
					i3 = getNodeAt(prop->values, 3),
					i4 = getNodeAt(prop->values, 4),
					i5 = getNodeAt(prop->values, 5),
					i6 = getNodeAt(prop->values, 6);
				if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM && (*i3)->type == ANT_ATOM &&
					(*i4)->type == ANT_ATOM && (*i5)->type == ANT_ATOM && (*i6)->type == ANT_ATOM)
				{
					AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get(),
						*atom2 = (AtomAbstractNode*)(*i2).get(), *atom3 = (AtomAbstractNode*)(*i3).get(),
						*atom4 = (AtomAbstractNode*)(*i4).get(), *atom5 = (AtomAbstractNode*)(*i5).get(),
						*atom6 = (AtomAbstractNode*)(*i6).get();
					String names[6];
					names[0] = atom0->value;
					names[1] = atom1->value;
					names[2] = atom2->value;
					names[3] = atom3->value;
					names[4] = atom4->value;
					names[5] = atom5->value;
					mUnit->setCubicTextureName(names, atom6->id == ID_COMBINED_UVW);
				}

			}
			else
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			break;
		case ID_TEX_COORD_SET:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				Real val = 0.0f;
				if(getNumber(prop->values.front(), &val))
					mUnit->setTextureCoordSet(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_TEX_ADDRESS_MODE:
			{
				if(prop->values.empty())
				{
					getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
				}
				else
				{
					AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), 
						i1 = getNodeAt(prop->values, 1), 
						i2 = getNodeAt(prop->values, 2);
					TextureUnitState::UVWAddressingMode mode;
					mode.u = mode.v = mode.w = TextureUnitState::TAM_WRAP;
					
					if(i0 != prop->values.end() && (*i0)->type == ANT_ATOM)
					{
						AtomAbstractNode *atom = (AtomAbstractNode*)(*i0).get();
						switch(atom->id)
						{
						case ID_WRAP:
							mode.u = TextureUnitState::TAM_WRAP;
							break;
						case ID_CLAMP:
							mode.u = TextureUnitState::TAM_CLAMP;
							break;
						case ID_MIRROR:
							mode.u = TextureUnitState::TAM_MIRROR;
							break;
						case ID_BORDER:
							mode.u = TextureUnitState::TAM_BORDER;
							break;
						default:
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
					mode.v = mode.u;
					mode.w = mode.u;
					
					if(i1 != prop->values.end() && (*i1)->type == ANT_ATOM)
					{
						AtomAbstractNode *atom = (AtomAbstractNode*)(*i1).get();
						switch(atom->id)
						{
						case ID_WRAP:
							mode.v = TextureUnitState::TAM_WRAP;
							break;
						case ID_CLAMP:
							mode.v = TextureUnitState::TAM_CLAMP;
							break;
						case ID_MIRROR:
							mode.v = TextureUnitState::TAM_MIRROR;
							break;
						case ID_BORDER:
							mode.v = TextureUnitState::TAM_BORDER;
							break;
						default:
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}

					if(i2 != prop->values.end() && (*i2)->type == ANT_ATOM)
					{
						AtomAbstractNode *atom = (AtomAbstractNode*)(*i2).get();
						switch(atom->id)
						{
						case ID_WRAP:
							mode.w = TextureUnitState::TAM_WRAP;
							break;
						case ID_CLAMP:
							mode.w = TextureUnitState::TAM_CLAMP;
							break;
						case ID_MIRROR:
							mode.w = TextureUnitState::TAM_MIRROR;
							break;
						case ID_BORDER:
							mode.w = TextureUnitState::TAM_BORDER;
							break;
						default:
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}

					mUnit->setTextureAddressingMode(mode);
				}
			}
			break;
		case ID_TEX_BORDER_COLOUR:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else
			{
				ColourValue val;
				if(getColour(prop->values.begin(), prop->values.end(), &val))
					mUnit->setTextureBorderColour(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_FILTERING:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() == 1)
			{
				if(prop->values.front()->type == ANT_ATOM)
				{
					AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
					switch(atom->id)
					{
					case ID_NONE:
						mUnit->setTextureFiltering(TFO_NONE);
						break;
					case ID_BILINEAR:
						mUnit->setTextureFiltering(TFO_BILINEAR);
						break;
					case ID_TRILINEAR:
						mUnit->setTextureFiltering(TFO_TRILINEAR);
						break;
					case ID_ANISOTROPIC:
						mUnit->setTextureFiltering(TFO_ANISOTROPIC);
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			else if(prop->values.size() == 3)
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
					i1 = getNodeAt(prop->values, 1),
					i2 = getNodeAt(prop->values, 2);
				if((*i0)->type == ANT_ATOM &&
					(*i1)->type == ANT_ATOM &&
					(*i2)->type == ANT_ATOM)
				{
					AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(),
						*atom1 = (AtomAbstractNode*)(*i1).get(),
						*atom2 = (AtomAbstractNode*)(*i2).get();
					FilterOptions tmin, tmax, tmip;
					switch(atom0->id)
					{
					case ID_NONE:
						tmin = FO_NONE;
						break;
					case ID_POINT:
						tmin = FO_POINT;
						break;
					case ID_LINEAR:
						tmin = FO_LINEAR;
						break;
					case ID_ANISOTROPIC:
						tmin = FO_ANISOTROPIC;
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}

					switch(atom1->id)
					{
					case ID_NONE:
						tmax = FO_NONE;
						break;
					case ID_POINT:
						tmax = FO_POINT;
						break;
					case ID_LINEAR:
						tmax = FO_LINEAR;
						break;
					case ID_ANISOTROPIC:
						tmax = FO_ANISOTROPIC;
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}

					switch(atom2->id)
					{
					case ID_NONE:
						tmip = FO_NONE;
						break;
					case ID_POINT:
						tmip = FO_POINT;
						break;
					case ID_LINEAR:
						tmip = FO_LINEAR;
						break;
					case ID_ANISOTROPIC:
						tmip = FO_ANISOTROPIC;
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}

					mUnit->setTextureFiltering(tmin, tmax, tmip);
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			else
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			break;
		case ID_MAX_ANISOTROPY:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				Real val = 0.0f;
				if(getNumber(prop->values.front(), &val))
					mUnit->setTextureAnisotropy(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_MIPMAP_BIAS:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				Real val = 0.0f;
				if(getNumber(prop->values.front(), &val))
					mUnit->setTextureMipmapBias(val);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_COLOUR_OP:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				if(prop->values.front()->type == ANT_ATOM)
				{
					AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
					switch(atom->id)
					{
					case ID_REPLACE:
						mUnit->setColourOperation(LBO_REPLACE);
						break;
					case ID_ADD:
						mUnit->setColourOperation(LBO_ADD);
						break;
					case ID_MODULATE:
						mUnit->setColourOperation(LBO_MODULATE);
						break;
					case ID_ALPHA_BLEND:
						mUnit->setColourOperation(LBO_ALPHA_BLEND);
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_COLOUR_OP_EX:
			if(prop->values.size() < 3)
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 6)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
					i1 = getNodeAt(prop->values, 1),
					i2 = getNodeAt(prop->values, 2);
				if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM)
				{
					AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(),
						*atom1 = (AtomAbstractNode*)(*i1).get(),
						*atom2 = (AtomAbstractNode*)(*i2).get();
					LayerBlendOperationEx op = LBX_ADD;
					LayerBlendSource source1 = LBS_CURRENT, source2 = LBS_TEXTURE;
					ColourValue arg1, arg2;
					Real manualBlend = 0.0f;

					switch(atom0->id)
					{
					case ID_SOURCE1:
						op = LBX_SOURCE1;
						break;
					case ID_SOURCE2:
						op = LBX_SOURCE2;
						break;
					case ID_MODULATE:
						op = LBX_MODULATE;
						break;
					case ID_MODULATE_X2:
						op = LBX_MODULATE_X2;
						break;
					case ID_MODULATE_X4:
						op = LBX_MODULATE_X4;
						break;
					case ID_ADD:
						op = LBX_ADD;
						break;
					case ID_ADD_SIGNED:
						op = LBX_ADD_SIGNED;
						break;
					case ID_ADD_SMOOTH:
						op = LBX_ADD_SMOOTH;
						break;
					case ID_SUBTRACT:
						op = LBX_SUBTRACT;
						break;
					case ID_BLEND_DIFFUSE_ALPHA:
						op = LBX_BLEND_DIFFUSE_ALPHA;
						break;
					case ID_BLEND_TEXTURE_ALPHA:
						op = LBX_BLEND_TEXTURE_ALPHA;
						break;
					case ID_BLEND_CURRENT_ALPHA:
						op = LBX_BLEND_CURRENT_ALPHA;
						break;
					case ID_BLEND_MANUAL:
						op = LBX_BLEND_MANUAL;
						break;
					case ID_DOT_PRODUCT:
						op = LBX_DOTPRODUCT;
						break;
					case ID_BLEND_DIFFUSE_COLOUR:
						op = LBX_BLEND_DIFFUSE_COLOUR;
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}

					switch(atom1->id)
					{
					case ID_SRC_CURRENT:
						source1 = LBS_CURRENT;
						break;
					case ID_SRC_TEXTURE:
						source1 = LBS_TEXTURE;
						break;
					case ID_SRC_DIFFUSE:
						source1 = LBS_DIFFUSE;
						break;
					case ID_SRC_SPECULAR:
						source1 = LBS_SPECULAR;
						break;
					case ID_SRC_MANUAL:
						source1 = LBS_MANUAL;
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}

					switch(atom2->id)
					{
					case ID_SRC_CURRENT:
						source2 = LBS_CURRENT;
						break;
					case ID_SRC_TEXTURE:
						source2 = LBS_TEXTURE;
						break;
					case ID_SRC_DIFFUSE:
						source2 = LBS_DIFFUSE;
						break;
					case ID_SRC_SPECULAR:
						source2 = LBS_SPECULAR;
						break;
					case ID_SRC_MANUAL:
						source2 = LBS_MANUAL;
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}

					if(op == LBX_BLEND_MANUAL)
					{
						AbstractNodeList::const_iterator i3 = getNodeAt(prop->values, 3);
						if(i3 != prop->values.end())
						{
							if((*i3)->type == ANT_ATOM && ((AtomAbstractNode*)(*i3).get())->isNumber())
								manualBlend = ((AtomAbstractNode*)(*i3).get())->getNumber();
							else
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
						else
						{
							getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
						}
					}

					AbstractNodeList::const_iterator j = getNodeAt(prop->values, 3);
					if(op == LBX_BLEND_MANUAL)
						++j;
					if(source1 == LBS_MANUAL)
					{
						if(j != prop->values.end())
						{
							if(!getColour(j, prop->values.end(), &arg1))
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
						else
						{
							getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
						}
					}
					if(source2 == LBS_MANUAL)
					{
						if(j != prop->values.end())
						{
							if(!getColour(j, prop->values.end(), &arg2))
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
						else
						{
							getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
						}
					}

					mUnit->setColourOperationEx(op, source1, source2, arg1, arg2, manualBlend);
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_COLOUR_OP_MULTIPASS_FALLBACK:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 2)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
				SceneBlendFactor sbf0, sbf1;
				if(getSceneBlendFactor(*i0, &sbf0) && getSceneBlendFactor(*i1, &sbf1))
					mUnit->setColourOpMultipassFallback(sbf0, sbf1);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_ALPHA_OP_EX:
			if(prop->values.size() < 3)
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 6)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
					i1 = getNodeAt(prop->values, 1),
					i2 = getNodeAt(prop->values, 2);
				if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM)
				{
					AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(),
						*atom1 = (AtomAbstractNode*)(*i1).get(),
						*atom2 = (AtomAbstractNode*)(*i2).get();
					LayerBlendOperationEx op = LBX_ADD;
					LayerBlendSource source1 = LBS_CURRENT, source2 = LBS_TEXTURE;
					Real arg1, arg2;
					Real manualBlend = 0.0f;

					switch(atom0->id)
					{
					case ID_SOURCE1:
						op = LBX_SOURCE1;
						break;
					case ID_SOURCE2:
						op = LBX_SOURCE2;
						break;
					case ID_MODULATE:
						op = LBX_MODULATE;
						break;
					case ID_MODULATE_X2:
						op = LBX_MODULATE_X2;
						break;
					case ID_MODULATE_X4:
						op = LBX_MODULATE_X4;
						break;
					case ID_ADD:
						op = LBX_ADD;
						break;
					case ID_ADD_SIGNED:
						op = LBX_ADD_SIGNED;
						break;
					case ID_ADD_SMOOTH:
						op = LBX_ADD_SMOOTH;
						break;
					case ID_SUBTRACT:
						op = LBX_SUBTRACT;
						break;
					case ID_BLEND_DIFFUSE_ALPHA:
						op = LBX_BLEND_DIFFUSE_ALPHA;
						break;
					case ID_BLEND_TEXTURE_ALPHA:
						op = LBX_BLEND_TEXTURE_ALPHA;
						break;
					case ID_BLEND_CURRENT_ALPHA:
						op = LBX_BLEND_CURRENT_ALPHA;
						break;
					case ID_BLEND_MANUAL:
						op = LBX_BLEND_MANUAL;
						break;
					case ID_DOT_PRODUCT:
						op = LBX_DOTPRODUCT;
						break;
					case ID_BLEND_DIFFUSE_COLOUR:
						op = LBX_BLEND_DIFFUSE_COLOUR;
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}

					switch(atom1->id)
					{
					case ID_SRC_CURRENT:
						source1 = LBS_CURRENT;
						break;
					case ID_SRC_TEXTURE:
						source1 = LBS_TEXTURE;
						break;
					case ID_SRC_DIFFUSE:
						source1 = LBS_DIFFUSE;
						break;
					case ID_SRC_SPECULAR:
						source1 = LBS_SPECULAR;
						break;
					case ID_SRC_MANUAL:
						source1 = LBS_MANUAL;
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}

					switch(atom2->id)
					{
					case ID_SRC_CURRENT:
						source2 = LBS_CURRENT;
						break;
					case ID_SRC_TEXTURE:
						source2 = LBS_TEXTURE;
						break;
					case ID_SRC_DIFFUSE:
						source2 = LBS_DIFFUSE;
						break;
					case ID_SRC_SPECULAR:
						source2 = LBS_SPECULAR;
						break;
					case ID_SRC_MANUAL:
						source2 = LBS_MANUAL;
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}

					if(op == LBX_BLEND_MANUAL)
					{
						AbstractNodeList::const_iterator i3 = getNodeAt(prop->values, 3);
						if(i3 != prop->values.end())
						{
							if((*i3)->type == ANT_ATOM && ((AtomAbstractNode*)(*i3).get())->isNumber())
								manualBlend = ((AtomAbstractNode*)(*i3).get())->getNumber();
							else
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
						else
						{
							getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
						}
					}

					AbstractNodeList::const_iterator j = getNodeAt(prop->values, 3);
					if(op == LBX_BLEND_MANUAL)
						++j;
					if(source1 == LBS_MANUAL)
					{
						if(j != prop->values.end())
						{
							if(!getNumber(*j, &arg1))
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
							else
								++j;
						}
						else
						{
							getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
						}
					}
					if(source2 == LBS_MANUAL)
					{
						if(j != prop->values.end())
						{
							if(!getNumber(*j, &arg2))
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
						else
						{
							getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
						}
					}

					mUnit->setAlphaOperation(op, source1, source2, arg1, arg2, manualBlend);
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_ENV_MAP:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				if(prop->values.front()->type == ANT_ATOM)
				{
					AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
					switch(atom->id)
					{
					case ID_OFF:
						mUnit->setEnvironmentMap(false);
						break;
					case ID_SPHERICAL:
						mUnit->setEnvironmentMap(true, TextureUnitState::ENV_CURVED);
						break;
					case ID_PLANAR:
						mUnit->setEnvironmentMap(true, TextureUnitState::ENV_PLANAR);
						break;
					case ID_CUBIC_REFLECTION:
						mUnit->setEnvironmentMap(true, TextureUnitState::ENV_REFLECTION);
						break;
					case ID_CUBIC_NORMAL:
						mUnit->setEnvironmentMap(true, TextureUnitState::ENV_NORMAL);
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_SCROLL:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 2)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
				Real x, y;
				if(getNumber(*i0, &x) && getNumber(*i1, &y))
					mUnit->setTextureScroll(x, y);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_SCROLL_ANIM:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 2)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
				Real x, y;
				if(getNumber(*i0, &x) && getNumber(*i1, &y))
					mUnit->setScrollAnimation(x, y);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_ROTATE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				Real angle;
				if(getNumber(prop->values.front(), &angle))
					mUnit->setTextureRotate(Degree(angle));
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_ROTATE_ANIM:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				Real angle;
				if(getNumber(prop->values.front(), &angle))
					mUnit->setRotateAnimation(angle);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_SCALE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 2)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1);
				Real x, y;
				if(getNumber(*i0, &x) && getNumber(*i1, &y))
					mUnit->setTextureScale(x, y);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_WAVE_XFORM:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 6)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1),
					i2 = getNodeAt(prop->values, 2), i3 = getNodeAt(prop->values, 3),
					i4 = getNodeAt(prop->values, 4), i5 = getNodeAt(prop->values, 5);
				if((*i0)->type == ANT_ATOM && (*i1)->type == ANT_ATOM && (*i2)->type == ANT_ATOM &&
					(*i3)->type == ANT_ATOM && (*i4)->type == ANT_ATOM && (*i5)->type == ANT_ATOM)
				{
					AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
					TextureUnitState::TextureTransformType type;
					WaveformType wave;
					Real base = 0.0f, freq = 0.0f, phase = 0.0f, amp = 0.0f;

					switch(atom0->id)
					{
					case ID_SCROLL_X:
						type = TextureUnitState::TT_TRANSLATE_U;
						break;
					case ID_SCROLL_Y:
						type = TextureUnitState::TT_TRANSLATE_V;
						break;
					case ID_SCALE_X:
						type = TextureUnitState::TT_SCALE_U;
						break;
					case ID_SCALE_Y:
						type = TextureUnitState::TT_SCALE_V;
						break;
					case ID_ROTATE:
						type = TextureUnitState::TT_ROTATE;
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}

					switch(atom1->id)
					{
					case ID_SINE:
						wave = WFT_SINE;
						break;
					case ID_TRIANGLE:
						wave = WFT_TRIANGLE;
						break;
					case ID_SQUARE:
						wave = WFT_SQUARE;
						break;
					case ID_SAWTOOTH:
						wave = WFT_SAWTOOTH;
						break;
					case ID_INVERSE_SAWTOOTH:
						wave = WFT_INVERSE_SAWTOOTH;
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}

					if(!getNumber(*i2, &base) || !getNumber(*i3, &freq) || !getNumber(*i4, &phase) || !getNumber(*i5, &amp))
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);

					mUnit->setTransformAnimation(type, wave, base, freq, phase, amp);
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_TRANSFORM:
			{
				Matrix4 m;
				if(getMatrix4(prop->values.begin(), prop->values.end(), &m))
					mUnit->setTextureTransform(m);
				else
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
			}
			break;
		case ID_BINDING_TYPE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				if(prop->values.front()->type == ANT_ATOM)
				{
					AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
					switch(atom->id)
					{
					case ID_VERTEX:
						mUnit->setBindingType(TextureUnitState::BT_VERTEX);
						break;
					case ID_FRAGMENT:
						mUnit->setBindingType(TextureUnitState::BT_FRAGMENT);
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_CONTENT_TYPE:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
			}
			else if(prop->values.size() > 1)
			{
				getCompiler()->addError(CE_FEWERPARAMETERSEXPECTED, prop->file, prop->line);
			}
			else
			{
				if(prop->values.front()->type == ANT_ATOM)
				{
					AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
					switch(atom->id)
					{
					case ID_NAMED:
						mUnit->setContentType(TextureUnitState::CONTENT_NAMED);
						break;
					case ID_SHADOW:
						mUnit->setContentType(TextureUnitState::CONTENT_SHADOW);
						break;
					default:
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		}
	}

	// GpuProgramTranslator
	ScriptCompiler::GpuProgramTranslator::GpuProgramTranslator()
	{
	}

	void ScriptCompiler::GpuProgramTranslator::processObject(ObjectAbstractNode *obj)
	{
		if(obj->name.empty())
		{
			getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
			return;
		}

		std::list<std::pair<String,String> > customParameters;
		String syntax, source;
		AbstractNodePtr params;
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = (PropertyAbstractNode*)(*i).get();
				if(prop->id == ID_SOURCE)
				{
					if(!prop->values.empty())
					{
						if(prop->values.front()->type == ANT_ATOM)
							source = ((AtomAbstractNode*)prop->values.front().get())->value;
						else
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					else
					{
						getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
					}
				}
				else if(prop->id == ID_SYNTAX)
				{
					if(!prop->values.empty())
					{
						if(prop->values.front()->type == ANT_ATOM)
							syntax = ((AtomAbstractNode*)prop->values.front().get())->value;
						else
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					else
					{
						getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
					}
				}
				else
				{
					String name = prop->name, value;
					bool first = true;
					for(AbstractNodeList::iterator i = prop->values.begin(); i != prop->values.end(); ++i)
					{
						if((*i)->type == ANT_ATOM)
						{
							if(!first)
								value += " ";
							else
								first = false;
							value += ((AtomAbstractNode*)(*i).get())->value;
						}
					}
					customParameters.push_back(std::make_pair(name, value));
				}
			}
			else if((*i)->type == ANT_OBJECT)
			{
				if(((ObjectAbstractNode*)(*i).get())->id == ID_DEFAULT_PARAMS)
					params = *i;
			}
		}

		// Allocate the program
		GpuProgramPtr prog;
		if(getCompilerListener())
			prog = getCompilerListener()->createGpuProgram(obj->name, getCompiler()->getResourceGroup(), source, obj->id == ID_VERTEX_PROGRAM ? GPT_VERTEX_PROGRAM : GPT_FRAGMENT_PROGRAM, syntax); 
		else
			prog = GpuProgramManager::getSingleton().createProgram(obj->name, getCompiler()->getResourceGroup(), source, obj->id == ID_VERTEX_PROGRAM ? GPT_VERTEX_PROGRAM : GPT_FRAGMENT_PROGRAM, syntax);

		// Check that allocation worked
		if(prog.isNull())
		{
			getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
			return;
		}

		prog->_notifyOrigin(obj->file);

		// Set the custom parameters
		for(std::list<std::pair<String,String> >::iterator i = customParameters.begin(); i != customParameters.end(); ++i)
			prog->setParameter(i->first, i->second);

		obj->context = Any(prog.get());

		// Set up default parameters
		if(prog->isSupported() && !params.isNull())
		{
			GpuProgramParametersSharedPtr ptr = prog->getDefaultParameters();
			GpuProgramParametersTranslator translator(ptr);
			Translator::translate(&translator, params, getCompiler());
		}
		prog->touch();
	}

	void ScriptCompiler::GpuProgramTranslator::processProperty(Ogre::PropertyAbstractNode *prop)
	{
	}

	// HighLevelGpuProgramTranslator
	ScriptCompiler::HighLevelGpuProgramTranslator::HighLevelGpuProgramTranslator()
	{
	}

	void ScriptCompiler::HighLevelGpuProgramTranslator::processObject(ObjectAbstractNode *obj)
	{
		if(obj->name.empty())
		{
			getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
			return;
		}

		std::list<std::pair<String,String> > customParameters;
		String source, language = ((AtomAbstractNode*)obj->values.front().get())->value;
		AbstractNodePtr params;
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = (PropertyAbstractNode*)(*i).get();
				if(prop->id == ID_SOURCE)
				{
					if(!prop->values.empty())
					{
						if(prop->values.front()->type == ANT_ATOM)
							source = ((AtomAbstractNode*)prop->values.front().get())->value;
						else
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
					else
					{
						getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
					}
				}
				else
				{
					String name = prop->name, value;
					bool first = true;
					for(AbstractNodeList::iterator i = prop->values.begin(); i != prop->values.end(); ++i)
					{
						if((*i)->type == ANT_ATOM)
						{
							if(!first)
								value += " ";
							else
								first = false;
							value += ((AtomAbstractNode*)(*i).get())->value;
						}
					}
					customParameters.push_back(std::make_pair(name, value));
				}
			}
			else if((*i)->type == ANT_OBJECT)
			{
				if(((ObjectAbstractNode*)(*i).get())->id == ID_DEFAULT_PARAMS)
					params = *i;
			}
		}

		// Allocate the program
		HighLevelGpuProgramPtr prog;
		if(getCompilerListener())
			prog = getCompilerListener()->createHighLevelGpuProgram(obj->name, getCompiler()->getResourceGroup(), language, obj->id == ID_VERTEX_PROGRAM ? GPT_VERTEX_PROGRAM : GPT_FRAGMENT_PROGRAM, source); 
		else
		{
			prog = HighLevelGpuProgramManager::getSingleton().createProgram(obj->name, getCompiler()->getResourceGroup(), language, obj->id == ID_VERTEX_PROGRAM ? GPT_VERTEX_PROGRAM : GPT_FRAGMENT_PROGRAM);
			prog->setSourceFile(source);
		}

		// Check that allocation worked
		if(prog.isNull())
		{
			getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
			return;
		}

		prog->_notifyOrigin(obj->file);

		// Set the custom parameters
		for(std::list<std::pair<String,String> >::iterator i = customParameters.begin(); i != customParameters.end(); ++i)
			prog->setParameter(i->first, i->second);

		obj->context = Any(prog.get());

		// Set up default parameters
		if(prog->isSupported() && !params.isNull())
		{
			GpuProgramParametersSharedPtr ptr = prog->getDefaultParameters();
			GpuProgramParametersTranslator translator(ptr);
			Translator::translate(&translator, params, getCompiler());
		}
		prog->touch();
	}

	void ScriptCompiler::HighLevelGpuProgramTranslator::processProperty(Ogre::PropertyAbstractNode *)
	{
	}

	// UnifiedGpuProgramTranslator
	ScriptCompiler::UnifiedGpuProgramTranslator::UnifiedGpuProgramTranslator()
	{
	}

	void ScriptCompiler::UnifiedGpuProgramTranslator::processObject(ObjectAbstractNode *obj)
	{
		if(obj->name.empty())
		{
			getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
			return;
		}

		std::list<std::pair<String,String> > customParameters;
		String source;
		AbstractNodePtr params;
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = (PropertyAbstractNode*)(*i).get();
				if(prop->name == "delegate")
				{
					String value;
					if(!prop->values.empty() && prop->values.front()->type == ANT_ATOM)
						value = ((AtomAbstractNode*)prop->values.front().get())->value;
					if(getCompilerListener())
						getCompilerListener()->getGpuProgramName(&value);
					customParameters.push_back(std::make_pair("delegate", value));
				}
				else
				{
					String name = prop->name, value;
					bool first = true;
					for(AbstractNodeList::iterator i = prop->values.begin(); i != prop->values.end(); ++i)
					{
						if((*i)->type == ANT_ATOM)
						{
							if(!first)
								value += " ";
							else
								first = false;
							value += ((AtomAbstractNode*)(*i).get())->value;
						}
					}
					customParameters.push_back(std::make_pair(name, value));
				}
			}
		}

		// Allocate the program
		HighLevelGpuProgramPtr prog;
		if(getCompilerListener())
			prog = getCompilerListener()->createHighLevelGpuProgram(obj->name, getCompiler()->getResourceGroup(), "unified", obj->id == ID_VERTEX_PROGRAM ? GPT_VERTEX_PROGRAM : GPT_FRAGMENT_PROGRAM, source); 
		else
		{
			prog = HighLevelGpuProgramManager::getSingleton().createProgram(obj->name, getCompiler()->getResourceGroup(), "unified", obj->id == ID_VERTEX_PROGRAM ? GPT_VERTEX_PROGRAM : GPT_FRAGMENT_PROGRAM);
			prog->setSourceFile(source);
		}

		// Check that allocation worked
		if(prog.isNull())
		{
			getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
			return;
		}

		prog->_notifyOrigin(obj->file);

		// Set the custom parameters
		for(std::list<std::pair<String,String> >::iterator i = customParameters.begin(); i != customParameters.end(); ++i)
			prog->setParameter(i->first, i->second);

		obj->context = Any(prog.get());

		// Set up default parameters
		if(prog->isSupported() && !params.isNull())
		{
			GpuProgramParametersSharedPtr ptr = prog->getDefaultParameters();
			GpuProgramParametersTranslator translator(ptr);
			Translator::translate(&translator, params, getCompiler());
		}
		prog->touch();
	}

	void ScriptCompiler::UnifiedGpuProgramTranslator::processProperty(Ogre::PropertyAbstractNode *)
	{
	}

	// GpuProgramParametersTranslator
	ScriptCompiler::GpuProgramParametersTranslator::GpuProgramParametersTranslator(const Ogre::GpuProgramParametersSharedPtr &params)
		:mParams(params), mAnimParametricsCount(0)
	{
	}

	void ScriptCompiler::GpuProgramParametersTranslator::processObject(ObjectAbstractNode *obj)
	{
		mAnimParametricsCount = 0;

		// Set up the parameters
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				Translator::translate(this, *i, getCompiler());
			}
			else if((*i)->type == ANT_OBJECT)
			{
				Translator::translate(0, *i, getCompiler());
			}
		}
	}

	void ScriptCompiler::GpuProgramParametersTranslator::processProperty(Ogre::PropertyAbstractNode *prop)
	{
		switch(prop->id)
		{
		case ID_PARAM_INDEXED:
		case ID_PARAM_NAMED:
			{
				if(prop->values.size() >= 3)
				{
					bool named = (prop->id == ID_PARAM_NAMED);
					AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1), 
						k = getNodeAt(prop->values, 2);

					if((*i0)->type != ANT_ATOM || (*i1)->type != ANT_ATOM)
					{
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						return;
					}

					AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
					if(!named && !atom0->isNumber())
					{
						getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
						return;
					}

					String name;
					size_t index = 0;
					// Assign the name/index
					if(named)
						name = atom0->value;
					else
						index = atom0->getNumber();
				
					// Determine the type
					if(atom1->value == "matrix4x4")
					{	
						Matrix4 m;
						if(getMatrix4(k, prop->values.end(), &m))
						{
							try
							{
								if(named)
									mParams->setNamedConstant(name, m);
								else
									mParams->setConstant(index, m);
							}
							catch(...)
							{
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
							}
						}
						else
						{
							getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
						}
					}
					else
					{
						// Find the number of parameters
						bool isValid = true;
						GpuProgramParameters::ElementType type;
						int count = 0;
						if(atom1->value.find("float") != String::npos)
						{
							type = GpuProgramParameters::ET_REAL;
							if(atom1->value.size() >= 6)
								count = StringConverter::parseInt(atom1->value.substr(5));
							else
							{
								count = 1;
							}
						}
						else if(atom1->value.find("int") != String::npos)
						{
							type = GpuProgramParameters::ET_INT;
							if(atom1->value.size() >= 4)
								count = StringConverter::parseInt(atom1->value.substr(3));
							else
							{
								count = 1;
							}
						}
						else
						{
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
							isValid = false;
						}

						if(isValid)
						{
							// First, clear out any offending auto constants
							if(named)
								mParams->clearNamedAutoConstant(name);
							else
								mParams->clearAutoConstant(index);

							int roundedCount = count%4 != 0 ? count + 4 - (count%4) : count;
							if(type == GpuProgramParameters::ET_INT)
							{
								int *vals = new int[roundedCount];
								if(getInts(k, prop->values.end(), vals, roundedCount))
								{
									try
									{
										if(named)
											mParams->setNamedConstant(name, vals, count, 1);
										else
											mParams->setConstant(index, vals, roundedCount/4);
									}
									catch(...)
									{
										getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
									}
								}
								else
								{
									getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
								}
								delete[] vals;
							}
							else
							{
								float *vals = new float[roundedCount];
								if(getFloats(k, prop->values.end(), vals, roundedCount))
								{
									try
									{
										if(named)
											mParams->setNamedConstant(name, vals, count, 1);
										else
											mParams->setConstant(index, vals, roundedCount/4);
									}
									catch(...)
									{
										getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
									}
								}
								else
								{
									getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
								}
								delete[] vals;
							}
						}
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		case ID_PARAM_INDEXED_AUTO:
		case ID_PARAM_NAMED_AUTO:
			{
				bool named = (prop->id == ID_PARAM_NAMED_AUTO);
				String name;
				size_t index = 0;

				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0),
					i1 = getNodeAt(prop->values, 1), i2 = getNodeAt(prop->values, 2);
				if((*i0)->type != ANT_ATOM || (*i1)->type != ANT_ATOM)
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					return;
				}
				AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i0).get(), *atom1 = (AtomAbstractNode*)(*i1).get();
				if(!named && !atom0->isNumber())
				{
					getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
					return;
				}

				if(named)
					name = atom0->value;
				else
					index = atom0->getNumber();

				// Look up the auto constant
				StringUtil::toLowerCase(atom1->value);
				const GpuProgramParameters::AutoConstantDefinition *def =
					GpuProgramParameters::getAutoConstantDefinition(atom1->value);
				if(def)
				{
					switch(def->dataType)
					{
					case GpuProgramParameters::ACDT_NONE:
						// Set the auto constant
						try
						{
							if(named)
								mParams->setNamedAutoConstant(name, def->acType);
							else
								mParams->setAutoConstant(index, def->acType);
						}
						catch(...)
						{
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
						break;
					case GpuProgramParameters::ACDT_INT:
						if(def->acType == GpuProgramParameters::ACT_ANIMATION_PARAMETRIC)
						{
							try
							{
								if(named)
									mParams->setNamedAutoConstant(name, def->acType, mAnimParametricsCount++);
								else
									mParams->setAutoConstant(index, def->acType, mAnimParametricsCount++);
							}
							catch(...)
							{
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
							}
						}
						else
						{
							// Only certain texture projection auto params will assume 0
							// Otherwise we will expect that 3rd parameter
							if(i2 == prop->values.end())
							{
								if(def->acType == GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX ||
									def->acType == GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX ||
									def->acType == GpuProgramParameters::ACT_SPOTLIGHT_VIEWPROJ_MATRIX ||
									def->acType == GpuProgramParameters::ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX
									)
								{
									try
									{
										if(named)
											mParams->setNamedAutoConstant(name, def->acType, 0);
										else
											mParams->setAutoConstant(index, def->acType, 0);
									}
									catch(...)
									{
										getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
									}
								}
								else
								{
									getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
								}
							}
							else
							{
								if((*i2)->type == ANT_ATOM && ((AtomAbstractNode*)(*i2).get())->isNumber())
								{
									try
									{
										if(named)
											mParams->setNamedAutoConstant(name, def->acType, (size_t)((AtomAbstractNode*)(*i2).get())->getNumber());
										else
											mParams->setAutoConstant(index, def->acType, (size_t)((AtomAbstractNode*)(*i2).get())->getNumber());
									}
									catch(...)
									{
										getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
									}
								}
							}
						}
						break;
					case GpuProgramParameters::ACDT_REAL:
						if(def->acType == GpuProgramParameters::ACT_TIME ||
							def->acType == GpuProgramParameters::ACT_FRAME_TIME)
						{
							Real f = 1.0f;
							if(i2 != prop->values.end() && (*i2)->type == ANT_ATOM && ((AtomAbstractNode*)(*i2).get())->isNumber())
								f = ((AtomAbstractNode*)(*i2).get())->getNumber();
							
							try
							{
								if(named)
									mParams->setNamedAutoConstantReal(name, def->acType, f);
								else
									mParams->setAutoConstantReal(index, def->acType, f);
							}
							catch(...)
							{
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
							}
						}
						else
						{
							if(i2 != prop->values.end())
							{
								if((*i2)->type == ANT_ATOM && ((AtomAbstractNode*)(*i2).get())->isNumber())
								{
									try
									{
										if(named)
											mParams->setNamedAutoConstantReal(name, def->acType, ((AtomAbstractNode*)(*i2).get())->getNumber());
										else
											mParams->setAutoConstantReal(index, def->acType, ((AtomAbstractNode*)(*i2).get())->getNumber());
									}
									catch(...)
									{
										getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
									}
								}
								else
								{
									getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
								}
							}
							else
							{
								getCompiler()->addError(CE_NUMBEREXPECTED, prop->file, prop->line);
							}
						}
						break;
					}
				}
				else
				{
					getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				}
			}
			break;
		}
	}

	// ParticleSystemTranslator
	ScriptCompiler::ParticleSystemTranslator::ParticleSystemTranslator()
		:mSystem(0)
	{
	}

	void ScriptCompiler::ParticleSystemTranslator::processObject(ObjectAbstractNode *obj)
	{
		// Find the name
		if(obj->name.empty())
		{
			getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
			return;
		}

		// Allocate the particle system
		if(getCompilerListener())
			mSystem = getCompilerListener()->createParticleSystem(obj->name, getCompiler()->getResourceGroup());
		else
			mSystem = ParticleSystemManager::getSingleton().createTemplate(obj->name, getCompiler()->getResourceGroup());

		if(!mSystem)
		{
			getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
			return;
		}

		mSystem->removeAllEmitters();
		mSystem->removeAllAffectors();

		obj->context = Any(mSystem);

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				ObjectAbstractNode *obj = (ObjectAbstractNode*)(*i).get();
				switch(obj->id)
				{
				case ID_EMITTER:
					{
						// The name of this emitter is the type
						if(obj->name.empty())
						{
							getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
							continue;
						}
						ParticleEmitter *emitter = mSystem->addEmitter(obj->name);
						if(!emitter)
						{
							getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
							continue;
						}
						ScriptCompiler::ParticleEmitterTranslator translator(emitter);
						Translator::translate(&translator, *i, getCompiler());
					}
					break;
				case ID_AFFECTOR:
					{
						// The name of this emitter is the type
						if(obj->name.empty())
						{
							getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
							continue;
						}
						ParticleAffector *affector = mSystem->addAffector(obj->name);
						if(!affector)
						{
							getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
							continue;
						}
						ScriptCompiler::ParticleAffectorTranslator translator(affector);
						Translator::translate(&translator, *i, getCompiler());
					}
					break;
				default:
					Translator::translate((ScriptCompiler::Translator*)0, *i, getCompiler());
				}
			}
			else if((*i)->type == ANT_PROPERTY)
			{
				Translator::translate(this, *i, getCompiler());
			}
		}
	}

	void ScriptCompiler::ParticleSystemTranslator::processProperty(PropertyAbstractNode *prop)
	{
		switch(prop->id)
		{
		case ID_MATERIAL:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
				return;
			}
			else
			{
				if(prop->values.front()->type == ANT_ATOM)
				{
					String name = ((AtomAbstractNode*)prop->values.front().get())->value;
					if(getCompilerListener())
						getCompilerListener()->getMaterialName(&name);
					if(!mSystem->setParameter("material", name))
					{
						if(mSystem->getRenderer())
						{
							if(!mSystem->getRenderer()->setParameter("material", name))
								getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						}
					}
				}
			}
			break;
		default:
			if(prop->values.empty())
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
				return;
			}
			else
			{
				String name = prop->name, value;

				// Glob the values together
				for(AbstractNodeList::iterator i = prop->values.begin(); i != prop->values.end(); ++i)
				{
					if((*i)->type == ANT_ATOM)
					{
						if(value.empty())
							value = ((AtomAbstractNode*)(*i).get())->value;
						else
							value = value + " " + ((AtomAbstractNode*)(*i).get())->value;
					}
					else
					{
						getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
						return;
					}
				}

				if(!mSystem->setParameter(name, value))
				{
					if(mSystem->getRenderer())
					{
						if(!mSystem->getRenderer()->setParameter(name, value))
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
					}
				}
			}
			break;
		}
	}

	// ParticleEmitterTranslator
	ScriptCompiler::ParticleEmitterTranslator::ParticleEmitterTranslator(Ogre::ParticleEmitter *emitter)
		:mEmitter(emitter)
	{
	}

	void ScriptCompiler::ParticleEmitterTranslator::processObject(Ogre::ObjectAbstractNode *obj)
	{
		obj->context = Any(mEmitter);

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				Translator::translate((Translator*)0, *i, getCompiler());
			}
			else if((*i)->type == ANT_PROPERTY)
			{
				Translator::translate(this, *i, getCompiler());
			}
		}
	}

	void ScriptCompiler::ParticleEmitterTranslator::processProperty(Ogre::PropertyAbstractNode *prop)
	{
		String value;

		// Glob the values together
		for(AbstractNodeList::iterator i = prop->values.begin(); i != prop->values.end(); ++i)
		{
			if((*i)->type == ANT_ATOM)
			{
				if(value.empty())
					value = ((AtomAbstractNode*)(*i).get())->value;
				else
					value = value + " " + ((AtomAbstractNode*)(*i).get())->value;
			}
			else
			{
				getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				return;
			}
		}

		if(!mEmitter->setParameter(prop->name, value))
		{
			getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
		}
	}

	// ParticleAffectorTranslator
	ScriptCompiler::ParticleAffectorTranslator::ParticleAffectorTranslator(Ogre::ParticleAffector *affector)
		:mAffector(affector)
	{
	}

	void ScriptCompiler::ParticleAffectorTranslator::processObject(Ogre::ObjectAbstractNode *obj)
	{
		obj->context = Any(mAffector);

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				Translator::translate((Translator*)0, *i, getCompiler());
			}
			else if((*i)->type == ANT_PROPERTY)
			{
				Translator::translate(this, *i, getCompiler());
			}
		}
	}

	void ScriptCompiler::ParticleAffectorTranslator::processProperty(Ogre::PropertyAbstractNode *prop)
	{
		String value;

		// Glob the values together
		for(AbstractNodeList::iterator i = prop->values.begin(); i != prop->values.end(); ++i)
		{
			if((*i)->type == ANT_ATOM)
			{
				if(value.empty())
					value = ((AtomAbstractNode*)(*i).get())->value;
				else
					value = value + " " + ((AtomAbstractNode*)(*i).get())->value;
			}
			else
			{
				getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
				return;
			}
		}

		if(!mAffector->setParameter(prop->name, value))
		{
			getCompiler()->addError(CE_INVALIDPARAMETERS, prop->file, prop->line);
		}
	}

	// CompositorTranslator
	ScriptCompiler::CompositorTranslator::CompositorTranslator()
	{
	}

	void ScriptCompiler::CompositorTranslator::processObject(ObjectAbstractNode *obj)
	{
		if(obj->name.empty())
		{
			getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);
			return;
		}

		// Create the compositor
		if(getCompilerListener())
			mCompositor = getCompilerListener()->createCompositor(obj->name, getCompiler()->getResourceGroup());
		else
			mCompositor = CompositorManager::getSingleton().create(obj->name, getCompiler()->getResourceGroup());

		if(mCompositor.isNull())
		{
			getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
			return;
		}

		mCompositor->removeAllTechniques();
		obj->context = Any(mCompositor.get());

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				ObjectAbstractNode *obj = (ObjectAbstractNode*)(*i).get();
				switch(obj->id)
				{
				case ID_TECHNIQUE:
					{
						CompositionTechniqueTranslator translator(mCompositor->createTechnique());
						Translator::translate(&translator, *i, getCompiler());
					}
					break;
				default:
					Translator::translate(0, *i, getCompiler());
				}
			}
			else if((*i)->type == ANT_PROPERTY)
			{
				Translator::translate(this, *i, getCompiler());
			}
		}
	}

	void ScriptCompiler::CompositorTranslator::processProperty(PropertyAbstractNode *obj)
	{
	}

	// CompositionTechniqueTranslator
	ScriptCompiler::CompositionTechniqueTranslator::CompositionTechniqueTranslator(CompositionTechnique *technique)
		:mTechnique(technique)
	{
	}

	void ScriptCompiler::CompositionTechniqueTranslator::processObject(ObjectAbstractNode *obj)
	{
		obj->context = Any(mTechnique);

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				ObjectAbstractNode *obj = (ObjectAbstractNode*)(*i).get();
				switch(obj->id)
				{
				case ID_TARGET:
					{
						CompositionTargetPass *target = mTechnique->createTargetPass();
						CompositionTargetPassTranslator translator(target);
						Translator::translate(&translator, *i, getCompiler());
					}
					break;
				case ID_TARGET_OUTPUT:
					{
						CompositionTargetPassTranslator translator(mTechnique->getOutputTargetPass());
						Translator::translate(&translator, *i, getCompiler());
					}
					break;
				default:
					Translator::translate(0, *i, getCompiler());
				}
			}
			else if((*i)->type == ANT_PROPERTY)
			{
				Translator::translate(this, *i, getCompiler());
			}
		}
	}

	void ScriptCompiler::CompositionTechniqueTranslator::processProperty(PropertyAbstractNode *prop)
	{
		switch(prop->id)
		{
		case ID_TEXTURE:
			size_t atomIndex = 1;

			AbstractNodeList::const_iterator i = getNodeAt(prop->values, 0);

			if((*i)->type != ANT_ATOM)
			{
				PROP_ERROR(CE_INVALIDPARAMETERS);
				return;
			}
			// Save the first atom, should be name
			AtomAbstractNode *atom0 = (AtomAbstractNode*)(*i).get();

			size_t width = 0, height = 0;
			float widthFactor = 1.0f, heightFactor = 1.0f;
			bool widthSet = false, heightSet = false, formatSet = false;
			Ogre::PixelFormatList formats;

			while (atomIndex < prop->values.size())
			{
				i = getNodeAt(prop->values, atomIndex++);
				if((*i)->type != ANT_ATOM)
				{
					PROP_ERROR(CE_INVALIDPARAMETERS);
					return;
				}
				AtomAbstractNode *atom = (AtomAbstractNode*)(*i).get();

				switch(atom->id)
				{
				case ID_TARGET_WIDTH:
					width = 0;
					widthSet = true;
					break;
				case ID_TARGET_HEIGHT:
					height = 0;
					heightSet = true;
					break;
				case ID_TARGET_WIDTH_SCALED:
				case ID_TARGET_HEIGHT_SCALED:
					{
						bool *pSetFlag;
						size_t *pSize;
						float *pFactor;
						if (atom->id == ID_TARGET_WIDTH_SCALED)
						{
							pSetFlag = &widthSet;
							pSize = &width;
							pFactor = &widthFactor;
						}
						else
						{
							pSetFlag = &heightSet;
							pSize = &height;
							pFactor = &heightFactor;
						}
						// advance to next to get scaling
						i = getNodeAt(prop->values, atomIndex++);
						if((*i)->type != ANT_ATOM)
						{
							PROP_ERROR(CE_INVALIDPARAMETERS);
							return;
						}
						atom = (AtomAbstractNode*)(*i).get();
						if (!atom->isNumber())
						{
							PROP_ERROR(CE_INVALIDPARAMETERS);
							return;
						}

						*pSize = 0;
						*pFactor = atom->getNumber();
						*pSetFlag = true;
					}
					break;
				default:
					if (atom->isNumber())
					{
						if (atomIndex == 2)
						{
							width = atom->getNumber();
							widthSet = true;
						}
						else if (atomIndex == 3)
						{
							height = atom->getNumber();
							heightSet = true;
						}
						else
						{
							PROP_ERROR(CE_INVALIDPARAMETERS);
							return;
						}
					}
					else
					{
						// pixel format?
						PixelFormat format = PixelUtil::getFormatFromName(atom->value, true);
						if (format == PF_UNKNOWN)
						{
							PROP_ERROR(CE_INVALIDPARAMETERS);
							return;
						}
						formats.push_back(format);
						formatSet = true;
					}

				}
			}
			if (!widthSet || !heightSet || !formatSet)
			{
				getCompiler()->addError(CE_STRINGEXPECTED, prop->file, prop->line);
				return;
			}
	

			// No errors, create
			String name = atom0->value;
			if(getCompilerListener())
				getCompilerListener()->getTextureNames(&name, 1);
			CompositionTechnique::TextureDefinition *def = mTechnique->createTextureDefinition(name);
			def->width = width;
			def->height = height;
			def->widthFactor = widthFactor;
			def->heightFactor = heightFactor;
			def->formatList = formats;

			break;
		}
	}

	// CompositionTargetPassTranslator
	ScriptCompiler::CompositionTargetPassTranslator::CompositionTargetPassTranslator(CompositionTargetPass *target)
		:mTarget(target)
	{
	}

	void ScriptCompiler::CompositionTargetPassTranslator::processObject(ObjectAbstractNode *obj)
	{
		obj->context = Any(mTarget);

		if(!obj->name.empty())
		{
			String name = obj->name;
			if(getCompilerListener())
				getCompilerListener()->getTextureNames(&name, 1);
			mTarget->setOutputName(name);
		}

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				ObjectAbstractNode *obj = (ObjectAbstractNode*)(*i).get();
				switch(obj->id)
				{
				case ID_PASS:
					{
						CompositionPassTranslator translator(mTarget->createPass());
						Translator::translate(&translator, *i, getCompiler());
					}
					break;
				default:
					Translator::translate(0, *i, getCompiler());
				}
			}
			else if((*i)->type == ANT_PROPERTY)
			{
				Translator::translate(this, *i, getCompiler());
			}
		}
	}

	void ScriptCompiler::CompositionTargetPassTranslator::processProperty(PropertyAbstractNode *prop)
	{
		switch(prop->id)
		{
		case ID_INPUT:
			if(prop->values.empty())
			{
				PROP_ERROR(CE_STRINGEXPECTED);
				return;
			}
			else if (prop->values.size() > 1)
			{
				PROP_ERROR(CE_FEWERPARAMETERSEXPECTED);
				return;
			}
			else
			{
				if(prop->values.front()->type == ANT_ATOM)
				{
					AtomAbstractNode *atom = (AtomAbstractNode*)prop->values.front().get();
					switch(atom->id)
					{
					case ID_NONE:
						mTarget->setInputMode(CompositionTargetPass::IM_NONE);
						break;
					case ID_PREVIOUS:
						mTarget->setInputMode(CompositionTargetPass::IM_PREVIOUS);
						break;
					default:
						PROP_ERROR(CE_INVALIDPARAMETERS);
					}
				}
				else
				{
					PROP_ERROR(CE_INVALIDPARAMETERS);
				}
			}
			break;
		case ID_ONLY_INITIAL:
			if(prop->values.empty())
			{
				PROP_ERROR(CE_STRINGEXPECTED);
				return;
			}
			else if (prop->values.size() > 1)
			{
				PROP_ERROR(CE_FEWERPARAMETERSEXPECTED);
				return;
			}
			else
			{
				bool val;
				if(getBoolean(prop->values.front(), &val))
				{
					mTarget->setOnlyInitial(val);
				}
				else
				{
					PROP_ERROR(CE_INVALIDPARAMETERS);
				}
			}
			break;
		case ID_VISIBILITY_MASK:
			if(prop->values.empty())
			{
				PROP_ERROR(CE_STRINGEXPECTED);
				return;
			}
			else if (prop->values.size() > 1)
			{
				PROP_ERROR(CE_FEWERPARAMETERSEXPECTED);
				return;
			}
			else
			{
				Real val;
				if(getNumber(prop->values.front(), &val))
				{
					mTarget->setVisibilityMask(val);
				}
				else
				{
					PROP_ERROR(CE_INVALIDPARAMETERS);
				}
			}
			break;
		case ID_LOD_BIAS:
			if(prop->values.empty())
			{
				PROP_ERROR(CE_STRINGEXPECTED);
				return;
			}
			else if (prop->values.size() > 1)
			{
				PROP_ERROR(CE_FEWERPARAMETERSEXPECTED);
				return;
			}
			else
			{
				Real val;
				if(getNumber(prop->values.front(), &val))
				{
					mTarget->setLodBias(val);
				}
				else
				{
					PROP_ERROR(CE_INVALIDPARAMETERS);
				}
			}
			break;
		case ID_MATERIAL_SCHEME:
			if(prop->values.empty())
			{
				PROP_ERROR(CE_STRINGEXPECTED);
				return;
			}
			else if (prop->values.size() > 1)
			{
				PROP_ERROR(CE_FEWERPARAMETERSEXPECTED);
				return;
			}
			else
			{
				String val;
				if(getString(prop->values.front(), &val))
				{
					mTarget->setMaterialScheme(val);
				}
				else
				{
					PROP_ERROR(CE_INVALIDPARAMETERS);
				}
			}
			break;
		}
	}

	// CompositionPassTranslator
	ScriptCompiler::CompositionPassTranslator::CompositionPassTranslator(CompositionPass *pass)
		:mPass(pass)
	{
	}

	void ScriptCompiler::CompositionPassTranslator::processObject(ObjectAbstractNode *obj)
	{
		obj->context = Any(mPass);

		// The name is the type of the pass
		if(obj->name.empty())
		{
			OBJ_ERROR(CE_OBJECTNAMEEXPECTED);
			return;
		}

		if(obj->name == "clear")
			mPass->setType(CompositionPass::PT_CLEAR);
		else if(obj->name == "stencil")
			mPass->setType(CompositionPass::PT_STENCIL);
		else if(obj->name == "render_quad")
			mPass->setType(CompositionPass::PT_RENDERQUAD);
		else if(obj->name == "render_scene")
			mPass->setType(CompositionPass::PT_RENDERSCENE);
		else
		{
			OBJ_ERROR(CE_INVALIDPARAMETERS);
			return;
		}

		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_OBJECT)
			{
				Translator::translate(0, *i, getCompiler());
			}
			else if((*i)->type == ANT_PROPERTY)
			{
				Translator::translate(this, *i, getCompiler());
			}
		}
	}

	void ScriptCompiler::CompositionPassTranslator::processProperty(PropertyAbstractNode *prop)
	{
		switch(prop->id)
		{
		case ID_MATERIAL:
			if(prop->values.empty())
			{
				PROP_ERROR(CE_STRINGEXPECTED);
				return;
			}
			else if (prop->values.size() > 1)
			{
				PROP_ERROR(CE_FEWERPARAMETERSEXPECTED);
				return;
			}
			else
			{
				String val;
				if(getString(prop->values.front(), &val))
				{
					if(getCompilerListener())
						getCompilerListener()->getMaterialName(&val);
					mPass->setMaterialName(val);
				}
				else
				{
					PROP_ERROR(CE_INVALIDPARAMETERS);
				}
			}
			break;
		case ID_INPUT:
			if(prop->values.size() < 2)
			{
				PROP_ERROR(CE_STRINGEXPECTED);
				return;
			}
			else if (prop->values.size() > 3)
			{
				PROP_ERROR(CE_FEWERPARAMETERSEXPECTED);
				return;
			}
			else
			{
				AbstractNodeList::const_iterator i0 = getNodeAt(prop->values, 0), i1 = getNodeAt(prop->values, 1), i2 = getNodeAt(prop->values, 2);
				Real id;
				String name;
				if(getNumber(*i0, &id) && getString(*i1, &name))
				{
					Real index = 0;
					if(i2 != prop->values.end())
					{
						if(!getNumber(*i2, &index))
						{
							PROP_ERROR(CE_NUMBEREXPECTED);
							return;
						}
					}
					
					if(getCompilerListener())
						getCompilerListener()->getTextureNames(&name, 1);
					mPass->setInput(id, name, index);
				}
				else
				{
					PROP_ERROR(CE_INVALIDPARAMETERS);
				}
			}
			break;
		case ID_IDENTIFIER:
			if(prop->values.empty())
			{
				PROP_ERROR(CE_STRINGEXPECTED);
				return;
			}
			else if (prop->values.size() > 1)
			{
				PROP_ERROR(CE_FEWERPARAMETERSEXPECTED);
				return;
			}
			else
			{
				Real val;
				if(getNumber(prop->values.front(), &val))
				{
					mPass->setIdentifier(val);
				}
				else
				{
					PROP_ERROR(CE_INVALIDPARAMETERS);
				}
			}
			break;
		case ID_FIRST_RENDER_QUEUE:
			if(prop->values.empty())
			{
				PROP_ERROR(CE_STRINGEXPECTED);
				return;
			}
			else if (prop->values.size() > 1)
			{
				PROP_ERROR(CE_FEWERPARAMETERSEXPECTED);
				return;
			}
			else
			{
				Real val;
				if(getNumber(prop->values.front(), &val))
				{
					mPass->setFirstRenderQueue(val);
				}
				else
				{
					PROP_ERROR(CE_INVALIDPARAMETERS);
				}
			}
			break;
		case ID_LAST_RENDER_QUEUE:
			if(prop->values.empty())
			{
				PROP_ERROR(CE_STRINGEXPECTED);
				return;
			}
			else if (prop->values.size() > 1)
			{
				PROP_ERROR(CE_FEWERPARAMETERSEXPECTED);
				return;
			}
			else
			{
				Real val;
				if(getNumber(prop->values.front(), &val))
				{
					mPass->setLastRenderQueue(val);
				}
				else
				{
					PROP_ERROR(CE_INVALIDPARAMETERS);
				}
			}
			break;
		case ID_BUFFERS:
			{
				uint32 buffers = 0;
				for(AbstractNodeList::iterator k = prop->values.begin(); k != prop->values.end(); ++k)
				{
					if((*k)->type == ANT_ATOM)
					{
						switch(((AtomAbstractNode*)(*k).get())->id)
						{
						case ID_COLOUR:
							buffers |= FBT_COLOUR;
							break;
						case ID_DEPTH:
							buffers |= FBT_DEPTH;
							break;
						case ID_STENCIL:
							buffers |= FBT_STENCIL;
							break;
						default:
							PROP_ERROR(CE_INVALIDPARAMETERS);
						}
					}
					else
						PROP_ERROR(CE_INVALIDPARAMETERS);
				}
				mPass->setClearBuffers(buffers);
			}
			break;
		case ID_COLOUR_VALUE:
			{
				if(prop->values.empty())
				{
					PROP_ERROR(CE_NUMBEREXPECTED);
					return;
				}
				ColourValue val;
				if(getColour(prop->values.begin(), prop->values.end(), &val))
					mPass->setClearColour(val);
				else
					PROP_ERROR(CE_INVALIDPARAMETERS);
			}
			break;
		case ID_DEPTH_VALUE:
			{
				if(prop->values.empty())
				{
					PROP_ERROR(CE_NUMBEREXPECTED);
					return;
				}
				Real val;
				if(getNumber(prop->values.front(), &val))
					mPass->setClearDepth(val);
				else
					PROP_ERROR(CE_INVALIDPARAMETERS);
			}
			break;
		case ID_STENCIL_VALUE:
			{
				if(prop->values.empty())
				{
					PROP_ERROR(CE_NUMBEREXPECTED);
					return;
				}
				Real val;
				if(getNumber(prop->values.front(), &val))
					mPass->setClearStencil(val);
				else
					PROP_ERROR(CE_INVALIDPARAMETERS);
			}
			break;
		case ID_CHECK:
			{
				if(prop->values.empty())
				{
					PROP_ERROR(CE_STRINGEXPECTED);
					return;
				}
				bool val;
				if(getBoolean(prop->values.front(), &val))
					mPass->setStencilCheck(val);
				else
					PROP_ERROR(CE_INVALIDPARAMETERS);
			}
			break;
		case ID_COMP_FUNC:
			{
				if(prop->values.empty())
				{
					PROP_ERROR(CE_STRINGEXPECTED);
					return;
				}
				CompareFunction func;
				if(getCompareFunction(prop->values.front(), &func))
					mPass->setStencilFunc(func);
				else
					PROP_ERROR(CE_INVALIDPARAMETERS);
			}
			break;
		case ID_REF_VALUE:
			{
				if(prop->values.empty())
				{
					PROP_ERROR(CE_NUMBEREXPECTED);
					return;
				}
				Real val;
				if(getNumber(prop->values.front(), &val))
					mPass->setStencilRefValue(val);
				else
					PROP_ERROR(CE_INVALIDPARAMETERS);
			}
			break;
		case ID_MASK:
			{
				if(prop->values.empty())
				{
					PROP_ERROR(CE_NUMBEREXPECTED);
					return;
				}
				Real val;
				if(getNumber(prop->values.front(), &val))
					mPass->setStencilMask(val);
				else
					PROP_ERROR(CE_INVALIDPARAMETERS);
			}
			break;
		case ID_FAIL_OP:
			{
				if(prop->values.empty())
				{
					PROP_ERROR(CE_STRINGEXPECTED);
					return;
				}
				StencilOperation val;
				if(getStencilOp(prop->values.front(), &val))
					mPass->setStencilFailOp(val);
				else
					PROP_ERROR(CE_INVALIDPARAMETERS);
			}
			break;
		case ID_DEPTH_FAIL_OP:
			{
				if(prop->values.empty())
				{
					PROP_ERROR(CE_STRINGEXPECTED);
					return;
				}
				StencilOperation val;
				if(getStencilOp(prop->values.front(), &val))
					mPass->setStencilDepthFailOp(val);
				else
					PROP_ERROR(CE_INVALIDPARAMETERS);
			}
			break;
		case ID_PASS_OP:
			{
				if(prop->values.empty())
				{
					PROP_ERROR(CE_STRINGEXPECTED);
					return;
				}
				StencilOperation val;
				if(getStencilOp(prop->values.front(), &val))
					mPass->setStencilPassOp(val);
				else
					PROP_ERROR(CE_INVALIDPARAMETERS);
			}
			break;
		case ID_TWO_SIDED:
			{
				if(prop->values.empty())
				{
					PROP_ERROR(CE_STRINGEXPECTED);
					return;
				}
				bool val;
				if(getBoolean(prop->values.front(), &val))
					mPass->setStencilTwoSidedOperation(val);
				else
					PROP_ERROR(CE_INVALIDPARAMETERS);
			}
			break;
		}
	}

	// ScriptCompilerManager
	template<> ScriptCompilerManager *Singleton<ScriptCompilerManager>::ms_Singleton = 0;
	
	ScriptCompilerManager* ScriptCompilerManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
	//-----------------------------------------------------------------------
    ScriptCompilerManager& ScriptCompilerManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
	//-----------------------------------------------------------------------
	ScriptCompilerManager::ScriptCompilerManager()
		:mListener(0)
	{
		OGRE_LOCK_AUTO_MUTEX
        mScriptPatterns.push_back("*.os");
#if OGRE_USE_NEW_COMPILERS == 1
		mScriptPatterns.push_back("*.program");
		mScriptPatterns.push_back("*.material");
		mScriptPatterns.push_back("*.particle");
		mScriptPatterns.push_back("*.compositor");
#endif
		ResourceGroupManager::getSingleton()._registerScriptLoader(this);

		OGRE_THREAD_POINTER_SET(mScriptCompiler, new ScriptCompiler());
	}
	//-----------------------------------------------------------------------
	ScriptCompilerManager::~ScriptCompilerManager()
	{
		OGRE_THREAD_POINTER_DELETE(mScriptCompiler);
	}
	//-----------------------------------------------------------------------
	void ScriptCompilerManager::setListener(ScriptCompilerListener *listener)
	{
		OGRE_LOCK_AUTO_MUTEX
		mListener = listener;
	}
	//-----------------------------------------------------------------------
	ScriptCompilerListener *ScriptCompilerManager::getListener()
	{
		return mListener;
	}
	//-----------------------------------------------------------------------
    const StringVector& ScriptCompilerManager::getScriptPatterns(void) const
    {
        return mScriptPatterns;
    }
    //-----------------------------------------------------------------------
    Real ScriptCompilerManager::getLoadingOrder(void) const
    {
        /// Load relatively early, before most script loaders run
        return 90.0f;
    }
    //-----------------------------------------------------------------------
    void ScriptCompilerManager::parseScript(DataStreamPtr& stream, const String& groupName)
    {
#if OGRE_THREAD_SUPPORT
		// check we have an instance for this thread (should always have one for main thread)
		if (!mScriptCompiler.get())
		{
			// create a new instance for this thread - will get deleted when
			// the thread dies
			mScriptCompiler.reset(new ScriptCompiler());
		}
#endif
		// Set the listener on the compiler before we continue
		{
			OGRE_LOCK_AUTO_MUTEX
			mScriptCompiler->setListener(mListener);
		}
        mScriptCompiler->compile(stream->getAsString(), stream->getName(), groupName);
    }
}


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

	void ScriptCompilerListener::preASTConversion(ConcreteNodeListPtr nodes, ScriptCompiler::IdMap *ids)
	{
		
	}

	std::pair<bool,ScriptCompiler::Translator*> ScriptCompilerListener::preTranslation(const AbstractNodePtr &node)
	{
		return std::make_pair(false, (ScriptCompiler::Translator*)0);
	}

	MaterialPtr ScriptCompilerListener::allocateMaterial(const String &name, const String &group)
	{
		return (MaterialPtr)MaterialManager::getSingleton().create(name, group);
	}

	// ScriptCompiler
	ScriptCompiler::ScriptCompiler()
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

				// Everything up until the colon is a "value" of this object
				while(iter != temp.end() && (*iter)->type != CNT_COLON)
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
	ScriptCompiler::Translator::Translator(Ogre::ScriptCompiler *compiler)
		:mCompiler(compiler)
	{
	}

	void ScriptCompiler::Translator::translate(Translator *translator, const AbstractNodePtr &node)
	{
		// First check if the compiler listener will override this node
		bool process = true;
		if(translator->mCompiler && translator->mCompiler->mListener)
		{
			std::pair<bool,ScriptCompiler::Translator*> p = translator->mCompiler->mListener->preTranslation(node);
			if(p.first && p.second)
			{
				// Call the returned translator
				p.second->process(node);
				process = false;
			}
		}

		// Call the suggested translator
		// Or ignore the node if no translator is given
		if(process && translator)
			translator->process(node);
	}

	ScriptCompiler *ScriptCompiler::Translator::getCompiler()
	{
		return mCompiler;
	}

	ScriptCompilerListener *ScriptCompiler::Translator::getCompilerListener()
	{
		return mCompiler->mListener;
	}

	bool ScriptCompiler::Translator::getBoolean(const AbstractNodePtr &node, bool *result)
	{
		if(node->type != ANT_ATOM)
		{
			mCompiler->addError(CE_INVALIDPARAMETERS, node->file, node->line);
			return false;
		}
		AtomAbstractNode *atom = (AtomAbstractNode*)node.get();
		if(atom->id != 1 || atom->id != 0)
		{
			mCompiler->addError(CE_INVALIDPARAMETERS, node->file, node->line);
			return false;
		}
		
		*result = atom->id == 1 ? true : false;
		return true;
	}

	// MaterialTranslator
	ScriptCompiler::MaterialTranslator::MaterialTranslator(ScriptCompiler *compiler)
		:Translator(compiler)
	{
	}

	void ScriptCompiler::MaterialTranslator::process(const AbstractNodePtr &node)
	{
		assert(node->type == ANT_OBJECT);

		ObjectAbstractNode *obj = (ObjectAbstractNode*)node.get();
		if(obj->name.empty())
			getCompiler()->addError(CE_OBJECTNAMEEXPECTED, obj->file, obj->line);

		// Create a material with the given name
		if(getCompilerListener())
			mMaterial = getCompilerListener()->allocateMaterial(obj->name, getCompiler()->getResourceGroup());
		else
			mMaterial = MaterialManager::getSingleton().create(obj->name, getCompiler()->getResourceGroup());

		if(mMaterial.isNull())
		{
			getCompiler()->addError(CE_OBJECTALLOCATIONERROR, obj->file, obj->line);
			return;
		}

		mMaterial->removeAllTechniques();

		// Set the properties for the material
		for(AbstractNodeList::iterator i = obj->children.begin(); i != obj->children.end(); ++i)
		{
			if((*i)->type == ANT_PROPERTY)
			{
				PropertyAbstractNode *prop = (PropertyAbstractNode*)(*i).get();
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
							getCompiler()->addError(CE_INVALIDPARAMETERS, prop->values.front()->file, prop->values.front()->line);
					}
					break;
				case ID_TRANSPARENCY_CASTS_SHADOWS:
					break;
				case ID_SET_TEXTURE_ALIAS:
					break;
				}
			}
			else if((*i)->type == ANT_OBJECT)
			{
				ObjectAbstractNode *obj = (ObjectAbstractNode*)(*i).get();
				if(obj->id == ID_TECHNIQUE)
				{
					// Compile the technique
				}
			}
		}
	}
}
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
#include "OgreMaterialScriptCompiler2.h"
#include "OgreMaterialManager.h"
#include "OgreTechnique.h"
#include "OgrePass.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"

namespace Ogre{

	// MaterialScriptCompilerListener
	MaterialScriptCompilerListener::MaterialScriptCompilerListener()
	{
	}

	bool MaterialScriptCompilerListener::processNode(ScriptNodeList::iterator &iter, ScriptNodeList::iterator &end, MaterialScriptCompiler2*)
	{
		return false;
	}

	Material *MaterialScriptCompilerListener::getMaterial(const Ogre::String &name, const Ogre::String &group)
	{
		Material *material = (Material*)MaterialManager::getSingleton().create(name, group).get();
		material->removeAllTechniques();

		return material;
	}

	GpuProgram *MaterialScriptCompilerListener::getGpuProgram(const String &name, const String &group, GpuProgramType type, const String &syntax, const String &source)
	{
		return GpuProgramManager::getSingleton().createProgram(name, group, source, type, syntax).get();
	}

	HighLevelGpuProgram *MaterialScriptCompilerListener::getHighLevelGpuProgram(const String &name, const String &group, GpuProgramType type, const String &language)
	{
		return HighLevelGpuProgramManager::getSingleton().createProgram(name, group, language, type).get();
	}

	void MaterialScriptCompilerListener::preApplyTextureAliases(Ogre::AliasTextureNamePairList &aliases)
	{
	}

	String MaterialScriptCompilerListener::getTexture(const Ogre::String &name)
	{
		// Return the name un-transformed
		// We trust that the texture will exist in the manager by the time it is needed
		return name;
	}

	// MaterialScriptCompiler2
	MaterialScriptCompiler2::MaterialScriptCompiler2()
		:mListener(0)
	{
		mAllowNontypedObjects = false; // All material objects must be typed

		// Set up the word ids
		mWordIDs["material"] = ID_MATERIAL;
		mWordIDs["vertex_program"] = ID_VERTEX_PROGRAM;
		mWordIDs["fragment_program"] = ID_FRAGMENT_PROGRAM;
		mWordIDs["technique"] = ID_TECHNIQUE;
		mWordIDs["pass"] = ID_PASS;
		mWordIDs["texture_unit"] = ID_TEXTURE_UNIT;
		mWordIDs["vertex_program_ref"] = ID_VERTEX_PROGRAM_REF;
		mWordIDs["fragment_program_ref"] = ID_FRAGMENT_PROGRAM_REF;
		mWordIDs["shadow_caster_vertex_program_ref"] = ID_SHADOW_CASTER_VERTEX_PROGRAM_REF;
		mWordIDs["shadow_receiver_vertex_program_ref"] = ID_SHADOW_RECEIVER_VERTEX_PROGRAM_REF;
		mWordIDs["shadow_receiver_fragment_program_ref"] = ID_SHADOW_RECEIVER_FRAGMENT_PROGRAM_REF;

		mWordIDs["lod_distances"] = ID_LOD_DISTANCES;
		mWordIDs["receive_shadows"] = ID_RECEIVE_SHADOWS;
		mWordIDs["transparency_casts_shadows"] = ID_TRANSPARENCY_CASTS_SHADOWS;
		mWordIDs["set_texture_alias"] = ID_SET_TEXTURE_ALIAS;

		mWordIDs["source"] = ID_SOURCE;
		mWordIDs["syntax"] = ID_SYNTAX;
		mWordIDs["default_params"] = ID_DEFAULT_PARAMS;
		mWordIDs["param_indexed"] = ID_PARAM_INDEXED;
		mWordIDs["param_named"] = ID_PARAM_NAMED;
		mWordIDs["param_indexed_auto"] = ID_PARAM_INDEXED_AUTO;
		mWordIDs["param_named_auto"] = ID_PARAM_NAMED_AUTO;

		mWordIDs["scheme"] = ID_SCHEME;
		mWordIDs["lod_index"] = ID_LOD_INDEX;

		mWordIDs["ambient"] = ID_AMBIENT;
		mWordIDs["diffuse"] = ID_DIFFUSE;
		mWordIDs["specular"] = ID_SPECULAR;
		mWordIDs["emissive"] = ID_EMISSIVE;
			mWordIDs["vertex_colour"] = ID_VERTEX_COLOUR;
		mWordIDs["scene_blend"] = ID_SCENE_BLEND;
		mWordIDs["colour_blend"] = ID_COLOUR_BLEND;
			mWordIDs["one"] = ID_ONE;
			mWordIDs["zero"] = ID_ZERO;
			mWordIDs["dest_colour"] = ID_DEST_COLOUR;
			mWordIDs["src_colour"] = ID_SRC_COLOUR;
			mWordIDs["one_minus_src_colour"] = ID_ONE_MINUS_SRC_COLOUR;
			mWordIDs["one_minus_dest_colour"] = ID_ONE_MINUS_DEST_COLOUR;
			mWordIDs["dest_alpha"] = ID_DEST_ALPHA;
			mWordIDs["src_alpha"] = ID_SRC_ALPHA;
			mWordIDs["one_minus_dest_alpha"] = ID_ONE_MINUS_DEST_ALPHA;
			mWordIDs["one_minus_src_alpha"] = ID_ONE_MINUS_SRC_ALPHA;
		mWordIDs["separate_scene_blend"] = ID_SEPARATE_SCENE_BLEND;
		mWordIDs["depth_check"] = ID_DEPTH_CHECK;
		mWordIDs["depth_write"] = ID_DEPTH_WRITE;
		mWordIDs["depth_func"] = ID_DEPTH_FUNC;
		mWordIDs["depth_bias"] = ID_DEPTH_BIAS;
		mWordIDs["iteration_depth_bias"] = ID_ITERATION_DEPTH_BIAS;
			mWordIDs["always_fail"] = ID_ALWAYS_FAIL;
			mWordIDs["always_pass"] = ID_ALWAYS_PASS;
			mWordIDs["less_equal"] = ID_LESS_EQUAL;
			mWordIDs["less"] = ID_LESS;
			mWordIDs["equal"] = ID_EQUAL;
			mWordIDs["not_equal"] = ID_NOT_EQUAL;
			mWordIDs["greater_equal"] = ID_GREATER_EQUAL;
			mWordIDs["greater"] = ID_GREATER;
		mWordIDs["alpha_rejection"] = ID_ALPHA_REJECTION;
		mWordIDs["light_scissor"] = ID_LIGHT_SCISSOR;
		mWordIDs["light_clip_planes"] = ID_LIGHT_CLIP_PLANES;
		mWordIDs["illumination_stage"] = ID_ILLUMINATION_STAGE;
			mWordIDs["decal"] = ID_DECAL;
		mWordIDs["cull_hardware"] = ID_CULL_HARDWARE;
			mWordIDs["clockwise"] = ID_CLOCKWISE;
			mWordIDs["anticlockwise"] = ID_ANTICLOCKWISE;
		mWordIDs["cull_software"] = ID_CULL_SOFTWARE;
			mWordIDs["back"] = ID_BACK;
			mWordIDs["front"] = ID_FRONT;
		mWordIDs["normalise_normals"] = ID_NORMALISE_NORMALS;
		mWordIDs["lighting"] = ID_LIGHTING;
		mWordIDs["shading"] = ID_SHADING;
			mWordIDs["flat"] = ID_FLAT;
			mWordIDs["gouraud"] = ID_GOURAUD;
			mWordIDs["phong"] = ID_PHONG;
		mWordIDs["polygon_mode"] = ID_POLYGON_MODE;
		mWordIDs["polygon_mode_overrideable"] = ID_POLYGON_MODE_OVERRIDEABLE;
		mWordIDs["fog_override"] = ID_FOG_OVERRIDE;
			mWordIDs["none"] = ID_NONE;
			mWordIDs["linear"] = ID_LINEAR;
			mWordIDs["exp"] = ID_EXP;
			mWordIDs["exp2"] = ID_EXP2;
		mWordIDs["colour_write"] = ID_COLOUR_WRITE;
		mWordIDs["max_lights"] = ID_MAX_LIGHTS;
		mWordIDs["start_light"] = ID_START_LIGHT;
		mWordIDs["iteration"] = ID_ITERATION;
			mWordIDs["once"] = ID_ONCE;
			mWordIDs["once_per_light"] = ID_ONCE_PER_LIGHT;
			mWordIDs["per_n_lights"] = ID_PER_N_LIGHTS;
			mWordIDs["per_light"] = ID_PER_LIGHT;
			mWordIDs["point"] = ID_POINT;
			mWordIDs["spot"] = ID_SPOT;
			mWordIDs["directional"] = ID_DIRECTIONAL;
		mWordIDs["point_size"] = ID_POINT_SIZE;
		mWordIDs["point_sprites"] = ID_POINT_SPRITES;
		mWordIDs["point_size_min"] = ID_POINT_SIZE_MIN;
		mWordIDs["point_size_max"] = ID_POINT_SIZE_MAX;
	}

	void MaterialScriptCompiler2::setListener(MaterialScriptCompilerListener *listener)
	{
		mListener = listener;
	}

	bool MaterialScriptCompiler2::compileImpl(ScriptNodeListPtr nodes)
	{
		ScriptNodeList::iterator i = nodes->begin();
		while(i != nodes->end())
		{
			// Delegate some processing to the listener
			if(!processNode(i, nodes->end()))
			{
				if((*i)->token != "abstract")
				{
					if((*i)->wordID == ID_MATERIAL)
						compileMaterial(*i);
				}
				++i;
			}
		}
		return mErrors.empty();
	}

	bool MaterialScriptCompiler2::processNode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		if(mListener)
			return mListener->processNode(i, end, this);
		return false;
	}

	ScriptNodeListPtr MaterialScriptCompiler2::loadImportPath(const Ogre::String &name)
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

	void MaterialScriptCompiler2::compileMaterial(const ScriptNodePtr &node)
	{
		// Up until the '{' combine tokens to produce the name
		if(node->children.empty())
		{
			addError(CE_STRINGEXPECTED, node->file, node->line, -1);
			return;
		}
		ScriptNodeList::iterator i = node->children.begin();
		if((*i)->token.empty())
		{
			addError(CE_STRINGEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		// Allocate the material
		if(mListener)
			mMaterial = mListener->getMaterial((*i)->token, mGroup);
		else
			mMaterial = (Material*)MaterialManager::getSingleton().create((*i)->token, mGroup).get();
		if(!mMaterial)
		{
			addError(CE_OBJECTALLOCATIONERROR, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		mMaterial->removeAllTechniques();

		// The material doesn't support any more options in the header, so skip ahead to '{'
		i = findNode(i, node->children.end(), SNT_LBRACE);
		if(i == node->children.end())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				if((*j)->wordID == ID_LOD_DISTANCES)
				{
					// Collect the children into the lod list
					Material::LodDistanceList lods;
					for(ScriptNodeList::iterator k = (*j)->children.begin(); k != (*j)->children.end(); ++k)
					{
						if((*k)->type == SNT_NUMBER)
							lods.push_back((*k)->data);
						else
							addError(CE_NUMBEREXPECTED, (*k)->file, (*k)->line, (*k)->column);
					}
					mMaterial->setLodLevels(lods);
				}
				else if((*j)->wordID == ID_RECEIVE_SHADOWS)
				{
					if(!(*j)->children.empty())
						mMaterial->setReceiveShadows(isTruthValue((*j)->children.front()->token));
				}
				else if((*j)->wordID == ID_TRANSPARENCY_CASTS_SHADOWS)
				{
					if(!(*j)->children.empty())
						mMaterial->setTransparencyCastsShadows(isTruthValue((*j)->children.front()->token));
				}
				else if((*j)->wordID == ID_SET_TEXTURE_ALIAS)
				{
					ScriptNodePtr name = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
						value = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0);
					bool isValid = true;
					if(name.isNull() || value.isNull())
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
						isValid = false;
					}
					if(isValid && name->token.empty())
					{
						addError(CE_INVALIDPROPERTYVALUE, name->file, name->line, name->column);
						isValid = false;
					}
					if(isValid && value->token.empty())
					{
						addError(CE_INVALIDPROPERTYVALUE, value->file, value->line, value->column);
						isValid = false;
					}
					if(isValid)
						mTextureAliases[name->token] = value->token;
				}
				else if((*j)->wordID == ID_TECHNIQUE)
				{
					compileTechnique(*j);
				}
				++j;
			}
		}

		// Set the texture aliases
		if(mListener)
			mListener->preApplyTextureAliases(mTextureAliases);
		mMaterial->applyTextureAliases(mTextureAliases);

		mMaterial = 0;
		mTextureAliases.clear();
	}

	void MaterialScriptCompiler2::compileTechnique(const ScriptNodePtr &node)
	{
		if(node->children.empty())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}

		// Create the technique
		Technique *technique = mMaterial->createTechnique();

		// The first child is possibly the optional name of the technique
		ScriptNodeList::iterator i = node->children.begin();
		if((*i)->type != SNT_LBRACE)
		{
			if(!(*i)->token.empty())
				technique->setName((*i)->token);

			// Find the '{'
			i = findNode(i, node->children.end(), SNT_LBRACE);
		}

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				if((*j)->wordID == ID_SCHEME)
				{
					if(!(*j)->children.empty() && !(*j)->children.front()->token.empty())
						technique->setSchemeName((*j)->children.front()->token);
				}
				else if((*j)->wordID == ID_LOD_INDEX)
				{
					if(!(*j)->children.empty() && (*j)->children.front()->type == SNT_NUMBER)
						technique->setLodIndex((*j)->children.front()->data);
					else
						addError(CE_INVALIDPROPERTYVALUE, (*j)->file, (*j)->line, -1);
				}
				else if((*j)->wordID == ID_PASS)
				{
					compilePass(*j, technique);
				}
				++j;
			}
		}
	}

	void MaterialScriptCompiler2::compilePass(const ScriptNodePtr &node, Technique *technique)
	{
		if(node->children.empty())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}

		// Create the pass
		Pass *pass = technique->createPass();

		// The first child is possibly the optional name of the technique
		ScriptNodeList::iterator i = node->children.begin();
		if((*i)->type != SNT_LBRACE)
		{
			if(!(*i)->token.empty())
				pass->setName((*i)->token);

			// Find the '{'
			i = findNode(i, node->children.end(), SNT_LBRACE);
		}

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				if((*j)->wordID == ID_AMBIENT)
				{
					if(!(*j)->children.empty())
					{
						ColourValue c;
						if((*j)->children.front()->wordID == ID_VERTEX_COLOUR)
							pass->setVertexColourTracking(Ogre::TVC_AMBIENT);
						else if(getColourValue((*j)->children.begin(), (*j)->children.end(), c))
							pass->setAmbient(c);
					}
				}
				else if((*j)->wordID == ID_DIFFUSE)
				{
					if(!(*j)->children.empty())
					{
						ColourValue c;
						if((*j)->children.front()->wordID == ID_VERTEX_COLOUR)
							pass->setVertexColourTracking(Ogre::TVC_DIFFUSE);
						else if(getColourValue((*j)->children.begin(), (*j)->children.end(), c))
							pass->setDiffuse(c);
					}
				}
				else if((*j)->wordID == ID_SPECULAR)
				{
					if(!(*j)->children.empty())
					{
						ColourValue c;
						if((*j)->children.front()->wordID == ID_VERTEX_COLOUR)
							pass->setVertexColourTracking(Ogre::TVC_SPECULAR);
						else if(getColourValue((*j)->children.begin(), (*j)->children.end(), c))
							pass->setSpecular(c);
					}
				}
				else if((*j)->wordID == ID_EMISSIVE)
				{
					if(!(*j)->children.empty())
					{
						ColourValue c;
						if((*j)->children.front()->wordID == ID_VERTEX_COLOUR)
							pass->setVertexColourTracking(Ogre::TVC_EMISSIVE);
						else if(getColourValue((*j)->children.begin(), (*j)->children.end(), c))
							pass->setSelfIllumination(c);
					}
				}
				else if((*j)->wordID == ID_SCENE_BLEND)
				{
					// We can use the 1 argument kind, or the 2 argument kind
					if((*j)->children.size() == 1)
					{
						ScriptNodeList::iterator k = (*j)->children.begin();
						switch((*k)->wordID)
						{
						case ID_ADD:
							pass->setSceneBlending(Ogre::SBT_ADD);
							break;
						case ID_MODULATE:
							pass->setSceneBlending(SBT_MODULATE);
							break;
						case ID_COLOUR_BLEND:
							pass->setSceneBlending(SBT_TRANSPARENT_COLOUR);
							break;
						case ID_ALPHA_BLEND:
							pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
							break;
						default:
							addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
						}
					}
					else if((*j)->children.size() == 2)
					{
						SceneBlendFactor sbf1, sbf2;
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1);
						bool isValid = true;
						
						if(!getBlendFactor(node1, sbf1))
						{
							addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
							isValid = false;
						}
						if(!getBlendFactor(node2, sbf2))
						{
							addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
							isValid = false;
						}
						if(isValid)
							pass->setSceneBlending(sbf1, sbf2);
					}
					else
						addError(CE_INVALIDPROPERTYVALUE, (*j)->file, (*j)->line, -1);
				}
				else if((*j)->wordID = ID_SEPARATE_SCENE_BLEND)
				{
					// We can use 2 or 4 argument varieties
					if((*j)->children.size() == 2)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1);
						SceneBlendType sbt, sbta;
						bool isValid = true;
						switch(node1->wordID)
						{
						case ID_ADD:
							sbt = SBT_ADD;
							break;
						case ID_MODULATE:
							sbt = SBT_MODULATE;
							break;
						case ID_COLOUR_BLEND:
							sbt = SBT_TRANSPARENT_COLOUR;
							break;
						case ID_ALPHA_BLEND:
							sbt = SBT_TRANSPARENT_ALPHA;
							break;
						default:
							addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
							isValid = false;
						}

						switch(node2->wordID)
						{
						case ID_ADD:
							sbta = SBT_ADD;
							break;
						case ID_MODULATE:
							sbta = SBT_MODULATE;
							break;
						case ID_COLOUR_BLEND:
							sbta = SBT_TRANSPARENT_COLOUR;
							break;
						case ID_ALPHA_BLEND:
							sbta = SBT_TRANSPARENT_ALPHA;
							break;
						default:
							addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
							isValid = false;
						}
						
						if(isValid)
							pass->setSeparateSceneBlending(sbt, sbta);
					}
					else if((*j)->children.size() == 4)
					{
						SceneBlendFactor sbf1, sbf2, sbfa1, sbfa2;
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1),
							node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2),
							node4 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 3);
						bool isValid = true;
						
						if(!getBlendFactor(node1, sbf1))
						{
							addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
							isValid = false;
						}
						if(!getBlendFactor(node2, sbf2))
						{
							addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
							isValid = false;
						}
						if(!getBlendFactor(node3, sbfa1))
						{
							addError(CE_INVALIDPROPERTYVALUE, node3->file, node3->line, node3->column);
							isValid = false;
						}
						if(!getBlendFactor(node4, sbfa2))
						{
							addError(CE_INVALIDPROPERTYVALUE, node4->file, node4->line, node4->column);
							isValid = false;
						}
						if(isValid)
							pass->setSeparateSceneBlending(sbf1, sbf2, sbfa1, sbfa2);
					}
					else
						addError(CE_INVALIDPROPERTYVALUE, (*j)->file, (*j)->line, -1);
				}
				else if((*j)->wordID == ID_DEPTH_CHECK)
				{
					if(!(*j)->children.empty())
						pass->setDepthCheckEnabled(isTruthValue((*j)->children.front()->token));
				}
				else if((*j)->wordID == ID_DEPTH_WRITE)
				{
					if(!(*j)->children.empty())
						pass->setDepthWriteEnabled(isTruthValue((*j)->children.front()->token));
				}
				else if((*j)->wordID == ID_DEPTH_FUNC)
				{
					ScriptNodePtr node = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0);
					CompareFunction func;
					if(!node.isNull())
					{
						if(getCompareFunction(node, func))
							pass->setDepthFunction(func);
						else
							addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
						
				}
				else if((*j)->wordID == ID_DEPTH_BIAS)
				{
					ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
						node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1);
					if(!node1.isNull() && node1->type == SNT_NUMBER)
					{
						if(!node2.isNull() && node2->type == SNT_NUMBER)
							pass->setDepthBias(node1->data, node2->data);
						else
							pass->setDepthBias(node1->data);
					}
					else
					{
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_ITERATION_DEPTH_BIAS)
				{
					ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0);
					if(!node1.isNull() && node1->type == SNT_NUMBER)
					{
						pass->setIterationDepthBias(node1->data);
					}
					else
					{
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_ALPHA_REJECTION)
				{
					ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
						node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1);
					if(!node1.isNull())
					{
						if(!node2.isNull() && node2->type == SNT_NUMBER)
						{
							CompareFunction func;
							if(getCompareFunction(node1, func))
							{
								pass->setAlphaRejectSettings(func, node2->data);
							}
							else
							{
								addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
							}
						}
						else
						{
							addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_LIGHT_SCISSOR)
				{
					if(!(*j)->children.empty())
						pass->setLightScissoringEnabled(isTruthValue((*j)->children.front()->token));
				}
				else if((*j)->wordID == ID_LIGHT_CLIP_PLANES)
				{
					if(!(*j)->children.empty())
						pass->setLightClipPlanesEnabled(isTruthValue((*j)->children.front()->token));
				}
				else if((*j)->wordID == ID_ILLUMINATION_STAGE)
				{
					if(!(*j)->children.empty())
					{
						ScriptNodeList::iterator k = (*j)->children.begin();
						switch((*k)->wordID)
						{
						case ID_AMBIENT:
							pass->setIlluminationStage(IS_AMBIENT);
							break;
						case ID_PER_LIGHT:
							pass->setIlluminationStage(IS_PER_LIGHT);
							break;
						case ID_DECAL:
							pass->setIlluminationStage(IS_DECAL);
							break;
						default:
							addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_CULL_HARDWARE)
				{
					if(!(*j)->children.empty())
					{
						ScriptNodeList::iterator k = (*j)->children.begin();
						switch((*k)->wordID)
						{
						case ID_CLOCKWISE:
							pass->setCullingMode(Ogre::CULL_CLOCKWISE);
							break;
						case ID_ANTICLOCKWISE:
							pass->setCullingMode(CULL_ANTICLOCKWISE);
							break;
						case ID_NONE:
							pass->setCullingMode(CULL_NONE);
							break;
						default:
							addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_CULL_SOFTWARE)
				{
					if(!(*j)->children.empty())
					{
						ScriptNodeList::iterator k = (*j)->children.begin();
						switch((*k)->wordID)
						{
						case ID_FRONT:
							pass->setManualCullingMode(MANUAL_CULL_FRONT);
							break;
						case ID_ANTICLOCKWISE:
							pass->setManualCullingMode(MANUAL_CULL_BACK);
							break;
						case ID_NONE:
							pass->setManualCullingMode(MANUAL_CULL_NONE);
							break;
						default:
							addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_NORMALISE_NORMALS)
				{
					if(!(*j)->children.empty())
						pass->setNormaliseNormals(isTruthValue((*j)->children.front()->token));
				}
				else if((*j)->wordID == ID_LIGHTING)
				{
					if(!(*j)->children.empty())
						pass->setLightingEnabled(isTruthValue((*j)->children.front()->token));
				}
				else if((*j)->wordID == ID_SHADING)
				{
					if(!(*j)->children.empty())
					{
						ScriptNodeList::iterator k = (*j)->children.begin();
						switch((*k)->wordID)
						{
						case ID_FLAT:
							pass->setShadingMode(Ogre::SO_FLAT);
							break;
						case ID_GOURAUD:
							pass->setShadingMode(SO_GOURAUD);
							break;
						case ID_PHONG:
							pass->setShadingMode(SO_PHONG);
							break;
						default:
							addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_POLYGON_MODE)
				{
					if(!(*j)->children.empty())
					{
						ScriptNodeList::iterator k = (*j)->children.begin();
						switch((*k)->wordID)
						{
						case ID_SOLID:
							pass->setPolygonMode(PM_SOLID);
							break;
						case ID_POINTS:
							pass->setPolygonMode(PM_POINTS);
							break;
						case ID_WIREFRAME:
							pass->setPolygonMode(PM_WIREFRAME);
							break;
						default:
							addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_POLYGON_MODE_OVERRIDEABLE)
				{
					if(!(*j)->children.empty())
						pass->setPolygonModeOverrideable(isTruthValue((*j)->children.front()->token));
				}
				else if((*j)->wordID == ID_FOG_OVERRIDE)
				{
					if(!(*j)->children.empty())
					{
						bool fog = isTruthValue((*j)->children.front()->token);
						if(!fog)
							pass->setFog(fog);
						else
						{
							if((*j)->children.size() == 6)
							{
								Ogre::FogMode mode;
								ColourValue c;
								Real start, end, density;
								bool isValid = true;
								ScriptNodeList::iterator k = (*j)->children.begin();
								++k;

								switch((*k)->wordID)
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
									addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
									isValid = false;
								}

								++k;
								if(!getColourValue(k, (*j)->children.end(), c))
								{
									addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
									isValid = false;
								}

								++k;
								if((*k)->type == SNT_NUMBER)
								{
									density = (*k)->data;
								}
								else
								{
									addError(CE_NUMBEREXPECTED, (*k)->file, (*k)->line, (*k)->column);
									isValid = false;
								}
					
								++k;
								if((*k)->type == SNT_NUMBER)
								{
									start = (*k)->data;
								}
								else
								{
									addError(CE_NUMBEREXPECTED, (*k)->file, (*k)->line, (*k)->column);
									isValid = false;
								}

								++k;
								if((*k)->type == SNT_NUMBER)
								{
									end = (*k)->data;
								}
								else
								{
									addError(CE_NUMBEREXPECTED, (*k)->file, (*k)->line, (*k)->column);
									isValid = false;
								}

								if(isValid)
									pass->setFog(true, mode, c, density, start, end);
							}
							else
							{
								pass->setFog(fog);
							}
						}
					}
				}
				else if((*j)->wordID == ID_COLOUR_WRITE)
				{
					if(!(*j)->children.empty())
						pass->setColourWriteEnabled(isTruthValue((*j)->children.front()->token));
				}
				else if((*j)->wordID == ID_MAX_LIGHTS)
				{
					if(!(*j)->children.empty())
					{
						if((*j)->children.front()->type == SNT_NUMBER)
							pass->setMaxSimultaneousLights((*j)->children.front()->data);
						else
							addError(CE_NUMBEREXPECTED, (*j)->children.front()->file, 
								(*j)->children.front()->line, (*j)->children.front()->column);
					}
					else
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
				}
				else if((*j)->wordID == ID_START_LIGHT)
				{
					if(!(*j)->children.empty())
					{
						if((*j)->children.front()->type == SNT_NUMBER)
							pass->setStartLight((*j)->children.front()->data);
						else
							addError(CE_NUMBEREXPECTED, (*j)->children.front()->file, 
								(*j)->children.front()->line, (*j)->children.front()->column);
					}
					else
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
				}
				else if((*j)->wordID == ID_ITERATION)
				{
					if((*j)->children.empty())
					{
						addError(CE_VALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
					else
					{
						ScriptNodePtr node = (*j)->children.front();
						if(node->wordID == ID_ONCE)
						{
							pass->setIteratePerLight(false);
						}
						else if(node->wordID == ID_ONCE_PER_LIGHT)
						{
							if((*j)->children.size() > 1)
							{
								ScriptNodePtr node2 = 
									getNodeAt((*j)->children.begin(), (*j)->children.end(), 1);
								switch(node2->wordID)
								{
								case ID_POINT:
									pass->setIteratePerLight(true, true, Light::LT_POINT);
									break;
								case ID_SPOT:
									pass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
									break;
								case ID_DIRECTIONAL:
									pass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
									break;
								default:
									addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
								}
							}
							else
							{
								pass->setIteratePerLight(true, false);
							}
						}
						else if(node->type == SNT_NUMBER)
						{
							pass->setPassIterationCount(node->data);
							if((*j)->children.size() > 1)
							{
								ScriptNodePtr node2 = 
									getNodeAt((*j)->children.begin(), (*j)->children.end(), 1);
								if(node2->wordID == ID_PER_LIGHT)
								{
									if((*j)->children.size() > 2)
									{
										ScriptNodePtr node3 = 
											getNodeAt((*j)->children.begin(), (*j)->children.end(), 2);
										switch(node3->wordID)
										{
										case ID_POINT:
											pass->setIteratePerLight(true, true, Light::LT_POINT);
											break;
										case ID_SPOT:
											pass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
											break;
										case ID_DIRECTIONAL:
											pass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
											break;
										default:
											addError(CE_INVALIDPROPERTYVALUE, node3->file, node3->line, node3->column);
										}
									}
									else
									{
										pass->setIteratePerLight(true, false);
									}
								}
								else if(node2->wordID == ID_PER_N_LIGHTS)
								{
									ScriptNodePtr node3 = 
												getNodeAt((*j)->children.begin(), (*j)->children.end(), 2);
									if(node3->type == SNT_NUMBER)
									{
										pass->setLightCountPerIteration(node3->data);
										ScriptNodePtr node4 = 
												getNodeAt((*j)->children.begin(), (*j)->children.end(), 3);
										if(!node4.isNull())
										{
											switch(node4->wordID)
											{
											case ID_POINT:
												pass->setIteratePerLight(true, true, Light::LT_POINT);
												break;
											case ID_SPOT:
												pass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
												break;
											case ID_DIRECTIONAL:
												pass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
												break;
											default:
												addError(CE_INVALIDPROPERTYVALUE, node4->file, node4->line, node4->column);
											}
										}
									}
									else
									{
										addError(CE_NUMBEREXPECTED, node3->file, node3->line, node3->column);
									}
								}
								else
								{
									addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
								}
							}
						}
						else
						{
							addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
						}
					}
				}
				else if((*j)->wordID == ID_POINT_SIZE)
				{
					if((*j)->children.empty())
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
					else
					{
						ScriptNodePtr node = (*j)->children.front();
						if(node->type == SNT_NUMBER)
							pass->setPointSize(node->data);
						else
							addError(CE_NUMBEREXPECTED, node->file, node->line, node->column);
					}
				}
				else if((*j)->wordID == ID_POINT_SPRITES)
				{
					if(!(*j)->children.empty())
						pass->setPointSpritesEnabled(isTruthValue((*j)->children.front()->token));
				}
				else if((*j)->wordID == ID_POINT_SIZE_ATTENUATION)
				{
					ScriptNodePtr node1 = (*j)->children.front();
					if(node1.isNull())
					{
						addError(CE_TRUTHVALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
					else
					{
						bool enabled = isTruthValue(node1->token);
						if(enabled)
						{
							ScriptNodePtr node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1),
								node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2),
								node4 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 3);
							bool isValid = true;
							if(node2.isNull() || node3.isNull() || node4.isNull())
							{
								addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
								isValid = false;
							}
							if(node2->type != SNT_NUMBER)
							{
								addError(CE_NUMBEREXPECTED, node2->file, node2->line, node2->column);
								isValid = false;
							}
							if(node3->type != SNT_NUMBER)
							{
								addError(CE_NUMBEREXPECTED, node3->file, node3->line, node3->column);
								isValid = false;
							}
							if(node4->type != SNT_NUMBER)
							{
								addError(CE_NUMBEREXPECTED, node4->file, node4->line, node4->column);
								isValid = false;
							}

							if(isValid)
								pass->setPointAttenuation(true, node2->data, node3->data, node4->data);
						}
						else
						{
							pass->setPointAttenuation(false);
						}
					}
				}
				else if((*j)->wordID == ID_POINT_SIZE_MIN)
				{
					if((*j)->children.empty())
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
					else
					{
						ScriptNodePtr node = (*j)->children.front();
						if(node->type == SNT_NUMBER)
							pass->setPointMinSize(node->data);
						else
							addError(CE_NUMBEREXPECTED, node->file, node->line, node->column);
					}
				}
				else if((*j)->wordID == ID_POINT_SIZE_MAX)
				{
					if((*j)->children.empty())
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
					else
					{
						ScriptNodePtr node = (*j)->children.front();
						if(node->type == SNT_NUMBER)
							pass->setPointMaxSize(node->data);
						else
							addError(CE_NUMBEREXPECTED, node->file, node->line, node->column);
					}
				}
				++j;
			}
		}
	}

	void MaterialScriptCompiler2::compileTextureUnit(const ScriptNodePtr &node, Pass *pass)
	{
		if(node->children.empty())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}

		// Create the TextureUnitState
		TextureUnitState *unit = pass->createTextureUnitState();

		// The first child is possibly the optional name
		ScriptNodeList::iterator i = node->children.begin();
		if((*i)->type != SNT_LBRACE)
		{
			if(!(*i)->token.empty())
				unit->setName((*i)->token);

			// Find the '{'
			i = findNode(i, node->children.end(), SNT_LBRACE);
		}

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(processNode(j, (*i)->children.end()))
			{
				if((*j)->wordID == ID_TEXTURE_ALIAS)
				{
					if(!(*j)->children.empty())
						unit->setTextureNameAlias((*j)->children.front()->token);
					else
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
				}
				else if((*j)->wordID == ID_TEXTURE)
				{
					ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0);
					if(!node1.isNull())
					{
						ScriptNodePtr node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1);
						Ogre::TextureType texType = Ogre::TEX_TYPE_2D;
						bool isValid = true;
						if(!node2.isNull())
						{
							switch(node2->wordID)
							{
							case ID_1D:
								texType = TEX_TYPE_2D;
								break;
							case ID_2D:
								break;
							case ID_3D:
								texType = TEX_TYPE_3D;
								break;
							case ID_CUBIC:
								texType = TEX_TYPE_CUBE_MAP;
								break;
							default:
								addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
								isValid = false;
							}
						}

						if(isValid)
							unit->setTextureName(node1->token, texType);

						ScriptNodePtr node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2);
						if(!node3.isNull())
						{
							if(node3->wordID == ID_UNLIMITED)
							{
								unit->setNumMipmaps(-1);
							}
							else if(node3->type == SNT_NUMBER)
							{
								unit->setNumMipmaps(node3->data);
							}
							else
							{
								addError(CE_INVALIDPROPERTYVALUE, node3->file, node3->line, node3->column);
							}
						}

						ScriptNodePtr node4 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 3);
						if(!node4.isNull())
						{
							if(node4->wordID == ID_ALPHA)
							{
								unit->setIsAlpha(true);
							}
							else
							{
								addError(CE_INVALIDPROPERTYVALUE, node4->file, node4->line, node4->column);
							}
						}

						ScriptNodePtr node5 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 4);
						if(!node5.isNull())
						{
							PixelFormat format = PixelUtil::getFormatFromName(node5->token, true);
							if(format == Ogre::PF_UNKNOWN)
								addError(CE_INVALIDPROPERTYVALUE, node5->file, node5->line, node5->column);
							else
								unit->setDesiredFormat(format);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_ANIM_TEXTURE)
				{
					if((*j)->children.size() >= 3)
					{
						ScriptNodePtr node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1);
						if(node2->type == SNT_NUMBER) // Short form
						{
							ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
								node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 3);
							if(node3->type == SNT_NUMBER)
								unit->setAnimatedTextureName(node1->token, node2->data, node3->data);
							else
								addError(CE_INVALIDPROPERTYVALUE, node3->file, node3->line, node3->column);
						}
						else
						{
							int n = 0, count = (*j)->children.size() - 1;
							Real duration = 0.0f;
							String *frames = new String[count];
							for(ScriptNodeList::iterator k = (*j)->children.begin(); k != (*j)->children.end(); ++k, ++n)
							{
								if(n < count)
									frames[n] = (*k)->token;
								else if(n == count && (*k)->type == SNT_NUMBER)
									duration = (*k)->data;
								else
									addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
							}
							unit->setAnimatedTextureName(frames, count, duration);
							delete[] frames;
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				++j;
			}
		}
	}

	bool MaterialScriptCompiler2::getColourValue(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, ColourValue &c)
	{
		if(i == end)
			return false;

		if((*i)->type == SNT_NUMBER)
		{
			c.r = (*i)->data;
			++i;
		}
		else
		{
			return false;
		}

		// We can bail out now successfully
		if(i == end)
			return true;
		if((*i)->type == SNT_NUMBER)
		{
			c.g = (*i)->data;
			++i;
		}
		else
		{
			return false;
		}

		// We can bail out now successfully
		if(i == end)
			return true;
		if((*i)->type == SNT_NUMBER)
		{
			c.b = (*i)->data;
			++i;
		}
		else
		{
			return false;
		}

		// We can bail out now successfully
		if(i == end)
			return true;
		if((*i)->type == SNT_NUMBER)
		{
			c.a = (*i)->data;
			++i;
		}
		else
		{
			return false;
		}

		return true;
	}

	bool MaterialScriptCompiler2::getBlendFactor(const ScriptNodePtr &node, SceneBlendFactor &sbf)
	{
		bool success = true;
		switch(node->wordID)
		{
		case ID_ONE:
			sbf = Ogre::SBF_ONE;
			break;
		case ID_ZERO:
			sbf = SBF_ZERO;
			break;
		case ID_DEST_COLOUR:
			sbf = Ogre::SBF_DEST_COLOUR;
			break;
		case ID_DEST_ALPHA:
			sbf = SBF_DEST_ALPHA;
			break;
		case ID_SRC_ALPHA:
			sbf = SBF_SOURCE_ALPHA;
			break;
		case ID_SRC_COLOUR:
			sbf = SBF_SOURCE_COLOUR;
			break;
		case ID_ONE_MINUS_DEST_COLOUR:
			sbf = SBF_ONE_MINUS_DEST_COLOUR;
			break;
		case ID_ONE_MINUS_SRC_COLOUR:
			sbf = SBF_ONE_MINUS_SOURCE_COLOUR;
			break;
		case ID_ONE_MINUS_DEST_ALPHA:
			sbf = SBF_ONE_MINUS_DEST_ALPHA;
			break;
		case ID_ONE_MINUS_SRC_ALPHA:
			sbf = SBF_ONE_MINUS_SOURCE_ALPHA;
			break;
		default:
			success = false;
		}
		return success;
	}

	bool MaterialScriptCompiler2::getCompareFunction(const ScriptNodePtr &node, CompareFunction &func)
	{
		bool success = true;
		switch(node->wordID)
		{
		case ID_ALWAYS_FAIL:
			func = Ogre::CMPF_ALWAYS_FAIL;
			break;
		case ID_ALWAYS_PASS:
			func = CMPF_ALWAYS_PASS;
			break;
		case ID_LESS:
			func = CMPF_LESS;
			break;
		case ID_LESS_EQUAL:
			func = CMPF_LESS_EQUAL;
			break;
		case ID_EQUAL:
			func = CMPF_EQUAL;
			break;
		case ID_NOT_EQUAL:
			func = CMPF_NOT_EQUAL;
			break;
		case ID_GREATER_EQUAL:
			func = CMPF_GREATER_EQUAL;
			break;
		case ID_GREATER:
			func = CMPF_GREATER;
			break;
		default:
			success = false;
		}
		return success;
	}
	
}


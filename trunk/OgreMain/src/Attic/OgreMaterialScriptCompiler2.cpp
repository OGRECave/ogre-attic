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
		// The first child is the name of the material
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
					if(name.isNull() || value.isNull())
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
						continue;
					}
					if(name->token.empty())
					{
						addError(CE_INVALIDPROPERTYVALUE, name->file, name->line, name->column);
						continue;
					}
					if(value->token.empty())
					{
						addError(CE_INVALIDPROPERTYVALUE, value->file, value->line, value->column);
						continue;
					}
					mTextureAliases[name->token] = value->token;
				}
				else if((*j)->wordID == ID_TECHNIQUE)
				{

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

	}

	void MaterialScriptCompiler2::compilePass(const ScriptNodePtr &node)
	{

	}
	
}


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

#define REQUIRE(num,err)	if(!nodeExists(i, end, num))\
							{\
								addError(err, (*i)->file, (*i)->line, -1);\
								return;\
							}
#define REQUIRE_TYPE(t,err)	if((*i)->type != t)\
							{\
								addError(err, (*i)->file, (*i)->line, (*i)->column);\
								return;\
							}

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

		mSyntaxCodes.insert("vs_1_1");
		mSyntaxCodes.insert("vs_2_0");
		mSyntaxCodes.insert("vs_2_x");
		mSyntaxCodes.insert("vs_3_0");
		mSyntaxCodes.insert("vs_1_1");
		mSyntaxCodes.insert("arbvp1");
		mSyntaxCodes.insert("vp20");
		mSyntaxCodes.insert("vp30");
		mSyntaxCodes.insert("vp40");
		mSyntaxCodes.insert("ps_1_1");
		mSyntaxCodes.insert("ps_1_2");
		mSyntaxCodes.insert("ps_1_3");
		mSyntaxCodes.insert("ps_1_4");
		mSyntaxCodes.insert("ps_2_0");
		mSyntaxCodes.insert("ps_2_x");
		mSyntaxCodes.insert("ps_3_0");
		mSyntaxCodes.insert("ps_3_x");
		mSyntaxCodes.insert("arbfp1");
		mSyntaxCodes.insert("fp20");
		mSyntaxCodes.insert("fp30");
		mSyntaxCodes.insert("fp40");
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
				if((*i)->token == "abstract")
				{
					// Abstract is followed by the type then the name.
					// Jump past the abstract and the token for the object type and the name.
					++i;
					++i;
					++i;
				}
				else if((*i)->token == "material")
				{
					compileMaterial(i, nodes->end());
				}
				else if((*i)->token == "vertex_program")
				{
					compileVertexProgram(i, nodes->end());
				}
				else if((*i)->token == "fragment_program")
				{
					compileFragmentProgram(i, nodes->end());
				}
				else
				{

				}
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

	void MaterialScriptCompiler2::compileMaterial(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		// The next token needs to be the name of the material
		{
			ScriptNodeList::iterator j = i;
			++j;

			if(j == end)
			{
				addError(CE_OBJECTNAMEEXPECTED, (*i)->file, (*i)->line, -1);
				return;
			}
			i = j;
		}
		if((*i)->type != SNT_STRING)
		{
			addError(CE_OBJECTNAMEEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		// We have verified that a name is here, so get the material
		if(mListener)
		{
			mMaterial = mListener->getMaterial((*i)->token, mGroup);
		}
		else
		{
			mMaterial = (Material*)MaterialManager::getSingleton().create((*i)->token, mGroup).get();
			mMaterial->removeAllTechniques();
		}

		if(!mMaterial)
		{
			addError(CE_OBJECTALLOCATIONERROR, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		// The next token should be the '{'
		{
			ScriptNodeList::iterator j = i;
			j++;

			// We can't end here!
			if(j == end)
			{
				// Bail out!
				addError(CE_OPENBRACEEXPECTED, (*i)->file, (*i)->line, -1);
				mMaterial = 0;
				return;
			}

			// The next token must be the '{' starting the particle system object's body
			if((*j)->type != SNT_LBRACE)
			{
				addError(CE_OPENBRACEEXPECTED, (*j)->file, (*j)->line, (*j)->column);
				mMaterial = 0;
				return;
			}
			
			// We're ok, so continue on
			i = j;
		}

		// We hit the '{', so descend into it to continue compilation
		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				if((*j)->token == "lod_distances")
					compileLodDistances(j, (*i)->children.end());
				else if((*j)->token == "receive_shadows")
					compileReceiveShadows(j, (*i)->children.end());
				else if((*j)->token == "transparency_casts_shadows")
					compileTransparencyCastsShadows(j, (*i)->children.end());
				else if((*j)->token == "set_texture_alias")
					compileLodDistances(j, (*i)->children.end());
				else if((*j)->token == "technique")
					compileTechnique(j, (*i)->children.end());
				else
				{
					addError(CE_UNKNOWNTOKEN, (*i)->file, (*i)->line, (*i)->column);
					++i;
				}
			}
		}

		// We are finished with the object, so consume the '}'
		++i; // '{'
		++i; // '}'

		// Verify we have some techniques
		if(mMaterial->getNumTechniques() == 0)
			mMaterial->createTechnique();

		// Apply the texture alises
		if(mListener)
			mListener->preApplyTextureAliases(mTextureAliases);
		mMaterial->applyTextureAliases(mTextureAliases);

		// Reset the pointer to the system
		mMaterial = 0;
		// Remove texture alises
		mTextureAliases.clear();
	}

	void MaterialScriptCompiler2::compileLodDistances(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;
		REQUIRE_TYPE(SNT_NUMBER, CE_NUMBEREXPECTED)
		
		Ogre::Material::LodDistanceList lods;

		// Read any number tokens after this one
		while(i != end && (*i)->type == SNT_NUMBER)
		{
			lods.push_back(StringConverter::parseReal((*i)->token));
			++i;
		}

		if(!lods.empty())
			mMaterial->setLodLevels(lods);
	}

	void MaterialScriptCompiler2::compileReceiveShadows(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		REQUIRE(1, CE_TRUTHVALUEEXPECTED)
		++i;
		mMaterial->setReceiveShadows(isTruthValue((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileTransparencyCastsShadows(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		REQUIRE(1, CE_TRUTHVALUEEXPECTED)
		++i;
		mMaterial->setTransparencyCastsShadows(isTruthValue((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileSetTextureAlias(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		REQUIRE(2, CE_STRINGEXPECTED)

		++i;
		String alias = (*i)->token;
		++i;
		String tex = (*i)->token;
		mTextureAliases[alias] = tex;
		++i;
	}

	void MaterialScriptCompiler2::compileTechnique(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		// Consume the "technique" node
		REQUIRE(1, CE_OPENBRACEEXPECTED)
		++i;

		// Create the technique being compiled
		Technique *technique = mMaterial->createTechnique();

		// Consume the name if there is one
		if((*i)->type != SNT_LBRACE)
		{
			technique->setName((*i)->token);

			REQUIRE(1, CE_OPENBRACEEXPECTED)
			++i;
		}

		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		// Hit the '{', so compile the technique's parameters
		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				if((*j)->token == "scheme")
					compileScheme(j, (*i)->children.end(), technique);
				else if((*j)->token == "lod_index")
					compileLodIndex(j, (*i)->children.end(), technique);
				else if((*j)->token == "pass")
					compilePass(j, (*i)->children.end(), technique);
				else
				{
					addError(CE_UNKNOWNTOKEN, (*j)->file, (*j)->line, (*j)->column);
					++j;
				}
			}
		}

		// Consume the '{' and the '}'
		++i;
		++i;

		// Verify this technique has at least one pass
		if(technique->getNumPasses() == 0)
			technique->createPass();
	}

	void MaterialScriptCompiler2::compileScheme(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Technique *technique)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;
		technique->setSchemeName((*i)->token);
		++i;
	}

	void MaterialScriptCompiler2::compileLodIndex(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Technique *technique)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;
		REQUIRE_TYPE(SNT_NUMBER, CE_NUMBEREXPECTED)
		technique->setLodIndex(StringConverter::parseInt((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compilePass(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Technique *technique)
	{
		// Consume the "pass" node
		REQUIRE(1, CE_OPENBRACEEXPECTED)
		++i;

		// Create the technique being compiled
		Pass *pass = technique->createPass();

		// Consume the name if there is one
		if((*i)->type != SNT_LBRACE)
		{
			pass->setName((*i)->token);

			REQUIRE(1, CE_OPENBRACEEXPECTED)
			++i;
		}

		// After a possible name consumption, check again for the left brace
		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		// Hit the '{', so compile the pass's parameters
		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!processNode(j, (*i)->children.end()))
			{
				if((*j)->token == "ambient")
					compileAmbient(j, (*i)->children.end(), pass);
				else if((*j)->token == "diffuse")
					compileDiffuse(j, (*i)->children.end(), pass);
				else if((*j)->token == "specular")
					compileSpecular(j, (*i)->children.end(), pass);
				else if((*j)->token == "emissive")
					compileSpecular(j, (*i)->children.end(), pass);
				else if((*j)->token == "scene_blend")
					compileSceneBlend(j, (*i)->children.end(), pass);
				else if((*j)->token == "separate_scene_blend")
					compileSeparateSceneBlend(j, (*i)->children.end(), pass);
				else if((*j)->token == "depth_check")
					compileDepthCheck(j, (*i)->children.end(), pass);
				else if((*j)->token == "depth_write")
					compileDepthWrite(j, (*i)->children.end(), pass);
				else if((*j)->token == "depth_func")
					compileDepthFunc(j, (*i)->children.end(), pass);
				else if((*j)->token == "depth_bias")
					compileDepthBias(j, (*i)->children.end(), pass);
				else if((*j)->token == "iteration_depth_bias")
					compileIterationDepthBias(j, (*i)->children.end(), pass);
				else if((*j)->token == "alpha_rejection")
					compileAlphaRejection(j, (*i)->children.end(), pass);
				else if((*j)->token == "light_scissor")
					compileLightScissor(j, (*i)->children.end(), pass);
				else if((*j)->token == "light_clip_planes")
					compileLightClipPlanes(j, (*i)->children.end(), pass);
				else if((*j)->token == "illumination_stage")
					compileIlluminationStage(j, (*i)->children.end(), pass);
				else if((*j)->token == "cull_hardware")
					compileCullHardware(j, (*i)->children.end(), pass);
				else if((*j)->token == "cull_software")
					compileCullSoftware(j, (*i)->children.end(), pass);
				else if((*j)->token == "normalise_normals")
					compileNormaliseNormals(j, (*i)->children.end(), pass);
				else if((*j)->token == "lighting")
					compileLighting(j, (*i)->children.end(), pass);
				else if((*j)->token == "shading")
					compileShading(j, (*i)->children.end(), pass);
				else if((*j)->token == "polygon_mode")
					compilePolygonMode(j, (*i)->children.end(), pass);
				else if((*j)->token == "polygon_mode_overrideable")
					compilePolygonModeOverrideable(j, (*i)->children.end(), pass);
				else if((*j)->token == "fog_override")
					compileFogOverride(j, (*i)->children.end(), pass);
				else if((*j)->token == "colour_write")
					compileColourWrite(j, (*i)->children.end(), pass);
				else if((*j)->token == "max_lights")
					compileNormaliseNormals(j, (*i)->children.end(), pass);
				else if((*j)->token == "start_light")
					compileStartLight(j, (*i)->children.end(), pass);
				else if((*j)->token == "iteration")
					compileIteration(j, (*i)->children.end(), pass);
				else if((*j)->token == "point_size")
					compilePointSize(j, (*i)->children.end(), pass);
				else if((*j)->token == "point_sprites")
					compilePointSprites(j, (*i)->children.end(), pass);
				else if((*j)->token == "point_size_attenuation")
					compilePointSizeAttenuation(j, (*i)->children.end(), pass);
				else if((*j)->token == "point_size_min")
					compilePointSizeMin(j, (*i)->children.end(), pass);
				else if((*j)->token == "point_size_max")
					compilePointSizeMax(j, (*i)->children.end(), pass);
				else if((*j)->token == "texture_unit")
					compileTextureUnit(j, (*i)->children.end(), pass);
				else
				{
					addError(CE_UNKNOWNTOKEN, (*i)->file, (*i)->line, (*i)->column);
					++j;
				}
			}
		}

		// Consume the '{' and the '}'
		++i;
		++i;
	}

	void MaterialScriptCompiler2::compileAmbient(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;

		if((*i)->token == "vertexcolour")
		{
			pass->setVertexColourTracking(TVC_AMBIENT);
			++i;
		}
		else
		{
			ColourValue c;
			if(parseColour(i, end, c))
				pass->setAmbient(c);
			else
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
		}
	}

	void MaterialScriptCompiler2::compileDiffuse(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;

		if((*i)->token == "vertexcolour")
		{
			pass->setVertexColourTracking(TVC_DIFFUSE);
			++i;
		}
		else
		{
			ColourValue c;
			if(parseColour(i, end, c))
				pass->setDiffuse(c);
			else
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
		}
	}

	void MaterialScriptCompiler2::compileSpecular(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;

		if((*i)->token == "vertexcolour")
		{
			pass->setVertexColourTracking(TVC_SPECULAR);
			++i;
		}
		else
		{
			ColourValue c;
			if(parseColour(i, end, c))
				pass->setSpecular(c);
			else
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
		}
	}

	void MaterialScriptCompiler2::compileEmissive(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;

		if((*i)->token == "vertexcolour")
		{
			pass->setVertexColourTracking(TVC_EMISSIVE);
			++i;
		}
		else
		{
			ColourValue c;
			if(parseColour(i, end, c))
				pass->setSelfIllumination(c);
			else
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
		}
	}

	void MaterialScriptCompiler2::compileSceneBlend(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;
		
		String blend = (*i)->token;
		if(blend == "replace")
			pass->setSceneBlending(SBT_REPLACE);
		else if(blend == "add")
			pass->setSceneBlending(SBT_ADD);
		else if(blend == "modulate")
			pass->setSceneBlending(SBT_MODULATE);
		else if(blend == "colour_blend")
			pass->setSceneBlending(SBT_TRANSPARENT_COLOUR);
		else if(blend == "alpha_blend")
			pass->setSceneBlending(SBT_TRANSPARENT_ALPHA);
		else
		{
			// Two arguments now expected
			REQUIRE(1, CE_STRINGEXPECTED)

			SceneBlendFactor src, dst;
			if(!parseBlendFactor(blend, src))
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
				return;
			}

			++i;
			if(!parseBlendFactor((*i)->token, dst))
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
				return;
			}

			pass->setSceneBlending(src, dst);
		}
		++i;
	}

	void MaterialScriptCompiler2::compileSeparateSceneBlend(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass)
	{
		// Check the first token to see if we are using the 2 or 4 argument version
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;

		bool useSbt = true;
		Ogre::SceneBlendType sbt1, sbt2;
		SceneBlendFactor colourSbf1, colourSbf2, alphaSbf1, alphaSbf2;

		String str = (*i)->token;
		if(str == "replace")
			sbt1 = SBT_REPLACE;
		else if(str == "add")
			sbt1 = SBT_ADD;
		else if(str == "modulate")
			sbt1 = SBT_MODULATE;
		else if(str == "colour_blend")
			sbt1 = SBT_TRANSPARENT_COLOUR;
		else if(str == "alpha_blend")
			sbt1 = SBT_TRANSPARENT_ALPHA;
		else
			useSbt = false;

		if(useSbt)
		{
			// We only need to get one more blending type
			REQUIRE(1, CE_STRINGEXPECTED)
			++i;

			str = (*i)->token;
			if(str == "replace")
				sbt2 = SBT_REPLACE;
			else if(str == "add")
				sbt2 = SBT_ADD;
			else if(str == "modulate")
				sbt2 = SBT_MODULATE;
			else if(str == "colour_blend")
				sbt2 = SBT_TRANSPARENT_COLOUR;
			else if(str == "alpha_blend")
				sbt2 = SBT_TRANSPARENT_ALPHA;
			else
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
				return;
			}
			pass->setSeparateSceneBlending(sbt1, sbt2);
			++i;
		}
		else
		{
			// First colour sbf
			if(!parseBlendFactor(str, colourSbf1))
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
				return;
			}

			// Second colour sbf
			REQUIRE(3, CE_STRINGEXPECTED)
			++i;
			String str2 = (*i)->token;
			++i;
			String str3 = (*i)->token;
			++i;
			String str4 = (*i)->token;

			if(!parseBlendFactor(str2, colourSbf2))
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
				return;
			}

			if(!parseBlendFactor(str3, alphaSbf1))
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
				return;
			}

			if(!parseBlendFactor(str4, alphaSbf2))
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
				return;
			}

			pass->setSeparateSceneBlending(colourSbf1, colourSbf2, alphaSbf1, alphaSbf2);
			++i;
		}
	}

	void MaterialScriptCompiler2::compileDepthCheck(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_TRUTHVALUEEXPECTED)
		++i;
		pass->setDepthCheckEnabled(isTruthValue((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileDepthWrite(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_TRUTHVALUEEXPECTED)
		++i;
		pass->setDepthCheckEnabled(isTruthValue((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileDepthFunc(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;
		
		CompareFunction func = Ogre::CMPF_ALWAYS_FAIL;
		if(!parseCompareFunction((*i)->token, func))
		{
			addError(CE_STRINGEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		pass->setDepthFunction(func);
	}

	void MaterialScriptCompiler2::compileDepthBias(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;
		
		float constBias = 0.0f, slopeBias = 0.0f;
		constBias = StringConverter::parseReal((*i)->token);
		++i;
		if((*i)->type == SNT_NUMBER)
		{
			slopeBias = StringConverter::parseReal((*i)->token);
			++i;
		}

		pass->setDepthBias(constBias, slopeBias);
	}

	void MaterialScriptCompiler2::compileIterationDepthBias(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;
		pass->setIterationDepthBias(StringConverter::parseReal((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileAlphaRejection(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(2, CE_STRINGEXPECTED)
		++i;

		CompareFunction func = Ogre::CMPF_ALWAYS_FAIL;
		unsigned char val = 0;

		if(!parseCompareFunction((*i)->token, func))
		{
			addError(CE_STRINGEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		++i;
		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		val = StringConverter::parseInt((*i)->token);
		pass->setAlphaRejectSettings(func, val);
	}

	void MaterialScriptCompiler2::compileLightScissor(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_TRUTHVALUEEXPECTED)
		++i;
		pass->setLightScissoringEnabled(isTruthValue((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileLightClipPlanes(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_TRUTHVALUEEXPECTED)
		++i;
		pass->setLightClipPlanesEnabled(isTruthValue((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileIlluminationStage(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;

		Ogre::IlluminationStage is = Ogre::IS_AMBIENT;
		if((*i)->token == "ambient")
			is = IS_AMBIENT;
		else if((*i)->token == "decal")
			is = IS_DECAL;
		else if((*i)->token == "per_light")
			is = IS_PER_LIGHT;
		else
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		pass->setIlluminationStage(is);
		++i;
	}

	void MaterialScriptCompiler2::compileCullHardware(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;

		Ogre::CullingMode cm = Ogre::CULL_NONE;
		if((*i)->token == "none")
			cm = CULL_NONE;
		else if((*i)->token == "clockwise")
			cm = CULL_CLOCKWISE;
		else if((*i)->token == "anticlockwise")
			cm = CULL_ANTICLOCKWISE;
		else
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		pass->setCullingMode(cm);
		++i;
	}

	void MaterialScriptCompiler2::compileCullSoftware(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;

		Ogre::ManualCullingMode cm = Ogre::MANUAL_CULL_NONE;
		if((*i)->token == "none")
			cm = MANUAL_CULL_NONE;
		else if((*i)->token == "back")
			cm = MANUAL_CULL_BACK;
		else if((*i)->token == "front")
			cm = MANUAL_CULL_FRONT;
		else
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		pass->setManualCullingMode(cm);
		++i;
	}

	void MaterialScriptCompiler2::compileNormaliseNormals(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_TRUTHVALUEEXPECTED)
		++i;
		pass->setNormaliseNormals(isTruthValue((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileLighting(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_TRUTHVALUEEXPECTED)
		++i;
		pass->setLightingEnabled(isTruthValue((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileShading(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;
		
		Ogre::ShadeOptions so = Ogre::SO_PHONG;
		if((*i)->token == "phong")
			so = SO_PHONG;
		else if((*i)->token == "gouraud")
			so = SO_GOURAUD;
		else if((*i)->token == "flat")
			so = SO_FLAT;
		else
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		pass->setShadingMode(so);
		++i;
	}

	void MaterialScriptCompiler2::compilePolygonMode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;
		
		PolygonMode pm = Ogre::PM_SOLID;
		if((*i)->token == "solid")
			pm = PM_SOLID;
		else if((*i)->token == "wireframe")
			pm = PM_WIREFRAME;
		else if((*i)->token == "points")
			pm = PM_POINTS;
		else
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		pass->setPolygonMode(pm);
		++i;
	}

	void MaterialScriptCompiler2::compilePolygonModeOverrideable(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_TRUTHVALUEEXPECTED)
		++i;
		pass->setPolygonModeOverrideable(isTruthValue((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileFogOverride(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_TRUTHVALUEEXPECTED)
		++i;
		
		bool fogOverride = isTruthValue((*i)->token);
		if(fogOverride)
		{
			REQUIRE(7, CE_STRINGEXPECTED)
			++i;

			String type = (*i)->token;
			Ogre::FogMode fm = Ogre::FOG_NONE;
			if(type == "none")
				fm = FOG_NONE;
			else if(type == "linear")
				fm = FOG_LINEAR;
			else if(type == "exp")
				fm = FOG_EXP;
			else if(type == "exp2")
				fm = FOG_EXP2;
			else
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
				return;
			}
			++i;

			ColourValue c;
			if(!parseColour(i, end, c))
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
				return;
			}

			Real density = 0.0f, start = 0.0f, stop = 0.0f;
			if((*i)->type != SNT_NUMBER)
			{
				addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
				return;
			}
			density = StringConverter::parseReal((*i)->token);
			++i;

			if((*i)->type != SNT_NUMBER)
			{
				addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
				return;
			}
			start = StringConverter::parseReal((*i)->token);
			++i;

			if((*i)->type != SNT_NUMBER)
			{
				addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
				return;
			}
			stop = StringConverter::parseReal((*i)->token);
			++i;

			pass->setFog(fogOverride, fm, c, density, start, stop);
		}
		else
		{
			pass->setFog(false);
			++i;
		}
	}

	void MaterialScriptCompiler2::compileColourWrite(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_TRUTHVALUEEXPECTED)
		++i;
		pass->setColourWriteEnabled(isTruthValue((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileMaxLights(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;
		
		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		
		pass->setMaxSimultaneousLights(StringConverter::parseUnsignedInt((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileStartLight(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;
		
		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		
		pass->setStartLight(StringConverter::parseUnsignedInt((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileIteration(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;
		
		if((*i)->type == SNT_NUMBER)
		{
			// It is the numbered version
			pass->setPassIterationCount(StringConverter::parseInt((*i)->token));
			++i;
			
			if(i != end && (*i)->token == "per_light")
			{
				++i;
				// We can either do it for a specific light type, or all lights
				if(i != end && (*i)->token == "point")
				{
					pass->setIteratePerLight(true, true, Light::LT_POINT);
					++i;
				}
				else if(i != end && (*i)->token == "directional")
				{
					pass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
					++i;
				}
				else if(i != end && (*i)->token == "spot")
				{
					pass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
					++i;
				}
				else
				{
					pass->setIteratePerLight(true, false);
				}
			}
			else if((*i)->token == "per_n_lights")
			{
				REQUIRE(1, CE_NUMBEREXPECTED)
				++i;
				if((*i)->type != SNT_NUMBER)
				{
					addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
					return;
				}

				unsigned short lc = StringConverter::parseUnsignedInt((*i)->token);
				pass->setLightCountPerIteration(lc);

				++i;
				if(i != end && (*i)->token == "point")
				{
					pass->setIteratePerLight(true, true, Light::LT_POINT);
					++i;
				}
				else if(i != end && (*i)->token == "directional")
				{
					pass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
					++i;
				}
				else if(i != end && (*i)->token == "spot")
				{
					pass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
					++i;
				}
				else
				{
					pass->setIteratePerLight(true, false);
				}
			}
		}
		else
		{
			if((*i)->token == "once")
			{
				pass->setIteratePerLight(false);
				++i;
			}
			else if((*i)->token == "once_per_light")
			{
				++i;
				if(i != end && (*i)->token == "point")
				{
					pass->setIteratePerLight(true, true, Light::LT_POINT);
					++i;
				}
				else if(i != end && (*i)->token == "directional")
				{
					pass->setIteratePerLight(true, true, Light::LT_DIRECTIONAL);
					++i;
				}
				else if(i != end && (*i)->token == "spot")
				{
					pass->setIteratePerLight(true, true, Light::LT_SPOTLIGHT);
					++i;
				}
				else
				{
					pass->setIteratePerLight(true, false);
				}
			}
			else
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			}
		}
	}

	void MaterialScriptCompiler2::compilePointSize(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;
		
		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		
		pass->setPointSize(StringConverter::parseReal((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compilePointSprites(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_TRUTHVALUEEXPECTED)
		++i;
		pass->setPointSpritesEnabled(isTruthValue((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compilePointSizeAttenuation(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_TRUTHVALUEEXPECTED)
		++i;
		
		bool on = isTruthValue((*i)->token);
		if(on)
		{
			++i;
			if(i != end && (*i)->type == SNT_NUMBER)
			{
				Real constant = StringConverter::parseReal((*i)->token), linear = 0.0f, quad = 0.0f;
				++i;
				if(i != end && (*i)->type == SNT_NUMBER)
				{
					linear = StringConverter::parseReal((*i)->token);
					++i;
					if(i != end && (*i)->type == SNT_NUMBER)
					{
						quad = StringConverter::parseReal((*i)->token);
						++i;
					}
				}
				pass->setPointAttenuation(true, constant, linear, quad);
			}
			else
			{
				pass->setPointAttenuation(true);
			}
		}
		else
		{
			pass->setPointAttenuation(false);
			++i;
		}
	}

	void MaterialScriptCompiler2::compilePointSizeMin(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;
		
		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		
		pass->setPointMinSize(StringConverter::parseReal((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compilePointSizeMax(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::Pass *pass)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;
		
		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		
		pass->setPointMaxSize(StringConverter::parseReal((*i)->token));
		++i;
	}

	void MaterialScriptCompiler2::compileTextureUnit(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Pass *pass)
	{
		REQUIRE(1, CE_OPENBRACEEXPECTED)
		++i;

		TextureUnitState *texUnit = pass->createTextureUnitState();

		if((*i)->type != SNT_LBRACE)
		{
			texUnit->setName((*i)->token);
			
			REQUIRE(1, CE_OPENBRACEEXPECTED)
			++i;
		}

		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if((*j)->token == "texture_alias")
			{
				compileTextureAlias(j, (*i)->children.end(), texUnit); 
			}
			else if((*j)->token == "texture")
			{
				compileTexture(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "anim_texture")
			{
				compileAnimTexture(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "cubic_texture")
			{
				compileCubicTexture(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "tex_coord_set")
			{
				compileTexCoordSet(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "tex_address_mode")
			{
				compileTexAddressMode(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "tex_border_colour")
			{
				compileTexBorderColour(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "filtering")
			{
				compileFiltering(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "max_anisotropy")
			{
				compileMaxAnisotropy(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "mipmap_bias")
			{
				compileMipmapBias(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "colour_op")
			{
				compileColourOp(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "colour_op_ex")
			{
				compileColourOpEx(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "colour_op_multipass_fallback")
			{
				compileColourOpMultipassFallback(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "alpha_op_ex")
			{
				compileAlphaOpEx(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "env_map")
			{
				compileEnvMap(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "scroll")
			{
				compileScroll(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "scroll_anim")
			{
				compileScrollAnim(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "rotate")
			{
				compileRotate(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "rotate_anim")
			{
				compileRotateAnim(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "scale")
			{
				compileScale(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "wave_xform")
			{
				compileWaveXForm(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "transform")
			{
				compileTransform(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "binding_type")
			{
				compileBindingType(j, (*i)->children.end(), texUnit);
			}
			else if((*j)->token == "content_type")
			{
				compileContentType(j, (*i)->children.end(), texUnit);
			}
			else
			{
				addError(CE_UNKNOWNTOKEN, (*j)->file, (*j)->line, (*j)->column);
				++j;
			}
		}

		++i; // Consume the '{'
		++i; // Consume the '}'
	}

	void MaterialScriptCompiler2::compileTextureAlias(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;
		unitState->setTextureNameAlias((*i)->token);
		++i;
	}
	void MaterialScriptCompiler2::compileTexture(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;

		// Read the name
		String name = (*i)->token;
		++i;
		
		// Read the type
		Ogre::TextureType type = Ogre::TEX_TYPE_2D;
		if(i != end)
		{
			bool increment = true;
			if((*i)->token == "1d" || (*i)->token == "1D")
				type = Ogre::TEX_TYPE_1D;
			else if((*i)->token == "2d" || (*i)->token == "2D")
				type = Ogre::TEX_TYPE_2D;
			else if((*i)->token == "3d" || (*i)->token == "3D")
				type = Ogre::TEX_TYPE_3D;
			else if(Ogre::StringUtil::match((*i)->token, "cubic", false))
				type = Ogre::TEX_TYPE_CUBE_MAP;
			else
				increment = false;

			if(increment)
				++i;
		}

		// Set the texture option
		unitState->setTextureName(name, type);

		// Check for mipmapping options
		if(i != end && ((*i)->type == SNT_NUMBER || StringUtil::match((*i)->token, "unlimited", false)))
		{
			int n = (*i)->type == SNT_NUMBER ? StringConverter::parseInt((*i)->token) : -1;
			unitState->setNumMipmaps(n);
			++i;
		}
		else
		{
			// Bail out to short-circuit further processing
			return;
		}

		// Check for the 'alpha' option
		if(i != end && StringUtil::match((*i)->token, "alpha", false))
		{
			unitState->setIsAlpha(true);
			++i;
		}

		// Check now for the pixel format
		if(i != end)
		{
			Ogre::PixelFormat pf = Ogre::PixelUtil::getFormatFromName((*i)->token, true);
			if(pf != PF_UNKNOWN)
			{
				unitState->setDesiredFormat(pf);
				++i;
			}
		}
	}
	void MaterialScriptCompiler2::compileAnimTexture(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(3, CE_STRINGEXPECTED)
		++i;

		// Read in the first name
		String name = (*i)->token;
		++i;

		// If the next is a string as well then it is assumed to be the long form
		if((*i)->type == SNT_NUMBER)
		{
			int numFrames = StringConverter::parseInt((*i)->token);
			++i;
			Real duration = StringConverter::parseReal((*i)->token);
			++i;
			unitState->setAnimatedTextureName(name, numFrames, duration);
		}
		else
		{
			std::vector<String> names;
			names.push_back(name);

			while(i != end && (*i)->type != SNT_NUMBER)
			{
				names.push_back((*i)->token);
				++i;
			}
			if(i == end)
				return;
			
			String *strs = new String[names.size()];
			for(size_t j = 0; j < names.size(); ++j)
				strs[j] = names[j];
			unitState->setAnimatedTextureName(strs, names.size(), StringConverter::parseReal((*i)->token));
			delete[] strs;
		}
	}
	void MaterialScriptCompiler2::compileCubicTexture(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(2, CE_STRINGEXPECTED)
		++i;

		String name = (*i)->token;
		++i;
		if(StringUtil::match((*i)->token, "combinedUVW", false) ||
			StringUtil::match((*i)->token, "separateUV", false))
		{
			unitState->setCubicTextureName(name, StringUtil::match((*i)->token, "combinedUVW", false));
			++i;
		}
		else
		{
			String names[6];
			names[0] = name;
			names[1] = (*i)->token;

			REQUIRE(5, CE_STRINGEXPECTED)
			++i;
			names[2] = (*i)->token;
			++i;
			names[3] = (*i)->token;
			++i;
			names[4] = (*i)->token;
			++i;
			names[5] = (*i)->token;
			++i;
			unitState->setCubicTextureName(names);
			++i;
		}
	}
	void MaterialScriptCompiler2::compileTexCoordSet(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		unitState->setTextureCoordSet(StringConverter::parseUnsignedInt((*i)->token));
	}
	void MaterialScriptCompiler2::compileTexAddressMode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;

		Ogre::TextureUnitState::TextureAddressingMode uMode, vMode, wMode;
		if((*i)->token == "wrap")
		{
			uMode = TextureUnitState::TAM_WRAP;
			++i;
		}
		else if((*i)->token == "clamp")
		{
			uMode = TextureUnitState::TAM_CLAMP;
			++i;
		}
		else if((*i)->token == "mirror")
		{
			uMode = TextureUnitState::TAM_MIRROR;
			++i;
		}
		else if((*i)->token == "border")
		{
			uMode = TextureUnitState::TAM_BORDER;
			++i;
		}
		else
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		if(i == end)
		{
			unitState->setTextureAddressingMode(uMode);
			return;
		}
		else
		{
			// Read the V addressing mode
			if((*i)->token == "wrap")
			{
				vMode = TextureUnitState::TAM_WRAP;
				++i;
			}
			else if((*i)->token == "clamp")
			{
				vMode = TextureUnitState::TAM_CLAMP;
				++i;
			}
			else if((*i)->token == "mirror")
			{
				vMode = TextureUnitState::TAM_MIRROR;
				++i;
			}
			else if((*i)->token == "border")
			{
				vMode = TextureUnitState::TAM_BORDER;
				++i;
			}
			else
			{
				unitState->setTextureAddressingMode(uMode);
				return;
			}

			// Bail out again
			if(i == end)
			{
				unitState->setTextureAddressingMode(uMode, vMode, TextureUnitState::TAM_WRAP);
				return;
			}

			// Read w addressing mode
			if((*i)->token == "wrap")
			{
				wMode = TextureUnitState::TAM_WRAP;
				unitState->setTextureAddressingMode(uMode, vMode, wMode);
				++i;
			}
			else if((*i)->token == "clamp")
			{
				wMode = TextureUnitState::TAM_CLAMP;
				unitState->setTextureAddressingMode(uMode, vMode, wMode);
				++i;
			}
			else if((*i)->token == "mirror")
			{
				wMode = TextureUnitState::TAM_MIRROR;
				unitState->setTextureAddressingMode(uMode, vMode, wMode);
				++i;
			}
			else if((*i)->token == "border")
			{
				wMode = TextureUnitState::TAM_BORDER;
				unitState->setTextureAddressingMode(uMode, vMode, wMode);
				++i;
			}
			else
			{
				unitState->setTextureAddressingMode(uMode, vMode, TextureUnitState::TAM_WRAP);
			}
		}
	}
	void MaterialScriptCompiler2::compileTexBorderColour(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		++i;

		ColourValue clr;
		if(parseColour(i, end, clr))
			unitState->setTextureBorderColour(clr);
		else
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
	}
	void MaterialScriptCompiler2::compileFiltering(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;

		// Use a look-ahead to find if there are filter options indicating it's the extended form
		// Otherwise, assume it is the simple form
		bool extended = false;
		ScriptNodeList::iterator j = i;
		++j;
		if(j != end && ((*j)->token == "none" || (*j)->token == "point" ||
			(*j)->token == "linear" || (*j)->token == "anisotropic"))
			extended = true;

		if(extended)
		{
			FilterOptions min, mag, mip;

			if((*i)->token == "point")
				min = FO_POINT;
			else if((*i)->token == "linear")
				min = FO_LINEAR;
			else if((*i)->token == "anisotropic")
				min = FO_ANISOTROPIC;
			else
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
				return;
			}

			++i;
			if((*i)->token == "point")
				mag = FO_POINT;
			else if((*i)->token == "linear")
				mag = FO_LINEAR;
			else if((*i)->token == "anisotropic")
				mag = FO_ANISOTROPIC;
			else
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
				return;
			}

			++i;
			if((*i)->token == "point")
				mip = FO_POINT;
			else if((*i)->token == "linear")
				mip = FO_LINEAR;
			else if((*i)->token == "anisotropic")
				mip = FO_ANISOTROPIC;
			else if((*i)->token == "none")
				mip = FO_NONE;
			else
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
				return;
			}

			unitState->setTextureFiltering(min, mag, mip);
			++i;
		}
		else
		{
			Ogre::TextureFilterOptions tfo;
			if((*i)->token == "none")
				tfo = Ogre::TFO_NONE;
			else if((*i)->token == "bilinear")
				tfo = TFO_BILINEAR;
			else if((*i)->token == "trilinear")
				tfo = TFO_TRILINEAR;
			else if((*i)->token == "anisotropic")
				tfo = TFO_ANISOTROPIC;
			else
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
				return;
			}

			unitState->setTextureFiltering(tfo);
			++i;
		}
	}
	void MaterialScriptCompiler2::compileMaxAnisotropy(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		unitState->setTextureAnisotropy(StringConverter::parseUnsignedInt((*i)->token));
		++i;
	}
	void MaterialScriptCompiler2::compileMipmapBias(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		unitState->setTextureMipmapBias(StringConverter::parseReal((*i)->token));
		++i;
	}
	void MaterialScriptCompiler2::compileColourOp(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;

		if((*i)->token == "replace")
		{
			unitState->setColourOperation(Ogre::LBO_REPLACE);
			++i;
		}
		else if((*i)->token == "add")
		{
			unitState->setColourOperation(LBO_ADD);
			++i;
		}
		else if((*i)->token == "modulate")
		{
			unitState->setColourOperation(LBO_MODULATE);
			++i;
		}
		else if((*i)->token == "alpha_blend")
		{
			unitState->setColourOperation(Ogre::LBO_ALPHA_BLEND);
			++i;
		}
		else
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
	}

	void MaterialScriptCompiler2::compileColourOpEx(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(3, CE_STRINGEXPECTED)
		++i;

		LayerBlendOperationEx op;
        LayerBlendSource src1, src2;
        Real manual = 0.0;
        ColourValue colSrc1 = ColourValue::White;
        ColourValue colSrc2 = ColourValue::White;

		// Parse the blend operation
		if(!parseBlendOp((*i)->token, op))
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		++i;

		// Parse the blend source 1
		if(!parseBlendSource((*i)->token, src1))
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		++i;

		// Parse the blend source 2
		if(!parseBlendSource((*i)->token, src2))
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		++i;

		if(i == end)
		{
			unitState->setColourOperationEx(op, src1, src2, colSrc1, colSrc2, manual);
			return;
		}

		// Get the manual blend amount
		if(op == Ogre::LBX_BLEND_MANUAL)
		{
			if((*i)->type == SNT_NUMBER)
			{
				manual = StringConverter::parseReal((*i)->token);
				++i;
			}
			else
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			}
		}

		// Get the colors for manual modes
		if(src1 == Ogre::LBS_MANUAL)
		{
			if(!parseColour(i, end, colSrc1))
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
		}
		if(src2 == Ogre::LBS_MANUAL)
		{
			if(!parseColour(i, end, colSrc2))
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
		}

		unitState->setColourOperationEx(op, src1, src2, colSrc1, colSrc2, manual);
	}
	void MaterialScriptCompiler2::compileColourOpMultipassFallback(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(2, CE_STRINGEXPECTED)
		++i;

		Ogre::SceneBlendFactor src, dst;
		if(!parseBlendFactor((*i)->token, src))
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		++i;
		if(!parseBlendFactor((*i)->token, dst))
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		++i;

		unitState->setColourOpMultipassFallback(src, dst);
	}
	void MaterialScriptCompiler2::compileAlphaOpEx(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(3, CE_STRINGEXPECTED)
		++i;

		LayerBlendOperationEx op;
        LayerBlendSource src1, src2;
        Real manual = 0.0, alphaSrc1 = 0.0, alphaSrc2 = 0.0;

		// Parse the blend operation
		if(!parseBlendOp((*i)->token, op))
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		++i;

		// Parse the blend source 1
		if(!parseBlendSource((*i)->token, src1))
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		++i;

		// Parse the blend source 2
		if(!parseBlendSource((*i)->token, src2))
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		++i;

		if(i == end)
		{
			unitState->setAlphaOperation(op, src1, src2, alphaSrc1, alphaSrc2, manual);
			return;
		}

		// Get the manual blend amount
		if(op == Ogre::LBX_BLEND_MANUAL)
		{
			if((*i)->type == SNT_NUMBER)
			{
				manual = StringConverter::parseReal((*i)->token);
				++i;
			}
			else
			{
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			}
		}

		// Get the colors for manual modes
		if(src1 == Ogre::LBS_MANUAL)
		{
			if((*i)->type != SNT_NUMBER)
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			else
			{
				alphaSrc1 = StringConverter::parseReal((*i)->token);
				++i;
			}
		}
		if(src2 == Ogre::LBS_MANUAL)
		{
			if((*i)->type != SNT_NUMBER)
				addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			else
			{
				alphaSrc2 = StringConverter::parseReal((*i)->token);
				++i;
			}
		}

		unitState->setAlphaOperation(op, src1, src2, alphaSrc1, alphaSrc2, manual);
	}
	void MaterialScriptCompiler2::compileEnvMap(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;

		if((*i)->token == "off")
		{
			unitState->setEnvironmentMap(false);
			++i;
		}
		else if((*i)->token == "spherical")
		{
			unitState->setEnvironmentMap(true, TextureUnitState::ENV_CURVED);
			++i;
		}
		else if((*i)->token == "planar")
		{
			unitState->setEnvironmentMap(true, TextureUnitState::ENV_PLANAR);
			++i;
		}
		else if((*i)->token == "cubic_reflection")
		{
			unitState->setEnvironmentMap(true, TextureUnitState::ENV_REFLECTION);
			++i;
		}
		else if((*i)->token == "cubic_normal")
		{
			unitState->setEnvironmentMap(true, TextureUnitState::ENV_NORMAL);
			++i;
		}
		else
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
		}
	}
	void MaterialScriptCompiler2::compileScroll(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(2, CE_NUMBEREXPECTED)
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		Real uScroll = StringConverter::parseReal((*i)->token);
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		Real vScroll = StringConverter::parseReal((*i)->token);
		++i;

		unitState->setTextureScroll(uScroll, vScroll);
	}
	void MaterialScriptCompiler2::compileScrollAnim(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(2, CE_NUMBEREXPECTED)
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		Real uScroll = StringConverter::parseReal((*i)->token);
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		Real vScroll = StringConverter::parseReal((*i)->token);
		++i;

		unitState->setScrollAnimation(uScroll, vScroll);
	}
	void MaterialScriptCompiler2::compileRotate(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		unitState->setTextureRotate(Degree(StringConverter::parseReal((*i)->token)));
		++i;
	}
	void MaterialScriptCompiler2::compileRotateAnim(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(1, CE_NUMBEREXPECTED)
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		unitState->setRotateAnimation(StringConverter::parseReal((*i)->token));
		++i;
	}
	void MaterialScriptCompiler2::compileScale(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(2, CE_NUMBEREXPECTED)
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		Real xScale = StringConverter::parseReal((*i)->token);
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		Real yScale = StringConverter::parseReal((*i)->token);
		++i;

		unitState->setTextureScale(xScale, yScale);
	}
	void MaterialScriptCompiler2::compileWaveXForm(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(6, CE_STRINGEXPECTED)
		++i;

		TextureUnitState::TextureTransformType tt;
		Ogre::WaveformType wt;
		Real base = 0.0, freq = 0.0, phase = 0.0, amp = 0.0;

		if(!parseXFormType((*i)->token, tt))
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		++i;

		if(!parseWaveType((*i)->token, wt))
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		base = StringConverter::parseReal((*i)->token);
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		freq = StringConverter::parseReal((*i)->token);
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		phase = StringConverter::parseReal((*i)->token);
		++i;

		if((*i)->type != SNT_NUMBER)
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}
		amp = StringConverter::parseReal((*i)->token);
		++i;

		unitState->setTransformAnimation(tt, wt, base, freq, phase, amp);
	}
	void MaterialScriptCompiler2::compileTransform(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(16, CE_NUMBEREXPECTED)
		++i;
	}
	void MaterialScriptCompiler2::compileBindingType(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;

		TextureUnitState::BindingType type;
		if((*i)->token == "vertex")
			type = TextureUnitState::BT_VERTEX;
		else if((*i)->token == "fragment")
			type = TextureUnitState::BT_FRAGMENT;
		else
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		unitState->setBindingType(type);
		++i;
	}
	void MaterialScriptCompiler2::compileContentType(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, TextureUnitState *unitState)
	{
		REQUIRE(1, CE_STRINGEXPECTED)
		++i;

		TextureUnitState::ContentType type;
		if((*i)->token == "named")
			type = TextureUnitState::CONTENT_NAMED;
		else if((*i)->token == "shadow")
			type = TextureUnitState::CONTENT_SHADOW;
		else
		{
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		unitState->setContentType(type);
		++i;
	}

	void MaterialScriptCompiler2::compileVertexProgram(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		REQUIRE(2, CE_STRINGEXPECTED)
		++i;

		// Declare all of the variables
		GpuProgramType type = Ogre::GPT_VERTEX_PROGRAM;
		String origin = (*i)->file, name, lang, source, syntax, profiles;
		ScriptNodeList::iterator iterDefaultParams = end;
		std::vector<std::pair<String,String> > customParams;
		bool includesSkeletalAnimation = false, includesMorphAnimation = false, usesVertexTextureFetch = false;
		int includesPoseAnimation = 0;
		name = (*i)->token;
		++i;
		lang = (*i)->token;
		++i;

		// The next token should be the '{'
		REQUIRE(1, CE_OPENBRACEEXPECTED)
		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if((*j)->token == "syntax")
			{
				// Check that the next token exists
				ScriptNodeList::iterator k = j;
				++k;
				if(k == (*i)->children.end())
				{
					addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					break;
				}
				syntax = (*k)->token;
				j = k;
				++j;
			}
			else if((*j)->token == "source")
			{
				// Check that the next token exists
				ScriptNodeList::iterator k = j;
				++k;
				if(k == (*i)->children.end())
				{
					addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					break;
				}
				source = (*k)->token;
				j = k;
				++j;
			}
			else if((*j)->token == "default_params")
			{
				iterDefaultParams = j;
				++j; // Consume "default_params"
				if(j != (*i)->children.end()) // Consume '{'
					++j;
				if(j != (*i)->children.end()) // Consume '}'
					++j;
			}
			else if((*j)->token == "profiles")
			{
				++j;
				if(!parseProfiles(j, (*i)->children.end(), profiles))
				{
					addError(CE_INVALIDPROPERTYVALUE, (*j)->file, (*j)->line, (*j)->column);
					break;
				}
			}
			else if((*j)->token == "includes_skeletal_animation")
			{
				// Check that the next token exists
				ScriptNodeList::iterator k = j;
				++k;
				if(k == (*i)->children.end())
				{
					addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					break;
				}
				includesSkeletalAnimation = isTruthValue((*k)->token);
				j = k;
				++j;
			}
			else if((*j)->token == "includes_morph_animation")
			{
				// Check that the next token exists
				ScriptNodeList::iterator k = j;
				++k;
				if(k == (*i)->children.end())
				{
					addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					break;
				}
				includesMorphAnimation = isTruthValue((*k)->token);
				j = k;
				++j;
			}
			else if((*j)->token == "includes_pose_animation")
			{
				// Check that the next token exists
				ScriptNodeList::iterator k = j;
				++k;
				if(k == (*i)->children.end())
				{
					addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					break;
				}
				includesPoseAnimation = StringConverter::parseInt((*k)->token);
				j = k;
				++j;
			}
			else if((*j)->token == "uses_vertex_texture_fetch")
			{
				// Check that the next token exists
				ScriptNodeList::iterator k = j;
				++k;
				if(k == (*i)->children.end())
				{
					addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					break;
				}
				usesVertexTextureFetch = isTruthValue((*k)->token);
				j = k;
				++j;
			}
			else
			{
				String name = (*j)->token, value;

				// Check that the next token exists
				ScriptNodeList::iterator k = j;
				++k;
				if(k == (*i)->children.end())
				{
					addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					break;
				}
				value = (*k)->token;
				customParams.push_back(std::make_pair(name, value));
				j = k;
				++j;
			}
		}

		// Consume the '{' and the '}'
		++i;
		++i;

		// Clear the program
		mProgram = 0;
		mParams.setNull();
	}

	void MaterialScriptCompiler2::compileFragmentProgram(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		REQUIRE(2, CE_STRINGEXPECTED)
		++i;

		// Declare all of the variables
		GpuProgramType type = Ogre::GPT_FRAGMENT_PROGRAM;
		String origin = (*i)->file, name, lang, source, syntax, profiles;
		ScriptNodeList::iterator iterDefaultParams = end;
		std::vector<std::pair<String,String> > customParams;
		name = (*i)->token;
		++i;
		lang = (*i)->token;
		++i;

		// The next token should be the '{'
		REQUIRE(1, CE_OPENBRACEEXPECTED)
		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if((*j)->token == "syntax")
			{
				// Check that the next token exists
				ScriptNodeList::iterator k = j;
				++k;
				if(k == (*i)->children.end())
				{
					addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					break;
				}
				syntax = (*k)->token;
				j = k;
				++j;
			}
			else if((*j)->token == "source")
			{
				// Check that the next token exists
				ScriptNodeList::iterator k = j;
				++k;
				if(k == (*i)->children.end())
				{
					addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					break;
				}
				source = (*k)->token;
				j = k;
				++j;
			}
			else if((*j)->token == "default_params")
			{
				iterDefaultParams = j;
				++j; // Consume "default_params"
				if(j != (*i)->children.end()) // Consume '{'
					++j;
				if(j != (*i)->children.end()) // Consume '}'
					++j;
			}
			else if((*j)->token == "profiles")
			{
				++j;
				if(!parseProfiles(j, (*i)->children.end(), profiles))
				{
					addError(CE_INVALIDPROPERTYVALUE, (*j)->file, (*j)->line, (*j)->column);
					break;
				}
			}
			else
			{
				String name = (*j)->token, value;

				// Check that the next token exists
				ScriptNodeList::iterator k = j;
				++k;
				if(k == (*i)->children.end())
				{
					addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					break;
				}
				value = (*k)->token;
				customParams.push_back(std::make_pair(name, value));
				j = k;
				++j;
			}
		}

		// Consume the '{' and the '}'
		++i;
		++i;

		// Clear the program
		mProgram = 0;
		mParams.setNull();
	}

	void MaterialScriptCompiler2::compileDefaultParameters(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::GpuProgramParametersSharedPtr &params)
	{
		REQUIRE(1, CE_OPENBRACEEXPECTED)
		++i;

		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			
		}

		++i; // Consume the '{'
		++i; // Consume the '}'
	}

	void MaterialScriptCompiler2::compileManualConstant(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::GpuProgramParametersSharedPtr &params)
	{

	}

	void MaterialScriptCompiler2::compileAutoConstant(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::GpuProgramParametersSharedPtr &params)
	{

	}

	bool MaterialScriptCompiler2::parseColour(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::ColourValue &c)
	{
		if(i == end || (*i)->type != SNT_NUMBER)
			return false;

		c.r = StringConverter::parseReal((*i)->token);
		++i;

		if(i != end && (*i)->type == SNT_NUMBER)
		{
			c.g = StringConverter::parseReal((*i)->token);
			++i;

			if(i != end && (*i)->type == SNT_NUMBER)
			{
				c.b = StringConverter::parseReal((*i)->token);
				++i;

				if(i != end && (*i)->type == SNT_NUMBER)
				{
					c.a = StringConverter::parseReal((*i)->token);
					++i;
				}
			}
		}

		return true;
	}

	bool MaterialScriptCompiler2::parseBlendFactor(const String &str, SceneBlendFactor &factor)
	{
		bool retval = true;
		if(str == "dest_colour")
			factor = SBF_DEST_COLOUR;
		else if(str == "src_colour")
			factor = SBF_SOURCE_COLOUR;
		else if(str == "one_minus_dest_colour")
			factor = SBF_ONE_MINUS_DEST_COLOUR;
		else if(str == "one_minus_src_colour")
			factor = SBF_ONE_MINUS_SOURCE_COLOUR;
		else if(str == "dest_alpha")
			factor = SBF_DEST_ALPHA;
		else if(str == "src_alpha")
			factor = SBF_SOURCE_ALPHA;
		else if(str == "one_minus_dest_alpha")
			factor = SBF_ONE_MINUS_DEST_ALPHA;
		else if(str == "one_minus_src_alpha")
			factor = SBF_ONE_MINUS_SOURCE_ALPHA;
		else
			retval = false;
		return retval;
	}

	bool MaterialScriptCompiler2::parseCompareFunction(const Ogre::String &str, Ogre::CompareFunction &func)
	{
		bool retval = true;
		if(str == "always_fail")
			func = CMPF_ALWAYS_FAIL;
		else if(str == "always_pass")
			func = CMPF_ALWAYS_PASS;
		else if(str == "less")
			func = CMPF_LESS;
		else if(str == "less_equal")
			func = CMPF_LESS_EQUAL;
		else if(str == "equal")
			func = CMPF_EQUAL;
		else if(str == "not_equal")
			func = CMPF_NOT_EQUAL;
		else if(str == "greater_equal")
			func = CMPF_GREATER_EQUAL;
		else if(str == "greater")
			func = CMPF_GREATER;
		else
			retval = false;
		return retval;
	}

	bool MaterialScriptCompiler2::parseBlendOp(const Ogre::String &str, Ogre::LayerBlendOperationEx &op)
	{
		bool retval = true;
		if(str == "source1")
			op = Ogre::LBX_SOURCE1;
		else if(str == "source2")
			op = LBX_SOURCE2;
		else if(str == "modulate")
			op = LBX_MODULATE;
		else if(str == "modulate_x2")
			op = Ogre::LBX_MODULATE_X2;
		else if(str == "modulate_x4")
			op = LBX_MODULATE_X4;
		else if(str == "add")
			op = LBX_ADD;
		else if(str == "add_signed")
			op = Ogre::LBX_ADD_SIGNED;
		else if(str == "add_smooth")
			op = LBX_ADD_SMOOTH;
		else if(str == "subtract")
			op = LBX_SUBTRACT;
		else if(str == "blend_diffuse_alpha")
			op = Ogre::LBX_BLEND_DIFFUSE_ALPHA;
		else if(str == "blend_texture_alpha")
			op = LBX_BLEND_TEXTURE_ALPHA;
		else if(str == "blend_current_alpha")
			op = LBX_BLEND_CURRENT_ALPHA;
		else if(str == "blend_manual")
			op = LBX_BLEND_MANUAL;
		else if(str == "dotproduct")
			op = LBX_DOTPRODUCT;
		else if(str == "blend_diffuse_colour")
			op = LBX_BLEND_DIFFUSE_COLOUR;
		else
			retval = false;
		return retval;
	}

	bool MaterialScriptCompiler2::parseBlendSource(const Ogre::String &str, Ogre::LayerBlendSource &source)
	{
		bool retval = true;
		if(str == "src_current")
			source = Ogre::LBS_CURRENT;
		else if(str == "src_texture")
			source = LBS_TEXTURE;
		else if(str == "src_diffuse")
			source = LBS_DIFFUSE;
		else if(str == "src_specular")
			source = LBS_SPECULAR;
		else if(str == "src_manual")
			source = LBS_MANUAL;
		return retval;
	}

	bool MaterialScriptCompiler2::parseXFormType(const String &str, TextureUnitState::TextureTransformType &type)
	{
		bool retval = true;
		if(str == "scroll_x")
			type = TextureUnitState::TT_TRANSLATE_U;
		else if(str == "scroll_y")
			type = TextureUnitState::TT_TRANSLATE_V;
		else if(str == "rotate")
			type = TextureUnitState::TT_ROTATE;
		else if(str == "scale_x")
			type = TextureUnitState::TT_SCALE_U;
		else if(str == "scale_y")
			type = TextureUnitState::TT_SCALE_V;
		else
			retval = false;
		return retval;
	}

	bool MaterialScriptCompiler2::parseWaveType(const Ogre::String &str, Ogre::WaveformType &wave)
	{
		bool retval = true;
		if(str == "sine")
			wave = Ogre::WFT_SINE;
		else if(str == "triangle")
			wave = WFT_TRIANGLE;
		else if(str == "square")
			wave = WFT_SQUARE;
		else if(str == "sawtooth")
			wave = WFT_SAWTOOTH;
		else if(str == "inverse_sawtooth")
			wave = WFT_INVERSE_SAWTOOTH;
		else
			retval = false;
		return retval;
	}

	bool MaterialScriptCompiler2::parseProfiles(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, String &profiles)
	{
		while(i != end && mSyntaxCodes.find((*i)->token) != mSyntaxCodes.end())
		{
			if(profiles.empty())
				profiles = (*i)->token;
			else
				profiles = profiles + " " + (*i)->token;
			++i;
		}

		return !profiles.empty();
	}
}

#undef REQUIRE


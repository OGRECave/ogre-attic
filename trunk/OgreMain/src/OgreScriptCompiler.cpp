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
#include "OgreLogManager.h"

namespace Ogre{

	// ScriptCompilerListener
	ScriptNodeListPtr ScriptCompiler::Listener::importFile(const String &name)
	{
		return ScriptNodeListPtr();
	}

	void ScriptCompiler::Listener::preParse(Ogre::WordIDMap &ids, ObjectIDSet &obj)
	{

	}

	bool ScriptCompiler::Listener::errorRaised(const ScriptCompiler::ErrorPtr &error)
	{
		return true;
	}

	bool ScriptCompiler::Listener::overrideNode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		return false;
	}

	Material *ScriptCompiler::Listener::getMaterial(const String &name, const String &group)
	{
		return (Material*)MaterialManager::getSingleton().create(name, group).get();
	}

	void ScriptCompiler::Listener::preApplyTextureAliases(std::map<String,String> &aliases)
	{
	}

	GpuProgram *ScriptCompiler::Listener::getGpuProgram(const String &name, const String &group, const String &source, GpuProgramType type, const String &syntax)
	{
		return (GpuProgram*)GpuProgramManager::getSingleton().createProgram(name, group, source, type, syntax).get();
	}

	HighLevelGpuProgram *ScriptCompiler::Listener::getHighLevelGpuProgram(const String &name, const String &group, const String &language, GpuProgramType type, const String &source)
	{
		return (HighLevelGpuProgram*)HighLevelGpuProgramManager::getSingleton().createProgram(name, group, language, type).get();
	}

	void ScriptCompiler::Listener::getGpuProgramName(String &name)
	{
	}

	ParticleSystem *ScriptCompiler::Listener::getParticleSystem(const String &name, const String &group)
	{
		return (ParticleSystem*)ParticleSystemManager::getSingleton().createTemplate(name, group);
	}

	void ScriptCompiler::Listener::getMaterialName(String &name)
	{
	}
	
	Compositor *ScriptCompiler::Listener::getCompositor(const String &name, const String &group)
	{
		return (Compositor*)CompositorManager::getSingleton().create(name, group).get();
	}

	void ScriptCompiler::Listener::getTextures(String *names, int count)
	{
	}

	// ScriptCompiler
	/// This enum defines the integer ids for keywords this compiler handles
	enum
	{
		ID_MATERIAL,
		ID_VERTEX_PROGRAM,
		ID_FRAGMENT_PROGRAM,
		ID_TECHNIQUE,
		ID_PASS,
		ID_TEXTURE_UNIT,
		ID_VERTEX_PROGRAM_REF,
		ID_FRAGMENT_PROGRAM_REF,
		ID_SHADOW_CASTER_VERTEX_PROGRAM_REF,
		ID_SHADOW_RECEIVER_VERTEX_PROGRAM_REF,
		ID_SHADOW_RECEIVER_FRAGMENT_PROGRAM_REF,
		
		ID_LOD_DISTANCES,
		ID_RECEIVE_SHADOWS,
		ID_TRANSPARENCY_CASTS_SHADOWS,
		ID_SET_TEXTURE_ALIAS,

		ID_SOURCE,
		ID_SYNTAX,
		ID_DEFAULT_PARAMS,
		ID_PARAM_INDEXED,
		ID_PARAM_NAMED,
		ID_PARAM_INDEXED_AUTO,
		ID_PARAM_NAMED_AUTO,

		ID_SCHEME,
		ID_LOD_INDEX,

		ID_AMBIENT,
		ID_DIFFUSE,
		ID_SPECULAR,
		ID_EMISSIVE,
			ID_VERTEX_COLOUR,
		ID_SCENE_BLEND,
			ID_COLOUR_BLEND,
			ID_ONE,
			ID_ZERO,
			ID_DEST_COLOUR,
			ID_SRC_COLOUR,
			ID_ONE_MINUS_DEST_COLOUR,
			ID_ONE_MINUS_SRC_COLOUR,
			ID_DEST_ALPHA,
			ID_SRC_ALPHA,
			ID_ONE_MINUS_DEST_ALPHA,
			ID_ONE_MINUS_SRC_ALPHA,
		ID_SEPARATE_SCENE_BLEND,
		ID_DEPTH_CHECK,
		ID_DEPTH_WRITE,
		ID_DEPTH_FUNC,
		ID_DEPTH_BIAS,
		ID_ITERATION_DEPTH_BIAS,
			ID_ALWAYS_FAIL,
			ID_ALWAYS_PASS,
			ID_LESS_EQUAL,
			ID_LESS,
			ID_EQUAL,
			ID_NOT_EQUAL,
			ID_GREATER_EQUAL,
			ID_GREATER,
		ID_ALPHA_REJECTION,
		ID_LIGHT_SCISSOR,
		ID_LIGHT_CLIP_PLANES,
		ID_ILLUMINATION_STAGE,
			ID_DECAL,
		ID_CULL_HARDWARE,
			ID_CLOCKWISE,
			ID_ANTICLOCKWISE,
		ID_CULL_SOFTWARE,
			ID_BACK,
			ID_FRONT,
		ID_NORMALISE_NORMALS,
		ID_LIGHTING,
		ID_SHADING,
			ID_FLAT, 
			ID_GOURAUD,
			ID_PHONG,
		ID_POLYGON_MODE,
			ID_SOLID,
			ID_WIREFRAME,
			ID_POINTS,
		ID_POLYGON_MODE_OVERRIDEABLE,
		ID_FOG_OVERRIDE,
			ID_NONE,
			ID_LINEAR,
			ID_EXP,
			ID_EXP2,
		ID_COLOUR_WRITE,
		ID_MAX_LIGHTS,
		ID_START_LIGHT,
		ID_ITERATION,
			ID_ONCE,
			ID_ONCE_PER_LIGHT,
			ID_PER_LIGHT,
			ID_PER_N_LIGHTS,
			ID_POINT,
			ID_SPOT,
			ID_DIRECTIONAL,
		ID_POINT_SIZE,
		ID_POINT_SPRITES,
		ID_POINT_SIZE_ATTENUATION,
		ID_POINT_SIZE_MIN,
		ID_POINT_SIZE_MAX,

		ID_TEXTURE_ALIAS,
		ID_TEXTURE,
			ID_1D,
			ID_2D,
			ID_3D,
			ID_CUBIC,
			ID_UNLIMITED,
			ID_ALPHA,
		ID_ANIM_TEXTURE,
		ID_CUBIC_TEXTURE,
			ID_SEPARATE_UV,
			ID_COMBINED_UVW,
		ID_TEX_COORD_SET,
		ID_TEX_ADDRESS_MODE,
			ID_WRAP,
			ID_CLAMP,
			ID_BORDER,
			ID_MIRROR,
		ID_TEX_BORDER_COLOUR,
		ID_FILTERING,
			ID_BILINEAR,
			ID_TRILINEAR,
			ID_ANISOTROPIC,
		ID_MAX_ANISOTROPY,
		ID_MIPMAP_BIAS,
		ID_COLOUR_OP,
			ID_REPLACE,
			ID_ADD,
			ID_MODULATE,
			ID_ALPHA_BLEND,
		ID_COLOUR_OP_EX,
			ID_SOURCE1,
			ID_SOURCE2,
			ID_MODULATE_X2,
			ID_MODULATE_X4,
			ID_ADD_SIGNED,
			ID_ADD_SMOOTH,
			ID_SUBTRACT,
			ID_BLEND_DIFFUSE_COLOUR,
			ID_BLEND_DIFFUSE_ALPHA,
			ID_BLEND_TEXTURE_ALPHA,
			ID_BLEND_CURRENT_ALPHA,
			ID_BLEND_MANUAL,
			ID_DOT_PRODUCT,
			ID_SRC_CURRENT,
			ID_SRC_TEXTURE,
			ID_SRC_DIFFUSE,
			ID_SRC_SPECULAR,
			ID_SRC_MANUAL,
		ID_COLOUR_OP_MULTIPASS_FALLBACK,
		ID_ALPHA_OP_EX,
		ID_ENV_MAP,
			ID_SPHERICAL,
			ID_PLANAR,
			ID_CUBIC_REFLECTION,
			ID_CUBIC_NORMAL,
		ID_SCROLL,
		ID_SCROLL_ANIM,
		ID_ROTATE,
		ID_ROTATE_ANIM,
		ID_SCALE,
		ID_WAVE_XFORM,
			ID_SCROLL_X,
			ID_SCROLL_Y,
			ID_SCALE_X,
			ID_SCALE_Y,
			ID_SINE,
			ID_TRIANGLE,
			ID_SQUARE,
			ID_SAWTOOTH,
			ID_INVERSE_SAWTOOTH,
		ID_TRANSFORM,
		ID_BINDING_TYPE,
			ID_VERTEX,
			ID_FRAGMENT,
		ID_CONTENT_TYPE,
			ID_NAMED,
			ID_SHADOW,

		ID_PARTICLE_SYSTEM,
		ID_EMITTER,
		ID_AFFECTOR,

		ID_COMPOSITOR,
			ID_TARGET,
			ID_TARGET_OUTPUT,

			ID_INPUT,
				ID_PREVIOUS,
				ID_TARGET_WIDTH,
				ID_TARGET_HEIGHT,
			ID_ONLY_INITIAL,
			ID_VISIBILITY_MASK,
			ID_LOD_BIAS,
			ID_MATERIAL_SCHEME,

			ID_CLEAR,
			ID_STENCIL,
			ID_RENDER_SCENE,
			ID_RENDER_QUAD,
			ID_IDENTIFIER,
			ID_FIRST_RENDER_QUEUE,
			ID_LAST_RENDER_QUEUE,

			ID_BUFFERS,
				ID_COLOUR,
				ID_DEPTH,
			ID_COLOUR_VALUE,
			ID_DEPTH_VALUE,
			ID_STENCIL_VALUE,

			ID_CHECK,
			ID_COMP_FUNC,
			ID_REF_VALUE,
			ID_MASK,
			ID_FAIL_OP,
				ID_KEEP,
				ID_INCREMENT,
				ID_DECREMENT,
				ID_INCREMENT_WRAP,
				ID_DECREMENT_WRAP,
				ID_INVERT,
			ID_DEPTH_FAIL_OP,
			ID_PASS_OP,
			ID_TWO_SIDED
	};

	ScriptCompiler::ScriptCompiler()
		:mListener(0), mMaterial(0), mSystem(0), mCompositor(0)
	{
		mWordIDs["on"] = ID_ON;
		mWordIDs["off"] = ID_OFF;
		mWordIDs["true"] = ID_TRUE;
		mWordIDs["false"] = ID_FALSE;
		mWordIDs["yes"] = ID_YES;
		mWordIDs["no"] = ID_NO;

		// Material ids
		mObjectIDs.insert("material");
		mObjectIDs.insert("vertex_program");
		mObjectIDs.insert("fragment_program");
		mObjectIDs.insert("default_params");
		mObjectIDs.insert("technique");
		mObjectIDs.insert("pass");
		mObjectIDs.insert("texture_unit");
		mObjectIDs.insert("vertex_program_ref");
		mObjectIDs.insert("fragment_program_ref");
		mObjectIDs.insert("shadow_caster_vertex_program_ref");
		mObjectIDs.insert("shadow_receiver_vertex_program_ref");
		mObjectIDs.insert("shadow_receiver_fragment_program_ref");

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

		mWordIDs["texture_alias"] = ID_TEXTURE_ALIAS;
		mWordIDs["texture"] = ID_TEXTURE;
			mWordIDs["1d"] = ID_1D;
			mWordIDs["2d"] = ID_2D;
			mWordIDs["3d"] = ID_3D;
			mWordIDs["cubic"] = ID_CUBIC;
			mWordIDs["unlimited"] = ID_UNLIMITED;
			mWordIDs["alpha"] = ID_ALPHA;
		mWordIDs["anim_texture"] = ID_ANIM_TEXTURE;
		mWordIDs["cubic_texture"] = ID_CUBIC_TEXTURE;
			mWordIDs["separateUV"] = ID_SEPARATE_UV;
			mWordIDs["combinedUVW"] = ID_COMBINED_UVW;
		mWordIDs["tex_coord_set"] = ID_TEX_COORD_SET;
		mWordIDs["tex_address_mode"] = ID_TEX_ADDRESS_MODE;
			mWordIDs["wrap"] = ID_WRAP;
			mWordIDs["clamp"] = ID_CLAMP;
			mWordIDs["mirror"] = ID_MIRROR;
			mWordIDs["border"] = ID_BORDER;
		mWordIDs["filtering"] = ID_FILTERING;
			mWordIDs["bilinear"] = ID_BILINEAR;
			mWordIDs["trilinear"] = ID_TRILINEAR;
			mWordIDs["anisotropic"] = ID_ANISOTROPIC;
		mWordIDs["max_anisotropy"] = ID_MAX_ANISOTROPY;
		mWordIDs["mipmap_bias"] = ID_MIPMAP_BIAS;
		mWordIDs["colour_op"] = ID_COLOUR_OP;
			mWordIDs["replace"] = ID_REPLACE;
			mWordIDs["add"] = ID_ADD;
			mWordIDs["modulate"] = ID_MODULATE;
			mWordIDs["alpha_blend"] = ID_ALPHA_BLEND;
		mWordIDs["colour_op_ex"] = ID_COLOUR_OP_EX;
			mWordIDs["source1"] = ID_SOURCE1;
			mWordIDs["source2"] = ID_SOURCE2;
			mWordIDs["modulate"] = ID_MODULATE;
			mWordIDs["modulate_x2"] = ID_MODULATE_X2;
			mWordIDs["modulate_x4"] = ID_MODULATE_X4;
			mWordIDs["add_signed"] = ID_ADD_SIGNED;
			mWordIDs["add_smooth"] = ID_ADD_SMOOTH;
			mWordIDs["blend_diffuse_alpha"] = ID_BLEND_DIFFUSE_ALPHA;
			mWordIDs["blend_texture_alpha"] = ID_BLEND_TEXTURE_ALPHA;
			mWordIDs["blend_current_alpha"] = ID_BLEND_CURRENT_ALPHA;
			mWordIDs["blend_manual"] = ID_BLEND_MANUAL;
			mWordIDs["dotproduct"] = ID_DOT_PRODUCT;
			mWordIDs["blend_diffuse_colour"] = ID_BLEND_DIFFUSE_COLOUR;
			mWordIDs["src_current"] = ID_SRC_CURRENT;
			mWordIDs["src_texture"] = ID_SRC_TEXTURE;
			mWordIDs["src_diffuse"] = ID_SRC_DIFFUSE;
			mWordIDs["src_specular"] = ID_SRC_SPECULAR;
			mWordIDs["src_manual"] = ID_SRC_MANUAL;
		mWordIDs["colour_op_multipass_fallback"] = ID_COLOUR_OP_MULTIPASS_FALLBACK;
		mWordIDs["alpha_op_ex"] = ID_ALPHA_OP_EX;
		mWordIDs["env_map"] = ID_ENV_MAP;
			mWordIDs["spherical"] = ID_SPHERICAL;
			mWordIDs["planar"] = ID_PLANAR;
			mWordIDs["cubic_reflection"] = ID_CUBIC_REFLECTION;
			mWordIDs["cubic_normal"] = ID_CUBIC_NORMAL;
		mWordIDs["scroll"] = ID_SCROLL;
		mWordIDs["scroll_anim"] = ID_SCROLL_ANIM;
		mWordIDs["rotate"] = ID_ROTATE;
		mWordIDs["rotate_anim"] = ID_ROTATE_ANIM;
		mWordIDs["scale"] = ID_SCALE;
		mWordIDs["wave_xform"] = ID_WAVE_XFORM;
			mWordIDs["scroll_x"] = ID_SCROLL_X;
			mWordIDs["scroll_y"] = ID_SCROLL_Y;
			mWordIDs["scale_x"] = ID_SCALE_X;
			mWordIDs["scale_y"] = ID_SCALE_Y;
			mWordIDs["sine"] = ID_SINE;
			mWordIDs["triangle"] = ID_TRIANGLE;
			mWordIDs["sawtooth"] = ID_SAWTOOTH;
			mWordIDs["square"] = ID_SQUARE;
			mWordIDs["inverse_sawtooth"] = ID_INVERSE_SAWTOOTH;
		mWordIDs["transform"] = ID_TRANSFORM;
		mWordIDs["binding_type"] = ID_BINDING_TYPE;
			mWordIDs["vertex"] = ID_VERTEX;
			mWordIDs["fragment"] = ID_FRAGMENT;
		mWordIDs["content_type"] = ID_CONTENT_TYPE;
			mWordIDs["named"] = ID_NAMED;
			mWordIDs["shadow"] = ID_SHADOW;

		// Particle system
		mObjectIDs.insert("particle_system");
		mObjectIDs.insert("emitter");
		mObjectIDs.insert("affector");

		mWordIDs["particle_system"] = ID_PARTICLE_SYSTEM;
		mWordIDs["emitter"] = ID_EMITTER;
		mWordIDs["affector"] = ID_AFFECTOR;

		// Compositor
		mObjectIDs.insert("compositor");
		mObjectIDs.insert("target");
		mObjectIDs.insert("target_output");
		mObjectIDs.insert("clear");
		mObjectIDs.insert("stencil");

		mWordIDs["compositor"] = ID_COMPOSITOR;
		mWordIDs["target"] = ID_TARGET;
		mWordIDs["target_output"] = ID_TARGET_OUTPUT;

		mWordIDs["input"] = ID_INPUT;
			mWordIDs["none"] = ID_NONE;
			mWordIDs["previous"] = ID_PREVIOUS;
			mWordIDs["target_width"] = ID_TARGET_WIDTH;
			mWordIDs["target_height"] = ID_TARGET_HEIGHT;
		mWordIDs["only_initial"] = ID_ONLY_INITIAL;
		mWordIDs["visibility_mask"] = ID_VISIBILITY_MASK;
		mWordIDs["lod_bias"] = ID_LOD_BIAS;
		mWordIDs["material_scheme"] = ID_MATERIAL_SCHEME;

		mWordIDs["clear"] = ID_CLEAR;
		mWordIDs["stencil"] = ID_STENCIL;
		mWordIDs["render_scene"] = ID_RENDER_SCENE;
		mWordIDs["render_quad"] = ID_RENDER_QUAD;
		mWordIDs["identifier"] = ID_IDENTIFIER;
		mWordIDs["first_render_queue"] = ID_FIRST_RENDER_QUEUE;
		mWordIDs["last_render_queue"] = ID_LAST_RENDER_QUEUE;

		mWordIDs["buffers"] = ID_BUFFERS;
			mWordIDs["colour"] = ID_COLOUR;
			mWordIDs["depth"] = ID_DEPTH;
		mWordIDs["colour_value"] = ID_COLOUR_VALUE;
		mWordIDs["depth_value"] = ID_DEPTH_VALUE;
		mWordIDs["stencil_value"] = ID_STENCIL_VALUE;

		mWordIDs["check"] = ID_CHECK;
		mWordIDs["comp_func"] = ID_COMP_FUNC;
		mWordIDs["ref_value"] = ID_REF_VALUE;
		mWordIDs["mask"] = ID_MASK;
		mWordIDs["fail_op"] = ID_FAIL_OP;
			mWordIDs["keep"] = ID_KEEP;
			mWordIDs["increment"] = ID_INCREMENT;
			mWordIDs["decrement"] = ID_DECREMENT;
			mWordIDs["increment_wrap"] = ID_INCREMENT_WRAP;
			mWordIDs["decrement_wrap"] = ID_DECREMENT_WRAP;
			mWordIDs["invert"] = ID_INVERT;
		mWordIDs["depth_fail_op"] = ID_DEPTH_FAIL_OP;
		mWordIDs["pass_op"] = ID_PASS_OP;
		mWordIDs["two_sided"] = ID_TWO_SIDED;
	}

	bool ScriptCompiler::compile(const String &text, const String &group, const String &source)
	{
		// Before parsing, allow a listener to modify the word id map
		preParse();

		// Delegate to the other overload of this function
		return compile(parse(text, source, mWordIDs, mObjectIDs), group);
	}

	/*
	static void print(ScriptNodeList &nodes, int tab)
	{
		String tabstr = "";
		for(int i = 0; i < tab; ++i)
			tabstr += "\t";
		for(ScriptNodeList::iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			String str = tabstr + (*i)->token + " (" + (*i)->file + ")";
			Ogre::LogManager::getSingleton().logMessage(str);
			print((*i)->children, tab + 1);
		}
	}
	*/

	bool ScriptCompiler::compile(ScriptNodeListPtr nodes, const String &group)
	{
		// Set up the context
		mGroup = group;

		// Set up the import mechanisms
		mImportTable.clear();
		mImports.clear();
		if(!nodes->empty())
			mImports.insert(std::make_pair(nodes->front()->file, ScriptNodeListPtr()));

		// Clear errors
		mErrors.clear();

		// Set up the global scope, the top level
		mStack.clear();
		pushScope();

		processImports(nodes);
		processObjects(*nodes.get(), nodes);
		processVariables(*nodes.get(), nodes);
		//print(*nodes.get(), 0);
		return compileImpl(nodes);
	}

	bool ScriptCompiler::compile(DataStreamPtr &stream, const String &group)
	{
		return compile(stream->getAsString(), group, stream->getName());
	}

	const String &ScriptCompiler::getGroup() const
	{
		return mGroup;
	}

	const ScriptCompiler::ErrorList &ScriptCompiler::getErrors() const
	{
		return mErrors;
	}

	const WordIDMap &ScriptCompiler::getWordIDs() const
	{
		return mWordIDs;
	}

	void ScriptCompiler::setListener(Listener *listener)
	{
		mListener = listener;
	}

	bool ScriptCompiler::compileImpl(const ScriptNodeListPtr &nodes)
	{
		ScriptNodeList::iterator i = nodes->begin();
		while(i != nodes->end())
		{
			// Delegate some processing to the listener
			if(!overrideNode(i, nodes->end()))
			{
				if((*i)->wordID == ID_MATERIAL)
					compileMaterial(*i);
				else if((*i)->wordID == ID_VERTEX_PROGRAM || (*i)->wordID == ID_FRAGMENT_PROGRAM)
					compileGpuProgram(*i);
				else if((*i)->wordID == ID_PARTICLE_SYSTEM)
					compileParticleSystem(*i);
				else if((*i)->wordID == ID_COMPOSITOR)
					compileCompositor(*i);
				++i;
			}
		}

		return mErrors.empty();
	}

	bool ScriptCompiler::overrideNode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		return mListener ? mListener->overrideNode(i, end) : false;
	}

	void ScriptCompiler::processVariables(ScriptNodeList &nodes, const ScriptNodeListPtr &top)
	{
		// First we process the current level for variable assignments
		ScriptNodeList::iterator i = nodes.begin(), end = nodes.end(), cur;
		while(i != end)
		{
			cur = i++;
			if((*cur)->type == SNT_VARIABLE_ASSIGN)
			{
				// There should be two children, name and value
				ScriptNodePtr next1 = getNodeAt((*cur)->children.begin(), (*cur)->children.end(), 0),
					next2 = getNodeAt((*cur)->children.begin(), (*cur)->children.end(), 1);
				if(next1.isNull())
				{
					addError(CE_VARIABLEEXPECTED, (*cur)->file, (*cur)->line, (*cur)->column);
					continue;
				}
				else if(next1->type != SNT_VARIABLE)
				{
					addError(CE_VARIABLEEXPECTED, next1->file, next1->line, next1->column);
					continue;
				}
				if(next2.isNull())
				{
					addError(CE_VARIABLEVALUEEXPECTED, (*cur)->file, (*cur)->line, (*cur)->column);
					continue;
				}
				else if(next2->type != SNT_QUOTE)
				{
					addError(CE_VARIABLEVALUEEXPECTED, next1->file, next1->line, next1->column);
					continue;
				}

				// Insert this string into the current scope
				setVariable(next1->token, next2->token);

				// Remove the variable assignment node
				nodes.erase(cur);
			}
		}

		// Now handle variable references
		i = nodes.begin(), end = nodes.end();
		while(i != end)
		{
			ScriptNodeList::iterator cur = i++;

			// Handle the use of variables
			bool currentValid = true;
			if((*cur)->type == SNT_VARIABLE)
			{
				// Look up the value of the variable in the current scope
				std::pair<bool,String> var = findVariable((*cur)->token);

				// If the variable is defined, then process its value
				if(var.first)
				{
					// 1. parse, 2. inheritance, 3. variables
					ScriptNodeListPtr newNodes = parseChunk(var.second, (*cur)->file, mWordIDs);
					if(!newNodes.isNull() && !newNodes->empty())
					{
						processObjects(*newNodes.get(), top);
						processVariables(*newNodes.get(), top);
					}

					// Insert the AST in place of the variable usage
					if(!newNodes.isNull() && !newNodes->empty())
						nodes.insert(cur, newNodes->begin(), newNodes->end());

					// Remove the current node
					nodes.erase(cur);
					currentValid = false;
				}
				else
					addError(CE_UNDEFINEDVARIABLE, (*cur)->file, (*cur)->line, (*cur)->column);
			}
			else if((*cur)->type == SNT_LBRACE)
			{
				pushScope();
			}
			else if((*cur)->type == SNT_RBRACE)
			{
				popScope();
			}

			// Descend into children first
			if(currentValid && !(*cur)->children.empty() && (*cur)->token != "abstract")
				processVariables((*cur)->children, top);
		}
	}

	void ScriptCompiler::processObjects(ScriptNodeList &nodes, const ScriptNodeListPtr &top)
	{
		ScriptNodeList::iterator i = nodes.begin();
		while(i != nodes.end())
		{
			ScriptNodeList::iterator cur = i++;

			// Recurse so that children can take advantage of inheritance too
			if(!(*cur)->children.empty())
				processObjects((*cur)->children, top);

			if((*cur)->type == SNT_COLON) // This is the indicator that inheritance is occuring
			{
				// The child of this node is the name of the inherited object
				// The next node should be a '{'
				ScriptNodePtr target = getNodeAt((*cur)->children.begin(), (*cur)->children.end(), 0), 
					brace = getNodeAt(cur, nodes.end(), 1);

				if(target.isNull())
				{
					addError(CE_OBJECTNAMEEXPECTED, (*cur)->file, (*cur)->line, -1);
					continue;
				}
				if(brace.isNull())
				{
					addError(CE_OPENBRACEEXPECTED, (*cur)->file, (*cur)->line, -1);
					continue;
				}
				else if(brace->type != SNT_LBRACE)
				{
					addError(CE_OPENBRACEEXPECTED, brace->file, brace->line, brace->column);
					continue;
				}

				// Remove the ':', along with the inherited object name
				nodes.erase(cur);

				// Use the locateImportTarget function to extract the AST of the inherited object
				// Check the top-level section of the script first
				ScriptNodeListPtr newNodes = locateTarget(*top.get(), target->token);
				// Check the import table if nothing was found
				if(newNodes.isNull() || newNodes->empty())
					newNodes = locateTarget(mImportTable, target->token);
				if(!newNodes.isNull() && !newNodes->empty())
				{
					// Find the '{', since we only copy the content of the inherited object
					ScriptNodePtr obj = *newNodes->begin();
					ScriptNodeList::iterator braceIter = 
						findNode(obj->children.begin(), obj->children.end(), SNT_LBRACE);
					if(braceIter !=  obj->children.end())
					{
						overlayObject(*braceIter, brace);
						//brace->children.insert(brace->children.begin(), 
						//	(*braceIter)->children.begin(), (*braceIter)->children.end());
					}
				}
			}
		}
	}

	void ScriptCompiler::processImports(ScriptNodeListPtr &nodes)
	{
		// We only need to iterate over the top-level of nodes
		ScriptNodeList::iterator i = nodes->begin(), last = nodes->end();
		while(i != nodes->end())
		{
			// We move to the next node here and save the current one.
			// If any replacement happens, then we are still assured that
			// i points to the node *after* the replaced nodes, no matter
			// how many insertions and deletions may happen
			ScriptNodeList::iterator cur = i++;
			if((*cur)->type == SNT_IMPORT)
			{
				// There should be two nodes below this one
				ScriptNodePtr importTarget = getNodeAt((*cur)->children.begin(), (*cur)->children.end(), 0),
					importPath = getNodeAt((*cur)->children.begin(), (*cur)->children.end(), 1);

				// Remove this import statement
				nodes->erase(cur);

				// Only process if the file's contents haven't been loaded
				if(mImports.find(importPath->token) == mImports.end())
				{
					// Load the script
					ScriptNodeListPtr importedNodes = loadImportPath(importPath->token);
					if(!importedNodes.isNull() && !importedNodes->empty())
					{
						processImports(importedNodes);
						processObjects(*importedNodes.get(), importedNodes);
					}
					if(!importedNodes.isNull() && !importedNodes->empty())
						mImports.insert(std::make_pair(importPath->token, importedNodes));
				}

				// Handle the target request now
				// If it is a '*' import we remove all previous requests and just use the '*'
				// Otherwise, ensure '*' isn't already registered and register our request
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
					// Insert the entire AST into the import table
					mImportTable.insert(mImportTable.begin(), i->second->begin(), i->second->end());
					continue; // Skip ahead to the next file
				}
				else
				{
					for(; j != end; ++j)
					{
						// Locate this target and insert it into the import table
						ScriptNodeListPtr newNodes = locateTarget(*(i->second.get()), j->second);
						if(!newNodes.isNull() && !newNodes->empty())
							mImportTable.insert(mImportTable.begin(), newNodes->begin(), newNodes->end());
					}
				}
			}
		}
	}

	ScriptNodeListPtr ScriptCompiler::loadImportPath(const Ogre::String &name)
	{
		ScriptNodeListPtr nodes;

		if(mListener)
			nodes = mListener->importFile(name);

		if(nodes.isNull() && ResourceGroupManager::getSingletonPtr())
		{
			DataStreamPtr stream = ResourceGroupManager::getSingleton().openResource(name, mGroup);
			if(!stream.isNull())
			{
				nodes = parse(stream->getAsString(), name);
			}
		}

		return nodes;
	}

	ScriptNodeListPtr ScriptCompiler::locateTarget(ScriptNodeList &nodes, const String &target)
	{
		ScriptNodeList::iterator iter = nodes.end();
	
		// Search for a top-level object node
		for(ScriptNodeList::iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			if((*i)->isObject)
			{
				if((*i)->token == "abstract")
				{
					// We're looking for the second child now
					ScriptNodeList::iterator j = (*i)->children.begin();
					if(j != (*i)->children.end())
						++j;
					if(j != (*i)->children.end() && (*j)->token == target)
					{
						iter = i;
						break;
					}
				}
				else
				{
					if(!(*i)->children.empty() && (*(*i)->children.begin())->token == target)
					{
						iter = i;
						break;
					}
				}
				
			}
		}

		ScriptNodeListPtr newNodes(new ScriptNodeList());
		if(iter != nodes.end())
		{
			newNodes->push_back(*iter);
		}
		return newNodes;
	}

	void ScriptCompiler::preParse()
	{
		if(mListener)
			mListener->preParse(mWordIDs, mObjectIDs);
	}

	bool ScriptCompiler::errorRaised(const ErrorPtr &err)
	{
		return mListener ? mListener->errorRaised(err) : true;
	}	

	bool ScriptCompiler::containsObject(const ScriptNodeList &nodes, const String &name)
	{
		// Search for a top-level object node
		for(ScriptNodeList::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
		{
			if((*i)->isObject)
			{
				if(!(*i)->children.empty() && (*(*i)->children.begin())->token == name)
				{
					return true;
				}
			}
		}
		return false;
	}

	void ScriptCompiler::overlayObject(const ScriptNodePtr &source, ScriptNodePtr &dest)
	{
		ScriptNodeList::iterator insertPos = dest->children.begin();
		int pos = 0;

		// Iterate through, copying all those that aren't objects, and overlaying those that are
		for(ScriptNodeList::const_iterator i = source->children.begin(); i != source->children.end(); ++i)
		{
			if((*i)->isObject)
			{
				if(pos >= 0)
				{
					// Attemp to find the pos-th object in the child
					// Which is of the same type as the current one
					int n = 0;
					ScriptNodeList::iterator iter = dest->children.end();
					for(ScriptNodeList::iterator j = dest->children.begin(); j != dest->children.end(); ++j)
					{
						if((*j)->isObject)
						{
							if(n == pos)
							{
								if((*j)->token == (*i)->token)
								{
									iter = j;
								}
								break;
							}
							else
								n++;
						}
					}

					// If the target node was not found, the pattern of overrides is broken,
					// which means there is no more overriding allowed in this object definition
					if(iter != dest->children.end())
					{
						// Find the '{' in both source and destination
						ScriptNodeList::iterator sourceBrace = findNode((*i)->children.begin(), (*i)->children.end(), SNT_LBRACE),
							destBrace = findNode((*iter)->children.begin(), (*iter)->children.end(), SNT_LBRACE);
						if(sourceBrace != (*i)->children.end() && destBrace != (*iter)->children.end())
						{
							overlayObject(*sourceBrace, *destBrace);
						}
					}
					else
					{
						pos = -1;
					}
				}

				// If we get into this condition, then we are just copying object definitions into the child
				if(pos == -1)
				{
					dest->children.insert(insertPos, copyNode(*i));
				}
				else
				{
					pos++;
				}
			}
			else
			{
				dest->children.insert(insertPos, copyNode(*i));
			}
		}
	}

	ScriptNodePtr ScriptCompiler::copyNode(const ScriptNodePtr &node)
	{
		ScriptNodePtr retval(new ScriptNode());
		retval->column = node->column;
		retval->data = node->data;
		retval->file = node->file;
		retval->isObject = node->isObject;
		retval->isProperty = node->isProperty;
		retval->line = node->line;
		retval->token = node->token;
		retval->type = node->type;
		retval->wordID = node->wordID;

		// Perform the copy operation on the children as well
		for(ScriptNodeList::iterator i = node->children.begin(); i != node->children.end(); ++i)
		{
			retval->children.push_back(copyNode(*i));
		}

		return retval;
	}

	ScriptNodePtr ScriptCompiler::getNodeAt(ScriptNodeList::const_iterator &from, ScriptNodeList::const_iterator &end, int index) const
	{
		int n = 0;
		for(ScriptNodeList::const_iterator i = from; i != end; ++i, ++n)
		{
			if(n == index)
				return *i;
		}
		return ScriptNodePtr();
	}

	ScriptNodeList::iterator ScriptCompiler::findNode(ScriptNodeList::iterator &from, ScriptNodeList::iterator &to, uint32 type) const
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

	ScriptNodeList::const_iterator ScriptCompiler::findNode(ScriptNodeList::const_iterator &from, ScriptNodeList::const_iterator &to, uint32 type) const
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

	ScriptNodeList::iterator ScriptCompiler::findNode(ScriptNodeList::iterator &from, ScriptNodeList::iterator &to, const String &token) const
	{
		ScriptNodeList::iterator rslt = to;
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

	ScriptNodeList::const_iterator ScriptCompiler::findNode(ScriptNodeList::const_iterator &from, ScriptNodeList::const_iterator &to, const String &token) const
	{
		ScriptNodeList::const_iterator rslt = to;
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

	bool ScriptCompiler::verifyNextNodeType(ScriptNodeList::const_iterator &i, ScriptNodeList::const_iterator &end, uint32 type) const
	{
		ScriptNodeList::const_iterator j = i;
		++j;
		if(j == end)
			return false;
		return (*j)->type == type;
	}

	bool ScriptCompiler::getTruthValue(const ScriptNodePtr &node, bool &val) const
	{
		bool retval = true;
		switch(node->wordID)
		{
		case ID_ON:
			val = true;
			break;
		case ID_OFF:
			val = false;
			break;
		case ID_TRUE:
			val = true;
			break;
		case ID_FALSE:
			val = false;
			break;
		case ID_YES:
			val = true;
			break;
		case ID_NO:
			val = false;
			break;
		default:
			retval = false;
		}
		return retval;
	}

	void ScriptCompiler::addError(uint32 error, const String &file, int line, int col)
	{
		ScriptCompiler::ErrorPtr err(new ScriptCompiler::Error());
		err->error = error;
		err->file = file;
		err->line = line;
		err->column = col;

		// Allow a derived class to override the addition of this error
		if(errorRaised(err))
			mErrors.push_back(err);
	}

	void ScriptCompiler::pushScope()
	{
		LexicalScopePtr scope(new LexicalScope());

		// If there are scopes on the stack, copy the top one's contents into the new scope
		if(!mStack.empty())
			scope->vars = mStack.front()->vars;

		// Push the new scope onto the stack
		mStack.push_front(scope);
	}

	void ScriptCompiler::popScope()
	{
		if(!mStack.empty())
			mStack.pop_front();
	}

	std::pair<bool,String> ScriptCompiler::findVariable(const Ogre::String &name)
	{
		std::pair<bool,String> ret;
		ret.first = false;

		if(!mStack.empty())
		{
			// Grab the top stack and do a search on its variable map
			LexicalScopePtr &scope = mStack.front();
			ScopedVariableMap::iterator i = scope->vars.find(name);
			ret.first = i != scope->vars.end();
			if(ret.first)
				ret.second = i->second;
		}

		return ret;
	}

	void ScriptCompiler::setVariable(const String &name, const String &value)
	{
		if(!mStack.empty())
			mStack.front()->vars[name] = value;
	}

	void ScriptCompiler::compileMaterial(const ScriptNodePtr &node)
	{
		// Up until the '{' combine tokens to produce the name
		if(node->children.empty())
		{
			addError(CE_STRINGEXPECTED, node->file, node->line, -1);
			return;
		}

		String name;
		ScriptNodeList::iterator i = node->children.begin();
		while(i != node->children.end() && (*i)->type != SNT_LBRACE)
		{
			name += (*i)->token;
			i++;
		}

		// Allocate the material
		if(mListener)
			mMaterial = mListener->getMaterial(name, mGroup);
		else
			mMaterial = (Material*)MaterialManager::getSingleton().create(name, mGroup).get();
		if(!mMaterial)
		{
			addError(CE_OBJECTALLOCATIONERROR, node->file, node->line, node->column);
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
			if(!overrideNode(j, (*i)->children.end()))
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
					{
						bool val;
						if(getTruthValue((*j)->children.front(), val))
							mMaterial->setReceiveShadows(val);
						else
							addError(CE_INVALIDPROPERTYVALUE, (*j)->children.front()->file,
							(*j)->children.front()->line, (*j)->children.front()->column);
					}
					else
					{
						addError(CE_TRUTHVALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_TRANSPARENCY_CASTS_SHADOWS)
				{
					if(!(*j)->children.empty())
					{
						bool val;
						if(getTruthValue((*j)->children.front(), val))
							mMaterial->setTransparencyCastsShadows(val);
						else
							addError(CE_INVALIDPROPERTYVALUE, (*j)->children.front()->file,
							(*j)->children.front()->line, (*j)->children.front()->column);
					}
					else
					{
						addError(CE_TRUTHVALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
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

	void ScriptCompiler::compileTechnique(const ScriptNodePtr &node)
	{
		if(node->children.empty())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}

		// The first child is possibly the optional name of the technique
		String name;
		ScriptNodeList::iterator i = node->children.begin();
		if((*i)->type != SNT_LBRACE)
		{
			if(!(*i)->token.empty())
				name = (*i)->token;

			// Find the '{'
			i = findNode(i, node->children.end(), SNT_LBRACE);
		}

		// Create/get the technique
		Technique *technique = 0;
		if(name.empty())
			technique = mMaterial->createTechnique();
		else
		{
			technique = mMaterial->getTechnique(name);
			if(!technique)
			{
				technique = mMaterial->createTechnique();
				technique->setName(name);
			}
		}

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!overrideNode(j, (*i)->children.end()))
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

	void ScriptCompiler::compilePass(const ScriptNodePtr &node, Technique *technique)
	{
		if(node->children.empty())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}

		// The first child is possibly the optional name of the technique
		String name;
		ScriptNodeList::iterator i = node->children.begin();
		if((*i)->type != SNT_LBRACE)
		{
			if(!(*i)->token.empty())
				name = (*i)->token;

			// Find the '{'
			i = findNode(i, node->children.end(), SNT_LBRACE);
		}

		// Create the pass
		Pass *pass = 0;
		if(name.empty())
			pass = technique->createPass();
		else
		{
			pass = technique->getPass(name);
			if(!pass)
			{
				pass = technique->createPass();
				pass->setName(name);
			}
		}

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!overrideNode(j, (*i)->children.end()))
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
						ScriptNodeList::iterator k = (*j)->children.begin();
						if((*k)->wordID == ID_VERTEX_COLOUR)
							pass->setVertexColourTracking(Ogre::TVC_SPECULAR);
						else if(getColourValue(k, (*j)->children.end(), c))
						{
							pass->setSpecular(c);

							if(k != (*j)->children.end())
							{
								if((*k)->type == SNT_NUMBER)
									pass->setShininess((*k)->data);
								else
									addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
							}
						}
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
				else if((*j)->wordID == ID_SEPARATE_SCENE_BLEND)
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
						
						//if(isValid)
							//pass->setSeparateSceneBlending(sbt, sbta);
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
						//if(isValid)
							//pass->setSeparateSceneBlending(sbf1, sbf2, sbfa1, sbfa2);
					}
					else
						addError(CE_INVALIDPROPERTYVALUE, (*j)->file, (*j)->line, -1);
				}
				else if((*j)->wordID == ID_DEPTH_CHECK)
				{
					if(!(*j)->children.empty())
					{
						bool val;
						if(getTruthValue((*j)->children.front(), val))
							pass->setDepthCheckEnabled(val);
						else
							addError(CE_INVALIDPROPERTYVALUE, (*j)->children.front()->file,
							(*j)->children.front()->line, (*j)->children.front()->column);
					}
					else
					{
						addError(CE_TRUTHVALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_DEPTH_WRITE)
				{
					if(!(*j)->children.empty())
					{
						bool val;
						if(getTruthValue((*j)->children.front(), val))
							pass->setDepthWriteEnabled(val);
						else
							addError(CE_INVALIDPROPERTYVALUE, (*j)->children.front()->file,
							(*j)->children.front()->line, (*j)->children.front()->column);
					}
					else
					{
						addError(CE_TRUTHVALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
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
						//pass->setIterationDepthBias(node1->data);
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
					{
						/*
						bool val;
						if(getTruthValue((*j)->children.front(), val))
							pass->setLightScissoringEnabled(val);
						else
							addError(CE_INVALIDPROPERTYVALUE, (*j)->children.front()->file,
							(*j)->children.front()->line, (*j)->children.front()->column);
						*/
					}
					else
					{
						addError(CE_TRUTHVALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_LIGHT_CLIP_PLANES)
				{
					if(!(*j)->children.empty())
					{
						/*
						bool val;
						if(getTruthValue((*j)->children.front(), val))
							pass->setLightClipPlanesEnabled(val);
						else
							addError(CE_INVALIDPROPERTYVALUE, (*j)->children.front()->file,
							(*j)->children.front()->line, (*j)->children.front()->column);
						*/
					}
					else
					{
						addError(CE_TRUTHVALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_ILLUMINATION_STAGE)
				{
					if(!(*j)->children.empty())
					{
						/*
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
						*/
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
					{
						/*
						bool val;
						if(getTruthValue((*j)->children.front(), val))
							pass->setNormaliseNormals(val);
						else
							addError(CE_INVALIDPROPERTYVALUE, (*j)->children.front()->file,
							(*j)->children.front()->line, (*j)->children.front()->column);
						*/
					}
					else
					{
						addError(CE_TRUTHVALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_LIGHTING)
				{
					if(!(*j)->children.empty())
					{
						bool val;
						if(getTruthValue((*j)->children.front(), val))
							pass->setLightingEnabled(val);
						else
							addError(CE_INVALIDPROPERTYVALUE, (*j)->children.front()->file,
							(*j)->children.front()->line, (*j)->children.front()->column);
					}
					else
					{
						addError(CE_TRUTHVALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
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
					{
						/*
						bool val;
						if(getTruthValue((*j)->children.front(), val))
							pass->setPolygonModeOverrideable(val);
						else
							addError(CE_INVALIDPROPERTYVALUE, (*j)->children.front()->file,
							(*j)->children.front()->line, (*j)->children.front()->column);
						*/
					}
					else
					{
						addError(CE_TRUTHVALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_FOG_OVERRIDE)
				{
					if(!(*j)->children.empty())
					{
						bool fog;
						if(getTruthValue((*j)->children.front(), fog))
						{
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
						else
						{
							addError(CE_TRUTHVALUEEXPECTED, (*j)->children.front()->file,
								(*j)->children.front()->line, (*j)->children.front()->column);
						}
					}
				}
				else if((*j)->wordID == ID_COLOUR_WRITE)
				{
					if(!(*j)->children.empty())
					{
						bool val;
						if(getTruthValue((*j)->children.front(), val))
							pass->setColourWriteEnabled(val);
						else
							addError(CE_INVALIDPROPERTYVALUE, (*j)->children.front()->file,
							(*j)->children.front()->line, (*j)->children.front()->column);
					}
					else
					{
						addError(CE_TRUTHVALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
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
							pass->setIteratePerLight(false, false);
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
					{
						bool val;
						if(getTruthValue((*j)->children.front(), val))
							pass->setPointSpritesEnabled(val);
						else
							addError(CE_INVALIDPROPERTYVALUE, (*j)->children.front()->file,
							(*j)->children.front()->line, (*j)->children.front()->column);
					}
					else
					{
						addError(CE_TRUTHVALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
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
						bool enabled;
						if(getTruthValue((*j)->children.front(), enabled))
						{
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
						else
						{
							addError(CE_INVALIDPROPERTYVALUE, (*j)->children.front()->file,
								(*j)->children.front()->line, (*j)->children.front()->column);
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
				else if((*j)->wordID == ID_TEXTURE_UNIT)
				{
					compileTextureUnit(*j, pass);
				}
				else if((*j)->wordID == ID_VERTEX_PROGRAM_REF)
				{
					// Expect the name to be the first child
					if(!(*j)->children.empty())
					{
						ScriptNodeList::iterator k = (*j)->children.begin();
						String name = (*k)->token;
						if(mListener)
							mListener->getGpuProgramName(name);

						// Next is the '{', somewhere...
						k = findNode(k, (*j)->children.end(), SNT_LBRACE);
						if(k != (*j)->children.end())
						{
							GpuProgram *prog = 0;
							if(pass->hasVertexProgram() && (pass->getVertexProgramName() == name || name.empty()))
							{
								prog = pass->getVertexProgram().get();
							}
							else
							{
								prog = (GpuProgram*)GpuProgramManager::getSingleton().getByName(name).get();
								if(prog)
									pass->setVertexProgram(name);
							}

							if(prog)
							{
								if(prog->isSupported())
								{
									Ogre::GpuProgramParametersSharedPtr params = pass->getVertexProgramParameters();
									compileProgramParameters(*j, params);
								}
							}
							else
							{
								addError(CE_OBJECTALLOCATIONERROR, (*k)->file, (*k)->line, (*k)->column);
							}
						}
						else
						{
							addError(CE_OPENBRACEEXPECTED, (*j)->file, (*j)->line, -1);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_FRAGMENT_PROGRAM_REF)
				{
					// Expect the name to be the first child
					if(!(*j)->children.empty())
					{
						ScriptNodeList::iterator k = (*j)->children.begin();
						String name = (*k)->token;
						if(mListener)
							mListener->getGpuProgramName(name);

						// Next is the '{', somewhere...
						k = findNode(k, (*j)->children.end(), SNT_LBRACE);
						if(k != (*j)->children.end())
						{
							GpuProgram *prog = 0;
							if(pass->hasFragmentProgram() && (pass->getFragmentProgramName() == name || name.empty()))
							{
								prog = pass->getFragmentProgram().get();
							}
							else
							{
								prog = (GpuProgram*)GpuProgramManager::getSingleton().getByName(name).get();
								if(prog)
									pass->setFragmentProgram(name);
							}

							if(prog)
							{
								if(prog->isSupported())
								{
									Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
									compileProgramParameters(*j, params);
								}
							}
							else
							{
								addError(CE_OBJECTALLOCATIONERROR, (*k)->file, (*k)->line, (*k)->column);
							}
						}
						else
						{
							addError(CE_OPENBRACEEXPECTED, (*j)->file, (*j)->line, -1);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_SHADOW_CASTER_VERTEX_PROGRAM_REF)
				{
					// Expect the name to be the first child
					if(!(*j)->children.empty())
					{
						ScriptNodeList::iterator k = (*j)->children.begin();
						String name = (*k)->token;
						if(mListener)
							mListener->getGpuProgramName(name);

						// Next is the '{', somewhere...
						k = findNode(k, (*j)->children.end(), SNT_LBRACE);
						if(k != (*j)->children.end())
						{
							GpuProgram *prog = 0;
							if(pass->hasShadowCasterVertexProgram() && (pass->getShadowCasterVertexProgramName() == name || name.empty()))
							{
								prog = pass->getShadowCasterVertexProgram().get();
							}
							else
							{
								prog = (GpuProgram*)GpuProgramManager::getSingleton().getByName(name).get();
								if(prog)
									pass->setShadowCasterVertexProgram(name);
							}

							if(prog)
							{
								if(prog->isSupported())
								{
									Ogre::GpuProgramParametersSharedPtr params = pass->getShadowCasterVertexProgramParameters();
									compileProgramParameters(*j, params);
								}
							}
							else
							{
								addError(CE_OBJECTALLOCATIONERROR, (*k)->file, (*k)->line, (*k)->column);
							}
						}
						else
						{
							addError(CE_OPENBRACEEXPECTED, (*j)->file, (*j)->line, -1);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_SHADOW_RECEIVER_VERTEX_PROGRAM_REF)
				{
					// Expect the name to be the first child
					if(!(*j)->children.empty())
					{
						ScriptNodeList::iterator k = (*j)->children.begin();
						String name = (*k)->token;
						if(mListener)
							mListener->getGpuProgramName(name);

						// Next is the '{', somewhere...
						k = findNode(k, (*j)->children.end(), SNT_LBRACE);
						if(k != (*j)->children.end())
						{
							GpuProgram *prog = 0;
							if(pass->hasShadowReceiverVertexProgram() && (pass->getShadowReceiverVertexProgramName() == name || name.empty()))
							{
								prog = pass->getShadowReceiverVertexProgram().get();
							}
							else
							{
								prog = (GpuProgram*)GpuProgramManager::getSingleton().getByName(name).get();
								if(prog)
									pass->setShadowReceiverVertexProgram(name);
							}

							if(prog)
							{
								if(prog->isSupported())
								{
									Ogre::GpuProgramParametersSharedPtr params = pass->getShadowReceiverVertexProgramParameters();
									compileProgramParameters(*j, params);
								}
							}
							else
							{
								addError(CE_OBJECTALLOCATIONERROR, (*k)->file, (*k)->line, (*k)->column);
							}
						}
						else
						{
							addError(CE_OPENBRACEEXPECTED, (*j)->file, (*j)->line, -1);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_SHADOW_RECEIVER_FRAGMENT_PROGRAM_REF)
				{
					// Expect the name to be the first child
					if(!(*j)->children.empty())
					{
						ScriptNodeList::iterator k = (*j)->children.begin();
						String name = (*k)->token;

						// Next is the '{', somewhere...
						k = findNode(k, (*j)->children.end(), SNT_LBRACE);
						if(k != (*j)->children.end())
						{
							GpuProgram *prog = 0;
							if(pass->hasShadowReceiverFragmentProgram() && (pass->getShadowReceiverFragmentProgramName() == name || name.empty()))
							{
								prog = pass->getShadowReceiverFragmentProgram().get();
							}
							else
							{
								prog = (GpuProgram*)GpuProgramManager::getSingleton().getByName(name).get();
								if(prog)
									pass->setShadowReceiverFragmentProgram(name);
							}

							if(prog)
							{
								if(prog->isSupported())
								{
									Ogre::GpuProgramParametersSharedPtr params = pass->getShadowReceiverFragmentProgramParameters();
									compileProgramParameters(*j, params);
								}
							}
							else
							{
								addError(CE_OBJECTALLOCATIONERROR, (*k)->file, (*k)->line, (*k)->column);
							}
						}
						else
						{
							addError(CE_OPENBRACEEXPECTED, (*j)->file, (*j)->line, -1);
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

	void ScriptCompiler::compileTextureUnit(const ScriptNodePtr &node, Pass *pass)
	{
		if(node->children.empty())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}

		// The first child is possibly the optional name
		String name;
		ScriptNodeList::iterator i = node->children.begin();
		if((*i)->type != SNT_LBRACE)
		{
			if(!(*i)->token.empty())
				name = (*i)->token;

			// Find the '{'
			i = findNode(i, node->children.end(), SNT_LBRACE);
		}

		// Create the TextureUnitState
		TextureUnitState *unit = 0;
		if(name.empty())
			unit = pass->createTextureUnitState();
		else
		{
			unit = pass->getTextureUnitState(name);
			if(!unit)
			{
				unit = pass->createTextureUnitState();
				unit->setName(name);
			}
		}

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!overrideNode(j, (*i)->children.end()))
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
					ScriptNodePtr node1 = (*j)->children.front();
					if(!node1.isNull())
					{
						Ogre::TextureType texType = Ogre::TEX_TYPE_2D;
						bool isAlpha = false;
						PixelFormat format = PF_UNKNOWN;
						int mipmaps = MIP_DEFAULT;
						
						ScriptNodeList::iterator k = (*j)->children.begin();
						++k;
						while(k != (*j)->children.end())
						{
							switch((*k)->wordID)
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
								if((*k)->type == SNT_NUMBER)
									mipmaps = (*k)->data;
								else
									format = PixelUtil::getFormatFromName((*k)->token, true);
							}
							++k;
						}

						// Allow the listener to override the name
						String name = node1->token;
						if(mListener)
							mListener->getTextures(&name, 1);
						unit->setTextureName(name, texType);
						unit->setNumMipmaps(mipmaps);
						unit->setDesiredFormat(format);
						unit->setIsAlpha(isAlpha);
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
								node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2);
							if(node3->type == SNT_NUMBER)
							{
								String tex = node1->token;
								// Allow the listener to override
								if(mListener)
									mListener->getTextures(&tex, 1);
								unit->setAnimatedTextureName(tex, node2->data, node3->data);
							}
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
							// Allow the listener to override
							if(mListener)
								mListener->getTextures(frames, count);
							unit->setAnimatedTextureName(frames, count, duration);
							delete[] frames;
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_CUBIC_TEXTURE)
				{
					if((*j)->children.size() == 2)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1);
						String tex = node1->token;
						// Allow the listener to override
						if(mListener)
							mListener->getTextures(&tex, 1);
						switch(node2->wordID)
						{
						case ID_COMBINED_UVW:
							unit->setCubicTextureName(tex, true);
							break;
						case ID_SEPARATE_UV:
							unit->setCubicTextureName(tex, false);
							break;
						default:
							addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
						}
					}
					else if((*j)->children.size() == 6 || (*j)->children.size() == 7)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1),
							node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2),
							node4 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 3),
							node5 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 4),
							node6 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 5);
						String names[] = {node1->token, node2->token, node3->token, node4->token, node5->token, node6->token};
						// Allow the listener to override
						if(mListener)
							mListener->getTextures(names, 6);
						unit->setCubicTextureName(names, false);
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_TEX_COORD_SET)
				{
					if(!(*j)->children.empty())
					{
						ScriptNodePtr node = (*j)->children.front();
						if(node->type == SNT_NUMBER)
							unit->setTextureCoordSet(node->data);
						else
							addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
					}
					else
					{
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_TEX_ADDRESS_MODE)
				{
					if(!(*j)->children.empty())
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1),
							node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2);
						
						if(node2.isNull())
						{
							TextureUnitState::TextureAddressingMode mode;
							if(getTextureAddressingMode(node1, mode))
								unit->setTextureAddressingMode(mode);
							else
								addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
						}
						else
						{
							TextureUnitState::UVWAddressingMode mode;
							mode.w = TextureUnitState::TAM_WRAP;
							bool isValid = true;
							if(!getTextureAddressingMode(node1, mode.u))
							{
								addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
								isValid = false;
							}
							if(!getTextureAddressingMode(node2, mode.v))
							{
								addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
								isValid = false;
							}
							if(!node3.isNull())
							{
								if(!getTextureAddressingMode(node1, mode.w))
								{
									addError(CE_INVALIDPROPERTYVALUE, node3->file, node3->line, node3->column);
									isValid = false;
								}
							}
							if(isValid)
								unit->setTextureAddressingMode(mode);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_TEX_BORDER_COLOUR)
				{
					if(!(*j)->children.empty())
					{
						ColourValue c;
						ScriptNodeList::iterator k = (*j)->children.begin();
						if(getColourValue(k, (*j)->children.end(), c))
							unit->setTextureBorderColour(c);
						else
							addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
					}
					else
					{
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_FILTERING)
				{
					if(!(*j)->children.empty())
					{
						if((*j)->children.size() == 1)
						{
							ScriptNodePtr node = (*j)->children.front();
							switch(node->wordID)
							{
							case ID_NONE:
								unit->setTextureFiltering(TFO_NONE);
								break;
							case ID_BILINEAR:
								unit->setTextureFiltering(TFO_BILINEAR);
								break;
							case ID_TRILINEAR:
								unit->setTextureFiltering(TFO_TRILINEAR);
								break;
							case ID_ANISOTROPIC:
								unit->setTextureFiltering(TFO_ANISOTROPIC);
								break;
							default:
								addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
							}
						}
						else
						{
							ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
								node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1),
								node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2);
							if(node2.isNull() || node3.isNull())
							{
								addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
							}
							else
							{
								FilterOptions fmin, fmax, fmip;
								bool isValid = true;
								switch(node1->wordID)
								{
								case ID_NONE:
									fmin = FO_NONE;
									break;
								case ID_POINT:
									fmin = FO_POINT;
									break;
								case ID_LINEAR:
									fmin = FO_LINEAR;
									break;
								case ID_ANISOTROPIC:
									fmin = FO_ANISOTROPIC;
									break;
								default:
									addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
									isValid = false;
								}
								switch(node2->wordID)
								{
								case ID_NONE:
									fmax = FO_NONE;
									break;
								case ID_POINT:
									fmax = FO_POINT;
									break;
								case ID_LINEAR:
									fmax = FO_LINEAR;
									break;
								case ID_ANISOTROPIC:
									fmax = FO_ANISOTROPIC;
									break;
								default:
									addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
									isValid = false;
								}
								switch(node3->wordID)
								{
								case ID_NONE:
									fmip = FO_NONE;
									break;
								case ID_POINT:
									fmip = FO_POINT;
									break;
								case ID_LINEAR:
									fmip = FO_LINEAR;
									break;
								case ID_ANISOTROPIC:
									fmip = FO_ANISOTROPIC;
									break;
								default:
									addError(CE_INVALIDPROPERTYVALUE, node3->file, node3->line, node3->column);
									isValid = false;
								}

								if(isValid)
									unit->setTextureFiltering(fmin, fmax, fmip);
							}
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_MAX_ANISOTROPY)
				{
					if(!(*j)->children.empty())
					{
						ScriptNodePtr node = (*j)->children.front();
						if(node->type == SNT_NUMBER)
							unit->setTextureAnisotropy(node->data);
						else
							addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
					}
					else
					{
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_MIPMAP_BIAS)
				{
					if(!(*j)->children.empty())
					{
						ScriptNodePtr node = (*j)->children.front();
						if(node->type == SNT_NUMBER)
							unit->setTextureMipmapBias(node->data);
						else
							addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
					}
					else
					{
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_COLOUR_OP)
				{
					if(!(*j)->children.empty())
					{
						ScriptNodePtr node = (*j)->children.front();
						switch(node->wordID)
						{
						case ID_REPLACE:
							unit->setColourOperation(Ogre::LBO_REPLACE);
							break;
						case ID_ADD:
							unit->setColourOperation(LBO_ADD);
							break;
						case ID_MODULATE:
							unit->setColourOperation(LBO_MODULATE);
							break;
						case ID_ALPHA_BLEND:
							unit->setColourOperation(LBO_ALPHA_BLEND);
							break;
						default:
							addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_COLOUR_OP_EX)
				{
					if((*j)->children.size() >= 3)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1),
							node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2);
						LayerBlendOperationEx op;
						LayerBlendSource source1, source2;
						bool isValid = true;

						if(!getColourOperation(node1, op))
						{
							addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
							isValid = false;
						}
						if(!getColourOperationSource(node2, source1))
						{
							addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
							isValid = false;
						}
						if(!getColourOperationSource(node3, source2))
						{
							addError(CE_INVALIDPROPERTYVALUE, node3->file, node3->line, node3->column);
							isValid = false;
						}

						if(isValid)
						{
							Ogre::ColourValue c1 = ColourValue::White, c2 = ColourValue::White;
							Real r = 0.0f;

							if(op == LBX_BLEND_MANUAL)
							{
								ScriptNodePtr node4 = 
									getNodeAt((*j)->children.begin(), (*j)->children.end(), 3);
								if(!node4.isNull())
								{
									if(node4->type == SNT_NUMBER)
										r = node4->data;
									else
										addError(CE_INVALIDPROPERTYVALUE, node4->file, node4->line, node4->column);
								}
								else
								{
									addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
								}
							}

							ScriptNodeList::iterator k = (*j)->children.begin();
							++k;
							++k;
							++k;
							if(op == LBX_BLEND_MANUAL)
								++k;
							if(source1 == LBS_MANUAL)
							{
								if(k != (*j)->children.end())
								{
									if(!getColourValue(k, (*j)->children.end(), c1))
										addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
								}
								else
								{
									addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
								}
							}
							if(source2 == LBS_MANUAL)
							{
								if(k != (*j)->children.end())
								{
									if(!getColourValue(k, (*j)->children.end(), c2))
										addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
								}
								else
								{
									addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
								}
							}

							unit->setColourOperationEx(op, source1, source2, c1, c2, r);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_COLOUR_OP_MULTIPASS_FALLBACK)
				{
					if((*j)->children.size() >= 2)
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
							unit->setColourOpMultipassFallback(sbf1, sbf2);
					}
					else
						addError(CE_INVALIDPROPERTYVALUE, (*j)->file, (*j)->line, -1);
				}
				else if((*j)->wordID == ID_ALPHA_OP_EX)
				{
					if((*j)->children.size() >= 3)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1),
							node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2);
						LayerBlendOperationEx op;
						LayerBlendSource source1, source2;
						bool isValid = true;

						if(!getColourOperation(node1, op))
						{
							addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
							isValid = false;
						}
						if(!getColourOperationSource(node2, source1))
						{
							addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
							isValid = false;
						}
						if(!getColourOperationSource(node3, source2))
						{
							addError(CE_INVALIDPROPERTYVALUE, node3->file, node3->line, node3->column);
							isValid = false;
						}

						if(isValid)
						{
							Real src1 = 0.0f, src2 = 0.0f, factor = 0.0f;
							int index = 3;

							if(op == LBX_BLEND_MANUAL)
							{
								ScriptNodePtr node4 = 
									getNodeAt((*j)->children.begin(), (*j)->children.end(), index);
								if(!node4.isNull())
								{
									if(node4->type == SNT_NUMBER)
										factor = node4->data;
									else
										addError(CE_INVALIDPROPERTYVALUE, node4->file, node4->line, node4->column);
									++index;
								}
								else
								{
									addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
								}
							}

							if(source1 == LBS_MANUAL)
							{
								ScriptNodePtr node = getNodeAt((*j)->children.begin(), (*j)->children.end(), index);
								if(!node.isNull())
								{
									if(node->type == SNT_NUMBER)
										src1 = node->data;
									else
										addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
									++index;
								}
								else
								{
									addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
								}
							}
							if(source2 == LBS_MANUAL)
							{
								ScriptNodePtr node = getNodeAt((*j)->children.begin(), (*j)->children.end(), index);
								if(!node.isNull())
								{
									if(node->type == SNT_NUMBER)
										src2 = node->data;
									else
										addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
									++index;
								}
								else
								{
									addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
								}
							}

							unit->setAlphaOperation(op, source1, source2, src1, src2, factor);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_ENV_MAP)
				{
					if(!(*j)->children.empty())
					{
						ScriptNodePtr node = (*j)->children.front();
						switch(node->wordID)
						{
						case ID_OFF:
							unit->setEnvironmentMap(false);
							break;
						case ID_SPHERICAL:
							unit->setEnvironmentMap(true, TextureUnitState::ENV_CURVED);
							break;
						case ID_PLANAR:
							unit->setEnvironmentMap(true, TextureUnitState::ENV_PLANAR);
							break;
						case ID_CUBIC_REFLECTION:
							unit->setEnvironmentMap(true, TextureUnitState::ENV_REFLECTION);
							break;
						case ID_CUBIC_NORMAL:
							unit->setEnvironmentMap(true, TextureUnitState::ENV_NORMAL);
							break;
						default:
							addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_SCROLL)
				{
					if((*j)->children.size() >= 2)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1);
						bool isValid = true;
						if(node1->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
							isValid = false;
						}
						if(node2->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
							isValid = false;
						}
						if(isValid)
							unit->setTextureScroll(node1->data, node2->data);
					}
					else
					{
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_SCROLL_ANIM)
				{
					if((*j)->children.size() >= 2)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1);
						bool isValid = true;
						if(node1->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
							isValid = false;
						}
						if(node2->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
							isValid = false;
						}
						if(isValid)
							unit->setScrollAnimation(node1->data, node2->data);
					}
					else
					{
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_ROTATE)
				{
					if((*j)->children.size() >= 1)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0);
						if(node1->type == SNT_NUMBER)
						{
							unit->setTextureRotate(Ogre::Degree(node1->data));
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
				else if((*j)->wordID == ID_ROTATE_ANIM)
				{
					if((*j)->children.size() >= 1)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0);
						if(node1->type == SNT_NUMBER)
						{
							unit->setRotateAnimation(node1->data);
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
				else if((*j)->wordID == ID_SCALE)
				{
					if((*j)->children.size() >= 2)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1);
						bool isValid = true;
						if(node1->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
							isValid = false;
						}
						if(node2->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
							isValid = false;
						}
						if(isValid)
							unit->setTextureScale(node1->data, node2->data);
					}
					else
					{
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_WAVE_XFORM)
				{
					if((*j)->children.size() >= 6)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1),
							node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2),
							node4 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 3),
							node5 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 4),
							node6 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 5);
						TextureUnitState::TextureTransformType type;
						WaveformType wave;
						bool isValid = true;

						switch(node1->wordID)
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
							addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
							isValid = false;
						}
						
						switch(node2->wordID)
						{
						case ID_SINE:
							wave = Ogre::WFT_SINE;
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
							addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
							isValid = false;
						}

						if(node3->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node3->file, node3->line, node3->column);
							isValid = false;
						}

						if(node4->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node4->file, node4->line, node4->column);
							isValid = false;
						}

						if(node5->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node5->file, node5->line, node5->column);
							isValid = false;
						}

						if(node6->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node6->file, node6->line, node6->column);
							isValid = false;
						}

						if(isValid)
							unit->setTransformAnimation(type, wave, node3->data, node4->data, 
								node5->data, node6->data);
					}
					else
					{
						addError(CE_VALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_TRANSFORM)
				{
					if((*j)->children.size() >= 16)
					{
						Matrix4 m;
						if(getMatrix4((*j)->children.begin(), (*j)->children.end(), m))
							unit->setTextureTransform(m);
					}
					else
					{
						addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_BINDING_TYPE)
				{
					if(!(*j)->children.empty())
					{
						ScriptNodePtr node = (*j)->children.front();
						switch(node->wordID)
						{
						case ID_VERTEX:
							unit->setBindingType(TextureUnitState::BT_VERTEX);
							break;
						case ID_FRAGMENT:
							unit->setBindingType(TextureUnitState::BT_FRAGMENT);
							break;
						default:
							addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
						}
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_CONTENT_TYPE)
				{
					if(!(*j)->children.empty())
					{
						ScriptNodePtr node = (*j)->children.front();
						switch(node->wordID)
						{
						case ID_NAMED:
							unit->setContentType(TextureUnitState::CONTENT_NAMED);
							break;
						case ID_SHADOW:
							unit->setContentType(TextureUnitState::CONTENT_SHADOW);
							break;
						default:
							addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
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

	void ScriptCompiler::compileGpuProgram(const ScriptNodePtr &node)
	{
		if(node->children.size() < 3)
		{
			addError(CE_STRINGEXPECTED, node->file, node->line, -1);
			return;
		}

		// The first token is the name, the second is the language
		ScriptNodePtr node1 = getNodeAt(node->children.begin(), node->children.end(), 0),
			node2 = getNodeAt(node->children.begin(), node->children.end(), 1);
		
		// Allocate the program based on its language
		if(node2->token == "asm")
		{
			compileAsmGpuProgram(node1->token, node);
		}
		else if(node2->token == "unified")
		{
			compileUnifiedHighLevelGpuProgram(node1->token, node);
		}
		else
		{
			compileHighLevelGpuProgram(node1->token, node2->token, node);
		}
	}

	void ScriptCompiler::compileAsmGpuProgram(const Ogre::String &name, const Ogre::ScriptNodePtr &node)
	{
		GpuProgramType type = node->wordID == ID_VERTEX_PROGRAM ? GPT_VERTEX_PROGRAM : GPT_FRAGMENT_PROGRAM;
		
		ScriptNodeList::iterator i = findNode(node->children.begin(), node->children.end(), SNT_LBRACE);
		if(i == node->children.end())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}

		std::list<std::pair<String,String> > customParameters;
		String syntax, source;
		ScriptNodeList::iterator j = (*i)->children.begin(), paramIter = (*i)->children.end();
		while(j != (*i)->children.end())
		{
			if(!overrideNode(j, (*i)->children.end()))
			{
				if((*j)->wordID == ID_SOURCE)
				{
					if(!(*j)->children.empty())
						source = (*j)->children.front()->token;
					else
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
				}
				else if((*j)->wordID == ID_SYNTAX)
				{
					if(!(*j)->children.empty())
						syntax = (*j)->children.front()->token;
					else
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
				}
				else if((*j)->wordID == ID_DEFAULT_PARAMS)
				{
					paramIter = j;
				}
				else
				{
					// Expect name followed by any number of values. Put the values into 1 string
					if(!(*j)->children.empty())
					{
						ScriptNodeList::iterator k = (*j)->children.begin();
						String name = (*k)->token, value;

						++k;
						if(k != (*j)->children.end())
							value = (*k)->token;
						while(++k != (*j)->children.end())
							value = value + " " + (*k)->token;
						
						customParameters.push_back(std::make_pair(name, value));
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				++j;
			}
		}

		try{
			// Allocate the program
			GpuProgram *prog = 0;
			if(mListener)
				prog = mListener->getGpuProgram(name, mGroup, source, type, syntax);
			else
				prog = GpuProgramManager::getSingleton().createProgram(name, mGroup, source, type, syntax).get();
			if(prog == 0)
			{
				addError(CE_OBJECTALLOCATIONERROR, node->file, node->line, -1);
				return;
			}

			prog->_notifyOrigin(node->file);

			// Set custom parameters
			for(std::list<std::pair<String,String> >::iterator k = customParameters.begin(); k != customParameters.end(); ++k)
				prog->setParameter(k->first, k->second);

			// Set up default parameters
			if(prog->isSupported() && paramIter != (*i)->children.end())
			{
				GpuProgramParametersSharedPtr params = prog->getDefaultParameters();
				compileProgramParameters(*paramIter, params);
			}
			prog->touch();
		}catch(...){
		}
	}

	void ScriptCompiler::compileHighLevelGpuProgram(const String &name, const String &language, const ScriptNodePtr &node)
	{
		GpuProgramType type = node->wordID == ID_VERTEX_PROGRAM ? GPT_VERTEX_PROGRAM : GPT_FRAGMENT_PROGRAM;
		
		ScriptNodeList::iterator i = findNode(node->children.begin(), node->children.end(), SNT_LBRACE);
		if(i == node->children.end())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}

		String source;
		std::list<std::pair<String,String> > customParameters;
		ScriptNodeList::iterator j = (*i)->children.begin(), paramIter = (*i)->children.end();
		while(j != (*i)->children.end())
		{
			if(!overrideNode(j, (*i)->children.end()))
			{
				if((*j)->wordID == ID_DEFAULT_PARAMS)
				{
					paramIter = j;
				}
				else if((*j)->wordID == ID_SOURCE)
				{
					if(!(*j)->children.empty())
						source = (*j)->children.front()->token;
					else
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
				}
				else
				{
					// Expect name followed by any number of values. Put the values into 1 string
					if(!(*j)->children.empty())
					{
						ScriptNodeList::iterator k = (*j)->children.begin();
						String name = (*j)->token, value = (*k)->token;

						while(++k != (*j)->children.end())
							value = value + " " + (*k)->token;
						
						customParameters.push_back(std::make_pair(name, value));
					}
					else
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				++j;
			}
		}

		try{
			// Allocate the program
			HighLevelGpuProgram *prog = 0;
			if(mListener)
				prog = mListener->getHighLevelGpuProgram(name, mGroup, language, type, source);
			else
			{
				prog = HighLevelGpuProgramManager::getSingleton().createProgram(name, mGroup, language, type).get();
				prog->setSourceFile(source);
			}
			if(prog == 0)
			{
				addError(CE_OBJECTALLOCATIONERROR, node->file, node->line, -1);
				return;
			}

			prog->_notifyOrigin(node->file);

			// Set custom parameters
			for(std::list<std::pair<String,String> >::iterator k = customParameters.begin(); k != customParameters.end(); ++k)
				prog->setParameter(k->first, k->second);

			// Set up default parameters
			if(prog->isSupported() && paramIter != (*i)->children.end())
			{
				GpuProgramParametersSharedPtr params = prog->getDefaultParameters();
				compileProgramParameters(*paramIter, params);
			}
			prog->touch();
		}catch(...){
		}
	}

	void ScriptCompiler::compileUnifiedHighLevelGpuProgram(const String &name, const ScriptNodePtr &node)
	{
		GpuProgramType type = node->wordID == ID_VERTEX_PROGRAM ? GPT_VERTEX_PROGRAM : GPT_FRAGMENT_PROGRAM;
		
		ScriptNodeList::iterator i = findNode(node->children.begin(), node->children.end(), SNT_LBRACE);
		if(i == node->children.end())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}

		std::list<std::pair<String,String> > customParameters;
		ScriptNodeList::iterator j = (*i)->children.begin(), paramIter = (*i)->children.end();
		while(j != (*i)->children.end())
		{
			if(!overrideNode(j, (*i)->children.end()))
			{
				// Expect name followed by any number of values. Put the values into 1 string
				if((*j)->token == "delegate")
				{
					String name = (*j)->children.front()->token;
					if(mListener)
							mListener->getGpuProgramName(name);
					customParameters.push_back(std::make_pair("delegate", name));
				}
				else if(!(*j)->children.empty())
				{
					ScriptNodeList::iterator k = (*j)->children.begin();
					String name = (*j)->token, value = (*k)->token;

					while(++k != (*j)->children.end())
						value = value + " " + (*k)->token;
					
					customParameters.push_back(std::make_pair(name, value));
				}
				else
				{
					addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
				}
				++j;
			}
		}

		try{
			// Allocate the program
			HighLevelGpuProgram *prog = 0;
			if(mListener)
				prog = mListener->getHighLevelGpuProgram(name, mGroup, "unified", type, "");
			else
				prog = HighLevelGpuProgramManager::getSingleton().createProgram(name, mGroup, "unified", type).get();
			if(prog == 0)
			{
				addError(CE_OBJECTALLOCATIONERROR, node->file, node->line, -1);
				return;
			}

			prog->_notifyOrigin(node->file);

			// Set custom parameters
			for(std::list<std::pair<String,String> >::iterator k = customParameters.begin(); k != customParameters.end(); ++k)
				prog->setParameter(k->first, k->second);
			prog->touch();
		}catch(...){
		}
	}

	void ScriptCompiler::compileProgramParameters(const ScriptNodePtr &node, const GpuProgramParametersSharedPtr &params)
	{
		// Find the '{'
		ScriptNodeList::iterator i = findNode(node->children.begin(), node->children.end(), SNT_LBRACE);
		if(i == node->children.end())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}

		int animParametricsCount = 0;
		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!overrideNode(j, (*i)->children.end()))
			{
				if((*j)->wordID == ID_PARAM_INDEXED || (*j)->wordID == ID_PARAM_NAMED)
				{
					if((*j)->children.size() >= 3)
					{
						bool named = (*j)->wordID == ID_PARAM_NAMED;
						ScriptNodeList::iterator k = (*j)->children.begin();
						String name;
						size_t index = 0;

						// Assign the name/index
						if(named)
							name = (*k)->token;
						else
							index = (*k)->data;

						++k;
						
						// Determine the type
						if((*k)->token == "matrix4x4")
						{	
							++k;

							Matrix4 m;
							if(getMatrix4(k, (*j)->children.end(), m))
							{
								if(named)
									params->setNamedConstant(name, m);
								else
									params->setConstant(index, m);
							}
							else
							{
								if(k != (*j)->children.end())
									addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
								else
									addError(CE_INVALIDPROPERTYVALUE, (*j)->file, (*j)->line, -1);
							}
						}
						else
						{
							// Find the number of parameters
							bool isValid = true;
							GpuProgramParameters::ElementType type;
							int count = 0;
							if((*k)->token.find("float") != String::npos)
							{
								type = GpuProgramParameters::ET_REAL;
								if((*k)->token.size() >= 6)
									count = StringConverter::parseInt((*k)->token.substr(5));
								else
								{
									count = 1;
								}
							}
							else if((*k)->token.find("int") != String::npos)
							{
								type = GpuProgramParameters::ET_INT;
								if((*k)->token.size() >= 4)
									count = StringConverter::parseInt((*k)->token.substr(3));
								else
								{
									count = 1;
								}
							}
							else
							{
								addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
								isValid = false;
							}

							if(isValid)
							{
								// First, clear out any offending auto constants
								if(named)
									params->clearNamedAutoConstant(name);
								else
									params->clearAutoConstant(index);

								++k;
								int roundedCount = count%4 != 0 ? count + 4 - (count%4) : count;
								if(type == GpuProgramParameters::ET_INT)
								{
									int *vals = new int[roundedCount];
									if(getInts(k, (*j)->children.end(), vals, roundedCount))
									{
										if(named)
											params->setNamedConstant(name, vals, count, 1);
										else
											params->setConstant(index, vals, roundedCount/4);
									}
									else
									{
										if(k != (*j)->children.end())
											addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
										else
											addError(CE_INVALIDPROPERTYVALUE, (*j)->file, (*j)->line, -1);
									}
									delete[] vals;
								}
								else
								{
									float *vals = new float[roundedCount];
									if(getFloats(k, (*j)->children.end(), vals, roundedCount))
									{
										if(named)
											params->setNamedConstant(name, vals, count, 1);
										else
											params->setConstant(index, vals, roundedCount/4);
									}
									else
									{
										if(k != (*j)->children.end())
											addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
										else
											addError(CE_INVALIDPROPERTYVALUE, (*j)->file, (*j)->line, -1);
									}
									delete[] vals;
								}
							}
						}
					}
					else
					{
						if((*j)->wordID == ID_PARAM_INDEXED)
							addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
						else
							addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_PARAM_INDEXED_AUTO || (*j)->wordID == ID_PARAM_NAMED_AUTO)
				{
					bool named = (*j)->wordID == ID_PARAM_NAMED_AUTO;
					String name;
					size_t index = 0;

					ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
						node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1),
						node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2);

					if(named)
						name = node1->token;
					else
						index = node1->data;

					// Look up the auto constant
					StringUtil::toLowerCase(node2->token);
					const GpuProgramParameters::AutoConstantDefinition *def =
						GpuProgramParameters::getAutoConstantDefinition(node2->token);
					if(def)
					{
						switch(def->dataType)
						{
						case GpuProgramParameters::ACDT_NONE:
							// Set the auto constant
							if(named)
								params->setNamedAutoConstant(name, def->acType);
							else
								params->setAutoConstant(index, def->acType);
							break;
						case GpuProgramParameters::ACDT_INT:
							if(def->acType == GpuProgramParameters::ACT_ANIMATION_PARAMETRIC)
							{
								if(named)
									params->setNamedAutoConstant(name, def->acType, animParametricsCount++);
								else
									params->setAutoConstant(index, def->acType, animParametricsCount++);
							}
							else
							{
								// Only certain texture projection auto params will assume 0
								// Otherwise we will expect that 3rd parameter
								if(node3.isNull())
								{
									if(def->acType == GpuProgramParameters::ACT_TEXTURE_VIEWPROJ_MATRIX //||
										//def->acType == GpuProgramParameters::ACT_TEXTURE_WORLDVIEWPROJ_MATRIX ||
										//def->acType == GpuProgramParameters::ACT_SPOTLIGHT_VIEWPROJ_MATRIX ||
										//def->acType == GpuProgramParameters::ACT_SPOTLIGHT_WORLDVIEWPROJ_MATRIX
										)
									{
										if(named)
											params->setNamedAutoConstant(name, def->acType, 0);
										else
											params->setAutoConstant(index, def->acType, 0);
									}
									else
									{
										addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
									}
								}
								else
								{
									if(named)
										params->setNamedAutoConstant(name, def->acType, (size_t)node3->data);
									else
										params->setAutoConstant(index, def->acType, (size_t)node3->data);
								}
							}
							break;
						case GpuProgramParameters::ACDT_REAL:
							if(def->acType == GpuProgramParameters::ACT_TIME ||
								def->acType == GpuProgramParameters::ACT_FRAME_TIME)
							{
								Real f = 1.0f;
								if(!node3.isNull() && node3->type == SNT_NUMBER)
									f = node3->data;
								
								if(named)
									params->setNamedAutoConstantReal(name, def->acType, f);
								else
									params->setAutoConstantReal(index, def->acType, f);
							}
							else
							{
								if(!node3.isNull())
								{
									if(node3->type == SNT_NUMBER)
									{
										if(named)
											params->setNamedAutoConstantReal(name, def->acType, node3->data);
										else
											params->setAutoConstantReal(index, def->acType, node3->data);
									}
									else
									{
										addError(CE_INVALIDPROPERTYVALUE, node3->file, node3->line, node3->column);
									}
								}
								else
								{
									addError(CE_NUMBEREXPECTED, (*j)->file, (*j)->line, -1);
								}
							}
							break;
						}
					}
					else
					{
						addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
					}
				}
				++j;
			}
		}
	}

	bool ScriptCompiler::getColourValue(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, ColourValue &c)
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

	bool ScriptCompiler::getBlendFactor(const ScriptNodePtr &node, SceneBlendFactor &sbf)
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

	bool ScriptCompiler::getCompareFunction(const ScriptNodePtr &node, CompareFunction &func)
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

	bool ScriptCompiler::getTextureAddressingMode(const Ogre::ScriptNodePtr &node, TextureUnitState::TextureAddressingMode &mode)
	{
		bool success = true;
		switch(node->wordID)
		{
		case ID_WRAP:
			mode = TextureUnitState::TAM_WRAP;
			break;
		case ID_CLAMP:
			mode = TextureUnitState::TAM_CLAMP;
			break;
		case ID_BORDER:
			mode = TextureUnitState::TAM_BORDER;
			break;
		case ID_MIRROR:
			mode = TextureUnitState::TAM_MIRROR;
			break;
		default:
			success = false;
		}
		return success;
	}

	bool ScriptCompiler::getColourOperation(const Ogre::ScriptNodePtr &node, Ogre::LayerBlendOperationEx &op)
	{
		bool success = true;
		switch(node->wordID)
		{
		case ID_SOURCE1:
			op = Ogre::LBX_SOURCE1;
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
			success = false;
		}
		return success;
	}

	bool ScriptCompiler::getColourOperationSource(const ScriptNodePtr &node, LayerBlendSource &source)
	{
		bool success = true;
		switch(node->wordID)
		{
		case ID_SRC_CURRENT:
			source = Ogre::LBS_CURRENT;
			break;
		case ID_SRC_TEXTURE:
			source = LBS_TEXTURE;
			break;
		case ID_SRC_DIFFUSE:
			source = LBS_DIFFUSE;
			break;
		case ID_SRC_SPECULAR:
			source = LBS_SPECULAR;
			break;
		case ID_SRC_MANUAL:
			source = LBS_MANUAL;
			break;
		default:
			success = false;
		}
		return success;
	}

	bool ScriptCompiler::getMatrix4(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Matrix4 &m)
	{
		bool success = true;
		size_t index = 0;
		while(i != end && index < 16)
		{
			if((*i)->type == SNT_NUMBER)
				m[index%4][index/4] = (*i)->data;
			else
				break;
			index++;
		}

		success = (index == 16);

		return success;
	}

	bool ScriptCompiler::getInts(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, int *vals, int count)
	{
		bool success = true;
		int n = 0;
		while(n < count)
		{
			if(i != end)
			{
				if((*i)->type == SNT_NUMBER)
					vals[n] = (*i)->data;
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

	bool ScriptCompiler::getFloats(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, float *vals, int count)
	{
		bool success = true;
		int n = 0;
		while(n < count)
		{
			if(i != end)
			{
				if((*i)->type == SNT_NUMBER)
					vals[n] = (*i)->data;
				else
					break;
				++i;
			}
			else
				vals[n] = 0.0f;
			++n;
		}

		if(n < count)
			success = false;

		return success;
	}

	void ScriptCompiler::compileParticleSystem(const ScriptNodePtr &node)
	{
		// Get the first child, which should be the name
		if(node->children.empty())
		{
			addError(CE_STRINGEXPECTED, node->file, node->line, -1);
			return;
		}

		// Get the name
		String name;
		ScriptNodeList::iterator i = node->children.begin();
		while(i != node->children.end() && (*i)->type != SNT_LBRACE)
		{
			if(name.empty())
				name = (*i)->token;
			else
				name = name + " " + (*i)->token;
			++i;
		}

		// Check for the '{'
		if(i == node->children.end())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}
		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		// Use the listener to get the particle system object
		if(mListener)
			mSystem = mListener->getParticleSystem(name, mGroup);
		else
			mSystem = ParticleSystemManager::getSingleton().createTemplate(name, mGroup);
		if(!mSystem)
		{
			addError(CE_OBJECTALLOCATIONERROR, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		mSystem->removeAllEmitters();
		mSystem->removeAllAffectors();

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!overrideNode(j, (*i)->children.end()))
			{
				if((*j)->wordID == ID_EMITTER)
				{
					compileEmitter(*j);
				}
				else if((*j)->wordID == ID_AFFECTOR)
				{
					compileAffector(*j);
				}
				else if((*j)->wordID == ID_MATERIAL)
				{
					String value = getParameterValue((*j)->children.begin(), (*j)->children.end());
					if(mListener)
						mListener->getMaterialName(value);
					if(!mSystem->setParameter("material", value))
					{
						if(mSystem->getRenderer())
						{
							if(!mSystem->getRenderer()->setParameter("material", value))
								addError(CE_INVALIDPROPERTY, (*j)->file, (*j)->line, (*j)->column);
						}
					}
				}
				else
				{
					// Each property in the particle system has only 1 value associated with it
					// Construct the parameter values from the children of the property
					String name = (*j)->token, value = getParameterValue((*j)->children.begin(), (*j)->children.end());
					if(!mSystem->setParameter(name, value))
					{
						if(mSystem->getRenderer())
						{
							if(!mSystem->getRenderer()->setParameter(name, value))
								addError(CE_INVALIDPROPERTY, (*j)->file, (*j)->line, (*j)->column);
						}
					}
				}
				++j;
			}
		}

		// Reset the pointer to the system
		mSystem = 0;
	}

	void ScriptCompiler::compileEmitter(const ScriptNodePtr &node)
	{
		if(node->children.empty() || node->children.front()->type != SNT_WORD)
			return;

		// Create the emitter based on the first child
		ParticleEmitter *emitter = 0;
		String type = node->children.front()->token;
		try{
			emitter = mSystem->addEmitter(type);
		}catch(...){
			addError(CE_OBJECTALLOCATIONERROR, node->children.front()->file, 
				node->children.front()->line, node->children.front()->column);
			return;
		}

		// Jump ahead now to the '{' as the emitter does not support other parameters in the header
		ScriptNodeList::iterator i = findNode(node->children.begin(), node->children.end(), SNT_LBRACE);
		if(i == node->children.end())
			return;

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!overrideNode(j, (*i)->children.end()))
			{
				String name = (*j)->token, 
					value = getParameterValue((*j)->children.begin(), (*j)->children.end());
				if(!emitter->setParameter(name, value))
					addError(CE_INVALIDPROPERTY, (*j)->file, (*j)->line, (*j)->column);
				++j;
			}
		}
	}

	void ScriptCompiler::compileAffector(const ScriptNodePtr &node)
	{
		if(node->children.empty() || node->children.front()->type != SNT_WORD)
			return;

		// Create the emitter based on the first child
		ParticleAffector *affector = 0;
		String type = node->children.front()->token;
		try{
			affector = mSystem->addAffector(type);
		}catch(...){
			addError(CE_OBJECTALLOCATIONERROR, node->children.front()->file, 
				node->children.front()->line, node->children.front()->column);
			return;
		}

		// Jump ahead now to the '{' as the emitter does not support other parameters in the header
		ScriptNodeList::iterator i = findNode(node->children.begin(), node->children.end(), SNT_LBRACE);
		if(i == node->children.end())
			return;

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!overrideNode(j, (*i)->children.end()))
			{
				String name = (*j)->token, 
					value = getParameterValue((*j)->children.begin(), (*j)->children.end());
				if(!affector->setParameter(name, value))
					addError(CE_INVALIDPROPERTY, (*j)->file, (*j)->line, (*j)->column);
				++j;
			}
		}
	}

	String ScriptCompiler::getParameterValue(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end)
	{
		String retval;
		if(i != end)
		{
			retval = (*i)->token;
		}

		++i;
		while(i != end)
		{
			retval = retval + " " + (*i)->token;
			++i;
		}

		return retval;
	}

	void ScriptCompiler::compileCompositor(const ScriptNodePtr &node)
	{
		if(node->children.empty())
		{
			addError(CE_STRINGEXPECTED, node->file, node->line, -1);
			return;
		}

		// Anything up until the '{' is put together to form the compositor name
		ScriptNodeList::iterator i = node->children.begin();
		String name = (*i)->token;
		++i;
		while(i != node->children.end() && (*i)->type != SNT_LBRACE)
		{
			name = name + " " + (*i)->token;
			++i;
		}

		// Verify we reached the '{'
		if(i == node->children.end())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}
		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		if(mListener)
			mCompositor = mListener->getCompositor(name, mGroup);
		else
			mCompositor = (Compositor*)CompositorManager::getSingleton().create(name, mGroup).get();

		if(!mCompositor)
		{
			addError(CE_OBJECTALLOCATIONERROR, node->file, node->line, node->column);
			return;
		}

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!overrideNode(j, (*i)->children.end()))
			{
				// Only techniques are supported here
				if((*j)->wordID == ID_TECHNIQUE)
				{
					compileCompositionTechnique(*j);
				}
				++j;
			}
		}

		mCompositor = 0;
	}

	void ScriptCompiler::compileCompositionTechnique(const Ogre::ScriptNodePtr &node)
	{
		// The technique immediately starts with a '{'
		if(node->children.empty())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, node->column);
			return;
		}

		ScriptNodeList::iterator i = node->children.begin();
		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, node->column);
			return;
		}

		CompositionTechnique *technique = mCompositor->createTechnique();

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!overrideNode(j, (*i)->children.end()))
			{
				if((*j)->wordID == ID_TEXTURE)
				{
					if((*j)->children.size() >= 4)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1),
							node3 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 2),
							node4 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 3);

						if(node2->wordID != ID_TARGET_WIDTH && node2->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node2->file, node2->line, node2->column);
							goto fail;
						}
						if(node3->wordID != ID_TARGET_HEIGHT && node3->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node3->file, node3->line, node3->column);
							goto fail;
						}

						PixelFormat format = PixelUtil::getFormatFromName(node4->token, true);
						CompositionTechnique::TextureDefinition *def = 
							technique->createTextureDefinition(node1->token);
						def->width = node2->wordID == ID_TARGET_WIDTH ? 0 : node2->data;
						def->height = node3->wordID == ID_TARGET_HEIGHT ? 0 : node3->data;
						def->formatList.push_back(format);
					}
					else
					{
						addError(CE_VALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_TARGET)
				{
					compileCompositionTarget(*j, technique);
				}
				else if((*j)->wordID == ID_TARGET_OUTPUT)
				{
					compileCompositionTargetOutput(*j, technique);
				}
fail:
				++j;
			}
		}
	}

	void ScriptCompiler::compileCompositionTarget(const Ogre::ScriptNodePtr &node, Ogre::CompositionTechnique *technique)
	{
		if(node->children.empty())
		{
			addError(CE_STRINGEXPECTED, node->file, node->line, -1);
			return;
		}

		// Anything up until the '{' is put together to form the compositor name
		ScriptNodeList::iterator i = node->children.begin();
		String name = (*i)->token;
		++i;
		while(i != node->children.end() && (*i)->type != SNT_LBRACE)
		{
			name = name + " " + (*i)->token;
			++i;
		}

		// Verify we reached the '{'
		if(i == node->children.end())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}
		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		// Create the target
		CompositionTargetPass *target = technique->createTargetPass();
		target->setOutputName(name);

		compileCompositionTargetOptions((*i)->children.begin(), (*i)->children.end(), target);		
	}

	void ScriptCompiler::compileCompositionTargetOutput(const Ogre::ScriptNodePtr &node, Ogre::CompositionTechnique *technique)
	{
		// The target_output immediately starts with a '{'
		if(node->children.empty())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, node->column);
			return;
		}

		ScriptNodeList::iterator i = node->children.begin();
		if((*i)->type != SNT_LBRACE)
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, node->column);
			return;
		}

		compileCompositionTargetOptions((*i)->children.begin(), (*i)->children.end(), technique->getOutputTargetPass());
	}

	void ScriptCompiler::compileCompositionPass(const Ogre::ScriptNodePtr &node, Ogre::CompositionTargetPass *target)
	{
		// The pass starts out with a type
		if(node->children.empty())
		{
			addError(CE_STRINGEXPECTED, node->file, node->line, -1);
			return;
		}

		ScriptNodeList::iterator i = node->children.begin();
		CompositionPass::PassType type;
		switch((*i)->wordID)
		{
		case ID_CLEAR:
			type = CompositionPass::PT_CLEAR;
			break;
		case ID_STENCIL:
			type = CompositionPass::PT_STENCIL;
			break;
		case ID_RENDER_SCENE:
			type = CompositionPass::PT_RENDERSCENE;
			break;
		case ID_RENDER_QUAD:
			type = CompositionPass::PT_RENDERQUAD;
			break;
		default:
			addError(CE_INVALIDPROPERTYVALUE, (*i)->file, (*i)->line, (*i)->column);
			return;
		}

		// Search for the '{'
		i = findNode(i, node->children.end(), SNT_LBRACE);
		if(i == node->children.end())
		{
			addError(CE_OPENBRACEEXPECTED, node->file, node->line, -1);
			return;
		}

		CompositionPass *pass = target->createPass();
		pass->setType(type);

		ScriptNodeList::iterator j = (*i)->children.begin();
		while(j != (*i)->children.end())
		{
			if(!overrideNode(j, (*i)->children.end()))
			{
				if((*j)->wordID == ID_MATERIAL)
				{
					if((*j)->children.empty())
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
						goto fail;
					}

					String name = (*j)->children.front()->token;
					if(mListener)
						mListener->getMaterialName(name);
					pass->setMaterialName(name);
				}
				else if((*j)->wordID == ID_INPUT)
				{
					if((*j)->children.size() >= 2)
					{
						ScriptNodePtr node1 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 0),
							node2 = getNodeAt((*j)->children.begin(), (*j)->children.end(), 1);
						if(node1->type != SNT_NUMBER)
						{
							addError(CE_INVALIDPROPERTYVALUE, node1->file, node1->line, node1->column);
							goto fail;
						}

						pass->setInput(node1->data, node2->token);
					}
					else
					{
						addError(CE_VALUEEXPECTED, (*j)->file, (*j)->line, -1);
					}
				}
				else if((*j)->wordID == ID_IDENTIFIER)
				{
					if((*j)->children.empty())
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
						goto fail;
					}

					ScriptNodePtr node = (*j)->children.front();
					if(node->type == SNT_NUMBER)
						pass->setIdentifier(StringConverter::parseUnsignedInt(node->token));
					else
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
				}
				else if((*j)->wordID == ID_FIRST_RENDER_QUEUE)
				{
					if((*j)->children.empty())
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
						goto fail;
					}

					ScriptNodePtr node = (*j)->children.front();
					if(node->type == SNT_NUMBER)
						pass->setFirstRenderQueue(StringConverter::parseUnsignedInt(node->token));
					else
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
				}
				else if((*j)->wordID == ID_LAST_RENDER_QUEUE)
				{
					if((*j)->children.empty())
					{
						addError(CE_STRINGEXPECTED, (*j)->file, (*j)->line, -1);
						goto fail;
					}

					ScriptNodePtr node = (*j)->children.front();
					if(node->type == SNT_NUMBER)
						pass->setLastRenderQueue(StringConverter::parseUnsignedInt(node->token));
					else
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
				}
				else if((*j)->wordID == ID_CLEAR)
				{
					ScriptNodeList::iterator k = findNode((*j)->children.begin(), (*j)->children.end(), SNT_LBRACE);
					if(k == (*j)->children.end())
					{
						addError(CE_OPENBRACEEXPECTED, (*j)->file, (*j)->line, -1);
						goto fail;
					}

					while(k != (*j)->children.end())
					{
						if(!overrideNode(k, (*j)->children.end()))
						{
							if((*k)->wordID == ID_BUFFERS)
							{
								if((*k)->children.empty())
								{
									addError(CE_STRINGEXPECTED, (*k)->file, (*k)->line, -1);
									goto clear_fail;
								}

								uint32 buffers = 0;
								for(ScriptNodeList::iterator m = (*j)->children.begin(); m != (*j)->children.end(); ++m)
								{
									switch((*m)->wordID)
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
										addError(CE_INVALIDPROPERTYVALUE, (*m)->file, (*m)->line, (*m)->column);
									}
								}

								pass->setClearBuffers(buffers);
							}
							else if((*k)->wordID == ID_COLOUR_VALUE)
							{
								if((*k)->children.empty())
								{
									addError(CE_STRINGEXPECTED, (*k)->file, (*k)->line, -1);
									goto clear_fail;
								}

								ColourValue c;
								if(getColourValue((*k)->children.begin(), (*k)->children.end(), c))
									pass->setClearColour(c);
								else
									addError(CE_INVALIDPROPERTYVALUE, (*j)->file, (*j)->line, -1);
							}
							else if((*k)->wordID == ID_DEPTH_VALUE)
							{
								if((*k)->children.empty())
								{
									addError(CE_NUMBEREXPECTED, (*k)->file, (*k)->line, -1);
									goto clear_fail;
								}

								ScriptNodePtr node = (*k)->children.front();
								if(node->type == SNT_NUMBER)
									pass->setClearDepth(node->data);
								else
									addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
							}
							else if((*k)->wordID == ID_STENCIL_VALUE)
							{
								if((*k)->children.empty())
								{
									addError(CE_NUMBEREXPECTED, (*k)->file, (*k)->line, -1);
									goto clear_fail;
								}

								ScriptNodePtr node = (*k)->children.front();
								if(node->type == SNT_NUMBER)
									pass->setClearStencil(node->data);
								else
									addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
							}
clear_fail:
							++k;
						}
					}
				}
				else if((*j)->wordID == ID_STENCIL)
				{
					ScriptNodeList::iterator k = findNode((*j)->children.begin(), (*j)->children.end(), SNT_LBRACE);
					if(k == (*j)->children.end())
					{
						addError(CE_OPENBRACEEXPECTED, (*j)->file, (*j)->line, -1);
						goto fail;
					}

					while(k != (*j)->children.end())
					{
						if(!overrideNode(k, (*j)->children.end()))
						{
							if((*k)->wordID == ID_CHECK)
							{
								if((*k)->children.empty())
								{
									addError(CE_TRUTHVALUEEXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								bool val = true;
								ScriptNodePtr node = (*k)->children.front();
								if(getTruthValue(node, val))
									pass->setStencilCheck(val);
								else
									addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
							}
							else if((*k)->wordID == ID_COMP_FUNC)
							{
								if((*k)->children.empty())
								{
									addError(CE_TRUTHVALUEEXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								CompareFunction func;
								ScriptNodePtr node = (*k)->children.front();
								if(getCompareFunction(node, func))
									pass->setStencilFunc(func);
								else
									addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
							}
							else if((*k)->wordID == ID_REF_VALUE)
							{
								if((*k)->children.empty())
								{
									addError(CE_NUMBEREXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								ScriptNodePtr node = (*k)->children.front();
								if(node->type == SNT_NUMBER)
									pass->setStencilRefValue(node->data);
								else
									addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
							}
							else if((*k)->wordID == ID_MASK)
							{
								if((*k)->children.empty())
								{
									addError(CE_NUMBEREXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								ScriptNodePtr node = (*k)->children.front();
								if(node->type == SNT_NUMBER)
									pass->setStencilMask(StringConverter::parseUnsignedInt(node->token));
								else
									addError(CE_INVALIDPROPERTYVALUE, (*k)->file, (*k)->line, (*k)->column);
							}
							else if((*k)->wordID == ID_FAIL_OP)
							{
								if((*k)->children.empty())
								{
									addError(CE_TRUTHVALUEEXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								StencilOperation op;
								ScriptNodePtr node = (*k)->children.front();
								if(getStencilOp(node, op))
									pass->setStencilFailOp(op);
								else
									addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
							}
							else if((*k)->wordID == ID_DEPTH_FAIL_OP)
							{
								if((*k)->children.empty())
								{
									addError(CE_TRUTHVALUEEXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								StencilOperation op;
								ScriptNodePtr node = (*k)->children.front();
								if(getStencilOp(node, op))
									pass->setStencilDepthFailOp(op);
								else
									addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
							}
							else if((*k)->wordID == ID_PASS_OP)
							{
								if((*k)->children.empty())
								{
									addError(CE_TRUTHVALUEEXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								StencilOperation op;
								ScriptNodePtr node = (*k)->children.front();
								if(getStencilOp(node, op))
									pass->setStencilPassOp(op);
								else
									addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
							}
							if((*k)->wordID == ID_TWO_SIDED)
							{
								if((*k)->children.empty())
								{
									addError(CE_TRUTHVALUEEXPECTED, (*k)->file, (*k)->line, -1);
									goto stencil_fail;
								}

								bool val = true;
								ScriptNodePtr node = (*k)->children.front();
								if(getTruthValue(node, val))
									pass->setStencilTwoSidedOperation(val);
								else
									addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
							}
stencil_fail:
							++k;
						}
					}
				}
fail:
				++j;
			}
		}
	}

	void ScriptCompiler::compileCompositionTargetOptions(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Ogre::CompositionTargetPass *target)
	{
		while(i != end)
		{
			if(!overrideNode(i, end))
			{
				if((*i)->wordID == ID_INPUT)
				{
					if((*i)->children.empty())
					{
						addError(CE_STRINGEXPECTED, (*i)->file, (*i)->line, -1);
						goto fail;
					}
					ScriptNodePtr node = (*i)->children.front();
					switch(node->wordID)
					{
					case ID_NONE:
						target->setInputMode(CompositionTargetPass::IM_NONE);
						break;
					case ID_PREVIOUS:
						target->setInputMode(CompositionTargetPass::IM_PREVIOUS);
						break;
					default:
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
						goto fail;
					}
				}
				else if((*i)->wordID == ID_ONLY_INITIAL)
				{
					if((*i)->children.empty())
					{
						addError(CE_TRUTHVALUEEXPECTED, (*i)->file, (*i)->line, -1);
						goto fail;
					}

					bool val = false;
					ScriptNodePtr node = (*i)->children.front();
					if(getTruthValue(node, val))
						target->setOnlyInitial(val);
					else
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
				}
				else if((*i)->wordID == ID_VISIBILITY_MASK)
				{
					if((*i)->children.empty())
					{
						addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, -1);
						goto fail;
					}

					// Notice we don't use data here!
					// Just in case we do a conversion straight to an integer because we could lose
					// large numbers!
					ScriptNodePtr node = (*i)->children.front();
					if(node->type == SNT_NUMBER)
						target->setVisibilityMask(StringConverter::parseUnsignedInt(node->token));
					else
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
				}
				else if((*i)->wordID == ID_LOD_BIAS)
				{
					if((*i)->children.empty())
					{
						addError(CE_NUMBEREXPECTED, (*i)->file, (*i)->line, -1);
						goto fail;
					}

					ScriptNodePtr node = (*i)->children.front();
					if(node->type == SNT_NUMBER)
						target->setLodBias(node->data);
					else
						addError(CE_INVALIDPROPERTYVALUE, node->file, node->line, node->column);
				}
				else if((*i)->wordID == ID_MATERIAL_SCHEME)
				{
					if((*i)->children.empty())
					{
						addError(CE_STRINGEXPECTED, (*i)->file, (*i)->line, -1);
						goto fail;
					}

					ScriptNodePtr node = (*i)->children.front();
					target->setMaterialScheme(node->token);
				}
				else if((*i)->wordID == ID_PASS)
				{
					compileCompositionPass(*i, target);
				}
fail:
				++i;
			}
		}
	}

	bool ScriptCompiler::getStencilOp(const ScriptNodePtr &node, StencilOperation &op)
	{
		bool success = true;
		switch(node->wordID)
		{
		case ID_KEEP:
			op = SOP_KEEP;
			break;
		case ID_ZERO:
			op = SOP_ZERO;
			break;
		case ID_REPLACE:
			op = SOP_REPLACE;
			break;
		case ID_INCREMENT:
			op = SOP_INCREMENT;
			break;
		case ID_DECREMENT:
			op = SOP_DECREMENT;
			break;
		case ID_INCREMENT_WRAP:
			op = SOP_INCREMENT_WRAP;
			break;
		case ID_DECREMENT_WRAP:
			op = SOP_DECREMENT_WRAP;
			break;
		case ID_INVERT:
			op = SOP_INVERT;
			break;
		default:
			success = false;
		}
		return success;
	}

	// ScriptCompilerManager
	ScriptCompilerManager::ScriptCompilerManager()
		:mListener(0)
	{
		OGRE_THREAD_POINTER_SET(mCompiler, new ScriptCompiler());

		mScriptPatterns.push_back("*.os");
#if OGRE_USE_NEW_COMPILERS
		ResourceGroupManager::getSingleton()._registerScriptLoader(this);
#endif
	}

	ScriptCompilerManager::~ScriptCompilerManager()
	{
		OGRE_THREAD_POINTER_DELETE(mCompiler);
	}

	const StringVector &ScriptCompilerManager::getScriptPatterns() const
	{
		return mScriptPatterns;
	}

	void ScriptCompilerManager::parseScript(DataStreamPtr &stream, const String &groupName)
	{
#if OGRE_THREAD_SUPPORT
		if(!mCompiler.get())
			mCompiler.reset(new ScriptCompiler());
#endif

		mCompiler->setListener(mListener);
		mCompiler->compile(stream, groupName);
	}

	Real ScriptCompilerManager::getLoadingOrder() const
	{
		return 100.0f;
	}
}

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

#ifndef __MATERIALSCRIPTCOMPILER2_H_
#define __MATERIALSCRIPTCOMPILER2_H_

#include "OgreScriptCompiler.h"
#include "OgrePixelFormat.h"
#include "OgreTextureUnitState.h"

namespace Ogre{

	// Forward declaration for the compiler
	class MaterialScriptCompiler2;

	/// This is the listener for the new compiler, which allows for custom behavior and information callbacks during compilation.
	class _OgreExport MaterialScriptCompilerListener : public ScriptCompilerListener
	{
	public:
		MaterialScriptCompilerListener();
		/// Override this to do custom processing of the script nodes
		virtual bool processNode(ScriptNodeList::iterator &iter, ScriptNodeList::iterator &end, MaterialScriptCompiler2*);
		/// Overriding this method allows for custom material allocation
		virtual Material *getMaterial(const String &name, const String &group);
		/// Overriding this method allows for custom asm GPU program allocation
		virtual GpuProgram *getGpuProgram(const String &name, const String &group, GpuProgramType type, const String &syntax, const String &source);
		/// Overriding this method allows for custom high-level GPU program allocation
		virtual HighLevelGpuProgram *getHighLevelGpuProgram(const String &name, const String &group, GpuProgramType type, const String &language, const String &source);
		/// This is called just before texture aliases found in a script are applied to a material
		virtual void preApplyTextureAliases(Ogre::AliasTextureNamePairList &aliases);
		/// This accepts the name of a texture to be referenced, and is expected to ensure it loads and return the real name for it
		virtual void getTexture(String *names, int count);
	};
	
	/** This is the new compiler for material scripts. It uses the parser to parse the material
		and this class processes the token stream that is produced and compiles it into Ogre materials.
	*/
	class _OgreExport MaterialScriptCompiler2 : public ScriptCompiler
	{
	public:
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
		};
	public:
		/// Constructs a new compiler
		MaterialScriptCompiler2();

		/// Sets the listener to override behavior
		void setListener(MaterialScriptCompilerListener *listener);
	protected:
		/// This begins the compilation of the particle system from the final transformed AST
		bool compileImpl(ScriptNodeListPtr nodes);
		/// Delegates to the listener if it can, otherwise returns false. If it returns true, then some input was consumed.
		bool processNode(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);
		/// This is the override for loading imports
		ScriptNodeListPtr loadImportPath(const String &name);
		/// Allows a listener to override the word id map before parsing
		void preParse();
		/// Allows a listener to override error handling in the compiler
		bool errorRaised(const ScriptCompilerErrorPtr &error);
	private: // Private handlers to compile pieces of the material script
		void compileMaterial(const ScriptNodePtr &node);
		void compileTechnique(const ScriptNodePtr &node);
		void compilePass(const ScriptNodePtr &node, Technique *technique);
		void compileTextureUnit(const ScriptNodePtr &node, Pass *pass);
		void compileGpuProgram(const ScriptNodePtr &node);
		void compileAsmGpuProgram(const String &name, const ScriptNodePtr &node);
		void compileHighLevelGpuProgram(const String &name, const String &language, const ScriptNodePtr &node);
		void compileUnifiedHighLevelGpuProgram(const String &name, const ScriptNodePtr &node);
		void compileProgramParameters(const ScriptNodePtr &node, const GpuProgramParametersSharedPtr &params);
		bool getColourValue(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, ColourValue &c);
		bool getBlendFactor(const ScriptNodePtr &node, SceneBlendFactor &sbf);
		bool getCompareFunction(const ScriptNodePtr &node, CompareFunction &func);
		bool getTextureAddressingMode(const ScriptNodePtr &node, TextureUnitState::TextureAddressingMode &mode);
		bool getColourOperation(const ScriptNodePtr &node, Ogre::LayerBlendOperationEx &op);
		bool getColourOperationSource(const ScriptNodePtr &node, Ogre::LayerBlendSource &source);
		bool getMatrix4(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, Matrix4 &m);
		bool getInts(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, int *vals, int count);
		bool getFloats(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end, float *vals, int count);
	private:
		// The listener
		MaterialScriptCompilerListener *mListener;
		// This is the material being compiled
		Material *mMaterial;
		// Stores the texture aliases applied to a compiling material
		Ogre::AliasTextureNamePairList mTextureAliases;
	};

}

#endif

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

#ifndef __SCRIPTCOMPILER_H_
#define __SCRIPTCOMPILER_H_

#include "OgreSharedPtr.h"
#include "OgreMaterial.h"
#include "OgreHighLevelGpuProgram.h"

namespace Ogre
{
	/** These enums hold the types of the concrete parsed nodes */
	enum ConcreteNodeType
	{
		CNT_VARIABLE,
		CNT_VARIABLE_ASSIGN,
		CNT_WORD,
		CNT_IMPORT,
		CNT_QUOTE,
		CNT_LBRACE,
		CNT_RBRACE,
		CNT_COLON
	};

	/** The ConcreteNode is the struct that holds an un-conditioned sub-tree of parsed input */
	struct ConcreteNode;
	typedef SharedPtr<ConcreteNode> ConcreteNodePtr;
	typedef std::list<ConcreteNodePtr> ConcreteNodeList;
	typedef SharedPtr<ConcreteNodeList> ConcreteNodeListPtr;
	struct ConcreteNode
	{
		String token, file;
		unsigned int line;
		ConcreteNodeType type;
		ConcreteNodeList children;
		ConcreteNode *parent;
	};

	/** This enum holds the types of the possible abstract nodes */
	enum AbstractNodeType
	{
		ANT_UNKNOWN,
		ANT_ATOM,
		ANT_OBJECT,
		ANT_PROPERTY,
		ANT_IMPORT,
		ANT_VARIABLE_SET,
		ANT_VARIABLE_ACCESS
	};
	class AbstractNode;
	typedef SharedPtr<AbstractNode> AbstractNodePtr;
	typedef std::list<AbstractNodePtr> AbstractNodeList;
	typedef SharedPtr<AbstractNodeList> AbstractNodeListPtr;

	class _OgreExport AbstractNode
	{
	public:
		String file;
		unsigned int line;
		AbstractNodeType type;
		AbstractNode *parent;
	public:
		AbstractNode(AbstractNode *ptr);
		virtual AbstractNode *clone() const = 0; 
	};

	/** This is an abstract node which cannot be broken down further */
	class _OgreExport AtomAbstractNode : public AbstractNode
	{
	private:
		mutable bool mIsNumber, mNumberTest;
		mutable Real mNum;
	public:
		String value;
		uint32 id;
	public:
		AtomAbstractNode(AbstractNode *ptr);
		AbstractNode *clone() const;
		bool isNumber() const;
		Real getNumber() const;
	private:
		void parseNumber() const;
	};

	/** This specific abstract node represents a script object */
	class _OgreExport ObjectAbstractNode : public AbstractNode
	{
	private:
		std::map<String,String> mEnv;
	public:
		String name, cls, base;
		uint32 id;
		bool abstract;
		AbstractNodeList children;
		AbstractNodeList values;
	public:
		ObjectAbstractNode(AbstractNode *ptr);
		AbstractNode *clone() const;

		void addVariable(const String &name);
		void setVariable(const String &name, const String &value);
		std::pair<bool,String> getVariable(const String &name) const;
		const std::map<String,String> &getVariables() const;
	};

	/** This abstract node represents a script property */
	class _OgreExport PropertyAbstractNode : public AbstractNode
	{
	public:
		String name;
		uint32 id;
		AbstractNodeList values;
	public:
		PropertyAbstractNode(AbstractNode *ptr);
		AbstractNode *clone() const;
	};

	/** This abstract node represents an import statement */
	class _OgreExport ImportAbstractNode : public AbstractNode
	{
	public:
		String target, source;
	public:
		ImportAbstractNode();
		AbstractNode *clone() const;
	};

	/** This abstract node represents a variable assignment */
	class _OgreExport VariableAccessAbstractNode : public AbstractNode
	{
	public:
		String name;
	public:
		VariableAccessAbstractNode(AbstractNode *ptr);
		AbstractNode *clone() const;
	};

	class ScriptCompilerListener;

	/** This is the main class for the compiler. It calls the parser
		and processes the CST into an AST and then uses translators
		to translate the AST into the final resources.
	*/
	class _OgreExport ScriptCompiler
	{
	public: // Externally accessible types
		typedef std::map<String,uint32> IdMap;

		// The container for errors
		struct Error
		{
			String file;
			int line;
			uint32 code;
		};
		typedef SharedPtr<Error> ErrorPtr;
		typedef std::list<ErrorPtr> ErrorList;

		// These are the built-in error codes
		enum{
			CE_STRINGEXPECTED,
			CE_NUMBEREXPECTED,
			CE_FEWERPARAMETERSEXPECTED,
			CE_VARIABLEEXPECTED,
			CE_UNDEFINEDVARIABLE,
			CE_OBJECTNAMEEXPECTED,
			CE_OBJECTALLOCATIONERROR,
			CE_INVALIDPARAMETERS
		};
	public:
		ScriptCompiler();

		/// Takes in a string of script code and compiles it into resources
		/**
		 * @param str The script code
		 * @param source The source of the script code (e.g. a script file)
		 * @param group The resource group to place the compiled resources into
		 */
		bool compile(const String &str, const String &source, const String &group);
		/// Compiles resources from the given concrete node list
		bool compile(const ConcreteNodeListPtr &nodes, const String &group);
		/// Adds the given error to the compiler's list of errors
		void addError(uint32 code, const String &file, int line);
		/// Sets the listener used by the compiler
		void setListener(ScriptCompilerListener *listener);
		/// Returns the resource group currently set for this compiler
		const String &getResourceGroup() const;
	private: // Tree processing
		AbstractNodeListPtr convertToAST(const ConcreteNodeListPtr &nodes);
		/// This built-in function processes import nodes
		void processImports(AbstractNodeListPtr &nodes);
		/// Loads the requested script and converts it to an AST
		AbstractNodeListPtr loadImportPath(const String &name);
		/// Returns the abstract nodes from the given tree which represent the target
		AbstractNodeListPtr locateTarget(AbstractNodeList *nodes, const String &target);
		/// Handles object inheritance and variable expansion
		void processObjects(AbstractNodeList *nodes, const AbstractNodeListPtr &top);
		/// Handles processing the variables
		void processVariables(AbstractNodeList *nodes);
		/// This function overlays the given object on the destination object following inheritance rules
		void overlayObject(const AbstractNodePtr &source, ObjectAbstractNode *dest);
		/// This function sets up the initial values in word id map
		void initWordMap();
	private:
		// Resource group
		String mGroup;
		// The word -> id conversion table
		IdMap mIds;
		// This is an environment map
		typedef std::map<String,String> Environment;
		Environment mEnv;

		typedef std::map<String,AbstractNodeListPtr> ImportCacheMap;
		ImportCacheMap mImports; // The set of imported scripts to avoid circular dependencies
		typedef std::multimap<String,String> ImportRequestMap;
		ImportRequestMap mImportRequests; // This holds the target objects for each script to be imported

		// This stores the imports of the scripts, so they are separated and can be treated specially
		AbstractNodeList mImportTable;

		// Error list
		ErrorList mErrors;

		// The listener
		ScriptCompilerListener *mListener;
	private: // Internal helper classes and processors
		class AbstractTreeBuilder
		{
		private:
			AbstractNodeListPtr mNodes;
			AbstractNode *mCurrent;
			ScriptCompiler *mCompiler;
		public:
			AbstractTreeBuilder(ScriptCompiler *compiler);
			const AbstractNodeListPtr &getResult() const;
			void visit(ConcreteNode *node);
			static void visit(AbstractTreeBuilder *visitor, const ConcreteNodeList &nodes);
		};
		friend class AbstractTreeBuilder;
	public: // Public translater definitions
		// This enum are built-in word id values
		enum
		{
			ID_ON = 1,
			ID_OFF = 0,
			ID_TRUE = 1,
			ID_FALSE = 0,
			ID_YES = 1,
			ID_NO = 0
		};
		// This is the translator base class
		class Translator
		{
		private:
			ScriptCompiler *mCompiler;
		protected:
			ScriptCompiler *getCompiler();
			ScriptCompilerListener *getCompilerListener();
			AbstractNodeList::const_iterator getNodeAt(const AbstractNodeList &nodes, int index);
			bool getBoolean(const AbstractNodePtr &node, bool *result);
			bool getString(const AbstractNodePtr &node, String *result);
			bool getNumber(const AbstractNodePtr &node, Real *result);
			bool getColour(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, ColourValue *result);
			bool getSceneBlendFactor(const AbstractNodePtr &node, SceneBlendFactor *sbf);
			bool getCompareFunction(const AbstractNodePtr &node, CompareFunction *func);
			bool getMatrix4(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, Matrix4 *m);
			bool getInts(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, int *vals, int count);
			bool getFloats(AbstractNodeList::const_iterator i, AbstractNodeList::const_iterator end, float *vals, int count);
		public:
			Translator(ScriptCompiler *compiler);
			/// This static translation function requests a translation on the given node
			static void translate(Translator *translator, const AbstractNodePtr &node);
		protected:
			/// This function is called to process each object node
			virtual void processObject(ObjectAbstractNode*) = 0;
			/// This function is called to process each property node
			virtual void processProperty(PropertyAbstractNode*) = 0;
		};
		friend class Translater;
		class MaterialTranslator : public Translator
		{
		private:
			MaterialPtr mMaterial;
			Ogre::AliasTextureNamePairList mTextureAliases;
		public:
			MaterialTranslator(ScriptCompiler *compiler);
			void processObject(ObjectAbstractNode*);
			void processProperty(PropertyAbstractNode*);
		};
		class TechniqueTranslator : public Translator
		{
		private:
			Technique *mTechnique;
		public:
			TechniqueTranslator(ScriptCompiler *compiler, Technique *technique);
			void processObject(ObjectAbstractNode*);
			void processProperty(PropertyAbstractNode*);
		};
		class PassTranslator : public Translator
		{
		private:
			Pass *mPass;
		public:
			PassTranslator(ScriptCompiler *compiler, Pass *pass);
			void processObject(ObjectAbstractNode*);
			void processProperty(PropertyAbstractNode*);
		};
		class TextureUnitTranslator : public Translator
		{
		private:
			TextureUnitState *mUnit;
		public:
			TextureUnitTranslator(ScriptCompiler *compiler, TextureUnitState *unit);
			void processObject(ObjectAbstractNode*);
			void processProperty(PropertyAbstractNode*);
		};
		class GpuProgramTranslator : public Translator
		{
		public:
			GpuProgramTranslator(ScriptCompiler *compiler);
			void processObject(ObjectAbstractNode*);
			void processProperty(PropertyAbstractNode*);
		};
		class HighLevelGpuProgramTranslator : public Translator
		{
		public:
			HighLevelGpuProgramTranslator(ScriptCompiler *compiler);
			void processObject(ObjectAbstractNode*);
			void processProperty(PropertyAbstractNode*);
		};
		class UnifiedGpuProgramTranslator : public Translator
		{
		public:
			UnifiedGpuProgramTranslator(ScriptCompiler *compiler);
			void processObject(ObjectAbstractNode*);
			void processProperty(PropertyAbstractNode*);
		};
		class GpuProgramParametersTranslator : public Translator
		{
		private:
			GpuProgramParametersSharedPtr mParams;
			int mAnimParametricsCount;
		public:
			GpuProgramParametersTranslator(ScriptCompiler *compiler, const GpuProgramParametersSharedPtr &params);
			void processObject(ObjectAbstractNode*);
			void processProperty(PropertyAbstractNode*);
		};
		class ParticleSystemTranslator : public Translator
		{
		private:
			Ogre::ParticleSystem *mSystem;
		public:
			ParticleSystemTranslator(ScriptCompiler *compiler);
			void processObject(ObjectAbstractNode*);
			void processProperty(PropertyAbstractNode*);
		};
		class ParticleEmitterTranslator : public Translator
		{
		private:
			Ogre::ParticleEmitter *mEmitter;
		public:
			ParticleEmitterTranslator(ScriptCompiler *compiler, ParticleEmitter *emitter);
			void processObject(ObjectAbstractNode*);
			void processProperty(PropertyAbstractNode*);
		};
		class ParticleAffectorTranslator : public Translator
		{
		private:
			Ogre::ParticleAffector *mAffector;
		public:
			ParticleAffectorTranslator(ScriptCompiler *compiler, ParticleAffector *affector);
			void processObject(ObjectAbstractNode*);
			void processProperty(PropertyAbstractNode*);
		};
	};

	/** This is a listener for the compiler. The compiler can be customized with
		this listener. It lets you listen in on events occuring during compilation,
		hook them, and change the behavior.
	*/
	class _OgreExport ScriptCompilerListener
	{
	public:
		ScriptCompilerListener();

		/// Returns the concrete node list from the given file
		virtual ConcreteNodeListPtr importFile(const String &name);
		/// Allows for responding to and overriding behavior before a CST is translated into an AST
		virtual void preASTConversion(ConcreteNodeListPtr nodes, ScriptCompiler::IdMap *ids);
		/// Allows for overriding the translation of the given node into the concrete resource.
		virtual std::pair<bool,ScriptCompiler::Translator*> preObjectTranslation(ObjectAbstractNode *obj);
		/// Allows for overriding the translation of the given node into the concrete resource.
		virtual std::pair<bool,ScriptCompiler::Translator*> prePropertyTranslation(PropertyAbstractNode *prop);
		/// Called when an error occurred
		virtual void error(const ScriptCompiler::ErrorPtr &err);

		/// Must return the requested material
		virtual MaterialPtr createMaterial(const String &name, const String &group);
		/// Called before texture aliases are applied to a material
		virtual void preApplyTextureAliases(Ogre::AliasTextureNamePairList *aliases);
		/// Called before texture names are used
		virtual void getTextureNames(String *names, int count = 0);
		/// Called before a gpu program name is used
		virtual void getGpuProgramName(String *name);
		/// Called to return the requested GpuProgram
		virtual GpuProgramPtr createGpuProgram(const String &name, const String &group, const String &source, GpuProgramType type, const String &syntax);
		/// Called to return a HighLevelGpuProgram
		virtual HighLevelGpuProgramPtr createHighLevelGpuProgram(const String &name, const String &group, const String &language, GpuProgramType type, const String &source);

		/// Returns the requested particle system template
		virtual ParticleSystem *createParticleSystem(const String &name, const String &group);
		/// Processes the name of the material
		virtual void getMaterialName(String *name);
	};

	/// This enum defines the integer ids for keywords this compiler handles
	enum
	{
		ID_MATERIAL = 2,
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
			ID_TWO_SIDED,
		ID_END_BUILTIN_IDS
	};
}

#endif

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
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
-----------------------------------------------------------------------------
*/
#ifndef __MaterialSerializer_H__
#define __MaterialSerializer_H__

#include "OgrePrerequisites.h"
#include "OgreMaterial.h"
#include "OgreBlendMode.h"
#include "OgreTextureUnitState.h"
#include "OgreGpuProgram.h"

namespace Ogre {

    /** Enum to identify material sections. */
    enum MaterialScriptSection
    {
        MSS_NONE,
        MSS_MATERIAL,
        MSS_TECHNIQUE,
        MSS_PASS,
        MSS_TEXTUREUNIT,
        MSS_PROGRAM_REF,
		MSS_PROGRAM,
		MSS_TEXTURESOURCE
    };
	/** Struct for holding a program definition which is in progress. */
	struct MaterialScriptProgramDefinition
	{
		String name;
		GpuProgramType progType;
        String language;
		String source;
		String syntax;
        bool supportsSkeletalAnimation;
		std::map<String, String> customParameters;
	};
    /** Struct for holding the script context while parsing. */
    struct MaterialScriptContext 
    {
        MaterialScriptSection section;
        Material* material;
        Technique* technique;
        Pass* pass;
        TextureUnitState* textureUnit;
        GpuProgram* program; // used when referencing a program, not when defining it
        bool isProgramShadowCaster; // when referencing, are we in context of shadow caster
        bool isProgramShadowReceiver; // when referencing, are we in context of shadow caster
        GpuProgramParametersSharedPtr programParams;
		MaterialScriptProgramDefinition* programDef; // this is used while defining a program

		int techLev,	//Keep track of what tech, pass, and state level we are in
			passLev,
			stateLev;

		// Error reporting state
        size_t lineNo;
        String filename;
    };
    /// Function def for material attribute parser; return value determines if the next line should be {
    typedef bool (*ATTRIBUTE_PARSER)(String& params, MaterialScriptContext& context);

    /** Class for serializing Materials to / from a .material script.*/
    class _OgreExport MaterialSerializer
    {
    protected:
        /// Keyword-mapped attribute parsers.
        typedef std::map<String, ATTRIBUTE_PARSER> AttribParserList;

        MaterialScriptContext mScriptContext;

        /** internal method for parsing a material
        @returns true if it expects the next line to be a {
        */
        bool parseScriptLine(String& line);
        /** internal method for finding & invoking an attribute parser. */
        bool invokeParser(String& line, AttribParserList& parsers);
		/** Internal method for saving a program definition which has been
		    built up.
		*/
		void finishProgramDefinition(void);
        /// Parsers for the root of the material script
        AttribParserList mRootAttribParsers;
        /// Parsers for the material section of a script
        AttribParserList mMaterialAttribParsers;
        /// Parsers for the technique section of a script
        AttribParserList mTechniqueAttribParsers;
        /// Parsers for the pass section of a script
        AttribParserList mPassAttribParsers;
        /// Parsers for the texture unit section of a script
        AttribParserList mTextureUnitAttribParsers;
        /// Parsers for the program reference section of a script
        AttribParserList mProgramRefAttribParsers;
        /// Parsers for the program definition section of a script
        AttribParserList mProgramAttribParsers;

        void writeMaterial(const Material *pMat);
        void writeTechnique(const Technique* pTech);
        void writePass(const Pass* pPass);
		void writeTextureUnit(const TextureUnitState *pTex);

		void writeSceneBlendFactor(const SceneBlendFactor sbf_src, const SceneBlendFactor sbf_dest);
		void writeSceneBlendFactor(const SceneBlendFactor sbf);
		void writeCompareFunction(const CompareFunction cf);
		void writeColourValue(const ColourValue &colour, bool writeAlpha = false);
		void writeLayerBlendOperationEx(const LayerBlendOperationEx op);
		void writeLayerBlendSource(const LayerBlendSource lbs);
		
		typedef std::multimap<TextureUnitState::TextureEffectType, TextureUnitState::TextureEffect> EffectMap;

		void writeRotationEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex);
		void writeTransformEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex);
		void writeScrollEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex);
		void writeEnvironmentMapEffect(const TextureUnitState::TextureEffect& effect, const TextureUnitState *pTex);

        String convertFiltering(FilterOptions fo);
    public:
		/** default constructor*/
		MaterialSerializer();
		/** default destructor*/
		virtual ~MaterialSerializer() {};

		/** Queue an in-memory Material to the internal buffer for export.*/
        void queueForExport(const Material *pMat, bool clearQueued = false, bool exportDefaults = false);
        /** Exports queued material(s) to a named material script file. */
		void exportQueued(const String& filename);
        /** Exports an in-memory Material to the named material script file. */
        void exportMaterial(const Material *pMat, const String& filename, bool exportDefaults = false);
		/** Returns a string representing the parsed material(s) */
		const String &getQueuedAsString() const;
		/** Clears the internal buffer */
		void clearQueue();

        /** Parses a Material script file passed as a chunk.
        @remarks
            The filename is optional, if specified it will appear in the log
            of any errors which are reported.
        */
        void parseScript(DataChunk& chunk, const String& filename = "");



	private:
		String mBuffer;
		bool mDefaults;

		void beginSection(unsigned short level)
		{
			mBuffer += "\n";
            for (unsigned short i = 0; i < level; ++i)
            {
                mBuffer += "\t";
            }
            mBuffer += "{";
		}
		void endSection(unsigned short level)
		{
			mBuffer += "\n";
            for (unsigned short i = 0; i < level; ++i)
            {
                mBuffer += "\t";
            }
            mBuffer += "}";
		}

		void writeAttribute(unsigned short level, const String& att)
		{
			mBuffer += "\n";
            for (unsigned short i = 0; i < level; ++i)
            {
                mBuffer += "\t";
            }
            mBuffer += att;
		}

		void writeValue(const String& val)
		{
			mBuffer += (" " + val);
		}

		void writeComment(unsigned short level, const String& comment)
		{
			mBuffer += "\n";
            for (unsigned short i = 0; i < level; ++i)
            {
                mBuffer += "\t";
            }
            mBuffer += "// " + comment;
		}

    };
}
#endif

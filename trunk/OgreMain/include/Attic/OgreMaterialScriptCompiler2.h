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
		virtual GpuProgram *getGpuProgram(const String &name, const String &group, GpuProgramType type, const String &syntax);
		/// Overriding this method allows for custom high-level GPU program allocation
		virtual HighLevelGpuProgram *getHighLevelGpuProgram(const String &name, const String &group, GpuProgramType type, const String &language);
		/// This is called just before texture aliases found in a script are applied to a material
		virtual void preApplyTextureAliases(Ogre::AliasTextureNamePairList &aliases);
	};

	/// This is the enum for material compiler specific errors
	enum
	{
		ME_LODLISTEXPECTED = CE_BASE_ERRORS_END,
		ME_TECHNIQUEBODYEXPECTED
	};
	
	/** This is the new compiler for material scripts. It uses the parser to parse the material
		and this class processes the token stream that is produced and compiles it into Ogre materials.
	*/
	class _OgreExport MaterialScriptCompiler2 : public ScriptCompiler
	{
	public:
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
	private: // Private handlers to compile pieces of the material script
		void compileMaterial(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);
		void compileTechnique(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);
		void compilePass(ScriptNodeList::iterator &i, ScriptNodeList::iterator &end);
	private:
		// The listener
		MaterialScriptCompilerListener *mListener;
		// This is the material being compiled
		Material *mMaterial;
		// Stores the texture aliases applied to a compiling material
		Ogre::AliasTextureNamePairList mTextureAliases;
		// This is the GpuProgram that is being set up
		GpuProgram *mProgram;
		// This is the GpuProgramParameters being built for the program
		GpuProgramParametersSharedPtr mProgramParameters;
	};

}

#endif

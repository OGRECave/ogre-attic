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

#ifndef __PARTICLESCRIPTCOMPILER_H_
#define __PARTICLESCRIPTCOMPILER_H_

#include "OgreScriptCompiler.h"

namespace Ogre{

	/** This is the specific listener for the particle script compiler.
		It allows overriding behavior for specific functionality of this compiler.
	*/
	class _OgreExport ParticleScriptCompilerListener : public ScriptCompilerListener
	{
	public:
		ParticleScriptCompilerListener();
	};

	class _OgreExport ParticleScriptCompiler : public ScriptCompiler
	{
	public:
		ParticleScriptCompiler();

		/// Sets the listener for this compiler
		void setListener(ParticleScriptCompilerListener *listener);
	protected:
		/// This begins the compilation of the particle system from the final transformed AST
		bool compileImpl(ScriptNodeListPtr nodes);
		/// This is the special particle implementation of the processObjects function
		void processObjects(ScriptNodeList &nodes, const ScriptNodeListPtr &top);
		/// This is the override for loading imports
		ScriptNodeListPtr loadImportPath(const String &name);
		/// This is the override for locating import targets
		ScriptNodeListPtr locateImportTarget(ScriptNodeListPtr nodes, const String &target);
		/// Determines if the particle object already exists in the given tree
		bool containsObject(const ScriptNodeList &nodes, const String &name);
	private:
		ParticleScriptCompilerListener *mListener;
	};

}

#endif

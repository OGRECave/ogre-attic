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
#include "OgreParticleScriptCompiler.h"

namespace Ogre{

	// ParticleScriptCompilerListener
	ParticleScriptCompilerListener::ParticleScriptCompilerListener()
	{
	}

	// ParticleScriptCompiler
	ParticleScriptCompiler::ParticleScriptCompiler()
		:mListener(0)
	{
		mAllowNontypedObjects = true;

		// Set up the base particle object types
		mObjectTypes.insert("emitter");
		mObjectTypes.insert("affector");

		// Set up the built-in property names
	}

	void ParticleScriptCompiler::setListener(ParticleScriptCompilerListener *listener)
	{
		mListener = listener;
		mObjectTypes.insert(listener->getObjectTypeNames().begin(), listener->getObjectTypeNames().end());
		mPropertyNames.insert(listener->getPropertyNames().begin(), listener->getPropertyNames().end());
	}

	bool ParticleScriptCompiler::compileImpl(ScriptNodeListPtr nodes)
	{
		return false;
	}

	ScriptNodeListPtr ParticleScriptCompiler::loadImportPath(const Ogre::String &name)
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
}
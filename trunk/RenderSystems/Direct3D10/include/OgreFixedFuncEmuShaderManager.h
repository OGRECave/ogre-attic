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
#ifndef __FixedFuncEmuShaderManager_H__
#define __FixedFuncEmuShaderManager_H__

#include "OgreTextureManager.h"
#include "OgreGpuProgramUsage.h"
#include "OgreFixedFuncState.h"

namespace Ogre 
{
	class FixedFuncEmuShaderGenerator;
	
	class FixedFuncPrograms
	{
	protected:
		// Vertex program details
		GpuProgramUsage * mVertexProgramUsage;
		// Fragment program details
		GpuProgramUsage * mFragmentProgramUsage;
	public:
		FixedFuncPrograms();
		~FixedFuncPrograms();
		Ogre::GpuProgramUsage * getVertexProgramUsage() const;
		void setVertexProgramUsage(Ogre::GpuProgramUsage * val);
		Ogre::GpuProgramUsage * getFragmentProgramUsage() const;
		void setFragmentProgramUsage(Ogre::GpuProgramUsage * val);

		void releaseMemory();
	};

	class FixedFuncEmuShaderManager
	{
	protected:
		typedef std::map<String, FixedFuncEmuShaderGenerator *> FixedFuncEmuShaderGeneratorMap;
		FixedFuncEmuShaderGeneratorMap mFixedFuncEmuShaderGeneratorMap;

		typedef std::map<VertexBufferDeclaration, FixedFuncPrograms> VertexBufferDeclaration2FixedFuncProgramsMap;
		typedef std::map<FixedFuncState, VertexBufferDeclaration2FixedFuncProgramsMap> State2Declaration2ProgramsMap;
		typedef std::map<String, State2Declaration2ProgramsMap> Language2State2Declaration2ProgramsMap;
		Language2State2Declaration2ProgramsMap mLanguage2State2Declaration2ProgramsMap;

		std::vector<FixedFuncPrograms> mProgramsToDeleteAtTheEnd;

		FixedFuncPrograms & _createShaderPrograms(const String & generatorName,
			const VertexBufferDeclaration & vertexBufferDeclaration, 
			FixedFuncState &  fixedFuncState);
	public:
		FixedFuncEmuShaderManager();
		virtual ~FixedFuncEmuShaderManager();

		void registerGenerator(FixedFuncEmuShaderGenerator * generator);
		void unregisterGenerator(FixedFuncEmuShaderGenerator * generator);

		FixedFuncPrograms & getShaderPrograms(const String & generatorName,
			const VertexBufferDeclaration & vertexBufferDeclaration, 
			FixedFuncState &  fixedFuncState);

	};
}
#endif

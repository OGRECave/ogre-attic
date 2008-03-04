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
#include "OgreRoot.h"
#include "OgreFixedFuncEmuShaderManager.h"
#include "OgreFixedFuncEmuShaderGenerator.h"
#include "OgreHighLevelGpuProgram.h"
#include "OgreHighLevelGpuProgramManager.h"

namespace Ogre 
{
	//---------------------------------------------------------------------
	FixedFuncEmuShaderManager::FixedFuncEmuShaderManager()
	{

	}
	//---------------------------------------------------------------------
	FixedFuncEmuShaderManager::~FixedFuncEmuShaderManager()
	{
		for (size_t i = 0 ; i < mProgramsToDeleteAtTheEnd.size() ; i++)
		{
			mProgramsToDeleteAtTheEnd[i].releaseMemory();
		}
	}
	//---------------------------------------------------------------------
	void FixedFuncEmuShaderManager::registerGenerator( FixedFuncEmuShaderGenerator * generator )
	{
		mFixedFuncEmuShaderGeneratorMap[generator->getName()] = generator;
	}
	//---------------------------------------------------------------------
	void FixedFuncEmuShaderManager::unregisterGenerator( FixedFuncEmuShaderGenerator * generator )
	{
		mFixedFuncEmuShaderGeneratorMap.erase(generator->getName());
	}
	//---------------------------------------------------------------------
	FixedFuncPrograms & FixedFuncEmuShaderManager::_createShaderPrograms( const String & generatorName, 
		const VertexBufferDeclaration & vertexBufferDeclaration, FixedFuncState & fixedFuncState )
	{

		const String vertexProgramName = "VS";
		const String fragmentProgramName = "FP";

		FixedFuncEmuShaderGenerator * fixedFuncEmuShaderGenerator = mFixedFuncEmuShaderGeneratorMap[generatorName];
		String shaderSource = fixedFuncEmuShaderGenerator->getShaderSource(
			vertexProgramName,
			fragmentProgramName,
			vertexBufferDeclaration,
			fixedFuncState
			);

		
		// Vertex program details
		GpuProgramUsage * vertexProgramUsage = new GpuProgramUsage(GPT_VERTEX_PROGRAM);
		// Fragment program details
		GpuProgramUsage * fragmentProgramUsage = new GpuProgramUsage(GPT_FRAGMENT_PROGRAM);


		HighLevelGpuProgramPtr vs;
		HighLevelGpuProgramPtr fs;

		class LoadFromSourceGpuProgram : public HighLevelGpuProgram
		{
		public:
			void doLoadFromSource(void)
			{
				loadFromSource();
			};
		};


		static size_t shaderCount = 0;
		shaderCount++;
		vs = HighLevelGpuProgramManager::getSingleton().
			createProgram("VS_" + StringConverter::toString(shaderCount), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			fixedFuncEmuShaderGenerator->getLanguageName(), GPT_VERTEX_PROGRAM);	
		vs->setSource(shaderSource);
		vs->setParameter("entry_point",vertexProgramName);
		vs->setParameter("target",fixedFuncEmuShaderGenerator->getVpTarget());
		static_cast<LoadFromSourceGpuProgram *>(vs.get())->doLoadFromSource();

		vertexProgramUsage->setProgram(Ogre::GpuProgramPtr(vs));
		vertexProgramUsage->setParameters(vs->createParameters());

		fs = HighLevelGpuProgramManager::getSingleton().
			createProgram("FS_" + StringConverter::toString(shaderCount), ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			fixedFuncEmuShaderGenerator->getLanguageName(), GPT_FRAGMENT_PROGRAM);	
		fs->setSource(shaderSource);
		fs->setParameter("entry_point",fragmentProgramName);
		fs->setParameter("target",fixedFuncEmuShaderGenerator->getFpTarget());
		static_cast<LoadFromSourceGpuProgram *>(fs.get())->doLoadFromSource();

		fragmentProgramUsage->setProgram(Ogre::GpuProgramPtr(fs));
		fragmentProgramUsage->setParameters(fs->createParameters());

		FixedFuncPrograms newPrograms;
		newPrograms.setVertexProgramUsage(vertexProgramUsage);
		newPrograms.setFragmentProgramUsage(fragmentProgramUsage);

		mProgramsToDeleteAtTheEnd.push_back(newPrograms);
		mLanguage2State2Declaration2ProgramsMap[generatorName][fixedFuncState][vertexBufferDeclaration] = newPrograms;	
		return mLanguage2State2Declaration2ProgramsMap[generatorName][fixedFuncState][vertexBufferDeclaration];
	}
	//---------------------------------------------------------------------
	FixedFuncPrograms & FixedFuncEmuShaderManager::getShaderPrograms( const String & generatorName, 
		const VertexBufferDeclaration & vertexBufferDeclaration, FixedFuncState & fixedFuncState )
	{
		Language2State2Declaration2ProgramsMap::iterator langIter = mLanguage2State2Declaration2ProgramsMap.find(generatorName);
		if (langIter != mLanguage2State2Declaration2ProgramsMap.end())
		{
			State2Declaration2ProgramsMap::iterator fixedFuncStateIter = langIter->second.find(fixedFuncState);
			if (fixedFuncStateIter != langIter->second.end())
			{
				VertexBufferDeclaration2FixedFuncProgramsMap::iterator vertexBufferDeclarationIter = fixedFuncStateIter->second.find(vertexBufferDeclaration);
				if (vertexBufferDeclarationIter != fixedFuncStateIter->second.end())
				{
					return vertexBufferDeclarationIter->second;
				}
			}
		}

		// we didn't find so we will create
		return _createShaderPrograms(generatorName, vertexBufferDeclaration, fixedFuncState);
	}
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	//---------------------------------------------------------------------
	Ogre::GpuProgramUsage * FixedFuncPrograms::getVertexProgramUsage() const 
	{
		return mVertexProgramUsage;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::setVertexProgramUsage( Ogre::GpuProgramUsage * val )
	{
		mVertexProgramUsage = val;
	}
	//---------------------------------------------------------------------
	Ogre::GpuProgramUsage * FixedFuncPrograms::getFragmentProgramUsage() const
	{
		return mFragmentProgramUsage;
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::setFragmentProgramUsage( Ogre::GpuProgramUsage * val )
	{
		mFragmentProgramUsage = val;
	}
	//---------------------------------------------------------------------
	FixedFuncPrograms::FixedFuncPrograms() 
		: mVertexProgramUsage(NULL), 
		mFragmentProgramUsage(NULL)
	{

	}
	//---------------------------------------------------------------------
	FixedFuncPrograms::~FixedFuncPrograms()
	{
		
	}
	//---------------------------------------------------------------------
	void FixedFuncPrograms::releaseMemory()
	{
		if (mVertexProgramUsage)
		{
			delete(mVertexProgramUsage);
		}
		if (mFragmentProgramUsage)
		{
			delete(mFragmentProgramUsage);
		}
	}
}

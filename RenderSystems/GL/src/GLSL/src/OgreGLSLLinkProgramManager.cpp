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

#include "OgreGLSLLinkProgramManager.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLProgram.h"
#include "OgreStringConverter.h"

namespace Ogre {

	//-----------------------------------------------------------------------
	template<> GLSLLinkProgramManager* Singleton<GLSLLinkProgramManager>::ms_Singleton = 0;

	//-----------------------------------------------------------------------
    GLSLLinkProgramManager* GLSLLinkProgramManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }

	//-----------------------------------------------------------------------
    GLSLLinkProgramManager& GLSLLinkProgramManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }

	//-----------------------------------------------------------------------
	GLSLLinkProgramManager::GLSLLinkProgramManager(void) : mActiveVertexGpuProgram(NULL),
		mActiveFragmentGpuProgram(NULL), mActiveLinkProgram(NULL)
	{
		// Fill in the relationship between type names and enums
		mTypeEnumMap.insert(StringToEnumMap::value_type("float", GL_FLOAT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("vec2", GL_FLOAT_VEC2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("vec3", GL_FLOAT_VEC3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("vec4", GL_FLOAT_VEC4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler1D", GL_SAMPLER_1D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler2D", GL_SAMPLER_2D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler3D", GL_SAMPLER_3D));
		mTypeEnumMap.insert(StringToEnumMap::value_type("samplerCube", GL_SAMPLER_CUBE));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler1DShadow", GL_SAMPLER_1D_SHADOW));
		mTypeEnumMap.insert(StringToEnumMap::value_type("sampler2DShadow", GL_SAMPLER_2D_SHADOW));
		mTypeEnumMap.insert(StringToEnumMap::value_type("int", GL_INT));
		mTypeEnumMap.insert(StringToEnumMap::value_type("ivec2", GL_INT_VEC2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("ivec3", GL_INT_VEC3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("ivec4", GL_INT_VEC4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat2", GL_FLOAT_MAT2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat3", GL_FLOAT_MAT3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat4", GL_FLOAT_MAT4));
		// GL 2.1
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat2x2", GL_FLOAT_MAT2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat3x3", GL_FLOAT_MAT3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat4x4", GL_FLOAT_MAT4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat2x3", GL_FLOAT_MAT2x3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat3x2", GL_FLOAT_MAT3x2));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat3x4", GL_FLOAT_MAT3x4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat4x3", GL_FLOAT_MAT4x3));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat2x4", GL_FLOAT_MAT2x4));
		mTypeEnumMap.insert(StringToEnumMap::value_type("mat4x2", GL_FLOAT_MAT4x2));

	}

	//-----------------------------------------------------------------------
	GLSLLinkProgramManager::~GLSLLinkProgramManager(void)
	{
		// iterate through map container and delete link programs
		for (LinkProgramIterator currentProgram = LinkPrograms.begin();
			currentProgram != LinkPrograms.end(); ++currentProgram)
		{
			delete currentProgram->second;
		}

	}

	//-----------------------------------------------------------------------
	GLSLLinkProgram* GLSLLinkProgramManager::getActiveLinkProgram(void)
	{
		// if there is an active link program then return it
		if (mActiveLinkProgram)
			return mActiveLinkProgram;

		// no active link program so find one or make a new one
		// is there an active key?
		GLuint activeKey = 0;

		if (mActiveVertexGpuProgram)
		{
			activeKey = mActiveVertexGpuProgram->getProgramID() << 8;
		}

		if (mActiveFragmentGpuProgram)
		{
			activeKey += mActiveFragmentGpuProgram->getProgramID();
		}

		// only return a link program object if a vertex or fragment program exist
		if (activeKey > 0)
		{
			// find the key in the hash map
			LinkProgramIterator programFound = LinkPrograms.find(activeKey);
			// program object not found for key so need to create it
			if (programFound == LinkPrograms.end())
			{
				mActiveLinkProgram = new GLSLLinkProgram();
				LinkPrograms[activeKey] = mActiveLinkProgram;
				// tell shaders to attach themselves to the LinkProgram
				// let the shaders do the attaching since they may have several children to attach
				if (mActiveVertexGpuProgram)
				{
					mActiveVertexGpuProgram->getGLSLProgram()->attachToProgramObject( mActiveLinkProgram->getGLHandle() );
                    mActiveLinkProgram->setSkeletalAnimationIncluded(mActiveVertexGpuProgram->isSkeletalAnimationIncluded());
				}

				if (mActiveFragmentGpuProgram)
				{
					mActiveFragmentGpuProgram->getGLSLProgram()->attachToProgramObject( mActiveLinkProgram->getGLHandle() );
				}


			}
			else
			{
				// found a link program in map container so make it active
				mActiveLinkProgram = programFound->second;
			}

		}
		// make the program object active
		if (mActiveLinkProgram) mActiveLinkProgram->activate();

		return mActiveLinkProgram;

	}

	//-----------------------------------------------------------------------
	void GLSLLinkProgramManager::setActiveFragmentShader(GLSLGpuProgram* fragmentGpuProgram)
	{
		if (fragmentGpuProgram != mActiveFragmentGpuProgram)
		{
			mActiveFragmentGpuProgram = fragmentGpuProgram;
			// ActiveLinkProgram is no longer valid
			mActiveLinkProgram = NULL;
			// change back to fixed pipeline
			glUseProgramObjectARB(0);
		}
	}

	//-----------------------------------------------------------------------
	void GLSLLinkProgramManager::setActiveVertexShader(GLSLGpuProgram* vertexGpuProgram)
	{
		if (vertexGpuProgram != mActiveVertexGpuProgram)
		{
			mActiveVertexGpuProgram = vertexGpuProgram;
			// ActiveLinkProgram is no longer valid
			mActiveLinkProgram = NULL;
			// change back to fixed pipeline
			glUseProgramObjectARB(0);
		}
	}
	//---------------------------------------------------------------------
	void GLSLLinkProgramManager::completeUniformInfo(UniformReference& newUniformReference)
	{
		// default to real since most uniforms are real
		newUniformReference.isReal = true;
		newUniformReference.isSampler = false;
		// decode uniform size and type
		switch (newUniformReference.mType)
		{
		case GL_FLOAT:
			newUniformReference.mElementCount = 1;
			break;

		case GL_FLOAT_VEC2:
			newUniformReference.mElementCount = 2;
			break;

		case GL_FLOAT_VEC3:
			newUniformReference.mElementCount = 3;
			break;

		case GL_FLOAT_VEC4:
			newUniformReference.mElementCount = 4;
			break;

		case GL_SAMPLER_1D:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_1D_SHADOW:
		case GL_SAMPLER_2D_SHADOW:
		case GL_SAMPLER_2D_RECT_ARB:
		case GL_SAMPLER_2D_RECT_SHADOW_ARB:
			newUniformReference.isSampler = true;
		case GL_INT:
			newUniformReference.isReal = false;
			newUniformReference.mElementCount = 1;
			break;

		case GL_INT_VEC2:
			newUniformReference.isReal = false;
			newUniformReference.mElementCount = 2;
			break;

		case GL_INT_VEC3:
			newUniformReference.isReal = false;
			newUniformReference.mElementCount = 3;
			break;

		case GL_INT_VEC4:
			newUniformReference.isReal = false;
			newUniformReference.mElementCount = 4;
			break;

		case GL_FLOAT_MAT2:
			newUniformReference.mElementCount = 4;
			break;

		case GL_FLOAT_MAT3:
			newUniformReference.mElementCount = 9;
			break;

		case GL_FLOAT_MAT4:
			newUniformReference.mElementCount = 16;
			break;

		case GL_FLOAT_MAT2x3:
		case GL_FLOAT_MAT3x2:
			newUniformReference.mElementCount = 6;
			break;

		case GL_FLOAT_MAT2x4:
		case GL_FLOAT_MAT4x2:
			newUniformReference.mElementCount = 8;
			break;

		case GL_FLOAT_MAT3x4:
		case GL_FLOAT_MAT4x3:
			newUniformReference.mElementCount = 12;
			break;

		default:
			// Ignore silently for unknown/unsupported types
			break;

		}

	}
	//---------------------------------------------------------------------
	void GLSLLinkProgramManager::extractUniforms(GLhandleARB programObject, UniformReferenceList& list)
	{
		// scan through the active uniforms and add them to the reference list
		GLint uniformCount;

		#define BUFFERSIZE 200
		char   uniformName[BUFFERSIZE];
		//GLint location;
		UniformReference newUniformReference;

		// get the number of active uniforms
		glGetObjectParameterivARB(programObject, GL_OBJECT_ACTIVE_UNIFORMS_ARB,
			&uniformCount);

		// Loop over each of the active uniforms, and add them to the reference container
		// only do this for user defined uniforms, ignore built in gl state uniforms
		for (int index = 0; index < uniformCount; index++)
		{
			glGetActiveUniformARB(programObject, index, BUFFERSIZE, NULL, &newUniformReference.mArraySize, &newUniformReference.mType, uniformName);
			// don't add built in uniforms
			newUniformReference.mLocation = glGetUniformLocationARB(programObject, uniformName);
			if (newUniformReference.mLocation >= 0)
			{
				// user defined uniform found, add it to the reference list
				newUniformReference.mName = String( uniformName );

				completeUniformInfo(newUniformReference);


				list.push_back(newUniformReference);

				if (!newUniformReference.isSampler)
				{
					String origName = newUniformReference.mName;
					// also add [0] version since GLSL always supports this, and
					// makes consistent with Cg & HLSL array access modes
					newUniformReference.mName = origName + "[0]";
					list.push_back(newUniformReference);

					// Add parmeters for other array accessors [1+]
					// However, direct access limited to arrays of size
					// 8 or less, since very large arrays would cause
					// excessive lookups in updateUniforms
					// Count is a bit arbitrary but sounds right, unlikely to
					// selectively update anything larger than that - larger
					// arrays will be populated in bulk

					if (newUniformReference.mArraySize <= 8)
					{
						size_t i, size;
						size = newUniformReference.mArraySize;
						newUniformReference.mArraySize = 1;
						for (i = 1; i < size; i++)
						{
							newUniformReference.mName = origName + "[" + StringConverter::toString(i) + "]";
							newUniformReference.mLocation = glGetUniformLocationARB(programObject, newUniformReference.mName.c_str());
							if (newUniformReference.mLocation >= 0)
							{
								list.push_back(newUniformReference);
							}
						}
					}
				} // !isSampler

			} // end if
		} // end for

	}
	//---------------------------------------------------------------------
	void GLSLLinkProgramManager::extractUniforms(const String& src, 
		UniformReferenceList& list)
	{
		// Could have done this as a compiler but we don't want to have to 
		// create a BNF for the whole GLSL syntax, so use a simpler method
		String::size_type currPos = 0;
		String line;
		currPos = src.find("uniform", currPos);
		while (currPos != String::npos)
		{
			UniformReference newUniform;

			// skip 'uniform'
			currPos += 7;
			// find terminating semicolon
			String::size_type endPos = src.find(";", currPos);
			if (endPos == String::npos)
			{
				// problem, missing semicolon, abort
				break;
			}
			line = src.substr(currPos, endPos - currPos);

			StringVector parts = StringUtil::split(line, " \t\r\n");

			for (StringVector::iterator i = parts.begin(); i != parts.end(); ++i)
			{
				// Is this a type?
				StringToEnumMap::iterator typei = mTypeEnumMap.find(*i);
				if (typei != mTypeEnumMap.end())
				{
					newUniform.mType = typei->second;
					completeUniformInfo(newUniform);
				}
				else
				{
					// if this is not a type, and not empty, it should be the name
					StringUtil::trim(*i);
					if (i->empty()) continue;

					String::size_type arrayStart = i->find("[", 0);
					if (arrayStart != String::npos)
					{
						// potential name (if butted up to array)
						String name = i->substr(0, arrayStart);
						StringUtil::trim(name);
						if (!name.empty())
							newUniform.mName = name;

						String::size_type arrayEnd = i->find("]", arrayStart);
						String arrayDimTerm = i->substr(arrayStart + 1, arrayEnd - arrayStart - 1);
						StringUtil::trim(arrayDimTerm);
						// the array term might be a simple number or it might be
						// an expression (e.g. 24*3) or refer to a constant expression
						// we'd have to evaluate the expression which could get nasty
						// TODO
						newUniform.mArraySize = StringConverter::parseInt(arrayDimTerm);

					}
					else
					{
						newUniform.mName = *i;
						newUniform.mArraySize = 1;
					}

				}

			}

			list.push_back(newUniform);

			if (!newUniform.isSampler)
			{
				String origName = newUniform.mName;
				// also add [0] version since GLSL always supports this, and
				// makes consistent with Cg & HLSL array access modes
				newUniform.mName = origName + "[0]";
				list.push_back(newUniform);

				// Add parmeters for other array accessors [1+]
				// However, direct access limited to arrays of size
				// 8 or less, since very large arrays would cause
				// excessive lookups in updateUniforms
				// Count is a bit arbitrary but sounds right, unlikely to
				// selectively update anything larger than that - larger
				// arrays will be populated in bulk

				if (newUniform.mArraySize <= 8)
				{
					size_t i, size;
					size = newUniform.mArraySize;
					newUniform.mArraySize = 1;
					for (i = 1; i < size; i++)
					{
						newUniform.mName = origName + "[" + StringConverter::toString(i) + "]";
						list.push_back(newUniform);
					}
				}
			}

			// Find next one
			currPos = src.find("uniform", currPos);

		}
	}

}

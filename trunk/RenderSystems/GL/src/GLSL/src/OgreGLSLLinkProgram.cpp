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

#include "OgreGLSLExtSupport.h"
#include "OgreGLSLLinkProgram.h"
#include "OgreStringConverter.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLProgram.h"
#include "OgreGLSLLinkProgramManager.h"

namespace Ogre {

	//-----------------------------------------------------------------------
#define NO_ATTRIB 0xFFFF
	GLSLLinkProgram::GLSLLinkProgram(GLSLGpuProgram* vertexProgram, GLSLGpuProgram* fragmentProgram)
        : mVertexProgram(vertexProgram)
		, mFragmentProgram(fragmentProgram)
		, mUniformRefsBuilt(false)
        , mLinked(false)
		, mTangentAttrib(NO_ATTRIB)
		, mBinormalAttrib(NO_ATTRIB)
		, mBlendIndicesAttrib(NO_ATTRIB)
		, mBlendWeightsAttrib(NO_ATTRIB)

	{
			checkForGLSLError( "GLSLLinkProgram::GLSLLinkProgram", "Error prior to Creating GLSL Program Object", 0 );
		    mGLHandle = glCreateProgramObjectARB();
			checkForGLSLError( "GLSLLinkProgram::GLSLLinkProgram", "Error Creating GLSL Program Object", 0 );


			// tell shaders to attach themselves to the LinkProgram
			// let the shaders do the attaching since they may have several children to attach
			if (mVertexProgram)
			{
				mVertexProgram->getGLSLProgram()->attachToProgramObject(mGLHandle);
				setSkeletalAnimationIncluded(mVertexProgram->isSkeletalAnimationIncluded());
			}

			if (mFragmentProgram)
			{
				mFragmentProgram->getGLSLProgram()->attachToProgramObject(mGLHandle);
			}

	}

	//-----------------------------------------------------------------------
	GLSLLinkProgram::~GLSLLinkProgram(void)
	{
		glDeleteObjectARB(mGLHandle);

	}

	//-----------------------------------------------------------------------
	void GLSLLinkProgram::activate(void)
	{
		if (!mLinked)
		{
			glLinkProgramARB( mGLHandle );
			glGetObjectParameterivARB( mGLHandle, GL_OBJECT_LINK_STATUS_ARB, &mLinked );
			// force logging and raise exception if not linked
			checkForGLSLError( "GLSLLinkProgram::Activate",
				"Error linking GLSL Program Object", mGLHandle, !mLinked, !mLinked );
			if(mLinked)
			{
				logObjectInfo( String("GLSL link result : "), mGLHandle );
				buildGLUniformReferences();
				extractAttributes();
			}

		}

		if (mLinked)
		{
		    glUseProgramObjectARB( mGLHandle );
		}
	}

	//-----------------------------------------------------------------------
	void GLSLLinkProgram::extractAttributes(void)
	{
		GLint attrib = glGetAttribLocationARB(mGLHandle, "tangent");
		mTangentAttrib = (attrib == -1)? NO_ATTRIB : (GLuint)attrib;
		
		attrib = glGetAttribLocationARB(mGLHandle, "binormal");
		mBinormalAttrib = (attrib == -1)? NO_ATTRIB : (GLuint)attrib;

		attrib = glGetAttribLocationARB(mGLHandle, "blendIndices");
		mBlendIndicesAttrib = (attrib == -1)? NO_ATTRIB : (GLuint)attrib;

		attrib = glGetAttribLocationARB(mGLHandle, "blendWeights");
		mBlendWeightsAttrib = (attrib == -1)? NO_ATTRIB : (GLuint)attrib;

	}
	//-----------------------------------------------------------------------
	GLuint GLSLLinkProgram::getAttributeIndex(VertexElementSemantic semantic)
	{
		switch(semantic)
		{
		case VES_TANGENT:
			return mTangentAttrib;
		case VES_BINORMAL:
			return mBinormalAttrib;
		case VES_BLEND_WEIGHTS:
			return mBlendWeightsAttrib;
		case VES_BLEND_INDICES:
			return mBlendIndicesAttrib;
		default:
			assert(false && "Shouldn't be calling this with standard attribs!");
			return 0;
		};
	}
	//-----------------------------------------------------------------------
	bool GLSLLinkProgram::isAttributeValid(VertexElementSemantic semantic)
	{
		switch(semantic)
		{
		case VES_TANGENT:
			return mTangentAttrib != NO_ATTRIB;
		case VES_BINORMAL:
			return mBinormalAttrib != NO_ATTRIB;
		case VES_BLEND_WEIGHTS:
			return mBlendWeightsAttrib != NO_ATTRIB;
		case VES_BLEND_INDICES:
			return mBlendIndicesAttrib != NO_ATTRIB;
		default:
			return false;
		};
	}
	//-----------------------------------------------------------------------
	void GLSLLinkProgram::buildGLUniformReferences(void)
	{
		if (!mUniformRefsBuilt)
		{
			const GpuConstantDefinitionMap* vertParams = 0;
			const GpuConstantDefinitionMap* fragParams = 0;

			if (mVertexProgram)
			{
				vertParams = &(mVertexProgram->getGLSLProgram()->getConstantDefinitions().map);
			}
			if (mFragmentProgram)
			{
				fragParams = &(mFragmentProgram->getGLSLProgram()->getConstantDefinitions().map);
			}

			GLSLLinkProgramManager::getSingleton().extractUniforms(
				mGLHandle, vertParams, fragParams, mGLUniformReferences);

			mUniformRefsBuilt = true;
		}
	}

	//-----------------------------------------------------------------------
	void GLSLLinkProgram::updateUniforms(GpuProgramParametersSharedPtr params, 
		GpuProgramType fromProgType)
	{
		// iterate through uniform reference list and update uniform values
		GLUniformReferenceIterator currentUniform = mGLUniformReferences.begin();
		GLUniformReferenceIterator endUniform = mGLUniformReferences.end();

		for (;currentUniform != endUniform; ++currentUniform)
		{
			// Only pull values from buffer it's supposed to be in (vertex or fragment)
			// This method will be called twice, once for vertex program params, 
			// and once for fragment program params.
			if (fromProgType == currentUniform->mSourceProgType)
			{
				const GpuConstantDefinition* def = currentUniform->mConstantDef;
				GLsizei glArraySize = (GLsizei)def->arraySize;

				// get the index in the parameter real list
				switch (def->constType)
				{
				case GCT_FLOAT1:
					glUniform1fvARB(currentUniform->mLocation, glArraySize, 
						params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_FLOAT2:
					glUniform2fvARB(currentUniform->mLocation, glArraySize, 
						params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_FLOAT3:
					glUniform3fvARB(currentUniform->mLocation, glArraySize, 
						params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_FLOAT4:
					glUniform4fvARB(currentUniform->mLocation, glArraySize, 
						params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_MATRIX_2X2:
					glUniformMatrix2fvARB(currentUniform->mLocation, glArraySize, 
						GL_TRUE, params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_MATRIX_2X3:
					if (GLEW_VERSION_2_1)
					{
						glUniformMatrix2x3fv(currentUniform->mLocation, glArraySize, 
							GL_TRUE, params->getFloatPointer(def->physicalIndex));
					}
					break;
				case GCT_MATRIX_2X4:
					if (GLEW_VERSION_2_1)
					{
						glUniformMatrix2x4fv(currentUniform->mLocation, glArraySize, 
							GL_TRUE, params->getFloatPointer(def->physicalIndex));
					}
					break;
					break;
				case GCT_MATRIX_3X2:
					if (GLEW_VERSION_2_1)
					{
						glUniformMatrix3x2fv(currentUniform->mLocation, glArraySize, 
							GL_TRUE, params->getFloatPointer(def->physicalIndex));
					}
					break;
				case GCT_MATRIX_3X3:
					glUniformMatrix3fvARB(currentUniform->mLocation, glArraySize, 
						GL_TRUE, params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_MATRIX_3X4:
					if (GLEW_VERSION_2_1)
					{
						glUniformMatrix3x4fv(currentUniform->mLocation, glArraySize, 
							GL_TRUE, params->getFloatPointer(def->physicalIndex));
					}
					break;
				case GCT_MATRIX_4X2:
					if (GLEW_VERSION_2_1)
					{
						glUniformMatrix4x2fv(currentUniform->mLocation, glArraySize, 
							GL_TRUE, params->getFloatPointer(def->physicalIndex));
					}
					break;
				case GCT_MATRIX_4X3:
					if (GLEW_VERSION_2_1)
					{
						glUniformMatrix4x3fv(currentUniform->mLocation, glArraySize, 
							GL_TRUE, params->getFloatPointer(def->physicalIndex));
					}
					break;
				case GCT_MATRIX_4X4:
					glUniformMatrix4fvARB(currentUniform->mLocation, glArraySize, 
						GL_TRUE, params->getFloatPointer(def->physicalIndex));
					break;
				case GCT_INT1:
					glUniform1ivARB(currentUniform->mLocation, glArraySize, 
						params->getIntPointer(def->physicalIndex));
					break;
				case GCT_INT2:
					glUniform2ivARB(currentUniform->mLocation, glArraySize, 
						params->getIntPointer(def->physicalIndex));
					break;
				case GCT_INT3:
					glUniform3ivARB(currentUniform->mLocation, glArraySize, 
						params->getIntPointer(def->physicalIndex));
					break;
				case GCT_INT4:
					glUniform4ivARB(currentUniform->mLocation, glArraySize, 
						params->getIntPointer(def->physicalIndex));
					break;
				case GCT_SAMPLER1D:
				case GCT_SAMPLER1DSHADOW:
				case GCT_SAMPLER2D:
				case GCT_SAMPLER2DSHADOW:
				case GCT_SAMPLER3D:
				case GCT_SAMPLERCUBE:
					// samplers handled like 1-element ints
					glUniform1ivARB(currentUniform->mLocation, 1, 
						params->getIntPointer(def->physicalIndex));
					break;

				} // end switch
			}
  
  		} // end for
	}


	//-----------------------------------------------------------------------
	void GLSLLinkProgram::updatePassIterationUniforms(GpuProgramParametersSharedPtr params)
	{
		if (params->hasPassIterationNumber())
		{
			size_t index = params->getPassIterationNumberIndex();

			GLUniformReferenceIterator currentUniform = mGLUniformReferences.begin();
			GLUniformReferenceIterator endUniform = mGLUniformReferences.end();

			// need to find the uniform that matches the multi pass entry
			for (;currentUniform != endUniform; ++currentUniform)
			{
				// get the index in the parameter real list
				if (index == currentUniform->mConstantDef->physicalIndex)
				{
					glUniform1fvARB( currentUniform->mLocation, 1, params->getFloatPointer(index));
					// there will only be one multipass entry
					return;
				}
			}
		}

    }
} // namespace Ogre

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
#include "OgreGLSLLinkProgramManager.h"

namespace Ogre {

	//-----------------------------------------------------------------------
#define NO_ATTRIB 0xFFFF
	GLSLLinkProgram::GLSLLinkProgram(void)
        : mUniformRefsBuilt(false)
        , mLinked(false)
		, mTangentAttrib(NO_ATTRIB)
		, mBinormalAttrib(NO_ATTRIB)
		, mBlendIndicesAttrib(NO_ATTRIB)
		, mBlendWeightsAttrib(NO_ATTRIB)

	{
			checkForGLSLError( "GLSLLinkProgram::GLSLLinkProgram", "Error prior to Creating GLSL Program Object", 0 );
		    mGLHandle = glCreateProgramObjectARB();
			checkForGLSLError( "GLSLLinkProgram::GLSLLinkProgram", "Error Creating GLSL Program Object", 0 );

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
				buildUniformReferences();
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
	void GLSLLinkProgram::buildUniformReferences(void)
	{
		if (!mUniformRefsBuilt)
		{
			GLSLLinkProgramManager::getSingleton().extractUniforms(mGLHandle, mUniformReferences);

			mUniformRefsBuilt = true;
		}
	}

	//-----------------------------------------------------------------------
	void GLSLLinkProgram::updateUniforms(GpuProgramParametersSharedPtr params)
	{
        // float array buffer used to pass arrays to GL
        static float floatBuffer[1024];
		size_t size;
        float* pBuffer;		
        GLint* piBuffer;

		// iterate through uniform reference list and update uniform values
		UniformReferenceIterator currentUniform = mUniformReferences.begin();
		UniformReferenceIterator endUniform = mUniformReferences.end();

		GpuProgramParameters::RealConstantEntry* currentRealConstant;
		GpuProgramParameters::IntConstantEntry* currentIntConstant;

		while (currentUniform != endUniform)
		{
			// get the index in the parameter real list

			if (currentUniform->isReal)
			{
				currentRealConstant = params->getNamedRealConstantEntry( currentUniform->mName );
				if (currentRealConstant != NULL)
				{
					if (currentRealConstant->isSet)
					{
                        pBuffer = floatBuffer;
  
						switch (currentUniform->mElementCount)
						{
						case 1:
                            // Support arrays of float
                            if (currentUniform->mArraySize > 1)
                            {
                                // Build a combined buffer
								size = 0;
                                while (currentRealConstant->isSet && size < currentUniform->mArraySize)
                                {
                                    *pBuffer++ = currentRealConstant->val[0];
									++currentRealConstant;
									++size;
                                }
                                glUniform1fvARB(currentUniform->mLocation, size, floatBuffer);
                            }
                            else
                            {
								glUniform1fvARB(currentUniform->mLocation, 1, currentRealConstant->val);
                            }
							break;

						case 2:
                            // Support arrays of vec2
                            if (currentUniform->mArraySize > 1)
                            {
                                // Build a combined buffer
								size = 0;
								while (currentRealConstant->isSet && size < currentUniform->mArraySize)
                                {
                                    memcpy(pBuffer, currentRealConstant->val, sizeof(float) * 2);
									++currentRealConstant;
									++size;
                                    pBuffer += 2;
                                }
                                glUniform2fvARB(currentUniform->mLocation, size, floatBuffer);
                            }
                            else
                            {
								glUniform2fvARB(currentUniform->mLocation, 1, currentRealConstant->val);
                            }
							break;

						case 3:
                            // Support arrays of vec3
                            if (currentUniform->mArraySize > 1)
                            {
                                // Build a combined buffer
								size = 0;
								while (currentRealConstant->isSet && size < currentUniform->mArraySize)
								{
									memcpy(pBuffer, currentRealConstant->val, sizeof(float) * 3);
									++currentRealConstant;
									++size;
									pBuffer += 3;
								}
                                glUniform3fvARB(currentUniform->mLocation, size, floatBuffer);
                            }
                            else
                            {
	                            glUniform3fvARB(currentUniform->mLocation, 1, currentRealConstant->val);
                            }
                            break;

                        case 4:
							// Support arrays of vec4 and mat2
							if (currentUniform->mArraySize > 1)
							{
								// Build a combined buffer
								size = 0;
								while (currentRealConstant->isSet && size < currentUniform->mArraySize)
								{
									memcpy(pBuffer, currentRealConstant->val, sizeof(float) * 4);
									++currentRealConstant;
									++size;
									pBuffer += 4;
								}
								// mat2 or vec4?
								if (currentUniform->mType == GL_FLOAT_MAT2)
									glUniformMatrix2fvARB(currentUniform->mLocation, size, GL_TRUE, floatBuffer);
								else
									glUniform4fvARB(currentUniform->mLocation, size, floatBuffer);
							}
							else
							{
								// mat2 or vec4?
								if (currentUniform->mType == GL_FLOAT_MAT2)
									glUniformMatrix2fvARB( currentUniform->mLocation, 1, GL_TRUE, currentRealConstant->val);
								else
									glUniform4fvARB(currentUniform->mLocation, 1, currentRealConstant->val);
							}
							break;

                        case 6:
							// 2x3 / 3x2 matrices only supported in GL 2.1
							if (GLEW_VERSION_2_1)
							{
								// Support arrays of mat2x3 and mat3x2
								size = 0;
								while (currentRealConstant->isSet && size < currentUniform->mArraySize)
								{
									memcpy(pBuffer, currentRealConstant++->val, sizeof(float) * 4);
									memcpy(pBuffer + 4, currentRealConstant++->val, sizeof(float) * 2);
									pBuffer += 6;
									++size;
								}
								if (currentUniform->mType == GL_FLOAT_MAT2x3)
									glUniformMatrix2x3fv(currentUniform->mLocation, size, GL_TRUE, floatBuffer);
								else
									glUniformMatrix3x2fv(currentUniform->mLocation, size, GL_TRUE, floatBuffer);
							}
                            break;
  
                        case 8:
							// 2x4 / 4x2 matrices only supported in GL 2.1
							if (GLEW_VERSION_2_1)
							{
								// Support arrays of mat2x4and mat4x2
								size = 0;
								while (currentRealConstant->isSet && size < currentUniform->mArraySize)
								{
									memcpy(pBuffer, currentRealConstant++->val, sizeof(float) * 4);
									memcpy(pBuffer + 4, currentRealConstant++->val, sizeof(float) * 4);
									pBuffer += 8;
									++size;
								}
								if (currentUniform->mType == GL_FLOAT_MAT2x4)
									glUniformMatrix2x4fv(currentUniform->mLocation, size, GL_TRUE, floatBuffer);
								else
									glUniformMatrix4x2fv(currentUniform->mLocation, size, GL_TRUE, floatBuffer);
							}
							break;

                        case 9:
							size = 0;
							while (currentRealConstant->isSet && size < currentUniform->mArraySize)
							{
                                memcpy(pBuffer, currentRealConstant++->val, sizeof(float) * 4);
                                memcpy(pBuffer + 4, currentRealConstant++->val, sizeof(float) * 4);
                                memcpy(pBuffer + 8, currentRealConstant++->val, sizeof(float));
                                pBuffer += 9;
								++size;
                            }
                            glUniformMatrix3fvARB(currentUniform->mLocation, size, GL_TRUE, floatBuffer);
                            break;

                        case 12:
							// 3x4 / 4x3 matrices only supported in GL 2.1
							if (GLEW_VERSION_2_1)
							{
								// Support arrays of mat3x4and mat4x3
								size = 0;
								while (currentRealConstant->isSet && size < currentUniform->mArraySize)
								{
									memcpy(pBuffer, currentRealConstant++->val, sizeof(float) * 4);
									memcpy(pBuffer + 4, currentRealConstant++->val, sizeof(float) * 4);
									memcpy(pBuffer + 8, currentRealConstant++->val, sizeof(float) * 4);
									pBuffer += 12;
									++size;
								}
								if (currentUniform->mType == GL_FLOAT_MAT3x4)
									glUniformMatrix3x4fv(currentUniform->mLocation, size, GL_TRUE, floatBuffer);
								else
									glUniformMatrix4x3fv(currentUniform->mLocation, size, GL_TRUE, floatBuffer);
							}
							break;

                        case 16:
                            // Support arrays of mat4
							size = 0;
							while (currentRealConstant->isSet && size < currentUniform->mArraySize)
							{
                                memcpy(pBuffer, currentRealConstant++->val, sizeof(float) * 4);
                                memcpy(pBuffer + 4, currentRealConstant++->val, sizeof(float) * 4);
                                memcpy(pBuffer + 8, currentRealConstant++->val, sizeof(float) * 4);
                                memcpy(pBuffer + 12, currentRealConstant++->val, sizeof(float) * 4);
                                pBuffer += 16;
								++size;
                            }
                            glUniformMatrix4fvARB(currentUniform->mLocation, size, GL_TRUE, floatBuffer);
                            break;

						} // end switch
					}
				}
			}
			else
			{
				currentIntConstant = params->getNamedIntConstantEntry( currentUniform->mName );
				if (currentIntConstant != NULL)
				{
					if (currentIntConstant->isSet)
					{
                        piBuffer = (GLint*)floatBuffer;
  
						switch (currentUniform->mElementCount)
						{
						case 1:
                            // Support arrays of int
                            if (currentUniform->mArraySize > 1)
                            {
                                // Build a combined buffer
								size = 0;
								while (currentRealConstant->isSet && size < currentUniform->mArraySize)
								{
                                    memcpy(piBuffer, currentRealConstant++->val, sizeof(GLint));
                                    piBuffer++;
									++size;
                                }
                                glUniform1ivARB(currentUniform->mLocation, size, (const GLint*)floatBuffer);
                            }
                            else
                            {
								glUniform1ivARB( currentUniform->mLocation, 1, (const GLint*)currentIntConstant->val );
                            }
							break;

						case 2:
                            // Support arrays of ivec2
                            if (currentUniform->mArraySize > 1)
                            {
                                // Build a combined buffer
								size = 0;
								while (currentRealConstant->isSet && size < currentUniform->mArraySize)
								{
                                    memcpy(pBuffer, currentRealConstant++->val, sizeof(GLint) * 2);
                                    pBuffer += 2;
									++size;
                                }
                                glUniform2ivARB(currentUniform->mLocation, size, (const GLint*)floatBuffer);
                            }
                            else
                            {
								glUniform2ivARB( currentUniform->mLocation, 1, (const GLint*)currentIntConstant->val );
                            }
							break;

						case 3:
                            // Support arrays of vec3
                            if (currentUniform->mArraySize > 1)
                            {
                                // Build a combined buffer
								size = 0;
								while (currentRealConstant->isSet && size < currentUniform->mArraySize)
								{
                                    memcpy(pBuffer, currentRealConstant++->val, sizeof(GLint) * 3);
                                    pBuffer += 3;
									++size;
                                }
                                glUniform3ivARB(currentUniform->mLocation, size, (const GLint*)floatBuffer);
                            }
                            else
                            {
								glUniform3ivARB( currentUniform->mLocation, 1, (const GLint*)currentIntConstant->val );
                            }
							break;

						case 4:
                            // Support arrays of vec4
                            if (currentUniform->mArraySize > 1)
                            {
                                // Build a combined buffer
								size = 0;
								while (currentRealConstant->isSet && size < currentUniform->mArraySize)
								{
                                    memcpy(pBuffer, currentRealConstant++->val, sizeof(GLint) * 4);
                                    pBuffer += 4;
									++size;
                                }
                                glUniform4ivARB(currentUniform->mLocation, size, (const GLint*)floatBuffer);
                            }
                            else
                            {
								glUniform4ivARB( currentUniform->mLocation, 1, (const GLint*)currentIntConstant->val );
                            }
							break;
  
						} // end switch
					}
				}

			}
  
  
			// get the next uniform
			++currentUniform;

		} // end while
	}


	//-----------------------------------------------------------------------
	void GLSLLinkProgram::updatePassIterationUniforms(GpuProgramParametersSharedPtr params)
	{
		// iterate through uniform reference list and update pass iteration uniform values
		UniformReferenceIterator currentUniform = mUniformReferences.begin();
		UniformReferenceIterator endUniform = mUniformReferences.end();

		GpuProgramParameters::RealConstantEntry* currentRealConstant;
		//GpuProgramParameters::IntConstantEntry* currentIntConstant;

        currentRealConstant = params->getPassIterationEntry();
        if (currentRealConstant)
        {
            // need to find the uniform that matches the multi pass entry
		    while (currentUniform != endUniform)
		    {
			    // get the index in the parameter real list

			    if (currentUniform->isReal)
			    {

				    if (currentRealConstant == params->getNamedRealConstantEntry( currentUniform->mName ))
				    {
                        glUniform1fvARB( currentUniform->mLocation, 1, currentRealConstant->val );
                        // there will only be one multipass entry
                        return;
                    }
                }
			    // get the next uniform
			    ++currentUniform;
            }
        }

    }
} // namespace Ogre

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

#include "OgreGLSLExtSupport.h"
#include "OgreGpuProgram.h"
#include "OgreGLSLLinkProgram.h"

namespace Ogre {

	//-----------------------------------------------------------------------

	GLSLLinkProgram::GLSLLinkProgram(void) : mLinked(false),
		mUniformRefsBuilt(false)
	{
			checkForGLSLError( "GLSLLinkProgram::GLSLLinkProgram", "Error prior to Creating GLSL Program Object", 0 );
		    mGLHandle = glCreateProgramObjectARB_ptr();
			checkForGLSLError( "GLSLLinkProgram::GLSLLinkProgram", "Error Creating GLSL Program Object", 0 );

	}

	//-----------------------------------------------------------------------
	GLSLLinkProgram::~GLSLLinkProgram(void)
	{
		glDeleteObjectARB_ptr(mGLHandle);

	}

	//-----------------------------------------------------------------------
	void GLSLLinkProgram::activate(void)
	{
		if(!mLinked)
		{
			glLinkProgramARB_ptr( mGLHandle );
			glGetObjectParameterivARB_ptr( mGLHandle, GL_OBJECT_LINK_STATUS_ARB, &mLinked );
			// force logging and raise exception if not linked
			checkForGLSLError( "GLSLLinkProgram::Activate",
				"Error linking GLSL Program Object", mGLHandle, !mLinked, !mLinked );
			if(mLinked)
			{
				logObjectInfo( String("GLSL link result : "), mGLHandle );
				buildUniformReferences();
			}

		}

		if(mLinked)
		{
		    glUseProgramObjectARB_ptr( mGLHandle );
		}
	}

	//-----------------------------------------------------------------------
	void GLSLLinkProgram::buildUniformReferences(void)
	{
		if (!mUniformRefsBuilt)
		{
			// scan through the active uniforms and add them to the reference list
			int    uniformCount;
			GLint  size;
			//GLenum type;
			#define BUFFERSIZE 100
			char   uniformName[BUFFERSIZE];
			//GLint location;
			UniformReference newUniformReference;

			// get the number of active uniforms
			glGetObjectParameterivARB_ptr(mGLHandle, GL_OBJECT_ACTIVE_UNIFORMS_ARB,
					&uniformCount);

			// Loop over each of the active uniforms, and add them to the reference container
			// only do this for user defined uniforms, ignore built in gl state uniforms
			for (int index = 0; index < uniformCount; index++)
			{
				glGetActiveUniformARB_ptr(mGLHandle, index, BUFFERSIZE, NULL, &size, &newUniformReference.mType, uniformName);
				// don't add built in uniforms 
				newUniformReference.mLocation = glGetUniformLocationARB_ptr(mGLHandle, uniformName);
				if (newUniformReference.mLocation >= 0)
				{
					// user defined uniform found, add it to the reference list
					newUniformReference.mName = String( uniformName );
					// decode uniform size and type
					switch(newUniformReference.mType)
					{
						case GL_FLOAT:
							newUniformReference.isReal = true;
							newUniformReference.mElementCount = 1;
							break;

						case GL_FLOAT_VEC2_ARB:
							newUniformReference.isReal = true;
							newUniformReference.mElementCount = 2;
							break;

						case GL_FLOAT_VEC3_ARB:
							newUniformReference.isReal = true;
							newUniformReference.mElementCount = 3;
							break;

						case GL_FLOAT_VEC4_ARB:
							newUniformReference.isReal = true;
							newUniformReference.mElementCount = 4;
							break;

						case GL_INT:
						case GL_SAMPLER_1D_ARB:
						case GL_SAMPLER_2D_ARB:
						case GL_SAMPLER_3D_ARB:
						case GL_SAMPLER_CUBE_ARB:
							newUniformReference.isReal = false;
							newUniformReference.mElementCount = 1;
							break;

						case GL_INT_VEC2_ARB:
							newUniformReference.isReal = false;
							newUniformReference.mElementCount = 2;
							break;

						case GL_INT_VEC3_ARB:
							newUniformReference.isReal = false;
							newUniformReference.mElementCount = 3;
							break;

						case GL_INT_VEC4_ARB:
							newUniformReference.isReal = false;
							newUniformReference.mElementCount = 4;
							break;
					}// end switch

					mUniformReferences.push_back(newUniformReference);

				} // end if
			} // end for

			mUniformRefsBuilt = true;
		}
	}

	//-----------------------------------------------------------------------
	void GLSLLinkProgram::updateUniforms(GpuProgramParametersSharedPtr params)
	{
		// iterate through uniform reference list and update uniform values
		UniformReferenceIterator currentUniform = mUniformReferences.begin();
		UniformReferenceIterator endUniform = mUniformReferences.end();

		GpuProgramParameters::RealConstantEntry* currentRealConstant;
		GpuProgramParameters::IntConstantEntry* currentIntConstant;

		while( currentUniform != endUniform)
		{
			// get the index in the parameter real list

			if(currentUniform->isReal)
			{
				currentRealConstant = params->getNamedRealConstantEntry( currentUniform->mName );
				if(currentRealConstant != NULL)
				{
					if(currentRealConstant->isSet) 
					{
						switch(currentUniform->mElementCount)
						{
							case 1:
								glUniform1fvARB_ptr( currentUniform->mLocation, 1, currentRealConstant->val );
								break;

							case 2:
								glUniform2fvARB_ptr( currentUniform->mLocation, 1, currentRealConstant->val );
								break;

							case 3:
								glUniform3fvARB_ptr( currentUniform->mLocation, 1, currentRealConstant->val );
								break;

							case 4:
								glUniform4fvARB_ptr( currentUniform->mLocation, 1, currentRealConstant->val );
								break;

						} // end switch
					}
				}
			}
			else
			{
				currentIntConstant = params->getNamedIntConstantEntry( currentUniform->mName );
				if(currentIntConstant != NULL)
				{
					if(currentIntConstant->isSet) 
					{
						switch(currentUniform->mElementCount)
						{
							case 1:
								glUniform1ivARB_ptr( currentUniform->mLocation, 1, currentIntConstant->val );
								break;

							case 2:
								glUniform2ivARB_ptr( currentUniform->mLocation, 1, currentIntConstant->val );
								break;

							case 3:
								glUniform3ivARB_ptr( currentUniform->mLocation, 1, currentIntConstant->val );
								break;

							case 4:
								glUniform4ivARB_ptr( currentUniform->mLocation, 1, currentIntConstant->val );
								break;
						} // end switch
					}
				}

			}


			// get the next uniform
			++currentUniform;

		} // end while
	}


} // namespace Ogre

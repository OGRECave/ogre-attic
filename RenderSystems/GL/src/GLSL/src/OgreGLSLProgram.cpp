/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreGpuProgram.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreRenderSystemCapabilities.h"
#include "OgreStringConverter.h"
#include "OgreGpuProgramManager.h"
#include "OgreHighLevelGpuProgramManager.h"

#include "OgreGLSLProgram.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLExtSupport.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    GLSLProgram::CmdAttach GLSLProgram::msCmdAttach;
    //-----------------------------------------------------------------------
    //---------------------------------------------------------------------------
    void GLSLProgram::loadFromSource(void)
    {
		const char* SLSource = mSource.c_str();
		glShaderSourceARB_ptr(mGLHandle, 1, &SLSource, NULL);
		// check for load errors
		checkForGLSLError( "GLSLProgram::loadFromSource", "Cannot load GLSL high-level shader source : " + mName, 0 );

		compile();
    }
    
    //---------------------------------------------------------------------------
	bool GLSLProgram::compile(const bool checkErrors)
	{

		glCompileShaderARB_ptr(mGLHandle);
		// check for compile errors
		glGetObjectParameterivARB_ptr(mGLHandle, GL_OBJECT_COMPILE_STATUS_ARB, &mCompiled);
		// force exception if not compiled
		if (checkErrors)
		{
			checkForGLSLError( "GLSLProgram::loadFromSource", "Cannot compile GLSL high-level shader : " + mName + " ", mGLHandle, !mCompiled, !mCompiled );
			
			if (mCompiled)
			{
				logObjectInfo( mName + " : GLSL compiled ", mGLHandle );
			}
		}
		return (mCompiled == 1);

	}

	//-----------------------------------------------------------------------
	void GLSLProgram::createLowLevelImpl(void)
	{
		mAssemblerProgram = new GLSLGpuProgram( this );
	}

	//-----------------------------------------------------------------------
    void GLSLProgram::unloadImpl(void)
    {
		if (isSupported())
		{
			glDeleteObjectARB_ptr(mGLHandle);
		}

		// should we do this here?
		delete mAssemblerProgram;
		mAssemblerProgram = NULL;

    }

	//-----------------------------------------------------------------------
    void GLSLProgram::populateParameterNames(GpuProgramParametersSharedPtr params)
    {
		// can't populate parameter names in GLSL until link time
		// allow for names read from a material script to be added automatically to the list
		params->setAutoAddParamName(true);

    }

	//-----------------------------------------------------------------------
    GLSLProgram::GLSLProgram(ResourceManager* creator, 
        const String& name, ResourceHandle handle,
        const String& group, bool isManual, ManualResourceLoader* loader)
        : HighLevelGpuProgram(createor, name, handle, group, isManual, loader)
    {
		// add parameter command "attach" to the material serializer dictionary
        if (createParamDictionary("GLSLProgram"))
        {
            setupBaseParamDictionary();
            ParamDictionary* dict = getParamDictionary();

            dict->addParameter(ParameterDef("attach", 
                "name of another GLSL program needed by this program",
                PT_STRING),&msCmdAttach);
        }
        // Manually assign language now since we use it immediately
        mSyntaxCode = "glsl";

		// want scenemanager to pass on surface and light states to the rendersystem
		mPassSurfaceAndLightStates = true;
		// only create a shader object if glsl is supported
		if (isSupported())
		{
			checkForGLSLError( "GLSLProgram::GLSLProgram", "GL Errors before creating shader object", 0 );
			// create shader object
			mGLHandle = glCreateShaderObjectARB_ptr(
				(gpType == GPT_VERTEX_PROGRAM) ? GL_VERTEX_SHADER_ARB : GL_FRAGMENT_SHADER_ARB );

			checkForGLSLError( "GLSLProgram::GLSLProgram", "Error creating GLSL shader Object", 0 );
		}

        
    }

	//-----------------------------------------------------------------------
    String GLSLProgram::CmdAttach::doGet(const void *target) const
    {
        return (static_cast<const GLSLProgram*>(target))->getAttachedShaderNames();
    }
	//-----------------------------------------------------------------------
    void GLSLProgram::CmdAttach::doSet(void *target, const String& shaderNames)
    {
		//get all the shader program names: there could be more than one
		StringVector vecShaderNames = StringUtil::split(shaderNames, " \t", 0);

		size_t programNameCount = vecShaderNames.size();
		for ( size_t i = 0; i < programNameCount; ++i )
		{
	        static_cast<GLSLProgram*>(target)->attachChildShader(vecShaderNames[i]);
		}
    }

	//-----------------------------------------------------------------------
    void GLSLProgram::attachChildShader(const String& name)
	{
		// is the name valid and already loaded?
		// check with the high level program manager to see if it was loaded
		HighLevelGpuProgram* hlProgram = static_cast<HighLevelGpuProgram*> (HighLevelGpuProgramManager::getSingleton().getByName(name));
		if (hlProgram)
		{
			if (hlProgram->getSyntaxCode() == "glsl")
			{
				// make sure attached program source gets loaded and compiled
				// don't need a low level implementation for attached shader objects
				// loadHighLevelImpl will only load the source and compile once
				// so don't worry about calling it several times
				GLSLProgram* childShader = static_cast<GLSLProgram*>(hlProgram);
				// load the source and attach the child shader only if supported
				if (isSupported())
				{
					childShader->loadHighLevelImpl();
					// add to the container
					mAttachedGLSLPrograms.push_back( childShader );
					mAttachedShaderNames += name + " ";
				}
			}
		}
	}

	//-----------------------------------------------------------------------
	void GLSLProgram::attachToProgramObject( const GLhandleARB programObject )
	{
		glAttachObjectARB_ptr( programObject, mGLHandle );
		checkForGLSLError( "GLSLLinkProgram::GLSLLinkProgram",
			"Error attaching " + mName + " shader object to GLSL Program Object", programObject );
		// attach child objects
		GLSLProgramContainerIterator childprogramcurrent = mAttachedGLSLPrograms.begin();
		GLSLProgramContainerIterator childprogramend = mAttachedGLSLPrograms.end();

 		while (childprogramcurrent != childprogramend)
		{

			GLSLProgram* childShader = *childprogramcurrent;
			// bug in ATI GLSL linker : modules without main function must be recompiled each time 
			// they are linked to a different program object
			// don't check for compile errors since there won't be any
			// *** minor inconvenience until ATI fixes there driver
			childShader->compile(false);

			childShader->attachToProgramObject( programObject );

			childprogramcurrent++;
		}

	}

  
}

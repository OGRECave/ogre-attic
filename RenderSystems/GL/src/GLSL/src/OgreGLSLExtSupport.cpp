/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright (c) 2000-2005 The OGRE Team
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

#include "OgreLogManager.h"
#include "OgreGLSLExtSupport.h"
#include "OgreGLSupport.h"

namespace Ogre
{
	//
	// OpenGL Shading Language entry points
	//
	PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB_ptr = NULL;
	PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB_ptr = NULL;
	PFNGLDELETEOBJECTARBPROC glDeleteObjectARB_ptr = NULL;
	PFNGLDETACHOBJECTARBPROC glDetachObjectARB_ptr = NULL;
	PFNGLATTACHOBJECTARBPROC glAttachObjectARB_ptr = NULL;

	PFNGLSHADERSOURCEARBPROC glShaderSourceARB_ptr = NULL;
	PFNGLCOMPILESHADERARBPROC glCompileShaderARB_ptr = NULL;
	PFNGLLINKPROGRAMARBPROC glLinkProgramARB_ptr = NULL;
	PFNGLGETINFOLOGARBPROC glGetInfoLogARB_ptr = NULL;
	PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB_ptr = NULL;

	PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB_ptr = NULL;
	PFNGLGETOBJECTPARAMETERFVARBPROC glGetObjectParameterfvARB_ptr = NULL;
	PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB_ptr = NULL;
	PFNGLGETATTRIBLOCATIONARBPROC glGetAttribLocationARB_ptr = NULL;
	PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniformARB_ptr = NULL;

	PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB_ptr = NULL;

	PFNGLUNIFORM1FARBPROC glUniform1fARB_ptr = NULL;
	PFNGLUNIFORM2FARBPROC glUniform2fARB_ptr = NULL;
	PFNGLUNIFORM3FARBPROC glUniform3fARB_ptr = NULL;
	PFNGLUNIFORM4FARBPROC glUniform4fARB_ptr = NULL;

	PFNGLUNIFORM1IARBPROC glUniform1iARB_ptr = NULL;
	PFNGLUNIFORM2IARBPROC glUniform2iARB_ptr = NULL;
	PFNGLUNIFORM3IARBPROC glUniform3iARB_ptr = NULL;
	PFNGLUNIFORM4IARBPROC glUniform4iARB_ptr = NULL;

	PFNGLUNIFORM1FVARBPROC glUniform1fvARB_ptr = NULL;
	PFNGLUNIFORM2FVARBPROC glUniform2fvARB_ptr = NULL;
	PFNGLUNIFORM3FVARBPROC glUniform3fvARB_ptr = NULL;
	PFNGLUNIFORM4FVARBPROC glUniform4fvARB_ptr = NULL;

	PFNGLUNIFORM1IVARBPROC glUniform1ivARB_ptr = NULL;
	PFNGLUNIFORM2IVARBPROC glUniform2ivARB_ptr = NULL;
	PFNGLUNIFORM3IVARBPROC glUniform3ivARB_ptr = NULL;
	PFNGLUNIFORM4IVARBPROC glUniform4ivARB_ptr = NULL;

	PFNGLVERTEXATTRIB4FVARBPROC glVertexAttrib4fvARB_ptr = NULL;
	PFNGLVERTEXATTRIB3FVARBPROC glVertexAttrib3fvARB_ptr = NULL;
	PFNGLVERTEXATTRIB2FVARBPROC glVertexAttrib2fvARB_ptr = NULL;
	PFNGLVERTEXATTRIB1FVARBPROC glVertexAttrib1fvARB_ptr = NULL;
	PFNGLVERTEXATTRIB4FARBPROC glVertexAttrib4fARB_ptr = NULL;
	PFNGLVERTEXATTRIB3FARBPROC glVertexAttrib3fARB_ptr = NULL;
	PFNGLVERTEXATTRIB2FARBPROC glVertexAttrib2fARB_ptr = NULL;
	PFNGLVERTEXATTRIB1FARBPROC glVertexAttrib1fARB_ptr = NULL;


    //-----------------------------------------------------------------------------
	bool InitGLShaderLanguageExtensions( GLSupport& glSupport )
	{
		static bool init = false;

		// check that this has not been done before -- only need to get proc addresses once
		if (!init)
		{

			glCreateProgramObjectARB_ptr  = (PFNGLCREATEPROGRAMOBJECTARBPROC) glSupport.getProcAddress( "glCreateProgramObjectARB" );
			glCreateShaderObjectARB_ptr   = (PFNGLCREATESHADEROBJECTARBPROC) glSupport.getProcAddress( "glCreateShaderObjectARB" );
			glDeleteObjectARB_ptr		  = (PFNGLDELETEOBJECTARBPROC) glSupport.getProcAddress( "glDeleteObjectARB" );
			glDetachObjectARB_ptr		  = (PFNGLDETACHOBJECTARBPROC) glSupport.getProcAddress( "glDetachObjectARB" );
			glAttachObjectARB_ptr		  = (PFNGLATTACHOBJECTARBPROC) glSupport.getProcAddress( "glAttachObjectARB" );
			glShaderSourceARB_ptr		  = (PFNGLSHADERSOURCEARBPROC) glSupport.getProcAddress( "glShaderSourceARB" );
			glCompileShaderARB_ptr		  = (PFNGLCOMPILESHADERARBPROC) glSupport.getProcAddress( "glCompileShaderARB" );
			glLinkProgramARB_ptr		  = (PFNGLLINKPROGRAMARBPROC) glSupport.getProcAddress( "glLinkProgramARB" );
			glGetInfoLogARB_ptr			  = (PFNGLGETINFOLOGARBPROC) glSupport.getProcAddress( "glGetInfoLogARB" );
			glUseProgramObjectARB_ptr	  = (PFNGLUSEPROGRAMOBJECTARBPROC) glSupport.getProcAddress( "glUseProgramObjectARB" );
			glGetObjectParameterivARB_ptr = (PFNGLGETOBJECTPARAMETERIVARBPROC) glSupport.getProcAddress( "glGetObjectParameterivARB" );
			glGetObjectParameterfvARB_ptr = (PFNGLGETOBJECTPARAMETERFVARBPROC) glSupport.getProcAddress( "glGetObjectParameterfvARB" );
			glGetUniformLocationARB_ptr	  = (PFNGLGETUNIFORMLOCATIONARBPROC) glSupport.getProcAddress( "glGetUniformLocationARB" );
			glGetActiveUniformARB_ptr	  = (PFNGLGETACTIVEUNIFORMARBPROC) glSupport.getProcAddress( "glGetActiveUniformARB" );
			glGetAttribLocationARB_ptr	  = (PFNGLGETATTRIBLOCATIONARBPROC) glSupport.getProcAddress( "glGetAttribLocationARB" );
			glBindAttribLocationARB_ptr	  = (PFNGLBINDATTRIBLOCATIONARBPROC) glSupport.getProcAddress( "glBindAttribLocationARB" );

			glUniform1fARB_ptr			  = (PFNGLUNIFORM1FARBPROC) glSupport.getProcAddress( "glUniform1fARB" );
			glUniform2fARB_ptr			  = (PFNGLUNIFORM2FARBPROC) glSupport.getProcAddress( "glUniform2fARB" );
			glUniform3fARB_ptr			  = (PFNGLUNIFORM3FARBPROC) glSupport.getProcAddress( "glUniform3fARB" );
			glUniform4fARB_ptr			  = (PFNGLUNIFORM4FARBPROC) glSupport.getProcAddress( "glUniform4fARB" );
			glUniform1iARB_ptr			  = (PFNGLUNIFORM1IARBPROC) glSupport.getProcAddress( "glUniform1iARB" );
			glUniform2iARB_ptr			  = (PFNGLUNIFORM2IARBPROC) glSupport.getProcAddress( "glUniform2iARB" );
			glUniform3iARB_ptr			  = (PFNGLUNIFORM3IARBPROC) glSupport.getProcAddress( "glUniform3iARB" );
			glUniform4iARB_ptr			  = (PFNGLUNIFORM4IARBPROC) glSupport.getProcAddress( "glUniform4iARB" );
			glUniform1fvARB_ptr			  = (PFNGLUNIFORM1FVARBPROC) glSupport.getProcAddress( "glUniform1fvARB" );
			glUniform2fvARB_ptr			  = (PFNGLUNIFORM2FVARBPROC) glSupport.getProcAddress( "glUniform2fvARB" );
			glUniform3fvARB_ptr			  = (PFNGLUNIFORM3FVARBPROC) glSupport.getProcAddress( "glUniform3fvARB" );
			glUniform4fvARB_ptr			  = (PFNGLUNIFORM4FVARBPROC) glSupport.getProcAddress( "glUniform4fvARB" );
			glUniform1ivARB_ptr			  = (PFNGLUNIFORM1IVARBPROC) glSupport.getProcAddress( "glUniform1ivARB" );
			glUniform2ivARB_ptr			  = (PFNGLUNIFORM2IVARBPROC) glSupport.getProcAddress( "glUniform2ivARB" );
			glUniform3ivARB_ptr			  = (PFNGLUNIFORM3IVARBPROC) glSupport.getProcAddress( "glUniform3ivARB" );
			glUniform4ivARB_ptr			  = (PFNGLUNIFORM4IVARBPROC) glSupport.getProcAddress( "glUniform4ivARB" );

			glVertexAttrib4fvARB_ptr	  = (PFNGLVERTEXATTRIB4FVARBPROC) glSupport.getProcAddress( "glVertexAttrib4fvARB" );
			glVertexAttrib3fvARB_ptr	  = (PFNGLVERTEXATTRIB3FVARBPROC) glSupport.getProcAddress( "glVertexAttrib3fvARB" );
			glVertexAttrib2fvARB_ptr	  = (PFNGLVERTEXATTRIB2FVARBPROC) glSupport.getProcAddress( "glVertexAttrib2fvARB" );
			glVertexAttrib1fvARB_ptr	  = (PFNGLVERTEXATTRIB1FVARBPROC) glSupport.getProcAddress( "glVertexAttrib1fvARB" );
			glVertexAttrib4fARB_ptr		  = (PFNGLVERTEXATTRIB4FARBPROC) glSupport.getProcAddress( "glVertexAttrib4fARB" );
			glVertexAttrib3fARB_ptr		  = (PFNGLVERTEXATTRIB3FARBPROC) glSupport.getProcAddress( "glVertexAttrib3fARB" );
			glVertexAttrib2fARB_ptr		  = (PFNGLVERTEXATTRIB2FARBPROC) glSupport.getProcAddress( "glVertexAttrib2fARB" );
			glVertexAttrib1fARB_ptr		  = (PFNGLVERTEXATTRIB1FARBPROC) glSupport.getProcAddress( "glVertexAttrib1fARB" );

			init = true;
		}

		return init;

	}

    //-----------------------------------------------------------------------------
    void checkForGLSLError(const String& ogreMethod, const String& errorTextPrefix, const GLhandleARB obj, const bool forceInfoLog, const bool forceException)
    {
		GLenum glErr;
		bool errorsFound = false;
		String msg = errorTextPrefix;

		// get all the GL errors
		glErr = glGetError();
		while (glErr != GL_NO_ERROR)
        {
			msg += "\n" + String((char*)gluErrorString(glErr)); 
			glErr = glGetError();
			errorsFound = true;
        }


		// if errors were found then put them in the Log and raise and exception
		if (errorsFound || forceInfoLog)
		{
			// if shader or program object then get the log message and send to the log manager
			msg += logObjectInfo( msg, obj );

            if (forceException) 
			{
				OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, msg, ogreMethod);
			}
		}
    }

    //-----------------------------------------------------------------------------
	String logObjectInfo(const String& msg, const GLhandleARB obj)
	{
		String logMessage = msg;

		if (obj > 0)
		{
			GLint infologLength = 0;

			glGetObjectParameterivARB_ptr(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength);

			if (infologLength > 0)
			{
				GLint charsWritten  = 0;

				GLcharARB * infoLog = new GLcharARB[infologLength];

				glGetInfoLogARB_ptr(obj, infologLength, &charsWritten, infoLog);
				logMessage += String(infoLog) + "\n";
				LogManager::getSingleton().logMessage(logMessage);

				delete [] infoLog;
			}
		}

		return logMessage;

	}


} // namespace Ogre

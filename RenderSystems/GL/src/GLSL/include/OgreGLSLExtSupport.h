/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2004 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/


#ifndef __GLSLExtSupport_H__
#define __GLSLExtSupport_H__

#include "OgreGLPrerequisites.h"

//
// OpenGL Shading Language entry points
//
namespace Ogre
{

	extern PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB_ptr;
	extern PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB_ptr;
	extern PFNGLDELETEOBJECTARBPROC glDeleteObjectARB_ptr;
	extern PFNGLDETACHOBJECTARBPROC glDetachObjectARB_ptr;
	extern PFNGLATTACHOBJECTARBPROC glAttachObjectARB_ptr;

	extern PFNGLSHADERSOURCEARBPROC glShaderSourceARB_ptr;
	extern PFNGLCOMPILESHADERARBPROC glCompileShaderARB_ptr;
	extern PFNGLLINKPROGRAMARBPROC glLinkProgramARB_ptr;
	extern PFNGLGETINFOLOGARBPROC glGetInfoLogARB_ptr;
	extern PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB_ptr;

	extern PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB_ptr;
	extern PFNGLGETOBJECTPARAMETERFVARBPROC glGetObjectParameterfvARB_ptr;
	extern PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB_ptr;
	extern PFNGLGETATTRIBLOCATIONARBPROC glGetAttribLocationARB_ptr;
	extern PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniformARB_ptr;

	extern PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB_ptr;

	extern PFNGLUNIFORM1FARBPROC glUniform1fARB_ptr;
	extern PFNGLUNIFORM2FARBPROC glUniform2fARB_ptr;
	extern PFNGLUNIFORM3FARBPROC glUniform3fARB_ptr;
	extern PFNGLUNIFORM4FARBPROC glUniform4fARB_ptr;

	extern PFNGLUNIFORM1IARBPROC glUniform1iARB_ptr;
	extern PFNGLUNIFORM2IARBPROC glUniform2iARB_ptr;
	extern PFNGLUNIFORM3IARBPROC glUniform3iARB_ptr;
	extern PFNGLUNIFORM4IARBPROC glUniform4iARB_ptr;

	extern PFNGLUNIFORM1FVARBPROC glUniform1fvARB_ptr;
	extern PFNGLUNIFORM2FVARBPROC glUniform2fvARB_ptr;
	extern PFNGLUNIFORM3FVARBPROC glUniform3fvARB_ptr;
	extern PFNGLUNIFORM4FVARBPROC glUniform4fvARB_ptr;

	extern PFNGLUNIFORM1IVARBPROC glUniform1ivARB_ptr;
	extern PFNGLUNIFORM2IVARBPROC glUniform2ivARB_ptr;
	extern PFNGLUNIFORM3IVARBPROC glUniform3ivARB_ptr;
	extern PFNGLUNIFORM4IVARBPROC glUniform4ivARB_ptr;

	extern PFNGLVERTEXATTRIB4FVARBPROC glVertexAttrib4fvARB_ptr;
	extern PFNGLVERTEXATTRIB3FVARBPROC glVertexAttrib3fvARB_ptr;
	extern PFNGLVERTEXATTRIB2FVARBPROC glVertexAttrib2fvARB_ptr;
	extern PFNGLVERTEXATTRIB1FVARBPROC glVertexAttrib1fvARB_ptr;
	extern PFNGLVERTEXATTRIB4FARBPROC glVertexAttrib4fARB_ptr;
	extern PFNGLVERTEXATTRIB3FARBPROC glVertexAttrib3fARB_ptr;
	extern PFNGLVERTEXATTRIB2FARBPROC glVertexAttrib2fARB_ptr;
	extern PFNGLVERTEXATTRIB1FARBPROC glVertexAttrib1fARB_ptr;

	// forward declarations
	class GLSLProgram;
	class GLSLGpuProgram;
	class GLSLLinkProgram;
	class GLSLLinkProgramManager;
	class GLSLProgramFactory;


	bool InitGLShaderLanguageExtensions( GLSupport& glSupport );
	/** Check for GL errors and report them in the Ogre Log.

	@param forceInfoLog if true then message from GL info log is obtained
	@param forceException if true then exception is generated if a GL error found
	*/
    void checkForGLSLError(const String& ogreMethod, const String& errorTextPrefix, GLhandleARB obj, bool forceInfoLog = false, bool forceException = false);

	/** if there is a message in GL info log then post it in the Ogre Log
	@param msg the info log message string is appended to this string
	@param obj the GL object that is used to retrieve the info log
	*/
	String logObjectInfo(const String& msg, GLhandleARB obj);


} // namespace Ogre

#endif // __GLSLExtSupport_H__

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
#ifndef __GLPrerequisites_H__
#define __GLPrerequisites_H__

#include "OgrePrerequisites.h"

#if OGRE_PLATFORM == PLATFORM_WIN32
#   define NOMINMAX // required to stop windows.h messing up std::min
#   include <windows.h>
#   include <wingdi.h>
#   include <GL/gl.h>
#   define GL_GLEXT_PROTOTYPES
#   include "glprocs.h"
#   include <GL/glu.h>
// Windows library does not include glSecondaryColorPointer even though it's standard now
#   define glSecondaryColorPointer glSecondaryColorPointerEXT
#elif OGRE_PLATFORM == PLATFORM_LINUX
// define GL_GLEXT_LEGACY so that Mesa headers won't try to include their own
// glext.h file.
#   define GL_GLEXT_LEGACY
#   include <GL/gl.h>
#   include <GL/glu.h>
#   define GL_GLEXT_PROTOTYPES
#elif OGRE_PLATFORM == PLATFORM_APPLE
#   define GL_GLEXT_PROTOTYPES
#   ifndef APIENTRY
#       define APIENTRY
#   endif
#   include <OpenGL/gl.h>
#   include <OpenGL/glu.h>
#endif
#include "GL/glext.h"

extern "C" {
// Pointer to glActiveTextureARB function
typedef void (APIENTRY *GL_ActiveTextureARB_Func)(GLenum);
extern GL_ActiveTextureARB_Func glActiveTextureARB_ptr;

// Pointer to glClientActiveTextureARB function
typedef void (APIENTRY *GL_ClientActiveTextureARB_Func)(GLenum);
extern GL_ClientActiveTextureARB_Func glClientActiveTextureARB_ptr;

// Pointer to glSecondaryColorPointerEXT function
typedef void (APIENTRY *GL_SecondaryColorPointerEXT_Func)(GLint, GLenum, GLsizei, const GLvoid*);
extern GL_SecondaryColorPointerEXT_Func glSecondaryColorPointerEXT_ptr;

// Pointer to glGenBuffersARB function
typedef void (APIENTRY *GL_GenBuffersARB_Func)(GLsizei, GLuint*);
extern GL_GenBuffersARB_Func glGenBuffersARB_ptr;

// Pointer to glBindBufferARB function
typedef void (APIENTRY *GL_BindBufferARB_Func)(GLenum, GLuint);
extern GL_BindBufferARB_Func glBindBufferARB_ptr;

// Pointer to glDeleteBuffersARB function
typedef void (APIENTRY *GL_DeleteBuffersARB_Func)(GLsizei, const GLuint*);
extern GL_DeleteBuffersARB_Func glDeleteBuffersARB_ptr;

// Pointer to glMapBufferARB function
typedef GLvoid* (APIENTRY *GL_MapBufferARB_Func)(GLenum, GLenum);
extern GL_MapBufferARB_Func glMapBufferARB_ptr;

// Pointer to glUnmapBufferARB function
typedef GLboolean (APIENTRY *GL_UnmapBufferARB_Func)(GLenum);
extern GL_UnmapBufferARB_Func glUnmapBufferARB_ptr;

// Pointer to glBufferSubDataARB function
typedef void (APIENTRY *GL_BufferSubDataARB_Func)(GLenum, GLintptrARB, GLsizeiptrARB, const GLvoid*);
extern GL_BufferSubDataARB_Func glBufferSubDataARB_ptr;

// Pointer to glBufferDataARB function
typedef void (APIENTRY *GL_BufferDataARB_Func)(GLenum, GLsizeiptrARB, const GLvoid* , GLenum);
extern GL_BufferDataARB_Func glBufferDataARB_ptr;

// Pointer to glGetBufferSubDataARB function
typedef void (APIENTRY *GL_GetBufferSubDataARB_Func)(GLenum, GLintptrARB, GLsizeiptrARB, GLvoid*);
extern GL_GetBufferSubDataARB_Func glGetBufferSubDataARB_ptr;

// Pointer to glGenProgramsARB function
typedef void (APIENTRY *GL_GenProgramsARB_Func)(GLsizei, GLuint*);
extern GL_GenProgramsARB_Func glGenProgramsARB_ptr;

// Pointer to glDeleteProgramsARB function
typedef void (APIENTRY *GL_DeleteProgramsARB_Func)(GLsizei, const GLuint*);
extern GL_DeleteProgramsARB_Func glDeleteProgramsARB_ptr;

// Pointer to glBindProgramARB function
typedef void (APIENTRY *GL_BindProgramARB_Func)(GLenum, GLuint);
extern GL_BindProgramARB_Func glBindProgramARB_ptr;

// Pointer to glProgramStringARB function
typedef void (APIENTRY *GL_ProgramStringARB_Func)(GLenum, GLenum, GLsizei, const GLvoid*);
extern GL_ProgramStringARB_Func glProgramStringARB_ptr;

// Pointer to glProgramLocalParameter4fvARB function
typedef void (APIENTRY *GL_ProgramLocalParameter4fvARB_Func)(GLenum, GLuint, const GLfloat *);
extern GL_ProgramLocalParameter4fvARB_Func glProgramLocalParameter4fvARB_ptr;

// Pointer to glProgramParameter4fvNV function
typedef void (APIENTRY *GL_ProgramParameter4fvNV_Func)(GLenum, GLuint, const GLfloat *);
extern GL_ProgramParameter4fvNV_Func glProgramParameter4fvNV_ptr;

// Pointer to glCombinerStageParameterfvNV function
typedef void (APIENTRY *GL_CombinerStageParameterfvNV_Func)(GLenum, GLenum, const GLfloat *);
extern GL_CombinerStageParameterfvNV_Func glCombinerStageParameterfvNV_ptr;

// Pointer to glCombinerParameterfvNV function
typedef void (APIENTRY *GL_CombinerParameterfvNV_Func)(GLenum, const GLfloat *);
extern GL_CombinerParameterfvNV_Func glCombinerParameterfvNV_ptr;

// Pointer to glCombinerParameteriNV function
typedef void (APIENTRY *GL_CombinerParameteriNV_Func)(GLenum, GLint);
extern GL_CombinerParameteriNV_Func glCombinerParameteriNV_ptr;

// Pointer to glCombinerInputNV function
typedef void (APIENTRY *GL_CombinerInputNV_Func)(GLenum, GLenum, GLenum, GLenum, GLenum, GLenum);
extern GL_CombinerInputNV_Func glCombinerInputNV_ptr;

// Pointer to glCombinerOutputNV function
typedef void (APIENTRY *GL_CombinerOutputNV_Func)(GLenum, GLenum, GLenum, GLenum, GLenum, GLenum, GLenum, GLboolean, GLboolean, GLboolean);

extern GL_CombinerOutputNV_Func glCombinerOutputNV_ptr;

// Pointer to glFinalCombinerInputNV function
typedef void (APIENTRY *GL_FinalCombinerInputNV_Func)(GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);

extern GL_FinalCombinerInputNV_Func glFinalCombinerInputNV_ptr;

// Pointer to glGetProgramivARB function
typedef void (APIENTRY *GL_GetProgramivARB_Func)(GLenum, GLenum, GLint *);
extern GL_GetProgramivARB_Func glGetProgramivARB_ptr;

// Pointer to glLoadProgramNV function
typedef void (APIENTRY *GL_LoadProgramNV_Func)(GLenum, GLuint, GLsizei, const GLubyte *);
extern GL_LoadProgramNV_Func glLoadProgramNV_ptr;

// Pointer to glTrackMatrixNV function
typedef void (APIENTRY *GL_TrackMatrixNV_Func)(GLenum, GLuint, GLenum, GLenum);
extern GL_TrackMatrixNV_Func glTrackMatrixNV_ptr;

// Pointer to glActiveStencilFaceEXT function
typedef void (APIENTRY *GL_ActiveStencilFaceEXT_Func)(GLenum);
extern GL_ActiveStencilFaceEXT_Func glActiveStencilFaceEXT_ptr;

// Pointer to glGenOcclusionQueriesNV function
typedef void (APIENTRY *GL_GenOcclusionQueriesNV_Func) (GLsizei n, GLuint *ids);
extern GL_GenOcclusionQueriesNV_Func glGenOcclusionQueriesNV_ptr;

// Pointer to glDeleteOcclusionQueriesNV function
typedef void (APIENTRY *GL_DeleteOcclusionQueriesNV_Func) (GLsizei n, const GLuint *ids);
extern GL_DeleteOcclusionQueriesNV_Func glDeleteOcclusionQueriesNV_ptr;

// Pointer to glIsOcclusionQueryNV function
typedef GLboolean (APIENTRY *GL_IsOcclusionQueryNV_Func) (GLuint id);
extern GL_IsOcclusionQueryNV_Func glIsOcclusionQueryNV_ptr;

// Pointer to glBeginOcclusionQueryNV function
typedef void (APIENTRY *GL_BeginOcclusionQueryNV_Func) (GLuint id);
extern GL_BeginOcclusionQueryNV_Func glBeginOcclusionQueryNV_ptr;

// Pointer to glEndOcclusionQueryNV function
typedef void (APIENTRY *GL_EndOcclusionQueryNV_Func) (void);
extern GL_EndOcclusionQueryNV_Func glEndOcclusionQueryNV_ptr;

// Pointer to glGetOcclusionQueryivNV function
typedef void (APIENTRY *GL_GetOcclusionQueryivNV_Func) (GLuint id, GLenum pname, GLint *params);
extern GL_GetOcclusionQueryivNV_Func glGetOcclusionQueryivNV_ptr;

// Pointer to glGetOcclusionQueryuivNV function
typedef void (APIENTRY *GL_GetOcclusionQueryuivNV_Func) (GLuint id, GLenum pname, GLuint *params);
extern GL_GetOcclusionQueryuivNV_Func glGetOcclusionQueryuivNV_ptr;


};

extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB_ptr;

namespace Ogre {
    // Forward declarations
    class GLSupport;
    class GLRenderSystem;
    class GLTexture;
    class GLTextureManager;
    class GLGpuProgram;

}


#endif

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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
#   include <windows.h>
#   include <wingdi.h>
#   include "gl.h"
#   define GL_GLEXT_PROTOTYPES
#   include "glprocs.h"
#   include <GL/glu.h>
#   include "glext.h"
// Windows library does not include glSecondaryColorPointer even though it's standard now
#   define glSecondaryColorPointer glSecondaryColorPointerEXT
#elif OGRE_PLATFORM == PLATFORM_LINUX
// define GL_GLEXT_LEGACY so that Mesa headers won't try to include their own
// glext.h file.
#   define GL_GLEXT_LEGACY
#   include <GL/gl.h>
#   include <GL/glu.h>
#   define GL_GLEXT_PROTOTYPES
#   include "glext.h"
#elif OGRE_PLATFORM == PLATFORM_APPLE
#   include <OpenGL/gl.h>
#   define GL_EXT_texture_env_combine 1
#   include <OpenGL/glext.h>
#   include <OpenGL/glu.h>
#endif

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

};

namespace Ogre {
    // Forward declarations
    class GLSupport;
    class GLRenderSystem;
    class GLTexture;
    class GLTextureManager;

}


#endif

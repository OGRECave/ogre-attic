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

// Need to define this for non win32 systems
#ifndef APIENTRY
#   define APIENTRY
#endif

namespace Ogre {
    // Forward declarations
    class GLSupport;
    class GLRenderSystem;
    class GLTexture;
    class GLTextureManager;

}


#endif

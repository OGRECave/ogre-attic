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
#elif OGRE_PLATFORM == PLATFORM_LINUX
#   include <GL/gl.h>
#   include <GL/glu.h>
#elif OGRE_PLATFORM == PLATFORM_APPLE
#   include <OpenGL/gl.h>
#   define GL_EXT_texture_env_combine 1
#   include <OpenGL/glext.h>
#   include <OpenGL/glu.h>
#endif

#ifndef GL_VERSION_1_3
#   include "glext.h"
// texture_env_combine
#   ifdef GL_ARB_texture_env_combine
#       define GL_COMBINE GL_COMBINE_ARB
#       define GL_COMBINE_RGB GL_COMBINE_RGB_ARB
#       define GL_COMBINE_ALPHA GL_COMBINE_ALPHA_ARB
#       define GL_RGB_SCALE GL_RGB_SCALE_ARB
#       define GL_ADD_SIGNED GL_ADD_SIGNED_ARB
#       define GL_INTERPOLATE GL_INTERPOLATE_ARB
#       define GL_CONSTANT GL_CONSTANT_ARB
#       define GL_PRIMARY_COLOR GL_PRIMARY_COLOR_ARB
#       define GL_PREVIOUS GL_PREVIOUS_ARB
#       define GL_SOURCE0_RGB GL_SOURCE0_RGB_ARB
#       define GL_SOURCE1_RGB GL_SOURCE1_RGB_ARB
#       define GL_SOURCE2_RGB GL_SOURCE2_RGB_ARB
#       define GL_SOURCE0_ALPHA GL_SOURCE0_ALPHA_ARB
#       define GL_SOURCE1_ALPHA GL_SOURCE1_ALPHA_ARB
#       define GL_SOURCE2_ALPHA GL_SOURCE2_ALPHA_ARB
#       define GL_OPERAND0_RGB GL_OPERAND0_RGB_ARB
#       define GL_OPERAND1_RGB GL_OPERAND1_RGB_ARB
#       define GL_OPERAND2_RGB GL_OPERAND2_RGB_ARB
#       define GL_OPERAND0_ALPHA GL_OPERAND0_ALPHA_ARB
#       define GL_OPERAND1_ALPHA GL_OPERAND1_ALPHA_ARB
#       define GL_OPERAND2_ALPHA GL_OPERAND2_ALPHA_ARB
#   elif GL_EXT_texture_env_combine
#       define GL_COMBINE GL_COMBINE_EXT
#       define GL_COMBINE_RGB GL_COMBINE_RGB_EXT
#       define GL_COMBINE_ALPHA GL_COMBINE_ALPHA_EXT
#       define GL_RGB_SCALE GL_RGB_SCALE_EXT
#       define GL_ADD_SIGNED GL_ADD_SIGNED_EXT
#       define GL_INTERPOLATE GL_INTERPOLATE_EXT
#       define GL_CONSTANT GL_CONSTANT_EXT
#       define GL_PRIMARY_COLOR GL_PRIMARY_COLOR_EXT
#       define GL_PREVIOUS GL_PREVIOUS_EXT
#       define GL_SOURCE0_RGB GL_SOURCE0_RGB_EXT
#       define GL_SOURCE1_RGB GL_SOURCE1_RGB_EXT
#       define GL_SOURCE2_RGB GL_SOURCE2_RGB_EXT
#       define GL_SOURCE0_ALPHA GL_SOURCE0_ALPHA_EXT
#       define GL_SOURCE1_ALPHA GL_SOURCE1_ALPHA_EXT
#       define GL_SOURCE2_ALPHA GL_SOURCE2_ALPHA_EXT
#       define GL_OPERAND0_RGB GL_OPERAND0_RGB_EXT
#       define GL_OPERAND1_RGB GL_OPERAND1_RGB_EXT
#       define GL_OPERAND2_RGB GL_OPERAND2_RGB_EXT
#       define GL_OPERAND0_ALPHA GL_OPERAND0_ALPHA_EXT
#       define GL_OPERAND1_ALPHA GL_OPERAND1_ALPHA_EXT
#       define GL_OPERAND2_ALPHA GL_OPERAND2_ALPHA_EXT
#   endif
// multitexture
#   ifdef GL_ARB_multitexture
#       define GL_MAX_TEXTURE_UNITS GL_MAX_TEXTURE_UNITS_ARB
#       define GL_TEXTURE0 GL_TEXTURE0_ARB
#   endif
// texture_env_dot3
#   ifdef GL_ARB_texture_env_dot3
#       define GL_DOT3_RGB GL_DOT3_RGB_ARB
#   elif GL_EXT_texture_env_dot3
#       define GL_DOT3_RGB GL_DOT3_RGB_EXT
#   endif
//#   ifndef GL_DOT3_RGB_EXT
//#       define GL_DOT3_RGB_EXT                   0x8740
//#   endif
// texture_cube_map
#   ifdef GL_ARB_texture_cube_map
#       define GL_TEXTURE_CUBE_MAP GL_TEXTURE_CUBE_MAP_ARB
#       define GL_TEXTURE_BINDING_CUBE_MAP GL_TEXTURE_BINDING_CUBE_MAP_ARB
#       define GL_TEXTURE_CUBE_MAP_POSITIVE_X GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB
#       define GL_TEXTURE_CUBE_MAP_NEGATIVE_X GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB
#       define GL_TEXTURE_CUBE_MAP_POSITIVE_Y GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB
#       define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB
#       define GL_TEXTURE_CUBE_MAP_POSITIVE_Z GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB
#       define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
#   elif GL_EXT_texture_cube_map
#       define GL_TEXTURE_CUBE_MAP GL_TEXTURE_CUBE_MAP_EXT
#       define GL_TEXTURE_BINDING_CUBE_MAP GL_TEXTURE_BINDING_CUBE_MAP_EXT
#       define GL_TEXTURE_CUBE_MAP_POSITIVE_X GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT
#       define GL_TEXTURE_CUBE_MAP_NEGATIVE_X GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT
#       define GL_TEXTURE_CUBE_MAP_POSITIVE_Y GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT
#       define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT
#       define GL_TEXTURE_CUBE_MAP_POSITIVE_Z GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT
#       define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT
#   endif
// texgen_reflection/texture_cube_map
#   ifdef GL_ARB_texture_cube_map
#       define GL_NORMAL_MAP GL_NORMAL_MAP_ARB
#       define GL_REFLECTION_MAP GL_REFLECTION_MAP_ARB
#   elif GL_EXT_texture_cube_map
#       define GL_NORMAL_MAP GL_NORMAL_MAP_EXT
#       define GL_REFLECTION_MAP GL_REFLECTION_MAP_EXT
#   elif GL_NV_texgen_reflection
#       define GL_NORMAL_MAP GL_NORMAL_MAP_NV
#       define GL_REFLECTION_MAP GL_REFLECTION_MAP_NV
#   endif
#endif

#ifndef GL_VERSION_1_4
// texture_mirrored_repeat
#   ifndef GL_MIRRORED_REPEAT
#       define GL_MIRRORED_REPEAT                0x8370
#   endif
// generate_mipmap
#   ifdef GL_SGIS_generate_mipmap 
#       define GL_GENERATE_MIPMAP GL_GENERATE_MIPMAP_SGIS
#   endif
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

/*
	ATI fragment shader Extension header file.
	setup by NFZ
	extracted from ATI 8500 SDK

** GL_ATI_fragment_shader
**
** Support:
**   Rage 128 * based  : Not Supported
**   Radeon   * based  : Not Supported
**   R200     * based  : Supported
**   R200 : 8500, 9000, 9100, 9200
**   also works on R300 but pointless since ARBFP1.0 supported
*/


#ifndef _GL_ATI_FRAGMENT_SHADER_H_
#define _GL_ATI_FRAGMENT_SHADER_H_

#include "OgreGLSupport.h"

bool InitATIFragmentShaderExtensions(Ogre::GLSupport& glSupport);

#endif	//_GL_ATI_FRAGMENT_SHADER_H_


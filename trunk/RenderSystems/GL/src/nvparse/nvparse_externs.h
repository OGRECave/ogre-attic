#ifndef NVPARSE_EXTERNS_H
#define NVPARSE_EXTERNS_H

extern nvparse_errors errors;
extern int line_number;
extern char * myin;

// HACK by SJS to get this damn stuff working with Win32
#ifdef _WIN32
    #define GL_GLEXT_PROTOTYPES
    #include <windows.h>
    #include <GL/gl.h>
    #include <GL/glext.h>
    #include "glprocs.h"
    #include <stdio.h>
#else
    #define GL_GLEXT_LEGACY
    #define GL_GLEXT_PROTOTYPES
	#if defined(__APPLE__) && defined(__GNUC__)
		#include <OpenGL/gl.h>
		#include "glext.h"
	#else
		#include <GL/gl.h>
		#include <GL/glext.h>
	#endif
#endif
// END HACK 

#endif

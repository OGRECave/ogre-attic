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
    #include <GL/gl.h>
    #define GL_GLEXT_PROTOTYPES
    #include <GL/glext.h>
#endif
// END HACK 

#endif

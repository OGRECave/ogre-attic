INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/RenderSystems/GL/include \
           -I$(top_srcdir)/OgreMain/include $(GLSUPPORT_CFLAGS) \
           -I$(top_srcdir)/PlatformManagers/$(OGRE_PLATFORM)/include \
           -I$(top_srcdir)/RenderSystems/GL/src/$(OGRE_GLSUPPORT) \
           -I$(top_srcdir)/RenderSystems/GL/src/GLSL/include

noinst_LIBRARIES=libGLSL.a
libGLSL_a_SOURCES = OgreGLSLExtSupport.cpp \
                    OgreGLSLLinkProgramManager.cpp \
                    OgreGLSLGpuProgram.cpp \
                    OgreGLSLProgram.cpp \
                    OgreGLSLLinkProgram.cpp \
                    OgreGLSLProgramFactory.cpp

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/RenderSystems/GL/include \
           -I$(top_srcdir)/OgreMain/include $(GLSUPPORT_CFLAGS) \
           -I$(top_srcdir)/PlatformManagers/$(OGRE_PLATFORM)/include \
           -I$(top_srcdir)/RenderSystems/GL/src/$(OGRE_GLSUPPORT)

pkglib_PROGRAMS = RenderSystem_GL.so

RenderSystem_GL_so_SOURCES = OgreGLEngineDll.cpp \
                           OgreGLRenderSystem.cpp \
						   OgreGLSupport.cpp \
                           OgreGLTexture.cpp \
                           OgreGLTextureManager.cpp \
                           OgreGTKGLSupport.cpp

RenderSystem_GL_so_LDFLAGS = $(SHARED_FLAGS) -L$(top_srcdir)/OgreMain/src
RenderSystem_GL_so_LDADD = $(GLSUPPORT_LIBS) $(GL_LIBS) -lOgreMain

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/RenderSystems/SDL/include -I$(top_srcdir)/OgreMain/include $(SDL_CFLAGS)

pkglib_PROGRAMS = RenderSystem_GL.so

RenderSystem_GL_so_SOURCES = OgreGLEngineDll.cpp \
                           OgreGLRenderSystem.cpp \
                           OgreGLTexture.cpp \
                           OgreGLTextureManager.cpp \
                           OgreSDLGLSupport.cpp \
                           OgreSDLWindow.cpp

RenderSystem_GL_so_LDFLAGS = $(SHARED_FLAGS) -L$(top_srcdir)/OgreMain/src
RenderSystem_GL_so_LDADD = $(SDL_LIBS) $(GL_LIBS) -lOgreMain

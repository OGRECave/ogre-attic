INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/RenderSystems/SDL/include -I$(top_srcdir)/OgreMain/include $(SDL_CFLAGS)

pkglib_PROGRAMS = RenderSystem_SDL.so

RenderSystem_SDL_so_SOURCES = OgreSDLEngineDll.cpp \
                           OgreSDLExtensionManager.cpp \
                           OgreSDLRenderSystem.cpp \
                           OgreSDLTexture.cpp \
                           OgreSDLTextureManager.cpp \
                           OgreSDLWindow.cpp

RenderSystem_SDL_so_LDFLAGS = $(SHARED_FLAGS) -L$(top_srcdir)/OgreMain/src
RenderSystem_SDL_so_LDADD = $(SDL_LIBS) $(GL_LIBS) -lOgreMain

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/RenderSystems/SDL/include -I$(top_srcdir)/OgreMain/include $(SDL_CFLAGS)

noinst_PROGRAMS = RenderSystem_SDL.so

RenderSystem_SDL_so_SOURCES = OgreSDLEngineDll.cpp \
                           OgreSDLRenderSystem.cpp \
                           OgreSDLTexture.cpp \
                           OgreSDLTextureManager.cpp \
                           OgreSDLWindow.cpp

RenderSystem_SDL_so_LDFLAGS = -shared
RenderSystem_SDL_so_LDADD = $(SDL_LIBS) -lGL -lGLU

install-exec-local:
	mkdir -m 775 -p $(pkglibdir)
	$(INSTALL_DATA) RenderSystem_SDL.so $(pkglibdir)/RenderSystem_SDL.so

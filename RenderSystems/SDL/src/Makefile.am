INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/RenderSystems/SDL/include -I$(top_srcdir)/OgreMain/include $(SDL_CFLAGS)

noinst_PROGRAMS = OgreSDLEngine.so

OgreSDLEngine_so_SOURCES = OgreSDLEngineDll.cpp \
                           OgreSDLRenderSystem.cpp \
                           OgreSDLTexture.cpp \
                           OgreSDLTextureManager.cpp \
                           OgreSDLWindow.cpp

OgreSDLEngine_so_LDFLAGS = -shared
OgreSDLEngine_so_LDADD = $(SDL_LIBS) -lGL -lGLU

install-exec-local:
	mkdir -m 775 -p $(pkglibdir)
	$(INSTALL_DATA) OgreSDLEngine.so $(pkglibdir)/OgreSDLEngine.so

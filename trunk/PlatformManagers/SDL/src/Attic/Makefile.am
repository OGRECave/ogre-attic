INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlatformManagers/SDL/include -I$(top_srcdir)/OgreMain/include $(SDL_CFLAGS)

noinst_PROGRAMS = OgreSDLPlatform.so

OgreSDLPlatform_so_SOURCES = OgreSDLInput.cpp \
                             OgreSDLPlatform.cpp


OgreSDLPlatform_so_LDFLAGS = -shared
OgreSDLPlatform_so_LDADD = $(SDL_LIBS) -lGL -lGLU

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlatformManagers/SDL/include -I$(top_srcdir)/OgreMain/include $(SDL_CFLAGS) `pkg-config --cflags gtkmm-2.0 libglademm-2.0`

noinst_PROGRAMS = OgrePlatform.so

OgrePlatform_so_SOURCES = OgreSDLInput.cpp \
                          OgreSDLPlatform.cpp


OgrePlatform_so_LDFLAGS = -shared `pkg-config --libs gtkmm-2.0 libglademm-2.0`
OgrePlatform_so_LDADD = $(SDL_LIBS) -lGL -lGLU 

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlatformManagers/SDL/include -I$(top_srcdir)/OgreMain/include $(SDL_CFLAGS) $(CFGTK_DEPS_CFLAGS) -DSHAREDIR=\"$(pkgdatadir)\" 

noinst_PROGRAMS = OgrePlatform.so

OgrePlatform_so_SOURCES = OgreSDLConfig_$(OGRE_CFGTK).cpp\
                          OgreSDLInput.cpp \
                          OgreSDLPlatform.cpp


OgrePlatform_so_LDFLAGS = -shared
OgrePlatform_so_LDADD = $(SDL_LIBS) $(CFGTK_DEPS_LIBS) -lGL -lGLU 

pkgdata_DATA = ogre-config.glade

install-exec-local:
	$(INSTALL_DATA) OgrePlatform.so $(top_builddir)/Samples/Common/bin/Debug/OgrePlatform.so

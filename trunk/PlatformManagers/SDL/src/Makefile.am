INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlatformManagers/SDL/include -I$(top_srcdir)/OgreMain/include $(SDL_CFLAGS) $(CFGTK_DEPS_CFLAGS) -DSHAREDIR=\"$(pkgdatadir)\" 

noinst_PROGRAMS = OgrePlatform.so

OgrePlatform_so_SOURCES = OgreSDLConfig.cpp\
                          OgreSDLInput.cpp \
                          OgreSDLPlatform.cpp

BUILT_SOURCES = OgreSDLConfig.cpp
EXTRA_DIST = OgreSDLConfig_gtk.cpp OgreSDLConfig_cli.cpp


OgrePlatform_so_LDFLAGS = $(SHARED_FLAGS) -L$(top_srcdir)/OgreMain/src
OgrePlatform_so_LDADD = $(SDL_LIBS) $(CFGTK_DEPS_LIBS) $(GL_LIBS) -lOgreMain

pkgdata_DATA = ogre-config.glade

OgreSDLConfig.cpp:
	-rm -f OgreSDLConfig.cpp
	$(LN_S) OgreSDLConfig_$(OGRE_CFGTK).cpp OgreSDLConfig.cpp

clean-local:
	-rm -f $(BUILT_SOURCES)

install-exec-local:
	$(INSTALL) .libs/OgrePlatform.so $(libdir)/OgrePlatform.so

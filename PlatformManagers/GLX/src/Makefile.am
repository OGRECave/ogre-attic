INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlatformManagers/GLX/include -I$(top_srcdir)/RenderSystems/GL/include -I$(top_srcdir)/OgreMain/include $(PLATFORM_CFLAGS) -DSHAREDIR=\"$(pkgdatadir)\" 

platformdir = $(libdir)
platform_PROGRAMS = libOgrePlatform.so

libOgrePlatform_so_SOURCES = OgreGLXConfig.cpp\
                          OgreGLXInput.cpp \
			  OgreGLXTimer.cpp \
                          OgreGLXPlatform.cpp

libOgrePlatform_so_LDFLAGS = $(SHARED_FLAGS) -L$(top_srcdir)/OgreMain/src
libOgrePlatform_so_LDADD = $(PLATFORM_LIBS) $(GL_LIBS) -lOgreMain -lXaw


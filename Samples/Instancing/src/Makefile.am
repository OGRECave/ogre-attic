INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Instancing/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(OIS_CFLAGS)

ogreexec_PROGRAMS = Instancing

Instancing_SOURCES = Instancing.cpp
Instancing_LDFLAGS = -L$(top_builddir)/OgreMain/src
Instancing_LDADD = -lOgreMain $(OIS_LIBS)

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Compositor/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include

noinst_PROGRAMS = Compositor

Compositor_SOURCES = Compositor.cpp
Compositor_LDFLAGS = -L$(top_builddir)/OgreMain/src
Compositor_LDADD = -lOgreMain

install-exec-local:
	$(INSTALL_PROGRAM) .libs/Compositor $(top_builddir)/Samples/Common/bin/Compositor

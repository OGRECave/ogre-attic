INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/SkyPlane/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include

noinst_PROGRAMS = SkyPlane
SkyPlane_SOURCES = SkyPlane.cpp
SkyPlane_LDFLAGS = -L$(top_srcdir)/OgreMain/src
SkyPlane_LDADD = -lOgreMain

install-exec-local:
	$(INSTALL_PROGRAM) SkyPlane $(top_builddir)/Samples/Common/bin/Debug/SkyPlane

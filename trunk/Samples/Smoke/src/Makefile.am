INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Smoke/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include

noinst_PROGRAMS = Smoke

Smoke_SOURCES = Smoke.cpp
Smoke_LDFLAGS = -L$(top_srcdir)/OgreMain/src
Smoke_LDADD = -lOgreMain

install-exec-local:
	$(INSTALL_PROGRAM) .libs/Smoke $(top_builddir)/Samples/Common/bin/Smoke

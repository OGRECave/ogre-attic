INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/../OgreMain/include -I$(top_srcdir)/Common/include -I$(top_srcdir)/ReferenceAppLayer/include

noinst_PROGRAMS = BspCollision

BspCollision_SOURCES = BspCollision.cpp
BspCollision_LDFLAGS = -L$(top_srcdir)/../OgreMain/src -L$(top_srcdir)/ReferenceAppLayer/src
BspCollision_LDADD = -lOgreMain -lOgreReferenceAppLayer

install-exec-local:
	$(INSTALL_PROGRAM) .libs/BspCollision $(top_builddir)/../Samples/Common/bin/BspCollision

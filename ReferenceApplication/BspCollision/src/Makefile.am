INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include -I$(top_srcdir)/ReferenceApplication/ReferenceAppLayer/include

noinst_PROGRAMS = BspCollsion

BspCollsion_SOURCES = BspCollision.cpp
BspCollsion_LDFLAGS = -L$(top_srcdir)/OgreMain/src
BspCollsion_LDADD = -lOgreMain -lOgreReferenceAppLayer

install-exec-local:
	$(INSTALL_PROGRAM) .libs/BspCollision $(top_builddir)/Samples/Common/bin/BspCollision

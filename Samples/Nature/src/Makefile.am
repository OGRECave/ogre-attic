INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Nature/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include

noinst_PROGRAMS = Nature 
Nature_SOURCES = Nature.cpp
Nature_LDFLAGS = -L$(top_srcdir)/OgreMain/src
Nature_LDADD = -lOgreMain -lpthread

install-exec-local:
	$(INSTALL_PROGRAM) .libs/Nature $(top_builddir)/Samples/Common/bin/Nature

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Transpacency/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include

noinst_PROGRAMS = RenderToTexture
RenderToTexture_SOURCES = RenderToTexture.cpp
RenderToTexture_LDFLAGS = -L$(top_srcdir)/OgreMain/src
RenderToTexture_LDADD = -lOgreMain

install-exec-local:
	$(INSTALL_PROGRAM) .libs/RenderToTexture $(top_builddir)/Samples/Common/bin/RenderToTexture

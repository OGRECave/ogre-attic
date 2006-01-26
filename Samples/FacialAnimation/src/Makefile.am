INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/FacialAnimation/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include

noinst_PROGRAMS = FacialAnimation

FacialAnimation_SOURCES = FacialAnimation.cpp
FacialAnimation_LDFLAGS = -L$(top_builddir)/OgreMain/src
FacialAnimation_LDADD = -lOgreMain

install-exec-local:
	$(INSTALL_PROGRAM) .libs/FacialAnimation $(top_builddir)/Samples/Common/bin/FacialAnimation

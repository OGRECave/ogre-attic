INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/CameraTrack/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include

noinst_PROGRAMS = CameraTrack

CameraTrack_SOURCES = CameraTrack.cpp
CameraTrack_LDFLAGS = -L$(top_srcdir)/OgreMain/src
CameraTrack_LDADD = -lOgreMain

install-exec-local:
	$(INSTALL_PROGRAM) CameraTrack $(top_builddir)/Samples/Common/bin/Debug/CameraTrack

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include @CEGUI_CFLAGS@

noinst_PROGRAMS = Gui
Gui_SOURCES = Gui.cpp
Gui_LDFLAGS = -L$(top_builddir)/OgreMain/src
Gui_LDADD = -lOgreMain @CEGUI_LIBS@

install-exec-local:
	$(INSTALL_PROGRAM) .libs/Gui $(top_builddir)/Samples/Common/bin/Gui

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/Gui/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include -I$(top_srcdir)/Plugins/GuiElements/include

noinst_PROGRAMS = Gui
Gui_SOURCES = Gui.cpp
Gui_LDFLAGS = -L$(top_srcdir)/OgreMain/src -L$(top_srcdir)/Plugins/GuiElements/src
Gui_LDADD = -lOgreMain -lPlugin_GuiElements

install-exec-local:
	$(INSTALL_PROGRAM) .libs/Gui $(top_builddir)/Samples/Common/bin/Gui

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/GuiElements/include -I$(top_srcdir)/OgreMain/include

noinst_PROGRAMS = Plugin_GuiElements.so

Plugin_GuiElements_so_SOURCES = OgreBorderPanelGuiElement.cpp \
				OgreGuiElements.cpp \
				OgrePanelGuiElement.cpp

Plugin_GuiElements_so_LDFLAGS = -shared

install-exec-local:
	mkdir -m 755 -p $(pkglibdir)
	$(INSTALL_DATA) Plugin_GuiElements.so $(pkglibdir)/Plugin_GuiElements.so

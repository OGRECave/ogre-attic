INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/GuiElements/include -I$(top_srcdir)/OgreMain/include $(FT2_CFLAGS) 

noinst_PROGRAMS = Plugin_GuiElements.so

Plugin_GuiElements_so_SOURCES = OgreBorderPanelGuiElement.cpp \
				OgreButtonGuiElement.cpp \
				OgreCursorGuiElement.cpp \
				OgreGuiElements.cpp \
				OgreListGuiElement.cpp \
				OgrePanelGuiElement.cpp \
				OgreTextAreaGuiElement.cpp

Plugin_GuiElements_so_LDFLAGS = $(SHARED_FLAGS)

install-exec-local:
	mkdir -m 755 -p $(pkglibdir)
	$(INSTALL_DATA) Plugin_GuiElements.so $(pkglibdir)/Plugin_GuiElements.so

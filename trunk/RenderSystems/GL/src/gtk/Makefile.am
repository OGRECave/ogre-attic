INCLUDES = $(STLPORT_CFLAGS) -I$(top_builddir)/RenderSystems/GL/include \
           -I$(top_builddir)/OgreMain/include $(GLSUPPORT_CFLAGS) \
           -I$(top_builddir)/PlatformManagers/$(OGRE_PLATFORM)/include \
           -I$(top_builddir)/RenderSystems/GL/include/$(OGRE_GLSUPPORT) \
           -I$(top_builddir)/RenderSystems/GL/src/$(OGRE_GLSUPPORT)

noinst_LTLIBRARIES = libsupport.la

libsupport_la_SOURCES = OgreGTKGLSupport.cpp OgreGTKWindow.cpp

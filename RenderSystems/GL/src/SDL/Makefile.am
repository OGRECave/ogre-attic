INCLUDES = $(STLPORT_CFLAGS) -I$(top_builddir)/RenderSystems/GL/include \
           -I$(top_builddir)/OgreMain/include $(GLSUPPORT_CFLAGS) \
           -I$(top_builddir)/PlatformManagers/$(OGRE_PLATFORM)/include 

noinst_LIBRARIES = libsupport.a

libsupport_a_SOURCES = OgreSDLGLSupport.cpp OgreSDLWindow.cpp

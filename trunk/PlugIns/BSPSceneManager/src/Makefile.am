INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/BSPSceneManager/include -I$(top_srcdir)/OgreMain/include

noinst_PROGRAMS = BSPSceneManager.so

BSPSceneManager_so_SOURCES = OgreBspLevel.cpp \
                             OgreBspNode.cpp \
                             OgreBspResourceManager.cpp \
                             OgreBspSceneManager.cpp \
                             OgreBspSceneManagerDll.cpp \
                             OgreQuake3ShaderManager.cpp \
                             OgreQuake3Shader.cpp \
                             OgreQuake3Level.cpp


BSPSceneManager_so_LDFLAGS = -shared

install-exec-local:
	mkdir -m 755 -p $(pkglibdir)
	$(INSTALL_DATA) BSPSceneManager.so $(pkglibdir)/BSPSceneManager.so

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/BSPSceneManager/include -I$(top_srcdir)/OgreMain/include

noinst_PROGRAMS = Plugin_BSPSceneManager.so

Plugin_BSPSceneManager_so_SOURCES = OgreBspLevel.cpp \
                             OgreBspNode.cpp \
                             OgreBspResourceManager.cpp \
                             OgreBspSceneManager.cpp \
                             OgreBspSceneManagerDll.cpp \
                             OgreQuake3ShaderManager.cpp \
                             OgreQuake3Shader.cpp \
                             OgreQuake3Level.cpp


Plugin_BSPSceneManager_so_LDFLAGS = $(SHARED_FLAGS)

install-exec-local:
	mkdir -m 755 -p $(pkglibdir)
	$(INSTALL_DATA) Plugin_BSPSceneManager.so $(pkglibdir)/Plugin_BSPSceneManager.so

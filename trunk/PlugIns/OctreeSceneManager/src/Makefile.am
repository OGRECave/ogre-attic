INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/OctreeSceneManager/include -I$(top_srcdir)/OgreMain/include

noinst_PROGRAMS = Plugin_OctreeSceneManager.so

Plugin_OctreeSceneManager_so_SOURCES = OgreOctreeSceneManagerDll.cpp \
                             OgreOctreeSceneManager.cpp \
                             OgreOctreeNode.cpp \
                             OgreOctreeCamera.cpp \
                             OgreOctree.cpp 


Plugin_OctreeSceneManager_so_LDFLAGS = -shared

install-exec-local:
	mkdir -m 755 -p $(pkglibdir)
	$(INSTALL_DATA) Plugin_OctreeSceneManager.so $(pkglibdir)/Plugin_OctreeSceneManager.so

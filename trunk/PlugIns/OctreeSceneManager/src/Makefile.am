INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/OctreeSceneManager/include -I$(top_srcdir)/OgreMain/include

noinst_PROGRAMS = OctreeSceneManager.so

OctreeSceneManager_so_SOURCES = OgreOctreeSceneManagerDll.cpp \
                             OgreOctreeSceneManager.cpp \
                             OgreOctreeNode.cpp \
                             OgreOctreeCamera.cpp \
                             OgreOctree.cpp 


OctreeSceneManager_so_LDFLAGS = -shared

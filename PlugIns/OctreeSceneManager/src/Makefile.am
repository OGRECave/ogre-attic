INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/OctreeSceneManager/include -I$(top_srcdir)/OgreMain/include

pkglib_PROGRAMS = Plugin_OctreeSceneManager.so

Plugin_OctreeSceneManager_so_SOURCES = \
							OgreHeightmapTerrainPageSource.cpp \
							OgreOctreeSceneManagerDll.cpp \
                            OgreOctreeSceneManager.cpp \
                            OgreOctreeNode.cpp \
                            OgreOctreeCamera.cpp \
							OgreOctree.cpp \
							OgreTerrainPage.cpp \
							OgreTerrainPageSource.cpp \
							OgreTerrainRenderable.cpp \
							OgreTerrainSceneManager.cpp \
							OgreTerrainVertexProgram.cpp 


Plugin_OctreeSceneManager_so_LDFLAGS = $(SHARED_FLAGS) -L$(top_srcdir)/OgreMain/src
Plugin_OctreeSceneManager_so_LDADD = -lOgreMain

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/BSPSceneManager/include -I$(top_srcdir)/OgreMain/include

pkglib_PROGRAMS = Plugin_BSPSceneManager.so

Plugin_BSPSceneManager_so_SOURCES = OgreBspLevel.cpp \
                             OgreBspNode.cpp \
                             OgreBspResourceManager.cpp \
                             OgreBspSceneManager.cpp \
                             OgreBspSceneManagerDll.cpp \
							 OgreBspSceneNode.cpp \
                             OgreQuake3ShaderManager.cpp \
                             OgreQuake3Shader.cpp \
                             OgreQuake3Level.cpp


Plugin_BSPSceneManager_so_LDFLAGS = $(SHARED_FLAGS) -L$(top_srcdir)/OgreMain/src
Plugin_BSPSceneManager_so_LDADD = -lOgreMain

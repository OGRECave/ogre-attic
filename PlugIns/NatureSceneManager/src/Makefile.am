INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/NatureSceneManager/include -I$(top_srcdir)/OgreMain/include

pkglib_PROGRAMS = Plugin_NatureSceneManager.so

Plugin_NatureSceneManager_so_SOURCES = NaturePatch.cpp \
				       NatureCustomPatch.cpp \
				       NatureTerrainPatch.cpp \
				       NaturePatchManager.cpp \
				       NatureSceneManager.cpp \
				       NatureSceneManagerDll.cpp \
				       HeightmapLoader.cpp

Plugin_NatureSceneManager_so_LDFLAGS = $(SHARED_FLAGS) -L$(top_srcdir)/OgreMain/src
Plugin_NatureSceneManager_so_LDADD = -lOgreMain

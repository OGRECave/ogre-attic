INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/NatureSceneManager/include -I$(top_srcdir)/OgreMain/include

pkglib_LTLIBRARIES = Plugin_NatureSceneManager.la

Plugin_NatureSceneManager_la_SOURCES = NaturePatch.cpp \
				       NatureCustomPatch.cpp \
				       NatureTerrainPatch.cpp \
				       NaturePatchManager.cpp \
				       NatureSceneManager.cpp \
				       NatureSceneManagerDll.cpp \
				       HeightmapLoader.cpp

Plugin_NatureSceneManager_la_LDFLAGS = -module $(PLUGIN_FLAGS) -L$(top_builddir)/OgreMain/src
Plugin_NatureSceneManager_la_LIBADD = -lOgreMain

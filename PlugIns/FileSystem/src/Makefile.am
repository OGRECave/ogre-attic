INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/FileSystem/include -I$(top_srcdir)/OgreMain/include

pkglib_LTLIBRARIES = Plugin_FileSystem.la

Plugin_FileSystem_la_SOURCES = FileSystem.cpp FileSystemFactory.cpp SearchOps.cpp

Plugin_FileSystem_la_LDFLAGS = -module $(PLUGIN_FLAGS) -L$(top_builddir)/OgreMain/src
Plugin_FileSystem_la_LIBADD = -lOgreMain


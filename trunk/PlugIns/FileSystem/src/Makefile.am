INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/FileSystem/include -I$(top_srcdir)/OgreMain/include

pkglib_PROGRAMS = Plugin_FileSystem.so

Plugin_FileSystem_so_SOURCES = FileSystem.cpp FileSystemFactory.cpp SearchOps.cpp

Plugin_FileSystem_so_LDFLAGS = $(SHARED_FLAGS) -L$(top_srcdir)/OgreMain/src
Plugin_FileSystem_so_LDADD = -lOgreMain

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/FileSystem/include -I$(top_srcdir)/OgreMain/include

noinst_PROGRAMS = Plugin_FileSystem.so

Plugin_FileSystem_so_SOURCES = FileSystem.cpp FileSystemFactory.cpp SearchOps.cpp

Plugin_FileSystem_so_LDFLAGS = -shared

install-exec-local:
	mkdir -m 755 -p $(pkglibdir)
	$(INSTALL_DATA) Plugin_FileSystem.so $(pkglibdir)/Plugin_FileSystem.so

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/FileSystem/include -I$(top_srcdir)/OgreMain/include

noinst_PROGRAMS = FileSystem.so

FileSystem_so_SOURCES = FileSystem.cpp FileSystemFactory.cpp SearchOps.cpp

FileSystem_so_LDFLAGS = -shared

install-exec-local:
	mkdir -m 755 -p $(pkglibdir)
	$(INSTALL_DATA) FileSystem.so $(pkglibdir)/FileSystem.so

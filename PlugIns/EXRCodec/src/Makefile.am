INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/EXRCodec/include -I$(top_srcdir)/OgreMain/include $(OPENEXR_CFLAGS)

pkglib_PROGRAMS = Plugin_EXRCodec.so

Plugin_EXRCodec_so_SOURCES = OgreEXRCodec.cpp \
			     O_IStream.cpp \
                             OgreEXRCodecDll.cpp 

Plugin_EXRCodec_so_LDFLAGS = $(SHARED_FLAGS) -L$(top_srcdir)/OgreMain/src
Plugin_EXRCodec_so_LDADD = -lOgreMain $(OPENEXR_LIBS)

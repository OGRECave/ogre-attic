INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/Samples/GTKDemo/include -I$(top_srcdir)/OgreMain/include -I$(top_srcdir)/Samples/Common/include $(PLATFORM_CFLAGS)

ogreexec_PROGRAMS = GTKDemo
GTKDemo_SOURCES = GTKDemo.cpp
GTKDemo_LDFLAGS = -L$(top_builddir)/OgreMain/src $(PLATFORM_LIBS)
GTKDemo_LDADD = -lOgreMain

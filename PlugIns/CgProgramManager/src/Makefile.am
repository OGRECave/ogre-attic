INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/CgProgramManager/include -I$(top_srcdir)/OgreMain/include

pkglib_PROGRAMS = Plugin_CgProgramManager.so

Plugin_CgProgramManager_so_SOURCES = OgreCgProgram.cpp \
                             OgreCgProgramFactory.cpp \
                             OgreCgProgramManagerDll.cpp 

Plugin_CgProgramManager_so_LDFLAGS = $(SHARED_FLAGS) -L$(top_srcdir)/OgreMain/src
Plugin_CgProgramManager_so_LDADD = -lOgreMain -lCg

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/ParticleFX/include -I$(top_srcdir)/OgreMain/include

pkglib_PROGRAMS = Plugin_ParticleFX.so

Plugin_ParticleFX_so_SOURCES = OgreAreaEmitter.cpp \
                        OgreBoxEmitter.cpp \
                        OgreColourFaderAffector.cpp \
                        OgreCylinderEmitter.cpp \
                        OgreEllipsoidEmitter.cpp \
                        OgreHollowEllipsoidEmitter.cpp \
                        OgreLinearForceAffector.cpp \
                        OgreParticleFX.cpp \
                        OgrePointEmitter.cpp \
                        OgreRingEmitter.cpp

Plugin_ParticleFX_so_LDFLAGS = $(SHARED_FLAGS) -L$(top_srcdir)/OgreMain/src
Plugin_ParticleFX_so_LDADD = -lOgreMain

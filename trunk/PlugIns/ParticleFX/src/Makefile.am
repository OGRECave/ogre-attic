INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/ParticleFX/include -I$(top_srcdir)/OgreMain/include

noinst_PROGRAMS = ParticleFX.so

ParticleFX_so_SOURCES = OgreAreaEmitter.cpp \
                        OgreBoxEmitter.cpp \
                        OgreColourFaderAffector.cpp \
                        OgreCylinderEmitter.cpp \
                        OgreEllipsoidEmitter.cpp \
                        OgreHollowEllipsoidEmitter.cpp \
                        OgreLinearForceAffector.cpp \
                        OgreParticleFX.cpp \
                        OgrePointEmitter.cpp \
                        OgreRingEmitter.cpp

ParticleFX_so_LDFLAGS = -shared

INCLUDES = $(STLPORT_CFLAGS) -I$(top_srcdir)/PlugIns/ParticleFX/include -I$(top_srcdir)/OgreMain/include

noinst_PROGRAMS = Plugin_ParticleFX.so

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

Plugin_ParticleFX_so_LDFLAGS = -shared

install-exec-local:
	mkdir -m 755 -p $(pkglibdir)
	$(INSTALL_DATA) Plugin_ParticleFX.so $(pkglibdir)/Plugin_ParticleFX.so

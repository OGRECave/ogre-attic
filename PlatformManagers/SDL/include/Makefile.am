noinst_HEADERS = OgreSDLConfig_cli.h OgreSDLConfig_gtk.h OgreSDLError.h OgreSDLInput.h

all:
	-rm -f OgreSDLConfig.h
	$(LN_S) OgreSDLConfig_$(OGRE_CFGTK).h OgreSDLConfig.h

noinst_HEADERS = OgreSDLConfig_cli.h \
				OgreSDLConfig_gtk.h \
				OgreSDLError.h \
				OgreSDLInput.h \
				OgreSDLTimer.h

all:
	-rm -f $(srcdir)/OgreSDLConfig.h
	$(LN_S) $(srcdir)/OgreSDLConfig_$(OGRE_CFGTK).h $(srcdir)/OgreSDLConfig.h

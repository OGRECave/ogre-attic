INCLUDES = -I../include -I../../../Samples/Common/include $(BOOST_CFLAGS) $(PYTHON_CFLAGS)

noinst_PROGRAMS = Ogre.so

Ogre_so_SOURCES = OgrePythonCameraLink.cpp \
                  OgrePythonGeneralLink.cpp \
                  OgrePythonLink.cpp \
                  OgrePythonMaterialLink.cpp \
                  OgrePythonMeshLink.cpp \
                  OgrePythonRenderLink.cpp \
                  OgrePythonSceneLink.cpp \
                  OgrePythonTextureLink.cpp \
                  OgrePythonUtilityLink.cpp

Ogre_so_LDFLAGS = -shared
Ogre_so_LDADD = $(BOOST_LIBS) $(PYTHON_LIBS) ../../../OgreMain/src/libOgreMain.la

install-exec-local:
	$(INSTALL_PROGRAM) .libs/Ogre.so ../../../Samples/Common/bin/Ogre.so

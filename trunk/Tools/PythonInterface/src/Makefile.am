INCLUDES = -I../include -I../../../Samples/Common/include $(BOOST_CFLAGS) $(PYTHON_CFLAGS)

noinst_PROGRAMS = Ogre.so

Ogre_so_SOURCES = general.cpp \
                  main.cpp \
                  AxisAlignedBox.cpp \
                  Camera.cpp \
                  ColourValue.cpp \
                  ConfigDialog.cpp \
                  Entity.cpp \
                  ErrorDialog.cpp \
                  EventTarget.cpp \
                  EventQueue.cpp \
                  FrameListener.cpp \
                  Input.cpp \
                  InputEvent.cpp \
                  KeyEvent.cpp \
                  KeyListener.cpp \
                  Light.cpp \
                  Material.cpp \
                  Matrix3.cpp \
                  Matrix4.cpp \
                  Mesh.cpp \
                  MeshManager.cpp \
                  MouseMotionListener.cpp \
                  MovableObject.cpp \
                  Node.cpp \
                  Plane.cpp \
                  PlatformManager.cpp \
                  PositionTarget.cpp \
                  Quaternion.cpp \
                  RenderSystem.cpp \
                  RenderTarget.cpp \
                  RenderWindow.cpp \
                  Resource.cpp \
                  ResourceManager.cpp \
                  Root.cpp \
                  SceneManager.cpp \
                  SceneNode.cpp \
                  Sphere.cpp \
                  SubMesh.cpp \
                  Texture.cpp \
                  TextureManager.cpp \
                  Vector3.cpp \
                  Viewport.cpp

Ogre_so_LDFLAGS = -shared
Ogre_so_LDADD = $(BOOST_LIBS) $(PYTHON_LIBS) ../../../OgreMain/src/libOgreMain.la

install-exec-local:
	$(INSTALL_PROGRAM) .libs/Ogre.so ../../../Samples/Common/bin/Ogre.so

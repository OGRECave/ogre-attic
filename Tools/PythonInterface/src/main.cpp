#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
 
#include "OgrePythonLink.h"
#include "OgrePythonFrameListener.h"
#include "ExampleFrameListener.h"
 
using namespace boost::python;
using namespace Ogre;

BOOST_PYTHON_MODULE(Ogre)
{
    exportEnums();
    exportGeneral();
    exportVector3();
    exportMatrix3();
    exportMatrix4();
    exportQuaternion();
    exportResource();
    exportResourceManager();
    exportRoot();
    exportRenderTarget();
    exportRenderWindow();
    exportCamera();
    exportViewport();
    exportColourValue();
    exportSceneManager();
    exportTexture();
    exportTextureManager();
    exportPlane();
    exportRenderSystem();
    exportMaterial();
    exportMeshManager();
    exportMesh();
    exportSubMesh();
    exportMovableObject();
    exportEntity();
    exportLight();
    exportNode();
    exportSceneNode();
    exportFrameListener();
    exportAxisAlignedBox();
    exportSphere();
    exportNode();
    exportInputEvent();
    exportKeyEvent();
    exportKeyListener();
    exportEventTarget();
    exportPositionTarget();
    exportPlatformManager();
    exportConfigDialog();
    exportErrorDialog();
    exportInput();
    exportEventQueue();
    exportMouseMotionListener();
}

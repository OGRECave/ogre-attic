#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"
#include "OgrePythonFrameListener.h"
#include "ExampleFrameListener.h"

using namespace boost::python;
using namespace Ogre;

void exportRoot()
{
    class_<Root>("Root", init<optional<const String&> >())
        .def("saveConfig", &Root::saveConfig)
        .def("restoreConfig", &Root::restoreConfig)
        .def("showConfigDialog", &Root::showConfigDialog)
        .def("addRenderSystem", &Root::addRenderSystem)
        .def("setSceneManager", &Root::setSceneManager)
        .def("getAvailableRenderers", &Root::getAvailableRenderers, rir())
        .def("setRenderSystem", &Root::setRenderSystem)
        .def("getRenderSystem", &Root::getRenderSystem, rir())
        .def("initialise", &Root::initialise, rir())
        .def("getSceneManager", &Root::getSceneManager, rir())
        .def("getTextureManager", &Root::getTextureManager, rir())
        .def("getMeshManager", &Root::getMeshManager, rir())
        .def("getErrorDescription", &Root::getErrorDescription)
        .def("addFrameListener", &Root::addFrameListener)
        .def("removeFrameListener", &Root::removeFrameListener)
        .def("startRendering", &Root::startRendering)
        .def("shutdown", &Root::shutdown)
        .def("addResourceLocation", &Root::addResourceLocation)
        .def("convertColourValue", &Root::convertColourValue)
        .def("createRenderWindow", &Root::createRenderWindow, rir())
        .def("showDebugOverlay", &Root::showDebugOverlay)
    ;

    enum_<ResourceType>("ResourceType")
        .value("RESTYPE_ALL", RESTYPE_ALL)
        .value("RESTYPE_TEXTURES", RESTYPE_TEXTURES)
        .value("RESTYPE_MODELS", RESTYPE_MODELS)
    ;
}

void exportLight()
{
    scope in_Light(
        class_<Light, bases<MovableObject> >("Light")
            .def("getAttenuationConstant", &Light::getAttenuationConstant)
            .def("getAttenuationLinear", &Light::getAttenuationLinear)
            .def("getAttenuationQuadric", &Light::getAttenuationQuadric)
            .def("getAttenuationRange", &Light::getAttenuationRange)
            .def("getDiffuseColour", &Light::getDiffuseColour)
            .def("getDirection", &Light::getDirection)
            .def("getPosition", &Light::getPosition)
            .def("getSpecularColour", &Light::getSpecularColour)
            .def("getSpotlightFalloff", &Light::getSpotlightFalloff)
            .def("getSpotlightInnerAngle", &Light::getSpotlightInnerAngle)
            .def("getSpotlightOuterAngle", &Light::getSpotlightOuterAngle)
            .def("getType", &Light::getType)
            .def("setAttenuation", &Light::setAttenuation)
            .def("setSpotlightRange", &Light::setSpotlightRange)
            .def("setType", &Light::setType)
            .def("setPosition", (void (Light::*)(Real,Real,Real))&Light::setPosition)
            .def("setPosition", (void (Light::*)(const Vector3&))&Light::setPosition)
            .def("setDirection", (void (Light::*)(const Vector3&))&Light::setDirection)
            .def("setDirection", (void (Light::*)(Real,Real,Real))&Light::setDirection)

            .def("setDiffuseColour", (void (Light::*)(const ColourValue&))&Light::setDiffuseColour)
            .def("setDiffuseColour", (void (Light::*)(Real,Real,Real))&Light::setDiffuseColour)
            .def("setSpecularColour", (void (Light::*)(const ColourValue&))&Light::setSpecularColour)
            .def("setSpecularColour", (void (Light::*)(Real,Real,Real))&Light::setSpecularColour)
    );

    enum_<Light::LightTypes>("LightTypes")
        .value("LT_POINT", Light::LT_POINT)
        .value("LT_DIRECTIONAL", Light::LT_DIRECTIONAL)
        .value("LT_SPOTLIGHT", Light::LT_SPOTLIGHT)
    ;
}

void exportFrameListener()
{
    class_<FrameEvent>("FrameEvent");

    class_<FrameListener, PythonFrameListener, boost::noncopyable>("FrameListener")
        .def("frameEnded", &PythonFrameListener::default_frameStarted)
        .def("frameStarted", &PythonFrameListener::default_frameEnded)
    ;

    class_<ExampleFrameListener, bases<FrameListener> >(
            "ExampleFrameListener", init<RenderWindow*, Camera*>());
}

BOOST_PYTHON_MODULE(Ogre)
{
    exportEnums();
    exportGeneral();
    exportVector3();
    exportMatrix3();
    exportMatrix4();
    exportQuaternion();
    exportResource();
    exportRoot();
    exportRenderTarget();
    exportRenderWindow();
    exportCamera();
    exportViewport();
    exportColourValue();
    exportSceneManager();
    exportTextureManager();
    exportPlane();
    exportRenderSystem();
    exportMaterial();
    exportMeshManager();
    exportMesh();
    exportEntity();
    exportLight();
    exportSceneNode();
    exportFrameListener();
    exportAxisAlignedBox();
    exportSphere();
}

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
    class_<Root>("Root")
        .def("showConfigDialog", &Root::showConfigDialog)
        .def("initialise", &Root::initialise, return_internal_reference<>())
        .def("restoreConfig", &Root::restoreConfig)
        .def("saveConfig", &Root::saveConfig)
        .def("getAvailableRenderers", &Root::getAvailableRenderers,
                return_internal_reference<>())
        .def("getRenderSystem", &Root::getRenderSystem,
                return_internal_reference<>())
        .def("getSceneManager", &Root::getSceneManager,
                return_internal_reference<>())
        .def("getTextureManager", &Root::getTextureManager,
                return_internal_reference<>())
        .def("addFrameListener", &Root::addFrameListener)
        .def("removeFrameListener", &Root::removeFrameListener)
        .def("startRendering", &Root::startRendering)
        .def("addResourceLocation", &Root::addResourceLocation)
    ;
}

void exportRenderWindow()
{
    class_<RenderTarget, boost::noncopyable>("RenderTarget", no_init)
        .def("addViewport", &RenderTarget::addViewport,
                return_internal_reference<>())
        .def("getColourDepth", &RenderTarget::getColourDepth)
        .def("getHeight", &RenderTarget::getHeight)
        .def("getName", &RenderTarget::getName)
        .def("getWidth", &RenderTarget::getWidth)
        .def("getLastFPS", &RenderTarget::getLastFPS)
        .def("getAverageFPS", &RenderTarget::getAverageFPS)
        .def("getBestFPS", &RenderTarget::getBestFPS)
        .def("getWorstFPS", &RenderTarget::getWorstFPS)
        .def("removeAllViewports", &RenderTarget::removeAllViewports)
        .def("resetStatistics", &RenderTarget::resetStatistics)
        .def("setStatsDisplay", &RenderTarget::setStatsDisplay)
        .def("update", &RenderTarget::update)
    ;

    class_<RenderWindow, bases<RenderTarget>, boost::noncopyable>(
            "RenderWindow", no_init)
        .def("isActive", &RenderWindow::isActive)
        .def("isClosed", &RenderWindow::isClosed)
        .def("isFullScreen", &RenderWindow::isFullScreen)
        .def("reposition", &RenderWindow::reposition)
        .def("resize", &RenderWindow::resize)
        .def("swapBuffers", &RenderWindow::swapBuffers)
    ;
}

void exportRenderSystem()
{
    class_<RenderSystem, boost::noncopyable>("RenderSystem", no_init)
        .def("addFrameListener", &RenderSystem::addFrameListener)
        .def("createRenderWindow", &RenderSystem::createRenderWindow, rir())
        .def("destroyRenderWindow", (void (RenderSystem::*)(const String&))&RenderSystem::destroyRenderWindow)
        .def("destroyRenderWindow", (void (RenderSystem::*)(RenderWindow*))&RenderSystem::destroyRenderWindow)
        .def("getName", &RenderSystem::getName, ccr())
        .def("getRenderWindow", &RenderSystem::getRenderWindow, rir())
        .def("removeFrameListener", &RenderSystem::removeFrameListener)
        .def("setAmbientLight", &RenderSystem::setAmbientLight)
        .def("setConfigOption", &RenderSystem::setConfigOption)
        .def("setShadingType", &RenderSystem::setShadingType)
        .def("setTextureFiltering", &RenderSystem::setTextureFiltering)
        .def("startRendering", &RenderSystem::startRendering)
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
}

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

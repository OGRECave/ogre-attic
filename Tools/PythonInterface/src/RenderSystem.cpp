#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
 
#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportRenderSystem()
{
    class_<RenderSystem, boost::noncopyable>("RenderSystem", no_init)
        .def("initialise", &RenderSystem::initialise, rir())
        .def("shutdown", &RenderSystem::shutdown)
        .def("addFrameListener", &RenderSystem::addFrameListener)
        .def("removeFrameListener", &RenderSystem::removeFrameListener)
        .def("attachRenderTarget", &RenderSystem::attachRenderTarget)
        .def("getRenderTarget", &RenderSystem::getRenderTarget, rir())
        .def("detachRenderTarget", &RenderSystem::detachRenderTarget, rir())
        .def("setWaitForVerticalBlank", &RenderSystem::setWaitForVerticalBlank)
        .def("getWaitForVerticalBlank", &RenderSystem::getWaitForVerticalBlank)
        .def("setStencilBufferParams", &RenderSystem::setStencilBufferParams)
        .def("softwareVertexBlend", &RenderSystem::softwareVertexBlend)
    ;

    enum_<TexCoordCalcMethod>("TexCoordCalcMethod")
        .value("TEXCALC_NONE", TEXCALC_NONE)
        .value("TEXCALC_ENVIRONMENT_MAP", TEXCALC_ENVIRONMENT_MAP)
        .value("TEXCALC_ENVIRONMENT_MAP_PLANAR", TEXCALC_ENVIRONMENT_MAP_PLANAR)
    ;

    enum_<StencilOperation>("StencilOperation")
        .value("SOP_KEEP", SOP_KEEP)
        .value("SOP_ZERO", SOP_ZERO)
        .value("SOP_REPLACE", SOP_REPLACE)
        .value("SOP_INCREMENT", SOP_INCREMENT)
        .value("SOP_DECREMENT", SOP_DECREMENT)
        .value("SOP_INVERT", SOP_INVERT)
    ;
}

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportRenderTarget()
{
    scope in_rt(

    class_<RenderTarget, boost::noncopyable>("RenderTarget", no_init)
        .def("getName", &RenderTarget::getName)
        .def("getMetrics", &RenderTarget::getMetrics)
        .def("getWidth", &RenderTarget::getWidth)
        .def("getHeight", &RenderTarget::getHeight)
        .def("getColourDepth", &RenderTarget::getColourDepth)
        .def("update", &RenderTarget::update)
        .def("addViewport", &RenderTarget::addViewport, rir())
        .def("getNumViewports", &RenderTarget::getNumViewports)
        .def("removeViewport", &RenderTarget::removeViewport)
        .def("removeAllViewports", &RenderTarget::removeAllViewports)
        .def("setStatsDisplay", &RenderTarget::setStatsDisplay)
        .def("getStatistics", &RenderTarget::getStatistics)
        .def("getLastFPS", &RenderTarget::getLastFPS)
        .def("getAverageFPS", &RenderTarget::getAverageFPS)
        .def("getBestFPS", &RenderTarget::getBestFPS)
        .def("getWorstFPS", &RenderTarget::getWorstFPS)
        .def("resetStatistics", &RenderTarget::resetStatistics)
        .def("getCustomAttribute", &RenderTarget::getCustomAttribute)
        .def("setDebugText", &RenderTarget::setDebugText)
        .def("addListener", &RenderTarget::addListener)
        .def("removeListener", &RenderTarget::removeListener)
        .def("removeAllListeners", &RenderTarget::removeAllListeners)
        .def("isActive", &RenderTarget::isActive)
        .def("setActive", &RenderTarget::setActive)
        .def("writeContentsToFile", &RenderTarget::writeContentsToFile)

    );

    enum_<RenderTarget::StatFlags>("StatFlags")
        .value("SF_NONE", RenderTarget::SF_NONE)
        .value("SF_FPS", RenderTarget::SF_FPS)
        .value("SF_AVG_FPS", RenderTarget::SF_AVG_FPS)
        .value("SF_BEST_FPS", RenderTarget::SF_BEST_FPS)
        .value("SF_WORST_FPS", RenderTarget::SF_WORST_FPS)
        .value("SF_TRIANGLE_COUNT", RenderTarget::SF_TRIANGLE_COUNT)
        .value("SF_ALL", RenderTarget::SF_ALL)
    ;
}

void exportRenderWindow()
{

    class_<RenderWindow, bases<RenderTarget>, boost::noncopyable>(
            "RenderWindow", no_init)
        .def("update", &RenderWindow::update)
        .def("isFullScreen", &RenderWindow::isFullScreen)
        .def("getMetrics", &RenderWindow::getMetrics)
    ;
}

void exportRenderSystem()
{
    class_<RenderSystem, boost::noncopyable>("RenderSystem", no_init)
        .def("initialise", &RenderSystem::initialise, rir())
        .def("shutdown", &RenderSystem::shutdown)
        .def("addFrameListener", &RenderSystem::addFrameListener)
        .def("removeFrameListener", &RenderSystem::removeFrameListener)
        .def("startRendering", &RenderSystem::startRendering)
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

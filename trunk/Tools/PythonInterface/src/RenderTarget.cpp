#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
 
#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(RT_addViewport_stub, addViewport, 1, 6)

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
        .def("addViewport", &RenderTarget::addViewport,
            RT_addViewport_stub()[rir()])
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


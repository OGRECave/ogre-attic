#ifdef HAVE_CONFIG_H
#   include "config.h"
#   ifdef HAVE_SNPRINTF
#       undef HAVE_SNPRINTF
#   endif
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportViewport()
{
    class_<Viewport>("Viewport",init<Camera*, RenderTarget*, float, float,
            float, float, int>())
        .def("getActualHeight", &Viewport::getActualHeight)
        .def("getActualLeft", &Viewport::getActualLeft)
        .def("getActualTop", &Viewport::getActualTop)
        .def("getActualWidth", &Viewport::getActualWidth)
        .def("getBackgroundColour", &Viewport::getBackgroundColour, ccr())
        .def("getCamera", &Viewport::getCamera, rir())
        .def("getClearEveryFrame", &Viewport::getClearEveryFrame)
        .def("getHeight", &Viewport::getHeight)
        .def("getLeft", &Viewport::getLeft)
        .def("getTarget", &Viewport::getTarget, rir())
        .def("getTop", &Viewport::getTop)
        .def("getWidth", &Viewport::getWidth)
        .def("setBackgroundColour", &Viewport::setBackgroundColour)
        .def("setClearEveryFrame", &Viewport::setClearEveryFrame)
        .def("setDimensions", &Viewport::setDimensions)
        .def("update", &Viewport::update)
    ;
}

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
 
#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportRenderWindow()
{

    class_<RenderWindow, bases<RenderTarget>, boost::noncopyable>(
            "RenderWindow", no_init)
        .def("update", &RenderWindow::update)
        .def("isFullScreen", &RenderWindow::isFullScreen)
        .def("getMetrics", &RenderWindow::getMetrics)
    ;
}


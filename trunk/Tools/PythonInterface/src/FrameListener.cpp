#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
 
#include "OgrePythonLink.h"
#include "OgrePythonFrameListener.h"
#include "ExampleFrameListener.h"
 
using namespace boost::python;
using namespace Ogre;

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

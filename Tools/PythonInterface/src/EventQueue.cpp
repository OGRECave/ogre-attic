#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
  
#include "OgrePythonLink.h"
#include "OgreEventQueue.h"
  
using namespace boost::python;
using namespace Ogre;

void exportEventQueue()
{
    class_<EventQueue>("EventQueue")
        .def("push", &EventQueue::push)
        .def("pop", &EventQueue::pop, rir())
        .def("activateEventQueue", &EventQueue::activateEventQueue)
        .def("getSize", &EventQueue::getSize)
    ;
}


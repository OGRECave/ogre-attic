#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
  
#include "OgrePythonLink.h"
#include "OgreInputEvent.h"

using namespace boost::python;
using namespace Ogre;

void exportInputEvent()
{
    class_<InputEvent>("InputEvent", init<EventTarget*, int, long, int>())
        .def("consume", &InputEvent::consume)
        .def("getModifiers", &InputEvent::getModifiers)
        .def("getWhen", &InputEvent::getWhen)
        .def("isAltDown", &InputEvent::isAltDown)
        .def("isConsumed", &InputEvent::isConsumed)
        .def("isControlDown", &InputEvent::isControlDown)
        .def("isMetaDown", &InputEvent::isMetaDown)
        .def("isShiftDown", &InputEvent::isShiftDown)
        .def("isEventBetween", &InputEvent::isEventBetween)
        .def("getID", &InputEvent::getID)
        .def("getSource", &InputEvent::getSource, rir())
    ;
}


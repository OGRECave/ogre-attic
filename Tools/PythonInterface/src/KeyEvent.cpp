#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
  
#include "OgrePythonLink.h"
#include "OgreKeyEvent.h"

using namespace boost::python;
using namespace Ogre;

void exportKeyEvent()
{
    class_<KeyEvent, bases<InputEvent> >("KeyEvent",
                init<PositionTarget*, int, int, Real, int>())
        .def("paramString", &KeyEvent::paramString)
        .def("getKey", &KeyEvent::getKey)
        .def("getKeyChar", &KeyEvent::getKeyChar)
    ;
}


#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportEntity()
{
    class_<Entity, bases<MovableObject> >("Entity", no_init)
        .def("getMesh", &Entity::getMesh, rir())
        .def("getName", &Entity::getName, ccr())
        .def("setMaterialName", &Entity::setMaterialName)
    ;
}


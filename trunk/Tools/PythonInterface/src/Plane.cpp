#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportPlane()
{
    scope plane_scope(

    class_<Plane>("Plane")
        .def(init<const Vector3&, Real>())
        .def(init<const Vector3&, const Vector3&>())
        .def(init<const Vector3&, const Vector3&, const Vector3&>())

        .def_readwrite("normal", &Plane::normal)
        .def_readwrite("d", &Plane::d)

        .def("getSide", &Plane::getSide)
        .def("getDistance", &Plane::getDistance)
    );

    enum_<Plane::Side>("Side")
        .value("NO_SIDE", Plane::NO_SIDE)
        .value("POSITIVE_SIDE", Plane::POSITIVE_SIDE)
        .value("NEGATIVE_SIDE", Plane::NEGATIVE_SIDE)
    ;
}


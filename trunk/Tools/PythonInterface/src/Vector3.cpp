#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

Real Vector3_get_item(Vector3& self, int index) 
{
    if (index >= 3) 
    {
        PyErr_SetString(PyExc_IndexError, "Index too large");
        throw_error_already_set();
    }

    return self[index];
}

void exportVector3()
{
    object vec_class = class_<Vector3>("Vector3", 
            "Standard 3-dimensional vector")

        .def(init<Real, Real, Real>())

        // Member variables
        .def_readwrite("x", &Vector3::x)
        .def_readwrite("y", &Vector3::y)
        .def_readwrite("z", &Vector3::z)

        // Random access
        .def("__getitem__", &Vector3_get_item)

        // Operator overloads
        .def(self == self)
        .def(self != self)
        .def(self + self)
        .def(self - self)
        .def(self * float())
        .def(self * self)
        .def(self / float())
        .def(self += self)
        .def(self -= self)
        .def(self *= float())
        .def(self /= float())
        .def(self < self)
        .def(self > self)

        // Member functions
        .def("length", &Vector3::length)
        .def("squaredLength", &Vector3::squaredLength)
        .def("dotProduct", &Vector3::dotProduct)
        .def("normalise", &Vector3::normalise, rir())
        .def("crossProduct", &Vector3::crossProduct)
        .def("midPoint", &Vector3::midPoint)
        .def("makeFloor", &Vector3::makeFloor)
        .def("makeCeil", &Vector3::makeCeil)
        .def("perpendicular", &Vector3::perpendicular)
        .def("randomDeviant", &Vector3::randomDeviant)
        .def("getRotationTo", &Vector3::getRotationTo)
    ;

    vec_class.attr("ZERO") = Vector3::ZERO;
    vec_class.attr("UNIT_X") = Vector3::UNIT_X;
    vec_class.attr("UNIT_Y") = Vector3::UNIT_Y;
    vec_class.attr("UNIT_Z") = Vector3::UNIT_Z;
    vec_class.attr("UNIT_SCALE") = Vector3::UNIT_SCALE;
}


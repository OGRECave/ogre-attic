#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

tuple Matrix4_getitem(Matrix4& self, int index) 
{
    if (index >= 4) 
    {
        PyErr_SetString(PyExc_IndexError, "Index too large");
        throw_error_already_set();
    }

    const Real* row = self[index];

    return make_tuple(row[0], row[1], row[2], row[3]);
}

void exportMatrix4()
{
    object mat_class = class_<Matrix4>("Matrix4",
            "A 4x4 matrix which can represent rotations around axes.")
        .def("__getitem__", &Matrix4_getitem)

        .def(self * other<Vector3>())
        .def(self + self)
        .def(self == self)

        .def("concatenate", &Matrix4::concatenate)
        .def("transpose", &Matrix4::transpose)
        .def("setTrans", &Matrix4::setTrans)
        .def("makeTrans", 
                (void (Matrix4::*)(const Vector3&)) &Matrix4::makeTrans)
        .def("makeTrans", 
                (void (Matrix4::*)(Real, Real, Real)) &Matrix4::makeTrans)
        .def("setScale", &Matrix4::setScale)
        .def("extract3x3Matrix", &Matrix4::extract3x3Matrix)
    ;

    mat_class.attr("ZERO") = Matrix4::ZERO;
    mat_class.attr("IDENTITY") = Matrix4::IDENTITY;
}


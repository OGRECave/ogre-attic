#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

tuple Matrix3_getitem(Matrix3& self, int index) 
{
    if (index >= 3) 
    {
        PyErr_SetString(PyExc_IndexError, "Index too large");
        throw_error_already_set();
    }

    Real* row = self[index];

    return make_tuple(row[0], row[1], row[2]);
}

void exportMatrix3()
{
    object mat_class = class_<Matrix3>("Matrix3",
            "A 3x3 matrix which can represent rotations around axes.")
        .def(init<Real,Real,Real,Real,Real,Real,Real,Real,Real>())
        .def(init<const Matrix3&>())

        .def("__getitem__", &Matrix3_getitem)

        .def(self == self)
        .def(self != self)
        .def(self + self)
        .def(self - self)
        .def(self * self)
        .def(self - self)
        .def(-self)
        .def(self * other<Vector3>())
        .def(self * float())

        .def("Transpose", &Matrix3::Transpose)
        .def("Inverse", 
                (bool (Matrix3::*)(Matrix3&, Real) const) &Matrix3::Inverse)
        .def("Inverse", (Matrix3 (Matrix3::*)(Real) const) &Matrix3::Inverse)
        .def("Determinant", &Matrix3::Determinant)
        .def("SingularValueDecomposition", &Matrix3::SingularValueDecomposition)
        .def("SingularValueComposition", &Matrix3::SingularValueComposition)
        .def("Orthonormalize", &Matrix3::Orthonormalize)
        .def("QDUDecomposition", &Matrix3::QDUDecomposition)
        .def("SpectralNorm", &Matrix3::SpectralNorm)
        .def("ToAxisAngle", &Matrix3::ToAxisAngle)
        .def("FromAxisAngle", &Matrix3::FromAxisAngle)
        .def("ToEulerAnglesXYZ", &Matrix3::ToEulerAnglesXYZ)
        .def("ToEulerAnglesXZY", &Matrix3::ToEulerAnglesXZY)
        .def("ToEulerAnglesYXZ", &Matrix3::ToEulerAnglesYXZ)
        .def("ToEulerAnglesYZX", &Matrix3::ToEulerAnglesYZX)
        .def("ToEulerAnglesZXY", &Matrix3::ToEulerAnglesZXY)
        .def("ToEulerAnglesZYX", &Matrix3::ToEulerAnglesZYX)
        .def("FromEulerAnglesXYZ", &Matrix3::FromEulerAnglesXYZ)
        .def("FromEulerAnglesXZY", &Matrix3::FromEulerAnglesXZY)
        .def("FromEulerAnglesYXZ", &Matrix3::FromEulerAnglesYXZ)
        .def("FromEulerAnglesYZX", &Matrix3::FromEulerAnglesYZX)
        .def("FromEulerAnglesZXY", &Matrix3::FromEulerAnglesZXY)
        .def("FromEulerAnglesZYX", &Matrix3::FromEulerAnglesZYX)
    ;

    mat_class.attr("EPSILON") = Matrix3::EPSILON;
    mat_class.attr("ZERO") = Matrix3::ZERO;
    mat_class.attr("IDENTITY") = Matrix3::IDENTITY;
}


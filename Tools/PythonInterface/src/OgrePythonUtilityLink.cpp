#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"
#include "OgreSphere.h"

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
    vec_class.attr("UNIT_Z") = Vector3::UNIT_Z;
    vec_class.attr("UNIT_SCALE") = Vector3::UNIT_SCALE;
}

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

void exportQuaternion()
{
    object quat = class_<Quaternion>("Quaternion", init<Real,Real,Real,Real>())
        .def_readwrite("w", &Quaternion::w)
        .def_readwrite("x", &Quaternion::x)
        .def_readwrite("y", &Quaternion::y)
        .def_readwrite("z", &Quaternion::z)

        .def(self + self)
        .def(self - self)
        .def(self * self)
        .def(self * float())
        .def(-self)
        .def(self == self)
        .def(self * other<Vector3>())
        
        .def("FromRotationMatrix", &Quaternion::FromRotationMatrix)
        .def("ToRotationMatrix", &Quaternion::ToRotationMatrix)
        .def("FromAngleAxis", &Quaternion::FromAngleAxis)
        .def("ToAngleAxis", &Quaternion::ToAngleAxis)
        .def("FromAxes", (void (Quaternion::*)(const Vector3&, const Vector3&,
                    const Vector3&)) &Quaternion::FromAxes)
        .def("ToAxes", (void(Quaternion::*)(Vector3&, Vector3&, Vector3&)) 
                &Quaternion::ToAxes)
        .def("Dot", &Quaternion::Dot)
        .def("Norm", &Quaternion::Norm)
        .def("Inverse", &Quaternion::Inverse)
        .def("UnitInverse", &Quaternion::UnitInverse)
        .def("Exp", &Quaternion::Exp)
        .def("Log", &Quaternion::Log)
        .def("Slerp", &Quaternion::Slerp)
        .def("SlerpExtraSpins", &Quaternion::SlerpExtraSpins)
        .def("Intermediate", &Quaternion::Intermediate)
        .def("Squad", &Quaternion::Squad)
    ;

    quat.attr("ms_fEpsilon") = Quaternion::ms_fEpsilon;
    quat.attr("ZERO") = Quaternion::ZERO;
    quat.attr("IDENTITY") = Quaternion::IDENTITY;
}

void exportColourValue()
{
    object cv = class_<ColourValue>("ColourValue", init<Real,Real,Real,Real>())
        .def_readwrite("r", &ColourValue::r)
        .def_readwrite("g", &ColourValue::g)
        .def_readwrite("b", &ColourValue::b)
        .def_readwrite("a", &ColourValue::a)

        .def(self == self)
        .def(self != self)

        .def("getAsLongRGBA", &ColourValue::getAsLongRGBA)
        .def("getAsLongARGB", &ColourValue::getAsLongARGB)
        .def("getAsLongABGR", &ColourValue::getAsLongABGR)
    ;

    cv.attr("Black") = ColourValue::Black;
    cv.attr("White") = ColourValue::White;
    cv.attr("Red") = ColourValue::Red;
    cv.attr("Green") = ColourValue::Green;
    cv.attr("Blue") = ColourValue::Blue;
}

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

void exportAxisAlignedBox()
{
    class_<AxisAlignedBox>("AxisAlignedBox")
        .def(init<const Vector3&, const Vector3&>())
        .def(init<Real, Real, Real, Real, Real, Real>())
        
        .def("getMinimum", &AxisAlignedBox::getMinimum)
        .def("getMaximum", &AxisAlignedBox::getMaximum)
        .def("setMinimum", (void (AxisAlignedBox::*)(const Vector3&))&AxisAlignedBox::setMinimum)
        .def("setMinimum", (void (AxisAlignedBox::*)(Real,Real,Real))&AxisAlignedBox::setMinimum)
        .def("setMaximum", (void (AxisAlignedBox::*)(const Vector3&))&AxisAlignedBox::setMaximum)
        .def("setMaximum", (void (AxisAlignedBox::*)(Real,Real,Real))&AxisAlignedBox::setMaximum)
        .def("setExtents", 
                (void (AxisAlignedBox::*)(const Vector3&, const Vector3&))
                &AxisAlignedBox::setExtents)
        .def("setExtents", 
                (void (AxisAlignedBox::*)(Real, Real, Real, Real, Real, Real))
                &AxisAlignedBox::setExtents)
        .def("getAllCorners", &AxisAlignedBox::getAllCorners, rir())
        .def("merge", &AxisAlignedBox::merge)
        .def("transform", &AxisAlignedBox::transform)
        .def("setNull", &AxisAlignedBox::setNull)
        .def("isNull", &AxisAlignedBox::isNull)
    ;
}

void exportSphere()
{
    class_<Sphere>("Sphere")
        .def(init<const Vector3&, Real>())

        .def("getRadius", &Sphere::getRadius)
        .def("setRadius", &Sphere::setRadius)
        .def("getCenter", &Sphere::getCenter)
        .def("setCenter", &Sphere::setCenter)
    ;
}


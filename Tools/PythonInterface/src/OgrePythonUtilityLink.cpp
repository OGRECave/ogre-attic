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
    vec_class.attr("UNIT_Z") = Vector3::UNIT_Z;
    vec_class.attr("UNIT_SCALE") = Vector3::UNIT_SCALE;
}

void exportMatrix3()
{
    object mat_class = class_<Matrix3>("Matrix3",
            "A 3x3 matrix which can represent rotations around axes.")
        .def(init<Real,Real,Real,Real,Real,Real,Real,Real,Real>())
    ;
}

void exportMatrix4()
{
    object mat_class = class_<Matrix4>("Matrix4",
            "A 4x4 matrix which can represent rotations around axes.")
    ;
}

void exportQuaternion()
{
    class_<Quaternion>("Quaternion", init<Real,Real,Real,Real>())
        .def("Dot", &Quaternion::Dot)
        .def("Exp", &Quaternion::Exp)
        .def("FromAngleAxis", &Quaternion::FromAngleAxis)
        .def("FromAxes", (void (Quaternion::*)(const Vector3&, const Vector3&,
                    const Vector3&)) &Quaternion::FromAxes)
        .def("FromRotationMatrix", &Quaternion::FromRotationMatrix)
        .def("Intermediate", &Quaternion::Intermediate)
        .def("Inverse", &Quaternion::Inverse)
        .def("Log", &Quaternion::Log)
        .def("Norm", &Quaternion::Norm)
        .def("Slerp", &Quaternion::Slerp)
        .def("SlerpExtraSpins", &Quaternion::SlerpExtraSpins)
        .def("Squad", &Quaternion::Squad)
        .def("ToAxes", (void(Quaternion::*)(Vector3&, Vector3&, Vector3&)) 
                &Quaternion::ToAxes)
        .def("ToRotationMatrix", &Quaternion::ToRotationMatrix)
        .def("UnitInverse", &Quaternion::UnitInverse)
    ;
}

void exportColourValue()
{
    class_<ColourValue>("ColourValue", init<Real,Real,Real,Real>())
        .def("getAsLongRGBA", &ColourValue::getAsLongRGBA)
    ;
}

void exportPlane()
{
    class_<Plane>("Plane")
        .def("getDistance", &Plane::getDistance)
        .def("getSide", &Plane::getSide)
    ;
}

void exportAxisAlignedBox()
{
    class_<AxisAlignedBox>("AxisAlignedBox")
        .def("getAllCorners", &AxisAlignedBox::getAllCorners, rir())
        .def("getMaximum", &AxisAlignedBox::getMaximum)
        .def("getMinimum", &AxisAlignedBox::getMinimum)
        .def("setMinimum", (void (AxisAlignedBox::*)(Real,Real,Real))&AxisAlignedBox::setMinimum)
        .def("setMinimum", (void (AxisAlignedBox::*)(const Vector3&))&AxisAlignedBox::setMinimum)
        .def("setMaximum", (void (AxisAlignedBox::*)(Real,Real,Real))&AxisAlignedBox::setMaximum)
        .def("setMaximum", (void (AxisAlignedBox::*)(const Vector3&))&AxisAlignedBox::setMaximum)
    ;
}

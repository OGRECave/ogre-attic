#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

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


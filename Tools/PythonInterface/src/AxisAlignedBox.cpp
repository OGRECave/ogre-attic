#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

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


#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"
#include "OgreSphere.h"

using namespace boost::python;
using namespace Ogre;

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


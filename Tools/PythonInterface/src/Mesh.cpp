#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportMesh()
{
    class_<Mesh>("Mesh", init<std::string>())
        .def("getNumSubMeshes", &Mesh::getNumSubMeshes)
        .def("getSubMesh", &Mesh::getSubMesh, rir())
        .def("getName", &Mesh::getName, ccr())
        .def("load", &Mesh::load)
        .def("unload", &Mesh::unload)
    ;
}


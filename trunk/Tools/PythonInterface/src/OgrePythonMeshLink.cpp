#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportMeshManager()
{
    class_<MeshManager>("MeshManager")
        .def("create", &MeshManager::create, rir())
        .def("load", &MeshManager::load, rir())
        .def("getByName", &MeshManager::getByName, rir())
    ;
}

void exportMesh()
{
    class_<Mesh>("Mesh", init<std::string>())
        .def("getNumSubMeshes", &Mesh::getNumSubMeshes)
        .def("getSubMesh", &Mesh::getSubMesh, rir())
        .def("getName", &Mesh::getName, ccr())
        .def("load", &Mesh::load)
        .def("unload", &Mesh::unload)
    ;

    class_<SubMesh>("SubMesh");
}

void exportEntity()
{
    class_<MovableObject, boost::noncopyable>("MovableObject", no_init);

    class_<Entity, bases<MovableObject> >("Entity", no_init)
        .def("getMesh", &Entity::getMesh, rir())
        .def("getName", &Entity::getName, ccr())
    ;
}



#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(MeshManager_load_overloads, MeshManager::load, 1, 2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(MeshManager_createPlane_overloads,
MeshManager::createPlane, 4, 11)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(MeshManager_createCurvedPlane_overloads,
MeshManager::createCurvedPlane, 4, 12)

void exportMeshManager()
{
    class_<MeshManager, bases<ResourceManager> >("MeshManager")
        .def("load", &MeshManager::load, MeshManager_load_overloads()[rir()])
        .def("create", &MeshManager::create, rir())
        .def("createManual", &MeshManager::createManual, rir())
        .def("createPlane", &MeshManager::createPlane,
            MeshManager_createPlane_overloads()[rir()])
        .def("createCurvedPlane", &MeshManager::createCurvedPlane,
            MeshManager_createCurvedPlane_overloads()[rir()])
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



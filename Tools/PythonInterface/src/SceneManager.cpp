#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportSceneManager()
{
    class_<SceneManager>("SceneManager")
        .def("addMaterial", &SceneManager::addMaterial)
        .def("clearScene", &SceneManager::clearScene, rir())
        .def("createCamera", &SceneManager::createCamera, rir())
        .def("createEntity", (Entity* (SceneManager::*)(const String&,
                        SceneManager::PrefabType))&SceneManager::createEntity,
                rir())
        .def("createEntity", (Entity* (SceneManager::*)(const String&, const
                        String&))&SceneManager::createEntity, rir())
        .def("createLight", &SceneManager::createLight, rir())
        .def("createMaterial", &SceneManager::createMaterial, rir())
        .def("createSceneNode", (SceneNode*
                    (SceneManager::*)(void))&SceneManager::createSceneNode,
                rir())
        .def("createSceneNode", (SceneNode* (SceneManager::*)(const
                        String&))&SceneManager::createSceneNode, rir())
        .def("getAmbientLight", &SceneManager::getAmbientLight)
        .def("getCamera", &SceneManager::getCamera, rir())
        .def("getDefaultMaterialSettings",
                &SceneManager::getDefaultMaterialSettings, rir())
        .def("getEntity", &SceneManager::getEntity, rir())
        .def("getLight", &SceneManager::getLight, rir())
        .def("getMaterial", (Material* (SceneManager::*)(const
                        String&))&SceneManager::getMaterial, rir())
        .def("getMaterial", (Material*
                    (SceneManager::*)(int))&SceneManager::getMaterial, rir())
        .def("getRootSceneNode", &SceneManager::getRootSceneNode, rir())
        .def("getSuggestedViewpoint", &SceneManager::getSuggestedViewpoint)
        .def("removeAllCameras", &SceneManager::removeAllCameras)
        .def("removeAllEntities", &SceneManager::removeAllEntities)
        .def("removeAllLights", &SceneManager::removeAllLights)
        .def("removeCamera", (void (SceneManager::*)(Camera*))&SceneManager::removeCamera)
        .def("removeCamera", (void (SceneManager::*)(const String&))&SceneManager::removeCamera)
        .def("removeEntity", (void (SceneManager::*)(Entity*))&SceneManager::removeEntity)
        .def("removeEntity", (void (SceneManager::*)(const String&))&SceneManager::removeEntity)
        .def("removeLight", (void (SceneManager::*)(Light*))&SceneManager::removeLight)
        .def("removeLight", (void (SceneManager::*)(const String&))&SceneManager::removeLight)
        .def("setAmbientLight", &SceneManager::setAmbientLight)
        .def("setSkyBox", &SceneManager::setSkyBox)
        .def("setSkyPlane", &SceneManager::setSkyPlane)
        .def("setSkyDome", &SceneManager::setSkyDome)
        .def("setWorldGeometry", &SceneManager::setWorldGeometry)
        .def("setFog", &SceneManager::setFog)
    ;
}


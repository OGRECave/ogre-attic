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
    ;
}

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(SceneNode_createChild_stubs, createChild,1,3)

void exportSceneNode()
{
    class_<SceneNode>("SceneNode", init<SceneManager*>())
        .def("createChild", (SceneNode* (SceneNode::*)(const String&, const
                        Vector3&, const Quaternion&))&SceneNode::createChild,
                SceneNode_createChild_stubs()[rir()])
        .def("attachCamera", &SceneNode::attachCamera)
        .def("attachObject", &SceneNode::attachObject)
        .def("attachLight", &SceneNode::attachLight)
        .def("detachObject", (MovableObject* (SceneNode::*)(const String&))
                &SceneNode::detachObject, rir())
        .def("getAttachedObject", (MovableObject* (SceneNode::*)(const String&))
                &SceneNode::getAttachedObject, rir())
        .def("getChild", (SceneNode* (SceneNode::*)(const String&) const)
                &SceneNode::getChild, rir())
        .def("getLocalAxes", &SceneNode::getLocalAxes)
        .def("getOrientation", &SceneNode::getOrientation, ccr())
        .def("getPosition", &SceneNode::getPosition, ccr())
        .def("numAttachedObjects", &SceneNode::numAttachedObjects)
        .def("numChildren", &SceneNode::numChildren)
        .def("pitch", &SceneNode::pitch)
        .def("removeChild",(SceneNode* (SceneNode::*)(const String&))
                &SceneNode::removeChild, rir())
        .def("resetOrientation", &SceneNode::resetOrientation)
        .def("roll", &SceneNode::roll)
        .def("rotate", (void (SceneNode::*)(const Vector3&,Real))&SceneNode::rotate)
        .def("rotate", (void (SceneNode::*)(const Quaternion&))&SceneNode::rotate)
        .def("setOrientation", &SceneNode::setOrientation)
        .def("setPosition", (void (SceneNode::*)(const Vector3&))&SceneNode::setPosition)
        .def("setPosition", (void (SceneNode::*)(Real,Real,Real))&SceneNode::setPosition)
        .def("translate", (void (SceneNode::*)(const Vector3&))&SceneNode::translate)
        .def("translate", (void (SceneNode::*)(Real,Real,Real))&SceneNode::translate)
        .def("translate", (void (SceneNode::*)(const Matrix3&, const Vector3&))&SceneNode::translate)
        .def("translate", (void (SceneNode::*)(const Matrix3&, Real,Real,Real))&SceneNode::translate)
        .def("yaw", &SceneNode::yaw)
    ;
}


#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

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


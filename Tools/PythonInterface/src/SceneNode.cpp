#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

SceneNode* SceneNodeCast(Node* node)
{ 
    return static_cast<SceneNode*>(node); 
}

void exportSceneNode()
{
    class_<SceneNode, bases<Node> >("SceneNode", init<SceneManager*>())
        .def(init<SceneManager*, const String&>())
        .def("attachObject", &SceneNode::attachObject)
        .def("numAttachedObjects", &SceneNode::numAttachedObjects)
        .def("getAttachedObject", 
                (MovableObject* (SceneNode::*) (const String&)) 
                &SceneNode::getAttachedObject, rir())
        .def("getAttachedObject", 
                (MovableObject* (SceneNode::*) (unsigned short)) 
                &SceneNode::getAttachedObject, rir())
        .def("detachObject", (MovableObject* (SceneNode::*) (unsigned short)) 
                    &SceneNode::detachObject, rir())
        .def("detachObject", (MovableObject* (SceneNode::*) (const String&))
                    &SceneNode::detachObject, rir())
        .def("detachAllObjects", &SceneNode::detachAllObjects)
        // XXX getAttachedObjectIterator
        .def("getCreator", &SceneNode::getCreator, rir())
        .def("removeAndDestroyChild", (void (SceneNode::*) (const String&)) 
                &SceneNode::removeAndDestroyChild)
        .def("removeAndDestroyChild", (void (SceneNode::*) (unsigned short)) 
                &SceneNode::removeAndDestroyChild)
        .def("removeAndDestroyAllChildren", &SceneNode::removeAndDestroyAllChildren)
        .def("showBoundingBox", &SceneNode::showBoundingBox)
        .def("getShowBoundingBox", &SceneNode::getShowBoundingBox)
    ;

    def("castNodeToSceneNode", SceneNodeCast, rir());
}


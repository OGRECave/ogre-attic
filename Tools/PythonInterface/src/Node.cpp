#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

struct NodeWrap : Node
{
    NodeWrap(PyObject* self) : _self(self)
    { }
    NodeWrap(PyObject* self, const String& name) : _self(self)
    { }
    Node* createChildImpl(void)
    { return call_method<Node*>(_self, "createChildImpl"); }
    Node* createChildImpl(const String& name)
    { return call_method<Node*>(_self, "createChildImpl", name); }
    PyObject* _self;
};

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Node_createChild2_stubs, createChild,0,2)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Node_createChild3_stubs, createChild,1,3)

void exportNode()
{
    class_<Node, NodeWrap, boost::noncopyable>("Node",  init<>())
        .def(init<const String&>())
        .def("getName", &Node::getName, ccr())
        .def("getParent", &Node::getParent, rir())
        .def("getOrientation", &Node::getOrientation, ccr())
        .def("setOrientation", (void (Node::*) (const Quaternion&)) 
            &Node::setOrientation)
        .def("setOrientation", (void (Node::*) (Real, Real, Real, Real)) 
            &Node::setOrientation)
        .def("resetOrientation", &Node::resetOrientation)
        .def("setPosition", (void (Node::*) (const Vector3&)) 
            &Node::setPosition)
        .def("setPosition", (void (Node::*) (Real, Real, Real)) 
            &Node::setPosition)
        .def("getPosition", &Node::getPosition, ccr())
        .def("setScale", (void (Node::*) (const Vector3&)) &Node::setScale)
        .def("setScale", (void (Node::*) (Real, Real, Real)) &Node::setScale)
        .def("getScale", &Node::getScale, ccr())
        .def("setInheritScale", &Node::setInheritScale)
        .def("getInheritScale", &Node::getInheritScale)
        .def("scale", (void (Node::*) (const Vector3&)) &Node::scale)
        .def("scale", (void (Node::*) (Real, Real, Real)) &Node::scale)
        .def("translate", (void (Node::*) (const Vector3&)) &Node::translate)
        .def("translate", (void (Node::*) (Real, Real, Real)) &Node::translate)
        .def("translate", (void (Node::*) (const Matrix3&, const Vector3&)) 
            &Node::translate)
        .def("translate", (void (Node::*) (const Matrix3&, Real, Real, Real)) 
            &Node::translate)
        .def("roll", &Node::roll)
        .def("pitch", &Node::pitch)
        .def("yaw", &Node::yaw)
        .def("rotate", (void (Node::*) (const Vector3&, Real)) &Node::rotate)
        .def("rotate", (void (Node::*) (const Quaternion&)) &Node::rotate)
        .def("getLocalAxes", &Node::getLocalAxes)
        .def("createChild", (Node* (Node::*) (const Vector3&, 
            const Quaternion&)) &Node::createChild,
            Node_createChild2_stubs()[rir()])
        .def("createChild", (Node* (Node::*) (const String&, const Vector3&, 
            const Quaternion&)) &Node::createChild,
            Node_createChild3_stubs()[rir()])
        .def("addChild", &Node::addChild)
        .def("numChildren", &Node::numChildren)
        .def("getChild", (Node* (Node::*) (unsigned short) const) 
                &Node::getChild, rir())
        .def("getChild", (Node* (Node::*) (const String&) const) 
                &Node::getChild, rir())
        // XXX getChildIterator support
        .def("removeChild", (Node* (Node::*) (unsigned short)) 
            &Node::removeChild, rir())
        .def("removeChild", (Node* (Node::*) (const String&)) 
            &Node::removeChild, rir())
        .def("removeAllChildren", &Node::removeAllChildren)
        .def("getMaterial", &Node::getMaterial, rir())
        .def("getRenderOperation", &Node::getRenderOperation)
        .def("getWorldTransforms", &Node::getWorldTransforms)
        .def("setInitialState", &Node::setInitialState)
        .def("resetToInitialState", &Node::resetToInitialState)
        .def("getInitialPosition", &Node::getInitialPosition, ccr())
        .def("getInitialOrientation", &Node::getInitialOrientation, ccr())
        .def("getInitialScale", &Node::getInitialScale, ccr())
        .def("getSquaredViewDepth", &Node::getSquaredViewDepth)
        .def("needUpdate", &Node::needUpdate)
        .def("requestUpdate", &Node::requestUpdate)
        .def("cancelUpdate", &Node::cancelUpdate)
    ;
}


#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
  
#include "OgrePythonLink.h"
#include "OgrePositionTarget.h"

using namespace boost::python;
using namespace Ogre;

class pythonPositionTarget : public PositionTarget
{
public:
    pythonPositionTarget(PyObject* self_) : self(self_)
    { }

    Real getLeft(void)
    { return call_method<Real>(self, "getLeft"); }
    Real getTop(void)
    { return call_method<Real>(self, "getTop"); }
    PositionTarget* getPositionTargetParent()
    { return call_method<PositionTarget*>(self, "getPositionTargetParent"); }
    bool isKeyEnabled()
    { return call_method<bool>(self, "isKeyEnabled"); }
private:
    PyObject* self;
};

void exportPositionTarget()
{
    class_<PositionTarget, bases<EventTarget>, boost::noncopyable>("PositionTarget", no_init);
}

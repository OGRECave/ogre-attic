#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
  
#include "OgrePythonLink.h"
#include "OgreEventTarget.h"

using namespace boost::python;
using namespace Ogre;

class pythonEventTarget : public EventTarget
{
public:
    pythonEventTarget(PyObject* self_) : self(self_)
    { }

    void processEvent(InputEvent* e)
    { return call_method<void>(self, "processEvent", e); }
private:
    PyObject* self;
};

void exportEventTarget()
{
    class_<EventTarget, pythonEventTarget, boost::noncopyable>("EventTarget");
}

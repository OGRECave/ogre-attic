#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
  
#include "OgrePythonLink.h"
#include "OgreEventListeners.h"
  
using namespace boost::python;
using namespace Ogre;

class pythonMouseMotionListener : public MouseMotionListener
{
public:
    pythonMouseMotionListener(PyObject* self_) : self(self_)
    { }

    void mouseMoved(MouseEvent* e)
    { return call_method<void>(self, "mouseMoved", e); }
    void mouseDragged(MouseEvent* e)
    { return call_method<void>(self, "mouseDragged", e); }
private:
    PyObject* self;
};

void exportMouseMotionListener()
{
    class_<MouseMotionListener, pythonMouseMotionListener, boost::noncopyable>(
        "MouseMotionListener", no_init);
}


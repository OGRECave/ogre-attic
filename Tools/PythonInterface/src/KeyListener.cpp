#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"
#include "OgreKeyEvent.h"
#include "OgreEventListeners.h"

using namespace boost::python;
using namespace Ogre;

class pythonKeyListener : public KeyListener
{
public:
    pythonKeyListener(PyObject* self_) : self(self_)
    { }

    void keyClicked(KeyEvent* e)
    { return call_method<void>(self, "keyClicked", e); }
    void keyPressed(KeyEvent* e)
    { return call_method<void>(self, "keyPressed", e); }
    void keyReleased(KeyEvent* e)
    { return call_method<void>(self, "keyReleased", e); }
private:
    PyObject* self;
};

void exportKeyListener()
{
    class_<KeyListener, pythonKeyListener, boost::noncopyable>("KeyListener");
}


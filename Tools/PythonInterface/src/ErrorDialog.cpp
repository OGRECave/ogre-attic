#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
  
#include "OgrePythonLink.h"
#include "OgreErrorDialog.h"
  
using namespace boost::python;
using namespace Ogre;

class pythonErrorDialog : public ErrorDialog
{
public:
    pythonErrorDialog(PyObject* self_) : self(self_)
    { }
    void display(String errorMessage, String logName = "")
    { return call_method<void>(self, "display", errorMessage, logName); }
private:
    PyObject* self;
};

void exportErrorDialog()
{
    class_<ErrorDialog, pythonErrorDialog, boost::noncopyable>("ErrorDialog",
                                                               no_init);
}

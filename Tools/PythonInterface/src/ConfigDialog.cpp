#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
  
#include "OgrePythonLink.h"
#include "OgreConfigDialog.h"
  
using namespace boost::python;
using namespace Ogre;

class pythonConfigDialog : public ConfigDialog
{
public:
    pythonConfigDialog(PyObject* self_) : self(self_)
    { }
    bool display(void)
    { return call_method<bool>(self, "display"); }
private:
    PyObject* self;
};

void exportConfigDialog()
{
    class_<ConfigDialog, pythonConfigDialog, boost::noncopyable>(
            "ConfigDialog", no_init);
}

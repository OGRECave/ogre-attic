#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

struct Resource_Wrap : Resource
{
    Resource_Wrap(PyObject* self) : _self(self) 
    { }
    virtual void load() { call_method<void>(_self, "load"); }
    PyObject* _self;
};

void exportResource()
{
    class_<Resource, boost::noncopyable, boost::shared_ptr<Resource_Wrap> >("Resource", no_init)
        .def("getName", &Resource::getName, ccr())
        .def("getLastAccess", &Resource::getLastAccess)
        .def("getSize", &Resource::getSize)
        .def("unload", &Resource::unload)
        .def("touch", &Resource::touch)
    ;
}


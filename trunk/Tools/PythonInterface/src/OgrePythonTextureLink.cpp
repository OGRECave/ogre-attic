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

void exportTextureManager()
{
    class_<ResourceManager, boost::noncopyable>("ResourceManager", no_init)
        .def("getByName", &ResourceManager::getByName, rir())
    ;

    class_<TextureManager, bases<ResourceManager>, boost::noncopyable >(
            "TextureManager", no_init)
        .def("getByName", &TextureManager::getByName, rir())
        .def("load", &TextureManager::load, rir())
        .def("unload", &TextureManager::unload)
        .def("setDefaultNumMipMaps", &TextureManager::setDefaultNumMipMaps)
        .def("getDefaultNumMipMaps", &TextureManager::getDefaultNumMipMaps)
        .def("enable32BitTextures", &TextureManager::enable32BitTextures)
    ;

    class_<Texture, boost::noncopyable>("Texture", no_init);
}


#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportTextureManager()
{
    class_<TextureManager, bases<ResourceManager>, boost::noncopyable >(
            "TextureManager", no_init)
        .def("getByName", &TextureManager::getByName, rir())
        .def("load", 
            (Texture* (TextureManager::*)(const String&, int, Real, int))
            &TextureManager::load, rir())
        .def("loadImage", &TextureManager::loadImage, rir())
        .def("load", (void (TextureManager::*)(Resource*, int))
            &TextureManager::load, rir())
        .def("unload", &TextureManager::unload)
        .def("setDefaultNumMipMaps", &TextureManager::setDefaultNumMipMaps)
        .def("getDefaultNumMipMaps", &TextureManager::getDefaultNumMipMaps)
        .def("enable32BitTextures", &TextureManager::enable32BitTextures)
    ;
}


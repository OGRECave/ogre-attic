#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportResourceManager()
{
    class_<ResourceManager, boost::noncopyable>("ResourceManager", no_init)
        .def("getByName", &ResourceManager::getByName, rir())
    ;
}

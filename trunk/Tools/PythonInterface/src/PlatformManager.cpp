#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
 
#include "OgrePythonLink.h"
#include "OgrePlatformManager.h"
 
using namespace boost::python;
using namespace Ogre;

void exportPlatformManager()
{
    class_<PlatformManager>("PlatformManager")
        .def("createConfigDialog", &PlatformManager::createConfigDialog, rir())
        .def("destroyConfigDialog", &PlatformManager::destroyConfigDialog)
        .def("createErrorDialog", &PlatformManager::createErrorDialog, rir())
        .def("destroyErrorDialog", &PlatformManager::destroyErrorDialog)
        .def("createInputReader", &PlatformManager::createInputReader, rir())
        .def("destroyInputReader", &PlatformManager::destroyInputReader)
        .def("createTimer", &PlatformManager::createTimer, rir())
        .def("destroyTimer", &PlatformManager::destroyTimer)
        .def("getSingleton", &PlatformManager::getSingleton, 
            return_value_policy<reference_existing_object>())
        .staticmethod("getSingleton")
    ;
}

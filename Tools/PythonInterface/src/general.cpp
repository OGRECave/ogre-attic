#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportEnums()
{
    enum_<SceneType>("SceneType")
        .value("ST_GENERIC", ST_GENERIC)
        .value("ST_EXTERIOR_CLOSE", ST_EXTERIOR_CLOSE)
        .value("ST_EXTERIOR_FAR", ST_EXTERIOR_FAR)
        .value("ST_INTERIOR", ST_INTERIOR)
    ;

    enum_<LayerBlendOperation>("LayerBlendOperation")
        .value("LBO_REPLACE", LBO_REPLACE)
        .value("LBO_ADD", LBO_ADD)
        .value("LBO_MODULATE", LBO_MODULATE)
        .value("LBO_ALPHA_BLEND", LBO_ALPHA_BLEND)
    ;
}

void exportGeneral()
{
    // These are some of the simple types in the system
    implicitly_convertible<String, std::string>();
    implicitly_convertible<std::string, String>();

    class_<ConfigOption>("ConfigOption", no_init)
        .def_readwrite("name", &ConfigOption::name)
        .def_readwrite("currentValue", &ConfigOption::currentValue)
        .def_readwrite("possibleValues", &ConfigOption::possibleValues)
        .def_readwrite("immutable", &ConfigOption::immutable)
    ;

    
}


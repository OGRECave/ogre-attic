#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif
 
#include "OgrePythonLink.h"
#include "OgrePythonFrameListener.h"
#include "ExampleFrameListener.h"
 
using namespace boost::python;
using namespace Ogre;

void exportLight()
{
    scope in_Light(
        class_<Light, bases<MovableObject> >("Light")
            .def("getAttenuationConstant", &Light::getAttenuationConstant)
            .def("getAttenuationLinear", &Light::getAttenuationLinear)
            .def("getAttenuationQuadric", &Light::getAttenuationQuadric)
            .def("getAttenuationRange", &Light::getAttenuationRange)
            .def("getDiffuseColour", &Light::getDiffuseColour)
            .def("getDirection", &Light::getDirection)
            .def("getPosition", &Light::getPosition)
            .def("getSpecularColour", &Light::getSpecularColour)
            .def("getSpotlightFalloff", &Light::getSpotlightFalloff)
            .def("getSpotlightInnerAngle", &Light::getSpotlightInnerAngle)
            .def("getSpotlightOuterAngle", &Light::getSpotlightOuterAngle)
            .def("getType", &Light::getType)
            .def("setAttenuation", &Light::setAttenuation)
            .def("setSpotlightRange", &Light::setSpotlightRange)
            .def("setType", &Light::setType)
            .def("setPosition", (void (Light::*)(Real,Real,Real))&Light::setPosition)
            .def("setPosition", (void (Light::*)(const Vector3&))&Light::setPosition)
            .def("setDirection", (void (Light::*)(const Vector3&))&Light::setDirection)
            .def("setDirection", (void (Light::*)(Real,Real,Real))&Light::setDirection)

            .def("setDiffuseColour", (void (Light::*)(const ColourValue&))&Light::setDiffuseColour)
            .def("setDiffuseColour", (void (Light::*)(Real,Real,Real))&Light::setDiffuseColour)
            .def("setSpecularColour", (void (Light::*)(const ColourValue&))&Light::setSpecularColour)
            .def("setSpecularColour", (void (Light::*)(Real,Real,Real))&Light::setSpecularColour)
    );

    enum_<Light::LightTypes>("LightTypes")
        .value("LT_POINT", Light::LT_POINT)
        .value("LT_DIRECTIONAL", Light::LT_DIRECTIONAL)
        .value("LT_SPOTLIGHT", Light::LT_SPOTLIGHT)
    ;
}


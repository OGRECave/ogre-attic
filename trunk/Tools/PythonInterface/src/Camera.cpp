#ifdef HAVE_CONFIG_H
#   include "config.h"
#   ifdef HAVE_SNPRINTF
#       undef HAVE_SNPRINTF
#   endif
#endif

#include "OgrePythonLink.h"
#include "OgreSphere.h"

using namespace boost::python;
using namespace Ogre;

void exportCamera()
{
    class_<Camera>("Camera", init<std::string, SceneManager*>())
        .def("getSceneManager", &Camera::getSceneManager,
                return_internal_reference<>())
        .def("getName", &Camera::getName, ccr())
        .def("setProjectionType", &Camera::setProjectionType)
        .def("getProjectionType", &Camera::getProjectionType)
        .def("setDetailLevel", &Camera::setDetailLevel)
        .def("getDetailLevel", &Camera::getDetailLevel)
        .def("setPosition",  (void (Camera::*)(Real, Real, Real))
            &Camera::setPosition)
        .def("setPosition",  (void (Camera::*)(const Vector3&)) 
            &Camera::setPosition)
        .def("getPosition", &Camera::getPosition, ccr())
        .def("move", &Camera::move)
        .def("moveRelative", &Camera::moveRelative)
        .def("setDirection", (void (Camera::*) (Real, Real, Real))&Camera::setDirection)
        .def("setDirection", (void (Camera::*) (const Vector3&))&Camera::setDirection)
        .def("getDirection", &Camera::getDirection)
        .def("lookAt", (void (Camera::*)(Real,Real,Real))&Camera::lookAt)
        .def("lookAt", (void (Camera::*)(const Vector3&))&Camera::lookAt)
        .def("roll", &Camera::roll)
        .def("yaw", &Camera::yaw)
        .def("pitch", &Camera::pitch)
        .def("rotate", (void (Camera::*)(const Vector3&,Real))&Camera::rotate)
        .def("rotate", (void (Camera::*)(const Quaternion&))&Camera::rotate)
        .def("setFixedYawAxis", &Camera::setFixedYawAxis)
        .def("setFOVy", &Camera::setFOVy)
        .def("getFOVy", &Camera::getFOVy)
        .def("setNearClipDistance", &Camera::setNearClipDistance)
        .def("getNearClipDistance", &Camera::getNearClipDistance)
        .def("setFarClipDistance", &Camera::setFarClipDistance)
        .def("getFarClipDistance", &Camera::getFarClipDistance)
        .def("setAspectRatio", &Camera::setAspectRatio)
        .def("getAspectRatio", &Camera::getAspectRatio)
        .def("getProjectionMatrix", &Camera::getProjectionMatrix, rir())
        .def("getViewMatrix", &Camera::getViewMatrix, rir())
        .def("getFrustumPlane", &Camera::getFrustumPlane, rir())
        .def("isVisible", 
                (bool (Camera::*)(const AxisAlignedBox&, FrustumPlane*))
                 &Camera::isVisible)
        .def("isVisible", 
                (bool (Camera::*)(const Sphere&, FrustumPlane*))
                 &Camera::isVisible)
        .def("isVisible", 
                (bool (Camera::*)(const Vector3&, FrustumPlane*))
                 &Camera::isVisible)
        .def("getOrientation", &Camera::getOrientation, rir())
        .def("setOrientation", &Camera::setOrientation)
        .def("getDerivedOrientation", &Camera::getDerivedOrientation)
        .def("getDerivedPosition", &Camera::getDerivedPosition)
        .def("getDerivedDirection", &Camera::getDerivedDirection)
        .def("getMovableType", &Camera::getMovableType)
        .def("setAutoTracking", &Camera::setAutoTracking)
        .def("setLodBias", &Camera::setLodBias)
        .def("getLodBias", &Camera::getLodBias)
    ;

    enum_<ProjectionType>("ProjectionType")
        .value("PT_ORTHOGRAPHIC", PT_ORTHOGRAPHIC)
        .value("PT_PERSPECTIVE", PT_PERSPECTIVE)
    ;

    enum_<FrustumPlane>("FrustumPlane")
        .value("FRUSTUM_PLANE_NEAR", FRUSTUM_PLANE_NEAR)
        .value("FRUSTUM_PLANE_FAR", FRUSTUM_PLANE_FAR)
        .value("FRUSTUM_PLANE_LEFT", FRUSTUM_PLANE_LEFT)
        .value("FRUSTUM_PLANE_RIGHT", FRUSTUM_PLANE_RIGHT)
        .value("FRUSTUM_PLANE_TOP", FRUSTUM_PLANE_TOP)
        .value("FRUSTUM_PLANE_BOTTOM", FRUSTUM_PLANE_TOP)
    ;
}

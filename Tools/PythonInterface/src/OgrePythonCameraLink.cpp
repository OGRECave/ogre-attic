#ifdef HAVE_CONFIG_H
#   include "config.h"
#   ifdef HAVE_SNPRINTF
#       undef HAVE_SNPRINTF
#   endif
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportCamera()
{
    class_<Camera>("Camera", init<std::string, SceneManager*>())
        .def("getAspectRatio", &Camera::getAspectRatio)
        .def("getDetailLevel", &Camera::getDetailLevel)
        .def("getDirection", &Camera::getDirection)
        .def("getFarClipDistance", &Camera::getFarClipDistance)
        .def("getFOVy", &Camera::getFOVy)
        .def("getFrustumPlane", &Camera::getFrustumPlane, ccr())
        .def("getName", &Camera::getName, ccr())
        .def("getNearClipDistance", &Camera::getNearClipDistance)
        .def("getPosition", &Camera::getPosition, ccr())
        .def("getProjectionMatrix", &Camera::getProjectionMatrix, ccr())
        .def("getProjectionType", &Camera::getProjectionType)
        .def("getSceneManager", &Camera::getSceneManager,
                return_internal_reference<>())
        .def("getViewMatrix", &Camera::getViewMatrix, ccr())
        .def("isAttached", &Camera::isAttached)
        .def("lookAt", (void (Camera::*)(Real,Real,Real))&Camera::lookAt)
        .def("lookAt", (void (Camera::*)(const Vector3&))&Camera::lookAt)
        .def("move", &Camera::move)
        .def("pitch", &Camera::pitch)
        .def("roll", &Camera::roll)
        .def("rotate", (void (Camera::*)(const Vector3&,Real))&Camera::rotate)
        .def("rotate", (void (Camera::*)(const Quaternion&))&Camera::rotate)
        .def("setAspectRatio", &Camera::setAspectRatio)
        .def("setDetailLevel", &Camera::setDetailLevel)
        .def("setDirection", (void (Camera::*) (Real, Real, Real))&Camera::setDirection)
        .def("setDirection", (void (Camera::*) (const Vector3&))&Camera::setDirection)
        .def("setFarClipDistance", &Camera::setFarClipDistance)
        .def("setFixedYawAxis", &Camera::setFixedYawAxis)
        .def("setFOVy", &Camera::setFOVy)
        .def("setNearClipDistance", &Camera::setNearClipDistance)
        .def("setPosition", (void (Camera::*)(Real,Real,Real))&Camera::setPosition)
        .def("setPosition", (void (Camera::*) (const Vector3&))&Camera::setPosition)
        .def("setProjectionType", &Camera::setProjectionType)
        .def("yaw", &Camera::yaw)
    ;
}

void exportViewport()
{
    class_<Viewport>("Viewport",init<Camera*, RenderTarget*, float, float,
            float, float, int>())
        .def("getActualHeight", &Viewport::getActualHeight)
        .def("getActualLeft", &Viewport::getActualLeft)
        .def("getActualTop", &Viewport::getActualTop)
        .def("getActualWidth", &Viewport::getActualWidth)
        .def("getBackgroundColour", &Viewport::getBackgroundColour, ccr())
        .def("getCamera", &Viewport::getCamera, rir())
        .def("getClearEveryFrame", &Viewport::getClearEveryFrame)
        .def("getHeight", &Viewport::getHeight)
        .def("getLeft", &Viewport::getLeft)
        .def("getTarget", &Viewport::getTarget, rir())
        .def("getTop", &Viewport::getTop)
        .def("getWidth", &Viewport::getWidth)
        .def("setBackgroundColour", &Viewport::setBackgroundColour)
        .def("setClearEveryFrame", &Viewport::setClearEveryFrame)
        .def("setDimensions", &Viewport::setDimensions)
        .def("update", &Viewport::update)
    ;
}


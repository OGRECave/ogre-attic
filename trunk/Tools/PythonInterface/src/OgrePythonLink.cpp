/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

  Copyright © 2000-2002 The OGRE Team
  Also see acknowledgements in Readme.html

    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

      This program is distributed in the hope that it will be useful, but WITHOUT
      ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
      FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License along with
        this program; if not, write to the Free Software Foundation, Inc., 59 Temple
        Place - Suite 330, Boston, MA 02111-1307, USA, or go to
        http://www.gnu.org/copyleft/lesser.txt.
        -----------------------------------------------------------------------------
*/
#include "OgrePythonLink.h"
#include "OgrePythonFrameListener.h"
#include "ExampleFrameListener.h"

/*
//-----------------------------------------------------------------------
Notes on using this file
If
*/

// Define Enum wrappers
template class boost::python::enum_as_int_converters<Ogre::SceneType>;
template class boost::python::enum_as_int_converters<Ogre::LoggingLevel>;
template class boost::python::enum_as_int_converters<Ogre::LogMessageLevel>;
template class boost::python::enum_as_int_converters<Ogre::RenderTarget::StatFlags>;
template class boost::python::enum_as_int_converters<Ogre::ProjectionType>;
template class boost::python::enum_as_int_converters<Ogre::SceneDetailLevel>;
template class boost::python::enum_as_int_converters<Ogre::FrustumPlane>;
template class boost::python::enum_as_int_converters<Ogre::Plane::Side>;
template class boost::python::enum_as_int_converters<Ogre::Light::LightTypes>;
template class boost::python::enum_as_int_converters<Ogre::ShadeOptions>;
template class boost::python::enum_as_int_converters<Ogre::TextureFilterOptions>;
template class boost::python::enum_as_int_converters<Ogre::SceneManager::PrefabType>;
template class boost::python::enum_as_int_converters<Ogre::ResourceType>;
template class boost::python::enum_as_int_converters<Ogre::Material::TextureLayer::TextureAddressingMode>;
template class boost::python::enum_as_int_converters<Ogre::LayerBlendOperation>;
template class boost::python::enum_as_int_converters<Ogre::Material::TextureLayer::TextureTransformType>;
template class boost::python::enum_as_int_converters<Ogre::WaveformType>;
template class boost::python::enum_as_int_converters<Ogre::CompareFunction>;
template class boost::python::enum_as_int_converters<Ogre::SceneBlendType>;
template class boost::python::enum_as_int_converters<Ogre::SceneBlendFactor>;
template class boost::python::enum_as_int_converters<Ogre::CullingMode>;


// Define pointer converters (the dangerous type, but I don't want copying!)
BOOST_PYTHON_BEGIN_CONVERSION_NAMESPACE // this is a gcc 2.95.2 bug workaround
#define BOOST_PYTHON_POINTER_CONVERTER(c) \
    PyObject* to_python(c* p) \
{ \
    return boost::python::python_extension_class_converters<c>::smart_ptr_to_python(p); \
}

BOOST_PYTHON_POINTER_CONVERTER(Ogre::RenderTarget)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::RenderWindow)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::RenderSystem)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::RenderSystemList)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::Resource)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::SceneManager)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::ResourceManager)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::TextureManager)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::Texture)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::MeshManager)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::Viewport)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::Camera)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::Material)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::Material::TextureLayer)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::Mesh)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::SubMesh)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::Light)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::Entity)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::SceneNode)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::Vector3 const)
BOOST_PYTHON_POINTER_CONVERTER(Ogre::MovableObject)

// Also implement converter for 'String' to/from Python
PyObject* to_python(Ogre::String s)
{
    // Use char converter
    return to_python(s.c_str());
}

Ogre::String from_python(PyObject* x, boost::python::type<Ogre::String>)
{
    // Get from char converter
    Ogre::String s = from_python(x, boost::python::type<const char*>());
    return s;
}
Ogre::String from_python(PyObject* x, boost::python::type<const Ogre::String>)
{
    // Get from char converter
    Ogre::String s = from_python(x, boost::python::type<const char*>());
    return s;
}
Ogre::String from_python(PyObject* x, boost::python::type<const Ogre::String&>)
{
    // Get from char converter
    Ogre::String s = from_python(x, boost::python::type<const char*>());
    return s;
}



BOOST_PYTHON_END_CONVERSION_NAMESPACE



namespace Ogre {


    //-----------------------------------------------------------------------
    PythonLink::PythonLink()
    {
        boostModule = 0;
    }

    //-----------------------------------------------------------------------
    PythonLink::~PythonLink()
    {
        if (boostModule)
            delete boostModule;
    }

    //-----------------------------------------------------------------------
    void PythonLink::exportClasses(void)
    {
        try {
            boostModule = new module_builder("OgrePython");
            exportGeneral();
            exportVector3();
            exportMatrix3();
            exportMatrix4();
            exportAxisAlignedBox();
            exportQuaternion();
            exportColourValue();
            exportResource();
            exportTextureManager();
            exportRoot();
            exportSceneManager();
            exportCamera();
            exportViewport();
            exportRenderWindow();
            exportPlane();
            exportRenderSystem();
            exportMaterial();
            exportMeshManager();
            exportMesh();
            exportLight();
            exportEntity();
            exportSceneNode();
            exportFrameListener();
        }
        catch (...)
        {
            // Boost manager for exceptions
            //boost::python::handle_exception();
        }


    }
    //-----------------------------------------------------------------------
    void PythonLink::exportGeneral(void)
    {
        // Enums
        // WaveformType
        boostModule->add((boost::python::detail::function *)to_python(WFT_SINE), "WFT_SINE");
        boostModule->add((boost::python::detail::function *)to_python(WFT_TRIANGLE), "WFT_TRIANGLE");
        boostModule->add((boost::python::detail::function *)to_python(WFT_SQUARE), "WFT_SQUARE");
        boostModule->add((boost::python::detail::function *)to_python(WFT_SAWTOOTH), "WFT_SAWTOOTH");
        boostModule->add((boost::python::detail::function *)to_python(WFT_INVERSE_SAWTOOTH), "WFT_INVERSE_SAWTOOTH");
        // CompareFunction
        boostModule->add((boost::python::detail::function *)to_python(CMPF_ALWAYS_FAIL), "CMPF_ALWAYS_FAIL");
        boostModule->add((boost::python::detail::function *)to_python(CMPF_ALWAYS_PASS), "CMPF_ALWAYS_PASS");
        boostModule->add((boost::python::detail::function *)to_python(CMPF_LESS), "CMPF_LESS");
        boostModule->add((boost::python::detail::function *)to_python(CMPF_LESS_EQUAL), "CMPF_LESS_EQUAL");
        boostModule->add((boost::python::detail::function *)to_python(CMPF_EQUAL), "CMPF_EQUAL");
        boostModule->add((boost::python::detail::function *)to_python(CMPF_NOT_EQUAL), "CMPF_NOT_EQUAL");
        boostModule->add((boost::python::detail::function *)to_python(CMPF_GREATER_EQUAL), "CMPF_GREATER_EQUAL");
        boostModule->add((boost::python::detail::function *)to_python(CMPF_GREATER), "CMPF_GREATER");
    }
    //-----------------------------------------------------------------------
    void PythonLink::exportVector3(void)
    {
        // Create the Python type object for our extension class
        class_builder<Vector3> cb(*boostModule, "Vector3");

        cb.def(constructor<>());
        cb.def(constructor<Real,Real,Real>());
        cb.def(&Vector3::crossProduct, "crossProduct");
        cb.def(&Vector3::dotProduct, "dotProduct");
        cb.def(&Vector3::length, "length");
        cb.def(&Vector3::midPoint, "midPoint");
        cb.def(&Vector3::normalise, "normalise");
        cb.def(&Vector3::squaredLength, "squaredLength");



    }
    //-----------------------------------------------------------------------
    void PythonLink::exportQuaternion(void)
    {
        // Create the Python type object for our extension class
        class_builder<Quaternion> cb(*boostModule, "Quaternion");

        cb.def(constructor<Real,Real,Real,Real>());
        cb.def(&Quaternion::Dot, "Dot");
        cb.def(&Quaternion::Exp, "Exp");
        cb.def(&Quaternion::FromAngleAxis, "FromAngleAxis");
        cb.def((void (Quaternion::*)(const Vector3&,const Vector3&, const Vector3&))&Quaternion::FromAxes, "FromAxes");
        cb.def(&Quaternion::FromRotationMatrix, "FromRotationMatrix");
        cb.def(&Quaternion::Intermediate, "Intermediate");
        cb.def(&Quaternion::Inverse, "Inverse");
        cb.def(&Quaternion::Log, "Log");
        cb.def(&Quaternion::Norm, "Norm");
        cb.def(&Quaternion::Slerp, "Slerp");
        cb.def(&Quaternion::SlerpExtraSpins, "SlerpExtraSpins");
        cb.def(&Quaternion::Squad, "Squad");
        //cb.def(&Quaternion::ToAngleAxis, "ToAngleAxis");
        cb.def((void (Quaternion::*)(Vector3&,Vector3&,Vector3&))&Quaternion::ToAxes, "ToAxes");
        cb.def(&Quaternion::ToRotationMatrix, "ToRotationMatrix");
        cb.def(&Quaternion::UnitInverse, "UnitInverse");



    }

    //-----------------------------------------------------------------------
    void PythonLink::exportMatrix3(void)
    {
        // Create the Python type object for our extension class
        class_builder<Matrix3> cb(*boostModule, "Matrix3");

        cb.def(constructor<>());


    }
    //-----------------------------------------------------------------------
    void PythonLink::exportMatrix4(void)
    {
        // Create the Python type object for our extension class
        class_builder<Matrix4> cb(*boostModule, "Matrix4");

        cb.def(constructor<>());

    }
    //-----------------------------------------------------------------------
    void PythonLink::exportResource(void)
    {
        // Create the Python type object for our extension class
        class_builder<Resource> cb(*boostModule, "Resource");
        cb.def(&Resource::getName, "getName");
        cb.def(&Resource::getLastAccess, "getLastAccess");
        cb.def(&Resource::getLastAccess, "getLastAccess");
        cb.def(&Resource::getSize, "getSize");
        cb.def(&Resource::load, "load");
        cb.def(&Resource::unload, "unload");
        cb.def(&Resource::touch, "touch");
    }

    //-----------------------------------------------------------------------
    void PythonLink::exportRoot(void)
    {
        // Create the Python type object for our extension class
        class_builder<Root> cb(*boostModule, "Root");

        // Define methods
        cb.def(constructor<>());
        cb.def(&Root::showConfigDialog, "showConfigDialog");
        cb.def(&Root::initialise, "initialise");
        cb.def(&Root::restoreConfig, "restoreConfig");
        cb.def(&Root::saveConfig, "saveConfig");
        cb.def(&Root::getAvailableRenderers, "getAvailableRenderers");
        cb.def(&Root::getRenderSystem, "getRenderSystem");
        cb.def(&Root::getSceneManager, "getSceneManager");
        cb.def(&Root::getTextureManager, "getTextureManager");
        cb.def(&Root::getMeshManager, "getMeshManager");
        cb.def(&Root::addFrameListener, "addFrameListener");
        cb.def(&Root::removeFrameListener, "removeFrameListener");
        cb.def(&Root::startRendering, "startRendering");
        cb.def(&Root::addResourceLocation, "addResourceLocation");


        // Enums

        // ResourceType
        boostModule->add((boost::python::detail::function *)to_python(RESTYPE_ALL), "RESTYPE_ALL");
        boostModule->add((boost::python::detail::function *)to_python(RESTYPE_TEXTURES), "RESTYPE_TEXTURES");
        boostModule->add((boost::python::detail::function *)to_python(RESTYPE_MODELS), "RESTYPE_MODELS");

        // LoggingLevel
        boostModule->add((boost::python::detail::function *)to_python(LL_LOW), "LL_LOW");
        boostModule->add((boost::python::detail::function *)to_python(LL_NORMAL), "LL_NORMAL");
        boostModule->add((boost::python::detail::function *)to_python(LL_BOREME), "LL_BOREME");

        // LogMessageLevel
        boostModule->add((boost::python::detail::function *)to_python(LML_TRIVIAL), "LML_TRIVIAL");
        boostModule->add((boost::python::detail::function *)to_python(LML_NORMAL), "LML_NORMAL");
        boostModule->add((boost::python::detail::function *)to_python(LML_CRITICAL), "LML_CRITICAL");



    }

    //-----------------------------------------------------------------------
    void PythonLink::exportRenderWindow(void)
    {
        // Also export superclass RenderTarget
        class_builder<RenderTarget> rt(*boostModule, "RenderTarget");
        rt.def(&RenderTarget::addViewport, "addViewport");
        rt.def(&RenderTarget::getColourDepth, "getColourDepth");
        rt.def(&RenderTarget::getHeight, "getHeight");
        rt.def(&RenderTarget::getName, "getName");
        rt.def(&RenderTarget::getWidth, "getWidth");
        rt.def(&RenderTarget::getLastFPS, "getLastFPS");
        rt.def(&RenderTarget::getAverageFPS, "getAverageFPS");
        rt.def(&RenderTarget::getBestFPS, "getBestFPS");
        rt.def(&RenderTarget::getWorstFPS, "getWorstFPS");
        rt.def(&RenderTarget::removeAllViewports, "removeAllViewports");
        rt.def(&RenderTarget::removeViewport, "removeViewport");
        rt.def(&RenderTarget::resetStatistics, "resetStatistics");
        rt.def(&RenderTarget::setStatsDisplay, "setStatsDisplay");
        rt.def(&RenderTarget::update, "update");

        // Enums
        // StatFlags
        boostModule->add((boost::python::detail::function *)to_python(RenderTarget::SF_NONE), "SF_NONE");
        boostModule->add((boost::python::detail::function *)to_python(RenderTarget::SF_AVG_FPS), "SF_AVG_FPS");
        boostModule->add((boost::python::detail::function *)to_python(RenderTarget::SF_BEST_FPS), "SF_BEST_FPS");
        boostModule->add((boost::python::detail::function *)to_python(RenderTarget::SF_WORST_FPS), "SF_WORST_FPS");
        boostModule->add((boost::python::detail::function *)to_python(RenderTarget::SF_ALL), "SF_ALL");


        // Create the Python type object for our extension class
        class_builder<RenderWindow> rw(*boostModule, "RenderWindow");

        // Define inheritence
        rw.declare_base(rt);

        // Define all superclass methods again (Python requires this)
        rw.def(&RenderWindow::addViewport, "addViewport");
        rw.def(&RenderWindow::getColourDepth, "getColourDepth");
        rt.def(&RenderTarget::getLastFPS, "getLastFPS");
        rt.def(&RenderTarget::getAverageFPS, "getAverageFPS");
        rt.def(&RenderTarget::getBestFPS, "getBestFPS");
        rt.def(&RenderTarget::getWorstFPS, "getWorstFPS");
        rw.def(&RenderWindow::getHeight, "getHeight");
        rw.def(&RenderWindow::getName, "getName");
        rw.def(&RenderWindow::getWidth, "getWidth");
        rw.def(&RenderWindow::removeAllViewports, "removeAllViewports");
        rw.def(&RenderWindow::removeViewport, "removeViewport");
        rw.def(&RenderWindow::resetStatistics, "resetStatistics");
        rw.def(&RenderWindow::setStatsDisplay, "setStatsDisplay");
        rw.def(&RenderWindow::update, "update");
        // New methods
        rw.def(&RenderWindow::isActive, "isActive");
        rw.def(&RenderWindow::isClosed, "isClosed");
        rw.def(&RenderWindow::isFullScreen, "isFullScreen");
        rw.def(&RenderWindow::reposition, "reposition");
        rw.def(&RenderWindow::resize, "resize");
        rw.def(&RenderWindow::swapBuffers, "swapBuffers");

    }

    //-----------------------------------------------------------------------
    void PythonLink::exportCamera(void)
    {
        class_builder<Camera> cb(*boostModule, "Camera");

        cb.def(&Camera::getAspectRatio, "getAspectRatio");
        cb.def(&Camera::getDetailLevel, "getDetailLevel");
        cb.def(&Camera::getDirection, "getDirection");
        cb.def(&Camera::getFarClipDistance, "getFarClipDistance");
        cb.def(&Camera::getFOVy, "getFOVy");
        cb.def(&Camera::getFrustumPlane, "getFrustumPlane");
        cb.def(&Camera::getName, "getName");
        cb.def(&Camera::getNearClipDistance, "getNearClipDistance");
        cb.def(&Camera::getPosition, "getPosition");
        cb.def(&Camera::getProjectionMatrix, "getProjectionMatrix");
        cb.def(&Camera::getProjectionType, "getProjectionType");
        cb.def(&Camera::getSceneManager, "getSceneManager");
        cb.def(&Camera::getViewMatrix, "getViewMatrix");
        cb.def(&Camera::isAttached, "isAttached");
        cb.def((void (Camera::*)(Real,Real,Real))&Camera::lookAt, "lookAt");
        cb.def((void (Camera::*)(const Vector3&))&Camera::lookAt, "lookAt");
        cb.def(&Camera::move, "move");
        cb.def(&Camera::pitch, "pitch");
        cb.def(&Camera::roll, "roll");
        cb.def((void (Camera::*)(const Vector3&,Real))&Camera::rotate, "rotate");
        cb.def((void (Camera::*)(const Quaternion&))&Camera::rotate, "rotate");
        cb.def(&Camera::setAspectRatio, "setAspectRatio");
        cb.def(&Camera::setDetailLevel, "setDetailLevel");
        cb.def((void (Camera::*) (Real, Real, Real))&Camera::setDirection, "setDirection");
        cb.def((void (Camera::*) (const Vector3&))&Camera::setDirection, "setDirection");
        cb.def(&Camera::setFarClipDistance, "setFarClipDistance");
        cb.def(&Camera::setFixedYawAxis, "setFixedYawAxis");
        cb.def(&Camera::setFOVy, "setFOVy");
        cb.def(&Camera::setNearClipDistance, "setNearClipDistance");
        cb.def((void (Camera::*)(Real,Real,Real))&Camera::setPosition, "setPosition");
        cb.def((void (Camera::*) (const Vector3&))&Camera::setPosition, "setPosition");
        cb.def(&Camera::setProjectionType, "setProjectionType");
        cb.def(&Camera::yaw, "yaw");

        // Enums
        // ProjectionType
        boostModule->add((boost::python::detail::function *)to_python(PT_ORTHOGRAPHIC), "PT_ORTHOGRAPHIC");
        boostModule->add((boost::python::detail::function *)to_python(PT_PERSPECTIVE), "PT_PERSPECTIVE");
        // SceneDetailLevel
        boostModule->add((boost::python::detail::function *)to_python(SDL_POINTSONLY), "SDL_POINTSONLY");
        boostModule->add((boost::python::detail::function *)to_python(SDL_WIREFRAME), "SDL_WIREFRAME");
        boostModule->add((boost::python::detail::function *)to_python(SDL_WIREFRAME), "SDL_FLATSHADE");
        boostModule->add((boost::python::detail::function *)to_python(SDL_WIREFRAME), "SDL_SMOOTHSHADE");
        boostModule->add((boost::python::detail::function *)to_python(SDL_WIREFRAME), "SDL_TEXTURED");
        // FrustumPlane
        boostModule->add((boost::python::detail::function *)to_python(FRUSTUM_PLANE_NEAR), "FRUSTUM_PLANE_NEAR");
        boostModule->add((boost::python::detail::function *)to_python(FRUSTUM_PLANE_FAR), "FRUSTUM_PLANE_FAR");
        boostModule->add((boost::python::detail::function *)to_python(FRUSTUM_PLANE_LEFT), "FRUSTUM_PLANE_LEFT");
        boostModule->add((boost::python::detail::function *)to_python(FRUSTUM_PLANE_RIGHT), "FRUSTUM_PLANE_RIGHT");
        boostModule->add((boost::python::detail::function *)to_python(FRUSTUM_PLANE_TOP), "FRUSTUM_PLANE_TOP");
        boostModule->add((boost::python::detail::function *)to_python(FRUSTUM_PLANE_BOTTOM), "FRUSTUM_PLANE_BOTTOM");




    }

    //-----------------------------------------------------------------------
    void PythonLink::exportViewport(void)
    {
        class_builder<Viewport> cb(*boostModule, "Viewport");

        cb.def(&Viewport::getActualHeight, "getActualHeight");
        cb.def(&Viewport::getActualLeft, "getActualLeft");
        cb.def(&Viewport::getActualTop, "getActualTop");
        cb.def(&Viewport::getActualWidth, "getActualWidth");
        cb.def(&Viewport::getBackgroundColour, "getBackgroundColour");
        cb.def(&Viewport::getCamera, "getCamera");
        cb.def(&Viewport::getClearEveryFrame, "getClearEveryFrame");
        cb.def(&Viewport::getHeight, "getHeight");
        cb.def(&Viewport::getLeft, "getLeft");
        cb.def(&Viewport::getTarget, "getTarget");
        cb.def(&Viewport::getTop, "getTop");
        cb.def(&Viewport::getWidth, "getWidth");
        cb.def(&Viewport::setBackgroundColour, "setBackgroundColour");
        cb.def(&Viewport::setClearEveryFrame, "setClearEveryFrame");
        cb.def(&Viewport::setDimensions, "setDimensions");
        cb.def(&Viewport::update, "update");

    }

    //-----------------------------------------------------------------------
    void PythonLink::exportColourValue(void)
    {
        class_builder<ColourValue> cb(*boostModule, "ColourValue");

        cb.def(constructor<Real,Real,Real,Real>());
        cb.def(&ColourValue::getAsLongRGBA, "getAsLongRGBA");

    }

    //-----------------------------------------------------------------------
    void PythonLink::exportSceneManager(void)
    {
        // Create the Python type object for our extension class
        class_builder<SceneManager> cb(*boostModule, "SceneManager");

        cb.def(&SceneManager::addMaterial, "addMaterial");
        cb.def(&SceneManager::clearScene, "clearScene");
        cb.def(&SceneManager::createCamera, "createCamera");
        cb.def((Entity* (SceneManager::*)(const String&, SceneManager::PrefabType))&SceneManager::createEntity, "createEntity");
        cb.def((Entity* (SceneManager::*)(const String&, const String&))&SceneManager::createEntity, "createEntity");
        cb.def(&SceneManager::createLight, "createLight");
        cb.def(&SceneManager::createMaterial, "createMaterial");
        cb.def(&SceneManager::createSceneNode, "createSceneNode");
        cb.def(&SceneManager::getAmbientLight, "getAmbientLight");
        cb.def(&SceneManager::getCamera, "getCamera");
        cb.def(&SceneManager::getDefaultMaterialSettings, "getDefaultMaterialSettings");
        cb.def(&SceneManager::getEntity, "getEntity");
        cb.def(&SceneManager::getLight, "getLight");
        cb.def((Material* (SceneManager::*)(const String&))&SceneManager::getMaterial, "getMaterial");
        cb.def((Material* (SceneManager::*)(int))&SceneManager::getMaterial, "getMaterial");
        cb.def(&SceneManager::getRootSceneNode, "getRootSceneNode");
        cb.def(&SceneManager::getSuggestedViewpoint, "getSuggestedViewpoint");
        cb.def(&SceneManager::removeAllCameras, "removeAllCameras");
        cb.def(&SceneManager::removeAllEntities, "removeAllEntities");
        cb.def(&SceneManager::removeAllLights, "removeAllLights");
        cb.def((void (SceneManager::*)(Camera*))&SceneManager::removeCamera, "removeCamera");
        cb.def((void (SceneManager::*)(const String&))&SceneManager::removeCamera, "removeCamera");
        cb.def((void (SceneManager::*)(Entity*))&SceneManager::removeEntity, "removeEntity");
        cb.def((void (SceneManager::*)(const String&))&SceneManager::removeEntity, "removeEntity");
        cb.def((void (SceneManager::*)(Light*))&SceneManager::removeLight, "removeLight");
        cb.def((void (SceneManager::*)(const String&))&SceneManager::removeLight, "removeLight");
        cb.def(&SceneManager::setAmbientLight, "setAmbientLight");
        cb.def(&SceneManager::setSkyBox, "setSkyBox");
        cb.def(&SceneManager::setSkyPlane, "setSkyPlane");
        cb.def(&SceneManager::setSkyDome, "setSkyDome");
        cb.def(&SceneManager::setWorldGeometry, "setWorldGeometry");

        // Enums
        // SceneType
        boostModule->add((boost::python::detail::function *)to_python(ST_GENERIC), "ST_GENERIC");
        boostModule->add((boost::python::detail::function *)to_python(ST_EXTERIOR_CLOSE), "ST_EXTERIOR_CLOSE");
        boostModule->add((boost::python::detail::function *)to_python(ST_EXTERIOR_FAR), "ST_EXTERIOR_FAR");
        boostModule->add((boost::python::detail::function *)to_python(ST_INTERIOR), "ST_INTERIOR");

        // Structs
        class_builder<ViewPoint> cbViewport(*boostModule, "ViewPoint");

    }
    //-----------------------------------------------------------------------
    void PythonLink::exportTextureManager()
    {

        // Export superclass too
        class_builder<ResourceManager> rmcb(*boostModule, "ResourceManager");
        rmcb.def(&ResourceManager::getByName, "getByName");

        // Create the Python type object for our extension class
        class_builder<TextureManager> texmgrcb(*boostModule, "TextureManager");

        // Define inheritence
        texmgrcb.declare_base(rmcb);

        // Define own methods, even inherited ones
        texmgrcb.def(&TextureManager::getByName, "getByName");
        texmgrcb.def(&TextureManager::load, "load");
        texmgrcb.def(&TextureManager::unload, "unload");
        texmgrcb.def(&TextureManager::setDefaultNumMipMaps, "setDefaultNumMipMaps");
        texmgrcb.def(&TextureManager::getDefaultNumMipMaps, "getDefaultNumMipMaps");
        texmgrcb.def(&TextureManager::enable32BitTextures, "enable32BitTextures");

        // Also export Texture
        class_builder<Texture> tcb(*boostModule, "Texture");


    }
    //-----------------------------------------------------------------------
    void PythonLink::exportPlane()
    {
        class_builder<Plane> cb(*boostModule, "Plane");

        cb.def(&Plane::getDistance, "getDistance");
        cb.def(&Plane::getSide, "getSide");

        // Enums
        // Side
        boostModule->add((boost::python::detail::function *)to_python(Plane::NO_SIDE), "NO_SIDE");
        boostModule->add((boost::python::detail::function *)to_python(Plane::POSITIVE_SIDE), "POSITIVE_SIDE");
        boostModule->add((boost::python::detail::function *)to_python(Plane::NEGATIVE_SIDE), "NEGATIVE_SIDE");


    }
    //-----------------------------------------------------------------------
    void PythonLink::exportRenderSystem(void)
    {
        class_builder<RenderSystem> cb(*boostModule, "RenderSystem");

        cb.def(&RenderSystem::addFrameListener, "addFrameListener");
        cb.def(&RenderSystem::createRenderWindow, "createRenderWindow");
        cb.def((void (RenderSystem::*)(const String&))&RenderSystem::destroyRenderWindow, "destroyRenderWindow");
        cb.def((void (RenderSystem::*)(RenderWindow*))&RenderSystem::destroyRenderWindow, "destroyRenderWindow");
        cb.def(&RenderSystem::getName, "getName");
        cb.def(&RenderSystem::getRenderWindow, "getRenderWindow");
        cb.def(&RenderSystem::removeFrameListener, "removeFrameListener");
        cb.def(&RenderSystem::setAmbientLight, "setAmbientLight");
        cb.def(&RenderSystem::setConfigOption, "setConfigOption");
        cb.def(&RenderSystem::setShadingType, "setShadingType");
        cb.def(&RenderSystem::setTextureFiltering, "setTextureFiltering");
        cb.def(&RenderSystem::startRendering, "startRendering");

        // Enums
        // ShadeOptions
        boostModule->add((boost::python::detail::function *)to_python(SO_FLAT), "SO_FLAT");
        boostModule->add((boost::python::detail::function *)to_python(SO_GOURAUD), "SO_GOURAUD");
        boostModule->add((boost::python::detail::function *)to_python(SO_PHONG), "SO_PHONG");
        // TextureFilterOptions
        boostModule->add((boost::python::detail::function *)to_python(TFO_NONE), "TFO_NONE");
        boostModule->add((boost::python::detail::function *)to_python(TFO_BILINEAR), "TFO_BILINEAR");
        boostModule->add((boost::python::detail::function *)to_python(TFO_TRILINEAR), "TFO_TRILINEAR");



    }
    //-----------------------------------------------------------------------
    void PythonLink::exportMaterial(void)
    {
        // Export inner class TextureLayer first
        class_builder<Material::TextureLayer> tlcb(*boostModule, "TextureLayer");

        tlcb.def(&Material::TextureLayer::getTextureName, "getTextureName");
        tlcb.def(&Material::TextureLayer::setTextureName, "setTextureName");
        tlcb.def((void (Material::TextureLayer::*)(const String&,int,Real))&Material::TextureLayer::setAnimatedTextureName, "setAnimatedTextureName");
        tlcb.def(&Material::TextureLayer::setCurrentFrame, "setCurrentFrame");
        tlcb.def(&Material::TextureLayer::getCurrentFrame, "getCurrentFrame");
        tlcb.def(&Material::TextureLayer::getFrameTextureName, "getFrameTextureName");
        tlcb.def(&Material::TextureLayer::getNumFrames, "getNumFrames");
        tlcb.def(&Material::TextureLayer::getTextureCoordSet, "getTextureCoordSet");
        tlcb.def(&Material::TextureLayer::setTextureCoordSet, "setTextureCoordSet");
        tlcb.def(&Material::TextureLayer::getTextureCoordSet, "getTextureCoordSet");
        tlcb.def(&Material::TextureLayer::getTextureAddressingMode, "getTextureAddressingMode");
        tlcb.def(&Material::TextureLayer::setTextureAddressingMode, "setTextureAddressingMode");
        tlcb.def(&Material::TextureLayer::setColourOperation, "setColourOperation");
        // Omit advanced colour blending ops for now
        tlcb.def(&Material::TextureLayer::setTextureScroll, "setTextureScroll");
        tlcb.def(&Material::TextureLayer::setTextureUScroll, "setTextureUScroll");
        tlcb.def(&Material::TextureLayer::setTextureVScroll, "setTextureVScroll");
        tlcb.def(&Material::TextureLayer::setTextureScale, "setTextureScale");
        tlcb.def(&Material::TextureLayer::setTextureUScale, "setTextureUScale");
        tlcb.def(&Material::TextureLayer::setTextureVScale, "setTextureVScale");
        tlcb.def(&Material::TextureLayer::setTextureRotate, "setTextureRotate");
        tlcb.def(&Material::TextureLayer::setEnvironmentMap, "setEnvironmentMap");
        tlcb.def(&Material::TextureLayer::setScrollAnimation, "setScrollAnimation");
        tlcb.def(&Material::TextureLayer::setRotateAnimation, "setRotateAnimation");
        tlcb.def(&Material::TextureLayer::setTransformAnimation, "setTransformAnimation");
        tlcb.def(&Material::TextureLayer::setAlphaRejectSettings, "setAlphaRejectSettings");
        tlcb.def(&Material::TextureLayer::removeAllEffects, "removeAllEffects");
        tlcb.def(&Material::TextureLayer::setBlank, "setBlank");
        tlcb.def(&Material::TextureLayer::isBlank, "isBlank");


        // TextureLayer Enums
        // TextureAddressingMode
        boostModule->add((boost::python::detail::function *)to_python(Material::TextureLayer::TAM_WRAP), "TAM_WRAP");
        boostModule->add((boost::python::detail::function *)to_python(Material::TextureLayer::TAM_MIRROR), "TAM_MIRROR");
        boostModule->add((boost::python::detail::function *)to_python(Material::TextureLayer::TAM_CLAMP), "TAM_CLAMP");
        // LayerBlendMode
        boostModule->add((boost::python::detail::function *)to_python(LBO_ADD), "LBO_ADD");
        boostModule->add((boost::python::detail::function *)to_python(LBO_REPLACE), "LBO_REPLACE");
        boostModule->add((boost::python::detail::function *)to_python(LBO_MODULATE), "LBO_MODULATE");
        boostModule->add((boost::python::detail::function *)to_python(LBO_ALPHA_BLEND), "LBO_ALPHA_BLEND");
        // TextureTransformType
        boostModule->add((boost::python::detail::function *)to_python(Material::TextureLayer::TT_TRANSLATE_U), "TT_TRANSLATE_U");
        boostModule->add((boost::python::detail::function *)to_python(Material::TextureLayer::TT_TRANSLATE_V), "TT_TRANSLATE_V");
        boostModule->add((boost::python::detail::function *)to_python(Material::TextureLayer::TT_SCALE_U), "TT_SCALE_U");
        boostModule->add((boost::python::detail::function *)to_python(Material::TextureLayer::TT_SCALE_V), "TT_SCALE_V");
        boostModule->add((boost::python::detail::function *)to_python(Material::TextureLayer::TT_ROTATE), "TT_ROTATE");


        // Export Material
        class_builder<Material> cb(*boostModule, "Material");

        cb.def(&Material::getAmbient, "getAmbient");
        cb.def(&Material::getDiffuse, "getDiffuse");
        cb.def(&Material::getSpecular, "getSpecular");
        cb.def(&Material::getSelfIllumination, "getSelfIllumination");
        cb.def((void (Material::*)(Real,Real,Real))&Material::setAmbient, "setAmbient");
        cb.def((void (Material::*)(Real,Real,Real))&Material::setDiffuse, "setDiffuse");
        cb.def((void (Material::*)(Real,Real,Real))&Material::setSpecular, "setSpecular");
        cb.def((void (Material::*)(Real,Real,Real))&Material::setSelfIllumination, "setSelfIllumination");
        cb.def(&Material::getShininess, "getShininess");
        cb.def(&Material::setShininess, "setShininess");


        cb.def((Material::TextureLayer* (Material::*)(String,int))&Material::addTextureLayer, "addTextureLayer");
        cb.def(&Material::getName, "getName");
        cb.def(&Material::getNumTextureLayers, "getNumTextureLayers");
        cb.def(&Material::getTextureLayer, "getTextureLayer");
        cb.def(&Material::removeTextureLayer, "removeTextureLayer");
        cb.def((void (Material::*)(SceneBlendType))&Material::setSceneBlending, "setSceneBlending");
        cb.def((void (Material::*)(SceneBlendFactor,SceneBlendFactor))&Material::setSceneBlending, "setSceneBlending");
        cb.def(&Material::getSourceBlendFactor, "getSourceBlendFactor");
        cb.def(&Material::getDestBlendFactor, "getDestBlendFactor");
        cb.def(&Material::isTransparent, "isTransparent");
        cb.def(&Material::setDepthCheckEnabled, "setDepthCheckEnabled");
        cb.def(&Material::getDepthCheckEnabled, "getDepthCheckEnabled");
        cb.def(&Material::getDepthWriteEnabled, "getDepthWriteEnabled");
        cb.def(&Material::setDepthWriteEnabled, "setDepthWriteEnabled");
        cb.def(&Material::setDepthFunction, "setDepthFunction");
        cb.def(&Material::setCullingMode, "setCullingMode");
        cb.def(&Material::getCullingMode, "getCullingMode");
        cb.def(&Material::setLightingEnabled, "setLightingEnabled");
        cb.def(&Material::getLightingEnabled, "getLightingEnabled");
        cb.def(&Material::setShadingMode, "setShadingMode");
        cb.def(&Material::getShadingMode, "getShadingMode");
        cb.def(&Material::setTextureFiltering, "setTextureFiltering");
        cb.def(&Material::getTextureFiltering, "getTextureFiltering");


        // Material enums
        // SceneBlendType
        boostModule->add((boost::python::detail::function *)to_python(SBT_TRANSPARENT_ALPHA), "SBT_TRANSPARENT_ALPHA");
        boostModule->add((boost::python::detail::function *)to_python(SBT_TRANSPARENT_COLOUR), "SBT_TRANSPARENT_COLOUR");
        boostModule->add((boost::python::detail::function *)to_python(SBT_ADD), "SBT_ADD");
        // SceneBlendFactor
        boostModule->add((boost::python::detail::function *)to_python(SBF_ONE), "SBF_ONE");
        boostModule->add((boost::python::detail::function *)to_python(SBF_ZERO), "SBF_ZERO");
        boostModule->add((boost::python::detail::function *)to_python(SBF_DEST_COLOUR), "SBF_DEST_COLOUR");
        boostModule->add((boost::python::detail::function *)to_python(SBF_SOURCE_COLOUR), "SBF_SOURCE_COLOUR");
        boostModule->add((boost::python::detail::function *)to_python(SBF_ONE_MINUS_DEST_COLOUR), "SBF_ONE_MINUS_DEST_COLOUR");
        boostModule->add((boost::python::detail::function *)to_python(SBF_ONE_MINUS_SOURCE_COLOUR), "SBF_ONE_MINUS_SOURCE_COLOUR");
        boostModule->add((boost::python::detail::function *)to_python(SBF_DEST_ALPHA), "SBF_DEST_ALPHA");
        boostModule->add((boost::python::detail::function *)to_python(SBF_SOURCE_ALPHA), "SBF_SOURCE_ALPHA");
        boostModule->add((boost::python::detail::function *)to_python(SBF_ONE_MINUS_DEST_ALPHA), "SBF_ONE_MINUS_DEST_ALPHA");
        boostModule->add((boost::python::detail::function *)to_python(SBF_ONE_MINUS_SOURCE_ALPHA), "SBF_ONE_MINUS_SOURCE_ALPHA");
        // CullingMode
        boostModule->add((boost::python::detail::function *)to_python(CULL_NONE), "CULL_NONE");
        boostModule->add((boost::python::detail::function *)to_python(CULL_CLOCKWISE), "CULL_CLOCKWISE");
        boostModule->add((boost::python::detail::function *)to_python(CULL_ANTICLOCKWISE), "CULL_ANTICLOCKWISE");







    }
    //-----------------------------------------------------------------------
    void PythonLink::exportMeshManager(void)
    {
        class_builder<MeshManager> cb(*boostModule, "MeshManager");

        cb.def(&MeshManager::create, "create");
        cb.def(&MeshManager::load, "load");
        cb.def(&MeshManager::getByName, "getByName");

    }
    //-----------------------------------------------------------------------
    void PythonLink::exportMesh(void)
    {
        class_builder<Mesh> cb(*boostModule, "Mesh");

        cb.def(&Mesh::getNumSubMeshes, "getNumSubMeshes");
        cb.def(&Mesh::getSubMesh, "getSubMesh");
        cb.def(&Mesh::getName, "getName");
        cb.def(&Mesh::load, "load");
        cb.def(&Mesh::unload, "unload");

        // Also export SubMesh
        class_builder<SubMesh> subcb(*boostModule, "SubMesh");

    }
    //-----------------------------------------------------------------------
    void PythonLink::exportEntity(void)
    {
        class_builder<MovableObject> cbmov(*boostModule, "MovableObject");

        class_builder<Entity> cb(*boostModule, "Entity");
        cb.declare_base(cbmov);

        cb.def(&Entity::getMesh, "getMesh");
        cb.def(&Entity::getName, "getName");

    }
    //-----------------------------------------------------------------------
    void PythonLink::exportLight(void)
    {
        class_builder<Light> cb(*boostModule, "Light");

        cb.def(constructor<>());
        cb.def(&Light::getAttenuationConstant, "getAttenuationConstant");
        cb.def(&Light::getAttenuationLinear, "getAttenuationLinear");
        cb.def(&Light::getAttenuationQuadric, "getAttenuationQuadric");
        cb.def(&Light::getAttenuationRange, "getAttenuationRange");
        cb.def(&Light::getDiffuseColour, "getDiffuseColour");
        cb.def(&Light::getDirection, "getDirection");
        cb.def(&Light::getPosition, "getPosition");
        cb.def(&Light::getSpecularColour, "getSpecularColour");
        cb.def(&Light::getSpotlightFalloff, "getSpotlightFalloff");
        cb.def(&Light::getSpotlightInnerAngle, "getSpotlightInnerAngle");
        cb.def(&Light::getSpotlightOuterAngle, "getSpotlightOuterAngle");
        cb.def(&Light::getType, "getType");
        cb.def(&Light::setAttenuation, "setAttenuation");
        cb.def(&Light::setSpotlightRange, "setSpotlightRange");
        cb.def(&Light::setType, "setType");

        // Hmm, keep getting LNK1179 errors when exporting the below??
        // Appears to be a compiler bug - GRRR
        cb.def((void (Light::*)(Real,Real,Real))&Light::setPosition, "setPosition");
        cb.def((void (Light::*)(const Vector3&))&Light::setPosition, "setPosition");
        cb.def((void (Light::*)(const Vector3&))&Light::setDirection, "setDirection");
        cb.def((void (Light::*)(Real,Real,Real))&Light::setDirection, "setDirection");

        cb.def((void (Light::*)(const ColourValue&))&Light::setDiffuseColour, "setDiffuseColour");
        cb.def((void (Light::*)(Real,Real,Real))&Light::setDiffuseColour, "setDiffuseColour");
        cb.def((void (Light::*)(const ColourValue&))&Light::setSpecularColour, "setSpecularColour");
        cb.def((void (Light::*)(Real,Real,Real))&Light::setSpecularColour, "setSpecularColour");
        // Enums
        // LightTypes
        boostModule->add((boost::python::detail::function *)to_python(Light::LT_POINT), "LT_POINT");
        boostModule->add((boost::python::detail::function *)to_python(Light::LT_DIRECTIONAL), "LT_DIRECTIONAL");
        boostModule->add((boost::python::detail::function *)to_python(Light::LT_SPOTLIGHT), "LT_SPOTLIGHT");

    }
    //-----------------------------------------------------------------------
    void PythonLink::exportSceneNode(void)
    {
        class_builder<SceneNode> cb(*boostModule, "SceneNode");

        cb.def(&SceneNode::addChild, "addChild");
        cb.def(&SceneNode::attachCamera, "attachCamera");
        cb.def(&SceneNode::attachObject, "attachObject");
        cb.def(&SceneNode::attachLight, "attachLight");
        cb.def(&SceneNode::detachCamera, "detachCamera");
        cb.def(&SceneNode::detachObject, "detachObject");
        cb.def(&SceneNode::detachLight, "detachLight");
        cb.def(&SceneNode::getAttachedCamera, "getAttachedCamera");
        cb.def(&SceneNode::getAttachedObject, "getAttachedObject");
        cb.def(&SceneNode::getAttachedLight, "getAttachedLight");
        cb.def(&SceneNode::getChild, "getChild");
        cb.def(&SceneNode::getLocalAxes, "getLocalAxes");
        cb.def(&SceneNode::getOrientation, "getOrientation");
        cb.def(&SceneNode::getParent, "getParent");
        cb.def(&SceneNode::getPosition, "getPosition");
        cb.def(&SceneNode::numAttachedCameras, "numAttachedCameras");
        cb.def(&SceneNode::numAttachedLights, "numAttachedLights");
        cb.def(&SceneNode::numAttachedObjects, "numAttachedObjects");
        cb.def(&SceneNode::numChildren, "numChildren");
        cb.def(&SceneNode::pitch, "pitch");
        cb.def(&SceneNode::removeChild, "removeChild");
        cb.def(&SceneNode::resetOrientation, "resetOrientation");
        cb.def(&SceneNode::roll, "roll");
        cb.def((void (SceneNode::*)(const Vector3&,Real))&SceneNode::rotate, "rotate");
        cb.def((void (SceneNode::*)(const Quaternion&))&SceneNode::rotate, "rotate");
        cb.def(&SceneNode::setOrientation, "setOrientation");
        cb.def((void (SceneNode::*)(const Vector3&))&SceneNode::setPosition, "setPosition");
        cb.def((void (SceneNode::*)(Real,Real,Real))&SceneNode::setPosition, "setPosition");
        cb.def((void (SceneNode::*)(const Vector3&))&SceneNode::translate, "translate");
        cb.def((void (SceneNode::*)(Real,Real,Real))&SceneNode::translate, "translate");
        cb.def((void (SceneNode::*)(const Matrix3&, const Vector3&))&SceneNode::translate, "translate");
        cb.def((void (SceneNode::*)(const Matrix3&, Real,Real,Real))&SceneNode::translate, "translate");
        cb.def(&SceneNode::yaw, "yaw");



    }

    //-----------------------------------------------------------------------
    void PythonLink::exportFrameListener(void)
    {
        // Export the FrameListener that can be used to call back Python code
        class_builder<FrameListener,PythonFrameListener> cb(*boostModule, "FrameListener");

        cb.def(constructor<>());
        cb.def(&FrameListener::frameEnded, "frameEnded", &PythonFrameListener::default_frameEnded);
        cb.def(&FrameListener::frameStarted, "frameStarted", &PythonFrameListener::default_frameStarted);


        // Export the C++ implemented standard frame listener
        class_builder<ExampleFrameListener> cbexample(*boostModule, "ExampleFrameListener");

        cbexample.def(constructor<RenderWindow*,Camera*>());
        cbexample.declare_base(cb);

        // Nothing else required for example frame listener


        // Also export FrameEvent
        class_builder<FrameEvent> evtcb(*boostModule, "FrameEvent");


    }
    //-----------------------------------------------------------------------
    void PythonLink::exportAxisAlignedBox(void)
    {
        class_builder<AxisAlignedBox> cb(*boostModule, "AxisAlignedBox");
        cb.def(constructor<>());
        cb.def(&AxisAlignedBox::getAllCorners, "getAllCorners");
        cb.def(&AxisAlignedBox::getMaximum, "getMaximum");
        cb.def(&AxisAlignedBox::getMinimum, "getMinimum");
        cb.def((void (AxisAlignedBox::*)(Real,Real,Real))&AxisAlignedBox::setMinimum, "setMinimum");
        cb.def((void (AxisAlignedBox::*)(const Vector3&))&AxisAlignedBox::setMinimum, "setMinimum");
        cb.def((void (AxisAlignedBox::*)(Real,Real,Real))&AxisAlignedBox::setMaximum, "setMaximum");
        cb.def((void (AxisAlignedBox::*)(const Vector3&))&AxisAlignedBox::setMaximum, "setMaximum");

    }
}

#ifdef HAVE_CONFIG_H
#   include "config.h"
#endif

#include "OgrePythonLink.h"

using namespace boost::python;
using namespace Ogre;

void exportMaterial()
{
    scope in_Material(
        class_<Material>("Material", init<const std::string&, bool>())
            .def("getAmbient", &Material::getAmbient, ccr())
            .def("getDiffuse", &Material::getDiffuse, ccr())
            .def("getSpecular", &Material::getSpecular, ccr())
            .def("getSelfIllumination", &Material::getSelfIllumination, ccr())
            .def("setAmbient", (void (Material::*)(Real,Real,Real))&Material::setAmbient)
            .def("setDiffuse", (void (Material::*)(Real,Real,Real))&Material::setDiffuse)
            .def("setSpecular", (void (Material::*)(Real,Real,Real))&Material::setSpecular)
            .def("setSelfIllumination", (void (Material::*)(Real,Real,Real))&Material::setSelfIllumination)
            .def("getShininess", &Material::getShininess)
            .def("setShininess", &Material::setShininess)


            .def("addTextureLayer",(Material::TextureLayer*
                    (Material::*)(String,int))&Material::addTextureLayer, rir())
            .def("getName", &Material::getName, ccr())
            .def("getNumTextureLayers", &Material::getNumTextureLayers)
            .def("getTextureLayer", &Material::getTextureLayer, rir())
            .def("removeTextureLayer", &Material::removeTextureLayer)
            .def("setSceneBlending", (void (Material::*)(SceneBlendType))&Material::setSceneBlending)
            .def("setSceneBlending", (void (Material::*)(SceneBlendFactor,SceneBlendFactor))&Material::setSceneBlending)
            .def("getSourceBlendFactor", &Material::getSourceBlendFactor)
            .def("getDestBlendFactor", &Material::getDestBlendFactor)
            .def("isTransparent", &Material::isTransparent)
            .def("setDepthCheckEnabled", &Material::setDepthCheckEnabled)
            .def("getDepthCheckEnabled", &Material::getDepthCheckEnabled)
            .def("getDepthWriteEnabled", &Material::getDepthWriteEnabled)
            .def("setDepthWriteEnabled", &Material::setDepthWriteEnabled)
            .def("setDepthFunction", &Material::setDepthFunction)
            .def("setCullingMode", &Material::setCullingMode)
            .def("getCullingMode", &Material::getCullingMode)
            .def("setLightingEnabled", &Material::setLightingEnabled)
            .def("getLightingEnabled", &Material::getLightingEnabled)
            .def("setShadingMode", &Material::setShadingMode)
            .def("getShadingMode", &Material::getShadingMode)
            .def("setTextureFiltering", &Material::setTextureFiltering)
            .def("getTextureFilterin", &Material::getTextureFiltering)
    );

    scope in_TextureLayer(
        class_<Material::TextureLayer>("TextureLayer", init<bool>())
                
            .def("getTextureName", &Material::TextureLayer::getTextureName, ccr())
            .def("setTextureName", &Material::TextureLayer::setTextureName)
            .def("setAnimatedTextureName", (void (Material::TextureLayer::*)(const String&,int,Real))&Material::TextureLayer::setAnimatedTextureName)
            .def("setCurrentFrame", &Material::TextureLayer::setCurrentFrame)
            .def("getCurrentFrame", &Material::TextureLayer::getCurrentFrame)
            .def("getFrameTextureName",
                    &Material::TextureLayer::getFrameTextureName, ccr())
            .def("getNumFrames", &Material::TextureLayer::getNumFrames)
            .def("getTextureCoordSet", &Material::TextureLayer::getTextureCoordSet)
            .def("setTextureCoordSet", &Material::TextureLayer::setTextureCoordSet)
            .def("getTextureCoordSet", &Material::TextureLayer::getTextureCoordSet)
            .def("getTextureAddressingMode", &Material::TextureLayer::getTextureAddressingMode)
            .def("setTextureAddressingMode", &Material::TextureLayer::setTextureAddressingMode)
            .def("setColourOperation", &Material::TextureLayer::setColourOperation)
            .def("setTextureScroll", &Material::TextureLayer::setTextureScroll)
            .def("setTextureUScroll", &Material::TextureLayer::setTextureUScroll)
            .def("setTextureVScroll", &Material::TextureLayer::setTextureVScroll)
            .def("setTextureScale", &Material::TextureLayer::setTextureScale)
            .def("setTextureUScale", &Material::TextureLayer::setTextureUScale)
            .def("setTextureVScale", &Material::TextureLayer::setTextureVScale)
            .def("setTextureRotate", &Material::TextureLayer::setTextureRotate)
            .def("setEnvironmentMap", &Material::TextureLayer::setEnvironmentMap)
            .def("setScrollAnimation", &Material::TextureLayer::setScrollAnimation)
            .def("setRotateAnimation", &Material::TextureLayer::setRotateAnimation)
            .def("setTransformAnimation", &Material::TextureLayer::setTransformAnimation)
            .def("setAlphaRejectSettings", &Material::TextureLayer::setAlphaRejectSettings)
            .def("removeAllEffects", &Material::TextureLayer::removeAllEffects)
            .def("setBlank", &Material::TextureLayer::setBlank)
            .def("isBlank", &Material::TextureLayer::isBlank)
    );


    // Enumerations

    enum_<Material::TextureLayer::TextureEffectType>("TextureEffectType")
        .value("ET_BUMP_MAP", Material::TextureLayer::ET_BUMP_MAP)
        .value("ET_ENVIRONMENT_MAP", Material::TextureLayer::ET_ENVIRONMENT_MAP)
        .value("ET_SCROLL", Material::TextureLayer::ET_SCROLL)
        .value("ET_ROTATE", Material::TextureLayer::ET_ROTATE)
        .value("ET_TRANSFORM", Material::TextureLayer::ET_TRANSFORM)
    ;

    enum_<Material::TextureLayer::EnvMapType>("EnvMapType")
        .value("ENV_PLANAR", Material::TextureLayer::ENV_PLANAR)
        .value("ENV_CURVED", Material::TextureLayer::ENV_CURVED)
    ;

    enum_<Material::TextureLayer::TextureTransformType>("TextureTransformType")
        .value("TT_TRANSLATE_U", Material::TextureLayer::TT_TRANSLATE_U)
        .value("TT_TRANSLATE_V", Material::TextureLayer::TT_TRANSLATE_V)
        .value("TT_SCALE_U", Material::TextureLayer::TT_SCALE_U)
        .value("TT_SCALE_V", Material::TextureLayer::TT_SCALE_V)
        .value("TT_ROTATE", Material::TextureLayer::TT_ROTATE)
    ;

    enum_<Material::TextureLayer::TextureAddressingMode>("TextureAddressingMode")
        .value("TAM_WRAP", Material::TextureLayer::TAM_WRAP)
        .value("TAM_MIRROR", Material::TextureLayer::TAM_MIRROR)
        .value("TAM_CLAMP", Material::TextureLayer::TAM_CLAMP)
    ;

    enum_<Material::TextureLayer::TextureCubeFace>("TextureCubeFace")
        .value("CUBE_FRONT", Material::TextureLayer::CUBE_FRONT)
        .value("CUBE_BACK", Material::TextureLayer::CUBE_BACK)
        .value("CUBE_LEFT", Material::TextureLayer::CUBE_LEFT)
        .value("CUBE_RIGHT", Material::TextureLayer::CUBE_RIGHT)
        .value("CUBE_UP", Material::TextureLayer::CUBE_UP)
        .value("CUBE_DOWN", Material::TextureLayer::CUBE_DOWN)
    ;
}


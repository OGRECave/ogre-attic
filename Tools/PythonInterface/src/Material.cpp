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


            .def("addTextureLayer",(TextureUnitState*
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
        class_<TextureUnitState>("TextureUnitState", init<bool>())
                
            .def("getTextureName", &TextureUnitState::getTextureName, ccr())
            .def("setTextureName", &TextureUnitState::setTextureName)
            .def("setAnimatedTextureName", (void (TextureUnitState::*)(const String&,int,Real))&TextureUnitState::setAnimatedTextureName)
            .def("setCurrentFrame", &TextureUnitState::setCurrentFrame)
            .def("getCurrentFrame", &TextureUnitState::getCurrentFrame)
            .def("getFrameTextureName",
                    &TextureUnitState::getFrameTextureName, ccr())
            .def("getNumFrames", &TextureUnitState::getNumFrames)
            .def("getTextureCoordSet", &TextureUnitState::getTextureCoordSet)
            .def("setTextureCoordSet", &TextureUnitState::setTextureCoordSet)
            .def("getTextureCoordSet", &TextureUnitState::getTextureCoordSet)
            .def("getTextureAddressingMode", &TextureUnitState::getTextureAddressingMode)
            .def("setTextureAddressingMode", &TextureUnitState::setTextureAddressingMode)
            .def("setColourOperation", &TextureUnitState::setColourOperation)
            .def("setTextureScroll", &TextureUnitState::setTextureScroll)
            .def("setTextureUScroll", &TextureUnitState::setTextureUScroll)
            .def("setTextureVScroll", &TextureUnitState::setTextureVScroll)
            .def("setTextureScale", &TextureUnitState::setTextureScale)
            .def("setTextureUScale", &TextureUnitState::setTextureUScale)
            .def("setTextureVScale", &TextureUnitState::setTextureVScale)
            .def("setTextureRotate", &TextureUnitState::setTextureRotate)
            .def("setEnvironmentMap", &TextureUnitState::setEnvironmentMap)
            .def("setScrollAnimation", &TextureUnitState::setScrollAnimation)
            .def("setRotateAnimation", &TextureUnitState::setRotateAnimation)
            .def("setTransformAnimation", &TextureUnitState::setTransformAnimation)
            .def("setAlphaRejectSettings", &TextureUnitState::setAlphaRejectSettings)
            .def("removeAllEffects", &TextureUnitState::removeAllEffects)
            .def("setBlank", &TextureUnitState::setBlank)
            .def("isBlank", &TextureUnitState::isBlank)
    );


    // Enumerations

    enum_<TextureUnitState::TextureEffectType>("TextureEffectType")
        .value("ET_ENVIRONMENT_MAP", TextureUnitState::ET_ENVIRONMENT_MAP)
        .value("ET_SCROLL", TextureUnitState::ET_SCROLL)
        .value("ET_ROTATE", TextureUnitState::ET_ROTATE)
        .value("ET_TRANSFORM", TextureUnitState::ET_TRANSFORM)
    ;

    enum_<TextureUnitState::EnvMapType>("EnvMapType")
        .value("ENV_PLANAR", TextureUnitState::ENV_PLANAR)
        .value("ENV_CURVED", TextureUnitState::ENV_CURVED)
    ;

    enum_<TextureUnitState::TextureTransformType>("TextureTransformType")
        .value("TT_TRANSLATE_U", TextureUnitState::TT_TRANSLATE_U)
        .value("TT_TRANSLATE_V", TextureUnitState::TT_TRANSLATE_V)
        .value("TT_SCALE_U", TextureUnitState::TT_SCALE_U)
        .value("TT_SCALE_V", TextureUnitState::TT_SCALE_V)
        .value("TT_ROTATE", TextureUnitState::TT_ROTATE)
    ;

    enum_<TextureUnitState::TextureAddressingMode>("TextureAddressingMode")
        .value("TAM_WRAP", TextureUnitState::TAM_WRAP)
        .value("TAM_MIRROR", TextureUnitState::TAM_MIRROR)
        .value("TAM_CLAMP", TextureUnitState::TAM_CLAMP)
    ;

    enum_<TextureUnitState::TextureCubeFace>("TextureCubeFace")
        .value("CUBE_FRONT", TextureUnitState::CUBE_FRONT)
        .value("CUBE_BACK", TextureUnitState::CUBE_BACK)
        .value("CUBE_LEFT", TextureUnitState::CUBE_LEFT)
        .value("CUBE_RIGHT", TextureUnitState::CUBE_RIGHT)
        .value("CUBE_UP", TextureUnitState::CUBE_UP)
        .value("CUBE_DOWN", TextureUnitState::CUBE_DOWN)
    ;
}


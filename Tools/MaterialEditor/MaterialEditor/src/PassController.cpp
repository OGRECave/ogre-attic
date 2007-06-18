#include "PassController.h"

#include "OgrePass.h"

#include "TechniqueController.h"

PassController::PassController(Pass* pass)
: mParentController(NULL), mPass(pass)
{
}

PassController::PassController(TechniqueController* parent, Pass* pass)
: mParentController(parent), mPass(pass)
{
}

PassController::~PassController()
{
}
	
TechniqueController* PassController::getParentController() const
{
	return mParentController;
}

Pass* PassController::getPass() const
{
	return mPass;
}
	
void PassController::setName(const String& name)
{
	mPass->setName(name);
	
	// TODO: Fire event
}

void PassController::setAmbient(Real red, Real green, Real blue)
{
	mPass->setAmbient(red, green, blue);
	
	// TODO: Fire event
}

void PassController::setAmbient(const ColourValue& ambient)
{
	mPass->setAmbient(ambient);
	
	// TODO: Fire event
}

void PassController::setDiffuse(Real red, Real green, Real blue, Real alpha)
{
	mPass->setDiffuse(red, green, blue, alpha);
	
	// TODO: Fire event
}

void PassController::setDiffuse(const ColourValue &diffuse)
{
	mPass->setDiffuse(diffuse);
	
	// TODO: Fire event
}

void PassController::setSpecular(Real red, Real green, Real blue, Real alpha)
{
	mPass->setSpecular(red, green, blue, alpha);
	
	// TODO: Fire event
}

void PassController::setSpecular(const ColourValue &specular)
{
	mPass->setSpecular(specular);
	
	// TODO: Fire event
}

void PassController::setShininess(Real val)
{
	mPass->setShininess(val);
	
	// TODO: Fire event
}

void PassController::setSelfIllumination(Real red, Real green, Real blue)
{
	mPass->setSelfIllumination(red, green, blue);
	
	// TODO: Fire event
}

void PassController::setSelfIllumination(const ColourValue& selfIllum)
{
	mPass->setSelfIllumination(selfIllum);
	
	// TODO: Fire event
}

void PassController::setVertexColourTracking(TrackVertexColourType tracking)
{
	mPass->setVertexColourTracking(tracking);
	
	// TODO: Fire event
}

void PassController::setPointSize(Real ps)
{
	mPass->setPointSize(ps);
	
	// TODO: Fire event
}

void PassController::setPointSpritesEnabled(bool enabled)
{
	mPass->setPointSpritesEnabled(enabled);
	
	// TODO: Fire event
}

void PassController::setPointAttenuation(bool enabled, Real constant /* =0.0f */, Real linear /* =1.0f */, Real quadratic /* =0.0f */)
{
	mPass->setPointAttenuation(enabled, constant, linear, quadratic);
	
	// TODO: Fire event
}

void PassController::setPointMinSize(Real min)
{
	mPass->setPointMinSize(min);
	
	// TODO: Fire event
}

void PassController::setPointMaxSize(Real max)
{
	mPass->setPointMaxSize(max);
	
	// TODO: Fire event
}

void PassController::setSceneBlending(const SceneBlendType sbt)
{
	mPass->setSceneBlending(sbt);
	
	// TODO: Fire event
}

void PassController::setSceneBlending(const SceneBlendFactor sourceFactor, const SceneBlendFactor destFactor)
{
	mPass->setSceneBlending(sourceFactor, destFactor);
	
	// TODO: Fire event
}

void PassController::setDepthCheckEnabled(bool enabled)
{
	mPass->setDepthCheckEnabled(enabled);
	
	// TODO: Fire event
}

void PassController::setDepthWriteEnabled(bool enabled)
{
	mPass->setDepthWriteEnabled(enabled);
	
	// TODO: Fire event
}

void PassController::setDepthFunction(CompareFunction func)
{
	mPass->setDepthFunction(func);
	
	// TODO: Fire event
}

void PassController::setColourWriteEnabled(bool enabled)
{
	mPass->setColourWriteEnabled(enabled);
	
	// TODO: Fire event
}

void PassController::setCullingMode(CullingMode mode)
{
	mPass->setCullingMode(mode);
	
	// TODO: Fire event
}

void PassController::setManualCullingMode(ManualCullingMode mode)
{
	mPass->setManualCullingMode(mode);
	
	// TODO: Fire event
}

void PassController::setLightingEnabled(bool enabled)
{
	mPass->setLightingEnabled(enabled);
	
	// TODO: Fire event
}

void PassController::setMaxSimultaneousLights(unsigned short maxLights)
{
	mPass->setMaxSimultaneousLights(maxLights);
	
	// TODO: Fire event
}

void PassController::setStartLight(unsigned short startLight)
{
	mPass->setStartLight(startLight);
	
	// TODO: Fire event
}

void PassController::setShadingMode(ShadeOptions mode)
{
	mPass->setShadingMode(mode);
	
	// TODO: Fire event
}

void PassController::setPolygonMode(PolygonMode mode)
{
	mPass->setPolygonMode(mode);
	
	// TODO: Fire event
}

void PassController::setFog(bool overrideScene, FogMode mode /* =FOG_NONE */, const ColourValue& colour /* =ColourValue::White */, Real expDensity /* =0.001 */, Real linearStart /* =0.0 */, Real linearEnd /* =1.0 */)
{
	mPass->setFog(overrideScene, mode, colour, expDensity, linearStart, linearEnd);
	
	// TODO: Fire event
}

void PassController::setDepthBias(float constantBias, float slopeScaleBias /* =0.0f */)
{
	mPass->setDepthBias(constantBias, slopeScaleBias);
	
	// TODO: Fire event
}

void PassController::setAlphaRejectSettings(CompareFunction func, unsigned char value)
{
	mPass->setAlphaRejectSettings(func, value);
	
	// TODO: Fire event
}

void PassController::setAlphaRejectFunction(CompareFunction func)
{
	mPass->setAlphaRejectFunction(func);
	
	// TODO: Fire event
}

void PassController::setAlphaRejectValue(unsigned char val)
{
	mPass->setAlphaRejectValue(val);
	
	// TODO: Fire event
}

void PassController::setIteratePerLight(bool enabled, bool onlyForOneLightType /* =true */, Light::LightTypes lightType /*=Light::LT_POINT */)
{
	mPass->setIteratePerLight(enabled, onlyForOneLightType, lightType);
	
	// TODO: Fire event
}

void PassController::setLightCountPerIteration(unsigned short c)
{
	mPass->setLightCountPerIteration(c);
	
	// TODO: Fire event
}

void PassController::setVertexProgram(const String& name, bool resetParams /* =true */)
{
	mPass->setVertexProgram(name, resetParams);
	
	// TODO: Fire event
}

void PassController::setVertexProgramParameters(GpuProgramParametersSharedPtr params)
{
	mPass->setVertexProgramParameters(params);
	
	// TODO: Fire event
}

void PassController::setShadowCasterVertexProgram(const String& name)
{
	mPass->setShadowCasterVertexProgram(name);
	
	// TODO: Fire event
}

void PassController::setShadowCasterVertexProgramParameters(GpuProgramParametersSharedPtr params)
{
	mPass->setShadowCasterVertexProgramParameters(params);
	
	// TODO: Fire event
}

void PassController::setShadowReceiverVertexProgram(const String& name)
{
	mPass->setShadowReceiverVertexProgram(name);
	
	// TODO: Fire event
}

void PassController::setShadowReceiverVertexProgramParameters(GpuProgramParametersSharedPtr params)
{
	mPass->setShadowReceiverVertexProgramParameters(params);
	
	// TODO: Fire event
}

void PassController::setShadowReceiverFragmentProgram(const String& name)
{
	mPass->setShadowReceiverFragmentProgram(name);
	
	// TODO: Fire event
}

void PassController::setShadowReceiverFragmentProgramParameters(GpuProgramParametersSharedPtr params)
{
	mPass->setShadowReceiverFragmentProgramParameters(params);
	
	// TODO: Fire event
}

void PassController::setFragmentProgram(const String& name, bool resetParams /* =true */)
{
	mPass->setFragmentProgram(name, resetParams);
	
	// TODO: Fire event
}

void PassController::setFragmentProgramParameters(GpuProgramParametersSharedPtr params)
{
	mPass->setFragmentProgramParameters(params);
	
	// TODO: Fire event
}

void PassController::setTextureFiltering(TextureFilterOptions filterType)
{
	mPass->setTextureFiltering(filterType);
	
	// TODO: Fire event
}

void PassController::setTextureAnisotropy(unsigned int maxAniso)
{
	mPass->setTextureAnisotropy(maxAniso);
	
	// TODO: Fire event
}

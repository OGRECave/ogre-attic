/*
-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------
*/
#ifndef _MATERIALCONTROLLER_H_
#define _MATERIALCONTROLLER_H_

#define MATERIAL_ALL 1 
#define MATERIAL_RECEIVE_SHADOWS 1 << 1
#define MATERIAL_TRANSPARENCY_CASTS_SHADOWS 1 << 2
#define MATERIAL_TECHNIQUE 1 << 3
#define MATERIAL_POINT_SIZE 1 << 4
#define MATERIAL_AMBIENT 1 << 5
#define MATERIAL_DIFFUSE 1 << 6
#define MATERIAL_SPECULAR 1 << 7
#define MATERIAL_DEPTH_CHECK 1 << 8
#define MATERIAL_SHININESS 1 << 9
#define MATERIAL_SELF_ILLUM 1 << 10
#define MATERIAL_DEPTH_WRITE 1 << 11
#define MATERIAL_DEPTH_FUNC 1 << 12
#define MATERIAL_COLOUR_WRITE 1 << 13
#define MATERIAL_CULLING_MODE 1 << 14
#define MATERIAL_MANUAL_CULLING_MODE 1 << 15
#define MATERIAL_LIGHTING_ENABLED 1 << 16
#define MATERIAL_SHADING_MODE 1 << 17
#define MATERIAL_FOG 1 << 18
#define MATERIAL_DEPTH_BIAS 1 << 19
#define MATERIAL_TEXTURE_FILTERING 1 << 20
#define MATERIAL_TEXTURE_ANISOTROPY 1 << 21
#define MATERIAL_PARAMETER 1 << 22

//#include "Subject.h"

#include <list>

#include <boost/signal.hpp>

#include "OgreMaterial.h"

namespace Ogre
{
	class Technique;
}

class TechniqueController;

using namespace Ogre;

typedef std::list<TechniqueController*> TechniqueControllerList;

class MaterialController //: public Subject
{
public:
	MaterialController(Material* material);
	virtual ~MaterialController();
		
	Material* getMaterial() const;
	const TechniqueControllerList* getTechniqueControllers() const;		

	void setReceiveShadows(bool enabled);
	void setTransparencyCastsShadows(bool enabled);
	TechniqueController* createTechnique(void);
	void removeTechnique(unsigned short index);
	void removeAllTechniques(void);
	
	void setAmbient(const ColourValue&  ambient);
	void setAmbient(Real red, Real green, Real blue);
	void setColourWriteEnabled(bool enabled);   
	void setCullingMode(CullingMode mode);   
	void setDepthBias(float constantBias, float slopeScaleBias);   
	void setDepthCheckEnabled(bool enabled);  
	void setDepthFunction(CompareFunction func);
	void setDepthWriteEnabled(bool enabled);   
	void setDiffuse(const ColourValue&  diffuse);
	void setDiffuse(Real red, Real green, Real blue, Real alpha);   
	void setFog(bool overrideScene, FogMode mode = FOG_NONE, const ColourValue& colour = ColourValue::White, Real expDensity = 0.001, Real linearStart = 0.0, Real linearEnd = 1.0);   
	void setLightingEnabled(bool enabled);
	void setManualCullingMode(ManualCullingMode mode);
	void setPointSize(Real ps);   
	void setSceneBlending(const SceneBlendFactor sourceFactor, const SceneBlendFactor  destFactor);   
	void setSceneBlending(const SceneBlendType sbt);   
	void setSelfIllumination(const ColourValue& selfIllum);   
	void setSelfIllumination(Real red, Real green, Real blue);   
	void setShadingMode(ShadeOptions mode);   
	void setShininess(Real val);   
	void setSpecular(const ColourValue& specular);   
	void setSpecular(Real red, Real green, Real blue, Real alpha);   
	void setTextureAnisotropy(unsigned int maxAniso);   
	void setTextureFiltering(TextureFilterOptions filterType);   

protected:
	Material* mMaterial;
	
	TechniqueControllerList mTechniqueControllers;
};

#endif // _MATERIALCONTROLLER_H_
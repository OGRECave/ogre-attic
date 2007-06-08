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

#include "Subject.h"

namespace Ogre
{
	class Material;
}

class MaterialController : public Subject
{
public:
	MaterialController(Material* material);
	virtual ~MaterialController();
	
	Material* getMaterial();
};

#endif // _MATERIALCONTROLLER_H_
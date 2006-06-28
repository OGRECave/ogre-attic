#ifndef _MATERIAL_H
#define _MATERIAL_H

#include "mayaExportLayer.h"
#include "paramList.h"

namespace OgreMayaExporter
{

	typedef enum {MT_LAMBERT,MT_PHONG,MT_BLINN} MaterialType;

	typedef enum {TOT_REPLACE,TOT_MODULATE,TOT_ADD,TOT_ALPHABLEND} TexOpType;

	typedef enum {TAM_CLAMP,TAM_BORDER,TAM_WRAP,TAM_MIRROR} TexAddressMode;

	typedef struct textureTag
	{
		MString filename;
		MString absFilename;
		TexOpType opType;
		MString uvsetName;
		int uvsetIndex;
		TexAddressMode am_u,am_v;
		double scale_u,scale_v;
		double scroll_u,scroll_v;
		double rot;
	} texture;


	/***** Class Material *****/
	class Material
	{
	public:
		//constructor
		Material();
		//destructor
		~Material();
		//get material name
		MString& name();
		//clear material data
		void clear();
		//load material data
		MStatus load(MFnLambertShader* pShader,MStringArray& uvsets,ParamList& params);
		//write material data to Ogre material script
		MStatus writeOgreScript(ParamList &params);
		//copy textures to path specified by params
		MStatus copyTextures(ParamList &params);
	public:
		//load texture data
		MStatus loadTexture(MFnDependencyNode* pTexNode,TexOpType& opType,MStringArray& uvsets,ParamList& params);

		MString m_name;
		MaterialType m_type;
		MColor m_ambient, m_diffuse, m_specular, m_emissive;
		bool m_lightingOff;
		bool m_isTransparent;
		bool m_isTextured;
		bool m_isMultiTextured;
		std::vector<texture> m_textures;
	};

};	//end of namespace

#endif
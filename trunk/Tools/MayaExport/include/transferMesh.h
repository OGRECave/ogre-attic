
#ifndef TRANSFER_MESH_H
#define TRANSFER_MESH_H

#include "transferSkeleton.h"


/***** structure for vertex info *****/
typedef struct vertextag
{
	int pointIdx;				//index to points list (position)
	int normalIdx;				//index to normals list
	float r,g,b,a;				//colour
	std::vector<float> u;		//u texture coordinates
	std::vector<float> v;		//v texture coordinates
	std::vector<float> vba;		//vertex bone assignements
	std::vector<int> jointIds;	//ids of joints affecting this vertex
	int next;			//index of next vertex with same position
} vertex;


/***** structure for face info *****/
typedef struct facetag
{
	long v[3];		//vertex indices
} face;


typedef enum {MT_LAMBERT,MT_PHONG,MT_BLINN} MaterialType;

typedef enum {TOT_REPLACE,TOT_MODULATE,TOT_ADD,TOT_ALPHABLEND} TexOpType;


/***** Class TransferMesh *****/
class TransferMesh
{
public:
	//constructor
	TransferMesh();
	//destructor
	~TransferMesh();
	//load mesh data from a maya Fn
	MStatus load(MDagPath& meshDag,ParamList &params,TransferSkeleton* pSkeleton = NULL);
	//write to xml file
	MStatus writeToXML(ParamList& params);
	//clear data
	void clear();

private:
	//internal members
	bool mLoaded;
	bool mUse32bitIndexes;
	MString mName;
	MFnLambertShader* pMaterial;
	MFnSkinCluster* pSkinCluster;
	MStringArray uvsets;
	MFloatPointArray points;
	MFloatVectorArray normals;
	std::vector<face> faces;
	std::vector<vertex> vertices;

	//internal methods
	MStatus writeMesh(ParamList &params);
	MStatus writeMaterial(ParamList &params);
	MStatus writeTexture(MFnDependencyNode &tex,TexOpType texOp,ParamList &params);
};

#endif
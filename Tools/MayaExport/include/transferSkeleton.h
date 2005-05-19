
#ifndef TRANSFER_SKELETON_H
#define TRANSFER_SKELETON_H

#include <maya/MDagPathArray.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MFnIkJoint.h>
#include <maya/MQuaternion.h>
#include <maya/MEulerRotation.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MMatrix.h>
#include <maya/MFnMatrixData.h>
#include <maya/MFnMesh.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MDagPath.h>
#include <maya/MItGeometry.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MAnimUtil.h>
#include <maya/MAnimControl.h>
#include <maya/MTime.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MFnCharacter.h>
#include <maya/MFnClip.h>
#include "paramlist.h"

typedef struct jointTag
{
	MString name;
	int id;
	MMatrix localMatrix;
	MMatrix worldMatrix;
	int parentIndex;
	double posx,posy,posz;
	double angle;
	double axisx,axisy,axisz;
} joint;

typedef struct keyframeTag
{
	double time;							//time of keyframe
	double tx,ty,tz;						//translation
	double angle,axis_x,axis_y,axis_z;		//rotation
	double sx,sy,sz;						//scale
} keyframe;

typedef struct trackTag
{
	MString bone;
	std::vector<keyframe> keyframes;
} track;

typedef struct animationTag
{
	MString name;
	double length;
	std::vector<track> tracks;
} animation;


/***** Class TransferSkeleton *****/
class TransferSkeleton
{
public:
	// constructor
	TransferSkeleton();
	// destructor
	~TransferSkeleton();
	// load from a maya mesh Fn
	MStatus load(MDagPath& meshDag,ParamList& params);
	// write to xml file
	MStatus writeToXML(ParamList& params);
	// clears joints info
	void clear();

	// public members
	std::vector<joint> joints;
	std::vector<animation> animations;
	ParamList m_params;
private:
	// internal methods
	MStatus loadSkeleton(MDagPath& jointDag);
	MStatus loadJoint(MDagPath& jointDag,joint* parent);
	MStatus loadAnims(MDagPath& jointDag,int jointId);
	MStatus loadClip(MDagPath& jointDag,int jointId,MString clipName,double start,double stop,double rate);
	MTransformationMatrix getJointMatrix(MDagPath& jointDag,joint* parent,double time = 0);
	// private members
};

std::vector<double> mergesorted(const std::vector<double>& v1, const std::vector<double>& v2);

#endif
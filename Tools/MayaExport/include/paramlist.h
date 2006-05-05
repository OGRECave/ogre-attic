
#ifndef PARAMLIST_H
#define PARAMLIST_H

#include "mayaExportLayer.h"

// Length units multipliers from Maya internal unit (cm)

#define CM2MM 10.0
#define CM2CM 1.0
#define CM2M  0.01
#define CM2IN 0.393701
#define CM2FT 0.0328084
#define CM2YD 0.0109361

namespace OgreMayaExporter
{
	typedef struct clipInfoTag
	{
		float start;							//start time of the clip
		float stop;								//end time of the clip
		float rate;								//sample rate of anim curves, -1 means auto
		MString name;							//clip name
	} clipInfo;

	typedef enum
	{
		NPT_CURFRAME,
		NPT_BINDPOSE,
		NPT_FRAME
	} NeutralPoseType;

	/***** Class ParamList *****/
	class ParamList
	{
	public:
		// class members
		bool exportMesh, exportMaterial, exportAnimCurves, exportCameras, exportAll, exportVBA,
			exportVertNorm, exportVertCol, exportVertColWhite, exportTexCoord, exportCamerasAnim,
			exportSkeleton, exportSkelAnims, exportVertAnims, exportMeshBin, exportSkelBin, exportWorldCoords, useSharedGeom,
			lightingOff, copyTextures, exportParticles;

		float lum;	// Length Unit Multiplier

		MString meshFilename, skeletonFilename, materialFilename, animFilename, camerasFilename, matPrefix,
			texOutputDir, particlesFilename;

		std::ofstream outMesh, outMaterial, outAnim, outCameras, outSkeleton, outParticles;

		MStringArray writtenMaterials;

		std::vector<clipInfo> skelClipList;
		std::vector<clipInfo> vertClipList;

		NeutralPoseType neutralPoseType;
		long neutralPoseFrame;				// frame to use as neutral pose (in case NPT_FRAME is the neutral pose type)

		// constructor
		ParamList()	{
			lum = 1.0;
			exportMesh = false;
			exportMaterial = false;
			exportSkeleton = false;
			exportSkelAnims = false;
			exportVertAnims = false;
			exportAnimCurves = false;
			exportCameras = false;
			exportParticles = false;
			exportMeshBin = false;
			exportSkelBin = false;
			exportAll = false;
			exportWorldCoords = false;
			exportVBA = false;
			exportVertNorm = false;
			exportVertCol = false;
			exportVertColWhite = false;
			exportTexCoord = false;
			exportCamerasAnim = false;
			useSharedGeom = false;
			lightingOff = false;
			copyTextures = false;
			meshFilename = "";
			skeletonFilename = "";
			materialFilename = "";
			animFilename = "";
			camerasFilename = "";
			particlesFilename = "";
			matPrefix = "";
			texOutputDir = "";
			skelClipList.clear();
			vertClipList.clear();
			neutralPoseType = NPT_CURFRAME;	// set default to current frame
			neutralPoseFrame = 0;
		}

		ParamList& operator=(ParamList& source)	{
			lum = source.lum;
			exportMesh = source.exportMesh;
			exportMaterial = source.exportMaterial;
			exportSkeleton = source.exportSkeleton;
			exportSkelAnims = source.exportSkelAnims;
			exportVertAnims = source.exportVertAnims;
			exportAnimCurves = source.exportAnimCurves;
			exportCameras = source.exportCameras;
			exportAll = source.exportAll;
			exportWorldCoords = source.exportWorldCoords;
			exportVBA = source.exportVBA;
			exportVertNorm = source.exportVertNorm;
			exportVertCol = source.exportVertCol;
			exportVertColWhite = source.exportVertColWhite;
			exportTexCoord = source.exportTexCoord;
			exportCamerasAnim = source.exportCamerasAnim;
			exportMeshBin = source.exportMeshBin;
			exportSkelBin = source.exportSkelBin;
			exportParticles = source.exportParticles;
			useSharedGeom = source.useSharedGeom;
			lightingOff = source.lightingOff;
			copyTextures = source.copyTextures;
			meshFilename = source.meshFilename;
			skeletonFilename = source.skeletonFilename;
			materialFilename = source.materialFilename;
			animFilename = source.animFilename;
			camerasFilename = source.camerasFilename;
			particlesFilename = source.particlesFilename;
			matPrefix = source.matPrefix;
			texOutputDir = source.texOutputDir;
			skelClipList.resize(source.skelClipList.size());
			for (int i=0; i< skelClipList.size(); i++)
			{
				skelClipList[i].name = source.skelClipList[i].name;
				skelClipList[i].start = source.skelClipList[i].start;
				skelClipList[i].stop = source.skelClipList[i].stop;
				skelClipList[i].rate = source.skelClipList[i].rate;
			}
			vertClipList.resize(source.vertClipList.size());
			for (int i=0; i< vertClipList.size(); i++)
			{
				vertClipList[i].name = source.vertClipList[i].name;
				vertClipList[i].start = source.vertClipList[i].start;
				vertClipList[i].stop = source.vertClipList[i].stop;
				vertClipList[i].rate = source.vertClipList[i].rate;
			}
			neutralPoseType = source.neutralPoseType;
			neutralPoseFrame = source.neutralPoseFrame;
			return *this;
		}

		// destructor
		~ParamList() {
			if (outMesh)
				outMesh.close();
			if (outMaterial)
				outMaterial.close();
			if (outSkeleton)
				outSkeleton.close();
			if (outAnim)
				outAnim.close();
			if (outCameras)
				outCameras.close();
			if (outParticles)
				outParticles.close();
		}
		// method to pars arguments and set parameters
		void parseArgs(const MArgList &args);
		// method to open files for writing
		MStatus openFiles();
		// method to close open output files
		MStatus closeFiles();
	};

};	//end namespace

#endif
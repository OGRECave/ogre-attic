
#include "paramlist.h"
#include <maya/MGlobal.h>

/***** Class ParamList *****/
// method to parse arguments from command line
namespace OgreMayaExporter
{
	void ParamList::parseArgs(const MArgList &args)
	{
		MStatus stat;
		// Parse arguments from command line
		for (unsigned int i = 0; i < args.length(); i++ )
		{
			if ((MString("-all") == args.asString(i,&stat)) && (MS::kSuccess == stat))
				exportAll = true;
			else if ((MString("-world") == args.asString(i,&stat)) && (MS::kSuccess == stat))
				exportWorldCoords = true;
			else if ((MString("-lu") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				MString lengthUnit = args.asString(++i,&stat);
				if (MString("pref") == lengthUnit)
				{
					MGlobal::executeCommand("currentUnit -q -l",lengthUnit,false);
				}
				if (MString("mm") == lengthUnit)
					lum = CM2MM;
				else if (MString("cm") == lengthUnit)
					lum = CM2CM;
				else if (MString("m") == lengthUnit)
					lum = CM2M;
				else if (MString("in") == lengthUnit)
					lum = CM2IN;
				else if (MString("ft") == lengthUnit)
					lum = CM2FT;
				else if (MString("yd") == lengthUnit)
					lum = CM2YD;
			}
			else if ((MString("-mesh") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportMesh = true;
				meshFilename = args.asString(++i,&stat);
			}
			else if ((MString("-mat") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportMaterial = true;
				materialFilename = args.asString(++i,&stat);
			}
			else if ((MString("-matPrefix") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				matPrefix = args.asString(++i,&stat);
			}
			else if ((MString("-copyTex") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				copyTextures = true;
				texOutputDir = args.asString(++i,&stat);
			}
			else if ((MString("-lightOff") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				lightingOff = true;
			}
			else if ((MString("-skel") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportSkeleton = true;
				skeletonFilename = args.asString(++i,&stat);
			}
			else if ((MString("-anims") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportAnims = true;
			}
			else if ((MString("-animCur") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportAnimCurves = true;
				animFilename = args.asString(++i,&stat);
			}
			else if ((MString("-cam") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportCameras = true;
				camerasFilename = args.asString(++i,&stat);
			}
			else if ((MString("-v") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportVBA = true;
			}
			else if ((MString("-n") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportVertNorm = true;
			}
			else if ((MString("-c") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportVertCol = true;
			}
			else if ((MString("-cw") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportVertCol = true;
				exportVertColWhite = true;
			}
			else if ((MString("-t") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportTexCoord = true;
			}
			else if ((MString("-camAnim") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportCamerasAnim = true;
			}
			else if ((MString("-meshbin") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportMeshBin = true;
			}
			else if ((MString("-skelbin") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportSkelBin = true;
			}
			else if ((MString("-particles") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				exportParticles = true;
				particlesFilename = args.asString(++i,&stat);
			}
			else if ((MString("-shared") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				useSharedGeom = true;
			}
			else if ((MString("-np") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				MString npType = args.asString(++i,&stat);
				if (npType == "curFrame")
					neutralPoseType = NPT_CURFRAME;
				else if (npType == "bindPose")
					neutralPoseType = NPT_BINDPOSE;
				else if (npType == "frame")
				{
					neutralPoseType = NPT_FRAME;
					neutralPoseFrame = args.asInt(++i,&stat);
				}
			}
			else if ((MString("-clip") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			{
				//get clip name
				MString clipName = args.asString(++i,&stat);
				//get clip range
				MString clipRangeType = args.asString(++i,&stat);
				double startTime, stopTime;
				if (clipRangeType == "startEnd")
				{
					startTime = args.asDouble(++i,&stat);
					stopTime = args.asDouble(++i,&stat);
					MString rangeUnits = args.asString(++i,&stat);
					if (rangeUnits == "frames")
					{
						//range specified in frames => convert to seconds
						MTime t1(startTime, MTime::uiUnit());
						MTime t2(stopTime, MTime::uiUnit());
						startTime = t1.as(MTime::kSeconds);
						stopTime = t2.as(MTime::kSeconds);
					}
				}
				else
				{
					//range specified by time slider
					MTime t1 = MAnimControl::minTime();
					MTime t2 = MAnimControl::maxTime();
					startTime = t1.as(MTime::kSeconds);
					stopTime = t2.as(MTime::kSeconds);
				}
				// get sample rate
				double rate;
				MString sampleRateType = args.asString(++i,&stat);
				if (sampleRateType == "sampleByFrames")
				{
					// rate specified in frames
					int intRate = args.asInt(++i,&stat);
					MTime t = MTime(intRate, MTime::uiUnit());
					rate = t.as(MTime::kSeconds);
				}
				else
				{
					// rate specified in seconds
					rate = args.asDouble(++i,&stat);
				}
				//add clip info
				clipInfo clip;
				clip.name = clipName;
				clip.start = startTime;
				clip.stop = stopTime;
				clip.rate = rate;
				clipList.push_back(clip);
				std::cout << "clip " << clipName.asChar() << "\n";
				std::cout << "start: " << startTime << ", stop: " << stopTime << "\n";
				std::cout << "rate: " << rate << "\n";
				std::cout << "-----------------\n";
			}
		}
	}


	// method to open output files for writing
	MStatus ParamList::openFiles()
	{
		MString msg;
		if (exportMesh)
		{
			outMesh.open(meshFilename.asChar());
			if (!outMesh)
			{
				std::cout << "Error opening file: " << meshFilename.asChar() << "\n";
				return MS::kFailure;
			}
		}
		if (exportMaterial)
		{
			outMaterial.open(materialFilename.asChar());
			if (!outMaterial)
			{
				std::cout << "Error opening file: " << materialFilename.asChar() << "\n";
				return MS::kFailure;
			}
		}
		if (exportSkeleton)
		{
			outSkeleton.open(skeletonFilename.asChar());
			if (!outSkeleton)
			{
				std::cout << "Error opening file: " << skeletonFilename.asChar() << "\n";
				return MS::kFailure;
			}
		}
		if (exportAnimCurves)
		{
			outAnim.open(animFilename.asChar());
			if (!outAnim)
			{
				std::cout << "Error opening file: " << animFilename.asChar() << "\n";
				return MS::kFailure;
			}
		}
		if (exportCameras)
		{
			outCameras.open(camerasFilename.asChar());
			if (!outCameras)
			{
				std::cout << "Error opening file: " << camerasFilename.asChar() << "\n";
				return MS::kFailure;
			}
		}
		if (exportParticles)
		{
			outParticles.open(particlesFilename.asChar());
			if (!outParticles)
			{
				std::cout << "Error opening file: " << particlesFilename.asChar() << "\n";
				return MS::kFailure;
			}
		}
		return MS::kSuccess;
	}

	// method to close open output files
	MStatus ParamList::closeFiles()
	{
		if (exportMesh)
			outMesh.close();
		
		if (exportMaterial)
			outMaterial.close();
	
		if (exportSkeleton)
			outSkeleton.close();
		
		if (exportAnimCurves)
			outAnim.close();
			
		if (exportCameras)
			outCameras.close();
		
		return MS::kSuccess;
	}

}	//end namespace
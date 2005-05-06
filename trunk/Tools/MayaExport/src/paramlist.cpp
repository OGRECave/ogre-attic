
#include "paramlist.h"
#include <maya/MGlobal.h>

/***** Class ParamList *****/
// method to parse arguments from command line
void ParamList::parseArgs(const MArgList &args)
{
	MStatus stat;
	for (unsigned int i = 0; i < args.length(); i++ )
	{
        if ((MString("-all") == args.asString(i,&stat)) && (MS::kSuccess == stat))
			exportAll = true;
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
			msg = "Error opening file: ";
			msg += meshFilename;
			msg += "\n";
			MGlobal::displayInfo(msg);
			return MS::kFailure;
		}
	}
	if (exportMaterial)
	{
		outMaterial.open(materialFilename.asChar());
		if (!outMaterial)
		{
			msg = "Error opening file: ";
			msg += materialFilename;
			msg += "\n";
			MGlobal::displayInfo(msg);
			return MS::kFailure;
		}
	}
	if (exportSkeleton)
	{
		outSkeleton.open(skeletonFilename.asChar());
		if (!outSkeleton)
		{
			msg = "Error opening file: ";
			msg += skeletonFilename;
			msg += "\n";
			MGlobal::displayInfo(msg);
			return MS::kFailure;
		}
	}
	if (exportAnimCurves)
	{
		outAnim.open(animFilename.asChar());
		if (!outAnim)
		{
			msg = "Error opening file: ";
			msg += animFilename;
			msg += "\n";
			MGlobal::displayInfo(msg);
			return MS::kFailure;
		}
	}
	if (exportCameras)
	{
		outCameras.open(camerasFilename.asChar());
		if (!outCameras)
		{
			msg = "Error opening file: ";
			msg += camerasFilename;
			msg += "\n";
			MGlobal::displayInfo(msg);
			return MS::kFailure;
		}
	}
	return MS::kSuccess;
}

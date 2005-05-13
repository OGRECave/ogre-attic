
#include "OgreExporter.h"

MStatus OgreExporter::doIt(const MArgList& args)
{
	// Parse the arguments.
	params.parseArgs(args);
	// Read info about the clips we have to transform
	int i,numClips,exportClip,rangeType,rateType,rangeUnits;
	double startTime,stopTime,rate;
	MString command,clipName;;
	//read number of clips
	MGlobal::executeCommand("eval \"$numClips+=0\"",numClips,false);
	//read clips data
	for (i=1; i<=numClips; i++)
	{
		command = "checkBox -q -v ExportClip";
		command += i;
		MGlobal::executeCommand(command,exportClip,false);
		if (exportClip)
		{
			//get clip name
			command = "textField -q -tx ClipName";
			command += i;
			MGlobal::executeCommand(command,clipName,false);
			//get clip range
			command = "radioButtonGrp -q -sl ClipRangeRadio";
			command += i;
			MGlobal::executeCommand(command,rangeType,false);
			if (rangeType == 1)
			{	//range specified from user
				command = "floatField -q -v ClipRangeStart";
				command += i;
				MGlobal::executeCommand(command,startTime,false);
				command = "floatField -q -v ClipRangeEnd";
				command += i;
				MGlobal::executeCommand(command,stopTime,false);
				//get range units
				command = "radioButtonGrp -q -sl ClipRangeUnits";
				command += i;
				MGlobal::executeCommand(command,rangeUnits,false);
				if (rangeUnits == 1)
				{	//range specified in range => convert to seconds
					MTime t1 = startTime;
					MTime t2 = stopTime;
					startTime = t1.as(MTime::kSeconds);
					stopTime = t2.as(MTime::kSeconds);
				}
			}
			else
			{	//range specified by time slider
				MTime t1 = MAnimControl::animationStartTime();
				MTime t2 = MAnimControl::animationEndTime();
				startTime = t1.as(MTime::kSeconds);
				stopTime = t2.as(MTime::kSeconds);
			}
			//get sample rate
			command = "radioButtonGrp -q -sl ClipRateType";
			command += i;
			MGlobal::executeCommand(command,rateType,false);
			MTime t;
			switch (rateType)
			{
			case 1:	//rate specified in frames
				command = "floatField -q -v ClipRateFrames";
				command += i;
				MGlobal::executeCommand(command,rate,false);
				t = rate;
				rate = t.as(MTime::kSeconds);
				break;
			case 2:	//rate specified in seconds
				command = "floatField -q -v ClipRateSeconds";
				command += i;
				MGlobal::executeCommand(command,rate,false);
				break;
			default://rate not specified, get from time slider
				rate = -1;
				break;
			}
			//add clip info
			clipInfo clip;
			clip.name = clipName;
			clip.start = startTime;
			clip.stop = stopTime;
			clip.rate = rate;
			params.clipList.push_back(clip);
			MString msg = "clip ";
			msg += clipName;
			MGlobal::displayInfo(msg);
			msg = "start: ";
			msg += startTime;
			msg += ", stop: ";
			msg += stopTime;
			MGlobal::displayInfo(msg);
			msg = "rate: ";
			msg += rate;
			MGlobal::displayInfo(msg);
			MGlobal::displayInfo("-----------------");
		}
	}

	// Create output files
	params.openFiles();

	// Clear skeleton data
	skeleton.clear();

	// Start mesh description
	if (params.exportMesh)
	{
		params.outMesh << "<mesh>\n";
		params.outMesh << "\t<submeshes>\n";
	}

	/**************************** BEGIN TRANSLATION **********************************/
	// NOTE: the whole scene is translated to a single mesh, internal meshes are subdivided
	//       into submeshes

	if (params.exportAll)
	{	// we are exporting the whole scene
		MItDag dagIter;
		MFnDagNode worldDag (dagIter.root());
		MDagPath worldPath;
		worldDag.getPath(worldPath);
		stat = translateNode(worldPath);
	}
	else
	{	// we are translating a selection
		// get the selection list
		MSelectionList activeList;
		stat = MGlobal::getActiveSelectionList(activeList);
		if (MS::kSuccess != stat)
		{
			MGlobal::displayInfo("Error retrieving selection list");
			return MS::kFailure;
		}
		MItSelectionList iter(activeList);
		
		for ( ; !iter.isDone(); iter.next())
		{								
			MDagPath dagPath;
			stat = iter.getDagPath(dagPath);	
			stat = translateNode(dagPath); 
		}							
	}
		

	// End mesh description
	if (params.exportMesh)
	{
		params.outMesh << "\t</submeshes>\n";
		if (params.exportSkeleton)
		{
			int ri = params.skeletonFilename.rindex('\\');
			int end = params.skeletonFilename.length() - 1;
			MString filename = params.skeletonFilename.substring(ri+1,end);
			if (filename.substring(filename.length()-4,filename.length()-1) == MString(".xml")
				&& filename.length() >= 5)
				filename = filename.substring(0,filename.length()-5);
			params.outMesh << "\t<skeletonlink name=\"" <<  filename.asChar() << "\"/>\n";
		}
		params.outMesh << "</mesh>\n";
	}

	// Write skeleton description
	if (params.exportSkeleton)
	{
		stat = skeleton.writeToXML(params);
		if (MS::kSuccess != stat)
		{
			MGlobal::displayInfo("Error writing skeleton data");
			return MS::kFailure;
		}
	}

	return MS::kSuccess;
}



// Method for iterating over nodes in a dependency graph from top to bottom, translating only meshes
MStatus OgreExporter::translateNode(MDagPath& dagPath)
{
	if (params.exportAnimCurves)
	{
		MItDependencyGraph animIter( dagPath.node(),
			MFn::kAnimCurve,
			MItDependencyGraph::kUpstream,
			MItDependencyGraph::kDepthFirst,
			MItDependencyGraph::kNodeLevel,
			&stat );
		
		if (stat)
		{
			for (; !animIter.isDone(); animIter.next())
			{
				MObject anim = animIter.thisNode(&stat);
				MFnAnimCurve animFn(anim,&stat);
				MString msg = "Found animation curve: ";
				msg += animFn.name();
				MGlobal::displayInfo(msg);
				msg = "Translating animation curve: ";
				msg += animFn.name();
				msg += "...";
				MGlobal::displayInfo(msg);
				stat = writeAnim(animFn);
				if (MS::kSuccess == stat)
					MGlobal::displayInfo("OK");
				else
				{
					MGlobal::displayInfo("Error, Aborting operation");
					return MS::kFailure;
				}
			}
		}
	}
	if (dagPath.hasFn(MFn::kMesh)&&(params.exportMesh||params.exportMaterial||params.exportSkeleton)
		&& (dagPath.childCount() == 0))
	{	// we have found a mesh shape node, it can't have any children, and it contains
		// all the mesh geometry data
		MDagPath meshDag = dagPath;
	//	meshDag.pop(1);
		MFnMesh meshFn(meshDag);
		if (!meshFn.isIntermediateObject())
		{
			MString msg = "Found mesh node: ";
			msg += meshDag.fullPathName();
			MGlobal::displayInfo(msg);
			if (params.exportSkeleton||params.exportVBA)
			{
				MGlobal::displayInfo("Loading skeleton info...");
				// get parent transform to translate
				stat = skeleton.load(meshDag,params);
				if (MS::kSuccess == stat)
					MGlobal::displayInfo("OK");
				else
				{
					MGlobal::displayInfo("Error, Aborting operation\n");
					return MS::kFailure;
				}
			}
			if (params.exportMesh||params.exportMaterial)
			{
				MString msg = "Translating mesh node ";
				msg += meshDag.fullPathName();
				msg += "...";
				MGlobal::displayInfo(msg);
				TransferMesh mesh;
				mesh.load(meshDag,params,&skeleton);
				stat = mesh.writeToXML(params);
				if (MS::kSuccess == stat)
					MGlobal::displayInfo("OK");
				else
				{
					MGlobal::displayInfo("Error, Aborting operation");
					return MS::kFailure;
				}
			}
		}
	}
	else if (dagPath.hasFn(MFn::kCamera)&&(params.exportCameras) && (!dagPath.hasFn(MFn::kShape)))
	{	// we have found a camera shape node, it can't have any children, and it contains
		// all information about the camera
		MFnCamera cameraFn(dagPath);
		if (!cameraFn.isIntermediateObject())
		{
			MString msg = "Found camera node: ";
			msg += dagPath.fullPathName();
			MGlobal::displayInfo(msg);
			msg = "Translating camera node: ";
			msg += dagPath.fullPathName();
			msg += "...";
			MGlobal::displayInfo(msg);
			stat = writeCamera(cameraFn);
			if (MS::kSuccess == stat)
				MGlobal::displayInfo("OK");
			else
			{
				MGlobal::displayInfo("Error, Aborting operation");
				return MS::kFailure;
			}
		}
	}
	// look for meshes and cameras within the node's children
	for (unsigned int i=0; i<dagPath.childCount(); i++)
	{
		MObject child = dagPath.child(i);
		MDagPath childPath;
		stat = MDagPath::getAPathTo(child,childPath);
		if (MS::kSuccess != stat)
		{
			MString msg = "Error retrieving path to child ";
			msg += (int)i;
			msg += " of: ";
			msg += dagPath.fullPathName();
			MGlobal::displayInfo(msg);
			return MS::kFailure;
		}
		stat = translateNode(childPath);
		if (MS::kSuccess != stat)
			return MS::kFailure;
	}
	return MS::kSuccess;
}



/********************************************************************************************************
 *                       Method to translate a single animation curve                                   *
 ********************************************************************************************************/
MStatus OgreExporter::writeAnim(MFnAnimCurve& anim)
{
	params.outAnim << "anim " << anim.name().asChar() << "\n";
	params.outAnim <<"{\n";
	params.outAnim << "\t//Time   /    Value\n";

	for (unsigned int i=0; i<anim.numKeys(); i++)
		params.outAnim << "\t" << anim.time(i).as(MTime::kSeconds) << "\t" << anim.value(i) << "\n";

	params.outAnim << "}\n\n";
	return MS::kSuccess;
}



/********************************************************************************************************
 *                           Method to translate a single camera                                        *
 ********************************************************************************************************/
MStatus OgreExporter::writeCamera(MFnCamera& camera)
{
	MPlug plug;
	MPlugArray srcplugarray;
	double dist;
	MAngle angle;
	MFnTransform* cameraTransform = NULL;
	MFnAnimCurve* animCurve = NULL;
	// get camera transform
	for (unsigned int i=0; i<camera.parentCount(); i++)
	{
		if (camera.parent(i).hasFn(MFn::kTransform))
		{
			cameraTransform = new MFnTransform(camera.parent(i));
			continue;
		}
	}
	// start camera description
	params.outCameras << "camera " << cameraTransform->partialPathName().asChar() << "\n";
	params.outCameras << "{\n";

	//write camera type
	params.outCameras << "\ttype ";
	if (camera.isOrtho())
		params.outCameras << "ortho\n";
	else
		params.outCameras << "persp\n";

	// write translation data
	params.outCameras << "\ttranslation\n";
	params.outCameras << "\t{\n";
	params.outCameras << "\t\tx ";
	plug = cameraTransform->findPlug("translateX");
	if (plug.isConnected() && params.exportCamerasAnim)
	{
		plug.connectedTo(srcplugarray,true,false,&stat);
		for (i=0; i < srcplugarray.length(); i++)
		{
			if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
			{
				if (animCurve)
					delete animCurve;
				animCurve = new MFnAnimCurve(srcplugarray[i].node());
				continue;
			}
			else if (i == srcplugarray.length()-1)
			{
				MGlobal::displayInfo("Invalid link to translateX attribute");
				return MS::kFailure;
			}
		}
		params.outCameras << "anim " << animCurve->name().asChar() << "\n";
	}
	else
	{
		plug.getValue(dist);
		params.outCameras << "= " << dist << "\n";
	}
	params.outCameras << "\t\ty ";
	plug = cameraTransform->findPlug("translateY");
	if (plug.isConnected() && params.exportCamerasAnim)
	{
		plug.connectedTo(srcplugarray,true,false,&stat);
		for (i=0; i< srcplugarray.length(); i++)
		{
			if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
			{
				if (animCurve)
					delete animCurve;
				animCurve = new MFnAnimCurve(srcplugarray[i].node());
				continue;
			}
			else if (i == srcplugarray.length()-1)
			{
				MGlobal::displayInfo("Invalid link to translateY attribute");
				return MS::kFailure;
			}
		}
		params.outCameras << "anim " << animCurve->name().asChar() << "\n";
	}
	else
	{
		plug.getValue(dist);
		params.outCameras << "= " << dist << "\n";
	}
	params.outCameras << "\t\tz ";
	plug = cameraTransform->findPlug("translateZ");
	if (plug.isConnected() && params.exportCamerasAnim)
	{
		plug.connectedTo(srcplugarray,true,false,&stat);
		for (i=0; i< srcplugarray.length(); i++)
		{
			if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
			{
				if (animCurve)
					delete animCurve;
				animCurve = new MFnAnimCurve(srcplugarray[i].node());
				continue;
			}
			else if (i == srcplugarray.length()-1)
			{
				MGlobal::displayInfo("Invalid link to translateZ attribute");
				return MS::kFailure;
			}
		}
		params.outCameras << "anim " << animCurve->name().asChar() << "\n";
	}
	else
	{
		plug.getValue(dist);
		params.outCameras << "= " << dist << "\n";
	}
	params.outCameras << "\t}\n";

	// write rotation data
	params.outCameras << "\trotation\n";
	params.outCameras << "\t{\n";
	params.outCameras << "\t\tx ";
	plug = cameraTransform->findPlug("rotateX");
	if (plug.isConnected() && params.exportCamerasAnim)
	{
		plug.connectedTo(srcplugarray,true,false,&stat);
		for (i=0; i< srcplugarray.length(); i++)
		{
			if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
			{
				if (animCurve)
					delete animCurve;
				animCurve = new MFnAnimCurve(srcplugarray[i].node());
				continue;
			}
			else if (i == srcplugarray.length()-1)
			{
				MGlobal::displayInfo("Invalid link to rotateX attribute");
				return MS::kFailure;
			}
		}
		params.outCameras << "anim " << animCurve->name().asChar() << "\n";
	}
	else
	{
		plug.getValue(angle);
		params.outCameras << "= " << angle.asDegrees() << "\n";
	}
	params.outCameras << "\t\ty ";
	plug = cameraTransform->findPlug("rotateY");
	if (plug.isConnected() && params.exportCamerasAnim)
	{
		plug.connectedTo(srcplugarray,true,false,&stat);
		for (i=0; i< srcplugarray.length(); i++)
		{
			if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
			{
				if (animCurve)
					delete animCurve;
				animCurve = new MFnAnimCurve(srcplugarray[i].node());
				continue;
			}
			else if (i == srcplugarray.length()-1)
			{
				MGlobal::displayInfo("Invalid link to rotateY attribute");
				return MS::kFailure;
			}
		}
		params.outCameras << "anim " << animCurve->name().asChar() << "\n";
	}
	else
	{
		plug.getValue(angle);
		params.outCameras << "= " << angle.asDegrees() << "\n";
	}
	params.outCameras << "\t\tz ";
	plug = cameraTransform->findPlug("rotateZ");
	if (plug.isConnected() && params.exportCamerasAnim)
	{
		plug.connectedTo(srcplugarray,true,false,&stat);
		for (i=0; i< srcplugarray.length(); i++)
		{
			if (srcplugarray[i].node().hasFn(MFn::kAnimCurve))
			{
				if (animCurve)
					delete animCurve;
				animCurve = new MFnAnimCurve(srcplugarray[i].node());
				continue;
			}
			else if (i == srcplugarray.length()-1)
			{
				MGlobal::displayInfo("Invalid link to rotateZ attribute");
				return MS::kFailure;
			}
		}
		params.outCameras << "anim " << animCurve->name().asChar() << "\n";
	}
	else
	{
		plug.getValue(angle);
		params.outCameras << "= " << angle.asDegrees() << "\n";
	}
	params.outCameras << "\t}\n";

	// end camera description
	params.outCameras << "}\n\n";
	if (cameraTransform != NULL)
		delete cameraTransform;
	if (animCurve != NULL)
		delete animCurve;
	return MS::kSuccess;
}

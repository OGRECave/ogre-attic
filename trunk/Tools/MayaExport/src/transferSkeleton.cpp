
#include "transferSkeleton.h"

/***** Class TransferSkeleton *****/
// Constructor
TransferSkeleton::TransferSkeleton()
{
	joints.clear();
	animations.clear();
}

// Destructor
TransferSkeleton::~TransferSkeleton()
{
}


// Load skeleton data from a maya mesh Fn
// joints are added to the ones already loaded and assigned an id
// joints with the same name as loaded ones are discarded
MStatus TransferSkeleton::load(MDagPath& meshDag,ParamList& params)
{
	MStatus stat;
	MPlug plug;
	MPlugArray srcplugarray;
	MFnSkinCluster* pSkinCluster;
	MDagPathArray influenceDags;
	MObject	kOutputObject,kObject;
	unsigned int numInfluenceObjs;
	bool foundSkinCluster = false;
	int i;

	m_params = params;
	MFnMesh mesh(meshDag);
	// Get the skin cluster associated to given mesh(if present)
    MItDependencyNodes kDepNodeIt( MFn::kSkinClusterFilter );            
    for( ;!kDepNodeIt.isDone() && !foundSkinCluster; kDepNodeIt.next()) 
	{            
		kObject = kDepNodeIt.item();
		pSkinCluster = new MFnSkinCluster(kObject);
		unsigned int uiNumGeometries = pSkinCluster->numOutputConnections();
		for(unsigned int uiGeometry = 0; uiGeometry < uiNumGeometries; ++uiGeometry ) 
		{
			unsigned int uiIndex = pSkinCluster->indexForOutputConnection(uiGeometry);
			kOutputObject = pSkinCluster->outputShapeAtIndex(uiIndex);
			if(kOutputObject == mesh.object()) 
			{
				MString msg = "Skin cluster found for mesh ";
				msg += mesh.name();
				MGlobal::displayInfo(msg);
				foundSkinCluster = true;
			}
		}
	}

	// Get the list of joints affecting this skin cluster
	if (foundSkinCluster && pSkinCluster)
	{
		numInfluenceObjs = pSkinCluster->influenceObjects(influenceDags,&stat);
		for (i=0; i<numInfluenceObjs; i++)
		{
			if (influenceDags[i].hasFn(MFn::kJoint))
			{
				stat = loadSkeleton(influenceDags[i]);
				if (MS::kSuccess != stat)
				{
					MGlobal::displayInfo("Error reading skeleton data\n");
					return MS::kFailure;
				}
			}
		}
	}
	return MS::kSuccess;
}


// Load skeleton data
// param joint is a joint of requested skeleton
MStatus TransferSkeleton::loadSkeleton(MDagPath& jointDag)
{
	MFnIkJoint jointFn(jointDag);
	MString msg = "Loading skeleton data, root joint: ";
	msg += jointFn.partialPathName();
	MGlobal::displayInfo(msg);
	int i;
	// Get skeleton root
	MDagPath rootDag = jointDag;
	while (jointDag.length())
	{
		jointDag.pop();
		if (jointDag.hasFn(MFn::kJoint))
			rootDag = jointDag;
	}
	MFnIkJoint rootFn(rootDag);

	// Check if skeleton has already been loaded
	for (i=0; i<joints.size(); i++)
	{
		// Skip skeleton if already loaded
		if (rootFn.partialPathName() == joints[i].name)
			return MS::kSuccess;
	}
	// Load joints starting from root
	return loadJoint(rootDag,NULL);
}


// Load a single joint data and iterate through it's children
MStatus TransferSkeleton::loadJoint(MDagPath& nodeDag,joint* parent)
{
	int i;
	joint newJoint;
	joint* parentJoint = parent;
	// if it is a joint node translate it and then proceed to child nodes, otherwise skip it
	// and proceed directly to child nodes
	if (nodeDag.hasFn(MFn::kJoint))
	{
		MFnIkJoint jointFn(nodeDag);
		// Display info
		MString msg = "Loading joint: ";
		msg += jointFn.partialPathName();
		if (parent)
		{
			msg += " (parent: ";
			msg += parent->name;
			msg += ")";
		}
		MGlobal::displayInfo(msg);
		// Get parent index
		int idx=-1;
		if (parent)
		{
			for (i=0; i<joints.size() && idx<0; i++)
			{
				if (joints[i].name == parent->name)
					idx=i;
			}
		}
		// Get Bind Pose Matrix
		// Note: we reset to the bind pose, then get current matrix
		// if bind pose could not be restored we use the current pose as a bind pose
		MSelectionList selectionList;
		MGlobal::getActiveSelectionList(selectionList);
		MGlobal::selectByName(nodeDag.fullPathName(),MGlobal::kReplaceList);
		MGlobal::executeCommand("dagPose -r -g -bp");
		MGlobal::setActiveSelectionList(selectionList,MGlobal::kReplaceList);
		// Get joint matrix
		MMatrix bindMatrix = nodeDag.inclusiveMatrix();
		// Calculate scaling factor inherited by parent
		double scale[3];
		if (parent)
		{
			MTransformationMatrix M(parent->worldMatrix);
			M.getScale(scale,MSpace::kWorld);
		}
		else
		{
			scale[0] = 1;
			scale[1] = 1;
			scale[2] = 1;
		}
		// Calculate Local Matrix
		MMatrix localMatrix;
		if (parent)
			localMatrix = bindMatrix * parent->worldMatrix.inverse();
		else
		{	// root node of skeleton
			if (m_params.exportWorldCoords)
				localMatrix = bindMatrix;
			else
				localMatrix = bindMatrix * nodeDag.exclusiveMatrix().inverse();
		}
		// Calculate rotation data
		double qx,qy,qz,qw;
		((MTransformationMatrix)localMatrix).getRotationQuaternion(qx,qy,qz,qw);
		MQuaternion rotation(qx,qy,qz,qw);
		MVector axis;
		double theta;
		rotation.getAxisAngle(axis,theta);
		axis.normalize();
		if (axis.length() < 0.5)
		{
			axis.x = 0;
			axis.y = 1;
			axis.z = 0;
			theta = 0;
		}
		// Set joint info
		newJoint.name = jointFn.partialPathName();
		newJoint.id = joints.size();
		newJoint.parentIndex = idx;
		newJoint.worldMatrix = bindMatrix;
		newJoint.localMatrix = localMatrix;
		newJoint.posx = localMatrix(3,0) * scale[0];
		newJoint.posy = localMatrix(3,1) * scale[1];
		newJoint.posz = localMatrix(3,2) * scale[2];
		newJoint.angle = theta;
		newJoint.axisx = axis.x;
		newJoint.axisy = axis.y;
		newJoint.axisz = axis.z;
		joints.push_back(newJoint);
		// Load joint animations
		if (m_params.exportAnims)
			loadAnims(nodeDag,joints.size()-1);
		// Restore joint to bind pose
		selectionList;
		MGlobal::getActiveSelectionList(selectionList);
		MGlobal::selectByName(nodeDag.fullPathName(),MGlobal::kReplaceList);
		MGlobal::executeCommand("dagPose -r -g -bp");
		MGlobal::setActiveSelectionList(selectionList,MGlobal::kReplaceList);
		// Get pointer to newly created joint
		parentJoint = &newJoint;
	}

	// Load children joints
	for (i=0; i<nodeDag.childCount();i++)
	{
		MObject child;
		child = nodeDag.child(i);
		MDagPath childDag = nodeDag;
		childDag.push(child);
		loadJoint(childDag,parentJoint);
	}

	return MS::kSuccess;
}


// Load animations
MStatus TransferSkeleton::loadAnims(MDagPath& jointDag,int jointId)
{
	int i;
	MStatus stat;
	MFnIkJoint jointFn(jointDag);
	MObject clipObj;
	MGlobal::displayInfo("Loading joint animations...");
	
	// save current time for later restore
	double curtime = MAnimControl::currentTime().as(MTime::kSeconds);

	// load animation clips for current joint
	for (i=0; i<m_params.clipList.size(); i++)
	{
		stat = loadClip(jointDag,jointId,m_params.clipList[i].name,m_params.clipList[i].start,
			m_params.clipList[i].stop,m_params.clipList[i].rate);
		if (stat == MS::kSuccess)
			MGlobal::displayInfo("Clip successfully loaded");
		else
			MGlobal::displayInfo("Failed loading clip");
	}
	
	//restore current time
	MAnimControl::setCurrentTime(MTime(curtime,MTime::kSeconds));

	return MS::kSuccess;
}


// Load given animation clip for given joint
MStatus TransferSkeleton::loadClip(MDagPath& jointDag,int jointId,MString clipName,double start,double stop,double rate)
{
	MStatus stat;
	int i,j;
	MFnIkJoint jointFn(jointDag);
	MString msg;
	std::vector<double> times,t1;
	std::vector<MFnAnimCurve*> animCurves;
	
	// display clip name
	msg = "animation \"";
	msg += clipName;
	msg += "\"";
	MGlobal::displayInfo(msg);

	// get animation (if it doesn't exist we create it)
	int animIdx = -1;
	for (i=0; i<animations.size(); i++)
	{
		if (animations[i].name == clipName.asChar())
			animIdx = i;
	}
	if (animIdx < 0)
	{
		animation a;
		a.name = clipName.asChar();
		a.tracks.clear();
		a.length = 0;
		animations.push_back(a);
		animIdx = animations.size() - 1;
	}

	// if no rate is specified we get all the keyframes from the connected curves
	if (rate <= 0)
	{
		animCurves.clear();
		// get plugs to check for animation
		MPlugArray plugs;
		plugs.clear();
		plugs.append(jointFn.findPlug("tx"));
		plugs.append(jointFn.findPlug("ty"));
		plugs.append(jointFn.findPlug("tz"));
		plugs.append(jointFn.findPlug("rx"));
		plugs.append(jointFn.findPlug("ry"));
		plugs.append(jointFn.findPlug("rz"));
		plugs.append(jointFn.findPlug("sx"));
		plugs.append(jointFn.findPlug("sy"));
		plugs.append(jointFn.findPlug("sz"));
		// get animation curves for the plugs
		MObjectArray anims;
		for (i=0; i<9; i++)
		{
			// if plug is animated, add the animation curve to the vector
			if (MAnimUtil::findAnimation(plugs[i],anims))
			{
				MFnAnimCurve* pAnimCurve = new MFnAnimCurve(anims[0]);
				animCurves.push_back(pAnimCurve);
			}
			// else add a NULL pointer (to know it's not animated)
			else
				animCurves.push_back(NULL);
		}
		// create list of times in which we have to evaluate curves
		
		if (animCurves[0])
		{
			for (i=0; i<animCurves[0]->numKeys(); i++)
				if ((animCurves[0]->time(i).as(MTime::kSeconds) >= start) &&
					(animCurves[0]->time(i).as(MTime::kSeconds) <= stop))
					times.push_back(animCurves[0]->time(i).as(MTime::kSeconds));
		}
		for (i=1; i<9; i++)
		{
			if (animCurves[i])
			{
				t1.clear();
				for (j=0; j<animCurves[i]->numKeys(); j++)
					if ((animCurves[i]->time(j).as(MTime::kSeconds) >= start) &&
					(animCurves[i]->time(j).as(MTime::kSeconds) <= stop))
						t1.push_back(animCurves[i]->time(j).as(MTime::kSeconds));
				times = mergesorted(times,t1);
			}
		}
	}
	// calculate times from parameters
	else
	{
		for (double t=start; t<=stop; t+=rate)
			times.push_back(t);
	}

	// get animation length
	double length=0;
	if (times.size() > 0)
		length = times[times.size()-1] - times[0];
	if (animations[animIdx].length < length)
		animations[animIdx].length = length;
	// create a track for current clip for current joint
	track animTrack;
	animTrack.bone = jointFn.partialPathName();
	animTrack.keyframes.clear();
	
	// evaluate animation curves at all keyframes
	for (i=0; i<times.size(); i++)
	{
		//evaluate curves at current time and create a keyframe for current track
		MTransformationMatrix matrix;
		MVector position;
		double scale[3];
		scale[0] = 1; scale[1] = 1; scale[2] = 1;
		int parentIdx = joints[jointId].parentIndex;
		//get joint matrix at given time
		//set time to desired time
		MAnimControl::setCurrentTime(MTime(times[i],MTime::kSeconds));
		matrix = jointDag.inclusiveMatrix();
		if (parentIdx >= 0)
		{
			//calculate inherited scale factor
			((MTransformationMatrix)jointDag.exclusiveMatrix()).getScale(scale,MSpace::kWorld);
			//calculate relative matrix
			matrix = jointDag.inclusiveMatrix() * jointDag.exclusiveMatrixInverse();
		}
		else
		{	// root joint
			if (m_params.exportWorldCoords)
				matrix = jointDag.inclusiveMatrix();
			else
				matrix = jointDag.inclusiveMatrix() * jointDag.exclusiveMatrixInverse();
		}
		//calculate position of joint at given time
		position.x = matrix.asMatrix()(3,0) * scale[0];
		position.y = matrix.asMatrix()(3,1) * scale[1];
		position.z = matrix.asMatrix()(3,2) * scale[2];
		//get relative transformation matrix
		matrix = matrix.asMatrix() * joints[jointId].localMatrix.inverse();
		//calculate rotation
		double qx,qy,qz,qw;
		((MTransformationMatrix)matrix).getRotationQuaternion(qx,qy,qz,qw);
		MQuaternion rotation(qx,qy,qz,qw);
		double theta;
		MVector axis;
		rotation.getAxisAngle(axis,theta);
		axis.normalize();
		if (axis.length() < 0.5)
		{
			axis.x = 0;
			axis.y = 1;
			axis.z = 0;
			theta = 0;
		}
		//create keyframe
		keyframe key;
		key.time = times[i] - times[0];
		key.tx = position.x - joints[jointId].posx;
		key.ty = position.y - joints[jointId].posy;
		key.tz = position.z - joints[jointId].posz;
		key.angle = theta;
		key.axis_x = axis.x;
		key.axis_y = axis.y;
		key.axis_z = axis.z;
		key.sx = 1;
		key.sy = 1;
		key.sz = 1;
		//add keyframe to current track
		animTrack.keyframes.push_back(key);
	}
	
	// add created track to current clip
	animations[animIdx].tracks.push_back(animTrack);
	
	if (length >0)
	{
		// display info
		msg = "length: ";
		msg += animations[animIdx].length;
		MGlobal::displayInfo(msg);
		msg = "num keyframes: ";
		msg += (int)animTrack.keyframes.size();
		MGlobal::displayInfo(msg);
	}
	//if the clip has no keyframes we delete it
	else
	{
		MGlobal::displayInfo("animation track has no keyframes, so it's deleted");
		animations[animIdx].tracks.pop_back();
		if (animations[animIdx].tracks.empty())
		{
			animations.erase(animations.begin()+animIdx);
		}
	}

	// free up memory
	for (i=0; i<animCurves.size(); i++)
		delete [] animCurves[i];

	return MS::kSuccess;
}


// Write to xml file specified by params
MStatus TransferSkeleton::writeToXML(ParamList& params)
{
	int i,j,k;
	// Start skeleton description
	params.outSkeleton << "<skeleton>\n";
	// Write bones list
	params.outSkeleton << "\t<bones>\n";
	// For each joint write it's description
	for (i=0; i<joints.size(); i++)
	{
		params.outSkeleton << "\t\t<bone id=\"" << joints[i].id << "\" name=\"" << joints[i].name.asChar() << "\">\n";
		params.outSkeleton << "\t\t\t<position x=\"" << joints[i].posx << "\" y=\"" << joints[i].posy
			<< "\" z=\"" << joints[i].posz << "\"/>\n";
		params.outSkeleton << "\t\t\t<rotation angle=\"" << joints[i].angle << "\">\n";
		params.outSkeleton << "\t\t\t\t<axis x=\"" << joints[i].axisx << "\" y=\"" << joints[i].axisy
			<< "\" z=\"" << joints[i].axisz << "\"/>\n";
		params.outSkeleton << "\t\t\t</rotation>\n";
		params.outSkeleton << "\t\t</bone>\n";
	}
	// End bones list
	params.outSkeleton << "\t</bones>\n";
	// Write bone hierarchy
	params.outSkeleton << "\t<bonehierarchy>\n";
	for (i=0; i<joints.size(); i++)
	{
		if (joints[i].parentIndex>=0)
		{
			params.outSkeleton << "\t\t<boneparent bone=\"" << joints[i].name.asChar() << "\" parent=\""
				<< joints[joints[i].parentIndex].name.asChar() << "\"/>\n";
		}
	}
	params.outSkeleton << "\t</bonehierarchy>\n";

	// Write animations dscription
	if (params.exportAnims)
	{
		params.outSkeleton << "\t<animations>\n";
		// For every animation
		for (i=0; i<animations.size(); i++)
		{
			// Write animation info
			params.outSkeleton << "\t\t<animation name=\"" << animations[i].name.asChar() << "\" length=\"" << 
				animations[i].length << "\">\n";
			// Write tracks
			params.outSkeleton << "\t\t\t<tracks>\n";
			// Cycle through tracks
			for (j=0; j<animations[i].tracks.size(); j++)
			{
				track t = animations[i].tracks[j];
				params.outSkeleton << "\t\t\t\t<track bone=\"" << t.bone.asChar() << "\">\n";
				// Write track keyframes
				params.outSkeleton << "\t\t\t\t\t<keyframes>\n";
				for (k=0; k<t.keyframes.size(); k++)
				{
					// time
					params.outSkeleton << "\t\t\t\t\t\t<keyframe time=\"" << t.keyframes[k].time << "\">\n";
					// translation
					params.outSkeleton << "\t\t\t\t\t\t\t<translate x=\"" << t.keyframes[k].tx << "\" y=\"" <<
						t.keyframes[k].ty << "\" z=\"" << t.keyframes[k].tz << "\"/>\n";
					// rotation
					params.outSkeleton << "\t\t\t\t\t\t\t<rotate angle=\"" << t.keyframes[k].angle << "\">\n";
					params.outSkeleton << "\t\t\t\t\t\t\t\t<axis x=\"" << t.keyframes[k].axis_x << "\" y=\"" <<
						t.keyframes[k].axis_y << "\" z=\"" << t.keyframes[k].axis_z << "\"/>\n";
					params.outSkeleton << "\t\t\t\t\t\t\t</rotate>\n";
					//scale
					params.outSkeleton << "\t\t\t\t\t\t\t<scale x=\"" << t.keyframes[k].sx << "\" y=\"" <<
						t.keyframes[k].sy << "\" z=\"" << t.keyframes[k].sz << "\"/>\n";
					params.outSkeleton << "\t\t\t\t\t\t</keyframe>\n";
				}
				params.outSkeleton << "\t\t\t\t\t</keyframes>\n";
				params.outSkeleton << "\t\t\t\t</track>\n";
			}
			// End tracks description
			params.outSkeleton << "\t\t\t</tracks>\n";
			// End animation description
			params.outSkeleton << "\t\t</animation>\n";
		}
		params.outSkeleton << "\t</animations>\n";
	}
	// End skeleton description
	params.outSkeleton << "</skeleton>\n";
	return MS::kSuccess;
}


// Clear joints info
void TransferSkeleton::clear()
{
	joints.clear();
	animations.clear();
}


// Merge two sorted vectors of doubles
std::vector<double> mergesorted(const std::vector<double>& v1, const std::vector<double>& v2)
{
	std::vector<double> res;
	if (v1.size() == 0)
		res = v2;
	else if (v2.size() == 0)
		res = v1;
	else
	{
		int i=0, j=0, k=0;
		// set starting element
		if (v2[j] >= v1[i])
			res.push_back(v1[i++]);
		else
			res.push_back(v2[j++]);
		// merge the two vectors until one of the two reaches its end
		while (i<v1.size() && j<v2.size())
		{
			if (v1[i]==res[k])
				i++;
			else if (v2[j]==res[k])
				j++;
			else
			{
				if (v2[j] >= v1[i])
				{
					res.push_back(v1[i++]);
					k++;
				}
				else
				{
					res.push_back(v2[j++]);
					k++;
				}
			}
		}
		// merge remaining items
		while (i<v1.size())
		{
			if (v1[i] == res[k])
				i++;
			else
			{
				res.push_back(v1[i++]);
				k++;
			}
		}
		while (j<v2.size())
		{
			if (v2[j] == res[k])
				j++;
			else
			{
				res.push_back(v2[j++]);
				k++;
			}
		}
	}
	return res;
}
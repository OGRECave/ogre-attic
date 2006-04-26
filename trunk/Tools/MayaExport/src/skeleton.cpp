#include "skeleton.h"
#include <maya/MFnMatrixData.h>

namespace OgreMayaExporter
{
	// Constructor
	Skeleton::Skeleton()
	{
		m_joints.clear();
		m_animations.clear();
		m_restorePose = "";
	}


	// Destructor
	Skeleton::~Skeleton()
	{
		clear();
	}


	// Clear skeleton data
	void Skeleton::clear()
	{
		m_joints.clear();
		m_animations.clear();
		m_restorePose = "";
	}


	// Load skeleton data from given skin cluster
	MStatus Skeleton::load(MFnSkinCluster* pSkinCluster,ParamList& params)
	{
		MStatus stat;
		//check for valid skin cluster pointer
		if (!pSkinCluster)
		{
			std::cout << "Could not load skeleton data, no skin cluster specified\n";
			std::cout.flush();
			return MS::kFailure;
		}
		//retrieve and load joints from the skin cluster
		MDagPath jointDag,rootDag;
		MDagPathArray influenceDags;
		int numInfluenceObjs = pSkinCluster->influenceObjects(influenceDags,&stat);
		std::cout << "num influence objects: " << numInfluenceObjs << "\n";
		std::cout.flush();
		for (int i=0; i<numInfluenceObjs; i++)
		{
			jointDag = influenceDags[i];
			if (influenceDags[i].hasFn(MFn::kJoint))
			{
				//retrieve root joint
				rootDag = jointDag;
				while (jointDag.length()>0)
				{
					jointDag.pop();
					if (jointDag.hasFn(MFn::kJoint) && jointDag.length()>0)
						rootDag = jointDag;
				}
				//check if skeleton has already been loaded
				bool skip = false;
				for (int j=0; j<m_joints.size() && !skip; j++)
				{
					//skip skeleton if already loaded
					if (rootDag.partialPathName() == m_joints[j].name)
					{
						skip = true;
					}
				}
				//load joints data from root
				if (!skip)
				{
					// load the skeleton
					std::cout <<  "Loading skeleton with root: " << rootDag.fullPathName().asChar() << "...\n";
					std::cout.flush();
					// save current selection list
					MSelectionList selectionList;
					MGlobal::getActiveSelectionList(selectionList);
					// select the root joint dag
					MGlobal::selectByName(rootDag.fullPathName(),MGlobal::kReplaceList);
					//save current pose (if no pose has been saved yet)
					if (m_restorePose == "")
					{
						MString poseName;
						MGlobal::executeCommand("dagPose -s",poseName,true);
						m_restorePose = poseName;
					}
					//set the skeleton to the desired neutral pose
					switch(params.neutralPoseType)
					{
					case NPT_FRAME:
						MAnimControl::setCurrentTime((double)params.neutralPoseFrame);
						break;
					case NPT_BINDPOSE:
						//disable constraints, IK, etc...
						MGlobal::executeCommand("doEnableNodeItems false all",true);
						// Note: we reset to the bind pose
						MGlobal::executeCommand("dagPose -r -g -bp",true);
						break;
					}
					//load joints data
					stat = loadJoint(rootDag,NULL,params,pSkinCluster);
					if (MS::kSuccess == stat)
					{
						std::cout << "OK\n";
						std::cout.flush();
					}
					else
					{
						std::cout << "Failed\n";
						std::cout.flush();
					}
					//restore selection list
					MGlobal::setActiveSelectionList(selectionList,MGlobal::kReplaceList);
				}
			}
		}

		return MS::kSuccess;
	}


	// Load a joint
	MStatus Skeleton::loadJoint(MDagPath& jointDag,joint* parent,ParamList& params,MFnSkinCluster* pSkinCluster)
	{
		MStatus stat;
		int i;
		joint newJoint;
		joint* parentJoint = parent;
		// if it is a joint node translate it and then proceed to child nodes, otherwise skip it
		// and proceed directly to child nodes
		if (jointDag.hasFn(MFn::kJoint))
		{
			MFnIkJoint jointFn(jointDag);
			// Display info
			std::cout << "Loading joint: " << jointFn.fullPathName().asChar();
			std::cout.flush();
			if (parent)
			{
				std::cout << " (parent: " << parent->name.asChar() << ")\n";
				std::cout.flush();
			}
			else
			{
				std::cout << "\n";
				std::cout.flush();
			}
			// Get parent index
			int idx=-1;
			if (parent)
			{
				for (i=0; i<m_joints.size() && idx<0; i++)
				{
					if (m_joints[i].name == parent->name)
						idx=i;
				}
			}
			// Get world bind matrix
			MMatrix bindMatrix = jointDag.inclusiveMatrix();;
			// Calculate local bind matrix
			MMatrix localMatrix;
			if (parent)
				localMatrix = bindMatrix * parent->bindMatrix.inverse();
			else
			{	// root node of skeleton
				localMatrix = bindMatrix;
			}
			// Get translation
			MVector translation = ((MTransformationMatrix)localMatrix).translation(MSpace::kPostTransform);
			if (fabs(translation.x) < PRECISION)
				translation.x = 0;
			if (fabs(translation.y) < PRECISION)
				translation.y = 0;
			if (fabs(translation.z) < PRECISION)
				translation.z = 0;
			// Calculate rotation data
			double qx,qy,qz,qw;
			((MTransformationMatrix)localMatrix).getRotationQuaternion(qx,qy,qz,qw);
			MQuaternion rotation(qx,qy,qz,qw);
			MVector axis;
			double theta;
			rotation.getAxisAngle(axis,theta);
			if (fabs(axis.x) < PRECISION)
				axis.x = 0;
			if (fabs(axis.y) < PRECISION)
				axis.y = 0;
			if (fabs(axis.z) < PRECISION)
				axis.z = 0;
			axis.normalize();
			if (fabs(theta) < PRECISION)
				theta = 0;
			if (axis.length() < 0.5)
			{
				axis.x = 0;
				axis.y = 1;
				axis.z = 0;
				theta = 0;
			}
			// Get joint scale
			double scale[3];
			((MTransformationMatrix)localMatrix).getScale(scale,MSpace::kPostTransform);
			if (fabs(scale[0]) < PRECISION)
				scale[0] = 0;
			if (fabs(scale[1]) < PRECISION)
				scale[1] = 0;
			if (fabs(scale[2]) < PRECISION)
				scale[2] = 0;
			// Set joint info
			newJoint.name = jointFn.partialPathName();
			newJoint.id = m_joints.size();
			newJoint.parentIndex = idx;
			newJoint.bindMatrix = bindMatrix;
			newJoint.localMatrix = localMatrix;
			newJoint.posx = translation.x * params.lum;
			newJoint.posy = translation.y * params.lum;
			newJoint.posz = translation.z * params.lum;
			newJoint.angle = theta;
			newJoint.axisx = axis.x;
			newJoint.axisy = axis.y;
			newJoint.axisz = axis.z;
			newJoint.scalex = 1;
			newJoint.scaley = 1;
			newJoint.scalez = 1;
			newJoint.jointDag = jointDag;
			m_joints.push_back(newJoint);
			// If root is a root joint, save it's index in the roots list
			if (idx < 0)
			{
				m_roots.push_back(m_joints.size() - 1);
			}
			// Get pointer to newly created joint
			parentJoint = &newJoint;
		}
		// Load child joints
		for (i=0; i<jointDag.childCount();i++)
		{
			MObject child;
			child = jointDag.child(i);
			MDagPath childDag = jointDag;
			childDag.push(child);
			loadJoint(childDag,parentJoint,params,pSkinCluster);
		}
		return MS::kSuccess;
	}


	// Load animations
	MStatus Skeleton::loadAnims(ParamList& params)
	{
		//enable constraints, IK, etc...
		MGlobal::executeCommand("doEnableNodeItems true all",true);
		MStatus stat;
		int i;
		// save current time for later restore
		MTime curTime = MAnimControl::currentTime();
		std::cout << "Loading joint animations...\n";
		std::cout.flush();
		// clear animations list
		m_animations.clear();
		// load animation clips for the whole skeleton
		for (i=0; i<params.clipList.size(); i++)
		{
			stat = loadClip(params.clipList[i].name,params.clipList[i].start,
				params.clipList[i].stop,params.clipList[i].rate,params);
			if (stat == MS::kSuccess)
			{
				std::cout << "Clip successfully loaded\n";
				std::cout.flush();
			}
			else
			{
				std::cout << "Failed loading clip\n";
				std::cout.flush();
			}
		}
		//restore current time
		MAnimControl::setCurrentTime(curTime);
		return MS::kSuccess;
	}

	// Load an animation clip
	MStatus Skeleton::loadClip(MString clipName,double start,double stop,double rate,ParamList& params)
	{
		MStatus stat;
		int i,j;
		MString msg;
		std::vector<double> times;
		// if skeleton has no joints we can't load the clip
		if (m_joints.size() < 0)
			return MS::kFailure;
		// display clip name
		std::cout << "clip \"" << clipName.asChar() << "\"\n";
		std::cout.flush();
		// calculate times from clip sample rate
		times.clear();
		for (double t=start; t<stop; t+=rate)
			times.push_back(t);
		times.push_back(stop);
		// get animation length
		double length=0;
		if (times.size() >= 0)
			length = times[times.size()-1] - times[0];
		if (length < 0)
		{
			std::cout << "invalid time range for the clip, we skip it\n";
			std::cout.flush();
			return MS::kFailure;
		}
		// create the animation
		animation a;
		a.name = clipName.asChar();
		a.tracks.clear();
		a.length = length;
		m_animations.push_back(a);
		int animIdx = m_animations.size() - 1;
		// create a track for current clip for all joints
		std::vector<track> animTracks;
		for (i=0; i<m_joints.size(); i++)
		{
			track t;
			t.bone = m_joints[i].name;
			t.keyframes.clear();
			animTracks.push_back(t);
		}
		// evaluate animation curves at selected times
		for (i=0; i<times.size(); i++)
		{
			//set time to wanted sample time
			MAnimControl::setCurrentTime(MTime(times[i],MTime::kSeconds));
			//load a keyframe for every joint at current time
			for (j=0; j<m_joints.size(); j++)
			{
				keyframe key = loadKeyframe(m_joints[j],times[i]-times[0],params);
				//add keyframe to joint track
				animTracks[j].keyframes.push_back(key);
			}
		}
		// add created tracks to current clip
		for (i=0; i<animTracks.size(); i++)
		{
			m_animations[animIdx].tracks.push_back(animTracks[i]);
		}
		// display info
		std::cout << "length: " << m_animations[animIdx].length << "\n";
		std::cout << "num keyframes: " << animTracks[0].keyframes.size() << "\n";
		std::cout.flush();
		// clip successfully loaded
		return MS::kSuccess;
	}

	// Load a keyframe for a given joint at current time
	keyframe Skeleton::loadKeyframe(joint& j,double time,ParamList& params)
	{
		MVector position;
		int parentIdx = j.parentIndex;
		// Get joint matrix
		MMatrix worldMatrix = j.jointDag.inclusiveMatrix();
		// Calculate Local Matrix
		MMatrix localMatrix;
		if (parentIdx >= 0)
		{
			// Get parent joint
			MDagPath parentDag = m_joints[parentIdx].jointDag;
			localMatrix = worldMatrix * parentDag.inclusiveMatrixInverse();
		}
		else
		{	// Root node of skeleton
		if (params.exportWorldCoords)
			localMatrix = worldMatrix;
		else
			localMatrix = worldMatrix * j.jointDag.exclusiveMatrixInverse();
		}
		// Get relative transformation matrix
		MMatrix relMatrix = localMatrix * j.localMatrix.inverse();
		// Get relative translation
		MVector translation = ((MTransformationMatrix)localMatrix).translation(MSpace::kPostTransform) - 
			((MTransformationMatrix)j.localMatrix).translation(MSpace::kPostTransform);
		if (fabs(translation.x) < PRECISION)
			translation.x = 0;
		if (fabs(translation.y) < PRECISION)
			translation.y = 0;
		if (fabs(translation.z) < PRECISION)
			translation.z = 0;
		// Get relative rotation
		double qx,qy,qz,qw;
		((MTransformationMatrix)relMatrix).getRotationQuaternion(qx,qy,qz,qw);
		MQuaternion rotation(qx,qy,qz,qw);
		MVector axis;
		double theta;
		rotation.getAxisAngle(axis,theta);
		if (fabs(axis.x) < PRECISION)
			axis.x = 0;
		if (fabs(axis.y) < PRECISION)
			axis.y = 0;
		if (fabs(axis.z) < PRECISION)
			axis.z = 0;
		axis.normalize();
		if (fabs(theta) < PRECISION)
			theta = 0;
		if (axis.length() < 0.5)
		{
			axis.x = 0;
			axis.y = 1;
			axis.z = 0;
			theta = 0;
		}
		// Get relative scale
		double scale[3];
		((MTransformationMatrix)relMatrix).getScale(scale,MSpace::kPostTransform);
		if (fabs(scale[0]) < PRECISION)
			scale[0] = 0;
		if (fabs(scale[1]) < PRECISION)
			scale[1] = 0;
		if (fabs(scale[2]) < PRECISION)
			scale[2] = 0;
		//create keyframe
		keyframe key;
		key.time = time;
		key.tx = translation.x * params.lum;
		key.ty = translation.y * params.lum;
		key.tz = translation.z * params.lum;
		key.angle = theta;
		key.axis_x = axis.x;
		key.axis_y = axis.y;
		key.axis_z = axis.z;
		key.sx = scale[0];
		key.sy = scale[1];
		key.sz = scale[2];
		return key;
	}


	// Restore skeleton pose
	void Skeleton::restorePose()
	{
		// save current selection list
		MSelectionList selectionList;
		MGlobal::getActiveSelectionList(selectionList);
		int i;
		for (i=0; i<m_roots.size(); i++)
		{
			MDagPath rootDag = m_joints[m_roots[i]].jointDag;
			// select the root joint dag
			MGlobal::selectByName(rootDag.fullPathName(),MGlobal::kReplaceList);
			// restore pose
			MString cmd = "dagPose -r -g -n \""+ m_restorePose;
			cmd += "\"";
			MGlobal::executeCommand(cmd,true);
		}
		//restore selection list
		MGlobal::setActiveSelectionList(selectionList,MGlobal::kReplaceList);
		//enable constraints, IK, etc...
		MGlobal::executeCommand("doEnableNodeItems true all",true);
	}



	// Get joint list
	std::vector<joint>& Skeleton::getJoints()
	{
		return m_joints;
	}



	// Get animations
	std::vector<animation>& Skeleton::getAnimations()
	{
		return m_animations;
	}



	// Write skeleton data to Ogre XML file
	MStatus Skeleton::writeXML(ParamList &params)
	{
		int i;
		// Start skeleton description
		params.outSkeleton << "<skeleton>\n";

		// Write bones list
		params.outSkeleton << "\t<bones>\n";
		// For each joint write it's description
		for (i=0; i<m_joints.size(); i++)
		{
			params.outSkeleton << "\t\t<bone id=\"" << m_joints[i].id << "\" name=\"" << m_joints[i].name.asChar() << "\">\n";
			params.outSkeleton << "\t\t\t<position x=\"" << m_joints[i].posx << "\" y=\"" << m_joints[i].posy
				<< "\" z=\"" << m_joints[i].posz << "\"/>\n";
			params.outSkeleton << "\t\t\t<rotation angle=\"" << m_joints[i].angle << "\">\n";
			params.outSkeleton << "\t\t\t\t<axis x=\"" << m_joints[i].axisx << "\" y=\"" << m_joints[i].axisy
				<< "\" z=\"" << m_joints[i].axisz << "\"/>\n";
			params.outSkeleton << "\t\t\t</rotation>\n";
			params.outSkeleton << "\t\t\t<scale x=\"" << m_joints[i].scalex << "\" y=\"" << m_joints[i].scaley
				<< "\" z=\"" << m_joints[i].scalez << "\"/>\n";
			params.outSkeleton << "\t\t</bone>\n";
		}
		params.outSkeleton << "\t</bones>\n";

		// Write bone hierarchy
		params.outSkeleton << "\t<bonehierarchy>\n";
		for (i=0; i<m_joints.size(); i++)
		{
			if (m_joints[i].parentIndex>=0)
			{
				params.outSkeleton << "\t\t<boneparent bone=\"" << m_joints[i].name.asChar() << "\" parent=\""
					<< m_joints[m_joints[i].parentIndex].name.asChar() << "\"/>\n";
			}
		}
		params.outSkeleton << "\t</bonehierarchy>\n";

		// Write animations description
		if (params.exportAnims)
		{
			params.outSkeleton << "\t<animations>\n";
			// For every animation
			for (i=0; i<m_animations.size(); i++)
			{
				// Write animation info
				params.outSkeleton << "\t\t<animation name=\"" << m_animations[i].name.asChar() << "\" length=\"" << 
					m_animations[i].length << "\">\n";
				// Write tracks
				params.outSkeleton << "\t\t\t<tracks>\n";
				// Cycle through tracks
				for (int j=0; j<m_animations[i].tracks.size(); j++)
				{
					track t = m_animations[i].tracks[j];
					params.outSkeleton << "\t\t\t\t<track bone=\"" << t.bone.asChar() << "\">\n";
					// Write track keyframes
					params.outSkeleton << "\t\t\t\t\t<keyframes>\n";
					for (int k=0; k<t.keyframes.size(); k++)
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


};	//end namespace
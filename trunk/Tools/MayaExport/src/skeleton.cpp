#include "skeleton.h"

namespace OgreMayaExporter
{
	// Constructor
	Skeleton::Skeleton()
	{
		m_pSkinCluster = NULL;
	}


	// Destructor
	Skeleton::~Skeleton()
	{
		clear();
	}


	// Clear skeleton data
	void Skeleton::clear()
	{
		m_pSkinCluster = NULL;
		m_joints.clear();
		m_animations.clear();
	}


	// Load skeleton data from mgiven skin cluster
	MStatus Skeleton::load(MFnSkinCluster* pSkinCluster,ParamList& params)
	{
		MStatus stat;
		//update skin cluster pointer
		if (pSkinCluster)
			m_pSkinCluster = pSkinCluster;
		else
		{
			std::cout << "Could not load skeleton data, no skin cluster specified\n";
			return MS::kFailure;
		}
		//retrieve and load joints from the skin cluster
		MDagPath jointDag,rootDag;
		MDagPathArray influenceDags;
		int numInfluenceObjs = m_pSkinCluster->influenceObjects(influenceDags,&stat);
		for (int i=0; i<numInfluenceObjs; i++)
		{
			if (influenceDags[i].hasFn(MFn::kJoint))
			{
				//retrieve root joint
				jointDag = influenceDags[i];
				rootDag = jointDag;
				while (jointDag.length())
				{
					jointDag.pop();
					if (jointDag.hasFn(MFn::kJoint))
						rootDag = jointDag;
				}
				//check if skeleton has already been loaded
				bool skip = false;
				for (int j=0; i<m_joints.size() && !skip; j++)
				{
					//skip skeleton if already loaded
					if (rootDag.partialPathName() == m_joints[j].name)
						skip = true;
				}
				//load joints data from root
				if (!skip)
				{
					std::cout <<  "Loading skeleton with root: " << jointDag.partialPathName().asChar() << "...\n";
					MSelectionList selectionList;
					MGlobal::getActiveSelectionList(selectionList);
					// Set Neutral Pose
					//if type is 1 we want the skin bind pose
					if (params.neutralPoseFrame == 1)
					{
						// Note: we reset to the bind pose, then get current matrix
						// if bind pose could not be restored we use the current pose as a bind pose
						MGlobal::selectByName(jointDag.partialPathName(),MGlobal::kReplaceList);
						MGlobal::executeCommand("dagPose -r -g -bp");
					}
					//if type is 2 we want specified frame as neutral pose
					else if (params.neutralPoseType == 2)
					{
						//set time to desired time
						MTime npTime = (double)params.neutralPoseFrame;
						MAnimControl::setCurrentTime(npTime.as(MTime::kSeconds));
					}
					
					//load joints data
					stat = loadJoint(jointDag,NULL,params);
					if (MS::kSuccess == stat)
						std::cout << "OK\n";
					else
						std::cout << "Failed\n";

					//restore skeleton to neutral pose
					if (params.neutralPoseFrame == 1)
					{
						MGlobal::executeCommand("dagPose -r -g -bp");
						MGlobal::setActiveSelectionList(selectionList,MGlobal::kReplaceList);
					}
					else if (params.neutralPoseType == 2)
					{
						//set time to desired time
						MTime npTime = (double)params.neutralPoseFrame;
						MAnimControl::setCurrentTime(npTime.as(MTime::kSeconds));
					}
				}
			}
		}

		return MS::kSuccess;
	}


	// Load a joint
	MStatus Skeleton::loadJoint(MDagPath& jointDag,joint* parent,ParamList& params)
	{
		int i;
		joint newJoint;
		joint* parentJoint = parent;
		// if it is a joint node translate it and then proceed to child nodes, otherwise skip it
		// and proceed directly to child nodes
		if (jointDag.hasFn(MFn::kJoint))
		{
			MFnIkJoint jointFn(jointDag);
			// Display info
			std::cout << "Loading joint: " << jointFn.partialPathName().asChar();
			if (parent)
				std::cout << " (parent: " << parent->name.asChar() << ")\n";
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
			// Get joint matrix
			MMatrix bindMatrix = jointDag.inclusiveMatrix();
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
				if (params.exportWorldCoords)
					localMatrix = bindMatrix;
				else
					localMatrix = bindMatrix * jointDag.exclusiveMatrix().inverse();
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
			newJoint.id = m_joints.size();
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
			m_joints.push_back(newJoint);
			// Load joint animations
			if (params.exportAnims)
				loadAnims(jointDag,m_joints.size()-1,params);
			// Get pointer to newly created joint
			parentJoint = &newJoint;
		}

		// Load children joints
		for (i=0; i<jointDag.childCount();i++)
		{
			MObject child;
			child = jointDag.child(i);
			MDagPath childDag = jointDag;
			childDag.push(child);
			loadJoint(childDag,parentJoint,params);
		}

		return MS::kSuccess;
	}


	// Load animations
	MStatus Skeleton::loadAnims(MDagPath& jointDag,int jointId,ParamList& params)
	{
		int i;
		MStatus stat;
		MFnIkJoint jointFn(jointDag);
		MObject clipObj;
		std::cout << "Loading joint animations...\n";

		// save current time for later restore
		double curtime = MAnimControl::currentTime().as(MTime::kSeconds);

		// load animation clips for current joint
		for (i=0; i<params.clipList.size(); i++)
		{
			stat = loadClip(jointDag,jointId,params.clipList[i].name,params.clipList[i].start,
				params.clipList[i].stop,params.clipList[i].rate,params);
			if (stat == MS::kSuccess)
				std::cout << "Clip successfully loaded\n";
			else
				std::cout << "Failed loading clip\n";
		}

		//restore current time
		MAnimControl::setCurrentTime(MTime(curtime,MTime::kSeconds));

		return MS::kSuccess;
	}


	// Load given animation clip for given joint
	MStatus Skeleton::loadClip(MDagPath& jointDag,int jointId,MString clipName,double start,double stop,
		double rate,ParamList& params)
	{
		MStatus stat;
		int i,j;
		MFnIkJoint jointFn(jointDag);
		MString msg;
		std::vector<double> times,t1;
		std::vector<MFnAnimCurve*> animCurves;

		// display clip name
		std::cout << "animation \"" << clipName.asChar() << "\"\n";

		// get animation (if it doesn't exist we create it)
		int animIdx = -1;
		for (i=0; i<m_animations.size(); i++)
		{
			if (m_animations[i].name == clipName.asChar())
				animIdx = i;
		}
		if (animIdx < 0)
		{
			animation a;
			a.name = clipName.asChar();
			a.tracks.clear();
			a.length = 0;
			m_animations.push_back(a);
			animIdx = m_animations.size() - 1;
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
		if (m_animations[animIdx].length < length)
			m_animations[animIdx].length = length;
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
			int parentIdx = m_joints[jointId].parentIndex;
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
				if (params.exportWorldCoords)
					matrix = jointDag.inclusiveMatrix();
				else
					matrix = jointDag.inclusiveMatrix() * jointDag.exclusiveMatrixInverse();
			}
			//calculate position of joint at given time
			position.x = matrix.asMatrix()(3,0) * scale[0];
			position.y = matrix.asMatrix()(3,1) * scale[1];
			position.z = matrix.asMatrix()(3,2) * scale[2];
			//get relative transformation matrix
			matrix = matrix.asMatrix() * m_joints[jointId].localMatrix.inverse();
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
			key.tx = position.x - m_joints[jointId].posx;
			key.ty = position.y - m_joints[jointId].posy;
			key.tz = position.z - m_joints[jointId].posz;
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
		m_animations[animIdx].tracks.push_back(animTrack);

		if (length >0)
		{
			// display info
			std::cout << "length: " << m_animations[animIdx].length << "\n";
			std::cout << "num keyframes: " << animTrack.keyframes.size() << "\n";
		}
		//if the clip has no keyframes we delete it
		else
		{
			std::cout << "animation track has no keyframes, so it's deleted\n";
			m_animations[animIdx].tracks.pop_back();
			if (m_animations[animIdx].tracks.empty())
			{
				m_animations.erase(m_animations.begin()+animIdx);
			}
		}

		// free up memory
		for (i=0; i<animCurves.size(); i++)
			delete [] animCurves[i];

		return MS::kSuccess;
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
		// Start skeleton description
		params.outSkeleton << "<skeleton>\n";

		// Write bones list
		params.outSkeleton << "\t<bones>\n";
		// For each joint write it's description
		for (int i=0; i<m_joints.size(); i++)
		{
			params.outSkeleton << "\t\t<bone id=\"" << m_joints[i].id << "\" name=\"" << m_joints[i].name.asChar() << "\">\n";
			params.outSkeleton << "\t\t\t<position x=\"" << m_joints[i].posx << "\" y=\"" << m_joints[i].posy
				<< "\" z=\"" << m_joints[i].posz << "\"/>\n";
			params.outSkeleton << "\t\t\t<rotation angle=\"" << m_joints[i].angle << "\">\n";
			params.outSkeleton << "\t\t\t\t<axis x=\"" << m_joints[i].axisx << "\" y=\"" << m_joints[i].axisy
				<< "\" z=\"" << m_joints[i].axisz << "\"/>\n";
			params.outSkeleton << "\t\t\t</rotation>\n";
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

};	//end namespace
#ifndef _SKELETON_H
#define _SKELETON_H

#include "mayaExportLayer.h"
#include "paramList.h"
#include "animation.h"

namespace OgreMayaExporter
{
	/***** structure to hold joint info *****/
	typedef struct jointTag
	{
		MString name;
		int id;
		MMatrix localMatrix;
		MMatrix bindMatrix;
		int parentIndex;
		double posx,posy,posz;
		double angle;
		double axisx,axisy,axisz;
		float scalex,scaley,scalez;
		MDagPath jointDag;
	} joint;


	/*********** Class Skeleton **********************/
	class Skeleton
	{
	public:
		//constructor
		Skeleton();
		//destructor
		~Skeleton();
		//clear skeleton data
		void clear();
		//load skeleton data
		MStatus load(MFnSkinCluster* pSkinCluster,ParamList& params);
		//load skeletal animations
		MStatus loadAnims(ParamList& params);
		//get joints
		std::vector<joint>& getJoints();
		//get animations
		std::vector<Animation>& getAnimations();
		//restore skeleton pose
		void restorePose();
		//write skeleton data to Ogre XML
		MStatus writeXML(ParamList &params);

	protected:
		//load a joint
		MStatus loadJoint(MDagPath& jointDag, joint* parent, ParamList& params,MFnSkinCluster* pSkinCluster);
		//load a clip
		MStatus loadClip(MString clipName,float start,float stop,float rate,ParamList& params);
		//load a keyframe for a particular joint at current time
		skeletonKeyframe loadKeyframe(joint& j,float time,ParamList& params);

		std::vector<joint> m_joints;
		std::vector<Animation> m_animations;
		std::vector<int> m_roots;
		MString m_restorePose;
	};

}	//end namespace

#endif

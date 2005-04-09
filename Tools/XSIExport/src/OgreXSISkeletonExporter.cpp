/*
-----------------------------------------------------------------------------
This source file is part of OGRE 
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under 
the terms of the GNU Lesser General Public License as published by the Free Software 
Foundation; either version 2 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, but WITHOUT 
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with 
this program; if not, write to the Free Software Foundation, Inc., 59 Temple 
Place - Suite 330, Boston, MA 02111-1307, USA, or go to 
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreXSISkeletonExporter.h"
#include "OgreResourceGroupManager.h"
#include "OgreSkeletonManager.h"
#include "OgreSkeleton.h"
#include "OgreBone.h"
#include "OgreAnimation.h"
#include "OgreAnimationTrack.h"
#include "OgreKeyFrame.h"
#include "OgreSkeletonSerializer.h"
#include "OgreQuaternion.h"
#include <xsi_model.h>
#include <xsi_kinematics.h>
#include <xsi_kinematicstate.h>
#include <xsi_math.h>
#include <xsi_rotation.h>
#include <xsi_animationsourceitem.h>
#include <xsi_source.h>
#include <xsi_fcurve.h>
#include <xsi_fcurvekey.h>
#include <xsi_time.h>
#include <xsi_chaineffector.h>
#include <xsi_chainroot.h>
#include <xsi_chainbone.h>
#include <xsi_matrix4.h>
#include <xsi_transformation.h>
#include <xsi_vector3.h>

using namespace XSI;

namespace Ogre
{
	//-----------------------------------------------------------------------------
	XsiSkeletonExporter::XsiSkeletonExporter()
	{
		mXsiSceneRoot = X3DObject(mXsiApp.GetActiveSceneRoot());
		mXSITrackTypeNames["posx"] = XTT_POS_X;
		mXSITrackTypeNames["posy"] = XTT_POS_Y;
		mXSITrackTypeNames["posz"] = XTT_POS_Z;
		mXSITrackTypeNames["rotx"] = XTT_ROT_X;
		mXSITrackTypeNames["roty"] = XTT_ROT_Y;
		mXSITrackTypeNames["rotz"] = XTT_ROT_Z;
		mXSITrackTypeNames["sclx"] = XTT_SCL_X;
		mXSITrackTypeNames["scly"] = XTT_SCL_Y;
		mXSITrackTypeNames["sclz"] = XTT_SCL_Z;
	}
	//-----------------------------------------------------------------------------
	XsiSkeletonExporter::~XsiSkeletonExporter()
	{
	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::exportSkeleton(const String& skeletonFileName, 
		DeformerMap& deformers, float framesPerSecond, AnimationList& animList)
	{
		LogOgreAndXSI(L"** Begin OGRE Skeleton Export **");

		SkeletonPtr skeleton = SkeletonManager::getSingleton().create("export",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		// construct the hierarchy
		buildBoneHierarchy(skeleton.get(), deformers, animList);
		// progress report
		ProgressManager::getSingleton().progress();

		// create animations 
		createAnimations(skeleton.get(), deformers, framesPerSecond, animList);
		// progress report
		ProgressManager::getSingleton().progress();


		SkeletonSerializer ser;
		ser.exportSkeleton(skeleton.get(), skeletonFileName);
		// progress report
		ProgressManager::getSingleton().progress();

		LogOgreAndXSI(L"** OGRE Skeleton Export Complete **");


	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::buildBoneHierarchy(Skeleton* pSkeleton, 
		DeformerMap& deformers, AnimationList& animList)
	{
		/// Copy all entries from map into a list so iterators won't get invalidated
		std::list<DeformerEntry*> deformerList;
		LogOgreAndXSI(L"-- Bones with vertex assignments:");
		for (DeformerMap::iterator i = deformers.begin(); i != deformers.end(); ++i)
		{
			DeformerEntry* deformer = i->second;
			deformerList.push_back(deformer);
			LogOgreAndXSI(deformer->obj.GetName());
		}

		/* XSI allows you to use any object at all as a bone, not just chain elements.
		   A typical choice is a hierarchy of nulls, for example. In order to 
		   build a skeleton hierarchy which represents the actual one, we need
		   to find the relationships between all the deformers that we found.
		   
		   Well do this by navigating up the scene tree from each bone, looking for
		   a match in the existing bone list or creating a new one where we need it
		   to reach the root. We add bones even if they're not assigned vertices
		   because the animation may depend on them. If the
		   traversal hits the scene root this bone is clearly a root bone 
		   (there may be more than one). 
	   */
		for (std::list<DeformerEntry*>::iterator i = deformerList.begin(); i != deformerList.end(); ++i)
		{
			DeformerEntry* deformer = *i;
			if (deformer->parentName.empty())
			{
				linkBoneWithParent(deformer, deformers, deformerList);
			}
		}

		// Now eliminate all bones without any animated kine parameters
		// Need to do this after we've determined all relationships
		for (std::list<DeformerEntry*>::iterator i = deformerList.begin(); i != deformerList.end(); ++i)
		{
			DeformerEntry* deformer = *i;
			validateAsBone(pSkeleton, deformer, deformers, deformerList, animList);
		}

		// Now link
		for (DeformerMap::iterator i = deformers.begin(); i != deformers.end(); ++i)
		{
			DeformerEntry* deformer = i->second;

			// link to parent
			if (!deformer->parentName.empty())
			{
				DeformerMap::iterator p = deformers.find(deformer->parentName);
				assert (p != deformers.end() && "Parent not found");
				assert (deformer->pBone && "Child bone not created");
				assert(p->second->pBone && "Parent bone not created");
				DeformerEntry* parent = p->second;
				parent->pBone->addChild(deformer->pBone);

			}
		}

	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::linkBoneWithParent(DeformerEntry* child, 
		DeformerMap& deformers, std::list<DeformerEntry*>& deformerList)
	{
		X3DObject parent(child->obj.GetParent());
		String childName = XSItoOgre(child->obj.GetName());

		if (child->obj == mXsiSceneRoot /* safety check for start node */)
			return;

		// Check for parenting by a chain end effector
		// These are sneaky little buggers - we actually want to attach the
		// child to the end of the final bone in the chain
		if (parent.IsA(XSI::siChainEffectorID))
		{
			ChainEffector effector(parent);
			CRefArray chainBones = effector.GetRoot().GetBones();
			// get the last
			parent = chainBones[chainBones.GetCount()-1];
			child->parentIsChainEndEffector = true;
			
		}
		// is the parent the scene root?
		if (parent == mXsiSceneRoot)
		{
			// we hit the root node
		}
		else
		{

			String parentName = XSItoOgre(parent.GetName());
			// Otherwise, check to see if the parent is in the deformer list
			DeformerMap::iterator i = deformers.find(parentName);
			DeformerEntry* parentDeformer = 0;
			if (i == deformers.end())
			{
				// not found, create entry for parent 
				parentDeformer = new DeformerEntry(deformers.size(), parent);
				deformers[parentName] = parentDeformer;
				deformerList.push_back(parentDeformer);
				LogOgreAndXSI(CString(L"Added ") + parent.GetName() + 
					CString(L" as a parent of ") + child->obj.GetName() );
			}
			else
			{
				parentDeformer = i->second;
			}

			// Link child entry with parent (not bone yet)
			// link child to parent by name
			child->parentName = parentName;
			parentDeformer->childNames.push_back(childName);


		}

	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::validateAsBone(Skeleton* pSkeleton, 
		DeformerEntry* deformer, 
		DeformerMap& deformers, std::list<DeformerEntry*>& deformerList, 
		AnimationList& animList)
	{
		/* The purpose of this method is to find out whether a node in the 
		   bone hierarchy is animated, and if not, to eliminate it and propagate
		   it's static transform contribution to it's children.
		   We do this because it's quite easy in XSI to build deep bone chains
		   with intermediate points that are only used for manipulation. We
		   don't want to include all of those.
	   */

		// TODO


		// if we weren't static, create bone
		if (!deformer->pBone)
		{
			String name = XSItoOgre(deformer->obj.GetName());
			deformer->pBone = pSkeleton->createBone(name, deformer->boneID);
			MATH::CTransformation trans; 

			if (deformer->parentName.empty())
			{
				// set transform on bone to global transform since no parents
				trans = deformer->obj.GetKinematics().GetGlobal().GetTransform();
			}
			else
			{
				// set transform on bone to local transform (since child)
				trans = deformer->obj.GetKinematics().GetLocal().GetTransform();
			}
			deformer->pBone->setPosition(XSItoOgre(trans.GetTranslation()));
			deformer->pBone->setOrientation(XSItoOgre(trans.GetRotation().GetQuaternion()));
			deformer->pBone->setScale(XSItoOgre(trans.GetScaling()));

			// special case a bone which is parented by a chain end
			if (deformer->parentIsChainEndEffector)
			{
				ChainEffector effector(deformer->obj.GetParent());
				CRefArray chainBones = effector.GetRoot().GetBones();
				// get the last
				X3DObject endBone = chainBones[chainBones.GetCount()-1];
				// offset along X the length of the bone
				double boneLen = endBone.GetParameterValue(L"Length");
				deformer->pBone->setPosition(
					deformer->pBone->getPosition() + Vector3::UNIT_X * boneLen);
			}

		}

	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::processActionSource(const XSI::ActionSource& actSource,
		DeformerMap& deformers)
	{
		// Clear existing deformer links
		for(DeformerMap::iterator di = deformers.begin(); di != deformers.end(); ++di)
		{
			for (int tt = XTT_POS_X; tt < XTT_COUNT; ++tt)
			{
				di->second->xsiTrack[tt].ResetObject();
			}
		}
		// Get all the items
		CRefArray items = actSource.GetItems();
		for (int i = 0; i < items.GetCount(); ++i)
		{
			XSI::AnimationSourceItem item = items[i];

			// Check the target
			String target = XSItoOgre(item.GetTarget());
			size_t firstDotPos = target.find_first_of(".");
			size_t lastDotPos = target.find_last_of(".");
			if (firstDotPos != String::npos && lastDotPos != String::npos)
			{
				String targetName = target.substr(0, firstDotPos);
				String paramName = target.substr(lastDotPos+1, 
					target.size() - lastDotPos - 1);
				// locate deformer
				DeformerMap::iterator di = deformers.find(targetName);
				if (di != deformers.end())
				{
					DeformerEntry* deformer = di->second;
					// determine parameter
					std::map<String, int>::iterator pi = mXSITrackTypeNames.find(paramName);
					if (pi != mXSITrackTypeNames.end())
					{
						deformer->xsiTrack[pi->second] = item;
						deformer->hasAnyTracks = true;
					}
				}
			}

		}
	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::createAnimations(Skeleton* pSkel, 
		DeformerMap& deformers, float fps, AnimationList& animList)
	{
		for (AnimationList::iterator ai = animList.begin(); ai != animList.end(); ++ai)
		{
			AnimationEntry& animEntry = *ai;
			// tease out all the animation source items
			processActionSource(animEntry.source, deformers);

			determineAnimationLength(animEntry);

			float animLength = (float)(animEntry.endFrame - animEntry.startFrame) / fps;
			StringUtil::StrStreamType str;
			str << "Creating animation " << animEntry.animationName << 
				" with length " << animLength << " seconds";
			LogOgreAndXSI(str.str());
			Animation* anim = pSkel->createAnimation(animEntry.animationName, animLength);

			createAnimationTracks(anim, animEntry, deformers, fps);
			
		}
	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::determineAnimationLength(AnimationEntry& animEntry)
	{
		bool first = true;
		CRefArray items = animEntry.source.GetItems();
		for (int i = 0; i < items.GetCount(); ++i)
		{
			XSI::AnimationSourceItem item = items[i];

			// skip invalid or non-FCurve items
			if (!item.IsValid() || !item.GetSource().IsA(XSI::siFCurveID))
				continue;

			FCurve fcurve = item.GetSource();
			CRefArray keys = fcurve.GetKeys();
			for (int k = 0; k < keys.GetCount(); ++k)
			{
				long currFrame = fcurve.GetKeyTime(k).GetTime();
				if (first)
				{
					animEntry.startFrame = currFrame;
					animEntry.endFrame = currFrame;
					first = false;
				}
				else 
				{
					if (currFrame < animEntry.startFrame)
					{
						animEntry.startFrame = currFrame;
					}
					if (currFrame > animEntry.endFrame)
					{
						animEntry.endFrame = currFrame;
					}
				}

			}
		}



	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::buildKeyframeList(DeformerEntry* deformer, 
		AnimationEntry& animEntry)
	{
		bool first = true;
		animEntry.frames.clear();
		for (int tt = XTT_POS_X; tt < XTT_COUNT; ++tt)
		{
			AnimationSourceItem item = deformer->xsiTrack[tt];
			// skip invalid or non-FCurve items
			if (!item.IsValid() || !item.GetSource().IsA(XSI::siFCurveID))
				continue;
			
			FCurve fcurve = item.GetSource();
			CRefArray keys = fcurve.GetKeys();
			for (int k = 0; k < keys.GetCount(); ++k)
			{
				long currFrame = fcurve.GetKeyTime(k).GetTime();
				if (first)
				{
					animEntry.startFrame = currFrame;
					animEntry.endFrame = currFrame;
					first = false;
				}
				else 
				{
					if (currFrame < animEntry.startFrame)
					{
						animEntry.startFrame = currFrame;
					}
					if (currFrame > animEntry.endFrame)
					{
						animEntry.endFrame = currFrame;
					}
				}
				
				animEntry.frames.insert(currFrame);
			}
		}
	


	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::createAnimationTracks(Animation* pAnim, 
		AnimationEntry& animEntry, DeformerMap& deformers, float fps)
	{
		/* We have to iterate over the list of deformers, and create a track
		 * for each one. Since XSI stores keys for all 9 components separately,
		 * we need to bake OGRE keyframes (which include position, rotation and 
		 * translation) by interpolation. We can merge the list of frames from all
		 * action source item fcurves, then use the Eval method to get XSI to sample
		 * the curve properly for us.
		 * We will also use this to ensure there is a keyframe at the start and end of the
		 * animation.
		 */
		for (DeformerMap::iterator di = deformers.begin(); di != deformers.end(); ++di)
		{
			DeformerEntry* deformer = di->second;

			// Skip deformers which have no animated parameters
			if (!deformer->hasAnyTracks)
				continue;

			StringUtil::StrStreamType str;
			str << "Creating track for bone " << deformer->pBone->getName() << 
				"(" << deformer->boneID << ")";
			LogOgreAndXSI(str.str());
			// create track
			AnimationTrack* track = pAnim->createTrack(deformer->boneID, deformer->pBone);

			XSI::MATH::CTransformation initialTransformation;
			if (deformer->pBone->getParent() == 0)
			{
				// Based on global
				initialTransformation = 
					deformer->obj.GetKinematics().GetGlobal().GetTransform();
			}
			else
			{
				// Based on local
				initialTransformation = 
					deformer->obj.GetKinematics().GetLocal().GetTransform();
			}
			XSI::MATH::CMatrix4 invTrans = initialTransformation.GetMatrix4();
			invTrans.InvertInPlace();

			double initposx, initposy, initposz;
			initialTransformation.GetTranslationValues(initposx, initposy, initposz);
			double initrotx, initroty, initrotz;
			initialTransformation.GetRotation().GetXYZAngles(initrotx, initroty, initrotz);
			double initsclx, initscly, initsclz;
			initialTransformation.GetScalingValues(initsclx, initscly, initsclz);


			// Get the keyframe numbers from all XSI tracks
			// XSI tracks might be sparse
			buildKeyframeList(deformer, animEntry);

			// Iterate over the frames and pull out the values we need
			// bake keyframe for all
			for (std::set<long>::iterator fi = animEntry.frames.begin(); 
				fi != animEntry.frames.end(); ++fi)
			{
				double posx = deriveKeyFrameValue(deformer->xsiTrack[XTT_POS_X], *fi, initposx);
				double posy = deriveKeyFrameValue(deformer->xsiTrack[XTT_POS_Y], *fi, initposy);
				double posz = deriveKeyFrameValue(deformer->xsiTrack[XTT_POS_Z], *fi, initposz);
				double rotx = deriveKeyFrameValue(deformer->xsiTrack[XTT_ROT_X], *fi, initrotx);
				double roty = deriveKeyFrameValue(deformer->xsiTrack[XTT_ROT_Y], *fi, initroty);
				double rotz = deriveKeyFrameValue(deformer->xsiTrack[XTT_ROT_Z], *fi, initrotz);
				double sclx = deriveKeyFrameValue(deformer->xsiTrack[XTT_SCL_X], *fi, initsclx);
				double scly = deriveKeyFrameValue(deformer->xsiTrack[XTT_SCL_Y], *fi, initscly);
				double sclz = deriveKeyFrameValue(deformer->xsiTrack[XTT_SCL_Z], *fi, initsclz);




				XSI::MATH::CTransformation transformation;

				if (deformer->pBone->getParent() != 0)
				{
					transformation.SetTranslationFromValues(posx, posy, posz);
					transformation.SetRotationFromXYZAnglesValues(
						XSI::MATH::DegreesToRadians(rotx),
						XSI::MATH::DegreesToRadians(roty),
						XSI::MATH::DegreesToRadians(rotz),
						XSI::MATH::CRotation::RotationOrder::siXYZ);
					XSI::MATH::CVector3 scaling(sclx, scly, sclz);
					transformation.SetScaling(scaling);


					XSI::MATH::CMatrix4 transformationMatrix = transformation.GetMatrix4();
					transformationMatrix.MulInPlace(invTrans);
					transformation.SetMatrix4(transformationMatrix);
				}

				// create keyframe
				KeyFrame* kf = track->createKeyFrame((float)(*fi - 1) / fps);
				kf->setTranslate(XSItoOgre(transformation.GetTranslation()));
				kf->setRotation(XSItoOgre(transformation.GetRotationQuaternion()));
				kf->setScale(XSItoOgre(transformation.GetScaling()));


			}
		}


	}
	//-----------------------------------------------------------------------------
	double XsiSkeletonExporter::deriveKeyFrameValue(
		XSI::AnimationSourceItem item, long frame, double defaultVal)
	{
		if (item.IsValid())
		{
			FCurve curve(item.GetSource());
			// let fcurve evaluate
			return curve.Eval(CTime(frame));
		}
		else
		{
			return defaultVal;
		}
	}
}

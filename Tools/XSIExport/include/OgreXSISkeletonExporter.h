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
#ifndef __XSISKELETONEXPORTER_H__
#define __XSISKELETONEXPORTER_H__

#include "OgrePrerequisites.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreXSIHelper.h"
#include <xsi_x3dobject.h>
#include <xsi_string.h>
#include <xsi_application.h>
#include <xsi_actionsource.h>

namespace Ogre {

	/** Class for performing a skeleton export from XSI.
	*/
	class XsiSkeletonExporter
	{
	public:
		XsiSkeletonExporter();
		virtual ~XsiSkeletonExporter();


		/** Export a skeleton to the provided filename.
		@param skeletonFileName The file name to export to
		@param deformers The list of deformers (bones) found during mesh traversal
		@param framesPerSecond The number of frames per second
		@param animList List of animation splits
		*/
		void exportSkeleton(const String& skeletonFileName, 
			DeformerList& deformers, float framesPerSecond, 
			AnimationList& animList);
	protected:
		// XSI Objects
		XSI::Application mXsiApp;
		XSI::X3DObject mXsiSceneRoot;
		std::map<String, int> mXSITrackTypeNames; 

		/// Build the bone hierarchy from a simple list of bones
		void buildBoneHierarchy(Skeleton* pSkeleton, DeformerList& deformers);
		/** Link the current bone with it's parent
		@returns True if it linked, false otherwise
		*/
		bool linkBoneWithParent(Skeleton* pSkeleton, XSI::X3DObject& child, DeformerList& deformers);
		/*
		/// Find all the action sources in the scene for the list of deformers
		void findActionSources(DeformerList& deformers);
		/// Find all the action sources against the given model for the list of deformers
		void findActionSources(const XSI::Model& obj, DeformerList& deformers);
		*/
		/// Process an action source
		void processActionSource(const XSI::ActionSource& source, DeformerList& deformers);
		/// Bake animations
		void createAnimations(Skeleton* pSkel, DeformerList& deformers, 
			float framesPerSecond, AnimationList& animList);
		/// Bake animation tracks, and return the time length found
		void createAnimationTracks(Animation* pAnim, AnimationEntry& animEntry, 
			DeformerList& deformers, float fps);
		/// Pre-parse the deformers animation to find the highest keyframe number
		long getMaxKeyFrame(DeformerList& deformerList);
		/// Derive a keyframe value from XSI's tracks
		float deriveKeyFrameValue(XSI::AnimationSourceItem item, long frame);
		

	};



}


#endif

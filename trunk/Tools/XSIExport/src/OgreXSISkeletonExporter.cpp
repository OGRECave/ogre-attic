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
#include <xsi_model.h>

using namespace XSI;

namespace Ogre
{
	//-----------------------------------------------------------------------------
	XsiSkeletonExporter::XsiSkeletonExporter()
	{
		mXsiSceneRoot = X3DObject(mXsiApp.GetActiveSceneRoot());
	}
	//-----------------------------------------------------------------------------
	XsiSkeletonExporter::~XsiSkeletonExporter()
	{
	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::exportSkeleton(const String& skeletonFileName, 
		DeformerList& deformers)
	{
		SkeletonPtr skeleton = SkeletonManager::getSingleton().create("export",
			ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		// construct the hierarchy
		buildBoneHierarchy(skeleton.get(), deformers);


		SkeletonSerializer ser;
		ser.exportSkeleton(skeleton.get(), skeletonFileName);


	}
	//-----------------------------------------------------------------------------
	void XsiSkeletonExporter::buildBoneHierarchy(Skeleton* pSkeleton, 
		DeformerList& deformers)
	{
		/* XSI allows you to use any object at all as a bone, not just chain elements.
		   A typical choice is a hierarchy of nulls, for example. In order to 
		   build a skeleton hierarchy which represents the actual one, we need
		   to find the relationships between all the deformers that we found.
		   We also need to include any objects which are between 2 bones, e.g.
		   if the bone hierarchy is A-B-C, but only bones A and C had envelopes
		   assigned to them, we still need to export B - so we need to pick that
		   one up during this pass.
		   Well do this by navigating up the scene tree from each bone, looking for
		   a match in the existing bone list. We'll keep a track of the bones 
		   we're traversing so that we can include intermediate bones. If the
		   traversal hits the scene root without finding another bone, this bone
		   is clearly a root bone (there may be more than one). 
	   */
		// Store current list size, to detect creation of new intermediate bones
		size_t listSize = deformers.size();
		for (DeformerList::iterator i = deformers.begin(); i != deformers.end(); ++i)
		{
			DeformerEntry* deformer = i->second;
			if (deformer->parentName.empty())
			{
				linkBoneWithParent(pSkeleton, deformer->obj, deformers);
			}
			if (deformers.size() != listSize)
			{
				// list has changed, iterator will be invalid
				// reset - we won't process the completed ones again
				i = deformers.begin();
				listSize = deformers.size();
			}
		}
		// Now that we've created all the bones, including transitive ones
		// we can link the Bone hierarchy
		for (DeformerList::iterator i = deformers.begin(); i != deformers.end(); ++i)
		{
			DeformerEntry* deformer = i->second;
			// link to parent
			if (!deformer->parentName.empty())
			{
				DeformerList::iterator p = deformers.find(deformer->parentName);
				assert (p != deformers.end() && deformer->pBone && p->second->pBone);
				DeformerEntry* parent = p->second;
				parent->pBone->addChild(deformer->pBone);

			}
		}

	}
	//-----------------------------------------------------------------------------
	bool XsiSkeletonExporter::linkBoneWithParent(Skeleton* pSkeleton, 
		X3DObject& child, DeformerList& deformers)
	{
		X3DObject parent(child.GetParent());
		String childName = XSItoOgre(child.GetName());
		String parentName = XSItoOgre(parent.GetName());

		// is the parent the scene root?
		if (parent == mXsiSceneRoot 
			|| child == mXsiSceneRoot /* safety check for start node */)
		{
			// clearly we didn't find any matching bones
			// create bone
			DeformerList::iterator c = deformers.find(childName);
			if (c != deformers.end())
			{
				DeformerEntry* deformer = c->second;
				if (!deformer->pBone)
				{
					deformer->pBone = pSkeleton->createBone(childName, deformer->boneID);
				}
			}

			return false;
		}

		// Otherwise, check to see if the parent is in the deformer list
		DeformerList::iterator i = deformers.find(XSItoOgre(parent.GetName()));
		bool createLink = false;
		if (i == deformers.end())
		{
			// not found, check higher
			createLink = linkBoneWithParent(pSkeleton, parent, deformers);
		}
		else
		{
			// ok, found one, add it
			createLink = true;
		}

		if (createLink)
		{
			// Link child with parent
			// Check child is present first (will not be if transient)
			DeformerList::iterator c = deformers.find(childName);
			DeformerEntry* deformer = 0;
			if (c == deformers.end())
			{
				deformer = new DeformerEntry(deformers.size(), child);
				deformers[childName] = deformer;
			}
			else
			{
				deformer = c->second;
			}
			// link child to parent
			deformer->parentName = parentName;
			// create bone
			if (!deformer->pBone)
			{
				deformer->pBone = pSkeleton->createBone(childName, deformer->boneID);
			}


		}

		return createLink;


	}
}
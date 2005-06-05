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
#include "OgreStableHeaders.h"
#include "OgreAnimation.h"
#include "OgreKeyFrame.h"
#include "OgreException.h"
#include "OgreSkeleton.h"
#include "OgreBone.h"
#include "OgreEntity.h"
#include "OgreSubEntity.h"

namespace Ogre {

    Animation::InterpolationMode Animation::msDefaultInterpolationMode = Animation::IM_LINEAR;
    Animation::RotationInterpolationMode 
        Animation::msDefaultRotationInterpolationMode = Animation::RIM_LINEAR;
    //---------------------------------------------------------------------
    Animation::Animation(const String& name, Real length) : mName(name), mLength(length)
    {
        mInterpolationMode = Animation::msDefaultInterpolationMode;
        mRotationInterpolationMode = Animation::msDefaultRotationInterpolationMode;
    }
    //---------------------------------------------------------------------
    Animation::~Animation()
    {
        destroyAllTracks();
    }
    //---------------------------------------------------------------------
    Real Animation::getLength(void) const
    {
        return mLength;
    }
    //---------------------------------------------------------------------
    NodeAnimationTrack* Animation::createNodeTrack(unsigned short handle)
    {
        NodeAnimationTrack* ret = new NodeAnimationTrack(this);

        mNodeTrackList[handle] = ret;
        return ret;
    }
    //---------------------------------------------------------------------
    NodeAnimationTrack* Animation::createNodeTrack(unsigned short handle, Node* node)
    {
        NodeAnimationTrack* ret = createNodeTrack(handle);

        ret->setAssociatedNode(node);

        return ret;
    }
    //---------------------------------------------------------------------
    unsigned short Animation::getNumNodeTracks(void) const
    {
        return (unsigned short)mNodeTrackList.size();
    }
    //---------------------------------------------------------------------
    NodeAnimationTrack* Animation::getNodeTrack(unsigned short handle) const
    {
        NodeTrackList::const_iterator i = mNodeTrackList.find(handle);

        if (i == mNodeTrackList.end())
        {
            OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot find node track with the specified handle", 
                "Animation::getNodeTrack");
        }

        return i->second;

    }
    //---------------------------------------------------------------------
    void Animation::destroyNodeTrack(unsigned short handle)
    {
        NodeTrackList::iterator i = mNodeTrackList.find(handle);

		if (i != mNodeTrackList.end())
		{
			delete i->second;
			mNodeTrackList.erase(i);
		}
    }
    //---------------------------------------------------------------------
    void Animation::destroyAllNodeTracks(void)
    {
        NodeTrackList::iterator i;
        for (i = mNodeTrackList.begin(); i != mNodeTrackList.end(); ++i)
        {
            delete i->second;
        }
        mNodeTrackList.clear();
    }
	//---------------------------------------------------------------------
	NumericAnimationTrack* Animation::createNumericTrack(unsigned short handle)
	{
		NumericAnimationTrack* ret = new NumericAnimationTrack(this);

		mNumericTrackList[handle] = ret;
		return ret;
	}
	//---------------------------------------------------------------------
	NumericAnimationTrack* Animation::createNumericTrack(unsigned short handle, 
		const AnimableValuePtr& anim)
	{
		NumericAnimationTrack* ret = createNumericTrack(handle);

		ret->setAssociatedAnimable(anim);

		return ret;
	}
	//---------------------------------------------------------------------
	unsigned short Animation::getNumNumericTracks(void) const
	{
		return (unsigned short)mNumericTrackList.size();
	}
	//---------------------------------------------------------------------
	NumericAnimationTrack* Animation::getNumericTrack(unsigned short handle) const
	{
		NumericTrackList::const_iterator i = mNumericTrackList.find(handle);

		if (i == mNumericTrackList.end())
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find Numeric track with the specified handle", 
				"Animation::getNumericTrack");
		}

		return i->second;

	}
	//---------------------------------------------------------------------
	void Animation::destroyNumericTrack(unsigned short handle)
	{
		NumericTrackList::iterator i = mNumericTrackList.find(handle);

		if (i != mNumericTrackList.end())
		{
			delete i->second;
			mNumericTrackList.erase(i);
		}
	}
	//---------------------------------------------------------------------
	void Animation::destroyAllNumericTracks(void)
	{
		NumericTrackList::iterator i;
		for (i = mNumericTrackList.begin(); i != mNumericTrackList.end(); ++i)
		{
			delete i->second;
		}
		mNumericTrackList.clear();
	}
	//---------------------------------------------------------------------
	VertexAnimationTrack* Animation::createVertexTrack(unsigned short handle)
	{
		VertexAnimationTrack* ret = new VertexAnimationTrack(this);

		mVertexTrackList[handle] = ret;
		return ret;

	}
	//---------------------------------------------------------------------
	VertexAnimationTrack* Animation::createVertexTrack(unsigned short handle, 
		VertexData* data)
	{
		VertexAnimationTrack* ret = createVertexTrack(handle);

		ret->setAssociatedVertexData(data);

		return ret;
	}
	//---------------------------------------------------------------------
	unsigned short Animation::getNumVertexTracks(void) const
	{
		return (unsigned short)mVertexTrackList.size();
	}
	//---------------------------------------------------------------------
	VertexAnimationTrack* Animation::getVertexTrack(unsigned short handle) const
	{
		VertexTrackList::const_iterator i = mVertexTrackList.find(handle);

		if (i == mVertexTrackList.end())
		{
			OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
				"Cannot find Vertex track with the specified handle", 
				"Animation::getVertexTrack");
		}

		return i->second;

	}
	//---------------------------------------------------------------------
	void Animation::destroyVertexTrack(unsigned short handle)
	{
		VertexTrackList::iterator i = mVertexTrackList.find(handle);

		if (i != mVertexTrackList.end())
		{
			delete i->second;
			mVertexTrackList.erase(i);
		}
	}
	//---------------------------------------------------------------------
	void Animation::destroyAllVertexTracks(void)
	{
		VertexTrackList::iterator i;
		for (i = mVertexTrackList.begin(); i != mVertexTrackList.end(); ++i)
		{
			delete i->second;
		}
		mVertexTrackList.clear();
	}
	//---------------------------------------------------------------------
	void Animation::destroyAllTracks(void)
	{
		destroyAllNodeTracks();
		destroyAllNumericTracks();
		destroyAllVertexTracks();
	}
    //---------------------------------------------------------------------
    const String& Animation::getName(void) const
    {
        return mName;
    }
    //---------------------------------------------------------------------
	void Animation::apply(Real timePos, Real weight, bool accumulate, Real scale)
    {
        NodeTrackList::iterator i;
        for (i = mNodeTrackList.begin(); i != mNodeTrackList.end(); ++i)
        {
            i->second->apply(timePos, weight, accumulate, scale);
        }
		NumericTrackList::iterator j;
		for (j = mNumericTrackList.begin(); j != mNumericTrackList.end(); ++j)
		{
			j->second->apply(timePos, weight, accumulate, scale);
		}
		VertexTrackList::iterator k;
		for (k = mVertexTrackList.begin(); k != mVertexTrackList.end(); ++k)
		{
			k->second->apply(timePos, weight, accumulate, scale);
		}

    }
    //---------------------------------------------------------------------
    void Animation::apply(Skeleton* skel, Real timePos, Real weight, 
		bool accumulate, Real scale)
    {
        NodeTrackList::iterator i;
        for (i = mNodeTrackList.begin(); i != mNodeTrackList.end(); ++i)
        {
            // get bone to apply to 
            Bone* b = skel->getBone(i->first);
            i->second->applyToNode(b, timePos, weight, accumulate, scale);
        }


    }
	//---------------------------------------------------------------------
	void Animation::apply(Entity* entity, Real timePos, 
		VertexAnimationTrack::TargetMode targetMode)
	{
		VertexTrackList::iterator i;
		for (i = mVertexTrackList.begin(); i != mVertexTrackList.end(); ++i)
		{
			unsigned short handle = i->first;
			VertexData* vertexData;
			TempBlendedBufferInfo* tmpInfo;
			if (handle == 0)
			{
				// shared vertex data
				vertexData = entity->_getMorphAnimVertexData();
				tmpInfo = entity->_getMorphAnimTempBufferInfo();

			}
			else
			{
				// sub entity vertex data (-1)
				vertexData = 
					entity->getSubEntity(handle - 1)->_getMorphAnimVertexData();
				tmpInfo = 
					entity->getSubEntity(handle - 1)->_getMorphAnimTempBufferInfo();
			}
			i->second->setTargetMode(targetMode);
			i->second->applyToVertexData(vertexData, tmpInfo, timePos);
		}

	}
    //---------------------------------------------------------------------
    void Animation::setInterpolationMode(InterpolationMode im)
    {
        mInterpolationMode = im;
    }
    //---------------------------------------------------------------------
    Animation::InterpolationMode Animation::getInterpolationMode(void) const
    {
        return mInterpolationMode;
    }
    //---------------------------------------------------------------------
    void Animation::setDefaultInterpolationMode(InterpolationMode im)
    {
        msDefaultInterpolationMode = im;
    }
    //---------------------------------------------------------------------
    Animation::InterpolationMode Animation::getDefaultInterpolationMode(void)
    {
        return msDefaultInterpolationMode;
    }
    //---------------------------------------------------------------------
    const Animation::NodeTrackList& Animation::_getNodeTrackList(void) const
    {
        return mNodeTrackList;

    }
	//---------------------------------------------------------------------
	const Animation::NumericTrackList& Animation::_getNumericTrackList(void) const
	{
		return mNumericTrackList;
	}
    //---------------------------------------------------------------------
    void Animation::setRotationInterpolationMode(RotationInterpolationMode im)
    {
        mRotationInterpolationMode = im;
    }
    //---------------------------------------------------------------------
    Animation::RotationInterpolationMode Animation::getRotationInterpolationMode(void) const
    {
        return mRotationInterpolationMode;
    }
    //---------------------------------------------------------------------
    void Animation::setDefaultRotationInterpolationMode(RotationInterpolationMode im)
    {
        msDefaultRotationInterpolationMode = im;
    }
    //---------------------------------------------------------------------
    Animation::RotationInterpolationMode Animation::getDefaultRotationInterpolationMode(void)
    {
        return msDefaultRotationInterpolationMode;
    }
    //---------------------------------------------------------------------
	void Animation::optimise(void)
	{
		// Iterate over the node tracks and identify those with no useful keyframes
		std::list<unsigned short> tracksToDestroy;
        NodeTrackList::iterator i;
        for (i = mNodeTrackList.begin(); i != mNodeTrackList.end(); ++i)
        {
			NodeAnimationTrack* track = i->second;
			if (!track->hasNonZeroKeyFrames())
			{
				// mark the entire track for destruction
				tracksToDestroy.push_back(i->first);
			}
			else
			{
				track->optimise();
			}
			
		}
		
		// Now destroy the tracks we marked for death
		for(std::list<unsigned short>::iterator h = tracksToDestroy.begin();
			h != tracksToDestroy.end(); ++h)
		{
			destroyNodeTrack(*h);
		}
		
	}

}



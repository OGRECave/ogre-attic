/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2002 The OGRE Team
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
#include "OgreAnimationTrack.h"
#include "OgreException.h"
#include "OgreSkeleton.h"
#include "OgreBone.h"

namespace Ogre {

    Animation::InterpolationMode Animation::msDefaultInterpolationMode = Animation::IM_LINEAR;
    //---------------------------------------------------------------------
    Animation::Animation(const String& name, Real length) : mName(name), mLength(length)
    {
        mInterpolationMode = Animation::msDefaultInterpolationMode;
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
    AnimationTrack* Animation::createTrack(unsigned short handle)
    {
        AnimationTrack* ret;

        ret = new AnimationTrack(this);

        mTrackList[handle] = ret;
        return ret;
    }
    //---------------------------------------------------------------------
    AnimationTrack* Animation::createTrack(unsigned short handle, Node* node)
    {
        AnimationTrack* ret = createTrack(handle);

        ret->setAssociatedNode(node);

        return ret;
    }
    //---------------------------------------------------------------------
    unsigned short Animation::getNumTracks(void) const
    {
        return (unsigned short)mTrackList.size();
    }
    //---------------------------------------------------------------------
    AnimationTrack* Animation::getTrack(unsigned short handle) const
    {
        TrackList::const_iterator i = mTrackList.find(handle);

        if (i == mTrackList.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot find track with the specified handle", 
                "Animation::getTrackByHandle");
        }

        return i->second;

    }
    //---------------------------------------------------------------------
    void Animation::destroyTrack(unsigned short handle)
    {
        TrackList::iterator i = mTrackList.find(handle);

        delete i->second;

        mTrackList.erase(i);
    }
    //---------------------------------------------------------------------
    void Animation::destroyAllTracks(void)
    {
        TrackList::iterator i;
        for (i = mTrackList.begin(); i != mTrackList.end(); ++i)
        {
            delete i->second;
        }
        mTrackList.clear();
    }
    //---------------------------------------------------------------------
    const String& Animation::getName(void) const
    {
        return mName;
    }
    //---------------------------------------------------------------------
	void Animation::apply(Real timePos, Real weight, bool accumulate)
    {
        TrackList::iterator i;
        for (i = mTrackList.begin(); i != mTrackList.end(); ++i)
        {
            i->second->apply(timePos, weight, accumulate);
        }


    }
    //---------------------------------------------------------------------
    void Animation::apply(Skeleton* skel, Real timePos, Real weight, bool accumulate)
    {
        TrackList::iterator i;
        for (i = mTrackList.begin(); i != mTrackList.end(); ++i)
        {
            // get bone to apply to 
            Bone* b = skel->getBone(i->first);
            i->second->applyToNode(b, timePos, weight, accumulate);
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
    const Animation::TrackList& Animation::_getTrackList(void) const
    {
        return mTrackList;

    }

}



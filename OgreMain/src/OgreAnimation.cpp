/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreAnimation.h"
#include "OgreKeyFrame.h"
#include "OgreAnimationTrack.h"
#include "OgreBoneTrack.h"

namespace Ogre {

    //---------------------------------------------------------------------
    Animation::Animation(Real length) : mLength(length)
    {
    }
    //---------------------------------------------------------------------
    Animation::~Animation()
    {
        destroyAllTracks();
    }
    //---------------------------------------------------------------------
    Real Animation::getLength(void)
    {
        return mLength;
    }
    //---------------------------------------------------------------------
    AnimationTrack* Animation::createTrack(const String& typeName)
    {
        AnimationTrack* ret;
        // TODO: do the selection of track type through plugin factory objects?
        if (typeName == "Bone")
        {
            ret = new BoneTrack(this);
        }

        mTrackList.push_back(ret);
        return ret;
    }
    //---------------------------------------------------------------------
    unsigned short Animation::getNumTracks(void)
    {
        return (unsigned short)mTrackList.size();
    }
    //---------------------------------------------------------------------
    AnimationTrack* Animation::getTrack(unsigned short index)
    {
        assert (index >= 0 && index < mTrackList.size() && 
            "Track index out of bounds");
        return mTrackList[index];
    }
    //---------------------------------------------------------------------
    void Animation::destroyTrack(unsigned short index)
    {
        TrackList::iterator i = mTrackList.begin();

        i += index;

        delete *i;

        mTrackList.erase(i);
    }
    //---------------------------------------------------------------------
    void Animation::destroyAllTracks(void)
    {
        TrackList::iterator i;
        for (i = mTrackList.begin(); i != mTrackList.end(); ++i)
        {
            delete *i;
        }
        mTrackList.clear();
    }
    //---------------------------------------------------------------------


}



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
#include "OgreAnimationTrack.h"
#include "OgreAnimation.h"
#include "OgreKeyFrame.h"

namespace Ogre {

    //---------------------------------------------------------------------
    AnimationTrack::AnimationTrack(Animation* parent) : mParent(parent)
    {
        // Create first keyframe
        createKeyFrame(0.0);
    }
    //---------------------------------------------------------------------
    AnimationTrack::~AnimationTrack()
    {
        removeAllKeyFrames();
    }
    //---------------------------------------------------------------------
    unsigned short AnimationTrack::getNumKeyFrames(void)
    {
        return (unsigned short)mKeyFrames.size();
    }
    //---------------------------------------------------------------------
    KeyFrame* AnimationTrack::getKeyFrame(unsigned short index)
    {
        assert (index >= 0 && index < mKeyFrames.size() && 
            "KeyFrame index out of bounds");
        return mKeyFrames[index];
    }
    //---------------------------------------------------------------------
    Real AnimationTrack::getKeyFramesAtTime(Real timePos, KeyFrame** keyFrame1, KeyFrame** keyFrame2)
    {
        Real totalAnimationLength = mParent->getLength();

        // Wrap time 
        while (timePos >= totalAnimationLength)
        {
            timePos -= totalAnimationLength;
        }

        KeyFrameList::iterator i = mKeyFrames.begin();
        while ((*i)->getTime() > timePos)
        {
            ++i;
        }

        // Parametric time
        Real t1, t2;
        // Get first and increment to next
        *keyFrame1 = *i++;
        // If no next keyframe, wrap back to first
        if (i == mKeyFrames.end())
        {
            *keyFrame2 = mKeyFrames[0];
            t2 = totalAnimationLength;
        }
        else
        {
            *keyFrame2 = *i;
            t2 = (*keyFrame2)->getTime();
        }

        t1 = (*keyFrame1)->getTime();

        if (t1 == t2)
        {
            // Same KeyFrame (only one)
            return 0.0;
        }
        else
        {
            return (timePos - t1) / (t2 - t1);
        }
    }
    //---------------------------------------------------------------------
    KeyFrame* AnimationTrack::createKeyFrame(Real timePos)
    {
        KeyFrame* kf = new KeyFrame(timePos);

        // Insert at correct location
        if (timePos > mMaxKeyFrameTime || (timePos == 0 && mKeyFrames.empty()))
        {
            // Quick insert at end
            mKeyFrames.push_back(kf);
            mMaxKeyFrameTime = timePos;
        }
        else
        {
            // Search 
            KeyFrameList::iterator i = mKeyFrames.begin();
            while ((*i)->getTime() > timePos && i != mKeyFrames.end())
            {
                ++i;
            }
            mKeyFrames.insert(i, kf);
        }

        return kf;

    }
    //---------------------------------------------------------------------
    void AnimationTrack::removeKeyFrame(unsigned short index)
    {
        assert (index >= 0 && index < mKeyFrames.size() && 
            "KeyFrame index out of bounds");

        KeyFrameList::iterator i = mKeyFrames.begin();

        i += index;

        delete *i;

        mKeyFrames.erase(i);

    }
    //---------------------------------------------------------------------
    void AnimationTrack::removeAllKeyFrames(void)
    {
        KeyFrameList::iterator i = mKeyFrames.begin();

        for (; i != mKeyFrames.end(); ++i)
        {
            delete *i;
        }
        mKeyFrames.clear();

    }
    //---------------------------------------------------------------------
}


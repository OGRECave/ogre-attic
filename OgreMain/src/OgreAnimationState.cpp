/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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

#include "OgreAnimationState.h"

namespace Ogre 
{

    //---------------------------------------------------------------------
    AnimationState::AnimationState()
    {
        mTimePos = 0;
        mLength = 0;
        mInvLength = 0;
        mWeight = 1.0;

    }
	//---------------------------------------------------------------------
	AnimationState::~AnimationState()
	{
	}
    //---------------------------------------------------------------------
    AnimationState::AnimationState(const String& animName, Real timePos, Real length, Real weight, bool enabled)
        : mAnimationName(animName), mTimePos(timePos), mWeight(weight), mEnabled(enabled)
    {
        setLength(length);
    }
    //---------------------------------------------------------------------
    String AnimationState::getAnimationName() const
    {
        return mAnimationName;
    }
    //---------------------------------------------------------------------
    void AnimationState::setAnimationName(const String& name)
    {
        mAnimationName = name;
    }
    //---------------------------------------------------------------------
    Real AnimationState::getTimePosition(void) const
    {
        return mTimePos;
    }
    //---------------------------------------------------------------------
    void AnimationState::setTimePosition(Real timePos)
    {
        mTimePos = timePos;
    }
    //---------------------------------------------------------------------
    Real AnimationState::getLength() const
    {
        return mLength;
    }
    //---------------------------------------------------------------------
    void AnimationState::setLength(Real len)
    {
        mLength = len;
        if (len != 0)
        {
            mInvLength = 1/len;
        }
        else
        {
            mInvLength = 0;
        }
    }
    //---------------------------------------------------------------------
    Real AnimationState::getWeight(void) const
    {
        return mWeight;
    }
    //---------------------------------------------------------------------
    void AnimationState::setWeight(Real weight)
    {
        mWeight = weight;
    }
    //---------------------------------------------------------------------
    void AnimationState::addTime(Real offset)
    {
        mTimePos = mTimePos + offset;

        // Wrap over upper bound
        while (mTimePos >= mLength)
        {
            mTimePos -= mLength;
        }

        // Wrap over lower bound
        while (mTimePos < 0)
        {
            mTimePos += mLength;
        }
    }
    //---------------------------------------------------------------------
    bool AnimationState::getEnabled(void) const
    {
        return mEnabled;
    }
    //---------------------------------------------------------------------
    void AnimationState::setEnabled(bool enabled)
    {
        mEnabled = enabled;
    }
    //---------------------------------------------------------------------
    bool AnimationState::operator==(const AnimationState& rhs) const
    {
        if (mAnimationName == rhs.mAnimationName &&
            mEnabled == rhs.mEnabled &&
            mTimePos == rhs.mTimePos &&
            mWeight == rhs.mWeight &&
            mLength == rhs.mLength)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    //---------------------------------------------------------------------
    bool AnimationState::operator!=(const AnimationState& rhs) const
    {
        return !(*this == rhs);
    }
    //---------------------------------------------------------------------
    Real AnimationState::getValue(void) const
    {
        return mTimePos * mInvLength;
    }
    //---------------------------------------------------------------------
    void AnimationState::setValue(Real value)
    {
        mTimePos = value * mLength;
    }



}


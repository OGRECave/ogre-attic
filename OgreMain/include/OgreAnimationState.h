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

#ifndef __AnimationState_H__
#define __AnimationState_H__

#include "OgrePrerequisites.h"

namespace Ogre {

    /** Represents the state of an animation and the weight of it's influence. */
    class _OgreExport AnimationState
    {
    public:
        /** Default constructor*/
        AnimationState();
        /** Constructor.
        @param animName The name of the animation.
        @param timePos The position of the animation in seconds
        @param length The total length of the animation
        @param weight Weight to apply (1.0 = full weight)
        */
        AnimationState(const String& animName, Real timePos, Real length, Real weight = 1.0);

    protected:
        String mAnimationName;
        Real mTimePos;
        Real mLength;
        Real mWeight;

    };

    // Define a set of animation states
    typedef std::vector<AnimationState> AnimationStateSet;


}

#endif


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

#ifndef __AnimationSet_H__
#define __AnimationSet_H__

#include "OgrePrerequisites.h"

#include "OgreString.h"
#include "OgreController.h"

namespace Ogre {

    /** Represents the state of an animation and the weight of it's influence. 
    @remarks
        Other classes can hold instances of this class to store the state of any animations
        they are using.
        This class implements the ControllerValue interface to enable automatic update of
        animation state through controllers.
    */
    class _OgreExport AnimationState : public ControllerValue
    {
    public:
        /// Default constructor for STL only
        AnimationState();
		/** Destructor - is here because class has virtual functions and some compilers 
			would whine if it won't exist.
		*/
		virtual ~AnimationState();
        
        /// Normal constructor with all params supplied
        AnimationState(const String& animName, Real timePos, Real length, Real weight = 1.0, bool enabled = false);
        /// Gets the name of the animation to which this state applies
        String getAnimationName() const;
        /// Sets the name of the animation to which this state applies
        void setAnimationName(const String& name);
        /// Gets the time position for this animation
        Real getTimePosition(void) const;
        /// Sets the time position for this animation
        void setTimePosition(Real timePos);
        /// Gets the total length of this animation (may be shorter than whole animation)
        Real getLength() const;
        /// Sets the total length of this animation (may be shorter than whole animation)
        void setLength(Real len);
        /// Gets the weight (influence) of this animation
        Real getWeight(void) const;
        /// Sets the weight (influence) of this animation
        void setWeight(Real weight);
        /// Modifies the time position, adjusting for animation length
        void addTime(Real offset);

        /// Returns true if this animation is currently enabled
        bool getEnabled(void) const;
        /// Sets whether this animation is enabled
        void setEnabled(bool enabled);

        /// Equality operator
        bool operator==(const AnimationState& rhs) const;
        // Inequality operator
        bool operator!=(const AnimationState& rhs) const;

        /** ControllerValue implementation. */
        Real getValue(void);

        /** ControllerValue implementation. */
        void setValue(Real value);

     
    protected:
        String mAnimationName;
        Real mTimePos;
        Real mLength;
        Real mInvLength;
        Real mWeight;
        bool mEnabled;

    };

    // A set of animation states
    typedef std::map<String, AnimationState> AnimationStateSet;


}

#endif


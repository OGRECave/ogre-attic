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

#ifndef __Animation_H__
#define __Animation_H__

#include "OgrePrerequisites.h"
#include "OgreString.h"

namespace Ogre {

    /** An animation sequence. 
    @remarks
        This class defines the interface for a sequence of animation, whether that
        be animation of a mesh, a path along a spline, or possibly more than one
        type of animation in one. An animation is made up of many 'tracks', which are
        the more specific types of animation.
    @par
        You should not create these animations directly. They will be created via a parent
        object which owns the animation, e.g. Skeleton.
    */
    class _OgreExport Animation
    {

    public:
        /** The types of animation interpolation available. */
        enum InterpolationMode
        {
            /** Values are interpolated along straight lines. */
            IM_LINEAR,
            /** Values are interpolated along a spline, resulting in smoother changes in direction. */
            IM_SPLINE
        };

        /** You should not use this constructor directly, use the parent object such as Skeleton instead.
        @param name The name of the animation, should be unique within it's parent (e.g. Skeleton)
        @param length The length of the animation in seconds.
        */
        Animation(const String& name, Real length);
        virtual ~Animation();

        /** Gets the name of this animation. */
        String getName(void) const;

        /** Gets the total length of the animation. */
        Real getLength(void) const;

        /** Creates an AnimationTrack. 
        @param handle Numeric handle to give the track, used for accessing the track later. 
            Must be unique within this Animation.
        */
        AnimationTrack* createTrack(unsigned short handle);

        /** Creates a new AnimationTrack automatically associated with a Node. 
        @remarks
            This method creates a standard AnimationTrack, but also associates it with a
            target Node which will receive all keyframe effects.
        @param handle Numeric handle to give the track, used for accessing the track later. 
            Must be unique within this Animation.
        @param node A pointer to the Node object which will be affected by this track
        */
        AnimationTrack* createTrack(unsigned short handle, Node* node);

        /** Gets the number of AnimationTrack objects which make up this animation. */
        unsigned short getNumTracks(void) const;

        /** Gets a track by it's handle. */
        AnimationTrack* getTrack(unsigned short handle) const;


        /** Destroys the track with the given handle. */
        void destroyTrack(unsigned short handle);

        /** Removes and destroys all tracks making up this animation. */
        void destroyAllTracks(void);

        /** Applies an animation given a specific time point and weight.
        @remarks
            Where you have associated animation tracks with Node objects, you can eaily apply
            an animation to those nodes by calling this method.
        @param timePos The time position in the animation to apply.
        @param weight The influence to give to this track, 1.0 for full influence, less to blend with
          other animations.
        */
        void apply(Real timePos, Real weight = 1.0);


        /** Tells the animation how to interpolate between keyframes.
        @remarks
            By default, animations normally interpolate linearly between keyframes. This is
            fast, but when animations include quick changes in direction it can look a little
            unnatural because directions change instantly at keyframes. An alternative is to
            tell the animation to interpolate along a spline, which is more expensive in terms
            of calculation time, but looks smoother because major changes in direction are 
            distributed around the keyframes rather than just at the keyframe.
        @par
            You can also change the default animation behaviour by calling 
            Animation::setDefaultInterpolationMode.
        */
        void setInterpolationMode(InterpolationMode im);

        /** Gets the current interpolation mode of this animation. 
        @remarks
            See setInterpolationMode for more info.
        */
        InterpolationMode getInterpolationMode(void) const;

        // Methods for setting the defaults
        /** Sets the default animation interpolation mode. 
        @remarks
            Every animation created after this option is set will have the new interpolation
            mode specified. You can also change the mode per animation by calling the 
            setInterpolationMode method on the instance in question.
        */
        static void setDefaultInterpolationMode(InterpolationMode im);

        /** Gets the default interpolation mode for all animations. */
        static InterpolationMode getDefaultInterpolationMode(void);


        typedef std::map<unsigned short, AnimationTrack*> TrackList;

        /// Fast access to NON-UPDATEABLE track list
        const TrackList& _getTrackList(void) const;



    protected:
        /// Tracks, indexed by handle
        TrackList mTrackList;
        String mName;

        Real mLength;

        InterpolationMode mInterpolationMode;

        static InterpolationMode msDefaultInterpolationMode;

        
    };



}


#endif


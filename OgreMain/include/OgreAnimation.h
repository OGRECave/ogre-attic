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
        /** You should not use this constructor directly, use the parent object such as Skeleton instead.
        @param name The name of the animation, should be unique within it's parent (e.g. Skeleton)
        @param length The length of the animation in seconds.
        */
        Animation(const String& name, Real length);
        virtual ~Animation();

        /** Gets the name of this animation. */
        String getName(void);

        /** Gets the total length of the animation. */
        Real getLength(void);

        /** Creates an AnimationTrack. 
        @remarks
            An animation track is the vehicle for actually applying a sequence of keyframes 
            to something. The type of the track depends on the kind of animation you
            want to do, e.g. "Bone" creates a BoneTrack object.
        @param typeName The type of track to create
        @param handle Numeric handle to give the track, used for accessing the track later. 
            Must be unique within this Animation.
        */
        AnimationTrack* createTrack(const String& typeName, unsigned short handle);

        /** Gets the number of AnimationTrack objects which make up this animation. */
        unsigned short getNumTracks(void);

        /** Gets a track by it's handle. */
        AnimationTrack* getTrack(unsigned short handle);


        /** Destroys the track with the given handle. */
        void destroyTrack(unsigned short handle);

        /** Removes and destroys all tracks making up this animation. */
        void destroyAllTracks(void);


    protected:
        /// Tracks, indexed by handle
        typedef std::map<unsigned short, AnimationTrack*> TrackList;
        TrackList mTrackList;
        String mName;

        Real mLength;

        
    };



}


#endif


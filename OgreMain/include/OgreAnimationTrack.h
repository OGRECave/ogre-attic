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

#ifndef __AnimationTrack_H__
#define __AnimationTrack_H__

#include "OgrePrerequisites.h"
#include "OgreSimpleSpline.h"
#include "OgreRotationalSpline.h"

namespace Ogre 
{
    /** A 'track' in an animation sequence, ie a sequence of keyframes which affect a
        certain type of animable object.
    @remarks
        This class is intended as a base for more complete classes which will actually
        animate specific types of object, e.g. a bone in a skeleton to affect
        skeletal animation. An animation will likely include multiple tracks each of which
        can be made up of many KeyFrame instances. Note that the use of tracks allows each animable
        object to have it's own number of keyframes, i.e. you do not have to have the
        maximum number of keyframes for all animable objects just to cope with the most
        animated one.
    @remarks
        Since the most common animable object is a Node, there are options in this class for associating
        the track with a Node which will receive keyframe updates automatically when the 'apply' method
        is called.
    */
    class _OgreExport AnimationTrack
    {
    public:
        /// Constructor
        AnimationTrack(Animation* parent);
        /// Constructor, associates with a Node
        AnimationTrack(Animation* parent, Node* targetNode);

        virtual ~AnimationTrack();

        /** Returns the number of keyframes in this animation. */
        unsigned short getNumKeyFrames(void) const;

        /** Returns the KeyFrame at the specified index. */
        KeyFrame* getKeyFrame(unsigned short index) const;

        /** Gets the 2 KeyFrame objects which are active at the time given, and the blend value between them.
        @remarks
            At any point in time  in an animation, there are either 1 or 2 keyframes which are 'active',
            1 if the time index is exactly on a keyframe, 2 at all other times i.e. the keyframe before
            and the keyframe after.
        @par
            This method returns those keyframes given a time index, and also returns a parametric
            value indicating the value of 't' representing where the time index falls between them.
            E.g. if it returns 0, the time index is exactly on keyFrame1, if it returns 0.5 it is
            half way between keyFrame1 and keyFrame2 etc.
        @param timePos The time index in seconds.
        @param keyFrame1 Pointer to a KeyFrame pointer which will receive the pointer to the 
            keyframe just before or at this time index.
        @param keyFrame2 Pointer to a KeyFrame pointer which will receive the pointer to the 
            keyframe just after this time index. 
        @param firstKeyIndex Pointer to an unsigned short which, if supplied, will receive the 
            index of the 'from' keyframe incase the caller needs it.
        @returns Parametric value indicating how far along the gap between the 2 keyframes the timePos
            value is, e.g. 0.0 for exactly at 1, 0.25 for a quarter etc. By definition the range of this 
            value is:  0.0 <= returnValue < 1.0 .
        */
        Real getKeyFramesAtTime(Real timePos, KeyFrame** keyFrame1, KeyFrame** keyFrame2,
            unsigned short* firstKeyIndex = 0) const;

        /** Creates a new KeyFrame and adds it to this animation at the given time index.
        @remarks
            It is better to create KeyFrames in time order. Creating them out of order can result 
            in expensive reordering processing. Note that a KeyFrame at time index 0.0 is always created
            for you, so you don't need to create this one, just access it using getKeyFrame(0);
        @param timePos The time from which this KeyFrame will apply.
        */
        KeyFrame* createKeyFrame(Real timePos);

        /** Removes a KeyFrame by it's index. */
        void removeKeyFrame(unsigned short index);

        /** Removes all the KeyFrames from this track. */
        void removeAllKeyFrames(void);


        /** Gets a KeyFrame object which contains the interpolated transforms at the time index specified.
        @remarks
            The KeyFrame objects held by this class are transformation snapshots at 
            discrete points in time. Normally however, you want to interpolate between these
            keyframes to produce smooth movement, and this method allows you to do this easily.
            In animation terminology this is called 'tweening'. 
        @param timeIndex The time (in relation to the whole animation sequence)
        @returns A new keyframe object containing the interpolated transforms. Note that the
            position and scaling transforms are linearly interpolated (lerp), whilst the rotation is
            spherically linearly interpolated (slerp) for the most natural result.
        */
        KeyFrame getInterpolatedKeyFrame(Real timeIndex) const;

        /** Applies an animation track at a certain position to the target node.
        @remarks
            When a track has bee associated with a target node, you can eaisly apply the animation
            to the target by calling this method.
        @param timePos The time position in the animation to apply.
        @param weight The influence to give to this track, 1.0 for full influence, less to blend with
          other animations.
        */
        void apply(Real timePos, Real weight = 1.0);

        /** Returns a pointer to the associated Node object (if any). */
        Node* getAssociatedNode(void) const;

        /** Sets the associated Node object which will be automatically affected by calls to 'apply'. */
        void setAssociatedNode(Node* node);

        /** As the 'apply' method but applies to a specified Node instead of associated node. */
        void applyToNode(Node* node, Real timePos, Real weight = 1.0);


    protected:
        typedef std::vector<KeyFrame*> KeyFrameList;
        KeyFrameList mKeyFrames;
        Real mMaxKeyFrameTime;
        Animation* mParent;
        Node* mTargetNode;

        // Flag indicating we need to rebuild the splines next time
        void buildInterpolationSplines(void) const;

        // Prebuilt splines, must be mutable since lazy-update in const method
        mutable bool mSplineBuildNeeded;
        mutable SimpleSpline mPositionSpline;
        mutable SimpleSpline mScaleSpline;
        mutable RotationalSpline mRotationSpline;
      

    };
}

#endif

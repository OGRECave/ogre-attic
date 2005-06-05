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

#ifndef __AnimationTrack_H__
#define __AnimationTrack_H__

#include "OgrePrerequisites.h"
#include "OgreSimpleSpline.h"
#include "OgreRotationalSpline.h"
#include "OgreKeyFrame.h"
#include "OgreAnimable.h"

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
	@remarks
		By default rotation is done using shortest-path algorithm.
		It is possible to change this behaviour using
		setUseShortestRotationPath() method.
    */
    class _OgreExport AnimationTrack
    {
    public:
        /// Constructor
        AnimationTrack(Animation* parent);

        virtual ~AnimationTrack();

        /** Returns the number of keyframes in this animation. */
        virtual unsigned short getNumKeyFrames(void) const;

        /** Returns the KeyFrame at the specified index. */
        virtual KeyFrame* getKeyFrame(unsigned short index) const;

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
        virtual Real getKeyFramesAtTime(Real timePos, KeyFrame** keyFrame1, KeyFrame** keyFrame2,
            unsigned short* firstKeyIndex = 0) const;

        /** Creates a new KeyFrame and adds it to this animation at the given time index.
        @remarks
            It is better to create KeyFrames in time order. Creating them out of order can result 
            in expensive reordering processing. Note that a KeyFrame at time index 0.0 is always created
            for you, so you don't need to create this one, just access it using getKeyFrame(0);
        @param timePos The time from which this KeyFrame will apply.
        */
        virtual KeyFrame* createKeyFrame(Real timePos);

        /** Removes a KeyFrame by it's index. */
        virtual void removeKeyFrame(unsigned short index);

        /** Removes all the KeyFrames from this track. */
        virtual void removeAllKeyFrames(void);


        /** Gets a KeyFrame object which contains the interpolated transforms at the time index specified.
        @remarks
            The KeyFrame objects held by this class are transformation snapshots at 
            discrete points in time. Normally however, you want to interpolate between these
            keyframes to produce smooth movement, and this method allows you to do this easily.
            In animation terminology this is called 'tweening'. 
        @param timeIndex The time (in relation to the whole animation sequence)
        @param kf Keyframe object to store results
        */
        virtual void getInterpolatedKeyFrame(Real timeIndex, KeyFrame* kf) const = 0;

        /** Applies an animation track to the designated target.
        @param timePos The time position in the animation to apply.
        @param weight The influence to give to this track, 1.0 for full influence, less to blend with
          other animations.
		@param acculumate Don't make weights relative to overall weights applied, 
			make them absolute and just add.
	    @param scale The scale to apply to translations and scalings, useful for 
			adapting an animation to a different size target.
        */
        virtual void apply(Real timePos, Real weight = 1.0, bool accumulate = false, 
			Real scale = 1.0f) = 0;

        /** Internal method used to tell the track that keyframe data has been 
            changed, which may cause it to rebuild some internal data. */
		virtual void _keyFrameDataChanged(void) const {}

    protected:
        typedef std::vector<KeyFrame*> KeyFrameList;
        KeyFrameList mKeyFrames;
        Real mMaxKeyFrameTime;
        Animation* mParent;

		/// Create a keyframe implementation - must be overridden
		virtual KeyFrame* createKeyFrameImpl(Real time) = 0;


    };

	/** Specialised AnimationTrack for dealing with generic animable values.
	*/
	class _OgreExport NumericAnimationTrack : public AnimationTrack
	{
	public:
		/// Constructor
		NumericAnimationTrack(Animation* parent);
		/// Constructor, associates with an AnimableValue
		NumericAnimationTrack(Animation* parent, AnimableValuePtr& target);

        /** Creates a new KeyFrame and adds it to this animation at the given time index.
        @remarks
            It is better to create KeyFrames in time order. Creating them out of order can result 
            in expensive reordering processing. Note that a KeyFrame at time index 0.0 is always created
            for you, so you don't need to create this one, just access it using getKeyFrame(0);
        @param timePos The time from which this KeyFrame will apply.
        */
        virtual NumericKeyFrame* createNumericKeyFrame(Real timePos);

		/// @copydoc AnimationTrack::getInterpolatedKeyFrame
		void getInterpolatedKeyFrame(Real timeIndex, KeyFrame* kf) const;

		/// @copydoc AnimationTrack::apply
		void apply(Real timePos, Real weight = 1.0, bool accumulate = false, 
			Real scale = 1.0f);

        /** Applies an animation track to a given animable value.
		@param anim The AnimableValue to which to apply the animation
        @param timePos The time position in the animation to apply.
        @param weight The influence to give to this track, 1.0 for full influence, less to blend with
          other animations.
	    @param scale The scale to apply to translations and scalings, useful for 
			adapting an animation to a different size target.
        */
		void applyToAnimable(const AnimableValuePtr& anim, Real timePos, 
			Real weight = 1.0, Real scale = 1.0f);

		/** Returns a pointer to the associated animable object (if any). */
		virtual const AnimableValuePtr& getAssociatedAnimable(void) const;

		/** Sets the associated animable object which will be automatically 
			affected by calls to 'apply'. */
		virtual void setAssociatedAnimable(const AnimableValuePtr& val);

		/** Returns the KeyFrame at the specified index. */
		NumericKeyFrame* getNumericKeyFrame(unsigned short index) const;


	protected:
		/// Target to animate
		AnimableValuePtr mTargetAnim;

		/// @copydoc AnimationTrack::createKeyFrameImpl
		KeyFrame* createKeyFrameImpl(Real time);


	};

	/** Specialised AnimationTrack for dealing with node transforms.
	*/
	class _OgreExport NodeAnimationTrack : public AnimationTrack
	{
	public:
		/// Constructor
		NodeAnimationTrack(Animation* parent);
		/// Constructor, associates with a Node
		NodeAnimationTrack(Animation* parent, Node* targetNode);
        /** Creates a new KeyFrame and adds it to this animation at the given time index.
        @remarks
            It is better to create KeyFrames in time order. Creating them out of order can result 
            in expensive reordering processing. Note that a KeyFrame at time index 0.0 is always created
            for you, so you don't need to create this one, just access it using getKeyFrame(0);
        @param timePos The time from which this KeyFrame will apply.
        */
        virtual TransformKeyFrame* createNodeKeyFrame(Real timePos);
		/** Returns a pointer to the associated Node object (if any). */
		virtual Node* getAssociatedNode(void) const;

		/** Sets the associated Node object which will be automatically affected by calls to 'apply'. */
		virtual void setAssociatedNode(Node* node);

		/** As the 'apply' method but applies to a specified Node instead of associated node. */
		virtual void applyToNode(Node* node, Real timePos, Real weight = 1.0, 
			bool accumulate = false, Real scale = 1.0f);

		/** Sets the method of rotation calculation */
		virtual void setUseShortestRotationPath(bool useShortestPath);

		/** Gets the method of rotation calculation */
		virtual bool getUseShortestRotationPath() const;

		/// @copydoc AnimationTrack::getInterpolatedKeyFrame
		void getInterpolatedKeyFrame(Real timeIndex, KeyFrame* kf) const;

		/// @copydoc AnimationTrack::apply
		void apply(Real timePos, Real weight = 1.0, bool accumulate = false, 
			Real scale = 1.0f);

		/// @copydoc AnimationTrack::_keyFrameDataChanged
		void _keyFrameDataChanged(void) const;

		/** Returns the KeyFrame at the specified index. */
		virtual TransformKeyFrame* getNodeKeyFrame(unsigned short index) const;


		/** Method to determine if this track has any KeyFrames which are
			doing anything useful - can be used to determine if this track
			can be optimised out.
		*/
		virtual bool hasNonZeroKeyFrames(void) const;

		/** Optimise the current track by removing any duplicate keyframes. */
		virtual void optimise(void);

	protected:
		/// Specialised keyframe creation
		KeyFrame* createKeyFrameImpl(Real time);
		// Flag indicating we need to rebuild the splines next time
		virtual void buildInterpolationSplines(void) const;

		Node* mTargetNode;
		// Prebuilt splines, must be mutable since lazy-update in const method
		mutable bool mSplineBuildNeeded;
		mutable SimpleSpline mPositionSpline;
		mutable SimpleSpline mScaleSpline;
		mutable RotationalSpline mRotationSpline;
		/// Defines if rotation is done using shortest path
		mutable bool mUseShortestRotationPath ;


	};

	/** Specialised AnimationTrack for dealing with changing vertex position information.
	*/
	class _OgreExport VertexAnimationTrack : public AnimationTrack
	{
	public:
		/** The target animation mode */
		enum TargetMode
		{
			/// Interpolate vertex positions in software
			AM_SOFTWARE, 
			/** Bind keyframe 1 to position, and keyframe 2 to a texture coordinate
				for interpolation in hardware */
			AM_HARDWARE
		};
		/// Constructor
		VertexAnimationTrack(Animation* parent);
		/// Constructor, associates with target VertexData and temp buffer (for software)
		VertexAnimationTrack(Animation* parent, VertexData* targetData, 
			TempBlendedBufferInfo* tmpinfo, TargetMode target = AM_SOFTWARE);

		/** Creates a new KeyFrame and adds it to this animation at the given time index.
		@remarks
		It is better to create KeyFrames in time order. Creating them out of order can result 
		in expensive reordering processing. Note that a KeyFrame at time index 0.0 is always created
		for you, so you don't need to create this one, just access it using getKeyFrame(0);
		@param timePos The time from which this KeyFrame will apply.
		*/
		virtual VertexKeyFrame* createVertexKeyFrame(Real timePos);

		/** This method in fact does nothing, since interpolation is not performed
			inside the keyframes for this type of track. 
		*/
		void getInterpolatedKeyFrame(Real timeIndex, KeyFrame* kf) const {}

		/// @copydoc AnimationTrack::apply
		void apply(Real timePos, Real weight = 1.0, bool accumulate = false, 
			Real scale = 1.0f);

		/** As the 'apply' method but applies to specified VertexData instead of 
			associated data. */
		virtual void applyToVertexData(VertexData* data, TempBlendedBufferInfo* tmp, 
			Real timePos, Real weight = 1.0, 
			bool accumulate = false, Real scale = 1.0f);


		/** Returns the KeyFrame at the specified index. */
		VertexKeyFrame* getVertexKeyFrame(unsigned short index) const;

		/** Sets the associated VertexData which this track will update. */
		void setAssociatedVertexData(VertexData* data) { mTargetVertexData = data; }
		/** Gets the associated VertexData which this track will update. */
		VertexData* getAssociatedVertexData(void) const { return mTargetVertexData; }

		/** Sets the associated TempBlendBufferInfo structure which this track 
			will use during the update, if the mode is software (user must configure). */
		void setAssociatedTempInfo(TempBlendedBufferInfo* tmp) { mTempInfo = tmp; }
		/** Gets the associated TempBlendBufferInfo structure which this track 
		will use during the update, if the mode is software (user must configure). */
		TempBlendedBufferInfo* getAssociatedTempInfo(void) const { return mTempInfo; }
		/// Set the target mode
		void setTargetMode(TargetMode m) { mTargetMode = m; }
		/// Get the target mode
		TargetMode getTargetMode(void) const { return mTargetMode; }


	protected:
		/// Target to animate
		VertexData* mTargetVertexData;
		/// Tempbuffer info used in software mode
		TempBlendedBufferInfo* mTempInfo;
		/// Mode to apply
		TargetMode mTargetMode;

		/// @copydoc AnimationTrack::createKeyFrameImpl
		KeyFrame* createKeyFrameImpl(Real time);


	};


}

#endif


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
#ifndef __PredefinedControllers_H__
#define __PredefinedControllers_H__

#include "OgrePrerequisites.h"

#include "OgreController.h"
#include "OgreFrameListener.h"
#include "OgreMaterial.h"

namespace Ogre {

    //-----------------------------------------------------------------------
    // Controller Values
    //-----------------------------------------------------------------------
    /** Predefined controller value for getting the latest frame time.
    */
    class _OgreExport FrameTimeControllerValue : public ControllerValue, public FrameListener
    {
    protected:
        Real mFrameTime;
		Real mTimeFactor;

    public:
        FrameTimeControllerValue();
        bool frameEnded(const FrameEvent &evt);
        bool frameStarted(const FrameEvent &evt);
        Real getValue(void) const;
        void setValue(Real value);
		Real getTimeFactor(void) const;
		void setTimeFactor(Real tf);
    };

    //-----------------------------------------------------------------------
    /** Predefined controller value for getting / setting the frame number of a texture layer
    */
    class _OgreExport TextureFrameControllerValue : public ControllerValue
    {
    protected:
        Material::TextureLayer* mTextureLayer;
    public:
        TextureFrameControllerValue(Material::TextureLayer* t);

        /** Gets the frame number as a parametric value in the range [0,1]
        */
        Real getValue(void) const;
        /** Sets the frame number as a parametric value in the range [0,1]; the actual frame number is value * (numFrames-1).
        */
        void setValue(Real value);

    };
    //-----------------------------------------------------------------------
    /** Predefined controller value for getting / setting a texture coordinate modifications (scales and translates).
        @remarks
            Affects can be applied to the scale or the offset of the u or v coordinates, or both. If separate
            modifications are required to u and v then 2 instances are required to control both independently, or 4
            if you ant separate u and v scales as well as separate u and v offsets.
        @par
            Because of the nature of this value, it can accept values outside the 0..1 parametric range.
    */
    class _OgreExport TexCoordModifierControllerValue : public ControllerValue
    {
    protected:
        bool mTransU, mTransV;
        bool mScaleU, mScaleV;
        bool mRotate;
        Material::TextureLayer* mTextureLayer;
    public:
        /** Constructor.
            @param
                t TextureLayer to apply the modification to.
            @param
                translateU If true, the u coordinates will be translated by the modification.
            @param
                translateV If true, the v coordinates will be translated by the modification.
            @param
                scaleU If true, the u coordinates will be scaled by the modification.
            @param
                scaleV If true, the v coordinates will be scaled by the modification.
            @param
                rotate If true, the texture will be rotated by the modification.
        */
        TexCoordModifierControllerValue(Material::TextureLayer* t, bool translateU = false, bool translateV = false,
            bool scaleU = false, bool scaleV = false, bool rotate = false );

        Real getValue(void) const;
        void setValue(Real value);

    };

    //-----------------------------------------------------------------------
    // Controller functions
    //-----------------------------------------------------------------------

    /** Predefined controller function for dealing with animation.
    */
    class _OgreExport AnimationControllerFunction : public ControllerFunction
    {
    protected:
        Real mSeqTime;
        Real mTime;
    public:
        /** Constructor.
            @param
                sequenceTime The amount of time in seconds it takes to loop through the whole animation sequence.
            @param
                timeOffset The offset in seconds at which to start (default is start at 0)
        */
        AnimationControllerFunction(Real sequenceTime, Real timeOffset = 0.0f);

        /** Overriden function.
        */
        Real calculate(Real source);
    };

    //-----------------------------------------------------------------------
    /** Predefined controller function which simply scales an input to an output value.
    */
    class _OgreExport ScaleControllerFunction : public ControllerFunction
    {
    protected:
        Real mScale;
    public:
        /** Constructor, requires a scale factor.
            @param
                scalefactor The multipier applied to the input to produce the output.
            @param
                deltaInput If true, signifies that the input will be a delta value such that the function should
                 add it to an internal counter before calculating the output.
        */
        ScaleControllerFunction(Real scalefactor, bool deltaInput);

        /** Overriden method.
        */
        Real calculate(Real source);

    };

    //-----------------------------------------------------------------------
    /** Predefined controller function based on a waveform.
        @remarks
            A waveform function translates parametric intput to parametric output based on a wave. The factors
            affecting the function are:
            - wave type - the shape of the wave
            - base - the base value of the output from the wave
            - frequency - the speed of the wave in cycles per second
            - phase - the offset of the start of the wave, e.g. 0.5 to start half-way through the wave
            - amplitude - scales the output so that instead of lying within [0,1] it lies within [0,1] * amplitude
        @par
            Note that for simplicity of integration with the rest of the controller insfrastructure, the output of
            the wave is parametric i.e. 0..1, rather than the typical wave output of [-1,1]. To compensate for this, the
            traditional output of the wave is scaled by the following function before output:
        @par
            output = (waveoutput + 1) * 0.5
        @par
            Hence a wave output of -1 becomes 0, a wave ouput of 1 becomes 1, and a wave output of 0 becomes 0.5.
    */
    class _OgreExport WaveformControllerFunction : public ControllerFunction
    {
    protected:
        WaveformType mWaveType;
        Real mBase;
        Real mFrequency;
        Real mPhase;
        Real mAmplitude;

        /** Overridden from ControllerFunction. */
        Real getAdjustedInput(Real input);

    public:
        /** Default contructor, requires at least a wave type, other parameters can be defaulted unless required.
            @param
                deltaInput If true, signifies that the input will be a delta value such that the function should
                add it to an internal counter before calculating the output.
        */
        WaveformControllerFunction(WaveformType wType, Real base = 0, Real frequency = 1, Real phase = 0, Real amplitude = 1, bool deltaInput = true);

        /** Overriden function.
        */
        Real calculate(Real source);

    };
    //-----------------------------------------------------------------------

}

#endif

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef _LIGHT_H__
#define _LIGHT_H__

#include "OgrePrerequisites.h"

#include "OgreColourValue.h"
#include "OgreVector3.h"
#include "OgreVector4.h"
#include "OgreString.h"
#include "OgreMovableObject.h"
#include "OgrePlaneBoundedVolume.h"

namespace Ogre {


    /** Representation of a dynamic light source in the scene.
        @remarks
            Lights are added to the scene like any other object. They contain various
            parameters like type, position, attenuation (how light intensity fades with
            distance), colour etc.
        @par
            The defaults when a light is created is pure white diffues light, with no
            attenuation (does not decrease with distance) and a range of 1000 world units.
        @par
            Lights are created by using the SceneManager::createLight method. They can subsequently be
            added to a SceneNode if required to allow them to move relative to a node in the scene. A light attached
            to a SceneNode is assumed to havea base position of (0,0,0) and a direction of (0,0,1) before modification
            by the SceneNode's own orientation. If not attached to a SceneNode,
            the light's position and direction is as set using setPosition and setDirection.
        @par
            Remember also that dynamic lights rely on modifying the colour of vertices based on the position of
            the light compared to an object's vertex normals. Dynamic lighting will only look good if the
            object being lit has a fair level of tesselation and the normals are properly set. This is particularly
            true for the spotlight which will only look right on highly tesselated models. In the future OGRE may be
            extended for certain scene types so an alternative to the standard dynamic lighting may be used, such
            as dynamic lightmaps.
    */
    class _OgreExport Light : public MovableObject
    {
    public:
        /// Temp tag used for sorting
        Real tempSquareDist;

        /// Defines the type of light
        enum LightTypes
        {
            /// Point light sources give off light equally in all directions, so require only position not direction
            LT_POINT,
            /// Directional lights simulate parallel light beams from a distant source, hence have direction but no position
            LT_DIRECTIONAL,
            /// Spotlights simulate a cone of light from a source so require position and direction, plus extra values for falloff
            LT_SPOTLIGHT
        };

        /** Default constructor (for Python mainly).
        */
        Light();

        /** Normal constructor. Should not be called directly, but rather the SceneManager::createLight method should be used.
        */
        Light(const String& name);

        /** Standard destructor.
        */
        ~Light();

        /** Returns the name of this light (cannot be modified).
        */
        const String& getName(void) const;

        /** Sets the type of light - see LightTypes for more info.
        */
        void setType(LightTypes type);

        /** Returns the light type.
        */
        LightTypes getType(void) const;

        /** Sets the colour of the diffuse light given off by this source.
            @remarks
                Material objects have ambient, diffuse and specular values which indicate how much of each type of
                light an object reflects. This value denotes the amount and colour of this type of light the light
                exudes into the scene. The actual appearance of objects is a combination of the two.
            @par
                Diffuse light simulates the typical light emenating from light sources and affects the base colour
                of objects together with ambient light.
        */
        void setDiffuseColour(Real red, Real green, Real blue);

        /** Sets the colour of the diffuse light given off by this source.
            @remarks
                Material objects have ambient, diffuse and specular values which indicate how much of each type of
                light an object reflects. This value denotes the amount and colour of this type of light the light
                exudes into the scene. The actual appearance of objects is a combination of the two.
            @par
                Diffuse light simulates the typical light emenating from light sources and affects the base colour
                of objects together with ambient light.
        */
        void setDiffuseColour(const ColourValue& colour);

        /** Returns the colour of the diffuse light given off by this light source (see setDiffuseColour for more info).
        */
        const ColourValue& getDiffuseColour(void) const;

        /** Sets the colour of the specular light given off by this source.
            @remarks
                Material objects have ambient, diffuse and specular values which indicate how much of each type of
                light an object reflects. This value denotes the amount and colour of this type of light the light
                exudes into the scene. The actual appearance of objects is a combination of the two.
            @par
                Specular light affects the appearance of shiny highlights on objects, and is also dependent on the
                'shininess' Material value.
        */
        void setSpecularColour(Real red, Real green, Real blue);

        /** Sets the colour of the specular light given off by this source.
            @remarks
                Material objects have ambient, diffuse and specular values which indicate how much of each type of
                light an object reflects. This value denotes the amount and colour of this type of light the light
                exudes into the scene. The actual appearance of objects is a combination of the two.
            @par
                Specular light affects the appearance of shiny highlights on objects, and is also dependent on the
                'shininess' Material value.
        */
        void setSpecularColour(const ColourValue& colour);

        /** Returns the colour of specular light given off by this light source.
        */
        const ColourValue& getSpecularColour(void) const;

        /** Sets the attenuation parameters of the light source ie how it diminishes with distance.
            @remarks
                Lights normally get fainter the further they are away. Also, each light is given a maximum range
                beyond which it cannot affect any objects.
            @par
                Light attentuation is not applicable to directional lights since they have an infinite range and
                constant intensity.
            @par
                This follows a standard attenuation approach - see any good 3D text for the details of what they mean
                since i don't have room here!
            @param
                range The absolute upper range of the light in world units
            @param
                constant The constant factor in the attenuation formula: 1.0 means never attenuate, 0.0 is complete attenuation
            @param
                linear The linear factor in the attenuation formula: 1 means attenuate evenly over the distance
            @param
                quadratic The quadratic factor in the attenuation formula: adds a curvature to the attenuation formula.
        */
        void setAttenuation(Real range, Real constant, Real linear, Real quadratic);

        /** Returns the absolute upper range of the light.
        */
        Real getAttenuationRange(void) const;

        /** Returns the constant factor in the attenuation formula.
        */
        Real getAttenuationConstant(void) const;

        /** Returns the linear factor in the attenuation formula.
        */
        Real getAttenuationLinear(void) const;

        /** Returns the quadric factor in the attenuation formula.
        */
        Real getAttenuationQuadric(void) const;

        /** Sets the position of the light.
            @remarks
                Applicable to point lights and spotlights only.
            @note
                This will be overridden if the light is attached to a SceneNode.
        */
        void setPosition(Real x, Real y, Real z);

        /** Sets the position of the light.
            @remarks
                Applicable to point lights and spotlights only.
            @note
                This will be overridden if the light is attached to a SceneNode.
        */
        void setPosition(const Vector3& vec);

        /** Returns the position of the light.
            @note
                Applicable to point lights and spotlights only.
        */
        const Vector3& getPosition(void) const;

        /** Sets the direction in which a light points.
            @remarks
                Applicable only to the spotlight and directional light types.
            @note
                This will be overridden if the light is attached to a SceneNode.
        */
        void setDirection(Real x, Real y, Real z);

        /** Sets the direction in which a light points.
            @remarks
                Applicable only to the spotlight and directional light types.
            @note
                This will be overridden if the light is attached to a SceneNode.
        */
        void setDirection(const Vector3& vec);

        /** Returns the light's direction.
            @remarks
                Applicable only to the spotlight and directional light types.
        */
        const Vector3& getDirection(void) const;

        /** Sets the range of a spotlight, i.e. the angle of the inner and outer cones and the rate of falloff between them.
            @param
                innerAngle Angle covered by the bright inner cone, in degrees
            @param
                outerAngle Angle covered by the outer cone, in degrees
            @param
                falloff The rate of falloff between the inner and outer cones. 1.0 means a linear falloff, less means slower falloff, higher means faster falloff.
        */
        void setSpotlightRange(Real innerAngle, Real outerAngle, Real falloff = 1.0);

        /** Returns the angle covered by the spotlights inner cone, in degrees.
        */
        Real getSpotlightInnerAngle(void) const;

        /** Returns the angle covered by the spotlights outer cone, in degrees.
        */
        Real getSpotlightOuterAngle(void) const;

        /** Returns the falloff between the inner and outer cones of the spotlight.
        */
        Real getSpotlightFalloff(void) const;

        /** Overridden from MovableObject */
        void _notifyCurrentCamera(Camera* cam);

        /** Overridden from MovableObject */
        const AxisAlignedBox& getBoundingBox(void) const;

        /** Overridden from MovableObject */
        void _updateRenderQueue(RenderQueue* queue);

        /** Overridden from MovableObject */
        const String& getMovableType(void) const;

        /** Retrieves the position of the light including any transform from nodes it is attached to. */
        const Vector3& getDerivedPosition(void) const;

        /** Retrieves the direction of the light including any transform from nodes it is attached to. */
        const Vector3& getDerivedDirection(void) const;

        /** Overridden from MovableObject.
        @remarks
            Although lights themselves are not 'visible', setting a light to invisible
            means it no longer affects the scene.
        */
        void setVisible(bool visible);

        /** Overridden from MovableObject */
        Real getBoundingRadius(void) const { return 0; /* not visible */ }

		/** Gets the details of this light as a 4D vector.
		@remarks
			Getting details of a light as a 4D vector can be useful for
			doing general calculations between different light types; for
			example the vector can represent both position lights (w=1.0f)
			and directional lights (w=0.0f) and be used in the same 
			calculations.
		*/
		Vector4 getAs4DVector(void) const;

        /** Internal method for calculating the 'near clip volume', which is
            the volume formed between the near clip rectangle of the 
            camera and the light.
        @remarks This volume is a pyramid for a point/spot light and
            a cuboid for a directional light. It can used to detect whether
            an object could be casting a shadow on the viewport. Note that
            the reference returned is to a shared volume which will be 
            reused across calls to this method.
        */
        const PlaneBoundedVolume& _getNearClipVolume(const Camera* cam);


    private:
        /// internal method for synchronising with parent node (if any)
        void update(void) const;
        String mName;

        LightTypes mLightType;
        Vector3 mPosition;
        ColourValue mDiffuse;
        ColourValue mSpecular;

        Vector3 mDirection;

        Real mSpotOuter;
        Real mSpotInner;
        Real mSpotFalloff;
        Real mRange;
        Real mAttenuationConst;
        Real mAttenuationLinear;
        Real mAttenuationQuad;

        mutable Vector3 mDerivedPosition;
        mutable Vector3 mDerivedDirection;
        /// Stored versions of parent orientation / position
        mutable Quaternion mLastParentOrientation;
        mutable Vector3 mLastParentPosition;

        /// Shared class-level name for Movable type
        static String msMovableType;

        PlaneBoundedVolume mNearClipVolume;



    };
} // Namespace
#endif

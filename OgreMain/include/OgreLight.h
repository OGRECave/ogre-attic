/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
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
#ifndef _LIGHT_H__
#define _LIGHT_H__

#include "OgrePrerequisites.h"

#include "OgreColourValue.h"
#include "OgreVector3.h"
#include "OgreString.h"
#include "OgreMovableObject.h"

namespace Ogre {


    /** Representation of a dynamic light source in the scene.
        @remarks
            Lights are added to the scene like any other object. They contain various
            parameters like type, position, attenuation (how light intensity fades with
            distance), colour etc.
        @par
            The defaults when a light is created is pure white diffues light, with linear
            attenuation (decreases steadily with distance) and a range of 1000 world units.
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
        Light(String name);

        /** Standard destructor.
        */
        ~Light();

        /** Returns the name of this light (cannot be modified).
        */
        String getName(void);

        /** Sets the type of light - see LightTypes for more info.
        */
        void setType(LightTypes type);

        /** Returns the light type.
        */
        LightTypes getType(void);

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
        ColourValue getDiffuseColour(void);

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
        ColourValue getSpecularColour(void);

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
        Real getAttenuationRange(void);

        /** Returns the constant factor in the attenuation formula.
        */
        Real getAttenuationConstant(void);

        /** Returns the linear factor in the attenuation formula.
        */
        Real getAttenuationLinear(void);

        /** Returns the quadric factor in the attenuation formula.
        */
        Real getAttenuationQuadric(void);

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
        Vector3 getPosition(void);

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
        Vector3 getDirection(void);

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
        Real getSpotlightInnerAngle(void);

        /** Returns the angle covered by the spotlights outer cone, in degrees.
        */
        Real getSpotlightOuterAngle(void);

        /** Returns the falloff between the inner and outer cones of the spotlight.
        */
        Real getSpotlightFalloff(void);

        /** Returns a true/false value indicating if this light has changed since it was last issued to the renderer.
        */
        bool isModified(void);

        /** Clears the light's modified flag (should only be done by the engine itself).
        */
        void _clearModified(void);

        /** Overridden from MovableObject */
        void _notifyCurrentCamera(Camera* cam);

        /** Overridden from MovableObject */
        const AxisAlignedBox& getBoundingBox(void) const;

        /** Overridden from MovableObject */
        void _updateRenderQueue(RenderQueue* queue);

        /** Overridden from MovableObject */
        String getMovableType(void);

        /** Retrieves the position of the light including any transform from nodes it is attached to. */
        const Vector3& getDerivedPosition(void);

        /** Retrieves the direction of the light including any transform from nodes it is attached to. */
        const Vector3& getDerivedDirection(void);


    private:
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

        bool mModified;

        Vector3 mDerivedPosition;
        Vector3 mDerivedDirection;
        /// Stored versions of parent orientation / position
        Quaternion mLastParentOrientation;
        Vector3 mLastParentPosition;

        /// Shared class-level name for Movable type
        static String msMovableType;



    };
} // Namespace
#endif

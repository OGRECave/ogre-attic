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
#include "OgreStableHeaders.h"
#include "OgreLight.h"

#include "OgreException.h"
#include "OgreSceneNode.h"
#include "OgreCamera.h"


namespace Ogre {
    String Light::msMovableType = "Light";

    //-----------------------------------------------------------------------
    Light::Light()
    {
        // Default to point light, white diffuse light, linear attenuation, fair range
        mLightType = LT_POINT;
        mDiffuse = ColourValue::White;
        mSpecular = ColourValue::Black;
        mRange = 5000;
        mAttenuationConst = 1.0f;
        mAttenuationLinear = 0.0f;
        mAttenuationQuad = 0.0f;

        // Center in world, direction irrelevant but set anyway
        mPosition = Vector3::ZERO;
        mDirection = Vector3::UNIT_Z;
        mParentNode = NULL;

    }
    //-----------------------------------------------------------------------
    Light::Light(const String& name)
    {
        mName = name;

        // Default to point light, white diffuse light, linear attenuation, fair range
        mLightType = LT_POINT;
        mDiffuse = ColourValue::White;
        mSpecular = ColourValue::Black;
        mRange = 100000;
        mAttenuationConst = 1.0f;
        mAttenuationLinear = 0.0f;
        mAttenuationQuad = 0.0f;

        // Center in world, direction irrelevant but set anyway
        mPosition = Vector3::ZERO;
        mDirection = Vector3::UNIT_Z;

        // Default some spot values
        mSpotInner = 30.0f;
        mSpotOuter = 40.0f;
        mSpotFalloff = 1.0f;
        mParentNode = NULL;


    }
    //-----------------------------------------------------------------------
    Light::~Light()
    {
    }
    //-----------------------------------------------------------------------
    const String& Light::getName(void) const
    {
        return mName;

    }
    //-----------------------------------------------------------------------
    void Light::setType(LightTypes type)
    {
        mLightType = type;
    }
    //-----------------------------------------------------------------------
    Light::LightTypes Light::getType(void) const
    {
        return mLightType;
    }
    //-----------------------------------------------------------------------
    void Light::setPosition(Real x, Real y, Real z)
    {
        mPosition.x = x;
        mPosition.y = y;
        mPosition.z = z;

    }
    //-----------------------------------------------------------------------
    void Light::setPosition(const Vector3& vec)
    {
        mPosition = vec;
    }
    //-----------------------------------------------------------------------
    const Vector3& Light::getPosition(void) const
    {
        return mPosition;
    }
    //-----------------------------------------------------------------------
    void Light::setDirection(Real x, Real y, Real z)
    {
        mDirection.x = x;
        mDirection.y = y;
        mDirection.z = z;
    }
    //-----------------------------------------------------------------------
    void Light::setDirection(const Vector3& vec)
    {
        mDirection = vec;
    }
    //-----------------------------------------------------------------------
    const Vector3& Light::getDirection(void) const
    {
        return mDirection;
    }
    //-----------------------------------------------------------------------
    void Light::setSpotlightRange(Real innerAngle, Real outerAngle, Real falloff)
    {

        if (mLightType != LT_SPOTLIGHT)
            Except(9999,
                "setSpotlightRange is only valid for spotlights.",
                "Light::setSpotlightRange");

        mSpotInner =innerAngle;
        mSpotOuter = outerAngle;
        mSpotFalloff = falloff;
    }
    //-----------------------------------------------------------------------
    Real Light::getSpotlightInnerAngle(void) const
    {
        return mSpotInner;
    }
    //-----------------------------------------------------------------------
    Real Light::getSpotlightOuterAngle(void) const
    {
        return mSpotOuter;
    }
    //-----------------------------------------------------------------------
    Real Light::getSpotlightFalloff(void) const
    {
        return mSpotFalloff;
    }
    //-----------------------------------------------------------------------
    void Light::setDiffuseColour(Real red, Real green, Real blue)
    {
        mDiffuse.r = red;
        mDiffuse.b = blue;
        mDiffuse.g = green;
    }
    //-----------------------------------------------------------------------
    void Light::setDiffuseColour(const ColourValue& colour)
    {
        mDiffuse = colour;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Light::getDiffuseColour(void) const
    {
        return mDiffuse;
    }
    //-----------------------------------------------------------------------
    void Light::setSpecularColour(Real red, Real green, Real blue)
    {
        mSpecular.r = red;
        mSpecular.b = blue;
        mSpecular.g = green;
    }
    //-----------------------------------------------------------------------
    void Light::setSpecularColour(const ColourValue& colour)
    {
        mSpecular = colour;
    }
    //-----------------------------------------------------------------------
    const ColourValue& Light::getSpecularColour(void) const
    {
        return mSpecular;
    }
    //-----------------------------------------------------------------------
    void Light::setAttenuation(Real range, Real constant,
                        Real linear, Real quadratic)
    {
        mRange = range;
        mAttenuationConst = constant;
        mAttenuationLinear = linear;
        mAttenuationQuad = quadratic;
    }
    //-----------------------------------------------------------------------
    Real Light::getAttenuationRange(void) const
    {
        return mRange;
    }
    //-----------------------------------------------------------------------
    Real Light::getAttenuationConstant(void) const
    {
        return mAttenuationConst;
    }
    //-----------------------------------------------------------------------
    Real Light::getAttenuationLinear(void) const
    {
        return mAttenuationLinear;
    }
    //-----------------------------------------------------------------------
    Real Light::getAttenuationQuadric(void) const
    {
        return mAttenuationQuad;
    }
    //-----------------------------------------------------------------------
    void Light::update(void) const
    {
        if (mParentNode)
        {
            if (!(mParentNode->_getDerivedOrientation() == mLastParentOrientation &&
                mParentNode->_getDerivedPosition() == mLastParentPosition))
            {
                // Ok, we're out of date with SceneNode we're attached to
                mLastParentOrientation = mParentNode->_getDerivedOrientation();
                mLastParentPosition = mParentNode->_getDerivedPosition();
                mDerivedDirection = mLastParentOrientation * mDirection;
                mDerivedPosition = (mLastParentOrientation * mPosition) + mLastParentPosition;
            }
        }
        else
        {
            mDerivedPosition = mPosition;
            mDerivedDirection = mDirection;
        }

    }
    //-----------------------------------------------------------------------
    void Light::_notifyCurrentCamera(Camera* cam)
    {
        // Do nothing
    }
    //-----------------------------------------------------------------------
    const AxisAlignedBox& Light::getBoundingBox(void) const
    {
        // Null, lights are not visible
        static AxisAlignedBox box;
        return box;
    }
    //-----------------------------------------------------------------------
    void Light::_updateRenderQueue(RenderQueue* queue)
    {
        // Do nothing
    }
    //-----------------------------------------------------------------------
    const String& Light::getMovableType(void) const
    {
        return msMovableType;
    }
    //-----------------------------------------------------------------------
    const Vector3& Light::getDerivedPosition(void) const
    {
        update();
        return mDerivedPosition;
    }
    //-----------------------------------------------------------------------
    const Vector3& Light::getDerivedDirection(void) const
    {
        update();
        return mDerivedDirection;
    }
    //-----------------------------------------------------------------------
    void Light::setVisible(bool visible)
    {
        MovableObject::setVisible(visible);
    }
    //-----------------------------------------------------------------------
	Vector4 Light::getAs4DVector(void) const
	{
		Vector4 ret;
        if (mLightType == Light::LT_DIRECTIONAL)
        {
            ret = -(getDerivedDirection()); // negate direction as 'position'
            ret.w = 0.0; // infinite distance
        }	
		else
        {
            ret = getDerivedPosition();
            ret.w = 1.0;
        }
		return ret;
	}
    //-----------------------------------------------------------------------
    const PlaneBoundedVolume& Light::_getNearClipVolume(const Camera* cam) const
    {
        // First check if the light is close to the near plane, since
        // in this case we have to build a degenerate clip volume
        mNearClipVolume.planes.clear();
        mNearClipVolume.outside = Plane::NEGATIVE_SIDE;

        Real n = cam->getNearClipDistance();
        // Homogenous position
        Vector4 lightPos = getAs4DVector();
        // 3D version (not the same as _getDerivedPosition, is -direction for
        // directional lights)
        Vector3 lightPos3 = Vector3(lightPos.x, lightPos.y, lightPos.z);

        // Get eye-space light position
        // use 4D vector so directional lights still work
        Vector4 eyeSpaceLight = cam->getViewMatrix() * lightPos;
        Matrix4 eyeToWorld = cam->getViewMatrix().inverse();
        // Find distance to light, project onto -Z axis
        Real d = eyeSpaceLight.dotProduct(
            Vector4(0, 0, -1, -n) );
        #define THRESHOLD 1e-6
        if (d > THRESHOLD || d < -THRESHOLD)
        {
            // light is not too close to the near plane
            // First find the worldspace positions of the corners of the viewport
            const Vector3 *corner = cam->getWorldSpaceCorners();
            // Iterate over world points and form side planes
            Vector3 normal;
            Vector3 lightDir;
            for (unsigned int i = 0; i < 4; ++i)
            {
                // Figure out light dir
                lightDir = lightPos3 - (corner[i] * lightPos.w);
                // Cross with anticlockwise corner, therefore normal points in
                normal = (corner[i] - corner[(i-1)%4])
                    .crossProduct(lightDir);
                normal.normalise();
                if (d < THRESHOLD)
                {
                    // invert normal
                    normal = -normal;
                }
                // NB last param to Plane constructor is negated because it's -d
                mNearClipVolume.planes.push_back(
                    Plane(normal, normal.dotProduct(corner[i])));

            }

            // Now do the near plane plane
            if (d > THRESHOLD)
            {
                // In front of near plane
                // remember the -d negation in plane constructor
                normal = eyeToWorld * -Vector3::UNIT_Z;
                normal.normalise();
                mNearClipVolume.planes.push_back(
                    Plane(normal, -normal.dotProduct(cam->getDerivedPosition())));
            }
            else
            {
                // Behind near plane
                // remember the -d negation in plane constructor
                normal = eyeToWorld * Vector3::UNIT_Z;
                normal.normalise();
                mNearClipVolume.planes.push_back(
                    Plane(normal, -normal.dotProduct(cam->getDerivedPosition())));
            }

            // Finally, for a point/spot light we can add a sixth plane
            // This prevents false positives from behind the light
            if (mLightType != LT_DIRECTIONAL)
            {
                // Direction from light to centre point of viewport 
                normal = (eyeToWorld * Vector3(0,0,-n)) - lightPos3;
                normal.normalise();
                // remember the -d negation in plane constructor
                mNearClipVolume.planes.push_back(
                    Plane(normal, normal.dotProduct(lightPos3)));

            }


        }
        else
        {
            // light is close to being on the near plane
            // degenerate volume including the entire scene 
            // we will always require light / dark caps
            mNearClipVolume.planes.push_back(Plane(Vector3::UNIT_Z, -n));
            mNearClipVolume.planes.push_back(Plane(-Vector3::UNIT_Z, n));
        }

        return mNearClipVolume;

    }
    //-----------------------------------------------------------------------
    const PlaneBoundedVolumeList& Light::_getFrustumClipVolumes(const Camera* cam) const
    {

        // Homogenous light position
        Vector4 lightPos = getAs4DVector();
        // 3D version (not the same as _getDerivedPosition, is -direction for
        // directional lights)
        Vector3 lightPos3 = Vector3(lightPos.x, lightPos.y, lightPos.z);

        const Vector3 *clockwiseVerts[4];

        Matrix4 eyeToWorld = cam->getViewMatrix().inverse();
        // Get worldspace frustum corners
        const Vector3* corners = cam->getWorldSpaceCorners();

        bool infiniteViewDistance = (cam->getFarClipDistance() == 0);

        mFrustumClipVolumes.clear();
        for (unsigned short n = 0; n < 6; ++n)
        {
            // Skip far plane if infinite view frustum
            if (infiniteViewDistance && n == FRUSTUM_PLANE_FAR)
                continue;

            const Plane& plane = cam->getFrustumPlane(n);
            Vector4 planeVec(plane.normal.x, plane.normal.y, plane.normal.z, plane.d);
            // planes face inwards, we need to know if light is on negative side
            Real d = planeVec.dotProduct(lightPos);
            if (d < -1e-06)
            {
                // Ok, this is a valid one
                // clockwise verts mean we can cross-product and always get normals
                // facing into the volume we create

                mFrustumClipVolumes.push_back(PlaneBoundedVolume());
                PlaneBoundedVolume& vol = mFrustumClipVolumes[mFrustumClipVolumes.size() - 1];
                switch(n)
                {
                case(FRUSTUM_PLANE_NEAR):
                    clockwiseVerts[0] = corners + 3;
                    clockwiseVerts[1] = corners + 2;
                    clockwiseVerts[2] = corners + 1;
                    clockwiseVerts[3] = corners + 0;
                    break;
                case(FRUSTUM_PLANE_FAR):
                    clockwiseVerts[0] = corners + 7;
                    clockwiseVerts[1] = corners + 6;
                    clockwiseVerts[2] = corners + 5;
                    clockwiseVerts[3] = corners + 4;
                    break;
                case(FRUSTUM_PLANE_LEFT):
                    clockwiseVerts[0] = corners + 2;
                    clockwiseVerts[1] = corners + 6;
                    clockwiseVerts[2] = corners + 5;
                    clockwiseVerts[3] = corners + 1;
                    break;
                case(FRUSTUM_PLANE_RIGHT):
                    clockwiseVerts[0] = corners + 7;
                    clockwiseVerts[1] = corners + 3;
                    clockwiseVerts[2] = corners + 0;
                    clockwiseVerts[3] = corners + 4;
                    break;
                case(FRUSTUM_PLANE_TOP):
                    clockwiseVerts[0] = corners + 0;
                    clockwiseVerts[1] = corners + 1;
                    clockwiseVerts[2] = corners + 5;
                    clockwiseVerts[3] = corners + 4;
                    break;
                case(FRUSTUM_PLANE_BOTTOM):
                    clockwiseVerts[0] = corners + 7;
                    clockwiseVerts[1] = corners + 6;
                    clockwiseVerts[2] = corners + 2;
                    clockwiseVerts[3] = corners + 3;
                    break;
                };

                // Build a volume
                // Iterate over world points and form side planes
                Vector3 normal;
                Vector3 lightDir;
                for (unsigned int i = 0; i < 4; ++i)
                {
                    // Figure out light dir
                    lightDir = lightPos3 - (*(clockwiseVerts[i]) * lightPos.w);
                    Vector3 edgeDir = *(clockwiseVerts[i]) - *(clockwiseVerts[(i-1)%4]);
                    // Cross with anticlockwise corner, therefore normal points in
                    normal = edgeDir.crossProduct(lightDir);
                    normal.normalise();
                    // NB last param to Plane constructor is negated because it's -d
                    vol.planes.push_back(
                        Plane(normal, normal.dotProduct(*(clockwiseVerts[i]))));

                }

                // Now do the near plane (this is the plane of the side we're 
                // talking about, with the normal inverted (d is already interpreted as -ve)
                vol.planes.push_back( Plane(-plane.normal, plane.d) );

                // Finally, for a point/spot light we can add a sixth plane
                // This prevents false positives from behind the light
                if (mLightType != LT_DIRECTIONAL)
                {
                    // re-use our own plane normal
                    // remember the -d negation in plane constructor
                    vol.planes.push_back(
                        Plane(plane.normal, 
                            plane.normal.dotProduct(lightPos3)));

                }


            }
        }
        return mFrustumClipVolumes;
    }




} // Namespace

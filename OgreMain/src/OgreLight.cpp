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

namespace Ogre {
    String Light::msMovableType = "Light";

    //-----------------------------------------------------------------------
    Light::Light()
    {
        // Default to point light, white diffuse light, linear attenuation, fair range
        mLightType = LT_POINT;
        mDiffuse = ColourValue::White;
        mSpecular = ColourValue::Black;
        mRange = 1000;
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






} // Namespace

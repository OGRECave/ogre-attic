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

        // Deafult modified
        mModified = true;
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


        // Deafult modified
        mModified = true;
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
        mModified = true;
    }
    //-----------------------------------------------------------------------
    Light::LightTypes Light::getType(void)
    {
        return mLightType;
    }
    //-----------------------------------------------------------------------
    void Light::setPosition(Real x, Real y, Real z)
    {
        mPosition.x = x;
        mPosition.y = y;
        mPosition.z = z;
        mModified = true;

    }
    //-----------------------------------------------------------------------
    void Light::setPosition(const Vector3& vec)
    {
        mPosition = vec;
        mModified = true;
    }
    //-----------------------------------------------------------------------
    Vector3 Light::getPosition(void)
    {
        return mPosition;
    }
    //-----------------------------------------------------------------------
    void Light::setDirection(Real x, Real y, Real z)
    {
        mDirection.x = x;
        mDirection.y = y;
        mDirection.z = z;
        mModified = true;
    }
    //-----------------------------------------------------------------------
    void Light::setDirection(const Vector3& vec)
    {
        mDirection = vec;
        mModified = true;
    }
    //-----------------------------------------------------------------------
    Vector3 Light::getDirection(void)
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
        mModified = true;
    }
    //-----------------------------------------------------------------------
    Real Light::getSpotlightInnerAngle(void)
    {
        return mSpotInner;
    }
    //-----------------------------------------------------------------------
    Real Light::getSpotlightOuterAngle(void)
    {
        return mSpotOuter;
    }
    //-----------------------------------------------------------------------
    Real Light::getSpotlightFalloff(void)
    {
        return mSpotFalloff;
    }
    //-----------------------------------------------------------------------
    void Light::setDiffuseColour(Real red, Real green, Real blue)
    {
        mDiffuse.r = red;
        mDiffuse.b = blue;
        mDiffuse.g = green;
        mModified = true;
    }
    //-----------------------------------------------------------------------
    void Light::setDiffuseColour(const ColourValue& colour)
    {
        mDiffuse = colour;
        mModified = true;
    }
    //-----------------------------------------------------------------------
    ColourValue Light::getDiffuseColour(void)
    {
        return mDiffuse;
    }
    //-----------------------------------------------------------------------
    void Light::setSpecularColour(Real red, Real green, Real blue)
    {
        mSpecular.r = red;
        mSpecular.b = blue;
        mSpecular.g = green;
        mModified = true;
    }
    //-----------------------------------------------------------------------
    void Light::setSpecularColour(const ColourValue& colour)
    {
        mSpecular = colour;
        mModified = true;
    }
    //-----------------------------------------------------------------------
    ColourValue Light::getSpecularColour(void)
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
        mModified = true;
    }
    //-----------------------------------------------------------------------
    Real Light::getAttenuationRange(void)
    {
        return mRange;
    }
    //-----------------------------------------------------------------------
    Real Light::getAttenuationConstant(void)
    {
        return mAttenuationConst;
    }
    //-----------------------------------------------------------------------
    Real Light::getAttenuationLinear(void)
    {
        return mAttenuationLinear;
    }
    //-----------------------------------------------------------------------
    Real Light::getAttenuationQuadric(void)
    {
        return mAttenuationQuad;
    }
    //-----------------------------------------------------------------------
    bool Light::isModified(void)
    {
        if (mParentNode)
        {
            if (!mModified && mParentNode->_getDerivedOrientation() == mLastParentOrientation &&
                mParentNode->_getDerivedPosition() == mLastParentPosition)
            {
                return false;
            }
            else
            {
                // Ok, we're out of date with SceneNode we're attached to
                mLastParentOrientation = mParentNode->_getDerivedOrientation();
                mLastParentPosition = mParentNode->_getDerivedPosition();
                mDerivedDirection = mLastParentOrientation * mDirection;
                mDerivedPosition = (mLastParentOrientation * mPosition) + mLastParentPosition;
                return true;
            }
        }
        else
        {
            mDerivedPosition = mPosition;
            mDerivedDirection = mDirection;
            return mModified;
        }

    }
    //-----------------------------------------------------------------------
    void Light::_clearModified(void)
    {

        mModified = false;
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
    const Vector3& Light::getDerivedPosition(void)
    {
        isModified();
        return mDerivedPosition;
    }
    //-----------------------------------------------------------------------
    const Vector3& Light::getDerivedDirection(void)
    {
        isModified();
        return mDerivedDirection;
    }
    //-----------------------------------------------------------------------
    void Light::setVisible(bool visible)
    {
        MovableObject::setVisible(visible);
        mModified = true;
    }






} // Namespace

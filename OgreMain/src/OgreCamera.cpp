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
#include "OgreCamera.h"

#include "OgreMath.h"
#include "OgreMatrix3.h"
#include "OgreSceneManager.h"
#include "OgreSceneNode.h"
#include "OgreAxisAlignedBox.h"
#include "OgreSphere.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

namespace Ogre {

    String Camera::msMovableType = "Camera";
    //-----------------------------------------------------------------------
    Camera::Camera( const String& name, SceneManager* sm)
        : mName( name )
    {
        // Init camera location & direction

        // Locate at (0,0,0)
        mPosition.x = mPosition.y = mPosition.z = 0;
        // Point down -Z axis
        mOrientation = Quaternion::IDENTITY;


        // Reasonable defaults to camera params
        mFOVy = Math::RadiansToAngleUnits(Math::PI/4.0);
        mNearDist = 100.0f;
        mFarDist = 100000.0f;
        mAspect = 1.33333333333333f;
        mProjType = PT_PERSPECTIVE;
        mSceneDetail = SDL_SOLID;
        setFixedYawAxis(true);    // Default to fixed yaw, like freelook since most people expect this

        invalidateFrustum();
        invalidateView();

        // Init matrices
        mViewMatrix = Matrix4::ZERO;
        mProjMatrix = Matrix4::ZERO;

        mParentNode = 0;

        // Record SceneManager
        mSceneMgr = sm;

        // Init no tracking
        mAutoTrackTarget = 0;

        // Init lod
        mSceneLodFactor = mSceneLodFactorInv =  1.0f;

        // no reflection
        mReflect = false;

        mVisible = false;


        mWindowSet = false;
    }

    //-----------------------------------------------------------------------
    Camera::~Camera()
    {
        // Do nothing
    }

    //-----------------------------------------------------------------------
    SceneManager* Camera::getSceneManager(void) const
    {
        return mSceneMgr;
    }
    //-----------------------------------------------------------------------
    const String& Camera::getName(void) const
    {
        return mName;
    }


    //-----------------------------------------------------------------------
    void Camera::setDetailLevel(SceneDetailLevel sd)
    {
        mSceneDetail = sd;
    }

    //-----------------------------------------------------------------------
    SceneDetailLevel Camera::getDetailLevel(void) const
    {
        return mSceneDetail;
    }

    //-----------------------------------------------------------------------
    void Camera::setPosition(Real x, Real y, Real z)
    {
        mPosition.x = x;
        mPosition.y = y;
        mPosition.z = z;
        invalidateView();
    }

    //-----------------------------------------------------------------------
    void Camera::setPosition(const Vector3& vec)
    {
        mPosition = vec;
        invalidateView();
    }

    //-----------------------------------------------------------------------
    const Vector3& Camera::getPosition(void) const
    {
        return mPosition;
    }

    //-----------------------------------------------------------------------
    void Camera::move(const Vector3& vec)
    {
        mPosition = mPosition + vec;
        invalidateView();
    }

    //-----------------------------------------------------------------------
    void Camera::moveRelative(const Vector3& vec)
    {
        // Transform the axes of the relative vector by camera's local axes
        Vector3 trans = mOrientation * vec;

        mPosition = mPosition + trans;
        invalidateView();
    }

    //-----------------------------------------------------------------------
    void Camera::setDirection(Real x, Real y, Real z)
    {
        setDirection(Vector3(x,y,z));
    }

    //-----------------------------------------------------------------------
    void Camera::setDirection(const Vector3& vec)
    {
        // Do nothing if given a zero vector
        // (Replaced assert since this could happen with auto tracking camera and
        //  camera passes through the lookAt point)
        if (vec == Vector3::ZERO) return;

        // Remember, camera points down -Z of local axes!
        // Therefore reverse direction of direction vector before determining local Z
        Vector3 zAdjustVec = -vec;
        zAdjustVec.normalise();


        if( mYawFixed )
        {
            Vector3 xVec = mYawFixedAxis.crossProduct( zAdjustVec );
            xVec.normalise();

            Vector3 yVec = zAdjustVec.crossProduct( xVec );
            yVec.normalise();

            mOrientation.FromAxes( xVec, yVec, zAdjustVec );
        }
        else
        {

            // Get axes from current quaternion
            Vector3 axes[3];
            updateView();
            mDerivedOrientation.ToAxes(axes);
            Quaternion rotQuat;
            if (-zAdjustVec == axes[2])
            {
                // Oops, a 180 degree turn (infinite possible rotation axes)
                // Default to yaw i.e. use current UP
                rotQuat.FromAngleAxis(Math::PI, axes[1]);
            }
            else
            {
                // Derive shortest arc to new direction
                rotQuat = axes[2].getRotationTo(zAdjustVec);

            }
            mOrientation = rotQuat * mOrientation;
        }


        // TODO If we have a fixed yaw axis, we mustn't break it by using the
        // shortest arc because this will sometimes cause a relative yaw
        // which will tip the camera

        invalidateView();

    }

    //-----------------------------------------------------------------------
    Vector3 Camera::getDirection(void) const
    {
        // Direction points down -Z by default
        return mOrientation * -Vector3::UNIT_Z;
    }

    //-----------------------------------------------------------------------
    Vector3 Camera::getUp(void) const
    {
        return mOrientation * Vector3::UNIT_Y;
    }

    //-----------------------------------------------------------------------
    Vector3 Camera::getRight(void) const
    {
        return mOrientation * Vector3::UNIT_X;
    }

    //-----------------------------------------------------------------------
    void Camera::lookAt(const Vector3& targetPoint)
    {

        updateView();
        this->setDirection(targetPoint - mDerivedPosition);

    }

    //-----------------------------------------------------------------------
    void Camera::lookAt( Real x, Real y, Real z )
    {
        Vector3 vTemp( x, y, z );
        this->lookAt(vTemp);

    }


    //-----------------------------------------------------------------------
    void Camera::roll(Real degrees)
    {
        // Rotate around local Z axis
        Vector3 zAxis = mOrientation * Vector3::UNIT_Z;
        rotate(zAxis, degrees);

        invalidateView();
    }

    //-----------------------------------------------------------------------
    void Camera::yaw(Real degrees)
    {
        Vector3 yAxis;

        if (mYawFixed)
        {
            // Rotate around fixed yaw axis
            yAxis = mYawFixedAxis;
        }
        else
        {
            // Rotate around local Y axis
            yAxis = mOrientation * Vector3::UNIT_Y;
        }

        rotate(yAxis, degrees);

        invalidateView();
    }

    //-----------------------------------------------------------------------
    void Camera::pitch(Real degrees)
    {
        // Rotate around local X axis
        Vector3 xAxis = mOrientation * Vector3::UNIT_X;
        rotate(xAxis, degrees);

        invalidateView();

    }

    //-----------------------------------------------------------------------
    void Camera::rotate(const Vector3& axis, Real degrees)
    {
        Quaternion q;
        q.FromAngleAxis(Math::AngleUnitsToRadians(degrees),axis);
        rotate(q);

    }
    //-----------------------------------------------------------------------
    void Camera::rotate(const Quaternion& q)
    {
        // Note the order of the mult, i.e. q comes after
        mOrientation = q * mOrientation;
        invalidateView();

    }

    //-----------------------------------------------------------------------
    void Camera::updateFrustum(void) const
    {
        Frustum::updateFrustum();
        // Set the clipping planes
        setWindowImpl();
    }

    //-----------------------------------------------------------------------
    bool Camera::isViewOutOfDate(void) const
    {
        // Overridden from Frustum to use local orientation / position offsets
        // Attached to node?
        if (mParentNode != 0)
        {
            if (!mRecalcView && mParentNode->_getDerivedOrientation() == mLastParentOrientation &&
                mParentNode->_getDerivedPosition() == mLastParentPosition)
            {
                return false;
            }
            else
            {
                // Ok, we're out of date with SceneNode we're attached to
                mLastParentOrientation = mParentNode->_getDerivedOrientation();
                mLastParentPosition = mParentNode->_getDerivedPosition();
                mDerivedOrientation = mLastParentOrientation * mOrientation;
                mDerivedPosition = (mLastParentOrientation * mPosition) + mLastParentPosition;
                return true;
            }
        }
        else
        {
            // Rely on own updates
            mDerivedOrientation = mOrientation;
            mDerivedPosition = mPosition;
            return mRecalcView;
        }
    }


    //-----------------------------------------------------------------------
    void Camera::updateView(void) const
    {
        Frustum::updateView();
        setWindowImpl();

    }
    // -------------------------------------------------------------------
    void Camera::invalidateView()
    {
        mRecalcView = true;
        mRecalcWindow = true;
    }
    // -------------------------------------------------------------------
    void Camera::invalidateFrustum(void)
    {
        mRecalcFrustum = true;
        mRecalcWindow = true;
    }
    //-----------------------------------------------------------------------
    void Camera::_renderScene(Viewport *vp, bool includeOverlays)
    {

        mSceneMgr->_renderScene(this, vp, includeOverlays);
    }


    //-----------------------------------------------------------------------
    std::ostream& operator<<( std::ostream& o, Camera& c )
    {
        o << "Camera(Name='" << c.mName << "', pos=" << c.mPosition;
        Vector3 dir(c.mOrientation*Vector3(0,0,-1));
        o << ", direction=" << dir << ",near=" << c.mNearDist;
        o << ", far=" << c.mFarDist << ", FOVy=" << c.mFOVy;
        o << ", aspect=" << c.mAspect << ", ";
        o << "NearFrustumPlane=" << c.mFrustumPlanes[FRUSTUM_PLANE_NEAR] << ", ";
        o << "FarFrustumPlane=" << c.mFrustumPlanes[FRUSTUM_PLANE_FAR] << ", ";
        o << "LeftFrustumPlane=" << c.mFrustumPlanes[FRUSTUM_PLANE_LEFT] << ", ";
        o << "RightFrustumPlane=" << c.mFrustumPlanes[FRUSTUM_PLANE_RIGHT] << ", ";
        o << "TopFrustumPlane=" << c.mFrustumPlanes[FRUSTUM_PLANE_TOP] << ", ";
        o << "BottomFrustumPlane=" << c.mFrustumPlanes[FRUSTUM_PLANE_BOTTOM];
        o << ")";

        return o;
    }

    //-----------------------------------------------------------------------
    void Camera::setFixedYawAxis(bool useFixed, const Vector3& fixedAxis)
    {
        mYawFixed = useFixed;
        mYawFixedAxis = fixedAxis;
    }

    //-----------------------------------------------------------------------
    void Camera::_notifyRenderedFaces(unsigned int numfaces)
    {
        mVisFacesLastRender = numfaces;
    }

    //-----------------------------------------------------------------------
    unsigned int Camera::_getNumRenderedFaces(void) const
    {
        return mVisFacesLastRender;
    }

    //-----------------------------------------------------------------------
    const Quaternion& Camera::getOrientation(void) const
    {
        return mOrientation;
    }

    //-----------------------------------------------------------------------
    void Camera::setOrientation(const Quaternion& q)
    {
        mOrientation = q;
        invalidateView();
    }
    //-----------------------------------------------------------------------
    Quaternion Camera::getDerivedOrientation(void) const
    {
        updateView();
        return mDerivedOrientation;
    }
    //-----------------------------------------------------------------------
    Vector3 Camera::getDerivedPosition(void) const
    {
        updateView();
        return mDerivedPosition;
    }
    //-----------------------------------------------------------------------
    Vector3 Camera::getDerivedDirection(void) const
    {
        // Direction points down -Z by default
        updateView();
        return mDerivedOrientation * -Vector3::UNIT_Z;
    }
    //-----------------------------------------------------------------------
    const String& Camera::getMovableType(void) const
    {
        return msMovableType;
    }
    //-----------------------------------------------------------------------
    void Camera::setAutoTracking(bool enabled, SceneNode* target, 
        const Vector3& offset)
    {
        if (enabled)
        {
            assert (target != 0 && "target cannot be a null pointer if tracking is enabled");
            mAutoTrackTarget = target;
            mAutoTrackOffset = offset;
        }
        else
        {
            mAutoTrackTarget = 0;
        }
    }
    //-----------------------------------------------------------------------
    void Camera::_autoTrack(void)
    {
        // NB assumes that all scene nodes have been updated
        if (mAutoTrackTarget)
        {
            lookAt(mAutoTrackTarget->_getDerivedPosition() + mAutoTrackOffset);
        }
    }
    //-----------------------------------------------------------------------
	void Camera::setLodBias(Real factor)
	{
		assert(factor > 0.0f && "Bias factor must be > 0!");
		mSceneLodFactor = factor;
		mSceneLodFactorInv = 1.0f / factor;
	}
    //-----------------------------------------------------------------------
	Real Camera::getLodBias(void) const
	{
		return mSceneLodFactor;
	}
    //-----------------------------------------------------------------------
	Real Camera::_getLodBiasInverse(void) const
	{
		return mSceneLodFactorInv;
	}
    //-----------------------------------------------------------------------
    Ray Camera::getCameraToViewportRay(Real screenX, Real screenY)
    {
        Real centeredScreenX = (screenX - 0.5f);
        Real centeredScreenY = (0.5f - screenY);

        Real normalizedSlope = Math::Tan(Math::DegreesToRadians(mFOVy / 2));
        Real viewportYToWorldY = normalizedSlope * mNearDist * 2;
        Real viewportXToWorldX = viewportYToWorldY * mAspect;

        Vector3 rayDirection(centeredScreenX * viewportXToWorldX,
            centeredScreenY * viewportYToWorldY,
            -mNearDist);
        rayDirection = getDerivedOrientation() * rayDirection;
        rayDirection.normalise();

        return Ray(getDerivedPosition(), rayDirection);
    } 

    // -------------------------------------------------------------------
    void Camera::setWindow (Real Left, Real Top, Real Right, Real Bottom)
    {
        mWLeft = Left;
        mWTop = Top;
        mWRight = Right;
        mWBottom = Bottom;

        mWindowSet = true;
        mRecalcWindow = true;

        invalidateView();
    }
    // -------------------------------------------------------------------
    void Camera::resetWindow ()
    {
        mWindowSet = false;
    }
    // -------------------------------------------------------------------
    void Camera::setWindowImpl() const
    {
        if (!mWindowSet || !mRecalcWindow)
            return;


        Real thetaY = Math::AngleUnitsToRadians(mFOVy / 2.0f);
        Real tanThetaY = Math::Tan(thetaY);
        //Real thetaX = thetaY * mAspect;
        Real tanThetaX = tanThetaY * mAspect;

        Real vpTop = tanThetaY * mNearDist;
        Real vpLeft = -tanThetaX * mNearDist;
        Real vpWidth = -2 * vpLeft;
        Real vpHeight = 2 * vpTop;

        Real wvpLeft   = vpLeft + mWLeft * vpWidth;
        Real wvpRight  = vpLeft + mWRight * vpWidth;
        Real wvpTop    = vpTop - mWTop * vpHeight;
        Real wvpBottom = vpTop - mWBottom * vpHeight;

        Vector3 vp_ul (wvpLeft, wvpTop, -mNearDist);
        Vector3 vp_ur (wvpRight, wvpTop, -mNearDist);
        Vector3 vp_bl (wvpLeft, wvpBottom, -mNearDist);
        Vector3 vp_br (wvpRight, wvpBottom, -mNearDist);

        Matrix4 inv = mViewMatrix.inverse();

        Vector3 vw_ul = inv * vp_ul;
        Vector3 vw_ur = inv * vp_ur;
        Vector3 vw_bl = inv * vp_bl;
        Vector3 vw_br = inv * vp_br;

        Vector3 position = getPosition();

        mWindowClipPlanes.clear();
        mWindowClipPlanes.push_back(Plane(position, vw_bl, vw_ul));
        mWindowClipPlanes.push_back(Plane(position, vw_ul, vw_ur));
        mWindowClipPlanes.push_back(Plane(position, vw_ur, vw_br));
        mWindowClipPlanes.push_back(Plane(position, vw_br, vw_bl));

        mRecalcWindow = false;

    }
    // -------------------------------------------------------------------
    const std::vector<Plane>& Camera::getWindowPlanes(void)
    {
        setWindowImpl();
        return mWindowClipPlanes;
    }
    // -------------------------------------------------------------------
    Real Camera::getBoundingRadius(void) const
    {
        // return a little bigger than the near distance
        // just to keep things just outside
        return mNearDist * 1.5;

    }
    //-----------------------------------------------------------------------
    const Vector3& Camera::getPositionForViewUpdate(void) const
    {
        // Note no update, because we're calling this from the update!
        return mDerivedPosition;
    }
    //-----------------------------------------------------------------------
    const Quaternion& Camera::getOrientationForViewUpdate(void) const
    {
        return mDerivedOrientation;
    }


} // namespace Ogre

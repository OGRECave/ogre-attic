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

        mRecalcFrustum = true;
        mRecalcView = true;

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


        updateView();
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
    void Camera::setProjectionType(ProjectionType pt)
    {
        mProjType = pt;
        mRecalcFrustum = true;
    }

    //-----------------------------------------------------------------------
    ProjectionType Camera::getProjectionType(void) const
    {
        return mProjType;
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
        mRecalcView = true;
    }

    //-----------------------------------------------------------------------
    void Camera::setPosition(const Vector3& vec)
    {
        mPosition = vec;
        mRecalcView = true;
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
        mRecalcView = true;
    }

    //-----------------------------------------------------------------------
    void Camera::moveRelative(const Vector3& vec)
    {
        // Transform the axes of the relative vector by camera's local axes
        Vector3 trans = mOrientation * vec;

        mPosition = mPosition + trans;
        mRecalcView = true;
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

        mRecalcView = true;

    }

    //-----------------------------------------------------------------------
    Vector3 Camera::getDirection(void) const
    {
        // Direction points down -Z by default
        return mOrientation * -Vector3::UNIT_Z;
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

        mRecalcView = true;
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

        mRecalcView = true;
    }

    //-----------------------------------------------------------------------
    void Camera::pitch(Real degrees)
    {
        // Rotate around local X axis
        Vector3 xAxis = mOrientation * Vector3::UNIT_X;
        rotate(xAxis, degrees);

        mRecalcView = true;

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
        mRecalcView = true;

    }

    //-----------------------------------------------------------------------
    void Camera::updateFrustum(void) const
    {
        // Overridden from Frustum to include API-specific projection
        if (mRecalcFrustum)
        {
            // Recalc if frustum params changed
            if (mProjType == PT_PERSPECTIVE)
            {
                // PERSPECTIVE transform, API specific
                Root::getSingleton().getRenderSystem()->_makeProjectionMatrix(mFOVy, 
                    mAspect, mNearDist, mFarDist, mProjMatrix);

                // PERSPECTIVE transform, API specific for Gpu Programs
                Root::getSingleton().getRenderSystem()->_makeProjectionMatrix(mFOVy, 
                    mAspect, mNearDist, mFarDist, mStandardProjMatrix, true);

                // standard perspective transform, not API specific
                Real thetaY = Math::AngleUnitsToRadians(mFOVy / 2.0f);
                Real tanThetaY = Math::Tan(thetaY);

                // Calculate co-efficients for the frustum planes
                // Special-cased for L = -R and B = -T i.e. viewport centered 
                // on direction vector.
                // Taken from ideas in WildMagic 0.2 http://www.magic-software.com
                //Real thetaX = thetaY * mAspect;
                Real tanThetaX = tanThetaY * mAspect;

                Real vpTop = tanThetaY * mNearDist;
                Real vpRight = tanThetaX * mNearDist;
                Real vpBottom = -vpTop;
                Real vpLeft = -vpRight;

                Real fNSqr = mNearDist * mNearDist;
                Real fLSqr = vpRight * vpRight;
                Real fRSqr = fLSqr;
                Real fTSqr = vpTop * vpTop;
                Real fBSqr = fTSqr;

                Real fInvLength = 1.0 / Math::Sqrt( fNSqr + fLSqr );
                mCoeffL[0] = mNearDist * fInvLength;
                mCoeffL[1] = -vpLeft * fInvLength;

                fInvLength = 1.0 / Math::Sqrt( fNSqr + fRSqr );
                mCoeffR[0] = -mNearDist * fInvLength;
                mCoeffR[1] = vpRight * fInvLength;

                fInvLength = 1.0 / Math::Sqrt( fNSqr + fBSqr );
                mCoeffB[0] = mNearDist * fInvLength;
                mCoeffB[1] = -vpBottom * fInvLength;

                fInvLength = 1.0 / Math::Sqrt( fNSqr + fTSqr );
                mCoeffT[0] = -mNearDist * fInvLength;
                mCoeffT[1] = vpTop * fInvLength;

            }
            else if (mProjType == PT_ORTHOGRAPHIC)
            {
                // ORTHOGRAPHIC projection

                // ---------------------------
                // Calculate Projection Matrix
                // ---------------------------
                // Get tangent of vertical FOV
               Real thetaY = Math::AngleUnitsToRadians(mFOVy / 2.0f);
                Real sinThetaY = Math::Sin(thetaY);
                Real thetaX = thetaY * mAspect;
                Real sinThetaX = Math::Sin(thetaX);
                Real w = 1.0 / (sinThetaX * mNearDist);
                Real h = 1.0 / (sinThetaY * mNearDist);
                Real q = 1.0 / (mFarDist - mNearDist);
                //Real qn = -(mFarDist + mNearDist) / (mFarDist - mNearDist);

                //----------------------------
                // Matrix elements
                //----------------------------

                // [ w   0   0   0  ]
                // [ 0   h   0   0  ]
                // [ 0   0   q   0  ]
                // [ 0   0   0   -1 ]


                mProjMatrix = Matrix4::ZERO;
                mProjMatrix[0][0] = w;
                mProjMatrix[1][1] = h;
                mProjMatrix[2][2] = -q;
                mProjMatrix[3][3] = 1;

                // Calculate co-efficients for the frustum planes
                // Special-cased for L = -R and B = -T i.e. viewport centered 
                // on direction vector.
                // Taken from ideas in WildMagic 0.2 http://www.magic-software.com
                Real vpTop = sinThetaY * mNearDist;
                Real vpRight = sinThetaX * mNearDist;
                Real vpBottom = -vpTop;
                Real vpLeft = -vpRight;

                Real fNSqr = mNearDist * mNearDist;
                Real fLSqr = vpRight * vpRight;
                Real fRSqr = fLSqr;
                Real fTSqr = vpTop * vpTop;
                Real fBSqr = fTSqr;

                Real fInvLength = 1.0 / Math::Sqrt( fNSqr + fLSqr );
                mCoeffL[0] = mNearDist * fInvLength;
                mCoeffL[1] = -vpLeft * fInvLength;

                fInvLength = 1.0 / Math::Sqrt( fNSqr + fRSqr );
                mCoeffR[0] = -mNearDist * fInvLength;
                mCoeffR[1] = vpRight * fInvLength;

                fInvLength = 1.0 / Math::Sqrt( fNSqr + fBSqr );
                mCoeffB[0] = mNearDist * fInvLength;
                mCoeffB[1] = -vpBottom * fInvLength;

                fInvLength = 1.0 / Math::Sqrt( fNSqr + fTSqr );
                mCoeffT[0] = -mNearDist * fInvLength;
                mCoeffT[1] = vpTop * fInvLength;

            }
            mRecalcFrustum = false;
        }
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
        if (isViewOutOfDate())
        {
            // ----------------------
            // Update the view matrix
            // ----------------------

            // View matrix is:
            //
            //  [ Lx  Uy  Dz  Tx  ]
            //  [ Lx  Uy  Dz  Ty  ]
            //  [ Lx  Uy  Dz  Tz  ]
            //  [ 0   0   0   1   ]
            //
            // Where T = -(Transposed(Rot) * Pos)

            // This is most efficiently done using 3x3 Matrices

            // Get orientation from quaternion

            Matrix3 rot;
            mDerivedOrientation.ToRotationMatrix(rot);
            Vector3 left = rot.GetColumn(0);
            Vector3 up = rot.GetColumn(1);
            Vector3 direction = rot.GetColumn(2);


            // Make the translation relative to new axes
            Matrix3 rotT = rot.Transpose();
            Vector3 trans = -rotT * mDerivedPosition;

            // Make final matrix
            // Must init entire matrix incase reflection was used
            mViewMatrix = Matrix4::IDENTITY;
            mViewMatrix = rotT; // fills upper 3x3
            mViewMatrix[0][3] = trans.x;
            mViewMatrix[1][3] = trans.y;
            mViewMatrix[2][3] = trans.z;

            // Deal with reflections
            if (mReflect)
            {
                mViewMatrix = mViewMatrix * mReflectMatrix;
            }


            // -------------------------
            // Update the frustum planes
            // -------------------------
            updateFrustum();
            // Use camera view direction for frustum, which is -Z not Z as for matrix calc
            Vector3 camDirection = mDerivedOrientation* -Vector3::UNIT_Z;
            // Calc distance along direction to position
            Real fDdE = camDirection.dotProduct(mDerivedPosition);

            // left plane
            mFrustumPlanes[FRUSTUM_PLANE_LEFT].normal = mCoeffL[0]*left +
                mCoeffL[1]*camDirection;
            mFrustumPlanes[FRUSTUM_PLANE_LEFT].d =
                -mDerivedPosition.dotProduct(mFrustumPlanes[FRUSTUM_PLANE_LEFT].normal);

            // right plane
            mFrustumPlanes[FRUSTUM_PLANE_RIGHT].normal = mCoeffR[0]*left +
                mCoeffR[1]*camDirection;
            mFrustumPlanes[FRUSTUM_PLANE_RIGHT].d =
                -mDerivedPosition.dotProduct(mFrustumPlanes[FRUSTUM_PLANE_RIGHT].normal);

            // bottom plane
            mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].normal = mCoeffB[0]*up +
                mCoeffB[1]*camDirection;
            mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].d =
                -mDerivedPosition.dotProduct(mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].normal);

            // top plane
            mFrustumPlanes[FRUSTUM_PLANE_TOP].normal = mCoeffT[0]*up +
                mCoeffT[1]*camDirection;
            mFrustumPlanes[FRUSTUM_PLANE_TOP].d =
                -mDerivedPosition.dotProduct(mFrustumPlanes[FRUSTUM_PLANE_TOP].normal);

            // far plane
            mFrustumPlanes[FRUSTUM_PLANE_FAR].normal = -camDirection;
            // d is distance along normal to origin
            mFrustumPlanes[FRUSTUM_PLANE_FAR].d = fDdE + mFarDist;

            // near plane
            mFrustumPlanes[FRUSTUM_PLANE_NEAR].normal = camDirection;
            mFrustumPlanes[FRUSTUM_PLANE_NEAR].d = -(fDdE + mNearDist);

            // Deal with reflection on frustum planes
            if (mReflect)
            {
                Vector3 pos = mReflectMatrix * mDerivedPosition;
                Vector3 dir = camDirection.reflect(mReflectPlane.normal);
                fDdE = dir.dotProduct(pos);
                for (unsigned int i = 0; i < 6; ++i)
                {
                    mFrustumPlanes[i].normal = mFrustumPlanes[i].normal.reflect(mReflectPlane.normal);
                    // Near / far plane dealt with differently since they don't pass through camera
                    switch (i)
                    {
                    case FRUSTUM_PLANE_NEAR:
                        mFrustumPlanes[i].d = -(fDdE + mNearDist);
                        break;
                    case FRUSTUM_PLANE_FAR:
                        mFrustumPlanes[i].d = fDdE + mFarDist;
                        break;
                    default:
                        mFrustumPlanes[i].d = -pos.dotProduct(mFrustumPlanes[i].normal);
                    }
                }
            }


            mRecalcView = false;

        }

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
        mRecalcView = true;
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
    void Camera::enableReflection(const Plane& p)
    {
        mReflect = true;
        mReflectPlane = p;
        mReflectMatrix = Math::buildReflectionMatrix(p);
        mRecalcView = true;
        
    }
    //-----------------------------------------------------------------------
    void Camera::disableReflection(void)
    {
        mReflect = false;
        mRecalcView = true;
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
} // namespace Ogre

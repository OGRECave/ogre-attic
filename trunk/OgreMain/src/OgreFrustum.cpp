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
#include "OgreFrustum.h"

#include "OgreMath.h"
#include "OgreMatrix3.h"
#include "OgreSceneNode.h"
#include "OgreSphere.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreCamera.h"
#include "OgreHardwareBufferManager.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"
#include "OgreMaterialManager.h"
#include "OgreRenderSystem.h"

namespace Ogre {

    String Frustum::msMovableType = "Frustum";
    //-----------------------------------------------------------------------
    Frustum::Frustum()
    {
        // Reasonable defaults to Frustum params
        mFOVy = Math::RadiansToAngleUnits(Math::PI/4.0);
        mNearDist = 100.0f;
        mFarDist = 100000.0f;
        mAspect = 1.33333333333333f;
        mProjType = PT_PERSPECTIVE;

        mRecalcFrustum = true;
        mRecalcView = true;

        // Init matrices
        mViewMatrix = Matrix4::ZERO;
        mProjMatrix = Matrix4::ZERO;
        mReflectMatrix = Matrix4::ZERO;

        // Initialise vertex & index data
        mVertexData.vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
        mVertexData.vertexCount = 32;
        mVertexData.vertexStart = 0;
        mVertexData.vertexBufferBinding->setBinding( 0,
            HardwareBufferManager::getSingleton().createVertexBuffer(
                sizeof(Real)*3, 32, HardwareBuffer::HBU_DYNAMIC) );

        // Initialise material
        mMaterial = static_cast<Material*>(
            MaterialManager::getSingleton().getByName("BaseWhiteNoLighting"));

        // Default to not visible
        mVisible = false;

        // no reflection
        mReflect = false;

        mParentNode = 0;

        updateView();
    }

    //-----------------------------------------------------------------------
    Frustum::~Frustum()
    {
        // Do nothing
    }

    //-----------------------------------------------------------------------
    void Frustum::setFOVy(Real fov)
    {
        mFOVy = fov;
        invalidateFrustum();
    }

    //-----------------------------------------------------------------------
    Real Frustum::getFOVy(void) const
    {
        return mFOVy;
    }


    //-----------------------------------------------------------------------
    void Frustum::setFarClipDistance(Real farPlane)
    {
        mFarDist = farPlane;
        invalidateFrustum();
    }

    //-----------------------------------------------------------------------
    Real Frustum::getFarClipDistance(void) const
    {
        return mFarDist;
    }

    //-----------------------------------------------------------------------
    void Frustum::setNearClipDistance(Real nearPlane)
    {
        if (nearPlane <= 0)
            Except(Exception::ERR_INVALIDPARAMS, "Near clip distance must be greater than zero.",
                "Frustum::setNearClipDistance");
        mNearDist = nearPlane;
        invalidateFrustum();
    }

    //-----------------------------------------------------------------------
    Real Frustum::getNearClipDistance(void) const
    {
        return mNearDist;
    }

    //-----------------------------------------------------------------------
    const Matrix4& Frustum::getProjectionMatrix(void) const
    {

        updateFrustum();

        return mProjMatrix;
    }
    //-----------------------------------------------------------------------
    const Matrix4& Frustum::getStandardProjectionMatrix(void) const
    {

        updateFrustum();

        return mStandardProjMatrix;
    }
    //-----------------------------------------------------------------------
    const Matrix4& Frustum::getViewMatrix(void) const
    {
        updateView();

        return mViewMatrix;

    }

    //-----------------------------------------------------------------------
    const Plane& Frustum::getFrustumPlane(unsigned short plane) const
    {
        // Make any pending updates to the calculated frustum
        updateView();

        return mFrustumPlanes[plane];

    }

    //-----------------------------------------------------------------------
    bool Frustum::isVisible(const AxisAlignedBox& bound, FrustumPlane* culledBy) const
    {
        // Null boxes always invisible
        if (bound.isNull()) return false;

        // Make any pending updates to the calculated frustum
        updateView();

        // Get corners of the box
        const Vector3* pCorners = bound.getAllCorners();


        // For each plane, see if all points are on the negative side
        // If so, object is not visible
        for (int plane = 0; plane < 6; ++plane)
        {
            if (mFrustumPlanes[plane].getSide(pCorners[0]) == Plane::NEGATIVE_SIDE &&
                mFrustumPlanes[plane].getSide(pCorners[1]) == Plane::NEGATIVE_SIDE &&
                mFrustumPlanes[plane].getSide(pCorners[2]) == Plane::NEGATIVE_SIDE &&
                mFrustumPlanes[plane].getSide(pCorners[3]) == Plane::NEGATIVE_SIDE &&
                mFrustumPlanes[plane].getSide(pCorners[4]) == Plane::NEGATIVE_SIDE &&
                mFrustumPlanes[plane].getSide(pCorners[5]) == Plane::NEGATIVE_SIDE &&
                mFrustumPlanes[plane].getSide(pCorners[6]) == Plane::NEGATIVE_SIDE &&
                mFrustumPlanes[plane].getSide(pCorners[7]) == Plane::NEGATIVE_SIDE)
            {
                // ALL corners on negative side therefore out of view
                if (culledBy)
                    *culledBy = (FrustumPlane)plane;
                return false;
            }

        }

        return true;
    }

    //-----------------------------------------------------------------------
    bool Frustum::isVisible(const Vector3& vert, FrustumPlane* culledBy) const
    {
        // Make any pending updates to the calculated frustum
        updateView();

        // For each plane, see if all points are on the negative side
        // If so, object is not visible
        for (int plane = 0; plane < 6; ++plane)
        {
            if (mFrustumPlanes[plane].getSide(vert) == Plane::NEGATIVE_SIDE)
            {
                // ALL corners on negative side therefore out of view
                if (culledBy)
                    *culledBy = (FrustumPlane)plane;
                return false;
            }

        }

        return true;
    }

    //-----------------------------------------------------------------------
    bool Frustum::isVisible(const Sphere& sphere, FrustumPlane* culledBy) const
    {
        // Make any pending updates to the calculated frustum
        updateView();

        // For each plane, see if sphere is on negative side
        // If so, object is not visible
        for (int plane = 0; plane < 6; ++plane)
        {
            // If the distance from sphere center to plane is negative, and 'more negative' 
            // than the radius of the sphere, sphere is outside frustum
            if (mFrustumPlanes[plane].getDistance(sphere.getCenter()) < -sphere.getRadius())
            {
                // ALL corners on negative side therefore out of view
                if (culledBy)
                    *culledBy = (FrustumPlane)plane;
                return false;
            }

        }

        return true;
    }
    //-----------------------------------------------------------------------
    void Frustum::updateFrustum(void) const
    {
        if (mRecalcFrustum)
        {
            // Common calcs
            Real thetaY = Math::AngleUnitsToRadians(mFOVy * 0.5f);
            Real tanThetaY = Math::Tan(thetaY);
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

            // Recalc if frustum params changed
            if (mProjType == PT_PERSPECTIVE)
            {

                // PERSPECTIVE transform, API specific
                Root::getSingleton().getRenderSystem()->_makeProjectionMatrix(mFOVy, 
                    mAspect, mNearDist, mFarDist, mProjMatrix);

                // PERSPECTIVE transform, API specific for Gpu Programs
                Root::getSingleton().getRenderSystem()->_makeProjectionMatrix(mFOVy, 
                    mAspect, mNearDist, mFarDist, mStandardProjMatrix, true);

                // Calculate co-efficients for the frustum planes
                // Special-cased for L = -R and B = -T i.e. viewport centered 
                // on direction vector.
                // Taken from ideas in WildMagic 0.2 http://www.magic-software.com
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
                // ORTHOGRAPHIC projection, API specific 
                Root::getSingleton().getRenderSystem()->_makeOrthoMatrix(mFOVy, 
                    mAspect, mNearDist, mFarDist, mProjMatrix);

                // Calculate co-efficients for the frustum planes
                // Special-cased for L = -R and B = -T i.e. viewport centered 
                // on direction vector.
                // Taken from ideas in WildMagic 0.2 http://www.magic-software.com

                // TODO: shouldn't this be changed for ortho?
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

            
            // Calculate bounding box (local)
            // Box is from 0, down -Z, max dimensions as determined from far plane
            Real farTop = tanThetaY * (mProjType == PT_ORTHOGRAPHIC? mNearDist : mFarDist);
            Real farRight = tanThetaX * (mProjType == PT_ORTHOGRAPHIC? mNearDist : mFarDist);
            Real farBottom = -farTop;
            Real farLeft = -farRight;
            Vector3 min(-farRight, -farTop, 0);
            Vector3 max(farRight, farTop, -mFarDist);
            mBoundingBox.setExtents(min, max);

            // Calculate vertex positions (local)
            // 0 is the origin
            // 1, 2, 3, 4 are the points on the near plane, top left first, clockwise
            // 5, 6, 7, 8 are the points on the far plane, top left first, clockwise
            HardwareVertexBufferSharedPtr vbuf = mVertexData.vertexBufferBinding->getBuffer(0);
            Real* pReal = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));

            // near plane (remember frustum is going in -Z direction)
            *pReal++ = vpLeft;  *pReal++ = vpTop;    *pReal++ = -mNearDist;
            *pReal++ = vpRight; *pReal++ = vpTop;    *pReal++ = -mNearDist;

            *pReal++ = vpRight; *pReal++ = vpTop;    *pReal++ = -mNearDist;
            *pReal++ = vpRight; *pReal++ = vpBottom; *pReal++ = -mNearDist;

            *pReal++ = vpRight; *pReal++ = vpBottom; *pReal++ = -mNearDist;
            *pReal++ = vpLeft;  *pReal++ = vpBottom; *pReal++ = -mNearDist;

            *pReal++ = vpLeft;  *pReal++ = vpBottom; *pReal++ = -mNearDist;
            *pReal++ = vpLeft;  *pReal++ = vpTop;    *pReal++ = -mNearDist;

            // far plane (remember frustum is going in -Z direction)
            *pReal++ = farLeft;  *pReal++ = farTop;    *pReal++ = -mFarDist;
            *pReal++ = farRight; *pReal++ = farTop;    *pReal++ = -mFarDist;

            *pReal++ = farRight; *pReal++ = farTop;    *pReal++ = -mFarDist;
            *pReal++ = farRight; *pReal++ = farBottom; *pReal++ = -mFarDist;

            *pReal++ = farRight; *pReal++ = farBottom; *pReal++ = -mFarDist;
            *pReal++ = farLeft;  *pReal++ = farBottom; *pReal++ = -mFarDist;

            *pReal++ = farLeft;  *pReal++ = farBottom; *pReal++ = -mFarDist;
            *pReal++ = farLeft;  *pReal++ = farTop;    *pReal++ = -mFarDist;

            // Sides of the pyramid
            *pReal++ = 0.0f;    *pReal++ = 0.0f;   *pReal++ = 0.0f;
            *pReal++ = vpLeft;  *pReal++ = vpTop;  *pReal++ = -mNearDist;

            *pReal++ = 0.0f;    *pReal++ = 0.0f;   *pReal++ = 0.0f;
            *pReal++ = vpRight; *pReal++ = vpTop;    *pReal++ = -mNearDist;

            *pReal++ = 0.0f;    *pReal++ = 0.0f;   *pReal++ = 0.0f;
            *pReal++ = vpRight; *pReal++ = vpBottom; *pReal++ = -mNearDist;

            *pReal++ = 0.0f;    *pReal++ = 0.0f;   *pReal++ = 0.0f;
            *pReal++ = vpLeft;  *pReal++ = vpBottom; *pReal++ = -mNearDist;

            // Sides of the box

            *pReal++ = vpLeft;  *pReal++ = vpTop;  *pReal++ = -mNearDist;
            *pReal++ = farLeft;  *pReal++ = farTop;  *pReal++ = -mFarDist;

            *pReal++ = vpRight; *pReal++ = vpTop;    *pReal++ = -mNearDist;
            *pReal++ = farRight; *pReal++ = farTop;    *pReal++ = -mFarDist;

            *pReal++ = vpRight; *pReal++ = vpBottom; *pReal++ = -mNearDist;
            *pReal++ = farRight; *pReal++ = farBottom; *pReal++ = -mFarDist;

            *pReal++ = vpLeft;  *pReal++ = vpBottom; *pReal++ = -mNearDist;
            *pReal++ = farLeft;  *pReal++ = farBottom; *pReal++ = -mFarDist;


            vbuf->unlock();

            mRecalcFrustum = false;
        }
    }

    //-----------------------------------------------------------------------
    bool Frustum::isViewOutOfDate(void) const
    {
        // Attached to node?
        if (mParentNode)
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
                return true;
            }
        }
        return mRecalcView;
    }

    //-----------------------------------------------------------------------
    bool Frustum::isFrustumOutOfDate(void) const
    {
        return mRecalcFrustum;
    }

    //-----------------------------------------------------------------------
    void Frustum::updateView(void) const
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
            const Quaternion& orientation = getOrientationForViewUpdate();
            const Vector3& position = getPositionForViewUpdate();
			orientation.ToRotationMatrix(rot);
            Vector3 left = rot.GetColumn(0);
            Vector3 up = rot.GetColumn(1);
            Vector3 direction = rot.GetColumn(2);


            // Make the translation relative to new axes
            Matrix3 rotT = rot.Transpose();
            Vector3 trans = -rotT * position;

            // Make final matrix
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
            // Use Frustum view direction for frustum, which is -Z not Z as for matrix calc
            Vector3 camDirection = orientation* -Vector3::UNIT_Z;
            // Calc distance along direction to position
            Real fDdE = camDirection.dotProduct(position);

            // left plane
            mFrustumPlanes[FRUSTUM_PLANE_LEFT].normal = mCoeffL[0]*left +
                    mCoeffL[1]*camDirection;
            mFrustumPlanes[FRUSTUM_PLANE_LEFT].d =
                    -position.dotProduct(mFrustumPlanes[FRUSTUM_PLANE_LEFT].normal);

            // right plane
            mFrustumPlanes[FRUSTUM_PLANE_RIGHT].normal = mCoeffR[0]*left +
                    mCoeffR[1]*camDirection;
            mFrustumPlanes[FRUSTUM_PLANE_RIGHT].d =
                    -position.dotProduct(mFrustumPlanes[FRUSTUM_PLANE_RIGHT].normal);

            // bottom plane
            mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].normal = mCoeffB[0]*up +
                    mCoeffB[1]*camDirection;
            mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].d =
                    -position.dotProduct(mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].normal);

            // top plane
            mFrustumPlanes[FRUSTUM_PLANE_TOP].normal = mCoeffT[0]*up +
                    mCoeffT[1]*camDirection;
            mFrustumPlanes[FRUSTUM_PLANE_TOP].d =
                    -position.dotProduct(mFrustumPlanes[FRUSTUM_PLANE_TOP].normal);

            // far plane
            mFrustumPlanes[FRUSTUM_PLANE_FAR].normal = -camDirection;
            // d is distance along normal to origin
            mFrustumPlanes[FRUSTUM_PLANE_FAR].d = fDdE + mFarDist;

            // near plane
            mFrustumPlanes[FRUSTUM_PLANE_NEAR].normal = camDirection;
            mFrustumPlanes[FRUSTUM_PLANE_NEAR].d = -(fDdE + mNearDist);

            // Update worldspace corners
            Matrix4 eyeToWorld = mViewMatrix.inverse();
            // Get worldspace frustum corners
            Real y = Math::Tan(mFOVy * 0.5);
            Real x = mAspect * y;
            Real neary = y * mNearDist;
            Real fary = y * (mProjType == PT_ORTHOGRAPHIC? mNearDist : mFarDist);
            Real nearx = x * mNearDist;
            Real farx = x * (mProjType == PT_ORTHOGRAPHIC? mNearDist : mFarDist);
            // near
            mWorldSpaceCorners[0] = eyeToWorld * Vector3( nearx,  neary, -mNearDist);
            mWorldSpaceCorners[1] = eyeToWorld * Vector3(-nearx,  neary, -mNearDist);
            mWorldSpaceCorners[2] = eyeToWorld * Vector3(-nearx, -neary, -mNearDist);
            mWorldSpaceCorners[3] = eyeToWorld * Vector3( nearx, -neary, -mNearDist);
            // far
            mWorldSpaceCorners[4] = eyeToWorld * Vector3( farx,  fary, -mFarDist);
            mWorldSpaceCorners[5] = eyeToWorld * Vector3(-farx,  fary, -mFarDist);
            mWorldSpaceCorners[6] = eyeToWorld * Vector3(-farx, -fary, -mFarDist);
            mWorldSpaceCorners[7] = eyeToWorld * Vector3( farx, -fary, -mFarDist);

            // Deal with reflection on frustum planes
            if (mReflect)
            {
                Vector3 pos = mReflectMatrix * position;
                Vector3 dir = camDirection.reflect(mReflectPlane.normal);
                fDdE = dir.dotProduct(pos);
                unsigned int i;
                for (i = 0; i < 6; ++i)
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
                // Also reflect corners
                for (i = 0; i < 8; ++i)
                {
                    mWorldSpaceCorners[i] = mReflectMatrix * mWorldSpaceCorners[i];
                }
            }

            mRecalcView = false;

        }

    }

    //-----------------------------------------------------------------------
    Real Frustum::getAspectRatio(void) const
    {
        return mAspect;
    }

    //-----------------------------------------------------------------------
    void Frustum::setAspectRatio(Real r)
    {
        mAspect = r;
        invalidateFrustum();
    }

    //-----------------------------------------------------------------------
    const AxisAlignedBox& Frustum::getBoundingBox(void) const
    {
        return mBoundingBox;
    }
    //-----------------------------------------------------------------------
    void Frustum::_updateRenderQueue(RenderQueue* queue)
    {
        // Add self 
        queue->addRenderable(this);
    }
    //-----------------------------------------------------------------------
    const String& Frustum::getMovableType(void) const
    {
        return msMovableType;
    }
    //-----------------------------------------------------------------------
	Real Frustum::getBoundingRadius(void) const
	{
		return mFarDist;
	}
    //-----------------------------------------------------------------------
    Material* Frustum::getMaterial(void) const
    {
        return mMaterial;
    }
    //-----------------------------------------------------------------------
    void Frustum::getRenderOperation(RenderOperation& op) 
    {
        updateView();
        updateFrustum();
        op.operationType = RenderOperation::OT_LINE_LIST;
        op.useIndexes = false;
        op.vertexData = &mVertexData;
    }
    //-----------------------------------------------------------------------
    void Frustum::getWorldTransforms(Matrix4* xform) const 
    {
        if (mParentNode)
            mParentNode->getWorldTransforms(xform);
    }
    //-----------------------------------------------------------------------
    const Quaternion& Frustum::getWorldOrientation(void) const 
    {
        if (mParentNode)
            return mParentNode->_getDerivedOrientation();
        else
            return Quaternion::IDENTITY;
    }
    //-----------------------------------------------------------------------
    const Vector3& Frustum::getWorldPosition(void) const 
    {
        if (mParentNode)
            return mParentNode->_getDerivedPosition();
        else
            return Vector3::ZERO;
    }
    //-----------------------------------------------------------------------
    Real Frustum::getSquaredViewDepth(const Camera* cam) const 
    {
        // Calc from centre
        if (mParentNode)
            return (cam->getDerivedPosition() 
                - mParentNode->_getDerivedPosition()).squaredLength();
        else
            return 0;
    }
    //-----------------------------------------------------------------------
    const LightList& Frustum::getLights(void) const 
    {
        // N/A
        static LightList ll;
        return ll;
    }
    //-----------------------------------------------------------------------
    const String& Frustum::getName(void) const
    {
        // NA
        return msMovableType;
    }
    //-----------------------------------------------------------------------
    void Frustum::_notifyCurrentCamera(Camera* cam)
    {
        // NA
    }

    // -------------------------------------------------------------------
    void Frustum::invalidateFrustum()
    {
        mRecalcFrustum = true;
    }
    // -------------------------------------------------------------------
    void Frustum::invalidateView()
    {
        mRecalcView = true;
    }
    // -------------------------------------------------------------------
    const Vector3* Frustum::getWorldSpaceCorners(void) const
    {
        updateView();

        return mWorldSpaceCorners;
    }
    //-----------------------------------------------------------------------
    void Frustum::setProjectionType(ProjectionType pt)
    {
        mProjType = pt;
        invalidateFrustum();
    }

    //-----------------------------------------------------------------------
    ProjectionType Frustum::getProjectionType(void) const
    {
        return mProjType;
    }
    //-----------------------------------------------------------------------
    const Vector3& Frustum::getPositionForViewUpdate(void) const
    {
        return mLastParentPosition;
    }
    //-----------------------------------------------------------------------
    const Quaternion& Frustum::getOrientationForViewUpdate(void) const
    {
        return mLastParentOrientation;
    }
    //-----------------------------------------------------------------------
    void Frustum::enableReflection(const Plane& p)
    {
        mReflect = true;
        mReflectPlane = p;
        mReflectMatrix = Math::buildReflectionMatrix(p);
        invalidateView();

    }
    //-----------------------------------------------------------------------
    void Frustum::disableReflection(void)
    {
        mReflect = false;
        invalidateView();
    }



} // namespace Ogre

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
    const Real Frustum::INFINITE_FAR_PLANE_ADJUST = 0.00001;
    //-----------------------------------------------------------------------
    Frustum::Frustum() : 
        mProjType(PT_PERSPECTIVE), 
        mFOVy(Radian(Math::PI/4.0)), 
        mFarDist(100000.0f), 
        mNearDist(100.0f), 
        mAspect(1.33333333333333f), 
        mProjMatrix(Matrix4::ZERO), 
        mViewMatrix(Matrix4::ZERO), 
        mRecalcFrustum(true), 
        mRecalcView(true), 
        mReflect(false), 
        mReflectMatrix(Matrix4::ZERO), 
        mLinkedReflectPlane(0),
        mObliqueDepthProjection(false), 
        mLinkedObliqueProjPlane(0)
    {
        // Initialise vertex & index data
        mVertexData.vertexDeclaration->addElement(0, 0, VET_FLOAT3, VES_POSITION);
        mVertexData.vertexCount = 32;
        mVertexData.vertexStart = 0;
        mVertexData.vertexBufferBinding->setBinding( 0,
            HardwareBufferManager::getSingleton().createVertexBuffer(
                sizeof(Real)*3, 32, HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY) );

        // Initialise material
        mMaterial = MaterialManager::getSingleton().getByName("BaseWhiteNoLighting"));
        
        // Alter superclass members
        mVisible = false;
        mParentNode = 0;

        mLastLinkedReflectionPlane.normal = Vector3::ZERO;
        mLastLinkedObliqueProjPlane.normal = Vector3::ZERO;


        updateView();
    }

    //-----------------------------------------------------------------------
    Frustum::~Frustum()
    {
        // Do nothing
    }

    //-----------------------------------------------------------------------
    void Frustum::setFOVy(const Radian& fov)
    {
        mFOVy = fov;
        invalidateFrustum();
    }

    //-----------------------------------------------------------------------
    const Radian& Frustum::getFOVy(void) const
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
            // Skip far plane if infinite view frustum
            if (mFarDist == 0 && plane == FRUSTUM_PLANE_FAR)
                continue;

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
            // Skip far plane if infinite view frustum
            if (mFarDist == 0 && plane == FRUSTUM_PLANE_FAR)
                continue;

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
            // Skip far plane if infinite view frustum
            if (mFarDist == 0 && plane == FRUSTUM_PLANE_FAR)
                continue;

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
        if (isFrustumOutOfDate())
        {
            // Common calcs
            Radian thetaY (mFOVy * 0.5f);
            Real tanThetaY = Math::Tan(thetaY);
            Real tanThetaX = tanThetaY * mAspect;
            Real vpTop = tanThetaY * mNearDist;
            Real vpRight = tanThetaX * mNearDist;
            Real vpBottom = -vpTop;
            Real vpLeft = -vpRight;

            RenderSystem* renderSystem = Root::getSingleton().getRenderSystem();
            // Recalc if frustum params changed
            if (mProjType == PT_PERSPECTIVE)
            {

                // PERSPECTIVE transform, API specific
                renderSystem->_makeProjectionMatrix(mFOVy, 
                    mAspect, mNearDist, mFarDist, mProjMatrix);

                // PERSPECTIVE transform, API specific for Gpu Programs
                renderSystem->_makeProjectionMatrix(mFOVy, 
                    mAspect, mNearDist, mFarDist, mStandardProjMatrix, true);

                if (mObliqueDepthProjection)
                {
                    // Translate the plane into view space
                    Plane viewSpaceNear = mViewMatrix * mObliqueProjPlane;
                    renderSystem->_applyObliqueDepthProjection(
                        mProjMatrix, viewSpaceNear, false);
                    renderSystem->_applyObliqueDepthProjection(
                        mStandardProjMatrix, viewSpaceNear, true);
                }
            }
            else if (mProjType == PT_ORTHOGRAPHIC)
            {
                // ORTHOGRAPHIC projection, API specific 
                Root::getSingleton().getRenderSystem()->_makeOrthoMatrix(mFOVy, 
                    mAspect, mNearDist, mFarDist, mProjMatrix);

                // ORTHOGRAPHIC projection, non-API specific 
                Root::getSingleton().getRenderSystem()->_makeOrthoMatrix(mFOVy, 
                    mAspect, mNearDist, mFarDist, mStandardProjMatrix, true);


            }

            
            // Calculate bounding box (local)
            // Box is from 0, down -Z, max dimensions as determined from far plane
            // If infinite view frustum just pick a far value
            Real farDist = (mFarDist == 0) ? 100000 : mFarDist;
            Real farTop = tanThetaY * (mProjType == PT_ORTHOGRAPHIC? mNearDist : farDist);
            Real farRight = tanThetaX * (mProjType == PT_ORTHOGRAPHIC? mNearDist : farDist);
            Real farBottom = -farTop;
            Real farLeft = -farRight;
            Vector3 min(-farRight, -farTop, 0);
            Vector3 max(farRight, farTop, -farDist);
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
            *pReal++ = farLeft;  *pReal++ = farTop;    *pReal++ = -farDist;
            *pReal++ = farRight; *pReal++ = farTop;    *pReal++ = -farDist;

            *pReal++ = farRight; *pReal++ = farTop;    *pReal++ = -farDist;
            *pReal++ = farRight; *pReal++ = farBottom; *pReal++ = -farDist;

            *pReal++ = farRight; *pReal++ = farBottom; *pReal++ = -farDist;
            *pReal++ = farLeft;  *pReal++ = farBottom; *pReal++ = -farDist;

            *pReal++ = farLeft;  *pReal++ = farBottom; *pReal++ = -farDist;
            *pReal++ = farLeft;  *pReal++ = farTop;    *pReal++ = -farDist;

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
            *pReal++ = farLeft;  *pReal++ = farTop;  *pReal++ = -farDist;

            *pReal++ = vpRight; *pReal++ = vpTop;    *pReal++ = -mNearDist;
            *pReal++ = farRight; *pReal++ = farTop;    *pReal++ = -farDist;

            *pReal++ = vpRight; *pReal++ = vpBottom; *pReal++ = -mNearDist;
            *pReal++ = farRight; *pReal++ = farBottom; *pReal++ = -farDist;

            *pReal++ = vpLeft;  *pReal++ = vpBottom; *pReal++ = -mNearDist;
            *pReal++ = farLeft;  *pReal++ = farBottom; *pReal++ = -farDist;


            vbuf->unlock();

            mRecalcFrustum = false;
        }
    }

    //-----------------------------------------------------------------------
    bool Frustum::isViewOutOfDate(void) const
    {
        bool returnVal = false;
        // Attached to node?
        if (mParentNode)
        {
            if (!mRecalcView && mParentNode->_getDerivedOrientation() == mLastParentOrientation &&
                mParentNode->_getDerivedPosition() == mLastParentPosition)
            {
                returnVal = false;
            }
            else
            {
                // Ok, we're out of date with SceneNode we're attached to
                mLastParentOrientation = mParentNode->_getDerivedOrientation();
                mLastParentPosition = mParentNode->_getDerivedPosition();
                returnVal = true;
            }
        }
        // Deriving reflection from linked plane?
        if (mReflect && mLinkedReflectPlane && 
            !(mLastLinkedReflectionPlane == mLinkedReflectPlane->_getDerivedPlane()))
        {
            mReflectPlane = mLinkedReflectPlane->_getDerivedPlane();
            mReflectMatrix = Math::buildReflectionMatrix(mReflectPlane);
            mLastLinkedReflectionPlane = mLinkedReflectPlane->_getDerivedPlane();
            returnVal = true;
        }

        return mRecalcView || returnVal;
    }

    //-----------------------------------------------------------------------
    bool Frustum::isFrustumOutOfDate(void) const
    {
        // Deriving custom near plane from linked plane?
        bool returnVal = false;
        if (mObliqueDepthProjection)
        {
            // Always out of date since plane needs to be in view space
            returnVal = true;
            // Update derived plane
            if (mLinkedObliqueProjPlane && 
                !(mLastLinkedObliqueProjPlane == mLinkedObliqueProjPlane->_getDerivedPlane()))
            {
                mObliqueProjPlane = mLinkedObliqueProjPlane->_getDerivedPlane();
                mLastLinkedObliqueProjPlane = mObliqueProjPlane;
            }
        }

        return mRecalcFrustum || returnVal;
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

            Matrix4 combo = mStandardProjMatrix * mViewMatrix;
            mFrustumPlanes[FRUSTUM_PLANE_LEFT].normal.x = combo[3][0] + combo[0][0];
            mFrustumPlanes[FRUSTUM_PLANE_LEFT].normal.y = combo[3][1] + combo[0][1];
            mFrustumPlanes[FRUSTUM_PLANE_LEFT].normal.z = combo[3][2] + combo[0][2];
            mFrustumPlanes[FRUSTUM_PLANE_LEFT].d = combo[3][3] + combo[0][3];

            mFrustumPlanes[FRUSTUM_PLANE_RIGHT].normal.x = combo[3][0] - combo[0][0];
            mFrustumPlanes[FRUSTUM_PLANE_RIGHT].normal.y = combo[3][1] - combo[0][1];
            mFrustumPlanes[FRUSTUM_PLANE_RIGHT].normal.z = combo[3][2] - combo[0][2];
            mFrustumPlanes[FRUSTUM_PLANE_RIGHT].d = combo[3][3] - combo[0][3];

            mFrustumPlanes[FRUSTUM_PLANE_TOP].normal.x = combo[3][0] - combo[1][0];
            mFrustumPlanes[FRUSTUM_PLANE_TOP].normal.y = combo[3][1] - combo[1][1];
            mFrustumPlanes[FRUSTUM_PLANE_TOP].normal.z = combo[3][2] - combo[1][2];
            mFrustumPlanes[FRUSTUM_PLANE_TOP].d = combo[3][3] - combo[1][3];

            mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].normal.x = combo[3][0] + combo[1][0];
            mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].normal.y = combo[3][1] + combo[1][1];
            mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].normal.z = combo[3][2] + combo[1][2];
            mFrustumPlanes[FRUSTUM_PLANE_BOTTOM].d = combo[3][3] + combo[1][3];

            mFrustumPlanes[FRUSTUM_PLANE_NEAR].normal.x = combo[3][0] + combo[2][0];
            mFrustumPlanes[FRUSTUM_PLANE_NEAR].normal.y = combo[3][1] + combo[2][1];
            mFrustumPlanes[FRUSTUM_PLANE_NEAR].normal.z = combo[3][2] + combo[2][2];
            mFrustumPlanes[FRUSTUM_PLANE_NEAR].d = combo[3][3] + combo[2][3];

            mFrustumPlanes[FRUSTUM_PLANE_FAR].normal.x = combo[3][0] - combo[2][0];
            mFrustumPlanes[FRUSTUM_PLANE_FAR].normal.y = combo[3][1] - combo[2][1];
            mFrustumPlanes[FRUSTUM_PLANE_FAR].normal.z = combo[3][2] - combo[2][2];
            mFrustumPlanes[FRUSTUM_PLANE_FAR].d = combo[3][3] - combo[2][3];

            // Renormalise any normals which were not unit length
            for(int i=0; i<6; i++ ) 
            {
                float length = mFrustumPlanes[i].normal.normalise();
                mFrustumPlanes[i].d /= length;
            }

            // Update worldspace corners
            Matrix4 eyeToWorld = mViewMatrix.inverse();
            // Get worldspace frustum corners
            // Treat infinite fardist as some arbitrary far value
            Real farDist = (mFarDist == 0)? 100000 : mFarDist;
            Real y = Math::Tan(mFOVy * 0.5);
            Real x = mAspect * y;
            Real neary = y * mNearDist;
            Real fary = y * (mProjType == PT_ORTHOGRAPHIC? mNearDist : farDist);
            Real nearx = x * mNearDist;
            Real farx = x * (mProjType == PT_ORTHOGRAPHIC? mNearDist : farDist);
            // near
            mWorldSpaceCorners[0] = eyeToWorld * Vector3( nearx,  neary, -mNearDist);
            mWorldSpaceCorners[1] = eyeToWorld * Vector3(-nearx,  neary, -mNearDist);
            mWorldSpaceCorners[2] = eyeToWorld * Vector3(-nearx, -neary, -mNearDist);
            mWorldSpaceCorners[3] = eyeToWorld * Vector3( nearx, -neary, -mNearDist);
            // far
            mWorldSpaceCorners[4] = eyeToWorld * Vector3( farx,  fary, -farDist);
            mWorldSpaceCorners[5] = eyeToWorld * Vector3(-farx,  fary, -farDist);
            mWorldSpaceCorners[6] = eyeToWorld * Vector3(-farx, -fary, -farDist);
            mWorldSpaceCorners[7] = eyeToWorld * Vector3( farx, -fary, -farDist);


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
        return (mFarDist == 0)? 100000 : mFarDist;
	}
    //-----------------------------------------------------------------------
    const MaterialPtr& Frustum::getMaterial(void) const
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
    void Frustum::invalidateFrustum() const
    {
        mRecalcFrustum = true;
    }
    // -------------------------------------------------------------------
    void Frustum::invalidateView() const
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
        mLinkedReflectPlane = 0;
        mReflectMatrix = Math::buildReflectionMatrix(p);
        invalidateView();

    }
    //-----------------------------------------------------------------------
    void Frustum::enableReflection(const MovablePlane* p)
    {
        mReflect = true;
        mLinkedReflectPlane = p;
        mReflectPlane = mLinkedReflectPlane->_getDerivedPlane();
        mReflectMatrix = Math::buildReflectionMatrix(mReflectPlane);
        mLastLinkedReflectionPlane = mLinkedReflectPlane->_getDerivedPlane();
        invalidateView();
    }
    //-----------------------------------------------------------------------
    void Frustum::disableReflection(void)
    {
        mReflect = false;
        mLastLinkedReflectionPlane.normal = Vector3::ZERO;
        invalidateView();
    }
    //---------------------------------------------------------------------
    bool Frustum::projectSphere(const Sphere& sphere, 
        Real* left, Real* top, Real* right, Real* bottom) const
    {
        // initialise
        *left = *bottom = -1.0f;
        *right = *top = 1.0f;

        // Transform light position into camera space
        Vector3 eyeSpacePos = getViewMatrix() * sphere.getCenter();

        if (eyeSpacePos.z < 0)
        {
            Real r = sphere.getRadius();
            // early-exit
            if (eyeSpacePos.squaredLength() <= r * r)
                return false;

            Vector3 screenSpacePos = getStandardProjectionMatrix() * eyeSpacePos;


            // perspective attenuate
            Vector3 spheresize(r, r, eyeSpacePos.z);
            spheresize = getStandardProjectionMatrix() * spheresize;

            Real possLeft = screenSpacePos.x - spheresize.x;
            Real possRight = screenSpacePos.x + spheresize.x;
            Real possTop = screenSpacePos.y + spheresize.y;
            Real possBottom = screenSpacePos.y - spheresize.y;

            *left = std::max(-1.0f, possLeft);
            *right = std::min(1.0f, possRight);
            *top = std::min(1.0f, possTop);
            *bottom = std::max(-1.0f, possBottom);

        }

        return (*left != -1.0f) || (*top != 1.0f) || (*right != 1.0f) || (*bottom != -1.0f);

    }
    //---------------------------------------------------------------------
    void Frustum::enableCustomNearClipPlane(const MovablePlane* plane)
    {
        mObliqueDepthProjection = true;
        mLinkedObliqueProjPlane = plane;
        mObliqueProjPlane = plane->_getDerivedPlane();
        invalidateFrustum();
    }
    //---------------------------------------------------------------------
    void Frustum::enableCustomNearClipPlane(const Plane& plane)
    {
        mObliqueDepthProjection = true;
        mLinkedObliqueProjPlane = 0;
        mObliqueProjPlane = plane;
        invalidateFrustum();
    }
    //---------------------------------------------------------------------
    void Frustum::disableCustomNearClipPlane(void)
    {
        mObliqueDepthProjection = false;
        mLinkedObliqueProjPlane = 0;
        invalidateFrustum();
    }
    //---------------------------------------------------------------------


} // namespace Ogre

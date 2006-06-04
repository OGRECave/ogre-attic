/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 The OGRE Team
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

#include "OgreOptimisedUtil.h"

#include "OgreVector3.h"
#include "OgreMatrix4.h"

namespace Ogre {

//-------------------------------------------------------------------------
// Local classes
//-------------------------------------------------------------------------

    /** General implementation of OptimisedUtil.
    @note
        Don't use this class directly, use OptimisedUtil instead.
    */
    class _OgrePrivate OptimisedUtilGeneral : public OptimisedUtil
    {
    public:
        /// @copydoc OptimisedUtil::softwareVertexSkinning
        virtual void softwareVertexSkinning(
            const float *srcPosPtr, float *destPosPtr,
            const float *srcNormPtr, float *destNormPtr,
            const float *blendWeightPtr, const unsigned char* blendIndexPtr,
            const Matrix4* const* blendMatrices,
            size_t srcPosStride, size_t destPosStride,
            size_t srcNormStride, size_t destNormStride,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numVertices);

        /// @copydoc OptimisedUtil::concatenateAffineMatrices
        virtual void concatenateAffineMatrices(
            const Matrix4& baseMatrix,
            const Matrix4* srcMatrices,
            Matrix4* dstMatrices,
            size_t numMatrices);
    };
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void OptimisedUtilGeneral::softwareVertexSkinning(
        const float *pSrcPos, float *pDestPos,
        const float *pSrcNorm, float *pDestNorm,
        const float *pBlendWeight, const unsigned char* pBlendIndex,
        const Matrix4* const* blendMatrices,
        size_t srcPosStride, size_t destPosStride,
        size_t srcNormStride, size_t destNormStride,
        size_t blendWeightStride, size_t blendIndexStride,
        size_t numWeightsPerVertex,
        size_t numVertices)
    {
        // Source vectors
        Vector3 sourceVec, sourceNorm;
        // Accumulation vectors
        Vector3 accumVecPos, accumVecNorm;

        // Loop per vertex
        for (size_t vertIdx = 0; vertIdx < numVertices; ++vertIdx)
        {
            // Load source vertex elements
            sourceVec.x = pSrcPos[0];
            sourceVec.y = pSrcPos[1];
            sourceVec.z = pSrcPos[2];

            if (pSrcNorm)
            {
                sourceNorm.x = pSrcNorm[0];
                sourceNorm.y = pSrcNorm[1];
                sourceNorm.z = pSrcNorm[2];
            }

            // Load accumulators
            accumVecPos = Vector3::ZERO;
            accumVecNorm = Vector3::ZERO;

            // Loop per blend weight
            //
            // Note: Don't change "unsigned short" here!!! If use "size_t" instead,
            // VC7.1 unroll this loop to four blend weights pre-iteration, and then
            // loss performance 10% in this function. Ok, this give a hint that we
            // should unroll this loop manually for better performance, will do that
            // later.
            //
            for (unsigned short blendIdx = 0; blendIdx < numWeightsPerVertex; ++blendIdx)
            {
                // Blend by multiplying source by blend matrix and scaling by weight
                // Add to accumulator
                // NB weights must be normalised!!
                Real weight = pBlendWeight[blendIdx];
                if (weight)
                {
                    // Blend position, use 3x4 matrix
                    const Matrix4& mat = *blendMatrices[pBlendIndex[blendIdx]];
                    accumVecPos.x +=
                        (mat[0][0] * sourceVec.x +
                         mat[0][1] * sourceVec.y +
                         mat[0][2] * sourceVec.z +
                         mat[0][3])
                         * weight;
                    accumVecPos.y +=
                        (mat[1][0] * sourceVec.x +
                         mat[1][1] * sourceVec.y +
                         mat[1][2] * sourceVec.z +
                         mat[1][3])
                         * weight;
                    accumVecPos.z +=
                        (mat[2][0] * sourceVec.x +
                         mat[2][1] * sourceVec.y +
                         mat[2][2] * sourceVec.z +
                         mat[2][3])
                         * weight;
                    if (pSrcNorm)
                    {
                        // Blend normal
                        // We should blend by inverse transpose here, but because we're assuming the 3x3
                        // aspect of the matrix is orthogonal (no non-uniform scaling), the inverse transpose
                        // is equal to the main 3x3 matrix
                        // Note because it's a normal we just extract the rotational part, saves us renormalising here
                        accumVecNorm.x +=
                            (mat[0][0] * sourceNorm.x +
                             mat[0][1] * sourceNorm.y +
                             mat[0][2] * sourceNorm.z)
                             * weight;
                        accumVecNorm.y +=
                            (mat[1][0] * sourceNorm.x +
                             mat[1][1] * sourceNorm.y +
                             mat[1][2] * sourceNorm.z)
                            * weight;
                        accumVecNorm.z +=
                            (mat[2][0] * sourceNorm.x +
                             mat[2][1] * sourceNorm.y +
                             mat[2][2] * sourceNorm.z)
                            * weight;
                    }
                }
            }

            // Stored blended vertex in hardware buffer
            pDestPos[0] = accumVecPos.x;
            pDestPos[1] = accumVecPos.y;
            pDestPos[2] = accumVecPos.z;

            // Stored blended vertex in temp buffer
            if (pSrcNorm)
            {
                // Normalise
                accumVecNorm.normalise();
                pDestNorm[0] = accumVecNorm.x;
                pDestNorm[1] = accumVecNorm.y;
                pDestNorm[2] = accumVecNorm.z;
                // Advance pointers
                advanceRawPointer(pSrcNorm, srcNormStride);
                advanceRawPointer(pDestNorm, destNormStride);
            }

            // Advance pointers
            advanceRawPointer(pSrcPos, srcPosStride);
            advanceRawPointer(pDestPos, destPosStride);
            advanceRawPointer(pBlendWeight, blendWeightStride);
            advanceRawPointer(pBlendIndex, blendIndexStride);
        }
    }
    //---------------------------------------------------------------------
    void OptimisedUtilGeneral::concatenateAffineMatrices(
        const Matrix4& baseMatrix,
        const Matrix4* pSrcMat,
        Matrix4* pDstMat,
        size_t numMatrices)
    {
        const Matrix4& m = baseMatrix;

        for (size_t i = 0; i < numMatrices; ++i)
        {
            const Matrix4& s = *pSrcMat;
            Matrix4& d = *pDstMat;

            // TODO: Promote following code to Matrix4 class.

            d[0][0] = m[0][0] * s[0][0] + m[0][1] * s[1][0] + m[0][2] * s[2][0];
            d[0][1] = m[0][0] * s[0][1] + m[0][1] * s[1][1] + m[0][2] * s[2][1];
            d[0][2] = m[0][0] * s[0][2] + m[0][1] * s[1][2] + m[0][2] * s[2][2];
            d[0][3] = m[0][0] * s[0][3] + m[0][1] * s[1][3] + m[0][2] * s[2][3] + m[0][3];

            d[1][0] = m[1][0] * s[0][0] + m[1][1] * s[1][0] + m[1][2] * s[2][0];
            d[1][1] = m[1][0] * s[0][1] + m[1][1] * s[1][1] + m[1][2] * s[2][1];
            d[1][2] = m[1][0] * s[0][2] + m[1][1] * s[1][2] + m[1][2] * s[2][2];
            d[1][3] = m[1][0] * s[0][3] + m[1][1] * s[1][3] + m[1][2] * s[2][3] + m[1][3];

            d[2][0] = m[2][0] * s[0][0] + m[2][1] * s[1][0] + m[2][2] * s[2][0];
            d[2][1] = m[2][0] * s[0][1] + m[2][1] * s[1][1] + m[2][2] * s[2][1];
            d[2][2] = m[2][0] * s[0][2] + m[2][1] * s[1][2] + m[2][2] * s[2][2];
            d[2][3] = m[2][0] * s[0][3] + m[2][1] * s[1][3] + m[2][2] * s[2][3] + m[2][3];

            d[3][0] = 0;
            d[3][1] = 0;
            d[3][2] = 0;
            d[3][3] = 1;

            ++pSrcMat;
            ++pDstMat;
        }
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    extern OptimisedUtil* _getOptimisedUtilGeneral(void)
    {
        static OptimisedUtilGeneral msOptimisedUtilGeneral;
        return &msOptimisedUtilGeneral;
    }

}

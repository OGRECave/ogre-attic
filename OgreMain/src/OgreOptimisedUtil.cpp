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

#include "OgrePlatformInformation.h"
#include "OgreSIMDHelper.h"
#include "OgreVector3.h"
#include "OgreMatrix4.h"

//#define __DO_PROFILE__
#ifdef __DO_PROFILE__
#include "OgreRoot.h"
#endif

//-------------------------------------------------------------------------
// Ok, I known, this file looks very ugly.
//
// The routines implemented in this file are performance oriented,
// which means saving every penny as possible. This requirement might
// break some C++/STL-rules.
//
//
// Some rules I'd like to respects:
//
// 1. Had better use unpacklo/hi, movelh/hl instead of shuffle because
//    it can saving one byte of binary code :)
// 2. Use add/sub instead of mul.
// 3. Eliminate prolog code of function call.
//
// The last, anything recommended by Intel Optimization Reference Manual.
//
//-------------------------------------------------------------------------

// Use unrolled SSE version when vertices exceeds this limit
#define OGRE_SSE_SKINNING_UNROLL_VERTICES  16

namespace Ogre {

    //---------------------------------------------------------------------
    // Helper to dealing with strided pointers
    //---------------------------------------------------------------------
    template <class T>
    static FORCEINLINE T* rawOffsetPointer(T* ptr, ptrdiff_t offset)
    {
        return (T*)((char*)(ptr) + offset);
    }
    template <class T>
    static FORCEINLINE void advanceRawPointer(T*& ptr, ptrdiff_t stride)
    {
        ptr = rawOffsetPointer(ptr, stride);
    }

    //---------------------------------------------------------------------
    // Software vertex skinning functions
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    // Forward declarations
    typedef void SoftwareVertexSkinningFunc(
        const float *pSrcPos, float *pDestPos,
        const float *pSrcNorm, float *pDestNorm,
        const float *pBlendWeight, const unsigned char* pBlendIndex,
        const Matrix4* const* blendMatrices,
        size_t srcPosStride, size_t destPosStride,
        size_t srcNormStride, size_t destNormStride,
        size_t blendWeightStride, size_t blendIndexStride,
        size_t numWeightsPerVertex,
        size_t numVertices);
    static SoftwareVertexSkinningFunc softwareVertexSkinning_Bootstrap;
    static SoftwareVertexSkinningFunc softwareVertexSkinning_General;
#if OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86 && OGRE_COMPILER == OGRE_COMPILER_MSVC
    static SoftwareVertexSkinningFunc softwareVertexSkinning_SSE_General;
    static SoftwareVertexSkinningFunc softwareVertexSkinning_SSE_Dispatch;
    static SoftwareVertexSkinningFunc softwareVertexSkinning_SSE_AMD;
#endif  // OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86 && OGRE_COMPILER == OGRE_COMPILER_MSVC

    //---------------------------------------------------------------------
    // Pointer to software skinning implementation, initialised to 'boot-strap',
    // and will update to really implementation in first time call to the function.
    // Use function pointer here because it can eliminate a bit of prolog code
    // of function call.
    static SoftwareVertexSkinningFunc* msSoftwareVertexSkinningFuncPtr =
        &softwareVertexSkinning_Bootstrap;
    //---------------------------------------------------------------------
    void OptimisedUtil::softwareVertexSkinning(
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
        (*msSoftwareVertexSkinningFuncPtr)(
            pSrcPos, pDestPos,
            pSrcNorm, pDestNorm,
            pBlendWeight, pBlendIndex,
            blendMatrices,
            srcPosStride, destPosStride,
            srcNormStride, destNormStride,
            blendWeightStride, blendIndexStride,
            numWeightsPerVertex,
            numVertices);
    }
    //---------------------------------------------------------------------
#ifdef __DO_PROFILE__
    typedef unsigned __int64 uint64;
#pragma warning(push)
#pragma warning(disable: 4035)  // no return value
    static __forceinline uint64 getCpuTimestamp(void)
    {
        __asm rdtsc
        // Return values in edx:eax, No return statment requirement here for VC.
    }
#pragma warning(pop)
    //---------------------------------------------------------------------
    static void softwareVertexSkinning_Profile(
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
        size_t t = Root::getSingleton().getCurrentFrameNumber() % 3;

        SoftwareVertexSkinningFunc* func;
        switch (t)
        {
        default:
        case 0:
            func = &softwareVertexSkinning_General;
            break;

        case 1:
            if (numVertices > OGRE_SSE_SKINNING_UNROLL_VERTICES)
            {
                func = &softwareVertexSkinning_SSE_Dispatch;
                break;
            }
            ++t;
            // Fall-through

        case 2:
            func = &softwareVertexSkinning_SSE_General;
            break;
        }

        uint64 start = getCpuTimestamp();

        (*func)(
            pSrcPos, pDestPos,
            pSrcNorm, pDestNorm,
            pBlendWeight, pBlendIndex,
            blendMatrices,
            srcPosStride, destPosStride,
            srcNormStride, destNormStride,
            blendWeightStride, blendIndexStride,
            numWeightsPerVertex,
            numVertices);

        uint64 ticks = getCpuTimestamp() - start;

        static uint64 timestamps[3] = { 0 };
        static uint counts[3] = { 0 };
        static uint avgTimeStamps[3];
        // You can put break point here while running test application, to
        // watch profile results.
        timestamps[t] += ticks;
        ++counts[t];
        avgTimeStamps[t] = timestamps[t] / counts[t];
    }
#endif  // __DO_PROFILE__
    //---------------------------------------------------------------------
    // Detect best software vertex skinning implementation and perform
    // really skinning, used as 'boot-starp' of software vertex skinning.
    static void softwareVertexSkinning_Bootstrap(
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
        // Detect best implementation

        msSoftwareVertexSkinningFuncPtr = &softwareVertexSkinning_General;

#if OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86 && OGRE_COMPILER == OGRE_COMPILER_MSVC
        //
        // Some speed test results (averaged number of CPU timestamp (RDTSC) per-function call):
        //
        //   Dagon SkeletonAnimation sample:
        //
        //                                      Pentium 4 3.0G HT       Althon XP 2500+
        //
        //      Shared Buffers, General C       763677                  462903
        //      Shared Buffers, Unrolled SSE    210030 *best*           369762
        //      Shared Buffers, General SSE     286202                  352412 *best*
        //
        //      Separated Buffers, General C    762640                  464840
        //      Separated Buffers, Unrolled SSE 219222 *best*           287992 *best*
        //      Separated Buffers, General SSE  290129                  341614
        //
        //      PosOnly, General C              388663                  257350
        //      PosOnly, Unrolled SSE           139814 *best*           200323 *best*
        //      PosOnly, General SSE            172693                  213704
        //
        //   Another my own test scene:
        //
        //                                      Pentium P4 3.0G HT      Althon XP 2500+
        //
        //      Shared Buffers, General C       74527                   -
        //      Shared Buffers, Unrolled SSE    22743 *best*            -
        //      Shared Buffers, General SSE     28527                   -
        //
        //
        // Note that speed test appears unaligned load/store instruction version
        // loss performance 5%-10% than aligned load/store version, even if both
        // of them access to aligned data. Thus, we should use aligned load/store
        // as soon as possible.
        //
        //
        // We are pick up the implementation based on test results above.
        //

        if (PlatformInformation::getCpuFeatures() & PlatformInformation::CPU_FEATURE_SSE)
        {
            msSoftwareVertexSkinningFuncPtr = &softwareVertexSkinning_SSE_Dispatch;

            if (PlatformInformation::getCpuIdentifier() == "AuthenticAMD-X86")
            {
                // Use special designed version for AMD.
                //
                // For AMD Athlon XP, it's prefer to never use unrolled version for shared
                // buffers at all, I guess because that version run out of CPU register count,
                // or L1 cache related problem, causing slight performance loss than general
                // version.
                //
                msSoftwareVertexSkinningFuncPtr = &softwareVertexSkinning_SSE_AMD;
            }

#ifdef __DO_PROFILE__
            msSoftwareVertexSkinningFuncPtr = &softwareVertexSkinning_Profile;
#endif
        }
#endif  // OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86 && OGRE_COMPILER == OGRE_COMPILER_MSVC

        // Call to detected function to performs actual calculation.

        (*msSoftwareVertexSkinningFuncPtr)(
            pSrcPos, pDestPos,
            pSrcNorm, pDestNorm,
            pBlendWeight, pBlendIndex,
            blendMatrices,
            srcPosStride, destPosStride,
            srcNormStride, destNormStride,
            blendWeightStride, blendIndexStride,
            numWeightsPerVertex,
            numVertices);
    }
    //---------------------------------------------------------------------
    // General C/C++ version skinning positions, and optional skinning normals.
    static void softwareVertexSkinning_General(
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
#if OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86 && OGRE_COMPILER == OGRE_COMPILER_MSVC
    //---------------------------------------------------------------------
    // Collapse a matrix at one time. The collapsed matrix are weighted
    // by blend-weights, and then can use to transform according vertex directly. 
    static FORCEINLINE void _collapseOneMatrix(
        __m128 &m00, __m128 &m01, __m128 &m02,
        const float* pBlendWeight, const unsigned char* pBlendIndex,
        const Matrix4* const* blendMatrices,
        size_t blendWeightStride, size_t blendIndexStride,
        size_t numWeightsPerVertex)
    {
        // Important Note:
        //   If reuse pMatrixXXX frequently, M$ VC7.1 will generate wrong code here!!!
        const Matrix4* pMatrix0, *pMatrix1, *pMatrix2, *pMatrix3;
        __m128 weight, weights;

        // Thank row-major matrix used in Ogre, it make we accessing affine matrix easy.
        switch (numWeightsPerVertex)
        {
        default:    // Just in case and make compiler happy
        case 1:
            // Eliminated multiply by weight since the weight should be equal to one always
            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[0]];
            m00 = __MM_LOAD_PS((*pMatrix0)[0]);
            m01 = __MM_LOAD_PS((*pMatrix0)[1]);
            m02 = __MM_LOAD_PS((*pMatrix0)[2]);
            break;

        case 2:
            // Based on the fact that accumulated weights are equal to one, by use lerp, replaced
            // two multiplies and one additive with one multiplie and two additives.
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 0*blendWeightStride) + 1);
            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[0]];
            m00 = __MM_LOAD_PS((*pMatrix0)[0]);
            m01 = __MM_LOAD_PS((*pMatrix0)[1]);
            m02 = __MM_LOAD_PS((*pMatrix0)[2]);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[1]];
            m00 = __MM_LERP2_PS(weight, m00, __MM_LOAD_PS((*pMatrix1)[0]));
            m01 = __MM_LERP2_PS(weight, m01, __MM_LOAD_PS((*pMatrix1)[1]));
            m02 = __MM_LERP2_PS(weight, m02, __MM_LOAD_PS((*pMatrix1)[2]));
            break;

        case 3:
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 0*blendWeightStride) + 0);
            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[0]];
            m00 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[0]), weight);
            m01 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[1]), weight);
            m02 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[2]), weight);
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 0*blendWeightStride) + 1);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[1]];
            m00 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[0]), weight, m00);
            m01 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[1]), weight, m01);
            m02 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[2]), weight, m02);
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 0*blendWeightStride) + 2);
            pMatrix2 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[2]];
            m00 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[0]), weight, m00);
            m01 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[1]), weight, m01);
            m02 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[2]), weight, m02);
            break;

        case 4:
            // Load four blend weights at one time, they will be shuffled later
            weights = _mm_loadu_ps(rawOffsetPointer(pBlendWeight, 0*blendWeightStride));

            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[0]];
            weight = _mm_shuffle_ps(weights, weights, 0x00);
            m00 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[0]), weight);
            m01 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[1]), weight);
            m02 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[2]), weight);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[1]];
            weight = _mm_shuffle_ps(weights, weights, 0x55);
            m00 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[0]), weight, m00);
            m01 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[1]), weight, m01);
            m02 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[2]), weight, m02);
            pMatrix2 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[2]];
            weight = _mm_shuffle_ps(weights, weights, 0xAA);
            m00 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[0]), weight, m00);
            m01 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[1]), weight, m01);
            m02 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[2]), weight, m02);
            pMatrix3 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[3]];
            weight = _mm_shuffle_ps(weights, weights, 0xFF);
            m00 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[0]), weight, m00);
            m01 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[1]), weight, m01);
            m02 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[2]), weight, m02);
            break;
        }
    }
    //---------------------------------------------------------------------
    // Collapse four matrices at one time. The collapsed matrix are weighted
    // by blend-weights, and then can use to transform according vertex directly. 
    static FORCEINLINE void _collapseFourMatrices(
        __m128 &m00, __m128 &m01, __m128 &m02,
        __m128 &m10, __m128 &m11, __m128 &m12,
        __m128 &m20, __m128 &m21, __m128 &m22,
        __m128 &m30, __m128 &m31, __m128 &m32,
        const float* pBlendWeight, const unsigned char* pBlendIndex,
        const Matrix4* const* blendMatrices,
        size_t blendWeightStride, size_t blendIndexStride,
        size_t numWeightsPerVertex)
    {
        // Important Note:
        //   If reuse pMatrixXXX frequently, M$ VC7.1 will generate wrong code here!!!
        const Matrix4* pMatrix0, *pMatrix1, *pMatrix2, *pMatrix3;
        __m128 weight, weights;

        // Thank row-major matrix used in Ogre, it make we accessing affine matrix easy.
        switch (numWeightsPerVertex)
        {
        default:    // Just in case and make compiler happy
        case 1:
            // Eliminated multiply by weight since the weight should be equal to one always
            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[0]];
            m00 = __MM_LOAD_PS((*pMatrix0)[0]);
            m01 = __MM_LOAD_PS((*pMatrix0)[1]);
            m02 = __MM_LOAD_PS((*pMatrix0)[2]);

            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 1*blendIndexStride)[0]];
            m10 = __MM_LOAD_PS((*pMatrix1)[0]);
            m11 = __MM_LOAD_PS((*pMatrix1)[1]);
            m12 = __MM_LOAD_PS((*pMatrix1)[2]);

            pMatrix2 = blendMatrices[rawOffsetPointer(pBlendIndex, 2*blendIndexStride)[0]];
            m20 = __MM_LOAD_PS((*pMatrix2)[0]);
            m21 = __MM_LOAD_PS((*pMatrix2)[1]);
            m22 = __MM_LOAD_PS((*pMatrix2)[2]);

            pMatrix3 = blendMatrices[rawOffsetPointer(pBlendIndex, 3*blendIndexStride)[0]];
            m30 = __MM_LOAD_PS((*pMatrix3)[0]);
            m31 = __MM_LOAD_PS((*pMatrix3)[1]);
            m32 = __MM_LOAD_PS((*pMatrix3)[2]);
            break;

        case 2:
            // Based on the fact that accumulated weights are equal to one, by use lerp, replaced
            // two multiplies and one additive with one multiplie and two additives.
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 0*blendWeightStride) + 1);
            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[0]];
            m00 = __MM_LOAD_PS((*pMatrix0)[0]);
            m01 = __MM_LOAD_PS((*pMatrix0)[1]);
            m02 = __MM_LOAD_PS((*pMatrix0)[2]);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[1]];
            m00 = __MM_LERP_PS(weight, m00, __MM_LOAD_PS((*pMatrix1)[0]));
            m01 = __MM_LERP_PS(weight, m01, __MM_LOAD_PS((*pMatrix1)[1]));
            m02 = __MM_LERP_PS(weight, m02, __MM_LOAD_PS((*pMatrix1)[2]));

            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 1*blendWeightStride) + 1);
            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 1*blendIndexStride)[0]];
            m10 = __MM_LOAD_PS((*pMatrix0)[0]);
            m11 = __MM_LOAD_PS((*pMatrix0)[1]);
            m12 = __MM_LOAD_PS((*pMatrix0)[2]);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 1*blendIndexStride)[1]];
            m10 = __MM_LERP_PS(weight, m10, __MM_LOAD_PS((*pMatrix1)[0]));
            m11 = __MM_LERP_PS(weight, m11, __MM_LOAD_PS((*pMatrix1)[1]));
            m12 = __MM_LERP_PS(weight, m12, __MM_LOAD_PS((*pMatrix1)[2]));

            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 2*blendWeightStride) + 1);
            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 2*blendIndexStride)[0]];
            m20 = __MM_LOAD_PS((*pMatrix0)[0]);
            m21 = __MM_LOAD_PS((*pMatrix0)[1]);
            m22 = __MM_LOAD_PS((*pMatrix0)[2]);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 2*blendIndexStride)[1]];
            m20 = __MM_LERP_PS(weight, m20, __MM_LOAD_PS((*pMatrix1)[0]));
            m21 = __MM_LERP_PS(weight, m21, __MM_LOAD_PS((*pMatrix1)[1]));
            m22 = __MM_LERP_PS(weight, m22, __MM_LOAD_PS((*pMatrix1)[2]));

            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 3*blendWeightStride) + 1);
            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 3*blendIndexStride)[0]];
            m30 = __MM_LOAD_PS((*pMatrix0)[0]);
            m31 = __MM_LOAD_PS((*pMatrix0)[1]);
            m32 = __MM_LOAD_PS((*pMatrix0)[2]);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 3*blendIndexStride)[1]];
            m30 = __MM_LERP_PS(weight, m30, __MM_LOAD_PS((*pMatrix1)[0]));
            m31 = __MM_LERP_PS(weight, m31, __MM_LOAD_PS((*pMatrix1)[1]));
            m32 = __MM_LERP_PS(weight, m32, __MM_LOAD_PS((*pMatrix1)[2]));
            break;

        case 3:
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 0*blendWeightStride) + 0);
            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[0]];
            m00 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[0]), weight);
            m01 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[1]), weight);
            m02 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[2]), weight);
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 0*blendWeightStride) + 1);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[1]];
            m00 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[0]), weight, m00);
            m01 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[1]), weight, m01);
            m02 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[2]), weight, m02);
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 0*blendWeightStride) + 2);
            pMatrix2 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[2]];
            m00 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[0]), weight, m00);
            m01 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[1]), weight, m01);
            m02 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[2]), weight, m02);

            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 1*blendWeightStride) + 0);
            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 1*blendIndexStride)[0]];
            m10 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[0]), weight);
            m11 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[1]), weight);
            m12 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[2]), weight);
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 1*blendWeightStride) + 1);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 1*blendIndexStride)[1]];
            m10 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[0]), weight, m10);
            m11 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[1]), weight, m11);
            m12 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[2]), weight, m12);
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 1*blendWeightStride) + 2);
            pMatrix2 = blendMatrices[rawOffsetPointer(pBlendIndex, 1*blendIndexStride)[2]];
            m10 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[0]), weight, m10);
            m11 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[1]), weight, m11);
            m12 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[2]), weight, m12);

            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 2*blendWeightStride) + 0);
            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 2*blendIndexStride)[0]];
            m20 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[0]), weight);
            m21 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[1]), weight);
            m22 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[2]), weight);
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 2*blendWeightStride) + 1);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 2*blendIndexStride)[1]];
            m20 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[0]), weight, m20);
            m21 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[1]), weight, m21);
            m22 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[2]), weight, m22);
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 2*blendWeightStride) + 2);
            pMatrix2 = blendMatrices[rawOffsetPointer(pBlendIndex, 2*blendIndexStride)[2]];
            m20 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[0]), weight, m20);
            m21 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[1]), weight, m21);
            m22 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[2]), weight, m22);

            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 3*blendWeightStride) + 0);
            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 3*blendIndexStride)[0]];
            m30 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[0]), weight);
            m31 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[1]), weight);
            m32 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[2]), weight);
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 3*blendWeightStride) + 1);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 3*blendIndexStride)[1]];
            m30 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[0]), weight, m30);
            m31 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[1]), weight, m31);
            m32 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[2]), weight, m32);
            weight = _mm_load_ps1(rawOffsetPointer(pBlendWeight, 3*blendWeightStride) + 2);
            pMatrix2 = blendMatrices[rawOffsetPointer(pBlendIndex, 3*blendIndexStride)[2]];
            m30 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[0]), weight, m30);
            m31 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[1]), weight, m31);
            m32 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[2]), weight, m32);
            break;

        case 4:
            // Load four blend weights at one time, they will be shuffled later
            weights = _mm_loadu_ps(rawOffsetPointer(pBlendWeight, 0*blendWeightStride));

            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[0]];
            weight = _mm_shuffle_ps(weights, weights, 0x00);
            m00 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[0]), weight);
            m01 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[1]), weight);
            m02 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[2]), weight);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[1]];
            weight = _mm_shuffle_ps(weights, weights, 0x55);
            m00 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[0]), weight, m00);
            m01 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[1]), weight, m01);
            m02 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[2]), weight, m02);
            pMatrix2 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[2]];
            weight = _mm_shuffle_ps(weights, weights, 0xAA);
            m00 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[0]), weight, m00);
            m01 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[1]), weight, m01);
            m02 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[2]), weight, m02);
            pMatrix3 = blendMatrices[rawOffsetPointer(pBlendIndex, 0*blendIndexStride)[3]];
            weight = _mm_shuffle_ps(weights, weights, 0xFF);
            m00 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[0]), weight, m00);
            m01 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[1]), weight, m01);
            m02 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[2]), weight, m02);

            // Load four blend weights at one time, they will be shuffled later
            weights = _mm_loadu_ps(rawOffsetPointer(pBlendWeight, 1*blendWeightStride));

            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 1*blendIndexStride)[0]];
            weight = _mm_shuffle_ps(weights, weights, 0x00);
            m10 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[0]), weight);
            m11 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[1]), weight);
            m12 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[2]), weight);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 1*blendIndexStride)[1]];
            weight = _mm_shuffle_ps(weights, weights, 0x55);
            m10 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[0]), weight, m10);
            m11 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[1]), weight, m11);
            m12 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[2]), weight, m12);
            pMatrix2 = blendMatrices[rawOffsetPointer(pBlendIndex, 1*blendIndexStride)[2]];
            weight = _mm_shuffle_ps(weights, weights, 0xAA);
            m10 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[0]), weight, m10);
            m11 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[1]), weight, m11);
            m12 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[2]), weight, m12);
            pMatrix3 = blendMatrices[rawOffsetPointer(pBlendIndex, 1*blendIndexStride)[3]];
            weight = _mm_shuffle_ps(weights, weights, 0xFF);
            m10 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[0]), weight, m10);
            m11 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[1]), weight, m11);
            m12 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[2]), weight, m12);

            // Load four blend weights at one time, they will be shuffled later
            weights = _mm_loadu_ps(rawOffsetPointer(pBlendWeight, 2*blendWeightStride));

            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 2*blendIndexStride)[0]];
            weight = _mm_shuffle_ps(weights, weights, 0x00);
            m20 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[0]), weight);
            m21 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[1]), weight);
            m22 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[2]), weight);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 2*blendIndexStride)[1]];
            weight = _mm_shuffle_ps(weights, weights, 0x55);
            m20 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[0]), weight, m20);
            m21 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[1]), weight, m21);
            m22 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[2]), weight, m22);
            pMatrix2 = blendMatrices[rawOffsetPointer(pBlendIndex, 2*blendIndexStride)[2]];
            weight = _mm_shuffle_ps(weights, weights, 0xAA);
            m20 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[0]), weight, m20);
            m21 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[1]), weight, m21);
            m22 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[2]), weight, m22);
            pMatrix3 = blendMatrices[rawOffsetPointer(pBlendIndex, 2*blendIndexStride)[3]];
            weight = _mm_shuffle_ps(weights, weights, 0xFF);
            m20 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[0]), weight, m20);
            m21 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[1]), weight, m21);
            m22 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[2]), weight, m22);

            // Load four blend weights at one time, they will be shuffled later
            weights = _mm_loadu_ps(rawOffsetPointer(pBlendWeight, 3*blendWeightStride));

            pMatrix0 = blendMatrices[rawOffsetPointer(pBlendIndex, 3*blendIndexStride)[0]];
            weight = _mm_shuffle_ps(weights, weights, 0x00);
            m30 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[0]), weight);
            m31 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[1]), weight);
            m32 = _mm_mul_ps(__MM_LOAD_PS((*pMatrix0)[2]), weight);
            pMatrix1 = blendMatrices[rawOffsetPointer(pBlendIndex, 3*blendIndexStride)[1]];
            weight = _mm_shuffle_ps(weights, weights, 0x55);
            m30 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[0]), weight, m30);
            m31 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[1]), weight, m31);
            m32 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix1)[2]), weight, m32);
            pMatrix2 = blendMatrices[rawOffsetPointer(pBlendIndex, 3*blendIndexStride)[2]];
            weight = _mm_shuffle_ps(weights, weights, 0xAA);
            m30 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[0]), weight, m30);
            m31 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[1]), weight, m31);
            m32 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix2)[2]), weight, m32);
            pMatrix3 = blendMatrices[rawOffsetPointer(pBlendIndex, 3*blendIndexStride)[3]];
            weight = _mm_shuffle_ps(weights, weights, 0xFF);
            m30 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[0]), weight, m30);
            m31 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[1]), weight, m31);
            m32 = __MM_MADD_PS(__MM_LOAD_PS((*pMatrix3)[2]), weight, m32);
            break;
        }
    }
    //---------------------------------------------------------------------
    // General SSE version skinning positions, and optional skinning normals.
    static void softwareVertexSkinning_SSE_General(
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
        for (size_t i = 0; i < numVertices; ++i)
        {
            // Collapse matrices
            __m128 m00, m01, m02;
            _collapseOneMatrix(
                m00, m01, m02,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex);

            // Advance blend weight and index pointers
            advanceRawPointer(pBlendWeight, blendWeightStride);
            advanceRawPointer(pBlendIndex, blendIndexStride);

            //------------------------------------------------------------------

            // Rearrange to column-major matrix with rows shuffled order to: Z 0 X Y
            __m128 m03 = _mm_setzero_ps();
            __MM_TRANSPOSE4x4_PS(m02, m03, m00, m01);

            //------------------------------------------------------------------
            // Transform position
            //------------------------------------------------------------------

            __m128 s0, s1, s2;

            // Load source position
            s0 = _mm_load_ps1(pSrcPos + 0);
            s1 = _mm_load_ps1(pSrcPos + 1);
            s2 = _mm_load_ps1(pSrcPos + 2);

            // Transform by collapsed matrix
            __m128 accumPos = __MM_DOT4x3_PS(m02, m03, m00, m01, s0, s1, s2);   // z 0 x y

            // Store blended position, no aligned requirement
            _mm_storeh_pi((__m64*)pDestPos, accumPos);
            _mm_store_ss(pDestPos+2, accumPos);

            // Advance source and target position pointers
            advanceRawPointer(pSrcPos, srcPosStride);
            advanceRawPointer(pDestPos, destPosStride);

            //------------------------------------------------------------------
            // Optional blend normal
            //------------------------------------------------------------------

            if (pSrcNorm)
            {
                // Load source normal
                s0 = _mm_load_ps1(pSrcNorm + 0);
                s1 = _mm_load_ps1(pSrcNorm + 1);
                s2 = _mm_load_ps1(pSrcNorm + 2);

                // Transform by collapsed matrix
                __m128 accumNorm = __MM_DOT3x3_PS(m02, m03, m00, s0, s1, s2);   // z 0 x y

                // Normalise normal
                __m128 tmp = _mm_mul_ps(accumNorm, accumNorm);                  // z^2 0 x^2 y^2
                tmp = __MM_ACCUM3_PS(tmp,
                        _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(0,3,1,2)),         // x^2 0 y^2 z^2
                        _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(2,0,1,3)));        // y^2 0 z^2 x^2
                // Note: zero divided here, but neglectable
                tmp = __MM_RSQRT_PS(tmp);
                accumNorm = _mm_mul_ps(accumNorm, tmp);

                // Store blended normal, no aligned requirement
                _mm_storeh_pi((__m64*)pDestNorm, accumNorm);
                _mm_store_ss(pDestNorm+2, accumNorm);

                // Advance source and target normal pointers
                advanceRawPointer(pSrcNorm, srcNormStride);
                advanceRawPointer(pDestNorm, destNormStride);
            }
        }
    }
    //---------------------------------------------------------------------
    // Special SSE version skinning shared buffers of position and normal,
    // and the buffer are packed.
    template <bool srcAligned, bool destAligned>
    struct SoftwareVertexSkinning_SSE_PosNorm_Shared_Packed
    {
        static void apply(
            const float* pSrc, float* pDest,
            const float* pBlendWeight, const unsigned char* pBlendIndex,
            const Matrix4* const* blendMatrices,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numIterations)
        {
            typedef SSEMemoryAccessor<srcAligned> SrcAccessor;
            typedef SSEMemoryAccessor<destAligned> DestAccessor;

            // Blending 4 vertices per-iteration
            for (size_t i = 0; i < numIterations; ++i)
            {
                // Collapse matrices
                __m128 m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32;
                _collapseFourMatrices(
                    m00, m01, m02,
                    m10, m11, m12,
                    m20, m21, m22,
                    m30, m31, m32,
                    pBlendWeight, pBlendIndex,
                    blendMatrices,
                    blendWeightStride, blendIndexStride,
                    numWeightsPerVertex);

                // Advance 4 vertices
                advanceRawPointer(pBlendWeight, 4 * blendWeightStride);
                advanceRawPointer(pBlendIndex, 4 * blendIndexStride);

                //------------------------------------------------------------------
                // Transform position/normals
                //------------------------------------------------------------------

                __m128 s0, s1, s2, s3, s4, s5, d0, d1, d2, d3, d4, d5;
                __m128 t0, t1, t2, t3, t4, t5;

                // Load source position/normals
                s0 = SrcAccessor::load(pSrc + 0);                       // px0 py0 pz0 nx0
                s1 = SrcAccessor::load(pSrc + 4);                       // ny0 nz0 px1 py1
                s2 = SrcAccessor::load(pSrc + 8);                       // pz1 nx1 ny1 nz1
                s3 = SrcAccessor::load(pSrc + 12);                      // px2 py2 pz2 nx2
                s4 = SrcAccessor::load(pSrc + 16);                      // ny2 nz2 px3 py3
                s5 = SrcAccessor::load(pSrc + 20);                      // pz3 nx3 ny3 nz3

                // Rearrange to component-major for batches calculate.

                t0 = _mm_unpacklo_ps(s0, s3);                           // px0 px2 py0 py2
                t1 = _mm_unpackhi_ps(s0, s3);                           // pz0 pz2 nx0 nx2
                t2 = _mm_unpacklo_ps(s1, s4);                           // ny0 ny2 nz0 nz2
                t3 = _mm_unpackhi_ps(s1, s4);                           // px1 px3 py1 py3
                t4 = _mm_unpacklo_ps(s2, s5);                           // pz1 pz3 nx1 nx3
                t5 = _mm_unpackhi_ps(s2, s5);                           // ny1 ny3 nz1 nz3

                s0 = _mm_unpacklo_ps(t0, t3);                           // px0 px1 px2 px3
                s1 = _mm_unpackhi_ps(t0, t3);                           // py0 py1 py2 py3
                s2 = _mm_unpacklo_ps(t1, t4);                           // pz0 pz1 pz2 pz3
                s3 = _mm_unpackhi_ps(t1, t4);                           // nx0 nx1 nx2 nx3
                s4 = _mm_unpacklo_ps(t2, t5);                           // ny0 ny1 ny2 ny3
                s5 = _mm_unpackhi_ps(t2, t5);                           // nz0 nz1 nz2 nz3

                // Transform by collapsed matrix

                // Shuffle row 0 of four collapsed matrices for calculate X component
                __MM_TRANSPOSE4x4_PS(m00, m10, m20, m30);

                // Transform X components
                d0 = __MM_DOT4x3_PS(m00, m10, m20, m30, s0, s1, s2);    // PX0 PX1 PX2 PX3
                d3 = __MM_DOT3x3_PS(m00, m10, m20, s3, s4, s5);         // NX0 NX1 NX2 NX3

                // Shuffle row 1 of four collapsed matrices for calculate Y component
                __MM_TRANSPOSE4x4_PS(m01, m11, m21, m31);

                // Transform Y components
                d1 = __MM_DOT4x3_PS(m01, m11, m21, m31, s0, s1, s2);    // PY0 PY1 PY2 PY3
                d4 = __MM_DOT3x3_PS(m01, m11, m21, s3, s4, s5);         // NY0 NY1 NY2 NY3

                // Shuffle row 2 of four collapsed matrices for calculate Z component
                __MM_TRANSPOSE4x4_PS(m02, m12, m22, m32);

                // Transform Z components
                d2 = __MM_DOT4x3_PS(m02, m12, m22, m32, s0, s1, s2);    // PZ0 PZ1 PZ2 PZ3
                d5 = __MM_DOT3x3_PS(m02, m12, m22, s3, s4, s5);         // NZ0 NZ1 NZ2 NZ3

                // Normalise normals
                __m128 tmp = __MM_DOT3x3_PS(d3, d4, d5, d3, d4, d5);
                tmp = __MM_RSQRT_PS(tmp);
                d3 = _mm_mul_ps(d3, tmp);
                d4 = _mm_mul_ps(d4, tmp);
                d5 = _mm_mul_ps(d5, tmp);

                // Arrange back to continuous format for store results

                t0 = _mm_unpacklo_ps(d0, d1);                           // PX0 PY0 PX1 PY1
                t1 = _mm_unpackhi_ps(d0, d1);                           // PX2 PY2 PX3 PY3
                t2 = _mm_unpacklo_ps(d2, d3);                           // PZ0 NX0 PZ1 NX1
                t3 = _mm_unpackhi_ps(d2, d3);                           // PZ2 NX2 PZ3 NX3
                t4 = _mm_unpacklo_ps(d4, d5);                           // NY0 NZ0 NY1 NZ1
                t5 = _mm_unpackhi_ps(d4, d5);                           // NY2 NZ2 NY3 NZ3

                d0 = _mm_movelh_ps(t0, t2);                             // PX0 PY0 PZ0 NX0
                d1 = _mm_shuffle_ps(t4, t0, _MM_SHUFFLE(3,2,1,0));      // NY0 NZ0 PX1 PY1
                d2 = _mm_movehl_ps(t4, t2);                             // PZ1 NX1 NY1 NZ1
                d3 = _mm_movelh_ps(t1, t3);                             // PX2 PY2 PZ2 NX2
                d4 = _mm_shuffle_ps(t5, t1, _MM_SHUFFLE(3,2,1,0));      // NY2 NZ2 PX3 PY3
                d5 = _mm_movehl_ps(t5, t3);                             // PZ3 NX3 NY3 NZ3

                // Store blended position/normals
                DestAccessor::store(pDest + 0, d0);
                DestAccessor::store(pDest + 4, d1);
                DestAccessor::store(pDest + 8, d2);
                DestAccessor::store(pDest + 12, d3);
                DestAccessor::store(pDest + 16, d4);
                DestAccessor::store(pDest + 20, d5);

                // Advance 4 vertices
                pSrc += 4 * (3 + 3);
                pDest += 4 * (3 + 3);
            }
        }
    };
    static FORCEINLINE void softwareVertexSkinning_SSE_PosNorm_Shared_Packed(
            const float* pSrcPos, float* pDestPos,
            const float* pBlendWeight, const unsigned char* pBlendIndex,
            const Matrix4* const* blendMatrices,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numIterations)
    {
        // pSrcPos might can't align to 16 bytes because 8 bytes alignment shift per-vertex

        // Instantiating two version only, since other alignement combination not that important.
        if (_isAlignedForSSE(pSrcPos) && _isAlignedForSSE(pDestPos))
        {
            SoftwareVertexSkinning_SSE_PosNorm_Shared_Packed<true, true>::apply(
                pSrcPos, pDestPos,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
        else
        {
            SoftwareVertexSkinning_SSE_PosNorm_Shared_Packed<false, false>::apply(
                pSrcPos, pDestPos,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
    }
    //---------------------------------------------------------------------
    // Special SSE version skinning separated buffers of position and normal,
    // both of position and normal buffer are packed.
    template <bool srcPosAligned, bool destPosAligned, bool srcNormAligned, bool destNormAligned>
    struct SoftwareVertexSkinning_SSE_PosNorm_Separated_Packed
    {
        static void apply(
            const float* pSrcPos, float* pDestPos,
            const float* pSrcNorm, float* pDestNorm,
            const float* pBlendWeight, const unsigned char* pBlendIndex,
            const Matrix4* const* blendMatrices,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numIterations)
        {
            typedef SSEMemoryAccessor<srcPosAligned> SrcPosAccessor;
            typedef SSEMemoryAccessor<destPosAligned> DestPosAccessor;
            typedef SSEMemoryAccessor<srcNormAligned> SrcNormAccessor;
            typedef SSEMemoryAccessor<destNormAligned> DestNormAccessor;

            // Blending 4 vertices per-iteration
            for (size_t i = 0; i < numIterations; ++i)
            {
                // Collapse matrices
                __m128 m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32;
                _collapseFourMatrices(
                    m00, m01, m02,
                    m10, m11, m12,
                    m20, m21, m22,
                    m30, m31, m32,
                    pBlendWeight, pBlendIndex,
                    blendMatrices,
                    blendWeightStride, blendIndexStride,
                    numWeightsPerVertex);

                // Advance 4 vertices
                advanceRawPointer(pBlendWeight, 4 * blendWeightStride);
                advanceRawPointer(pBlendIndex, 4 * blendIndexStride);

                //------------------------------------------------------------------
                // Transform positions
                //------------------------------------------------------------------

                __m128 s0, s1, s2, d0, d1, d2;

                // Load source positions
                s0 = SrcPosAccessor::load(pSrcPos + 0);                 // x0 y0 z0 x1
                s1 = SrcPosAccessor::load(pSrcPos + 4);                 // y1 z1 x2 y2
                s2 = SrcPosAccessor::load(pSrcPos + 8);                 // z2 x3 y3 z3

                // Arrange to 3x4 component-major for batches calculate
                __MM_TRANSPOSE4x3_PS(s0, s1, s2);

                // Transform by collapsed matrix

                // Shuffle row 0 of four collapsed matrices for calculate X component
                __MM_TRANSPOSE4x4_PS(m00, m10, m20, m30);

                // Transform X components
                d0 = __MM_DOT4x3_PS(m00, m10, m20, m30, s0, s1, s2);    // X0 X1 X2 X3

                // Shuffle row 1 of four collapsed matrices for calculate Y component
                __MM_TRANSPOSE4x4_PS(m01, m11, m21, m31);

                // Transform Y components
                d1 = __MM_DOT4x3_PS(m01, m11, m21, m31, s0, s1, s2);    // Y0 Y1 Y2 Y3

                // Shuffle row 2 of four collapsed matrices for calculate Z component
                __MM_TRANSPOSE4x4_PS(m02, m12, m22, m32);

                // Transform Z components
                d2 = __MM_DOT4x3_PS(m02, m12, m22, m32, s0, s1, s2);    // Z0 Z1 Z2 Z3

                // Arrange back to 4x3 continuous format for store results
                __MM_TRANSPOSE3x4_PS(d0, d1, d2);

                // Store blended positions
                DestPosAccessor::store(pDestPos + 0, d0);
                DestPosAccessor::store(pDestPos + 4, d1);
                DestPosAccessor::store(pDestPos + 8, d2);

                // Advance 4 vertices
                pSrcPos += 4 * 3;
                pDestPos += 4 * 3;

                //------------------------------------------------------------------
                // Transform normals
                //------------------------------------------------------------------

                // Load source normals
                s0 = SrcNormAccessor::load(pSrcNorm + 0);               // x0 y0 z0 x1
                s1 = SrcNormAccessor::load(pSrcNorm + 4);               // y1 z1 x2 y2
                s2 = SrcNormAccessor::load(pSrcNorm + 8);               // z2 x3 y3 z3

                // Arrange to 3x4 component-major for batches calculate
                __MM_TRANSPOSE4x3_PS(s0, s1, s2);

                // Transform by collapsed and shuffled matrices
                d0 = __MM_DOT3x3_PS(m00, m10, m20, s0, s1, s2);         // X0 X1 X2 X3
                d1 = __MM_DOT3x3_PS(m01, m11, m21, s0, s1, s2);         // Y0 Y1 Y2 Y3
                d2 = __MM_DOT3x3_PS(m02, m12, m22, s0, s1, s2);         // Z0 Z1 Z2 Z3

                // Normalise normals
                __m128 tmp = __MM_DOT3x3_PS(d0, d1, d2, d0, d1, d2);
                tmp = __MM_RSQRT_PS(tmp);
                d0 = _mm_mul_ps(d0, tmp);
                d1 = _mm_mul_ps(d1, tmp);
                d2 = _mm_mul_ps(d2, tmp);

                // Arrange back to 4x3 continuous format for store results
                __MM_TRANSPOSE3x4_PS(d0, d1, d2);

                // Store blended normals
                DestNormAccessor::store(pDestNorm + 0, d0);
                DestNormAccessor::store(pDestNorm + 4, d1);
                DestNormAccessor::store(pDestNorm + 8, d2);

                // Advance 4 vertices
                pSrcNorm += 4 * 3;
                pDestNorm += 4 * 3;
            }
        }
    };
    static FORCEINLINE void softwareVertexSkinning_SSE_PosNorm_Separated_Packed(
        const float* pSrcPos, float* pDestPos,
        const float* pSrcNorm, float* pDestNorm,
        const float* pBlendWeight, const unsigned char* pBlendIndex,
        const Matrix4* const* blendMatrices,
        size_t blendWeightStride, size_t blendIndexStride,
        size_t numWeightsPerVertex,
        size_t numIterations)
    {
        assert(_isAlignedForSSE(pSrcPos));

        // Instantiating two version only, since other alignement combination not that important.
        if (_isAlignedForSSE(pSrcNorm) && _isAlignedForSSE(pDestPos) && _isAlignedForSSE(pDestNorm))
        {
            SoftwareVertexSkinning_SSE_PosNorm_Separated_Packed<true, true, true, true>::apply(
                pSrcPos, pDestPos,
                pSrcNorm, pDestNorm,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
        else
        {
            SoftwareVertexSkinning_SSE_PosNorm_Separated_Packed<true, false, false, false>::apply(
                pSrcPos, pDestPos,
                pSrcNorm, pDestNorm,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
    }
    //---------------------------------------------------------------------
    // Special SSE version skinning position only, the position buffer are
    // packed.
    template <bool srcPosAligned, bool destPosAligned>
    struct SoftwareVertexSkinning_SSE_PosOnly_Packed
    {
        static void apply(
            const float* pSrcPos, float* pDestPos,
            const float* pBlendWeight, const unsigned char* pBlendIndex,
            const Matrix4* const* blendMatrices,
            size_t blendWeightStride, size_t blendIndexStride,
            size_t numWeightsPerVertex,
            size_t numIterations)
        {
            typedef SSEMemoryAccessor<srcPosAligned> SrcPosAccessor;
            typedef SSEMemoryAccessor<destPosAligned> DestPosAccessor;

            // Blending 4 vertices per-iteration
            for (size_t i = 0; i < numIterations; ++i)
            {
                // Collapse matrices
                __m128 m00, m01, m02, m10, m11, m12, m20, m21, m22, m30, m31, m32;
                _collapseFourMatrices(
                    m00, m01, m02,
                    m10, m11, m12,
                    m20, m21, m22,
                    m30, m31, m32,
                    pBlendWeight, pBlendIndex,
                    blendMatrices,
                    blendWeightStride, blendIndexStride,
                    numWeightsPerVertex);

                // Advance 4 vertices
                advanceRawPointer(pBlendWeight, 4 * blendWeightStride);
                advanceRawPointer(pBlendIndex, 4 * blendIndexStride);

                //------------------------------------------------------------------
                // Transform positions
                //------------------------------------------------------------------

                __m128 s0, s1, s2, d0, d1, d2;

                // Load source positions
                s0 = SrcPosAccessor::load(pSrcPos + 0);                 // x0 y0 z0 x1
                s1 = SrcPosAccessor::load(pSrcPos + 4);                 // y1 z1 x2 y2
                s2 = SrcPosAccessor::load(pSrcPos + 8);                 // z2 x3 y3 z3

                // Arrange to 3x4 component-major for batches calculate
                __MM_TRANSPOSE4x3_PS(s0, s1, s2);

                // Transform by collapsed matrix

                // Shuffle row 0 of four collapsed matrices for calculate X component
                __MM_TRANSPOSE4x4_PS(m00, m10, m20, m30);

                // Transform X components
                d0 = __MM_DOT4x3_PS(m00, m10, m20, m30, s0, s1, s2);    // X0 X1 X2 X3

                // Shuffle row 1 of four collapsed matrices for calculate Y component
                __MM_TRANSPOSE4x4_PS(m01, m11, m21, m31);

                // Transform Y components
                d1 = __MM_DOT4x3_PS(m01, m11, m21, m31, s0, s1, s2);    // Y0 Y1 Y2 Y3

                // Shuffle row 2 of four collapsed matrices for calculate Z component
                __MM_TRANSPOSE4x4_PS(m02, m12, m22, m32);

                // Transform Z components
                d2 = __MM_DOT4x3_PS(m02, m12, m22, m32, s0, s1, s2);    // Z0 Z1 Z2 Z3

                // Arrange back to 4x3 continuous format for store results
                __MM_TRANSPOSE3x4_PS(d0, d1, d2);

                // Store blended positions
                DestPosAccessor::store(pDestPos + 0, d0);
                DestPosAccessor::store(pDestPos + 4, d1);
                DestPosAccessor::store(pDestPos + 8, d2);

                // Advance 4 vertices
                pSrcPos += 4 * 3;
                pDestPos += 4 * 3;
            }
        }
    };
    static FORCEINLINE void softwareVertexSkinning_SSE_PosOnly_Packed(
        const float* pSrcPos, float* pDestPos,
        const float* pBlendWeight, const unsigned char* pBlendIndex,
        const Matrix4* const* blendMatrices,
        size_t blendWeightStride, size_t blendIndexStride,
        size_t numWeightsPerVertex,
        size_t numIterations)
    {
        assert(_isAlignedForSSE(pSrcPos));

        // Instantiating two version only, since other alignement combination not that important.
        if (_isAlignedForSSE(pDestPos))
        {
            SoftwareVertexSkinning_SSE_PosOnly_Packed<true, true>::apply(
                pSrcPos, pDestPos,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
        else
        {
            SoftwareVertexSkinning_SSE_PosOnly_Packed<true, false>::apply(
                pSrcPos, pDestPos,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numIterations);
        }
    }
    //---------------------------------------------------------------------
    // SSE version mainly function, pickup and perform special optimised function
    // based on passed in parameters.
    static void softwareVertexSkinning_SSE_Dispatch(
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
        // All position/normal pointers should be perfect aligned, but still check here
        // for avoid hardware buffer which allocated by potential buggy driver doesn't
        // support alignment properly.
        // Because we are used meta-function technique here, the code is easy to maintenance
        // and still provides all possible alignment combination.
        //

        // Use unrolled routines only if there a lot of vertices
        if (numVertices > OGRE_SSE_SKINNING_UNROLL_VERTICES)
        {
            if (pSrcNorm)
            {
                // Blend position and normal

                if (srcPosStride == sizeof(float) * (3 + 3) && destPosStride == sizeof(float) * (3 + 3) &&
                    pSrcNorm == pSrcPos + 3 && pDestNorm == pDestPos + 3)
                {
                    // Position and normal are sharing with packed buffer

                    size_t srcPosAlign = (size_t)pSrcPos & 15;
                    assert((srcPosAlign & 3) == 0);

                    // Blend unaligned vertices with general SIMD routine
                    if (srcPosAlign == 8)   // Because 8 bytes alignment shift per-vertex
                    {
                        size_t count = srcPosAlign / 8;
                        numVertices -= count;
                        softwareVertexSkinning_SSE_General(
                            pSrcPos, pDestPos,
                            pSrcNorm, pDestNorm,
                            pBlendWeight, pBlendIndex,
                            blendMatrices,
                            srcPosStride, destPosStride,
                            srcNormStride, destNormStride,
                            blendWeightStride, blendIndexStride,
                            numWeightsPerVertex,
                            count);

                        pSrcPos += count * (3 + 3);
                        pDestPos += count * (3 + 3);
                        pSrcNorm += count * (3 + 3);
                        pDestNorm += count * (3 + 3);
                        advanceRawPointer(pBlendWeight, count * blendWeightStride);
                        advanceRawPointer(pBlendIndex, count * blendIndexStride);
                    }

                    // Blend vertices, four vertices per-iteration
                    size_t numIterations = numVertices / 4;
                    softwareVertexSkinning_SSE_PosNorm_Shared_Packed(
                        pSrcPos, pDestPos,
                        pBlendWeight, pBlendIndex,
                        blendMatrices,
                        blendWeightStride, blendIndexStride,
                        numWeightsPerVertex,
                        numIterations);

                    // Advance pointers for remaining vertices
                    numVertices &= 3;
                    if (numVertices)
                    {
                        pSrcPos += numIterations * 4 * (3 + 3);
                        pDestPos += numIterations * 4 * (3 + 3);
                        pSrcNorm += numIterations * 4 * (3 + 3);
                        pDestNorm += numIterations * 4 * (3 + 3);
                        advanceRawPointer(pBlendWeight, numIterations * 4 * blendWeightStride);
                        advanceRawPointer(pBlendIndex, numIterations * 4 * blendIndexStride);
                    }
                }
                else if (srcPosStride == sizeof(float) * 3 && destPosStride == sizeof(float) * 3 &&
                         srcNormStride == sizeof(float) * 3 && destNormStride == sizeof(float) * 3)
                {
                    // Position and normal are separate buffers, and all of them are packed

                    size_t srcPosAlign = (size_t)pSrcPos & 15;
                    assert((srcPosAlign & 3) == 0);

                    // Blend unaligned vertices with general SIMD routine
                    if (srcPosAlign)
                    {
                        size_t count = srcPosAlign / 4;
                        numVertices -= count;
                        softwareVertexSkinning_SSE_General(
                            pSrcPos, pDestPos,
                            pSrcNorm, pDestNorm,
                            pBlendWeight, pBlendIndex,
                            blendMatrices,
                            srcPosStride, destPosStride,
                            srcNormStride, destNormStride,
                            blendWeightStride, blendIndexStride,
                            numWeightsPerVertex,
                            count);

                        pSrcPos += count * 3;
                        pDestPos += count * 3;
                        pSrcNorm += count * 3;
                        pDestNorm += count * 3;
                        advanceRawPointer(pBlendWeight, count * blendWeightStride);
                        advanceRawPointer(pBlendIndex, count * blendIndexStride);
                    }

                    // Blend vertices, four vertices per-iteration
                    size_t numIterations = numVertices / 4;
                    softwareVertexSkinning_SSE_PosNorm_Separated_Packed(
                        pSrcPos, pDestPos,
                        pSrcNorm, pDestNorm,
                        pBlendWeight, pBlendIndex,
                        blendMatrices,
                        blendWeightStride, blendIndexStride,
                        numWeightsPerVertex,
                        numIterations);

                    // Advance pointers for remaining vertices
                    numVertices &= 3;
                    if (numVertices)
                    {
                        pSrcPos += numIterations * 4 * 3;
                        pDestPos += numIterations * 4 * 3;
                        pSrcNorm += numIterations * 4 * 3;
                        pDestNorm += numIterations * 4 * 3;
                        advanceRawPointer(pBlendWeight, numIterations * 4 * blendWeightStride);
                        advanceRawPointer(pBlendIndex, numIterations * 4 * blendIndexStride);
                    }
                }
                else    // Not 'packed' form or wrong order between position and normal
                {
                    // Should never occuring, do nothing here just in case
                }
            }
            else    // !pSrcNorm
            {
                // Blend position only

                if (srcPosStride == sizeof(float) * 3 && destPosStride == sizeof(float) * 3)
                {
                    // All buffers are packed

                    size_t srcPosAlign = (size_t)pSrcPos & 15;
                    assert((srcPosAlign & 3) == 0);

                    // Blend unaligned vertices with general SIMD routine
                    if (srcPosAlign)
                    {
                        size_t count = srcPosAlign / 4;
                        numVertices -= count;
                        softwareVertexSkinning_SSE_General(
                            pSrcPos, pDestPos,
                            pSrcNorm, pDestNorm,
                            pBlendWeight, pBlendIndex,
                            blendMatrices,
                            srcPosStride, destPosStride,
                            srcNormStride, destNormStride,
                            blendWeightStride, blendIndexStride,
                            numWeightsPerVertex,
                            count);

                        pSrcPos += count * 3;
                        pDestPos += count * 3;
                        advanceRawPointer(pBlendWeight, count * blendWeightStride);
                        advanceRawPointer(pBlendIndex, count * blendIndexStride);
                    }

                    // Blend vertices, four vertices per-iteration
                    size_t numIterations = numVertices / 4;
                    softwareVertexSkinning_SSE_PosOnly_Packed(
                        pSrcPos, pDestPos,
                        pBlendWeight, pBlendIndex,
                        blendMatrices,
                        blendWeightStride, blendIndexStride,
                        numWeightsPerVertex,
                        numIterations);

                    // Advance pointers for remaining vertices
                    numVertices &= 3;
                    if (numVertices)
                    {
                        pSrcPos += numIterations * 4 * 3;
                        pDestPos += numIterations * 4 * 3;
                        advanceRawPointer(pBlendWeight, numIterations * 4 * blendWeightStride);
                        advanceRawPointer(pBlendIndex, numIterations * 4 * blendIndexStride);
                    }
                }
                else    // Not 'packed' form
                {
                    // Might occuring only if user forced software blending position only
                }
            }
        }

        // Blend remaining vertices, need to do it with SIMD for identical result,
        // since mixing general floating-point and SIMD algorithm will causing
        // floating-point error.
        if (numVertices)
        {
            softwareVertexSkinning_SSE_General(
                pSrcPos, pDestPos,
                pSrcNorm, pDestNorm,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                srcPosStride, destPosStride,
                srcNormStride, destNormStride,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numVertices);
        }
    }
    //---------------------------------------------------------------------
    // SSE version mainly function special designed for AMD, pickup and perform
    // special optimised function based on passed in parameters.
    static void softwareVertexSkinning_SSE_AMD(
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
        // All position/normal pointers should be perfect aligned, but still checking here
        // for avoid hardware buffer which allocated by potential buggy driver doesn't
        // support alignment properly.
        // Because we are used meta-function technique here, the code is easy to maintenance
        // and still provides all possible alignment combination.
        //

        // Use unrolled routines only if there a lot of vertices
        if (numVertices > OGRE_SSE_SKINNING_UNROLL_VERTICES)
        {
            if (pSrcNorm)
            {
                // Blend position and normal

                // For AMD Athlon XP, it's prefer to never use unrolled version for shared
                // buffers at all, I guess because that version run out of CPU register count,
                // or L1 cache related problem, causing slight performance loss than general
                // version.
                //
                if (srcPosStride == sizeof(float) * 3 && destPosStride == sizeof(float) * 3 &&
                    srcNormStride == sizeof(float) * 3 && destNormStride == sizeof(float) * 3)
                {
                    // Position and normal are separate buffers, and all of them are packed

                    size_t srcPosAlign = (size_t)pSrcPos & 15;
                    assert((srcPosAlign & 3) == 0);

                    // Blend unaligned vertices with general SIMD routine
                    if (srcPosAlign)
                    {
                        size_t count = srcPosAlign / 4;
                        numVertices -= count;
                        softwareVertexSkinning_SSE_General(
                            pSrcPos, pDestPos,
                            pSrcNorm, pDestNorm,
                            pBlendWeight, pBlendIndex,
                            blendMatrices,
                            srcPosStride, destPosStride,
                            srcNormStride, destNormStride,
                            blendWeightStride, blendIndexStride,
                            numWeightsPerVertex,
                            count);

                        pSrcPos += count * 3;
                        pDestPos += count * 3;
                        pSrcNorm += count * 3;
                        pDestNorm += count * 3;
                        advanceRawPointer(pBlendWeight, count * blendWeightStride);
                        advanceRawPointer(pBlendIndex, count * blendIndexStride);
                    }

                    // Blend vertices, four vertices per-iteration
                    size_t numIterations = numVertices / 4;
                    softwareVertexSkinning_SSE_PosNorm_Separated_Packed(
                        pSrcPos, pDestPos,
                        pSrcNorm, pDestNorm,
                        pBlendWeight, pBlendIndex,
                        blendMatrices,
                        blendWeightStride, blendIndexStride,
                        numWeightsPerVertex,
                        numIterations);

                    // Advance pointers for remaining vertices
                    numVertices &= 3;
                    if (numVertices)
                    {
                        pSrcPos += numIterations * 4 * 3;
                        pDestPos += numIterations * 4 * 3;
                        pSrcNorm += numIterations * 4 * 3;
                        pDestNorm += numIterations * 4 * 3;
                        advanceRawPointer(pBlendWeight, numIterations * 4 * blendWeightStride);
                        advanceRawPointer(pBlendIndex, numIterations * 4 * blendIndexStride);
                    }
                }
                else
                {
                    // Do nothing here, will fallback general version
                }
            }
            else    // !pSrcNorm
            {
                // Blend position only

                if (srcPosStride == sizeof(float) * 3 && destPosStride == sizeof(float) * 3)
                {
                    // All buffers are packed

                    size_t srcPosAlign = (size_t)pSrcPos & 15;
                    assert((srcPosAlign & 3) == 0);

                    // Blend unaligned vertices with general SIMD routine
                    if (srcPosAlign)
                    {
                        size_t count = srcPosAlign / 4;
                        numVertices -= count;
                        softwareVertexSkinning_SSE_General(
                            pSrcPos, pDestPos,
                            pSrcNorm, pDestNorm,
                            pBlendWeight, pBlendIndex,
                            blendMatrices,
                            srcPosStride, destPosStride,
                            srcNormStride, destNormStride,
                            blendWeightStride, blendIndexStride,
                            numWeightsPerVertex,
                            count);

                        pSrcPos += count * 3;
                        pDestPos += count * 3;
                        advanceRawPointer(pBlendWeight, count * blendWeightStride);
                        advanceRawPointer(pBlendIndex, count * blendIndexStride);
                    }

                    // Blend vertices, four vertices per-iteration
                    size_t numIterations = numVertices / 4;
                    softwareVertexSkinning_SSE_PosOnly_Packed(
                        pSrcPos, pDestPos,
                        pBlendWeight, pBlendIndex,
                        blendMatrices,
                        blendWeightStride, blendIndexStride,
                        numWeightsPerVertex,
                        numIterations);

                    // Advance pointers for remaining vertices
                    numVertices &= 3;
                    if (numVertices)
                    {
                        pSrcPos += numIterations * 4 * 3;
                        pDestPos += numIterations * 4 * 3;
                        advanceRawPointer(pBlendWeight, numIterations * 4 * blendWeightStride);
                        advanceRawPointer(pBlendIndex, numIterations * 4 * blendIndexStride);
                    }
                }
                else    // Not 'packed' form
                {
                    // Might occuring only if user forced software blending position only
                }
            }
        }

        // Blend remaining vertices, need to do it with SIMD for identical result,
        // since mixing general floating-point and SIMD algorithm will causing
        // floating-point error.
        if (numVertices)
        {
            softwareVertexSkinning_SSE_General(
                pSrcPos, pDestPos,
                pSrcNorm, pDestNorm,
                pBlendWeight, pBlendIndex,
                blendMatrices,
                srcPosStride, destPosStride,
                srcNormStride, destNormStride,
                blendWeightStride, blendIndexStride,
                numWeightsPerVertex,
                numVertices);
        }
    }
#endif // OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86 && OGRE_COMPILER == OGRE_COMPILER_MSVC

}

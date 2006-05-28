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
#ifndef __SIMDHelper_H__
#define __SIMDHelper_H__

#include "OgrePrerequisites.h"

// Additional platform-dependent header files

#if OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86 && OGRE_COMPILER == OGRE_COMPILER_MSVC
#include "OgreNoMemoryMacros.h"
#include <xmmintrin.h>
#include "OgreMemoryMacros.h"

#endif // OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86 && OGRE_COMPILER == OGRE_COMPILER_MSVC



//---------------------------------------------------------------------
// SIMD macros and helpers
//---------------------------------------------------------------------


namespace Ogre {

#if OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86 && OGRE_COMPILER == OGRE_COMPILER_MSVC

/** Macro __MM_RSQRT_PS calculate square root, which should be used for
    normalise normals only. It might be use NewtonRaphson reciprocal square
    root for high precision, or use SSE rsqrt instruction directly, based
    on profile to pick up perfect one.
@note:
    Prefer to never use NewtonRaphson reciprocal square root at all, since
    speed test indicate performance loss 10% for unrolled version, and loss
    %25 for general version (P4 3.0G HT). A slight loss in precision not
    that important in case of normalise normals.
*/
#if 1
#define __MM_RSQRT_PS(x)    _mm_rsqrt_ps(x)
#else
#define __MM_RSQRT_PS(x)    __mm_rsqrt_nr_ps(x) // Implemented below
#endif

/** Performing the transpose of a 4x4 matrix of single precision floating
    point values.
    Arguments r0, r1, r2, and r3 are __m128 values whose elements
    form the corresponding rows of a 4x4 matrix.
    The matrix transpose is returned in arguments r0, r1, r2, and
    r3 where r0 now holds column 0 of the original matrix, r1 now
    holds column 1 of the original matrix, etc.
*/
#define __MM_TRANSPOSE4x4_PS(r0, r1, r2, r3)                                        \
    {                                                                               \
        __m128 t3, t2, t1, t0;                                                      \
                                                                                    \
                                                            /* r00 r01 r02 r03 */   \
                                                            /* r10 r11 r12 r13 */   \
                                                            /* r20 r21 r22 r23 */   \
                                                            /* r30 r31 r32 r33 */   \
                                                                                    \
        t0 = _mm_unpacklo_ps(r0, r1);                       /* r00 r10 r01 r11 */   \
        t2 = _mm_unpackhi_ps(r0, r1);                       /* r02 r12 r03 r13 */   \
        t1 = _mm_unpacklo_ps(r2, r3);                       /* r20 r30 r21 r31 */   \
        t3 = _mm_unpackhi_ps(r2, r3);                       /* r22 r32 r23 r33 */   \
                                                                                    \
        r0 = _mm_movelh_ps(t0, t1);                         /* r00 r10 r20 r30 */   \
        r1 = _mm_movehl_ps(t1, t0);                         /* r01 r11 r21 r31 */   \
        r2 = _mm_movelh_ps(t2, t3);                         /* r02 r12 r22 r32 */   \
        r3 = _mm_movehl_ps(t3, t2);                         /* r03 r13 r23 r33 */   \
    }

/** Performing the transpose of a continuous stored rows of a 4x3 matrix to
    a 3x4 matrix of single precision floating point values.
    Arguments v0, v1, and v2 are __m128 values whose elements form the
    corresponding continuous stored rows of a 4x3 matrix.
    The matrix transpose is returned in arguments v0, v1, and v2, where
    v0 now holds column 0 of the original matrix, v1 now holds column 1
    of the original matrix, etc.
*/
#define __MM_TRANSPOSE4x3_PS(v0, v1, v2)                                            \
    {                                                                               \
        __m128 t0, t1, t2;                                                          \
                                                                                    \
                                                            /* r00 r01 r02 r10 */   \
                                                            /* r11 r12 r20 r21 */   \
                                                            /* r22 r30 r31 r32 */   \
                                                                                    \
        t0 = _mm_shuffle_ps(v0, v2, _MM_SHUFFLE(3,0,3,0));  /* r00 r10 r22 r32 */   \
        t1 = _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(1,0,2,1));  /* r01 r02 r11 r12 */   \
        t2 = _mm_shuffle_ps(v1, v2, _MM_SHUFFLE(2,1,3,2));  /* r20 r21 r30 r31 */   \
                                                                                    \
        v0 = _mm_shuffle_ps(t0, t2, _MM_SHUFFLE(2,0,1,0));  /* r00 r10 r20 r30 */   \
        v1 = _mm_shuffle_ps(t1, t2, _MM_SHUFFLE(3,1,2,0));  /* r01 r11 r21 r31 */   \
        v2 = _mm_shuffle_ps(t1, t0, _MM_SHUFFLE(3,2,3,1));  /* r02 r12 r22 r32 */   \
    }

/** Performing the transpose of a 3x4 matrix to a continuous stored rows of
    a 4x3 matrix of single precision floating point values.
    Arguments v0, v1, and v2 are __m128 values whose elements form the
    corresponding columns of a 3x4 matrix.
    The matrix transpose is returned in arguments v0, v1, and v2, as a
    continuous stored rows of a 4x3 matrix.
*/
#define __MM_TRANSPOSE3x4_PS(v0, v1, v2)                                            \
    {                                                                               \
        __m128 t0, t1, t2;                                                          \
                                                                                    \
                                                            /* r00 r10 r20 r30 */   \
                                                            /* r01 r11 r21 r31 */   \
                                                            /* r02 r12 r22 r32 */   \
                                                                                    \
        t0 = _mm_shuffle_ps(v0, v2, _MM_SHUFFLE(2,0,3,1));  /* r10 r30 r02 r22 */   \
        t1 = _mm_shuffle_ps(v1, v2, _MM_SHUFFLE(3,1,3,1));  /* r11 r31 r12 r32 */   \
        t2 = _mm_shuffle_ps(v0, v1, _MM_SHUFFLE(2,0,2,0));  /* r00 r20 r01 r21 */   \
                                                                                    \
        v0 = _mm_shuffle_ps(t2, t0, _MM_SHUFFLE(0,2,2,0));  /* r00 r01 r02 r10 */   \
        v1 = _mm_shuffle_ps(t1, t2, _MM_SHUFFLE(3,1,2,0));  /* r11 r12 r20 r21 */   \
        v2 = _mm_shuffle_ps(t0, t1, _MM_SHUFFLE(3,1,1,3));  /* r22 r30 r31 r32 */   \
    }

/// Accumulate four vector of single precision floating point values.
#define __MM_ACCUM4_PS(a, b, c, d)                                                  \
    _mm_add_ps(_mm_add_ps(a, b), _mm_add_ps(c, d))

/** Performing dot-product between four vector and three vector of single
    precision floating point values.
*/
#define __MM_DOT4x3_PS(r0, r1, r2, r3, v0, v1, v2)                                  \
    __MM_ACCUM4_PS(_mm_mul_ps(r0, v0), _mm_mul_ps(r1, v1), _mm_mul_ps(r2, v2), r3)

/// Accumulate three vector of single precision floating point values.
#define __MM_ACCUM3_PS(a, b, c)                                                     \
    _mm_add_ps(_mm_add_ps(a, b), c)

/** Performing dot-product between two of three vector of single precision
    floating point values.
*/
#define __MM_DOT3x3_PS(r0, r1, r2, v0, v1, v2)                                      \
    __MM_ACCUM3_PS(_mm_mul_ps(r0, v0), _mm_mul_ps(r1, v1), _mm_mul_ps(r2, v2))

/// Calculate multiply of two vector and plus another vector
#define __MM_MADD_PS(a, b, c)                                                       \
    _mm_add_ps(_mm_mul_ps(a, b), c)

/// Linear interpolation
#define __MM_LERP_PS(t, a, b)                                                       \
    __MM_MADD_PS(_mm_sub_ps(b, a), t, a)

/// Same as __MM_LERP_PS, but can generate more optimised code in VC somewhere.
#define __MM_LERP2_PS(t, a, b)                                                      \
    __MM_MADD_PS(t, _mm_sub_ps(b, a), a)

/// Same as _mm_load_ps, but can help VC generate more optimised code.
#define __MM_LOAD_PS(p)                                                             \
    (*(__m128*)(p))

/// Same as _mm_store_ps, but can help VC generate more optimised code.
#define __MM_STORE_PS(p, v)                                                         \
    (*(__m128*)(p) = (v))


    /** Helper to load/store SSE data based on whether or not aligned.
    */
    template <bool aligned = false>
    struct SSEMemoryAccessor
    {
        static FORCEINLINE __m128 load(const float *p)
        {
            return _mm_loadu_ps(p);
        }
        static FORCEINLINE void store(float *p, const __m128& v)
        {
            _mm_storeu_ps(p, v);
        }
    };
    // Special aligned accessor
    template <>
    struct SSEMemoryAccessor<true>
    {
        static FORCEINLINE const __m128& load(const float *p)
        {
            return __MM_LOAD_PS(p);
        }
        static FORCEINLINE void store(float *p, const __m128& v)
        {
            __MM_STORE_PS(p, v);
        }
    };

    /** Check whether or not the given pointer perfect aligned for SSE.
    */
    static FORCEINLINE bool _isAlignedForSSE(const void *p)
    {
        return (((size_t)p) & 15) == 0;
    }

    /** Calculate NewtonRaphson Reciprocal Square Root with formula:
            0.5 * rsqrt(x) * (3 - x * rsqrt(x)^2)
    */
    static FORCEINLINE __m128 __mm_rsqrt_nr_ps(const __m128& x)
    {
        static const __m128 v0pt5 = { 0.5f, 0.5f, 0.5f, 0.5f };
        static const __m128 v3pt0 = { 3.0f, 3.0f, 3.0f, 3.0f };
        __m128 t = _mm_rsqrt_ps(x);
        return _mm_mul_ps(_mm_mul_ps(v0pt5, t),
            _mm_sub_ps(v3pt0, _mm_mul_ps(_mm_mul_ps(x, t), t)));
    }

#endif  // OGRE_DOUBLE_PRECISION == 0 && OGRE_CPU == OGRE_CPU_X86 && OGRE_COMPILER == OGRE_COMPILER_MSVC

}

#endif // __SIMDHelper_H__

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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/
#ifndef __Matrix3_H__
#define __Matrix3_H__

#include "OgrePrerequisites.h"

#include "OgreVector3.h"

// NB All code adapted from Wild Magic 0.2 Matrix math (free source code)
// http://www.magic-software.com

// NOTE.  The (x,y,z) coordinate system is assumed to be right-handed.
// Coordinate axis rotation matrices are of the form
//   RX =    1       0       0
//           0     cos(t) -sin(t)
//           0     sin(t)  cos(t)
// where t > 0 indicates a counterclockwise rotation in the yz-plane
//   RY =  cos(t)    0     sin(t)
//           0       1       0
//        -sin(t)    0     cos(t)
// where t > 0 indicates a counterclockwise rotation in the zx-plane
//   RZ =  cos(t) -sin(t)    0
//         sin(t)  cos(t)    0
//           0       0       1
// where t > 0 indicates a counterclockwise rotation in the xy-plane.

namespace Ogre
{
    /** A 3x3 matrix which can represent rotations around axes.
        @note
            <b>All the code is adapted from the Wild Magic 0.2 Matrix
            library (http://www.magic-software.com).
        @par
            The coordinate system is assumed to be <b>right-handed</b>.
    */
    class _OgreExport Matrix3
    {
    public:
        // construction
        Matrix3 ();
        Matrix3 (const Real arr[3][3]);
        Matrix3 (const Matrix3& rkMatrix);
        Matrix3 (Real fEntry00, Real fEntry01, Real fEntry02,
                    Real fEntry10, Real fEntry11, Real fEntry12,
                    Real fEntry20, Real fEntry21, Real fEntry22);

        // member access, allows use of construct mat[r][c]
        Real* operator[] (int iRow) const;
        operator Real* ();
        Vector3 GetColumn (int iCol) const;
        void SetColumn(int iCol, const Vector3& vec);
        void FromAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);

        // assignment and comparison
        Matrix3& operator= (const Matrix3& rkMatrix);
        bool operator== (const Matrix3& rkMatrix) const;
        bool operator!= (const Matrix3& rkMatrix) const;

        // arithmetic operations
        Matrix3 operator+ (const Matrix3& rkMatrix) const;
        Matrix3 operator- (const Matrix3& rkMatrix) const;
        Matrix3 operator* (const Matrix3& rkMatrix) const;
        Matrix3 operator- () const;

        // matrix * vector [3x3 * 3x1 = 3x1]
        Vector3 operator* (const Vector3& rkVector) const;

        // vector * matrix [1x3 * 3x3 = 1x3]
        friend Vector3 operator* (const Vector3& rkVector,
            const Matrix3& rkMatrix);

        // matrix * scalar
        Matrix3 operator* (Real fScalar) const;

        // scalar * matrix
        friend Matrix3 operator* (Real fScalar, const Matrix3& rkMatrix);

        // utilities
        Matrix3 Transpose () const;
        bool Inverse (Matrix3& rkInverse, Real fTolerance = 1e-06) const;
        Matrix3 Inverse (Real fTolerance = 1e-06) const;
        Real Determinant () const;

        // singular value decomposition
        void SingularValueDecomposition (Matrix3& rkL, Vector3& rkS,
            Matrix3& rkR) const;
        void SingularValueComposition (const Matrix3& rkL,
            const Vector3& rkS, const Matrix3& rkR);

        // Gram-Schmidt orthonormalization (applied to columns of rotation matrix)
        void Orthonormalize ();

        // orthogonal Q, diagonal D, upper triangular U stored as (u01,u02,u12)
        void QDUDecomposition (Matrix3& rkQ, Vector3& rkD,
            Vector3& rkU) const;

        Real SpectralNorm () const;

        // matrix must be orthonormal
        void ToAxisAngle (Vector3& rkAxis, Real& rfRadians) const;
        void FromAxisAngle (const Vector3& rkAxis, Real fRadians);

        // The matrix must be orthonormal.  The decomposition is yaw*pitch*roll
        // where yaw is rotation about the Up vector, pitch is rotation about the
        // Right axis, and roll is rotation about the Direction axis.
        bool ToEulerAnglesXYZ (float& rfYAngle, float& rfPAngle,
            float& rfRAngle) const;
        bool ToEulerAnglesXZY (float& rfYAngle, float& rfPAngle,
            float& rfRAngle) const;
        bool ToEulerAnglesYXZ (float& rfYAngle, float& rfPAngle,
            float& rfRAngle) const;
        bool ToEulerAnglesYZX (float& rfYAngle, float& rfPAngle,
            float& rfRAngle) const;
        bool ToEulerAnglesZXY (float& rfYAngle, float& rfPAngle,
            float& rfRAngle) const;
        bool ToEulerAnglesZYX (float& rfYAngle, float& rfPAngle,
            float& rfRAngle) const;
        void FromEulerAnglesXYZ (float fYAngle, float fPAngle, float fRAngle);
        void FromEulerAnglesXZY (float fYAngle, float fPAngle, float fRAngle);
        void FromEulerAnglesYXZ (float fYAngle, float fPAngle, float fRAngle);
        void FromEulerAnglesYZX (float fYAngle, float fPAngle, float fRAngle);
        void FromEulerAnglesZXY (float fYAngle, float fPAngle, float fRAngle);
        void FromEulerAnglesZYX (float fYAngle, float fPAngle, float fRAngle);

        // eigensolver, matrix must be symmetric
        void EigenSolveSymmetric (Real afEigenvalue[3],
            Vector3 akEigenvector[3]) const;

        static void TensorProduct (const Vector3& rkU, const Vector3& rkV,
            Matrix3& rkProduct);

        static const Real EPSILON;
        static const Matrix3 ZERO;
        static const Matrix3 IDENTITY;

    protected:
        // support for eigensolver
        void Tridiagonal (Real afDiag[3], Real afSubDiag[3]);
        bool QLAlgorithm (Real afDiag[3], Real afSubDiag[3]);

        // support for singular value decomposition
        static const Real ms_fSvdEpsilon;
        static const int ms_iSvdMaxIterations;
        static void Bidiagonalize (Matrix3& kA, Matrix3& kL,
            Matrix3& kR);
        static void GolubKahanStep (Matrix3& kA, Matrix3& kL,
            Matrix3& kR);

        // support for spectral norm
        static Real MaxCubicRoot (Real afCoeff[3]);

        Real m[3][3];

        // for faster access
        friend class Matrix4;
    };
}
#endif

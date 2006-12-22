/*
-----------------------------------------------------------------------------
This source file is part of LEXIExporter

Copyright 2006 NDS Limited

Author(s):
Bo Krohn

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

/////////////////////////////////////////////////////
//
//  Matrix 4x4 class
//
/////////////////////////////////////////////////////

#ifndef __Matrix_4x4_Class__
#define __Matrix_4x4_Class__

//

class CMatrix {

	public:

		static const CMatrix _identity;
		static const CMatrix _zero;

		// Constructors/Destructor
		inline CMatrix()
		{
		}
		inline CMatrix(const CMatrix& m)
		{
			memcpy(mat, m.mat, 4 * 4 * sizeof(float));
		}
		inline CMatrix(const float* m)
		{
			memcpy(mat, m, 4 * 4 * sizeof(float));
		}

		// Direct access to matrix
		float mat[4][4];

		// Assignment
		inline CMatrix& operator = (const CMatrix& m)
		{
			memcpy(mat, m.mat, 4*4*sizeof(float));

			return *this;
		}

		// Comparing
		inline bool operator == (const CMatrix& m) const
		{
			return memcmp(mat, m.mat, 4*4*sizeof(float))==0 ? true : false;
		}

		inline bool operator != (const CMatrix& m) const
		{
			return memcmp(mat, m.mat, 4*4*sizeof(float))!=0 ? true : false;
		}

		inline bool IsIdentity() const
		{
			return memcmp(mat, _identity.mat, 4 * 4 * sizeof(float)) ? false : true;
		}

		// Operators
		inline CMatrix operator + (const CMatrix& m) const
		{
			CMatrix ma;

			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					ma.mat[r][c] = mat[r][c] + m.mat[r][c];
				}
			}

			return ma;
		}

		inline CMatrix& operator += (const CMatrix& m)
		{
			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					mat[r][c] += m.mat[r][c];
				}
			}

			return *this;
		}

		inline CMatrix operator - (const CMatrix& m) const
		{
			CMatrix ma;

			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					ma.mat[r][c] = mat[r][c] - m.mat[r][c];
				}
			}

			return ma;
		}

		inline CMatrix& operator -= (const CMatrix& m)
		{
			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					mat[r][c] -= m.mat[r][c];
				}
			}

			return *this;
		}

		inline CMatrix operator * (float f) const
		{
			CMatrix ma;

			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					ma.mat[r][c] = mat[r][c] * f;
				}
			}

			return ma;
		}

		inline CMatrix& operator *= (float f)
		{
			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					mat[r][c] *= f;
				}
			}

			return *this;
		}

		inline CMatrix operator / (float f) const
		{
			CMatrix ma;

			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					ma.mat[r][c] = mat[r][c] / f;
				}
			}

			return ma;
		}

		inline CMatrix& operator /= (float f)
		{
			for(unsigned int r=0; r<4; r++)
			{
				for(unsigned int c=0; c<4; c++)
				{
					mat[r][c] /= f;
				}
			}

			return *this;
		}

		// Functions
		inline void makeIdentity()
		{
			mat[0][0] = 1.0f; mat[0][1] = 0.0f; mat[0][2] = 0.0f; mat[0][3] = 0.0f;
			mat[1][0] = 0.0f; mat[1][1] = 1.0f; mat[1][2] = 0.0f; mat[1][3] = 0.0f;
			mat[2][0] = 0.0f; mat[2][1] = 0.0f; mat[2][2] = 1.0f; mat[2][3] = 0.0f;
			mat[3][0] = 0.0f; mat[3][1] = 0.0f; mat[3][2] = 0.0f; mat[3][3] = 1.0f; 
		}

		void makeLookAt(const CVec3& eye, const CVec3& point, const CVec3& up);

		void makeLookAtDirection(const CVec3& eye, const CVec3& dir, const CVec3& up);

		void makePerspective(float left, float right, float bottom, float top, float znear, float zfar);

		void makePerspectiveFOV(float hfov, float vfov, float aspect, float znear, float zfar);

		void makeOrthogonalPerspective(float left, float right, float bottom, float top, float znear, float zfar);

		inline void makeRotation(float angle, const CVec3& axis)
		{
			makeRotationRadians(UtilDegToRad(angle), axis);
		}

		inline void makeRotationRadians(float angle, const CVec3& axis)
		{
			setRotationRadians(angle, axis);
			setRow(3, CVec4::_w);
			setColumn(3, CVec3::_zero);
		}

		inline void makeRotation(float anglex, float angley, float anglez)
		{
			makeRotationRadians(UtilDegToRad(anglex), UtilDegToRad(angley), UtilDegToRad(anglez));
		}

		inline void makeRotationRadians(float anglex, float angley, float anglez)
		{
			setRotationRadians(anglex, angley, anglez);
			setRow(3, CVec4::_w);
			setColumn(3, CVec3::_zero);
		}

		inline void setRotation(float angle, const CVec3& axis)
		{
			setRotationRadians(UtilDegToRad(angle), axis);
		}

		void setRotationRadians(float angle, const CVec3& axis);

		inline void setRotation(float anglex, float angley, float anglez)
		{
			setRotationRadians(UtilDegToRad(anglex), UtilDegToRad(angley), UtilDegToRad(anglez));
		}

		void setRotationRadians(float anglex, float angley, float anglez);

		inline void getRotation(float& anglex, float& angley, float& anglez)
		{
			getRotationRadians(anglex, angley, anglez);

	        anglex = UtilRadToDeg(anglex);
	        angley = UtilRadToDeg(angley);
	        anglez = UtilRadToDeg(anglez);
		}

		void getRotationRadians(float& anglex, float& angley, float& anglez);

		inline void makeScale(const CVec3& scale)
		{
			makeIdentity();
			mat[0][0] = scale.x;
			mat[1][1] = scale.y;
			mat[2][2] = scale.z;
		}

		inline void makeScale(float scalar)
		{
			makeIdentity();
			mat[0][0] = scalar;
			mat[1][1] = scalar;
			mat[2][2] = scalar;
		}

		inline void setScale(const CVec3& scale)
		{
			mat[0][0] = scale.x;
			mat[1][1] = scale.y;
			mat[2][2] = scale.z;
		}

		inline void preScale(const CVec3& vector)
		{
			mat[0][0] *= vector.x;
			mat[0][1] *= vector.x;
			mat[0][2] *= vector.x;
			mat[0][3] *= vector.x;

			mat[1][0] *= vector.y;
			mat[1][1] *= vector.y;
			mat[1][2] *= vector.y;
			mat[1][3] *= vector.y;

			mat[2][0] *= vector.z;
			mat[2][1] *= vector.z;
			mat[2][2] *= vector.z;
			mat[2][3] *= vector.z;
		}

		inline void postScale(const CVec3& vector)
		{
			mat[0][0] *= vector.x;
			mat[0][1] *= vector.y;
			mat[0][2] *= vector.z;

			mat[1][0] *= vector.x;
			mat[1][1] *= vector.y;
			mat[1][2] *= vector.z;

			mat[2][0] *= vector.x;
			mat[2][1] *= vector.y;
			mat[2][2] *= vector.z;

			mat[3][0] *= vector.x;
			mat[3][1] *= vector.y;
			mat[3][2] *= vector.z;
		}

		inline void makeTranslation(const CVec3& translation)
		{
			mat[0][0] = 1.0f; mat[0][1] = 0.0f; mat[0][2] = 0.0f; mat[0][3] = 0.0f;
			mat[1][0] = 0.0f; mat[1][1] = 1.0f; mat[1][2] = 0.0f; mat[1][3] = 0.0f;
			mat[2][0] = 0.0f; mat[2][1] = 0.0f; mat[2][2] = 1.0f; mat[2][3] = 0.0f;
			mat[3][0] = translation.x;
			mat[3][1] = translation.y;
			mat[3][2] = translation.z;
			mat[3][3] = 1.0f;
		}

		void multiply(const CMatrix& m);

		void multiply(const CMatrix& m1, const CMatrix& m2);

		inline void preTranslation(const CVec3& translation)
		{
			mat[3][0] +=  translation.x * mat[0][0]
						+ translation.y * mat[1][0]
						+ translation.z * mat[2][0];

			mat[3][1] +=  translation.x * mat[0][1]
						+ translation.y * mat[1][1]
						+ translation.z * mat[2][1];
			
			mat[3][2] +=  translation.x * mat[0][2]
						+ translation.y * mat[1][2]
						+ translation.z * mat[2][2];
			
			mat[3][3] +=  translation.x * mat[0][3]
						+ translation.y * mat[1][3]
						+ translation.z * mat[2][3];
		}

		inline void postTranslation(const CVec3& translation)
		{
			mat[0][0] += mat[0][3] * translation.x;
			mat[0][1] += mat[0][3] * translation.y;
			mat[0][2] += mat[0][3] * translation.z;

			mat[1][0] += mat[1][3] * translation.x;
			mat[1][1] += mat[1][3] * translation.y;
			mat[1][2] += mat[1][3] * translation.z;

			mat[2][0] += mat[2][3] * translation.x;
			mat[2][1] += mat[2][3] * translation.y;
			mat[2][2] += mat[2][3] * translation.z;

			mat[3][0] += mat[3][3] * translation.x;
			mat[3][1] += mat[3][3] * translation.y;
			mat[3][2] += mat[3][3] * translation.z;
		}

		inline void preMultiply(const CMatrix& m)
		{
			multiply(m);
		}

		inline void postMultiply(const CMatrix& m)
		{
			multiply(*this, m);
		}

		inline void preRotation(float angle, const CVec3& axis)
		{
			preRotationRadians(UtilDegToRad(angle), axis);
		}

		inline void preRotationRadians(float angle, const CVec3& axis)
		{
			CMatrix m;
			m.makeRotationRadians(angle, axis);
			preMultiply(m);
		}

		inline void postRotation(float angle, const CVec3& axis)
		{
			postRotationRadians(UtilDegToRad(angle), axis);
		}

		inline void postRotationRadians(float angle, const CVec3& axis)
		{
			CMatrix m;
			m.makeRotationRadians(angle, axis);
			postMultiply(m);
		}

		inline void getTranslation(CVec3& translation) const
		{
			translation.x=mat[3][0];
			translation.y=mat[3][1];
			translation.z=mat[3][2];
		}

		inline void setTranslation(const CVec3& translation)
		{
			mat[3][0] = translation.x;
			mat[3][1] = translation.y;
			mat[3][2] = translation.z;
		}

		inline void getTranslation(CMatrix& translation) const
		{
			translation.makeTranslation(CVec3(mat[3][0], mat[3][1], mat[3][2]));
		}

		inline void transformPoint(CVec3* point) const
		{
			transformPoints(point, point, 1);
		}

		inline void transformPoints(CVec3* points, unsigned int iCount) const
		{
			transformPoints(points, points, iCount);
		}

		inline void transformPoint(CVec4* point) const
		{
			transformPoints(point, point, 1);
		}

		inline void transformPoints(CVec4* points, unsigned int iCount) const
		{
			transformPoints(points, points, iCount);
		}

		void transformPoints(const CVec3* from, CVec3* to, unsigned int iCount) const;
		void transformPoints(const CVec4* from, CVec4* to, unsigned int iCount) const;

		inline void transformVector(CVec3* vec) const
		{
			transformVectors(vec, vec, 1);
		}

		inline void transformVectors(CVec3* vecs, unsigned int iCount) const
		{
			transformVectors(vecs, vecs, iCount);
		}

		inline void transformVector(CVec4* vec) const
		{
			transformVectors(vec, vec, 1);
		}

		inline void transformVectors(CVec4* vecs, unsigned int iCount) const
		{
			transformVectors(vecs, vecs, iCount);
		}

		void transformVectors(const CVec3* from, CVec3* to, unsigned int iCount) const;
		void transformVectors(const CVec4* from, CVec4* to, unsigned int iCount) const;

		void invert();

		void transpose();

		float determinant() const;

		// Columns
		inline void getColumn(unsigned int iCol, CVec3& v) const
		{
			v.x=mat[0][iCol];
			v.y=mat[1][iCol];
			v.z=mat[2][iCol];
		}

		inline void getColumn(unsigned int iCol, CVec4& v) const
		{
			v.x=mat[0][iCol];
			v.y=mat[1][iCol];
			v.z=mat[2][iCol];
			v.w=mat[3][iCol];
		}

		inline void setColumn(unsigned int iCol, const CVec3& v)
		{
			mat[0][iCol]=v.x;
			mat[1][iCol]=v.y;
			mat[2][iCol]=v.z;
		}

		inline void setColumn(unsigned int iCol, const CVec4& v)
		{
			mat[0][iCol]=v.x;
			mat[1][iCol]=v.y;
			mat[2][iCol]=v.z;
			mat[3][iCol]=v.w;
		}

		// Rows
		inline void getRow(unsigned int iRow, CVec3& v) const
		{
			v.x=mat[iRow][0];
			v.y=mat[iRow][1];
			v.z=mat[iRow][2];
		}

		inline void getRow(unsigned int iRow, CVec4& v) const
		{
			v.x=mat[iRow][0];
			v.y=mat[iRow][1];
			v.z=mat[iRow][2];
			v.w=mat[iRow][3];
		}

		inline void setRow(unsigned int iRow, const CVec3& v)
		{
			mat[iRow][0]=v.x;
			mat[iRow][1]=v.y;
			mat[iRow][2]=v.z;
		}

		inline void setRow(unsigned int iRow, const CVec4& v)
		{
			mat[iRow][0]=v.x;
			mat[iRow][1]=v.y;
			mat[iRow][2]=v.z;
			mat[iRow][3]=v.w;
		}

		void adjoint(const CMatrix& in, CMatrix& out);

};

//

#endif // __Matrix_4x4_Class__

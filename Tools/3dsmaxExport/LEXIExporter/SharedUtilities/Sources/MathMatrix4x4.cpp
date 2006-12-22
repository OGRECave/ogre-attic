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

#include "stdafx.h"
#include "MathMatrix4x4.h"

//

static float mZero[16] = {		0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f };

const CMatrix CMatrix::_zero(mZero);

//

static float mIdentity[16] = {	1.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 1.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f };

const CMatrix CMatrix::_identity(mIdentity);

//

inline float det3x3(	float a1,float a2,float a3,
						float b1,float b2,float b3,
						float c1,float c2,float c3) 
{ 
	return a1*(b2*c3-b3*c2)-b1*(a2*c3-a3*c2)+c1*(a2*b3-a3*b2);
}

//

void CMatrix::makeLookAt(const CVec3& eye, const CVec3& point, const CVec3& up)
{
	CVec3 f;
	f.subtract(eye, point);			// view vector  (maps to z)
	const float flen = f.length2();
	if(F_Min < flen)
		f.scale( 1.0f / sqrtf(flen) );
	
	CVec3 upprime = up;
	const float ulen = upprime.length2();
	if(F_Min < ulen)
		upprime.scale( 1.0f / sqrtf(ulen) );
	
	CVec3 s; 
	s.cross(upprime, f);			// s = up X f  (maps to x)
	const float slen = s.length2();
	if(F_Min < slen)
		s.scale( 1.0f / sqrtf(slen) );

	CVec3 u;
	u.cross(f, s);				// u = f X s;   (maps to y)
	// s and f are normalized and orthogonal, so u is
		
	// this matrix maps to the eye point, we want to map the geometry so we
	// need the inverse. since it's an orthonormal matrix by construction, 
	// we can simply transpose it.
	// [1 0 0 0] [[  s ]]
	// [0 1 0 0] [[  u ]]
	// [0 0 1 0] [[  f ]]
	// [0 0 0 1]
	
	mat[0][0] =  s.x; mat[0][1] =  u.x; mat[0][2] = f.x; mat[0][3] =  0.0f;
	mat[1][0] =  s.y; mat[1][1] =  u.y; mat[1][2] = f.y; mat[1][3] =  0.0f;
	mat[2][0] =  s.z; mat[2][1] =  u.z; mat[2][2] = f.z; mat[2][3] =  0.0f;

	// translate eye to origin
	mat[3][0] = -( mat[0][0] * eye.x + mat[1][0] * eye.y + mat[2][0] * eye.z );
	mat[3][1] = -( mat[0][1] * eye.x + mat[1][1] * eye.y + mat[2][1] * eye.z );
	mat[3][2] = -( mat[0][2] * eye.x + mat[1][2] * eye.y + mat[2][2] * eye.z );
	mat[3][3] = 1.0f - ( mat[0][3] * eye.x + mat[1][3] * eye.y + mat[2][3] * eye.z );
}

void CMatrix::makeLookAtDirection(const CVec3& eye, const CVec3& dir, const CVec3& up)
{
	CVec3 f;
	f.negate(dir);				 // view vector  (maps to z)
	const float flen = f.length2();
	if(F_Min < flen)
		f.scale( 1.0f / sqrtf(flen) );

	CVec3 upprime = up;
	const float ulen = upprime.length2();
	if(F_Min < ulen)
		upprime.scale( 1.0f / sqrtf(ulen) );

	CVec3 s; 
	s.cross(upprime, f);			// s = up X f  (maps to x)
	const float slen = s.length2();
	if(F_Min < slen)
		s.scale( 1.0f / sqrtf(slen) );

	CVec3 u;
	u.cross(f, s);				// u = f X s;   (maps to y)
	// s and f are normalized and orthogonal, so u is
		
	// this matrix maps to the eye point, we want to map the geometry so we
	// need the inverse. since it's an orthonormal matrix by construction, 
	// we can simply transpose it.
	// [1 0 0 0] [[  s ]]
	// [0 1 0 0] [[  u ]]
	// [0 0 1 0] [[  f ]]
	// [0 0 0 1]
	
	mat[0][0] =  s.x; mat[0][1] =  u.x; mat[0][2] = f.x; mat[0][3] =  0.0f;
	mat[1][0] =  s.y; mat[1][1] =  u.y; mat[1][2] = f.y; mat[1][3] =  0.0f;
	mat[2][0] =  s.z; mat[2][1] =  u.z; mat[2][2] = f.z; mat[2][3] =  0.0f;

	// translate eye to origin
	mat[3][0] = -( mat[0][0] * eye.x + mat[1][0] * eye.y + mat[2][0] * eye.z );
	mat[3][1] = -( mat[0][1] * eye.x + mat[1][1] * eye.y + mat[2][1] * eye.z );
	mat[3][2] = -( mat[0][2] * eye.x + mat[1][2] * eye.y + mat[2][2] * eye.z );
	mat[3][3] = 1.0f - ( mat[0][3] * eye.x + mat[1][3] * eye.y + mat[2][3] * eye.z );
}

//

void CMatrix::makePerspective(float left, float right, float bottom, float top, float znear, float zfar)
{
	float temp = 1.0f / ( right - left );

	mat[0][0]= ( 2.0f * znear ) * temp;
	mat[1][0]= 0.0f;	
	mat[2][0]= ( right + left ) * temp;	// for asymmetric views
	mat[3][0]= 0.0f;
	
	temp = 1.0f / ( top - bottom );

	mat[0][1]= 0.0f;
	mat[1][1]= ( 2.0f * znear ) * temp;
	mat[2][1]= ( top + bottom ) * temp;	// for asymmetric views
	mat[3][1]= 0.0f;
	
	temp = 1.0f / ( zfar - znear );

	mat[0][2]= 0.0f;
	mat[1][2]= 0.0f;
	mat[2][2]= -( zfar + znear ) * temp;
	mat[3][2]=	( -2.0f * zfar * znear ) * temp;

	mat[0][3]= 0.0f;
	mat[1][3]= 0.0f;
	mat[2][3]= -1.0f;
	mat[3][3]= 0.0f;
}

void CMatrix::makePerspectiveFOV(float hfov, float vfov, float aspect, float znear, float zfar)
{
	float hfovRad=UtilDegToRad(hfov);
	float vfovRad=UtilDegToRad(vfov);

	float l,r,t,b;
	float h,v;

	if( hfovRad < 0.0f ) 
	{
		h = (znear * tanf( vfovRad * 0.5f )) * aspect;
		h = atanf( h / znear ) * 2.0f;
		v = vfovRad;
	}
	else 
	{
		v = (znear * tanf( hfovRad * 0.5f )) / aspect;
		v = atanf( v / znear ) * 2.0f;
		h = hfovRad;
	}

	l  = znear * -tanf( h * 0.5f );
	r  = - l;
	b  = znear * -tanf( v * 0.5f );
	t  = - b;

	makePerspective( l, r, b, t, znear, zfar );
}

void CMatrix::makeOrthogonalPerspective(float left, float right, float bottom, float top, float znear, float zfar)
{
	float temp = 1.0f / ( right - left );

	mat[0][0]= 2.0f * temp;
	mat[1][0]= 0.0f;
	mat[2][0]= 0.0f;
	mat[3][0]= - ( right + left ) * temp;

	temp = 1.0f / ( top - bottom );
	
	mat[0][1]= 0.0f;
	mat[1][1]= 2.0f * temp;
	mat[2][1]= 0.0f;
	mat[3][1]= -( top + bottom ) * temp;

	temp = 1.0f / ( zfar - znear );
	
	mat[0][2]= 0.0f;
	mat[1][2]= 0.0f;
	mat[2][2]= -2.0f * temp;
	mat[3][2]= -( zfar + znear ) * temp;
	
	mat[0][3]= 0.0f;
	mat[1][3]= 0.0f;
	mat[2][3]= 0.0f;
	mat[3][3]= 1.0f;
}

//

void CMatrix::setRotationRadians(float angle, const CVec3& axis)
{
	if(fabs(angle) < 0.0000005f) 
	{
		mat[0][0] = mat[1][1] = mat[2][2] = 1.f;
		mat[0][1] = mat[0][2] = mat[1][0] = mat[1][2] = mat[2][0] = mat[2][1] = 0.f;
	} 
	else 
	{
		float sine=sinf(angle);
		float cosine=cosf(angle);

		CVec3 sineAxis;
		sineAxis.scale(sine, axis);

		float t = 1.0f - cosine;
		float tx = t * axis.x;
		mat[0][0] = tx * axis.x + cosine;
		mat[0][1] = tx * axis.y + sineAxis.z;
		mat[0][2] = tx * axis.z - sineAxis.y;

		float ty = t * axis.y;
		mat[1][0] = ty * axis.x - sineAxis.z;
		mat[1][1] = ty * axis.y + cosine;
		mat[1][2] = ty * axis.z + sineAxis.x;

		float tz = t * axis.z;
		mat[2][0] = tz * axis.x + sineAxis.y;
		mat[2][1] = tz * axis.y - sineAxis.x;
		mat[2][2] = tz * axis.z + cosine;
	}
}

void CMatrix::setRotationRadians(float anglex, float angley, float anglez)
{
	float sx, sy, sz, cx, cy, cz;
	float sxsy, cxsz, cxcz;

	if( anglex != 0.0f ) { sincos( anglex, sx, cx ); }
	else { sx = 0.0f; cx = 1.0f; sxsy = 0.0f; }

	if( angley != 0.0f ) { sincos( angley, sy, cy ); sxsy = sx * sy; }
	else { sy = 0.0f; cy = 1.0f; sxsy = 0.0f; }

	if( anglez != 0.0f ) { sincos( anglez, sz, cz ); cxsz = cx * sz; }
	else { sz = 0.0f; cz = 1.0f; cxsz = 0.0f; }

	cxcz = cx * cz;

	mat[0][0] = cy * cz;
	mat[0][1] = cy * sz;
	mat[0][2] = -sy;

	mat[1][0] = sxsy * cz - cxsz;
	mat[1][1] = sxsy * sz + cx * cz;
	mat[1][2] = sx * cy;

	mat[2][0] = cxcz * sy + sx * sz;
	mat[2][1] = cxsz * sy - sx * cz;
	mat[2][2] = cx * cy;
}

void CMatrix::getRotationRadians(float& anglex, float& angley, float& anglez)
{
	CVec3 temp;
	CVec3 row0( (float*)mat[0] );
	CVec3 row1( (float*)mat[1] );
	CVec3 row2( (float*)mat[2] );

	if( mat[3][3] != 1.0f )
	{
		float global_scale_inverse = 1.0f / mat[3][3];
		row0.scale( global_scale_inverse );
		row1.scale( global_scale_inverse );
		row2.scale( global_scale_inverse );
	}

	// possible scale or shearing must be removed...
	row0.normalize();

	// Compute XY shear factor and make 2nd row orthogonal to 1st. 
	float shearXY = row0.dot( row1 );
	row1.addScaled( -shearXY, row0 );

	// Now, normalize 2nd row. 
	row1.normalize();

	// Compute XZ and YZ shears, orthogonalize 3rd row. 
	float shearXZ = row0.dot( row2 );
	row2.addScaled( -shearXZ, row0 );
	float shearYZ = row1.dot( row2 );
	row2.addScaled( -shearYZ, row1 );

	// Next, normalize 3rd row.
	row2.normalize();

	// Check for a coordinate system flip.  If the determinant is -1, then negate the rows.
	temp.cross( row1, row2 );
	if( row0.dot( temp ) < 0.0f )
	{
		row0.negate();
		row1.negate();
		row2.negate();
	}

	angley = asin( -row0.z );
	if( cosf( angley ) != 0.0f ) 
	{
		anglex = atan2f( row1.z, row2.z );
		anglez = atan2f( row0.y, row0.x );
	} 
	else 
	{
		anglex = atan2f( row1.x, row1.y );
		anglez = 0.0f;
	}
}

//

void CMatrix::multiply(const CMatrix& m)
{
	/*register */unsigned int i;
	float m2[4][4];

	for( i=0; i<4; i++ ) {

		m2[0][i] = ( m.mat[0][0] * mat[0][i] +
						  m.mat[0][1] * mat[1][i] +
						  m.mat[0][2] * mat[2][i] +
						  m.mat[0][3] * mat[3][i] );

		m2[1][i] = ( m.mat[1][0] * mat[0][i] +
						  m.mat[1][1] * mat[1][i] +
						  m.mat[1][2] * mat[2][i] +
						  m.mat[1][3] * mat[3][i] );

		m2[2][i] = ( m.mat[2][0] * mat[0][i] +
						  m.mat[2][1] * mat[1][i] +
						  m.mat[2][2] * mat[2][i] +
						  m.mat[2][3] * mat[3][i] );

		m2[3][i] = ( m.mat[3][0] * mat[0][i] +
						  m.mat[3][1] * mat[1][i] +
						  m.mat[3][2] * mat[2][i] +
						  m.mat[3][3] * mat[3][i] );
	}

	memcpy(mat, m2, 16*sizeof(float));
}

void CMatrix::multiply(const CMatrix& m1, const CMatrix& m2)
{
	register unsigned int i;

	if( this != &m1 && this != &m2 ) {

		for( i=0; i<4; i++ ) {

			mat[0][i] = ( m1.mat[0][0] * m2.mat[0][i] +
						  m1.mat[0][1] * m2.mat[1][i] +
						  m1.mat[0][2] * m2.mat[2][i] +
						  m1.mat[0][3] * m2.mat[3][i] );

			mat[1][i] = ( m1.mat[1][0] * m2.mat[0][i] +
						  m1.mat[1][1] * m2.mat[1][i] +
						  m1.mat[1][2] * m2.mat[2][i] +
						  m1.mat[1][3] * m2.mat[3][i] );

			mat[2][i] = ( m1.mat[2][0] * m2.mat[0][i] +
						  m1.mat[2][1] * m2.mat[1][i] +
						  m1.mat[2][2] * m2.mat[2][i] +
						  m1.mat[2][3] * m2.mat[3][i] );

			mat[3][i] = ( m1.mat[3][0] * m2.mat[0][i] +
						  m1.mat[3][1] * m2.mat[1][i] +
						  m1.mat[3][2] * m2.mat[2][i] +
						  m1.mat[3][3] * m2.mat[3][i] );
		}

	}
	else {
		
		float m3[4][4];

		for( i=0; i<4; i++ ) {

			m3[0][i] = (	m1.mat[0][0] * m2.mat[0][i] +
							m1.mat[0][1] * m2.mat[1][i] +
							m1.mat[0][2] * m2.mat[2][i] +
							m1.mat[0][3] * m2.mat[3][i] );

			m3[1][i] = (	m1.mat[1][0] * m2.mat[0][i] +
							m1.mat[1][1] * m2.mat[1][i] +
							m1.mat[1][2] * m2.mat[2][i] +
							m1.mat[1][3] * m2.mat[3][i] );

			m3[2][i] = ( m1.mat[2][0] * m2.mat[0][i] +
						  	  m1.mat[2][1] * m2.mat[1][i] +
						  	  m1.mat[2][2] * m2.mat[2][i] +
						  	  m1.mat[2][3] * m2.mat[3][i] );

			m3[3][i] = ( m1.mat[3][0] * m2.mat[0][i] +
						  	  m1.mat[3][1] * m2.mat[1][i] +
						  	  m1.mat[3][2] * m2.mat[2][i] +
						  	  m1.mat[3][3] * m2.mat[3][i] );
		}

		memcpy(mat, m3, 16*sizeof(float));
	}
}

//

void CMatrix::transformPoints(const CVec3* from, CVec3* to, unsigned int iCount) const
{
	register float t0, t1, t2, w;

	for( unsigned int i = 0; i < iCount; i++ )
	{
		t0 = from[i].x;
		t1 = from[i].y;
		t2 = from[i].z;

		to[i].x = (t0 * mat[0][0] + t1 * mat[1][0] + t2 * mat[2][0] + mat[3][0]);
		to[i].y = (t0 * mat[0][1] + t1 * mat[1][1] + t2 * mat[2][1] + mat[3][1]);
		to[i].z = (t0 * mat[0][2] + t1 * mat[1][2] + t2 * mat[2][2] + mat[3][2]);
		w = (t0 * mat[0][3] + t1 * mat[1][3] + t2 * mat[2][3] + mat[3][3]);

		if( w != 1.0f ) {
			if( fabs( w ) < F_MinValue )
				w = F_MinValue;
			w = 1.0f / w;
			to[i].x *= w;
			to[i].y *= w;
			to[i].z *= w;
		}
	}
}

void CMatrix::transformPoints(const CVec4* from, CVec4* to, unsigned int iCount) const
{
	register float t0, t1, t2, t3;

	for( unsigned int i = 0; i < iCount; i++ )
	{
		t0 = from[i].x;
		t1 = from[i].y;
		t2 = from[i].z;
		t3 = from[i].w;

		to[i].x = (t0 * mat[0][0] + t1 * mat[1][0] + t2 * mat[2][0] + t3 * mat[3][0]);
		to[i].y = (t0 * mat[0][1] + t1 * mat[1][1] + t2 * mat[2][1] + t3 * mat[3][1]);
		to[i].z = (t0 * mat[0][2] + t1 * mat[1][2] + t2 * mat[2][2] + t3 * mat[3][2]);
		to[i].w = (t0 * mat[0][3] + t1 * mat[1][3] + t2 * mat[2][3] + t3 * mat[3][3]);
	}
}

void CMatrix::transformVectors(const CVec3* from, CVec3* to, unsigned int iCount) const
{
	/*register */float t0, t1, t2;

	for(unsigned int i = 0; i < iCount; i++, from++, to++)
	{
		t0 = from->x;
		t1 = from->y;
		t2 = from->z;

		to->x = (t0 * mat[0][0] + t1 * mat[1][0] + t2 * mat[2][0]);
		to->y = (t0 * mat[0][1] + t1 * mat[1][1] + t2 * mat[2][1]);
		to->z = (t0 * mat[0][2] + t1 * mat[1][2] + t2 * mat[2][2]);
	}
}

void CMatrix::transformVectors(const CVec4* from, CVec4* to, unsigned int iCount) const
{
	/*register */float t0, t1, t2;

	for(unsigned int i = 0; i < iCount; i++, from++, to++)
	{
		t0 = from->x;
		t1 = from->y;
		t2 = from->z;

		to->x = (t0 * mat[0][0] + t1 * mat[1][0] + t2 * mat[2][0]);
		to->y = (t0 * mat[0][1] + t1 * mat[1][1] + t2 * mat[2][1]);
		to->z = (t0 * mat[0][2] + t1 * mat[1][2] + t2 * mat[2][2]);
		to->w = from->w;
	}
}

//

void CMatrix::invert()
{
	float det, idet;
	CMatrix local_matrix;

	const CMatrix& matrix=*this;

	// calculate the adjoint matrix 
	adjoint( matrix, local_matrix );
	// calculate the 4x4 determinant if the determinant is zero, 
	// then the inverse matrix is not unique.
	det = matrix.determinant();
	
	// This test is only made to avoid crash
	//   it is not a test of matrix inversibility
	if( fabs( det ) < F_Min )
		throw;

	// scale the adjoint matrix to get the inverse 
	idet = 1.0f / det;
	for(unsigned int i=0; i<4; ++i)
		for(unsigned int j=0; j<4; ++j)
			mat[i][j] = local_matrix.mat[i][j] * idet;
}

//

void CMatrix::transpose()
{
	float m[4][4];

	m[0][0] = mat[0][0];
	m[0][1] = mat[1][0];
	m[0][2] = mat[2][0];
	m[0][3] = mat[3][0];

	m[1][0] = mat[0][1];
	m[1][1] = mat[1][1];
	m[1][2] = mat[2][1];
	m[1][3] = mat[3][1];

	m[2][0] = mat[0][2];
	m[2][1] = mat[1][2];
	m[2][2] = mat[2][2];
	m[2][3] = mat[3][2];

	m[3][0] = mat[0][3];
	m[3][1] = mat[1][3];
	m[3][2] = mat[2][3];
	m[3][3] = mat[3][3];

	memcpy(mat, m, 16*sizeof(float));
}

//

float CMatrix::determinant() const
{
	float ans;
	float a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4;

	/* assign to individual variable names to aid selecting */
	/* correct elements */

	a1 = mat[0][0]; b1 = mat[0][1]; 
	c1 = mat[0][2]; d1 = mat[0][3];

	a2 = mat[1][0]; b2 = mat[1][1]; 
	c2 = mat[1][2]; d2 = mat[1][3];

	a3 = mat[2][0]; b3 = mat[2][1]; 
	c3 = mat[2][2]; d3 = mat[2][3];

	a4 = mat[3][0]; b4 = mat[3][1]; 
	c4 = mat[3][2]; d4 = mat[3][3];

	ans = a1 * det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4)
		- b1 * det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4)
		+ c1 * det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4)
		- d1 * det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);

	return ans;
}

//

void CMatrix::adjoint(const CMatrix& in, CMatrix& out)
{
    float a1, a2, a3, a4, b1, b2, b3, b4;
    float c1, c2, c3, c4, d1, d2, d3, d4;

    /* assign to individual variable names to aid  */
    /* selecting correct values  */

	a1 = in.mat[0][0]; b1 = in.mat[0][1]; 
	c1 = in.mat[0][2]; d1 = in.mat[0][3];

	a2 = in.mat[1][0]; b2 = in.mat[1][1]; 
	c2 = in.mat[1][2]; d2 = in.mat[1][3];

	a3 = in.mat[2][0]; b3 = in.mat[2][1];
	c3 = in.mat[2][2]; d3 = in.mat[2][3];

	a4 = in.mat[3][0]; b4 = in.mat[3][1]; 
	c4 = in.mat[3][2]; d4 = in.mat[3][3];


    /* row column labeling reversed since we transpose rows & columns */

    out.mat[0][0] =   det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4);
    out.mat[1][0] = - det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4);
    out.mat[2][0] =   det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4);
    out.mat[3][0] = - det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);
        
    out.mat[0][1] = - det3x3( b1, b3, b4, c1, c3, c4, d1, d3, d4);
    out.mat[1][1] =   det3x3( a1, a3, a4, c1, c3, c4, d1, d3, d4);
    out.mat[2][1] = - det3x3( a1, a3, a4, b1, b3, b4, d1, d3, d4);
    out.mat[3][1] =   det3x3( a1, a3, a4, b1, b3, b4, c1, c3, c4);
        
    out.mat[0][2] =   det3x3( b1, b2, b4, c1, c2, c4, d1, d2, d4);
    out.mat[1][2] = - det3x3( a1, a2, a4, c1, c2, c4, d1, d2, d4);
    out.mat[2][2] =   det3x3( a1, a2, a4, b1, b2, b4, d1, d2, d4);
    out.mat[3][2] = - det3x3( a1, a2, a4, b1, b2, b4, c1, c2, c4);
        
    out.mat[0][3] = - det3x3( b1, b2, b3, c1, c2, c3, d1, d2, d3);
    out.mat[1][3] =   det3x3( a1, a2, a3, c1, c2, c3, d1, d2, d3);
    out.mat[2][3] = - det3x3( a1, a2, a3, b1, b2, b3, d1, d2, d3);
    out.mat[3][3] =   det3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3);
}

//


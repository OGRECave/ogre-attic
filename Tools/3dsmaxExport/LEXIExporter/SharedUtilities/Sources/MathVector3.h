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
//  Vector3 class
//
/////////////////////////////////////////////////////

#ifndef __CVector3_Class__
#define __CVector3_Class__

//

class CMatrix;

class CVec3 {

	friend class CMatrix;

	public:

		static const CVec3 _zero;
		static const CVec3 _x;
		static const CVec3 _y;
		static const CVec3 _z;

		// Constructors/Destructor
		inline CVec3()
		{
		}
		inline CVec3(const CVec3& v)
		{
			x=v.x;
			y=v.y;
			z=v.z;
		}
		inline CVec3(float fX, float fY, float fZ)
		{
			x=fX;
			y=fY;
			z=fZ;
		}
		inline CVec3(const float* v)
		{
			x=v[0];
			y=v[1];
			z=v[2];
		}

		// Direct access to vector
		float x, y, z;

		// Ccess operators
		inline float& operator [] (unsigned int iIndex)
		{
			return ((float*)&x)[iIndex];
		}
		inline const float& operator [] (unsigned int iIndex) const
		{
			return ((const float*)&x)[iIndex];
		}

		// Assignment
		inline CVec3& operator = (const CVec3& v)
		{
			x=v.x;
			y=v.y;
			z=v.z;

			return *this;
		}

		// Comparing
		inline bool operator == (const CVec3& v) const
		{
			return (x==v.x && y==v.y && z==v.z);
		}

		inline bool operator != (const CVec3& v) const
		{
			return (x!=v.x || y!=v.y || z!=v.z);
		}

		// Operators
		inline CVec3 operator - () const
		{
			return CVec3(-x, -y, -z);
		}

		inline CVec3 operator + (const CVec3& v) const
		{
			return CVec3(x+v.x, y+v.y, z+v.z);
		}

		inline CVec3 operator - (const CVec3& v) const
		{
			return CVec3(x-v.x, y-v.y, z-v.z);
		}

		inline CVec3 operator * (const CVec3& v) const
		{
			return CVec3(x*v.x, y*v.y, z*v.z);
		}

		inline CVec3 operator / (const CVec3& v) const
		{
			return CVec3(x/v.x, y/v.y, z/v.z);
		}

		inline void add(const CVec3& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
		}

		inline void add(const CVec3& v1, const CVec3& v2)
		{
			x = v1.x + v2.x;
			y = v1.y + v2.y;
			z = v1.z + v2.z;
		}

		inline CVec3& operator += (const CVec3& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}

		inline void subtract(const CVec3& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
		}

		inline void subtract(const CVec3& v1, const CVec3& v2)
		{
			x = v1.x - v2.x;
			y = v1.y - v2.y;
			z = v1.z - v2.z;
		}

		inline CVec3& operator -= (const CVec3& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}

		inline CVec3& operator *= (const CVec3& v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			return *this;
		}

		inline CVec3& operator /= (const CVec3& v)
		{
			x /= v.x;
			y /= v.y;
			z /= v.z;
			return *this;
		}

		inline CVec3 operator * (float r) const
		{
			return CVec3(r*x, r*y, r*z);
		}

		inline CVec3 operator / (float r) const
		{
			float ri = 1.0f / r;
			return CVec3(ri * x, ri * y, ri * z);
		}

		inline CVec3& operator *= (float r)
		{
			x *= r;
			y *= r;
			z *= r;
			return *this;
		}

		inline CVec3& operator /= (float r)
		{
			float ri = 1.0f / r;
			x *= ri;
			y *= ri;
			z *= ri;
			return *this;
		}

		// Functions
		inline float dot(const CVec3& v) const
		{
			return (x*v.x + y*v.y + z*v.z);
		}

		inline void cross(const CVec3& v)
		{
			const float a = (y * v.z) - (z * v.y);
			const float b = (z * v.x) - (x * v.z);
			const float c = (x * v.y) - (y * v.x);
			x = a;
			y = b;
			z = c;
		}

		inline void cross(const CVec3& v1, const CVec3& v2)
		{
			x = (v1.y * v2.z) - (v1.z * v2.y);
			y = (v1.z * v2.x) - (v1.x * v2.z);
			z = (v1.x * v2.y) - (v1.y * v2.x);
		}

		inline void negate()
		{
			x = -x;
			y = -y;
			z = -z;
		}

		inline void negate(const CVec3& vector)
		{
			x = -vector.x;
			y = -vector.y;
			z = -vector.z;
		}

		inline float length() const
		{
			return sqrtf(x*x + y*y + z*z);
		}

		inline float length2() const
		{
			return x*x + y*y + z*z;
		}

		inline void normalize()
		{
			float scalar = 1.0f / sqrtf(x*x + y*y + z*z);
			x *= scalar;
			y *= scalar;
			z *= scalar;
		}

		inline float distance(const CVec3& v) const
		{
			float fX = v.x-x;
			float fY = v.y-y;
			float fZ = v.z-z;

			return sqrtf(fX*fX + fY*fY + fZ*fZ);
		}

		inline float distance2(const CVec3& v) const
		{
			float fX = v.x - x;
			float fY = v.y - y;
			float fZ = v.z - z;

			return fX*fX + fY*fY + fZ*fZ;
		}

		inline void scale(float scalar, const CVec3& vector)
		{
			x = scalar * vector.x;
	        y = scalar * vector.y;
	        z = scalar * vector.z;
		}

		inline void scale(float scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
		}

	    inline void addScaled(const CVec3& v1, float scalar, const CVec3& v2)
		{
			x = v1.x + scalar * v2.x;
			y = v1.y + scalar * v2.y;
			z = v1.z + scalar * v2.z;
		}

	    inline void addScaled(float scalar, const CVec3& vector)
		{
			x += scalar * vector.x;
			y += scalar * vector.y;
			z += scalar * vector.z;
		}

		inline void lerp(const float& scalar, const CVec3& v1, const CVec3& v2)
		{ 
			x = v1.x + scalar * (v2.x - v1.x);
			y = v1.y + scalar * (v2.y - v1.y);
			z = v1.z + scalar * (v2.z - v1.z);
		}

		inline void reflect(const CVec3& v)
		{
			float a = ((x * v.x) + (y * v.y) + (z * v.z)) * 2.0f;

			x *= a;
			x -= v.x;

			y *= a;
			y -= v.y;

			z *= a;
			z -= v.z;
		}

		inline void clamp()
		{
			if(x < 0.0f) x = 0.0f;
			else if(x > 1.0f) x = 1.0f;
			if(y < 0.0f) y = 0.0f;
			else if(y > 1.0f) y = 1.0f;
			if(z < 0.0f) z = 0.0f;
			else if(z > 1.0f) z = 1.0f;
		}

		inline void clamp(float min, float max)
		{
			if(x < min) x = min;
			else if(x > max) x = max;
			if(y < min) y = min;
			else if(y > max) y = max;
			if(z < min) z = min;
			else if(z > max) z = max;
		}

		inline unsigned int largestAxis() const
		{
			const float* pVals = &x;
			unsigned int l = 0;
			if(pVals[1] > pVals[l]) l = 1;
			if(pVals[2] > pVals[l]) l = 2;
			return l;
		}

		inline void zero()
		{
			x = y = z = 0.0f;
		}

};

//

#endif // __CVector3_Class__

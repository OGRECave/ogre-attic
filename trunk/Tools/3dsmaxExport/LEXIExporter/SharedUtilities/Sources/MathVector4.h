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
//  Vector4 class
//
/////////////////////////////////////////////////////

#ifndef __CVector4_Class__
#define __CVector4_Class__

//

class CVec4 {

	friend class CMatrix;

	public:

		static const CVec4 _zero;
		static const CVec4 _x;
		static const CVec4 _y;
		static const CVec4 _z;
		static const CVec4 _w;

		// Constructors/Destructor
		inline CVec4()
		{
		}
		inline CVec4(const CVec4& v)
		{
			x=v.x;
			y=v.y;
			z=v.z;
			w=v.w;
		}
		inline CVec4(float fX, float fY, float fZ, float fW)
		{
			x=fX;
			y=fY;
			z=fZ;
			w=fW;
		}
		inline CVec4(const float* v)
		{
			x=v[0];
			y=v[1];
			z=v[2];
			w=v[3];
		}

		// Direct access to vector
		float x, y, z, w;

		// Assignment
		inline CVec4& operator = (const CVec4& v)
		{
			x=v.x;
			y=v.y;
			z=v.z;
			w=v.w;

			return *this;
		}

		// Comparing
		inline bool operator == (const CVec4& v) const
		{
			return (x==v.x && y==v.y && z==v.z && w==v.w);
		}

		inline bool operator != (const CVec4& v) const
		{
			return (x!=v.x || y!=v.y || z!=v.z || w!=v.w);
		}

		// Operators
		inline CVec4 operator - () const
		{
			return CVec4(-x, -y, -z, -w);
		}

		inline CVec4 operator + (const CVec4& v) const
		{
			return CVec4(x+v.x, y+v.y, z+v.z, w+v.w);
		}

		inline CVec4 operator - (const CVec4& v) const
		{
			return CVec4(x-v.x, y-v.y, z-v.z, w-v.w);
		}

		inline CVec4 operator * (const CVec4& v) const
		{
			return CVec4(x*v.x, y*v.y, z*v.z, w*v.w);
		}

		inline CVec4 operator / (const CVec4& v) const
		{
			return CVec4(x/v.x, y/v.y, z/v.z, w/v.w);
		}

		inline void add(const CVec4& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
		}

		inline void add(const CVec4& v1, const CVec4& v2)
		{
			x = v1.x + v2.x;
			y = v1.y + v2.y;
			z = v1.z + v2.z;
			w = v1.w + v2.w;
		}

		inline CVec4& operator += (const CVec4& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
			return *this;
		}

		inline void subtract(const CVec4& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
		}

		inline void subtract(const CVec4& v1, const CVec4& v2)
		{
			x = v1.x - v2.x;
			y = v1.y - v2.y;
			z = v1.z - v2.z;
			w = v1.w - v2.w;
		}

		inline CVec4& operator -= (const CVec4& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
			return *this;
		}

		inline CVec4& operator *= (const CVec4& v)
		{
			x *= v.x;
			y *= v.y;
			z *= v.z;
			w *= v.w;
			return *this;
		}

		inline CVec4& operator /= (const CVec4& v)
		{
			x /= v.x;
			y /= v.y;
			z /= v.z;
			w /= v.w;
			return *this;
		}

		inline CVec4 operator * (float r) const
		{
			return CVec4(r*x, r*y, r*z, r*w);
		}

		inline CVec4 operator / (float r) const
		{
			float ri = 1.0f/r;
			return CVec4(ri*x, ri*y, ri*z, ri*w);
		}

		inline CVec4& operator *= (float r)
		{
			x *= r;
			y *= r;
			z *= r;
			w *= r;
			return *this;
		}

		inline CVec4& operator /= (float r)
		{
			float ri = 1.0f/r;
			x *= ri;
			y *= ri;
			z *= ri;
			w *= ri;
			return *this;
		}

		inline void scale(float scalar, const CVec4& v)
		{
			x = scalar * v.x;
			y = scalar * v.y;
			z = scalar * v.z;
			w = scalar * v.w;
		}

		inline void scale(float scalar)
		{
			x *= scalar;
	        y *= scalar;
	        z *= scalar;
	        w *= scalar;
		}

		inline void lerp(const float& scalar, const CVec4& v1, const CVec4& v2)
		{ 
			x = v1.x + scalar * (v2.x - v1.x);
			y = v1.y + scalar * (v2.y - v1.y);
			z = v1.z + scalar * (v2.z - v1.z);
			w = v1.w + scalar * (v2.w - v1.w);
		}

		inline void lerp3(const float& scalar, const CVec4& v1, const CVec4& v2)
		{ 
			x = v1.x + scalar * (v2.x - v1.x);
			y = v1.y + scalar * (v2.y - v1.y);
			z = v1.z + scalar * (v2.z - v1.z);
			w = v1.w;
		}

		inline void normalize3()
		{
			float st = 1.0f / sqrtf(x*x + y*y + z*z);
			x *= st;
	        y *= st;
	        z *= st;
		}

		inline void normalize()
		{
			float st = 1.0f / sqrtf(x*x + y*y + z*z);
			x *= st;
	        y *= st;
	        z *= st;
	        w *= st;
		}

		inline void clamp()
		{
			if(x < 0.0f) x = 0.0f;
			else if(x > 1.0f) x = 1.0f;
			if(y < 0.0f) y = 0.0f;
			else if(y > 1.0f) y = 1.0f;
			if(z < 0.0f) z = 0.0f;
			else if(z > 1.0f) z = 1.0f;
			if(w < 0.0f) w = 0.0f;
			else if(w > 1.0f) w = 1.0f;
		}

		inline void clamp(float min, float max)
		{
			if(x < min) x = min;
			else if(x > max) x = max;
			if(y < min) y = min;
			else if(y > max) y = max;
			if(z < min) z = min;
			else if(z > max) z = max;
			if(w < min) w = min;
			else if(w > max) w = max;
		}

		inline void zero()
		{
			x = y = z = w = 0.0f;
		}

		void fromMatrix(const CMatrix& m);

};

//

#endif // __CVector4_Class__

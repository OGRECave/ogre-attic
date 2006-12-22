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

//@£$sdk "Vector2 Class"

#ifndef __Accuro_Vector2__
#define __Accuro_Vector2__

#pragma once

//

class CVec2 {

	friend class CMatrix;

	public:

		static const CVec2 _zero;
		static const CVec2 _x;
		static const CVec2 _y;

		// Constructors/Destructor
		inline CVec2()
		{
		}
		inline CVec2(const CVec2& v)
		{
			x=v.x;
			y=v.y;
		}
		inline CVec2(float fX, float fY)
		{
			x=fX;
			y=fY;
		}
		inline CVec2(const float* v)
		{
			x=v[0];
			y=v[1];
		}

		// Direct access to vector
		float x, y;

		// Assignment
		inline CVec2& operator = (const CVec2& v)
		{
			x=v.x;
			y=v.y;

			return *this;
		}

		// Comparing
		inline bool operator == (const CVec2& v) const
		{
			return (x==v.x && y==v.y);
		}

		inline bool operator != (const CVec2& v) const
		{
			return (x!=v.x || y!=v.y);
		}

		// Operators
		inline CVec2 CVec2::operator - () const
		{
			return CVec2(-x, -y);
		}

		inline CVec2 CVec2::operator + (const CVec2& v) const
		{
			return CVec2(x+v.x, y+v.y);
		}

		inline CVec2 CVec2::operator - (const CVec2& v) const
		{
			return CVec2(x-v.x, y-v.y);
		}

		inline CVec2 CVec2::operator * (const CVec2& v) const
		{
			return CVec2(x*v.x, y*v.y);
		}

		inline CVec2 CVec2::operator / (const CVec2& v) const
		{
			return CVec2(x/v.x, y/v.y);
		}

		inline void add(const CVec2& v)
		{
			x += v.x;
			y += v.y;
		}

		inline void add(const CVec2& v1, const CVec2& v2)
		{
			x = v1.x + v2.x;
			y = v1.y + v2.y;
		}

		inline CVec2& CVec2::operator += (const CVec2& v)
		{
			x += v.x;
			y += v.y;
			return *this;
		}

		inline void subtract(const CVec2& v)
		{
			x += v.x;
			y += v.y;
		}

		inline void subtract(const CVec2& v1, const CVec2& v2)
		{
			x = v1.x - v2.x;
			y = v1.y - v2.y;
		}

		inline CVec2& CVec2::operator -= (const CVec2& v)
		{
			x -= v.x;
			y -= v.y;
			return *this;
		}

		inline CVec2& CVec2::operator *= (const CVec2& v)
		{
			x *= v.x;
			y *= v.y;
			return *this;
		}

		inline CVec2& CVec2::operator /= (const CVec2& v)
		{
			x /= v.x;
			y /= v.y;
			return *this;
		}

		inline CVec2 CVec2::operator * (float r) const
		{
			return CVec2(r*x, r*y);
		}

		inline void multiply(const CVec2& v1, const CVec2& v2)
		{
			x = v1.x * v2.x;
	        y = v1.y * v2.y;
		}

		inline void multiply(const CVec2& v)
		{
			x *= v.x;
	        y *= v.y;
		}

		inline void negate(const CVec2& v)
		{
			x = -v.x;
			y = -v.y;
		}

		inline void negate()
		{
			x = -x;
			y = -y;
		}

		inline float dot(const CVec2& v) const
		{
			return x * v.x + y * v.y;
		}

		inline void scale(float scalar, const CVec2& v)
		{
			x = scalar * v.x;
	        y = scalar * v.y;
		}

		inline void scale(float scalar)
		{
			x *= scalar;
	        y *= scalar;
		}

		inline void addScaled(const CVec2& v1, float scalar, const CVec2& v2)
		{
			x = v1.x + scalar * v2.x;
	        y = v1.y + scalar * v2.y;
		}

		inline void addScaled(float scalar, const CVec2& v)
		{
			x += scalar * v.x;
	        y += scalar * v.y;
		}

		inline void combine(float scalar1, const CVec2& v1, float scalar2, const CVec2& v2)
		{
			x = scalar1 * v1.x + scalar2 * v2.x;
	        y = scalar1 * v1.y + scalar2 * v2.y;
		}

		inline void normalize()
		{
			float t = length();
	        scale(1.0f / t);
		}

		inline void perpendicular(const CVec2& v)
		{
			float t = v.x;
			x = -v.y;
			y = t;
		}

		inline void perpendicular()
		{
			float t = x;
			x = -y;
			y = t;
		}

		inline float length2() const
		{
			return F_Square(x) + F_Square(y);
		}

		inline float length() const
		{
			return sqrtf(length2());
		}

		inline float distance2(const CVec2& v) const
		{
			return (F_Square(x - v.x) + F_Square(y - v.y));
		}

		inline float distance(const CVec2& v) const
		{
			return sqrtf(distance2(v));
		}

		inline void lerp(const float& scalar, const CVec2& v1, const CVec2& v2)
		{ 
			x = v1.x + scalar * (v2.x - v1.x);
			y = v1.y + scalar * (v2.y - v1.y);
		}

		inline float angle(const CVec2& v) const
		{
			return acosf(dot(v) / (length() * v.length()));
		}

		inline CVec2 CVec2::operator / (float r) const
		{
			float ri = 1.0f/r;
			return CVec2(ri*x, ri*y);
		}

		inline CVec2& CVec2::operator *= (float r)
		{
			x *= r;
			y *= r;
			return *this;
		}

		inline CVec2& CVec2::operator /= (float r)
		{
			float ri = 1.0f/r;
			x *= ri;
			y *= ri;
			return *this;
		}

		inline void clamp()
		{
			if(x < 0.0f) x = 0.0f;
			else if(x > 1.0f) x = 1.0f;
			if(y < 0.0f) y = 0.0f;
			else if(y > 1.0f) y = 1.0f;
		}

		inline void clamp(float min, float max)
		{
			if(x < min) x = min;
			else if(x > max) x = max;
			if(y < min) y = min;
			else if(y > max) y = max;
		}

		inline void zero()
		{
			x = y = 0.0f;
		}

};

//

#endif // __Accuro_Vector2__

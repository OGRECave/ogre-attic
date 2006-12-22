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
//  Math Defines
//
/////////////////////////////////////////////////////

#ifndef __Math_Defines__
#define __Math_Defines__

//

#define TV_PI					(3.1415926535897932384626433832795f)
#define TV_HalfPI				(TV_PI * 0.5f)
#define TV_PI2					(TV_PI * 2.0f)

#define UtilDegToRad(angle)		(0.017453293f * angle)
#define UtilRadToDeg(angle)		(57.295779513f * angle)

#define F_Min					FLT_MIN
#define F_Max					FLT_MAX

#define F_MinValue				(0.0000005f)

#define F_Square(v)				((v) * (v))

//

inline void sincos(const float& angle, float& s, float& c)
{
	s = sinf(angle);
	c = cosf(angle);
}

//

#endif // __Math_Defines__

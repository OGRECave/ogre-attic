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

#include "stdafx.h"
#include "MathVector4.h"

//

const CVec4 CVec4::_zero(0.0f, 0.0f, 0.0f, 0.0f);
const CVec4 CVec4::_x(1.0f, 0.0f, 0.0f, 0.0f);
const CVec4 CVec4::_y(0.0f, 1.0f, 0.0f, 0.0f);
const CVec4 CVec4::_z(0.0f, 0.0f, 1.0f, 0.0f);
const CVec4 CVec4::_w(0.0f, 0.0f, 0.0f, 1.0f);

//

void CVec4::fromMatrix(const CMatrix& m)
{
	float s;
	float trace = m.mat[0][0] + m.mat[1][1] + m.mat[2][2];

	if( trace >= 0.0f ) {

		s = sqrtf( trace + 1.0f );

		w = s * 0.5f;

		s = 0.5f / s;

		x = ( m.mat[2][1] - m.mat[1][2] ) * s;
		y = ( m.mat[0][2] - m.mat[2][0] ) * s;
		z = ( m.mat[1][0] - m.mat[0][1] ) * s;

	} 
	else {

		unsigned h = 0;

		if( m.mat[1][1] > m.mat[0][0] ) h = 1;
		if( m.mat[2][2] > m.mat[h][h] ) h = 2;

		switch( h ) {

		  case 0:
			s    = sqrtf( ( m.mat[0][0] - ( m.mat[1][1] + m.mat[2][2] ) ) + m.mat[3][3] );
			x = s * 0.5f;
			s    = 0.5f / s;
			y = ( m.mat[0][1] + m.mat[1][0] ) * s;
			z = ( m.mat[2][0] + m.mat[0][2] ) * s;
			w = ( m.mat[2][1] - m.mat[1][2] ) * s;
			break;

		  case 1:
			s    = sqrtf( ( m.mat[1][1] - ( m.mat[2][2] + m.mat[0][0] ) ) + m.mat[3][3] );
			y = s * 0.5f;
			s    = 0.5f  / s;
			z = ( m.mat[1][2] + m.mat[2][1] ) * s;
			x = ( m.mat[0][1] + m.mat[1][0] ) * s;
			w = ( m.mat[0][2] - m.mat[2][0] ) * s;
			break;

		  case 2:
			s    = sqrtf( ( m.mat[2][2] - ( m.mat[0][0] + m.mat[1][1] ) ) + m.mat[3][3] );
			z = s * 0.5f;
			s    = 0.5f / s;
			x = ( m.mat[2][0] + m.mat[0][2] ) * s;
			y = ( m.mat[1][2] + m.mat[2][1] ) * s;
			w = ( m.mat[1][0] - m.mat[0][1] ) * s;
			break;
		}
	}
}

//


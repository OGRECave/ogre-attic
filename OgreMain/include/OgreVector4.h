/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#ifndef __Vector4_H__
#define __Vector4_H__

#include "OgrePrerequisites.h"
#include "OgreVector3.h"

namespace Ogre
{

    /** 4-dimensional homogenous vector.
    */
    class _OgreExport Vector4
    {
    public:
        union {
            struct {
                Real x, y, z, w;        
            };
            Real val[4];
        };

    public:
        inline Vector4()
        {
        }

        inline Vector4( Real fX, Real fY, Real fZ, Real fW ) 
            : x( fX ), y( fY ), z( fZ ), w( fW)
        {
        }

        inline Vector4( Real afCoordinate[4] )
            : x( afCoordinate[0] ),
              y( afCoordinate[1] ),
              z( afCoordinate[2] ), 
              w (afCoordinate[3] )
        {
        }

        inline Vector4( int afCoordinate[4] )
        {
            x = (Real)afCoordinate[0];
            y = (Real)afCoordinate[1];
            z = (Real)afCoordinate[2];
            w = (Real)afCoordinate[3];
        }

        inline Vector4( const Real* const r )
            : x( r[0] ), y( r[1] ), z( r[2] ), w( r[3] )
        {
        }

        inline Vector4( const Vector4& rkVector )
            : x( rkVector.x ), y( rkVector.y ), z( rkVector.z ), w (rkVector.w)
        {
        }

        inline Real operator [] ( unsigned i ) const
        {
            assert( i < 4 );

            return *(&x+i);
        }

		inline Real& operator [] ( unsigned i )
        {
            assert( i < 4 );

            return *(&x+i);
        }

        /** Assigns the value of the other vector.
            @param
                rkVector The other vector
        */
        inline Vector4& operator = ( const Vector4& rkVector )
        {
            x = rkVector.x;
            y = rkVector.y;
            z = rkVector.z;            
            w = rkVector.w;            

            return *this;
        }

        inline bool operator == ( const Vector4& rkVector ) const
        {
            return ( x == rkVector.x && 
                y == rkVector.y && 
                z == rkVector.z &&
                w == rkVector.w );
        }

        inline bool operator != ( const Vector4& rkVector ) const
        {
            return ( x != rkVector.x || 
                y != rkVector.y || 
                z != rkVector.z ||
                w != rkVector.w );
        }

        inline Vector4& operator = (const Vector3& rhs)
        {
            x = rhs.x;
            y = rhs.y;
            z = rhs.z;
            w = 1.0f;
            return *this;
        }



        /** Calculates the dot (scalar) product of this vector with another.
            @param
                vec Vector with which to calculate the dot product (together
                with this one).
            @returns
                A float representing the dot product value.
        */
        inline Real dotProduct(const Vector4& vec) const
        {
            return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
        }
        /** Function for writing to a stream.
        */
        inline _OgreExport friend std::ostream& operator <<
            ( std::ostream& o, const Vector4& v )
        {
            o << "Vector4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
            return o;
        }
    };

}
#endif

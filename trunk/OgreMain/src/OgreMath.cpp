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
#include "OgreMath.h"
#include "asm_math.h"

namespace Ogre
{
    template<> Math* Singleton<Math>::ms_Singleton = 0;

    const Real Math::POS_INFINITY = std::numeric_limits<Real>::infinity();
    const Real Math::NEG_INFINITY = -std::numeric_limits<Real>::infinity();
    const Real Math::PI = Real( 4.0 * atan( 1.0 ) );
    const Real Math::TWO_PI = Real( 2.0 * PI );
    const Real Math::HALF_PI = Real( 0.5 * PI );

    int Math::mTrigTableSize;

    Real  Math::mTrigTableFactor;
    Real *Math::mSinTable = NULL;
    Real *Math::mTanTable = NULL;

    //-----------------------------------------------------------------------
    Math::Math( unsigned int trigTableSize )
    {
        mTrigTableSize = trigTableSize;
        mTrigTableFactor = mTrigTableSize / Math::TWO_PI;

        mSinTable = new Real[mTrigTableSize];
        mTanTable = new Real[mTrigTableSize];

        buildTrigTables();

        // Init random number generator
        srand( (unsigned)time(0) );
    }

    //-----------------------------------------------------------------------
    Math::~Math()
    {
        delete [] mSinTable;
        delete [] mTanTable;
    }

    //-----------------------------------------------------------------------
    void Math::buildTrigTables(void)
    {
        // Build trig lookup tables
        // Could get away with building only PI sized Sin table but simpler this 
        // way. Who cares, it'll ony use an extra 8k of memory anyway and I like 
        // simplicity.
        Real angle;
        for (int i = 0; i < mTrigTableSize; ++i)
        {
            angle = Math::TWO_PI * i / mTrigTableSize;
            mSinTable[i] = sin(angle);
            mTanTable[i] = tan(angle);
        }
    }
    //-----------------------------------------------------------------------
    int Math::IAbs (int iValue)
    {
        return ( iValue >= 0 ? iValue : -iValue );
    }
    //-----------------------------------------------------------------------
    int Math::ICeil (float fValue)
    {
        return int(ceil(fValue));
    }
    //-----------------------------------------------------------------------
    int Math::IFloor (float fValue)
    {
        return int(floor(fValue));
    }
    //-----------------------------------------------------------------------
    int Math::ISign (int iValue)
    {
        return ( iValue > 0 ? +1 : ( iValue < 0 ? -1 : 0 ) );
    }
    //-----------------------------------------------------------------------
    Real Math::Abs (Real fValue)
    {
        return Real(fabs(fValue));
    }
    //-----------------------------------------------------------------------
    Real Math::ACos (Real fValue)
    {
        if ( -1.0 < fValue )
        {
            if ( fValue < 1.0 )
                return Real(asm_arccos(fValue));
            else
                return 0.0;
        }
        else
        {
            return PI;
        }
    }
    //-----------------------------------------------------------------------
    Real Math::ASin (Real fValue)
    {
        if ( -1.0 < fValue )
        {
            if ( fValue < 1.0 )
                return Real(asm_arcsin(fValue));
            else
                return -HALF_PI;
        }
        else
        {
            return HALF_PI;
        }
    }
    //-----------------------------------------------------------------------
    Real Math::ATan (Real fValue)
    {
        return Real(asm_arctan(fValue));
    }
    //-----------------------------------------------------------------------
    Real Math::ATan2 (Real fY, Real fX)
    {
        return Real(atan2(fY,fX));
    }
    //-----------------------------------------------------------------------
    Real Math::Ceil (Real fValue)
    {
        return Real(ceil(fValue));
    }
    //-----------------------------------------------------------------------
    Real Math::Cos (Real fValue, bool useTables)
    {
        if (useTables)
        {
            // Convert to sin equivalent
            fValue += Math::HALF_PI;

            return Sin(fValue, useTables);

        }
        else
        {
            return Real(asm_cos(fValue));
        }
    }
    //-----------------------------------------------------------------------
    Real Math::Exp (Real fValue)
    {
        return Real(exp(fValue));
    }
    //-----------------------------------------------------------------------
    Real Math::Floor (Real fValue)
    {
        return Real(floor(fValue));
    }
    //-----------------------------------------------------------------------
    Real Math::Log (Real fValue)
    {
        return Real(asm_ln(fValue));
    }
    //-----------------------------------------------------------------------
    Real Math::Pow (Real fBase, Real fExponent)
    {
        return Real(pow(fBase,fExponent));
    }
    //-----------------------------------------------------------------------
    Real Math::Sign (Real fValue)
    {
        if ( fValue > 0.0 )
            return 1.0;

        if ( fValue < 0.0 )
            return -1.0;

        return 0.0;
    }
    //-----------------------------------------------------------------------
    Real Math::Sin (Real fValue, bool useTables)
    {
        if (useTables)
        {
            // Convert range to index values, wrap if required
            int idx;
            if (fValue >= 0)
            {
                idx = int(fValue * mTrigTableFactor) % mTrigTableSize;
            }
            else
            {
                idx = mTrigTableSize - (int(-fValue * mTrigTableFactor) % mTrigTableSize) - 1;
            }

            return mSinTable[idx];
        }
        else
        {
            return Real(asm_sin(fValue));
        }
    }
    //-----------------------------------------------------------------------
    Real Math::Sqr (Real fValue)
    {
        return fValue*fValue;
    }
    //-----------------------------------------------------------------------
    Real Math::Sqrt (Real fValue)
    {
        return Real(asm_sqrt(fValue));
    }
    //-----------------------------------------------------------------------
    Real Math::InvSqrt (Real fValue)
    {
        return Real(asm_rsq(fValue));
    }
    //-----------------------------------------------------------------------
    Real Math::UnitRandom ()
    {
        return asm_rand() / asm_rand_max();
    }
    
    //-----------------------------------------------------------------------
    Real Math::RangeRandom (Real fLow, Real fHigh)
    {
        return (fHigh-fLow)*UnitRandom() + fLow;
    }

    //-----------------------------------------------------------------------
    Real Math::SymmetricRandom ()
    {
		return 2.0f * UnitRandom() - 1.0f;
    }

    //-----------------------------------------------------------------------
    Real Math::DegreesToRadians(Real degrees)
    {
        static Real fDeg2Rad = PI / Real(180.0);

        return degrees * fDeg2Rad;
    }

    //-----------------------------------------------------------------------
    Real Math::RadiansToDegrees(Real radians)
    {
        static Real fRad2Deg = Real(180.0) / PI;

        return radians * fRad2Deg;
    }

    //-----------------------------------------------------------------------
    bool Math::pointInTri2D( Real px, Real py, Real ax, Real ay, Real bx, Real by, Real cx, Real cy )
    {
        Real v1x, v2x, v1y, v2y;
        bool bClockwise;

        v1x = bx - ax;
        v1y = by - ay;

        v2x = px - bx;
        v2y = py - by;

        // For the sake of readability
        #define Clockwise ( v1x * v2y - v1y * v2x >= 0.0 )

        bClockwise = Clockwise;

        v1x = cx - bx;
        v1y = cy - by;

        v2x = px - cx;
        v2y = py - cy;

        if( Clockwise != bClockwise )
            return false;

        v1x = ax - cx;
        v1y = ay - cy;

        v2x = px - ax;
        v2y = py - ay;

        if( Clockwise != bClockwise )
            return false;

        return true;

        // Clean up the #defines
        #undef Clockwise
    }

    //-----------------------------------------------------------------------
    Real Math::Tan(Real radians, bool useTables)
    {
        if (useTables)
        {
            // Convert range to index values, wrap if required
            int idx = int(radians *= mTrigTableFactor) % mTrigTableSize;
            return mTanTable[idx];
        }
        else
        {
            return Real(asm_tan(radians));
        }
    }

    //-----------------------------------------------------------------------
    bool Math::RealEqual( Real a, Real b, Real tolerance )
    {
        if ((b < (a + tolerance)) && (b > (a - tolerance)))
            return true;
        else
            return false;
    }

    //-----------------------------------------------------------------------
    Math& Math::getSingleton(void)
    {
        return Singleton<Math>::getSingleton();
    }

}

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
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#ifndef __Math_H__
#define __Math_H__

#include "OgrePrerequisites.h"
#include "OgreSingleton.h"

namespace Ogre
{
    /** Class to provide access to common mathematical functions.
        @remarks
            Most of the maths functions are aliased versions of the C runtime
            library functions. They are aliased here to provide future
            optimisation opportunities, either from faster RTLs or custom
            math approximations.
        @note
            <br>This is based on MgcMath.h from
            <a href="http://www.magic-software.com">Wild Magic</a>.
    */
    class _OgreExport Math : public Singleton<Math>
    {
    protected:
        /// Size of the trig tables as determined by constructor.
        static int mTrigTableSize;

        /// Radian -> index factor value ( mTrigTableSize / 2 * PI )
        static Real mTrigTableFactor;
        static Real* mSinTable;
        static Real* mTanTable;

        /** Private function to build trig tables.
        */
        void buildTrigTables();
    public:
        /** Default constructor.
            @param
                trigTableSize Optional parameter to set the size of the
                tables used to implement Sin, Cos, Tan
        */
        Math(unsigned int trigTableSize = 4096);

        /** Default destructor.
        */
        ~Math();

        static int IAbs (int iValue);
        static int ICeil (float fValue);
        static int IFloor (float fValue);
        static int ISign (int iValue);

        static Real Abs (Real fValue);
        static Real ACos (Real fValue);
        static Real ASin (Real fValue);
        static Real ATan (Real fValue);
        static Real ATan2 (Real fY, Real fX);
        static Real Ceil (Real fValue);

        /** Cosine function.
            @param
                fValue Angle in radians
            @param
                useTables If true, uses lookup tables rather than
                calculation - faster but less accurate.
        */
        static Real Cos (Real fValue, bool useTables = false);

        static Real Exp (Real fValue);

        static Real Floor (Real fValue);

        static Real Log (Real fValue);

        static Real Pow (Real kBase, Real kExponent);

        static Real Sign (Real fValue);

        /** Sine function.
            @param
                fValue Angle in radians
            @param
                useTables If true, uses lookup tables rather than
                calculation - faster but less accurate.
        */
        static Real Sin (Real fValue, bool useTables = false);

        static Real Sqr (Real fValue);

        static Real Sqrt (Real fValue);

        /** Inverse square root i.e. 1 / Sqrt(x), good for vector
            normalisation.
        */
        static Real InvSqrt(Real fValue);

        static Real UnitRandom ();  // in [0,1]

        static Real RangeRandom (Real fLow, Real fHigh);  // in [fLow,fHigh]

        static Real SymmetricRandom ();  // in [-1,1]

        /** Tangent function.
            @param
                fValue Angle in radians
            @param
                useTables If true, uses lookup tables rather than
                calculation - faster but less accurate.
        */
        static Real Tan (Real fValue, bool useTables = false);

        static Real DegreesToRadians(Real degrees);
        static Real RadiansToDegrees(Real radians);

        /** Checks wether a given point is inside a triangle, in a
            2-dimensional (Cartesian) space.
            @remarks
                The vertices of the triangle must be given in either
                trigonometrical (anticlockwise) or inverse trigonometrical
                (clockwise) order.
            @param
                px The X-coordinate of the point.
            @param
                py The Y-coordinate of the point.
            @param
                ax The X-coordinate of the triangle's first vertex.
            @param
                ay The Y-coordinate of the triangle's first vertex.
            @param
                bx The X-coordinate of the triangle's second vertex.
            @param
                by The Y-coordinate of the triangle's second vertex.
            @param
                cx The X-coordinate of the triangle's third vertex.
            @param
                cy The Y-coordinate of the triangle's third vertex.
            @returns
                If the point resides in the triangle, <b>true</b> is
                returned.
            @par
                If the point is outside the triangle, <b>false</b> is
                returned.
        */
        static bool pointInTri2D( Real px, Real pz, Real ax, Real az, Real bx, Real bz, Real cx, Real cz );

        /** Compare 2 reals, using tolerance for inaccuracies.
        */
        static bool RealEqual(Real a, Real b,
            Real tolerance = std::numeric_limits<Real>::epsilon());

        static const Real POS_INFINITY;
        static const Real NEG_INFINITY;
        static const Real PI;
        static const Real TWO_PI;
        static const Real HALF_PI;

        /** Override standard Singleton retrieval.
            @remarks
                Why do we do this? Well, it's because the Singleton
                implementation is in a .h file, which means it gets compiled
                into anybody who includes it. This is needed for the
                Singleton template to work, but we actually only want it
                compiled into the implementation of the class based on the
                Singleton, not all of them. If we don't change this, we get
                link errors when trying to use the Singleton-based class from
                an outside dll.
            @par
                This method just delegates to the template version anyway,
                but the implementation stays in this single compilation unit,
                preventing link errors.
        */
        static Math& getSingleton(void);
    };
}
#endif

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2002 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreRotationalSpline.h"



namespace Ogre {

    //---------------------------------------------------------------------
    RotationalSpline::RotationalSpline()
    {
    }
    //---------------------------------------------------------------------
    RotationalSpline::~RotationalSpline()
    {
    }
    //---------------------------------------------------------------------
    void RotationalSpline::addPoint(const Quaternion& p)
    {
        mPoints.push_back(p);
        if (mAutoCalc)
        {
            recalcTangents();
        }
    }
    //---------------------------------------------------------------------
    Quaternion RotationalSpline::interpolate(Real t)
    {
        // Work out which segment this is in
        Real fSeg = t * mPoints.size();
        unsigned int segIdx = (unsigned int)fSeg;
        // Apportion t 
        t = fSeg - segIdx;

        return interpolate(segIdx, t);

    }
    //---------------------------------------------------------------------
    Quaternion RotationalSpline::interpolate(unsigned int fromIndex, Real t)
    {
        // Bounds check
        assert (fromIndex >= 0 && (fromIndex + 1) < mPoints.size() &&
            "fromIndex must be >= 0 and there must be at least one point after it");

        // Fast special cases
        if (t == 0.0f)
        {
            return mPoints[fromIndex];
        }
        else if(t == 1.0f)
        {
            return mPoints[fromIndex + 1];
        }

        // Real interpolation
        // Use squad using tangents we've already set up
        Quaternion &p = mPoints[fromIndex];
        Quaternion &q = mPoints[fromIndex+1];
        Quaternion &a = mTangents[fromIndex];
        Quaternion &b = mTangents[fromIndex+1];

        return Quaternion::Squad(t, p, a, b, q);

    }
    //---------------------------------------------------------------------
    void RotationalSpline::recalcTangents(void)
    {
        // ShoeMake (1987) approach
        // Just like Catmull-Rom really, just more gnarly
        // And no, I don't understand how to derive this!
        //
        // let p = point[i], pInv = p.Inverse
        // tangent[i] = p * exp( -0.25 * ( log(pInv * point[i+1]) + log(pInv * point[i-1]) ) )
        //
        // Assume endpoint tangents are parallel with line with neighbour

        unsigned int i, numPoints;

        mTangents.resize(mPoints.size());

        numPoints = (unsigned int)mPoints.size();

        if (numPoints < 2)
        {
            // Can't do anything yet
            return;
        }

        Quaternion invp, part1, part2, preExp;
        for(i = 0; i < numPoints; ++i)
        {
            Quaternion &p = mPoints[i];
            invp = p.Inverse();

            if (i ==0)
            {
                // special case start
                part1 = (invp * mPoints[i+1]).Log();
                part2 = (invp * p).Log();
            }
            else if (i == numPoints-1)
            {
                // special case end
                part1 = (invp * p).Log();
                part2 = (invp * mPoints[i-1]).Log();
            }
            else
            {
                part1 = (invp * mPoints[i+1]).Log();
                part2 = (invp * mPoints[i-1]).Log();
            }

            preExp = -0.25 * (part1 + part2);
            mTangents[i] = p * preExp.Exp();
            
        }



    }
    //---------------------------------------------------------------------
    const Quaternion& RotationalSpline::getPoint(unsigned short index) const
    {
        assert (index >= 0 && index < mPoints.size() && 
            "Point index is out of bounds!!");

        return mPoints[index];
    }
    //---------------------------------------------------------------------
    unsigned short RotationalSpline::getNumPoints(void) const
    {
        return (unsigned short)mPoints.size();
    }
    //---------------------------------------------------------------------
    void RotationalSpline::clear(void)
    {
        mPoints.clear();
        mTangents.clear();
    }
    //---------------------------------------------------------------------
    void RotationalSpline::updatePoint(unsigned short index, const Quaternion& value)
    {
        assert (index >= 0 && index < mPoints.size() && 
            "Point index is out of bounds!!");

        mPoints[index] = value;
        if (mAutoCalc)
        {
            recalcTangents();
        }
    }
    //---------------------------------------------------------------------
    void RotationalSpline::setAutoCalculate(bool autoCalc)
    {
        mAutoCalc = autoCalc;
    }
    //---------------------------------------------------------------------



}





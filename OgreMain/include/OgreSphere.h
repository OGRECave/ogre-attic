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
#ifndef __Sphere_H_
#define __Sphere_H_

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgreVector3.h"

namespace Ogre {


    /** A sphere primitive, mostly used for bounds checking. 
    @remarks
        A sphere in math texts is normally represented by the function
        x^2 + y^2 + z^2 = r^2 (for sphere's centered on the origin). Ogre stores spheres
        simply as a center point and a radius.
    */
    class _OgreExport Sphere
    {
    protected:
        Real mRadius;
        Vector3 mCenter;
    public:
        /** Standard constructor - creates a unit sphere around the origin.*/
        Sphere() : mRadius(1.0), mCenter(Vector3::ZERO) {}
        /** Constructor allowing arbitrary spheres. 
            @param center The center point of the sphere.
            @param radius The radius of the sphere.
        */
        Sphere(const Vector3& center, Real radius)
            : mRadius(radius), mCenter(center) {}

        /** Returns the radius of the sphere. */
        Real getRadius(void) const { return mRadius; }

        /** Sets the radius of the sphere. */
        void setRadius(Real radius) { mRadius = radius; }

        /** Returns the center point of the sphere. */
        Vector3 getCenter(void) const { return mCenter; }

        /** Sets the center point of the sphere. */
        void setCenter(const Vector3& center) { mCenter = center; }
    };

}

#endif


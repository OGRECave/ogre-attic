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
#ifndef __Ray_H_
#define __Ray_H_

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgreVector3.h"

namespace Ogre {

    /** Representation of a ray in space, ie a line with an origin and direction. */
    class _OgreExport Ray
    {
    protected:
        Vector3 mOrigin;
        Vector3 mDirection;
    public:
        Ray():mOrigin(Vector3::ZERO), mDirection(Vector3::UNIT_Z) {}
        Ray(const Vector3& origin, const Vector3& direction)
            :mOrigin(origin), mDirection(direction) {}
        virtual ~Ray() {}

        /** Sets the origin of the ray. */
        void setOrigin(const Vector3& origin) {mOrigin = origin;} 
        /** Gets the origin of the ray. */
        const Vector3& getOrigin(void) const {return mOrigin;} 

        /** Sets the direction of the ray. */
        void setDirection(const Vector3& dir) {mDirection = dir;} 
        /** Gets the direction of the ray. */
        const Vector3& getDirection(void) const {return mDirection;} 

    };

}
#endif

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
#ifndef __Particle_H__
#define __Particle_H__

#include "OgrePrerequisites.h"
#include "OgreBillboard.h"

namespace Ogre {

    class _OgreExport Particle : public Billboard
    {
    protected:
        
    public:
        // Note the intentional public access to internal variables
        // Accessing via get/set would be too costly for 000's of particles

        // Current direction: now derived
        /// Time to live, number of seconds left of particles natural life
        Real mTimeToLive;
        /// Total Time to live, number of seconds of particles natural life
        Real mTotalTimeToLive;
		/// Speed of rotation in radians
		Real mRotationSpeed;


        Particle()
        : mTimeToLive(10), mTotalTimeToLive(10), mRotationSpeed(0)
        {
        }

		Real getRotationSpeed(void) const { return mRotationSpeed; }
		void setRotationSpeed(Real rotation) { mRotationSpeed = rotation; } 



        
    };
}

#endif


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
#ifndef __PointEmitterFactory_H__
#define __PointEmitterFactory_H__

#include "OgreParticleFXPrerequisites.h"
#include "OgreParticleEmitterFactory.h"
#include "OgrePointEmitter.h"


namespace Ogre {

    /** Factory class for particle emitter of type "Point".
    @remarks
        Creates instances of PointEmitter to be used in particle systems. 
    */
    class PointEmitterFactory : public ParticleEmitterFactory
    {
    protected:

    public:
        /** See ParticleEmitterFactory */
        String getName() 
        { 
            return "Point"; 
        }

        /** See ParticleEmitterFactory */
        ParticleEmitter* createEmitter(void) 
        {
            ParticleEmitter* emit = new PointEmitter();
            mEmitters.push_back(emit);
            return emit;
        }

    };

}

#endif


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
#ifndef __ParticleEmitterFactory_H__
#define __ParticleEmitterFactory_H__


#include "OgrePrerequisites.h"
#include "OgreParticleEmitter.h"
#include "OgreString.h"

namespace Ogre {

    /** Abstract class defining the interface to be implemented by creators of ParticleEmitter subclasses.
    @remarks
        Plugins or 3rd party applications can add new types of particle emitters to Ogre by creating
        subclasses of the ParticleEmitter class. Because multiple instances of these emitters may be
        required, a factory class to manage the instances is also required. 
    @par
        ParticleEmitterFactory subclasses must allow the creation and destruction of ParticleEmitter
        subclasses. They must also be registered with the ParticleSystemManager. All factories have
        a name which identifies them, examples might be 'point', 'cone', or 'box', and these can be 
        also be used from particle system scripts.
    */
    class _OgreExport ParticleEmitterFactory
    {
    protected:
        std::vector<ParticleEmitter*> mEmitters;
    public:
        ParticleEmitterFactory() {};
        virtual ~ParticleEmitterFactory() 
        {
            // Destroy all emitters
            std::vector<ParticleEmitter*>::iterator i;
            for (i = mEmitters.begin(); i != mEmitters.end(); ++i)
            {
                delete (*i);
            }
            
            mEmitters.clear();

        };

        /** Returns the name of the factory, the name which identifies the particle emitter type this factory creates. */
        virtual String getName() = 0;

        /** Creates a new emitter instance.
        @remarks
            The subclass MUST add a pointer to the created instance to mEmitters.
        */
        virtual ParticleEmitter* createEmitter(void) = 0;

        /** Destroys the emitter pointed to by the parameter (for early clean up if reauired). */
        virtual void destroyEmitter(ParticleEmitter* e)
        {
            std::vector<ParticleEmitter*>::iterator i;
            for (i = mEmitters.begin(); i != mEmitters.end(); ++i)
            {
                if ((*i) == e)
                {
                    mEmitters.erase(i);
                    delete e;
                }
            }
        }

    };


}


#endif


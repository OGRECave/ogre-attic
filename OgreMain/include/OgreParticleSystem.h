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
#ifndef __ParticleSystem_H__
#define __ParticleSystem_H__

#include "OgrePrerequisites.h"

#include "OgreBillboardSet.h"
#include "OgreVector3.h"
#include "OgreString.h"
#include "OgreAxisAlignedBox.h"
#include "OgreParticleIterator.h"
#include "OgreStringInterface.h"

namespace Ogre {

    /** Class defining particle system based special effects.
    @remarks
        Particle systems are special effects generators which are based on a number of moving points
        which are rendered perhaps using billboards (quads which always face the camera) to create
        the impression of things like like sparkles, smoke, blood spurts, dust etc.
    @par
        This class simply manages a single collection of particles with a shared local center point
        and a bounding box. The visual aspect of the particles is handled by the base BillboardSet class
        which the ParticleSystem manages automatically.
    @par
        Particle systems are created using the ParticleSystemManager::createParticleSystem method, never directly.
        In addition, like all subclasses of MovableObject, the ParticleSystem will only be considered for
        rendering once it has been attached to a SceneNode. 
    */
    class _OgreExport ParticleSystem : public BillboardSet
    {
    public:

        /** Command object for quota (see ParamCommand).*/
        class CmdQuota : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for material (see ParamCommand).*/
        class CmdMaterial : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for cull_each (see ParamCommand).*/
        class CmdCull : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for particle_width (see ParamCommand).*/
        class CmdWidth : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for particle_height (see ParamCommand).*/
        class CmdHeight : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for billboard type (see ParamCommand).*/
        class CmdBillboardType : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };
        /** Command object for common direction (see ParamCommand).*/
        class CmdCommonDirection : public ParamCommand
        {
        public:
            String doGet(void* target);
            void doSet(void* target, const String& val);
        };

        /// Default constructor required for STL creation in manager
        ParticleSystem();
        /** Creates a particle system with no emitters or affectors.
        @remarks
            You should use the ParticleSystemManager to create particle systems rather than creating
            them directly.
        */
        ParticleSystem(const String& name);

        virtual ~ParticleSystem();

        /** Gets the name of this particle system */
        virtual const String& getName(void) const;

        /** Adds an emitter to this particle system.
        @remarks
            Particles are created in a particle system by emitters - see the ParticleEmitter
            class for more details.
        @param 
            emitterType String identifying the emitter type to create. Emitter types are defined
            by registering new factories with the manager - see ParticleEmitterFactory for more details.
            Emitter types can be extended by OGRE, plugin authors or application developers.
        */
        ParticleEmitter* addEmitter(const String& emitterType);

        /** Retrieves an emitter by it's index (zero-based).
        @remarks
            Used to retrieve a pointer to an emitter for a particle system to procedurally change
            emission parameters etc.
            You should check how many emitters are registered against this system before calling
            this method with an arbitrary index using getNumEmitters.
        @param
            index Zero-based index of the emitter to retrieve.
        */
        ParticleEmitter* getEmitter(unsigned short index) const;

        /** Returns the number of emitters for this particle system. */
        unsigned short getNumEmitters(void) const;

        /** Removes an emitter from the system.
        @remarks
            Drops the emitter with the index specified from this system.
            You should check how many emitters are registered against this system before calling
            this method with an arbitrary index using getNumEmitters.
        @param
            index Zero-based index of the emitter to retrieve.
        */
        void removeEmitter(unsigned short index);

        /** Removes all the emitters from this system. */
        void removeAllEmitters(void);


        /** Adds an affector to this particle system.
        @remarks
            Particles are modified over time in a particle system by affectors - see the ParticleAffector
            class for more details.
        @param 
            affectorType String identifying the affector type to create. Affector types are defined
            by registering new factories with the manager - see ParticleAffectorFactory for more details.
            Affector types can be extended by OGRE, plugin authors or application developers.
        */
        ParticleAffector* addAffector(const String& affectorType);

        /** Retrieves an affector by it's index (zero-based).
        @remarks
            Used to retrieve a pointer to an affector for a particle system to procedurally change
            affector parameters etc.
            You should check how many affectors are registered against this system before calling
            this method with an arbitrary index using getNumAffectors.
        @param
            index Zero-based index of the affector to retrieve.
        */
        ParticleAffector* getAffector(unsigned short index) const;

        /** Returns the number of affectors for this particle system. */
        unsigned short getNumAffectors(void) const;

        /** Removes an affector from the system.
        @remarks
            Drops the affector with the index specified from this system.
            You should check how many affectors are registered against this system before calling
            this method with an arbitrary index using getNumAffectors.
        @param
            index Zero-based index of the affector to retrieve.
        */
        void removeAffector(unsigned short index);

        /** Removes all the affectors from this system. */
        void removeAllAffectors(void);

        /** Gets the number of individual particles in the system right now.
        @remarks
            The number of particles active in a system at a point in time depends on 
            the number of emitters, their emission rates, the time-to-live (TTL) each particle is
            given on emission (and whether any affectors modify that TTL) and the maximum
            number of particles allowed in this system at once (particle quota).
        */
        unsigned int getNumParticles(void) const;

        /** Returns the maximum number of particles this system is allowed to have active at once.
        @remarks
            See ParticleSystem::setParticleQuota for more info.
        */
        unsigned int getParticleQuota(void) const;

        /** Sets the maximum number of particles this system is allowed to have active at once.
        @remarks
            Particle systems all have a particle quota, i.e. a maximum number of particles they are 
            allowed to have active at a time. This allows the application to set a keep particle systems
            under control should they be affected by complex parameters which alter their emission rates
            etc. If a particle system reaches it's particle quota, none of the emitters will be able to 
            emit any more particles. As existing particles die, the spare capacity will be allocated
            equally across all emitters to be as consistent to the origina particle system style as possible.
        @param quota The maximum number of particles this system is allowed to have.
        */
        void setParticleQuota(unsigned int quota);


        /** Assignment operator for copying.
        @remarks
            This operator deep copies all particle emitters and effectors, but not particles. The
            system's name is also not copied.
        */
        ParticleSystem& operator=(const ParticleSystem& rhs);

        /** Updates the particles in the system based on time elapsed.
        @remarks
            This is called automatically every frame by OGRE.
        @param
            timeElapsed The amount of time, in seconds, since the last frame.
        */
        void _update(Real timeElapsed);

        /** Returns an iterator for stepping through all particles in this system.
        @remarks
            This method is designed to be used by people providing new ParticleAffector subclasses,
            this is the easiest way to step through all the particles in a system and apply the
            changes the affector wants to make.
        */
        ParticleIterator _getIterator(void);

        /** Overridden from MovableObject
            @see
                MovableObject
        */
        void getWorldTransforms(Matrix4* xform);

        /** Overridden from BillboardSet
        */
        void _notifyCurrentCamera(Camera* cam);

        /** Fast-forwards this system by the required number of seconds.
        @remarks
            This method allows you to fast-forward a system so that it effectively looks like
            it has already been running for the time you specify. This is useful to avoid the
            'startup sequence' of a system, when you want the system to be fully populated right
            from the start.
        @param
            time The number of seconds to fast-forward by.
        @param
            interval The sampling interval used to generate particles, apply affectors etc. The lower this
            is the more realistic the fast-forward, but it takes more iterations to do it.
        */
        void fastForward(Real time, Real interval = 0.1);
    protected:

        /// Command objects
        static CmdCull msCullCmd;
        static CmdHeight msHeightCmd;
        static CmdMaterial msMaterialCmd;
        static CmdQuota msQuotaCmd;
        static CmdWidth msWidthCmd;
        static CmdBillboardType msBillboardTypeCmd;
        static CmdCommonDirection msCommonDirectionCmd;


        /// Name of the particle system instance
        String mName;

        typedef std::vector<ParticleEmitter*> ParticleEmitterList;
        typedef std::vector<ParticleAffector*> ParticleAffectorList;
        
        /// List of particle emitters, ie sources of particles
        ParticleEmitterList mEmitters;
        /// List of particle affectors, ie modifiers of particles
        ParticleAffectorList mAffectors;

        /** Internal method used to expire dead particles. */
        void _expire(Real timeElapsed);

        /** Spawn new particles based on free quota and emitter requirements. */
        void _triggerEmitters(Real timeElapsed);

        /** Updates existing particle based on their momentum. */
        void _applyMotion(Real timeElapsed);

        /** Applies the effects of affectors. */
        void _triggerAffectors(Real timeElapsed);

        /** Overridden from BillboardSet to create Particle instead of Billboard */
        void increasePool(unsigned int size);

        /** Internal method for adding a new active particle.*/
        Particle* addParticle(void);

        /** Overidden from BillboardSet
            @see
                BillboardSet
        */
        void genBillboardAxes(Camera& cam, Vector3* pX, Vector3 *pY, const Billboard* pBill = 0);

        /** Internal method for initialising string interface. */
        void initParameters(void);
    };
}

#endif

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
#ifndef __ParticleSystem_H__
#define __ParticleSystem_H__

#include "OgrePrerequisites.h"

#include "OgreVector3.h"
#include "OgreString.h"
#include "OgreParticleIterator.h"
#include "OgreStringInterface.h"

namespace Ogre {

    /** Class defining particle system based special effects.
    @remarks
        Particle systems are special effects generators which are based on a 
        number of moving points to create the impression of things like like 
        sparkles, smoke, blood spurts, dust etc.
    @par
        This class simply manages a single collection of particles in world space
        with a shared local origin for emission. The visual aspect of the 
        particles is handled by a ParticleSystemRenderer instance.
    @par
        Particle systems are created using the ParticleSystemManager methods, never directly.
        In addition, like all subclasses of MovableObject, the ParticleSystem will only be considered for
        rendering once it has been attached to a SceneNode. 
    */
    class _OgreExport ParticleSystem : public StringInterface, public MovableObject
    {
    public:

        /** Command object for quota (see ParamCommand).*/
        class CmdQuota : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for material (see ParamCommand).*/
        class CmdMaterial : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for cull_each (see ParamCommand).*/
        class CmdCull : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for particle_width (see ParamCommand).*/
        class CmdWidth : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
            void doSet(void* target, const String& val);
        };
        /** Command object for particle_height (see ParamCommand).*/
        class CmdHeight : public ParamCommand
        {
        public:
            String doGet(const void* target) const;
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

        /** Sets the ParticleRenderer to be used to render this particle system.
        @remarks
            The main ParticleSystem just manages the creation and movement of 
            particles; they are rendered using functions in ParticleRenderer
            and the ParticleVisual instances they create.
		@param typeName String identifying the type of renderer to use; a new 
			instance of this type will be created; a factory must have been registered
			with ParticleSystemManager.
        */
        void setRenderer(const String& typeName);

        /** Gets the ParticleRenderer to be used to render this particle system. */
        ParticleSystemRenderer* getRenderer(void) const;

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

        /** Empties this set of all particles.
        */
        void clear();

        /** Gets the number of individual particles in the system right now.
        @remarks
            The number of particles active in a system at a point in time depends on 
            the number of emitters, their emission rates, the time-to-live (TTL) each particle is
            given on emission (and whether any affectors modify that TTL) and the maximum
            number of particles allowed in this system at once (particle quota).
        */
        size_t getNumParticles(void) const;

		/** Manually add a particle to the system. 
		@remarks
			Instead of using an emitter, you can manually add a particle to the system.
			You must initialise the returned particle instance immediately with the
			'emission' state.
		@note
			There is no corresponding 'destroyParticle' method - if you want to dispose of a
			particle manually (say, if you've used setSpeedFactor(0) to make particles live forever)
			you should use getParticle() and modify it's timeToLive to zero, meaning that it will
			get cleaned up in the next update.
		*/
		Particle* createParticle(void);

		/** Retrieve a particle from the system for manual tweaking.
		@remarks
			Normally you use an affector to alter particles in flight, but
			for small manually controlled particle systems you might want to use
			this method.
		*/
		Particle* getParticle(size_t index);

        /** Returns the maximum number of particles this system is allowed to have active at once.
        @remarks
            See ParticleSystem::setParticleQuota for more info.
        */
        size_t getParticleQuota(void) const;

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
        void setParticleQuota(size_t quota);


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

        /** Sets the name of the material to be used for this billboard set.
            @param
                name The new name of the material to use for this set.
        */
        virtual void setMaterialName(const String& name);

        /** Sets the name of the material to be used for this billboard set.
            @returns The name of the material that is used for this set.
        */
        virtual const String& getMaterialName(void) const;

        /** Overridden from MovableObject
            @see
                MovableObject
        */
        virtual void _notifyCurrentCamera(Camera* cam);

        /** Overridden from MovableObject
            @see
                MovableObject
        */
        virtual const AxisAlignedBox& getBoundingBox(void) const { return mAABB; }

        /** Overridden from MovableObject
            @see
                MovableObject
        */
        virtual Real getBoundingRadius(void) const { return mBoundingRadius; }

        /** Overridden from MovableObject
            @see
                MovableObject
        */
        virtual void _updateRenderQueue(RenderQueue* queue);

        /** Overridden from MovableObject */
        const String& getName(void) const { return mName; }

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

		/** Sets a 'speed factor' on this particle system, which means it scales the elapsed
			real time which has passed by this factor before passing it to the emitters, affectors,
			and the particle life calculation.
		@remarks
			An interesting side effect - if you want to create a completely manual particle system
			where you control the emission and life of particles yourself, you can set the speed
			factor to 0.0f, thus disabling normal particle emission, alteration, and death.
		*/
		void setSpeedFactor(Real speedFactor) { mSpeedFactor = speedFactor; }

		/** Gets the 'speed factor' on this particle system.
		*/
		Real getSpeedFactor(void) const { return mSpeedFactor; }


        /** Overridden from MovableObject */
        const String& getMovableType(void) const;

        /** Internal callback used by Particles to notify their parent that they have been resized.
        */
        virtual void _notifyParticleResized(void);

        /** Internal callback used by Particles to notify their parent that they have been rotated.
        */
        virtual void _notifyParticleRotated(void);

        /** Sets the default dimensions of the particles in this set.
            @remarks
                All particles in a set are created with these default dimensions. The set will render most efficiently if
                all the particles in the set are the default size. It is possible to alter the size of individual
                particles at the expense of extra calculation. See the Particle class for more info.
            @param width
                The new default width for the particles in this set.
            @param height
                The new default height for the particles in this set.
        */
        virtual void setDefaultDimensions(Real width, Real height);

        /** See setDefaultDimensions - this sets 1 component individually. */
        virtual void setDefaultWidth(Real width);
        /** See setDefaultDimensions - this gets 1 component individually. */
        virtual Real getDefaultWidth(void) const;
        /** See setDefaultDimensions - this sets 1 component individually. */
        virtual void setDefaultHeight(Real height);
        /** See setDefaultDimensions - this gets 1 component individually. */
        virtual Real getDefaultHeight(void) const;
        /** Returns whether or not particles in this are tested individually for culling. */
        virtual bool getCullIndividually(void) const;
        /** Sets whether culling tests particles in this individually as well as in a group.
        @remarks
            Particle sets are always culled as a whole group, based on a bounding box which 
            encloses all particles in the set. For fairly localised sets, this is enough. However, you
            can optionally tell the set to also cull individual particles in the set, i.e. to test
            each individual particle before rendering. The default is not to do this.
        @par
            This is useful when you have a large, fairly distributed set of particles, like maybe 
            trees on a landscape. You probably still want to group them into more than one
            set (maybe one set per section of landscape), which will be culled coarsely, but you also
            want to cull the particles individually because they are spread out. Whilst you could have
            lots of single-tree sets which are culled separately, this would be inefficient to render
            because each tree would be issued as it's own rendering operation.
        @par
            By calling this method with a parameter of true, you can have large particle sets which 
            are spaced out and so get the benefit of batch rendering and coarse culling, but also have
            fine-grained culling so unnecessary rendering is avoided.
        @param cullIndividual If true, each particle is tested before being sent to the pipeline as well 
            as the whole set having to pass the coarse group bounding test.
        */
        virtual void setCullIndividually(bool cullIndividual);

    protected:

        /// Command objects
        static CmdCull msCullCmd;
        static CmdHeight msHeightCmd;
        static CmdMaterial msMaterialCmd;
        static CmdQuota msQuotaCmd;
        static CmdWidth msWidthCmd;


        AxisAlignedBox mAABB;
        Real mBoundingRadius;
        Real mBoundsUpdateTime;

        /// World AABB, only used to compare world-space positions to calc bounds
        AxisAlignedBox mWorldAABB;

        /// Name of the system; used for location in the scene.
        String mName;
        /// Name of the material to use
        String mMaterialName;
        /// Pointer to the material to use
        MaterialPtr mpMaterial;
        /// Default width of each particle
        Real mDefaultWidth;
        /// Default height of each particle
        Real mDefaultHeight;
		/// Speed factor
		Real mSpeedFactor;

        typedef std::list<Particle*> ActiveParticleList;
        typedef std::deque<Particle*> FreeParticleQueue;
        typedef std::vector<Particle*> ParticlePool;

        /** Active particle list.
            @remarks
                This is a linked list of pointers to particles in the particle pool.
            @par
                This allows very fast instertions and deletions from anywhere in 
                the list to activate / deactivate particles as well as resuse of 
                Particle instances in the pool without construction & destruction 
                which avoids memory thrashing.
        */
        ActiveParticleList mActiveParticles;

        /** Free particle queue.
            @remarks
                This contains a list of the particles free for use as new instances
                as required by the set. Particle instances are preconstructed up 
                to the estimated size in the mParticlePool vector and are 
                referenced on this deque at startup. As they get used this deque
                reduces, as they get released back to to the set they get added back to the deque.
        */
        FreeParticleQueue mFreeParticles;

        /** Pool of particle instances for use and reuse in the active particle list.
            @remarks
                This vector will be preallocated with the estimated size of the set,and will extend as required.
        */
        ParticlePool mParticlePool;

        typedef std::vector<ParticleEmitter*> ParticleEmitterList;
        typedef std::vector<ParticleAffector*> ParticleAffectorList;
        
        /// List of particle emitters, ie sources of particles
        ParticleEmitterList mEmitters;
        /// List of particle affectors, ie modifiers of particles
        ParticleAffectorList mAffectors;

        /// The renderer used to render this particle system
        ParticleSystemRenderer* mRenderer;

        /// Do we cull each particle individually?
        bool mCullIndividual;

        /// The name of the type of renderer used to render this system
        String mRendererType;
        
        /// The number of particles in the pool.
        size_t mPoolSize;


        /** Internal method used to expire dead particles. */
        void _expire(Real timeElapsed);

        /** Spawn new particles based on free quota and emitter requirements. */
        void _triggerEmitters(Real timeElapsed);

        /** Updates existing particle based on their momentum. */
        void _applyMotion(Real timeElapsed);

        /** Applies the effects of affectors. */
        void _triggerAffectors(Real timeElapsed);

        /** Resize the internal pool of particles. */
        void increasePool(size_t size);

        /** Internal method for initialising string interface. */
        void initParameters(void);

		/// Internal method for creating ParticleVisualData instances for the pool
		void createVisualParticles(size_t poolstart, size_t poolend);
		/// Internal method for destroying ParticleVisualData instances for the pool
		void destroyVisualParticles(size_t poolstart, size_t poolend);

        /** Set the (initial) bounds of the particle system manually. 
        @remarks
            If you can, set the bounds of a particle system up-front and 
            call setBoundsUpdatePeriod(0); this is the most efficient way to
            organise it. Otherwise, set an initial bounds and let the bounds increase
            for a little while (the default is 5 seconds), after which time the 
            AABB is fixed to save time.
        */
        void setBounds(const AxisAlignedBox& aabb);

        /** Sets the period of time for which the bounds of the particle system 
            are measured and updated (increased only), after which time they
            are assumed to have reached their maximum and will not be 
            updated any more. 
        */
        void setBoundsUpdatePeriod(Real time) { mBoundsUpdateTime = time; }

        /** Internal method for updating the bounds of the particle system.
        @remarks
            This is called automatically for a period of time after the system's
            creation (5 seconds by default, settable by setBoundsUpdatePeriod) 
            to increase (and only increase) the bounds of the system according 
            to the emitted and affected particles. After this period, the 
            system is assumed to achieved its maximum size, and the bounds are
            no longer computed for efficiency. You can tweak the behaviour by 
            either setting the bounds manually (setBounds, preferred), or 
            changing the time over which the bounds are updated (performance cost).
            You can also call this method manually if you need to update the 
            bounds on an ad-hoc basis.
        */
        void _updateBounds(void);

    };

}

#endif

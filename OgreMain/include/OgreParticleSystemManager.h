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
#ifndef __ParticleSystemManager_H__
#define __ParticleSystemManager_H__


#include "OgrePrerequisites.h"
#include "OgreParticleSystem.h"
#include "OgreFrameListener.h"

namespace Ogre {

    /** Manages particle systems, particle system scripts (templates) and the available emitter & affector factories.
    @remarks
        This singleton class is responsible for creating and managing particle systems. All particle
        systems must be created and destroyed using this object. Remember that like all other MovableObject
        subclasses, ParticleSystems do not get rendered until they are attached to a SceneNode object.
    @par
        This class also manages factories for ParticleEmitter and ParticleAffector classes. To enable easy
        extensions to the types of emitters (particle sources) and affectors (particle modifiers), the
        ParticleSystemManager lets plugins or applications register factory classes which submit new
        subclasses to ParticleEmitter and ParticleAffector. Ogre comes with a number of them already provided,
        such as cone, sphere and box-shaped emitters, and simple affectors such as constant directional force
        and colour faders. However using this registration process, a plugin can create any behaviour
        required.
    @par
        This class also manages the loading and parsing of particle system scripts, which are text files
        describing named particle system templates. Instances of particle systems using these templates can
        then be created easily through the createParticleSystem method.
    */
    class _OgreExport ParticleSystemManager: public Singleton<ParticleSystemManager>, public FrameListener
    {
    protected:
        typedef std::map<String, ParticleSystem> ParticleTemplateMap;
        /// Templates based on scripts
        ParticleTemplateMap mSystemTemplates;
        
        typedef std::map<String, ParticleSystem*> ParticleSystemMap;
        /// Actual instantiated particle systems (may be based on template, may be manual)
        ParticleSystemMap mSystems;

        typedef std::map<String, ParticleEmitterFactory*> ParticleEmitterFactoryMap;
        /// Factories for named emitter types (can be extended using plugins)
        ParticleEmitterFactoryMap mEmitterFactories;

        typedef std::map<String, ParticleAffectorFactory*> ParticleAffectorFactoryMap;
        /// Factories for named affector types (can be extended using plugins)
        ParticleAffectorFactoryMap mAffectorFactories;

        /** Internal script parsing method. */
        void parseNewEmitter(const String& type, DataChunk& chunk, ParticleSystem* sys);
        /** Internal script parsing method. */
        void parseNewAffector(const String& type, DataChunk& chunk, ParticleSystem* sys);
        /** Internal script parsing method. */
        void parseAttrib(const String& line, ParticleSystem* sys);
        /** Internal script parsing method. */
        void parseEmitterAttrib(const String& line, ParticleEmitter* sys);
        /** Internal script parsing method. */
        void parseAffectorAttrib(const String& line, ParticleAffector* sys);
        /** Internal script parsing method. */
        void skipToNextCloseBrace(DataChunk& chunk);
        /** Internal script parsing method. */
        void skipToNextOpenBrace(DataChunk& chunk);
    public:

        ParticleSystemManager();
        virtual ~ParticleSystemManager();

        /** Adds a new 'factory' object for emitters to the list of available emitter types.
        @remarks
            This method allows plugins etc to add new particle emitter types to Ogre. Particle emitters
            are sources of particles, and generate new particles with their start positions, colours and
            momentums appropriately. Plugins would create new subclasses of ParticleEmitter which 
            emit particles a certain way, and register a subclass of ParticleEmitterFactory to create them (since multiple 
            emitters can be created for different particle systems).
        @par
            All particle emitter factories have an assigned name which is used to identify the emitter
            type. This must be unique.
        @par
            Note that the object passed to this function will not be destroyed by the ParticleSystemManager,
            since it may have been allocted on a different heap in the case of plugins. The caller must
            destroy the object later on, probably on plugin shutdown.
        @param
            factory Pointer to a ParticleEmitterFactory subclass created by the plugin or application code.
        */
        void addEmitterFactory(ParticleEmitterFactory* factory);

        /** Adds a new 'factory' object for affectors to the list of available affector types.
        @remarks
            This method allows plugins etc to add new particle affector types to Ogre. Particle
            affectors modify the particles in a system a certain way such as affecting their direction
            or changing their colour, lifespan etc. Plugins would
            create new subclasses of ParticleAffector which affect particles a certain way, and register
            a subclass of ParticleAffectorFactory to create them.
        @par
            All particle affector factories have an assigned name which is used to identify the affector
            type. This must be unique.
        @par
            Note that the object passed to this function will not be destroyed by the ParticleSystemManager,
            since it may have been allocted on a different heap in the case of plugins. The caller must
            destroy the object later on, probably on plugin shutdown.
        @param
            factory Pointer to a ParticleAffectorFactory subclass created by the plugin or application code.
        */
        void addAffectorFactory(ParticleAffectorFactory* factory);

        /** Adds a new particle system template to the list of available templates. 
        @remarks
            Instances of particle systems in a scene are not normally unique - often you want to place the
            same effect in many places. This method allows you to register a ParticleSystem as a named template,
            which can subsequently be used to create instances using the createSystem method.
        @par
            Note that particle system templates can either be created programmatically by an application 
            and registered using this method, or they can be defined in a script file (*.particle) which is
            loaded by the engine at startup, very much like Material scripts.
        @param
            name The name of the template. Must be unique across all templates.
        @param
            sysTemplate A reference to a particle system to be used as a template. This instance will 
                be copied to a template held in this class so the object passed in here may be destroyed
                independently.
            
        */
        void addTemplate(const String& name, const ParticleSystem& sysTemplate);

        /** Create a new particle system template. 
        @remarks
            This method is similar to the addTemplate method, except this just creates a new template
            and returns a pointer to it to be populated. Use this when you don't already have a system
            to add as a template and just want to create a new template which you will build up in-place.
        @param
            name The name of the template. Must be unique across all templates.
            
        */
        ParticleSystem* createTemplate(const String& name);

        /** Retrieves a particle system template for possible modification. 
        @remarks
            Modifying a template does not affect the settings on any ParticleSystems already created
            from this template.
        */
        ParticleSystem* getTemplate(const String& name);

        /** Basic method for creating a blank particle system.
        @remarks
            This method creates a new, blank ParticleSystem instance and returns a pointer to it.
            The caller should not delete this object, it will be freed at system shutdown, or can
            be released earlier using the destroySystem method.
        @par
            The instance returned from this method won't actually do anything because on creation a
            particle system has no emitters. The caller should manipulate the instance through it's 
            ParticleSystem methods to actually create a real particle effect. 
        @par
            Creating a particle system does not make it a part of the scene. As with other MovableObject
            subclasses, a ParticleSystem is not rendered until it is attached to a SceneNode. 
        @param
            name The name to give the ParticleSystem.
        @param 
            quota The maximum number of particles to allow in this system. 
        */
        ParticleSystem* createSystem(const String& name, unsigned int quota = 500);

        /** Creates a particle system based on a template.
        @remarks
            This method creates a new ParticleSystem instance based on the named template and returns a 
            pointer to the caller. The caller should not delete this object, it will be freed at system shutdown, 
            or can be released earlier using the destroySystem method.
        @par
            Each system created from a template takes the template's settings at the time of creation, 
            but is completely separate from the template from there on. 
        @par
            Creating a particle system does not make it a part of the scene. As with other MovableObject
            subclasses, a ParticleSystem is not rendered until it is attached to a SceneNode. 
        @par
            This is probably the more useful particle system creation method since it does not require manual
            setup of the system.
        @param 
            name The name to give the new particle system instance.
        @param 
            templateName The name of the template to base the new instance on.
        @param 
            quota The maximum number of particles to allow in this system (can be changed later)
        */
        ParticleSystem* createSystem(const String& name, const String& templateName, unsigned int quota = 500);

        /** Destroys a particle system, freeing it's memory and removing references to it in this class.
        @remarks
            You should ensure that before calling this method, the particle system has been detached from 
            any SceneNode objects, and that no other objects are referencing it.
        @param
            name The name of the ParticleSystem to destroy.
        */
        void destroySystem(const String& name);

        /** Destroys a particle system, freeing it's memory and removing references to it in this class.
        @remarks
            You should ensure that before calling this method, the particle system has been detached from 
            any SceneNode objects, and that no other objects are referencing it.
        @param
            sys Pointer to the ParticleSystem to be destroyed.
        */
        void destroySystem(ParticleSystem* sys);

        /** Retrieves a pointer to a system already created. */
        ParticleSystem* getSystem(const String& name);


        /** Internal method for creating a new emitter from a factory.
        @remarks
            Used internally by the engine to create new ParticleEmitter instances from named
            factories. Applications should use the ParticleSystem::addEmitter method instead, 
            which calls this method to create an instance.
        @param
            emitterType String name of the emitter type to be created. A factory of this type must have been registered.
        */
        ParticleEmitter* _createEmitter(const String& emitterType);

        /** Internal method for destroying an emitter.
        @remarks
            Because emitters are created by factories which may allocate memory from separate heaps,
            the memory allocated must be freed from the same place. This method is used to ask the factory
            to destroy the instance passed in as a pointer.
        @param
            emitter Pointer to emitter to be destroyed. On return this pointer will point to invalid (freed) memory.
        */
        void _destroyEmitter(ParticleEmitter* emitter);

        /** Internal method for creating a new affector from a factory.
        @remarks
            Used internally by the engine to create new ParticleAffector instances from named
            factories. Applications should use the ParticleSystem::addAffector method instead, 
            which calls this method to create an instance.
        @param
            effectorType String name of the affector type to be created. A factory of this type must have been registered.
        */
        ParticleAffector* _createAffector(const String& affectorType);

        /** Internal method for destroying an affector.
        @remarks
            Because affectors are created by factories which may allocate memory from separate heaps,
            the memory allocated must be freed from the same place. This method is used to ask the factory
            to destroy the instance passed in as a pointer.
        @param
            affector Pointer to affector to be destroyed. On return this pointer will point to invalid (freed) memory.
        */
        void _destroyAffector(ParticleAffector* affector);

        /** Frame event */
        bool frameStarted(const FrameEvent &evt);

        /** Frame event */
        bool frameEnded(const FrameEvent &evt);

        /** Init method to be called by OGRE system.
        @remarks
            Due to dependencies between various objects certain initialisation tasks cannot be done
            on construction. OGRE will call this method when the rendering subsystem is initialised.
        */
        void _initialise(void);
        /** Override standard Singleton retrieval.
            @remarks
                Why do we do this? Well, it's because the Singleton implementation is in a .h file,
                which means it gets compiled into anybody who includes it. This is needed for the Singleton
                template to work, but we actually only want it compiled into the implementation of the
                class based on the Singleton, not all of them. If we don't change this, we get link errors
                when trying to use the Singleton-based class from an outside dll.
            @pre
                This method just delegates to the template version anyway, but the implementation stays in this
                single compilation unit, preventing link errors.
        */
        static ParticleSystemManager& getSingleton(void);

        /** Parses a particle system script file passed as a chunk.
        */
        void parseScript(DataChunk& chunk);

        /** Parses all particle system script files in resource folders & archives.
        */
        void parseAllSources(const String& extension = ".particle");


    
    };

}

#endif


/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2003 The OGRE Team
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
#ifndef __HighLevelGpuProgramManager_H__
#define __HighLevelGpuProgramManager_H__

#include "OgrePrerequisites.h"
#include "OgreResourceManager.h"
#include "OgreSingleton.h"
#include "OgreException.h"
#include "OgreHighLevelGpuProgram.h"

namespace Ogre {

	/** Interface definition for factories of HighLevelGpuProgram. */
	class _OgreExport HighLevelGpuProgramFactory
	{
	public:
        HighLevelGpuProgramFactory() {}
        virtual ~HighLevelGpuProgramFactory() {}
		/// Get the name of the language this factory creates programs for
		virtual const String& getLanguage(void) const = 0;
        virtual HighLevelGpuProgram* create(const String& name, GpuProgramType gptype) = 0;
		virtual void destroy(HighLevelGpuProgram* prog) = 0;
	};
	/** This ResourceManager manages high-level vertex and fragment programs. 
	@remarks
		High-level vertex and fragment programs can be used instead of assembler programs
		as managed by GpuProgramManager; however they typically result in a GpuProgram
		being created as a derivative of the high-level program. High-level programs are
		easier to write, and can often be API-independent, unlike assembler programs. 
	@par
		This class not only manages the programs themselves, it also manages the factory
		classes which allow the creation of high-level programs using a variety of high-level
		syntaxes. Plugins can be created which register themselves as high-level program
		factories and as such the engine can be extended to accept virtually any kind of
		program provided a plugin is written.
	*/
	class _OgreExport HighLevelGpuProgramManager 
		: public ResourceManager, public Singleton<HighLevelGpuProgramManager>
	{
	public:
		typedef std::map<String, HighLevelGpuProgramFactory*> FactoryMap;
	protected:
		/// Factories capable of creating HighLevelGpuProgram instances
		FactoryMap mFactories;

		HighLevelGpuProgramFactory* getFactory(const String& language);

	public:
		HighLevelGpuProgramManager();
		~HighLevelGpuProgramManager();
		/** Add a new factory object for high-level programs of a given language. */
		void addFactory(HighLevelGpuProgramFactory* factory);

        /** Creates a new blank resource, compatible with this manager.
            @remarks
                Resource managers handle disparate types of resources. This method returns a pointer to a
                valid new instance of the kind of resource managed here. The caller should  complete the
                details of the returned resource and call ResourceManager::load to load the resource. Note
                that it is the CALLERS responsibility to destroy this object when it is no longer required
                (after calling ResourceManager::unload if it had been loaded).
        */
		virtual Resource* create( const String& name ) 
		{
			// N/A, we need to know the type
			Except(Exception::ERR_INTERNAL_ERROR, "You should call load "
				"with a specific type", "HighLevelGpuProgramManager::create");
		}



        /** Create a new, unloaded HighLevelGpuProgram. 
		@par
			This method creates a new program of the type specified as the second and third parameters.
			You will have to call further methods on the returned program in order to 
			define the program fully before you can load it.
		@param name The identifying name of the program
		@param language Code of the language to use (e.g. "cg")
		@param gptype The type of program to create
		*/
		virtual HighLevelGpuProgram* createProgram(
			const String& name, const String& language, GpuProgramType gptype, 
            int priority = 1);

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
        static HighLevelGpuProgramManager& getSingleton(void);
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
        static HighLevelGpuProgramManager* getSingletonPtr(void);


	};

}

#endif

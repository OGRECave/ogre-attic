/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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
#ifndef __GpuProgramManager_H_
#define __GpuProgramManager_H_

// Precompiler options
#include "OgrePrerequisites.h"
#include "OgreResourceManager.h"
#include "OgreException.h"
#include "OgreGpuProgram.h"
#include "OgreSingleton.h"

namespace Ogre {

	class _OgreExport GpuProgramManager : public ResourceManager, public Singleton<GpuProgramManager>
	{
	public:
		typedef std::set<String> SyntaxCodes;

	protected:
		/// Supported program syntax codes
		SyntaxCodes mSyntaxCodes;
	public:
		GpuProgramManager() {}
		virtual ~GpuProgramManager() {}

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
				"with a specific type", "GpuProgramManager::create");
		}


        /** Loads a GPU program from a file of assembly. 
		@remarks
			This method creates a new program of the type specified as the second parameter.
			As with all types of ResourceManager, this class will search for the file in
			all resource locations it has been configured to look in.
		@param name The name of the GpuProgram
		@param filename The file to load
		@param gptype The type of program to create
        @param syntaxCode The name of the syntax to be used for this program e.g. arbvp1, vs_1_1
		*/
		virtual GpuProgram* load(const String& name, const String& filename, GpuProgramType gptype, 
            const String& syntaxCode, int priority = 1);

		/** Loads a GPU program from a string of assembly code.
		@remarks
			The assembly code must be compatible with this manager - call the 
			getSupportedSyntax method for details of the supported syntaxes 
		@param name The identifying name to give this program, which can be used to
			retrieve this program later with getByName.
		@param code A string of assembly code which will form the program to run
		@param gptype The type of prgram to create.
        @param syntaxCode The name of the syntax to be used for this program e.g. arbvp1, vs_1_1
		*/
		virtual GpuProgram* loadFromString(const String& name, const String& code, GpuProgramType gptype,
            const String& syntaxCode, int priority = 1);

		/** Returns the syntaxes that this manager supports. */
		virtual const SyntaxCodes& getSupportedSyntax(void) const { return mSyntaxCodes; };

        /** Returns whether a given syntax code (e.g. "ps_1_3", "fp20", "arbvp1") is supported. */
        virtual bool isSyntaxSupported(const String& syntaxCode) const;
		
		/** Creates a new GpuProgramParameters instance which can be used to bind
            parameters to your programs.
        @remarks
            Program parameters can be shared between multiple programs if you wish.
        */
        virtual GpuProgramParametersSharedPtr createParameters(void) = 0;
        
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
        static GpuProgramManager& getSingleton(void);

        /** Create a new, unloaded GpuProgram from a file of assembly. 
        @remarks    
            Use this method in preference to the 'load' methods if you wish to define
            a GpuProgram, but not load it yet; useful for saving memory.
		@par
			This method creates a new program of the type specified as the second parameter.
			As with all types of ResourceManager, this class will search for the file in
			all resource locations it has been configured to look in. 
		@param name The name of the program
		@param filename The file to load
        @param syntaxCode The name of the syntax to be used for this program e.g. arbvp1, vs_1_1
		@param gptype The type of program to create
		*/
		virtual GpuProgram* createProgram(const String& name, const String& filename, 
			GpuProgramType gptype, const String& syntaxCode, int priority = 1);

		/** Create a GPU program from a string of assembly code.
        @remarks    
            Use this method in preference to the 'load' methods if you wish to define
            a GpuProgram, but not load it yet; useful for saving memory.
		@par
			The assembly code must be compatible with this manager - call the 
			getSupportedSyntax method for details of the supported syntaxes 
		@param name The identifying name to give this program, which can be used to
			retrieve this program later with getByName.
		@param code A string of assembly code which will form the program to run
		@param gptype The type of prgram to create.
        @param syntaxCode The name of the syntax to be used for this program e.g. arbvp1, vs_1_1
		*/
		virtual GpuProgram* createProgramFromString(const String& name, const String& code, 
            GpuProgramType gptype, const String& syntaxCode, int priority = 1);

        /** Internal method for populating the supported syntax codes, called by RenderSystem. */
        virtual void _pushSyntaxCode(const String& syntaxCode) { mSyntaxCodes.insert(syntaxCode); }
        /** Overrides the standard ResourceManager getByName method.
        @param name The name of the program to retrieve
        @param preferHighLevelPrograms If set to true (the default), high level programs will be
            returned in preference to low-level programs.
        */
        Resource* getByName(const String& name, bool preferHighLevelPrograms = true);
		/** Create a new GpuProgram. */
        virtual GpuProgram* create(const String& name, GpuProgramType gptype, const String& syntaxCode) = 0;
    protected:


	};

}

#endif

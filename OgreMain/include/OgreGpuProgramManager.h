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

namespace Ogre {

	class _OgreExport GpuProgramManager : public ResourceManager, public Singleton<GpuProgramManager>
	{
	public:
		typedef std::set<String> SyntaxCodes;

	protected:
		/// Supported program syntax codes
		SyntaxCodes mSyntaxCodes;
	public:

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
		@param filename The file to load, which will also become the 
			identifying name of the GpuProgram which is returned.
		@param gptype The type of program to create
		*/
		virtual GpuProgram* load(const String& filename, GpuProgramType gptype, int priority = 1);

		/** Loads a GPU program from a string of assembly code.
		@remarks
			The assembly code must be compatible with this manager - call the 
			getSupportedSyntax method for details of the supported syntaxes 
		@param name The identifying name to give this program, which can be used to
			retrieve this program later with getByName.
		@param code A string of assembly code which will form the program to run
		@param gptype The type of prgram to create.
		*/
		virtual GpuProgram* load(const String& name, const String& code, GpuProgramType gptype, int priority = 1);

		/** Returns the syntaxes that this manager supports. */
		virtual const SyntaxCodes& getSupportedSyntax(void) { return mSyntaxCodes; };
		
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
    protected:
		/** Create a new GpuProgram. */
        virtual GpuProgram* create(const String& name, GpuProgramType gptype) = 0;


	};

}

#endif

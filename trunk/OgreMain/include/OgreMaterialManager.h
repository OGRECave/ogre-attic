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
#ifndef __MATERIALMANAGER_H__
#define __MATERIALMANAGER_H__

#include "OgrePrerequisites.h"

#include "OgreSingleton.h"
#include "OgreResourceManager.h"
#include "OgreMaterial.h"
#include "OgreStringVector.h"
#include "OgreMaterialSerializer.h"

namespace Ogre {


    /** Class for managing Material settings for Ogre.
        @remarks
            Materials control the eventual surface rendering properties of geometry. This class
            manages the library of materials, dealing with programmatic registrations and lookups,
            as well as loading predefined Material settings from scripts.
        @par
            When loaded from a script, a Material is in an 'unloaded' state and only stores the settings
            required. It does not at that stage load any textures. This is because the material settings may be
            loaded 'en masse' from bulk material script files, but only a subset will actually be required.
        @par
            Because this is a subclass of ResourceManager, any files loaded will be searched for in any path or
            archive added to the resource paths/archives. See ResourceManager for details.
        @par
            For a definition of the material script format, see the Tutorials/MaterialScript.html file.
    */
    class _OgreExport MaterialManager : public ResourceManager, public Singleton<MaterialManager>
    {
    protected:

        typedef HashMap<int, Material*> MaterialHandleList;

		/// default texture filtering
		TextureFilterOptions mDefTextureFiltering;

		/// default maxAnisotropy
		int mDefAniso;

        /// Serializer
        MaterialSerializer mSerializer;

    public:
        /** Default constructor.
        */
        MaterialManager();

        /** Default destructor.
        */
        virtual ~MaterialManager();

		/** Intialises the material manager, which also triggers it to 
		 * parse all available .program and .material scripts. */
		void initialise(void);
        
		/** Parses a Material script file passed as a chunk.
        */
        void parseScript(DataChunk& chunk);

        /** Parses all material script files in resource folders & archives.
        */
        void parseAllSources(const String& extension = ".material");

        /** Create implementation required by ResourceManager.
        @remarks
            All Materials created by this method are deferred-load, ie none of
            the textures referenced by the TextureUnitState are loaded, and 
        */
        Resource* create( const String& name );

        /** Sets the default texture filtering to be used for loaded textures, for when textures are
            loaded automatically (e.g. by Material class) or when 'load' is called with the default
            parameters by the application.
            @note
                The default value is TFO_BILINEAR.
        */
        virtual void setDefaultTextureFiltering(TextureFilterOptions fo);
		/// get the default texture filtering
        virtual TextureFilterOptions getDefaultTextureFiltering() const;

        /** Sets the default anisotropy level to be used for loaded textures, for when textures are
            loaded automatically (e.g. by Material class) or when 'load' is called with the default
            parameters by the application.
            @note
                The default value is 1 (no anisotropy).
        */
		void setDefaultAnisotropy(int maxAniso);
		/// get the default maxAnisotropy
		int getDefaultAnisotropy() const;

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
        static MaterialManager& getSingleton(void);
    };

}

#endif

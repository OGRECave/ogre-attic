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

namespace Ogre {

    /// Function def for material attribute parser.
    typedef void (*MATERIAL_ATTRIB_PARSER)(StringVector::iterator&, int, Material*);
    /// Function def for texture layer attribute parser.
    typedef void (*TEXLAYER_ATTRIB_PARSER)(StringVector::iterator&, int, Material*, Material::TextureLayer*);

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
        void parseNewTextureLayer( DataChunk& chunk, Material* pMat );
        void parseAttrib( const String& line, Material* pMat);
        void parseLayerAttrib( const String& line, Material* pMat, Material::TextureLayer* pLayer );

        /// Keyword-mapped attribute parsers.
        //typedef std::map<String, MATERIAL_ATTRIB_PARSER> MatAttribParserList;
        typedef std::map<String, MATERIAL_ATTRIB_PARSER> MatAttribParserList;
        MatAttribParserList mMatAttribParsers;
        /// Keyword-mapped attribute parsers.
        typedef std::map<String, TEXLAYER_ATTRIB_PARSER> LayerAttribParserList;
        //typedef HashMap<String, TEXLAYER_ATTRIB_PARSER, std::hash<String>> LayerAttribParserList;
        LayerAttribParserList mLayerAttribParsers;

        typedef HashMap<int, Material*> MaterialHandleList;

        /// Materials by handle.
        MaterialHandleList mHandles;

		//default texture filtering
		TextureFilterOptions mDefTextureFiltering;

		//default maxAnisotropy
		int mDefAniso;

    public:
        /** Default constructor.
        */
        MaterialManager();

        /** Default destructor.
        */
        virtual ~MaterialManager();

        /** Adds a copy of a material created outside the MaterialManager to the master Material list.
            @note
                MaterialManager copies the Material so there are no memory management issues.
                However note that the Material's internal handle will be regenerated to ensure uniqueness.
            @par
                Note it is usually better to just use the create() method instead.
            @param mat A reference to a manually created Material
            @returns A pointer to the newly created Material based on the one passed in
        */
        Material* add(const Material& mat);


        /** Gets a pointer to a Material by it's numerical handle.
            @remarks
                Numerical handles are assigned on creation of a material, or when a copy is registered
                with the MaterialManager using the add method. Retreiving materials by handle is more
                efficient than doing so by name.
        */
        Material* getByHandle(int handle);

        /** Parses a Material script file passed as a chunk.
        */
        void parseScript(DataChunk& chunk);

        /** Parses all material script files in resource folders & archives.
        */
        void parseAllSources(const String& extension = ".material");

        /** Create implementation required by ResourceManager.
        */
        Resource* create( const String& name );

        /** Create implementation that creates a deferred-load Material, ie one that does not load any resources
            like texture files etc, and does not register itself with SceneManagers until it is actually used.
        */
        Material* createDeferred( const String& name );

        /** Sets the default texture filtering to be used for loaded textures, for when textures are
            loaded automatically (e.g. by Material class) or when 'load' is called with the default
            parameters by the application.
            @note
                The default value is TFO_BILINEAR.
        */
        virtual void setDefaultTextureFiltering(TextureFilterOptions fo);
		// get the default texture filtering
        virtual TextureFilterOptions getDefaultTextureFiltering();

        /** Sets the default anisotropy level to be used for loaded textures, for when textures are
            loaded automatically (e.g. by Material class) or when 'load' is called with the default
            parameters by the application.
            @note
                The default value is 1 (no anisotropy).
        */
		void setDefaultAnisotropy(int maxAniso);
		// get the default maxAnisotropy
		int getDefaultAnisotropy();

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

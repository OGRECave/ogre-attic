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
#ifndef _TextureManager_H__
#define _TextureManager_H__


#include "OgrePrerequisites.h"

#include "OgreResourceManager.h"
#include "OgreTexture.h"
#include "OgreSingleton.h"


namespace Ogre {

    /** Class for loading & managing textures.
        @remarks
            Note that this class is abstract - the particular
            RenderSystem that is in use at the time will create
            a concrete subclass of this. Note that the concrete
            class will be available via the abstract singleton
            obtained from TextureManager::getSingleton(), but
            you should not assume that it is available until you
            have a) initialised Ogre (after selecting a RenderSystem
            and calling initialise from the Root object), and b)
            created at least one window - this may be done at the
            same time as part a if you allow Ogre to autocreate one.
     */
    class _OgreExport TextureManager : public ResourceManager, public Singleton<TextureManager>
    {
    public:

        TextureManager(bool enable32Bit = true) : mIs32Bit(enable32Bit), mDefaultNumMipMaps(0) {}
        virtual ~TextureManager();

        /** Loads a texture from a file.
            @remarks
                Loads a texture from a graphics file (PNG, JPG or TGA). Textures
                will be kept in memory unless resources become short, in which
                case textures with lower priority will be unloaded first. Textures
                of equal priority will be unloaded on a 'least recently used' (LRU)
                basis. Textures will be automatically reloaded when used again
                but this will involve a performance hit.
            @par
                The budget for texture memory can be set through the
                ResourceManager::setMemoryBudget method.
            @param
                filename The file to load (JPG or PNG accepted, also BMP on Windows), 
                or a String identifier in some cases
            @param
                texType The type of texture to load/create, defaults to normal 2D textures
            @param
                numMipMaps The number of pre-filtered mipmaps to generate. If left to default (-1) then
                the TextureManager's default number of mipmaps will be used (see setDefaultNumMipMaps())
            @param
                gamma The gamma adjustment factor to apply to this texture (brightening/darkening)
            @param
                priority The higher the priority, the less likely this texture will be unloaded due to memory limits.
            @see
                ResourceManager::setMemoryBudget

        */
        virtual Texture * load( 
            const String& name, TextureType texType = TEX_TYPE_2D,
            int numMipMaps = -1, Real gamma = 1.0f, int priority = 1 );

        virtual Texture * loadImage( 
            const String &name, const Image &img, 
            TextureType texType = TEX_TYPE_2D,
            int iNumMipMaps = -1, Real gamma = 1.0f, int priority = 1 );
			
		virtual Texture *loadRawData( 
			const String &name, const DataChunk &pData, 
			ushort uWidth, ushort uHeight, PixelFormat eFormat,
            TextureType texType = TEX_TYPE_2D,
            int iNumMipMaps = -1, Real gamma = 1.0f, int priority = 1 );

        /** @copydoc ResourceManager::load */
        virtual void load( Resource *res, int priority = 1 )
        {
            ResourceManager::load( res, priority );
        }

        virtual Resource * create( const String& name )
        {
            return create(name, TEX_TYPE_2D);
        }

        virtual Texture * create( const String& name, TextureType texType) = 0;

        virtual Texture * createAsRenderTarget( const String& name ) = 0;

        virtual Texture * createManual( 
            const String & name,
            TextureType texType,
            uint width,
            uint height,
            uint num_mips,
            PixelFormat format,
            TextureUsage usage ) = 0;

        /** Manually unloads a texture from the loaded set.
        */
        virtual void unload(const String& filename);

        /** Enables / disables 32-bit textures.
        */
        virtual void enable32BitTextures(bool setting = true);

        /** Sets the default number of mipmaps to be used for loaded textures, for when textures are
            loaded automatically (e.g. by Material class) or when 'load' is called with the default
            parameters by the application.
            @note
                The default value is 0.
        */
        virtual void setDefaultNumMipMaps(int num);

        /** Sets the default number of mipmaps to be used for loaded textures.
        */
        virtual int getDefaultNumMipMaps()
        {
            return mDefaultNumMipMaps;
        }

        /** Override standard Singleton retrieval.
            @remarks
                Why do we do this? Well, it's because the Singleton implementation is in a .h file,
                which means it gets compiled into anybody who includes it. This is needed for the Singleton
                template to work, but we actually only want it compiled into the implementation of the
                class based on the Singleton, not all of them. If we don't change this, we get link errors
                when trying to use the Singleton-based class from an outside dll.
            @par
                This method just delegates to the template version anyway, but the implementation stays in this
                single compilation unit, preventing link errors.
        */
        static TextureManager& getSingleton(void);

    protected:
        bool mIs32Bit;
        int mDefaultNumMipMaps;
    };
}// Namespace

#endif

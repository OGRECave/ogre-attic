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

        TextureManager(bool enable32Bit = true);
        virtual ~TextureManager();

        /** Loads a texture from a file.
            @param
                name The file to load, or a String identifier in some cases
            @param
                group The name of the resource group to assign the texture to
            @param
                texType The type of texture to load/create, defaults to normal 2D textures
            @param
                numMipmaps The number of pre-filtered mipmaps to generate. If left to default (-1) then
                the TextureManager's default number of mipmaps will be used (see setDefaultNumMipmaps())
            @param
                gamma The gamma adjustment factor to apply to this texture (brightening/darkening)
        */
        virtual TexturePtr load( 
            const String& name, const String& group, 
            TextureType texType = TEX_TYPE_2D, int numMipmaps = -1, 
            Real gamma = 1.0f);

        /** Loads a texture from an Image object.
            @param
                name The name to give the resulting texture
            @param
                group The name of the resource group to assign the texture to
            @param
                img The Image object which contains the data to load
            @param
                texType The type of texture to load/create, defaults to normal 2D textures
            @param
                numMipmaps The number of pre-filtered mipmaps to generate. If left to default (-1) then
                the TextureManager's default number of mipmaps will be used (see setDefaultNumMipmaps())
            @param
                gamma The gamma adjustment factor to apply to this texture (brightening/darkening)
        */
        virtual TexturePtr loadImage( 
            const String &name, const String& group, const Image &img, 
            TextureType texType = TEX_TYPE_2D,
            int iNumMipmaps = -1, Real gamma = 1.0f);
			
        /** Loads a texture from a raw data stream.
            @param
                name The name to give the resulting texture
            @param
                group The name of the resource group to assign the texture to
            @param
                stream Incoming data stream
            @param
                width, height The dimensions of the texture
            @param
                format The format of the data being passed in; the manager reserves
                the right to create a different format for the texture if the 
                original format is not available in this context.
            @param
                texType The type of texture to load/create, defaults to normal 2D textures
            @param
                numMipmaps The number of pre-filtered mipmaps to generate. If left to default (-1) then
                the TextureManager's default number of mipmaps will be used (see setDefaultNumMipmaps())
            @param
                gamma The gamma adjustment factor to apply to this texture (brightening/darkening)
        */
        virtual TexturePtr loadRawData(const String &name, const String& group,
            DataStreamPtr& stream, ushort uWidth, ushort uHeight, 
            PixelFormat format, TextureType texType = TEX_TYPE_2D, 
            int iNumMipmaps = -1, Real gamma = 1.0f);

		/** Create a manual texture with specified width, height and depth (not loaded from a file).
            @param
                name The name to give the resulting texture
            @param
                group The name of the resource group to assign the texture to
            @param
                img The Image object which contains the data to load
            @param
                texType The type of texture to load/create, defaults to normal 2D textures
            @param
                width, height, depth The dimensions of the texture
            @param
                numMipmaps The number of pre-filtered mipmaps to generate. If left to default (-1) then
                the TextureManager's default number of mipmaps will be used (see setDefaultNumMipmaps())
            @param
                format The internal format you wish to request; the manager reserves
                the right to create a different format if the one you select is
                not available in this context.
            @param
                usage The kind of usage this texture is intended for
            @param
                loader If you intend the contents of the manual texture to be 
                regularly updated, to the extent that you don't need to recover 
                the contents if the texture content is lost somehow, you can leave
                this parameter as 0. However, if you intend to populate the
                texture only once, then you should implement ManualResourceLoader
                and pass a pointer to it in this parameter; this means that if the
                manual texture ever needs to be reloaded, the ManualResourceLoader
                will be called to do it.
        */
        virtual TexturePtr createManual(const String & name, const String& group,
            TextureType texType, uint width, uint height, uint depth, 
			uint num_mips, PixelFormat format, TextureUsage usage = TU_DEFAULT, ManualResourceLoader* loader = 0 );
			
        /** Create a manual texture with a depth of 1 (not loaded from a file).
            @param
                name The name to give the resulting texture
            @param
                group The name of the resource group to assign the texture to
            @param
                img The Image object which contains the data to load
            @param
                texType The type of texture to load/create, defaults to normal 2D textures
            @param
                width, height The dimensions of the texture
            @param
                numMipmaps The number of pre-filtered mipmaps to generate. If left to default (-1) then
                the TextureManager's default number of mipmaps will be used (see setDefaultNumMipmaps())
            @param
                format The internal format you wish to request; the manager reserves
                the right to create a different format if the one you select is
                not available in this context.
            @param
                usage The kind of usage this texture is intended for
            @param
                loader If you intend the contents of the manual texture to be 
                regularly updated, to the extent that you don't need to recover 
                the contents if the texture content is lost somehow, you can leave
                this parameter as 0. However, if you intend to populate the
                texture only once, then you should implement ManualResourceLoader
                and pass a pointer to it in this parameter; this means that if the
                manual texture ever needs to be reloaded, the ManualResourceLoader
                will be called to do it.
        */
        TexturePtr createManual(const String & name, const String& group,
            TextureType texType, uint width, uint height, uint num_mips,
            PixelFormat format, TextureUsage usage = TU_DEFAULT, ManualResourceLoader* loader = 0 )
		{
			return createManual(name, group, texType, width, height, 1, 
				num_mips, format, usage, loader);
		}

        /** Enables / disables 32-bit textures.
        */
        virtual void enable32BitTextures(bool setting = true);

        /** Sets the default number of mipmaps to be used for loaded textures, for when textures are
            loaded automatically (e.g. by Material class) or when 'load' is called with the default
            parameters by the application.
            @note
                The default value is 0.
        */
        virtual void setDefaultNumMipmaps(int num);

        /** Sets the default number of mipmaps to be used for loaded textures.
        */
        virtual int getDefaultNumMipmaps()
        {
            return mDefaultNumMipmaps;
        }

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
        static TextureManager& getSingleton(void);
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
        static TextureManager* getSingletonPtr(void);

    protected:

        bool mIs32Bit;
        int mDefaultNumMipmaps;
    };
}// Namespace

#endif

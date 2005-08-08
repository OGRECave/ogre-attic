/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#include "OgreStableHeaders.h"
#include "OgreTextureManager.h"
#include "OgreException.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    template<> TextureManager* Singleton<TextureManager>::ms_Singleton = 0;
    TextureManager* TextureManager::getSingletonPtr(void)
    {
        return ms_Singleton;
    }
    TextureManager& TextureManager::getSingleton(void)
    {  
        assert( ms_Singleton );  return ( *ms_Singleton );  
    }
    TextureManager::TextureManager(bool enable32Bit)
         : mIs32Bit(enable32Bit), mDefaultNumMipmaps(MIP_UNLIMITED)
    {
        mResourceType = "Texture";
        mLoadOrder = 75.0f;

        // Subclasses should register (when this is fully constructed)
    }
    //-----------------------------------------------------------------------
    TextureManager::~TextureManager()
    {
        // subclasses should unregister with resource group manager

    }
    //-----------------------------------------------------------------------
    TexturePtr TextureManager::load(const String &name, const String& group,
        TextureType texType, int numMipmaps, Real gamma)
    {
        TexturePtr tex = getByName(name);

        if(tex.isNull())
        {
            tex = create(name, group);
            tex->setTextureType(texType);
            tex->setNumMipmaps((numMipmaps == -1)? mDefaultNumMipmaps :
				static_cast<size_t>(numMipmaps));
            tex->setGamma(gamma);
            tex->enable32Bit(mIs32Bit);
        }
        tex->load();

        return tex;
    }

    //-----------------------------------------------------------------------
    TexturePtr TextureManager::loadImage( const String &name, const String& group,
        const Image &img, TextureType texType, int numMipmaps, Real gamma)
    {
        TexturePtr tex = create(name, group);

        tex->setTextureType(texType);
        tex->setNumMipmaps((numMipmaps == -1)? mDefaultNumMipmaps :
			static_cast<size_t>(numMipmaps));
        tex->setGamma(gamma);
        tex->enable32Bit(mIs32Bit);
        tex->loadImage(img);

        return tex;
    }
    //-----------------------------------------------------------------------
    TexturePtr TextureManager::loadRawData(const String &name, const String& group,
        DataStreamPtr& stream, ushort uWidth, ushort uHeight, 
        PixelFormat format, TextureType texType, 
        int numMipmaps, Real gamma)
	{
        TexturePtr tex = create(name, group);

        tex->setTextureType(texType);
        tex->setNumMipmaps((numMipmaps == -1)? mDefaultNumMipmaps :
			static_cast<size_t>(numMipmaps));
        tex->setGamma(gamma);
        tex->enable32Bit(mIs32Bit);
		tex->loadRawData(stream, uWidth, uHeight, format);
		
        return tex;
	}
    //-----------------------------------------------------------------------
    TexturePtr TextureManager::createManual(const String & name, const String& group,
        TextureType texType, uint width, uint height, uint depth, int numMipmaps,
        PixelFormat format, int usage, ManualResourceLoader* loader)
    {
        TexturePtr ret = create(name, group, true, loader);
        ret->setTextureType(texType);
        ret->setWidth(width);
        ret->setHeight(height);
		ret->setDepth(depth);
        ret->setNumMipmaps((numMipmaps == -1)? mDefaultNumMipmaps :
			static_cast<size_t>(numMipmaps));
        ret->setFormat(format);
        ret->setUsage(usage);
        ret->enable32Bit(mIs32Bit);

		// Auto-load RTT
		if (usage & TU_RENDERTARGET)
			ret->load();
        return ret;
    }
    //-----------------------------------------------------------------------
    void TextureManager::enable32BitTextures( bool setting )
    {
        // Reload all textures
        for( ResourceMap::iterator it = mResources.begin(); it != mResources.end(); ++it )
        {
            ((TexturePtr)it->second)->unload();
            ((TexturePtr)it->second)->enable32Bit(setting);
            ((TexturePtr)it->second)->load();
            mIs32Bit = setting;
        }
    }
    //-----------------------------------------------------------------------
    void TextureManager::setDefaultNumMipmaps( size_t num )
    {
        mDefaultNumMipmaps = num;
    }
}

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
#include "OgreTextureManager.h"
#include "OgreException.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    template<> TextureManager* Singleton<TextureManager>::ms_Singleton = 0;
    //-----------------------------------------------------------------------
    TextureManager::~TextureManager(){}
    //-----------------------------------------------------------------------
    Texture * TextureManager::load(
        const String &name, TextureType texType, int numMipMaps, 
        Real gamma, int priority )
    {
        Texture* tex = (Texture*)getByName( name );

        if( tex == NULL )
        {
            tex = (Texture*)create( name, texType );

            if (numMipMaps == -1)
                tex->setNumMipMaps(mDefaultNumMipMaps);
            else
                tex->setNumMipMaps(numMipMaps);

            tex->setGamma( gamma );
            tex->enable32Bit( mIs32Bit );

            ResourceManager::load( tex, priority );
        }

        return tex;
    }

    //-----------------------------------------------------------------------
    Texture * TextureManager::loadImage( 
        const String &name, const Image &img, TextureType texType,
        int iNumMipMaps /* = -1 */, Real gamma /* = 1.0f  */, int priority /* = 1 */ )
    {
        Texture *tex = (Texture*)create( name, texType );

        if( iNumMipMaps == -1 )
            tex->setNumMipMaps( mDefaultNumMipMaps );
        else
            tex->setNumMipMaps( iNumMipMaps );

        tex->setGamma( gamma );        
        tex->loadImage( img );

		std::pair<ResourceMap::iterator, bool> res = mResources.insert(
			ResourceMap::value_type( tex->getName(), tex));
		if (!res.second)
		{
			// Key was already used
			Except(Exception::ERR_DUPLICATE_ITEM, "A texture with the name " + tex->getName() + 
				" was already loaded.", "TextureManager::loadImage");
		}

        return tex;
    }
    //-----------------------------------------------------------------------
	Texture * TextureManager::loadRawData( 
		const String &name, const DataChunk &pData, 
		ushort uWidth, ushort uHeight, PixelFormat eFormat,
		TextureType texType,
		int iNumMipMaps, Real gamma, int priority )
	{
		Texture *tex = (Texture*)create( name, texType );
        if( iNumMipMaps == -1 )
            tex->setNumMipMaps( mDefaultNumMipMaps );
        else
            tex->setNumMipMaps( iNumMipMaps );

        tex->setGamma( gamma );        
		tex->loadRawData( pData, uWidth, uHeight, eFormat);
		
		std::pair<ResourceMap::iterator, bool> res = mResources.insert(
			ResourceMap::value_type( tex->getName(), tex));
		if (!res.second)
		{
			// Key was already used
			Except(Exception::ERR_DUPLICATE_ITEM, "A texture with the name " + tex->getName() + 
				" was already loaded.", "TextureManager::loadRawData");
		}

        return tex;
	}
    //-----------------------------------------------------------------------
    void TextureManager::unload( const String& filename )
    {
        Resource* res = getByName( filename );
        ResourceManager::unload( res );
    }
    //-----------------------------------------------------------------------
    void TextureManager::enable32BitTextures( bool setting )
    {
        // Reload all textures
        for( ResourceMap::iterator it = mResources.begin(); it != mResources.end(); ++it )
        {
            ((Texture*)it->second)->unload();
            ((Texture*)it->second)->enable32Bit(setting);
            ((Texture*)it->second)->load();
            mIs32Bit = setting;
        }
    }
    //-----------------------------------------------------------------------
    void TextureManager::setDefaultNumMipMaps( int num )
    {
        mDefaultNumMipMaps = num;
    }
    //-----------------------------------------------------------------------
    TextureManager& TextureManager::getSingleton(void)
    {
        return Singleton<TextureManager>::getSingleton();
    }
}

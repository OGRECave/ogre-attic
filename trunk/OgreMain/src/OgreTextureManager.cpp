/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreTextureManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    template<> TextureManager* Singleton<TextureManager>::ms_Singleton = 0;
    //-----------------------------------------------------------------------
    TextureManager::~TextureManager(){}
    //-----------------------------------------------------------------------
    Texture* TextureManager::load(String filename, int numMipMaps, Real gamma, int priority)
    {
        Texture* tex = (Texture*)getByName(filename);
        if (tex == 0)
        {
            tex = (Texture*)create(filename);
            if (numMipMaps == -1)
                tex->setNumMipMaps(mDefaultNumMipMaps);
            else
                tex->setNumMipMaps(numMipMaps);
            tex->setGamma(gamma);
            tex->enable32Bit(mIs32Bit);
            ResourceManager::load(tex, priority);
        }
        return tex;
    }
    //-----------------------------------------------------------------------
    void TextureManager::loadRawRGB(String name, void* buffer, int width, int height, int numMipMaps, Real gamma)
    {
        Texture* tex = (Texture*)create(name);
        if (numMipMaps == -1)
            tex->setNumMipMaps(mDefaultNumMipMaps);
        else
            tex->setNumMipMaps(numMipMaps);
        tex->setGamma(gamma);
        tex->enable32Bit(mIs32Bit);
        tex->loadRawRGB(buffer, width, height);

        mResources[tex->getName()] = tex;
    }
    //-----------------------------------------------------------------------
    void TextureManager::unload(String filename)
    {
        Resource* res = getByName(filename);
        ResourceManager::unload(res);
    }
    //-----------------------------------------------------------------------
    void TextureManager::enable32BitTextures(bool setting)
    {
        // Reload all textures
        for( ResourceMap::iterator it = mResources.begin(); it != mResources.end(); ++it )
        {
            ((Texture*)it->second)->unload();
            ((Texture*)it->second)->enable32Bit(setting);
            ((Texture*)it->second)->load();
        }
    }
    //-----------------------------------------------------------------------
    void TextureManager::setDefaultNumMipMaps(int num)
    {
        mDefaultNumMipMaps = num;
    }
    //-----------------------------------------------------------------------
    TextureManager& TextureManager::getSingleton(void)
    {
        return Singleton<TextureManager>::getSingleton();
    }


}

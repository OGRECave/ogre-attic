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

#include "OgreSDLTextureManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------------
    SDLTextureManager::SDLTextureManager()
        : TextureManager()
    {
    }
    //-----------------------------------------------------------------------------
    SDLTextureManager::~SDLTextureManager()
    {
        this->unloadAndDestroyAll();
    }
    //-----------------------------------------------------------------------------
    Resource* SDLTextureManager::create( const String& name)
    {
        SDLTexture* t = new SDLTexture(name);
        t->enable32Bit(mIs32Bit);
        return t;
    }
    //-----------------------------------------------------------------------------
    Texture* SDLTextureManager::createManual( const String& name, uint width,
        uint height, uint num_mips, PixelFormat format, TextureUsage usage )
    {
        SDLTexture* t = new SDLTexture(name, width, height, num_mips, format, usage);
        t->enable32Bit(mIs32Bit);
        return t;
    }

    //-----------------------------------------------------------------------------
    void SDLTextureManager::unloadAndDestroyAll()
    {
        // Unload & delete resources in turn
        for (ResourceMap::iterator i = mResources.begin(); i != mResources.end(); ++i)
        {
            i->second->unload();
            delete i->second;
        }

        // Empty the list
        mResources.clear();
    }
}

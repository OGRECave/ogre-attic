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

#include "OgreGLTextureManager.h"

namespace Ogre {
    //-----------------------------------------------------------------------------
    GLTextureManager::GLTextureManager(GLSupport& support)
        : TextureManager(), mGLSupport(support), mWarningTextureID(0)
    {
        // register with group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);

		createWarningTexture();
    }
    //-----------------------------------------------------------------------------
    GLTextureManager::~GLTextureManager()
    {
        // unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
		// Delete warning texture
		glDeleteTextures(1, &mWarningTextureID);
    }
    //-----------------------------------------------------------------------------
    Resource* GLTextureManager::createImpl(const String& name, ResourceHandle handle, 
        const String& group, bool isManual, ManualResourceLoader* loader, 
        const NameValuePairList* createParams)
    {
        return new GLTexture(this, name, handle, group, isManual, loader, mGLSupport);
    }

	//-----------------------------------------------------------------------------
	void GLTextureManager::createWarningTexture()
	{
		// Generate warning texture
		size_t width = 8;
		size_t height = 8;
		uint32 *data = new uint32[width*height];		// 0xXXRRGGBB
		// Yellow/black stripes
		for(size_t y=0; y<height; ++y)
		{
			for(size_t x=0; x<width; ++x)
			{
				data[y*width+x] = (((x+y)%8)<4)?0x000000:0xFFFF00;
			}
		}
		// Create GL resource
        glGenTextures(1, &mWarningTextureID);
		glBindTexture(GL_TEXTURE_2D, mWarningTextureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, (void*)data);
		// Free memory
		delete [] data;
	}


}

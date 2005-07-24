/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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
#include "OgreGLRenderTexture.h"
#include "OgreGLPixelFormat.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreGLHardwarePixelBuffer.h"

namespace Ogre {


//-----------------------------------------------------------------------------

template<> GLRTTManager* Singleton<GLRTTManager>::ms_Singleton = 0;
GLRTTManager::~GLRTTManager()
{
}
//-----------------------------------------------------------------------------  
    GLRenderTexture::GLRenderTexture(const String &name, const GLSurfaceDesc &target):
        RenderTexture(target.buffer, target.zoffset)
    {
        mName = name;
    }
    GLRenderTexture::~GLRenderTexture()
    {
    }
//-----------------------------------------------------------------------------  
    GLCopyingRenderTexture::GLCopyingRenderTexture(GLCopyingRTTManager *manager, const String &name, const GLSurfaceDesc &target):
        GLRenderTexture(name, target)
    {
    }
    void GLCopyingRenderTexture::getCustomAttribute(const String& name, void* pData)
    {
        if(name=="TARGET")
        {
			GLSurfaceDesc &target = *static_cast<GLSurfaceDesc*>(pData);
			target.buffer = static_cast<GLHardwarePixelBuffer*>(mBuffer);
			target.zoffset = mZOffset;
        }
    }
//-----------------------------------------------------------------------------  
    GLCopyingRTTManager::GLCopyingRTTManager()
    {
    }  
    GLCopyingRTTManager::~GLCopyingRTTManager()
    {
    }

    RenderTexture *GLCopyingRTTManager::createRenderTexture(const String &name, const GLSurfaceDesc &target)
    {
        return new GLCopyingRenderTexture(this, name, target);
    }
    
    bool GLCopyingRTTManager::checkFormat(PixelFormat format) 
    { 
        return true; 
    }

    void GLCopyingRTTManager::bind(RenderTarget *target)
    {
        // Nothing to do here
    }

    void GLCopyingRTTManager::unbind(RenderTarget *target)
    {
        // Copy on unbind
        GLSurfaceDesc surface;
		surface.buffer = 0;
        target->getCustomAttribute("TARGET", &surface);
        if(surface.buffer)
            static_cast<GLTextureBuffer*>(surface.buffer)->copyFromFramebuffer(surface.zoffset);
    }
	//---------------------------------------------------------------------------------------------

}


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

#include "OgreGLFBORenderTexture.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"

namespace Ogre {

    GLFBORenderTexture::GLFBORenderTexture( const String & name, unsigned int width, unsigned int height,
        TextureType texType, PixelFormat internalFormat, 
        const NameValuePairList *miscParams ):
        GLRenderTexture(name, width, height, texType, internalFormat, miscParams),
        mFB(0), mDepthRB(0), mStencilRB(0)
    {
        createFBO();
        
        // Register with render system
        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        rs->_registerFBO(this, mFB);     
    }
    
    GLFBORenderTexture::~GLFBORenderTexture()
    {
        // Unregister from render system
        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        rs->_unregisterFBO(this);  
        
        // Destroy depth
        glDeleteRenderbuffersEXT(1, &mDepthRB);
        // Destroy stencil
        glDeleteRenderbuffersEXT(1, &mStencilRB);
        // Destroy frame buffer object
        glDeleteFramebuffersEXT(1, &mFB);
    }

    void GLFBORenderTexture::_copyToTexture()
    {
        // Do nothing
    }
    
    /** Try a certain FBO format, and return the status. Also sets mDepthRB and mStencilRB.
        @returns GL_FRAMEBUFFER_UNSUPPORTED_EXT if this format combo is unsupported
                 GL_FRAMEBUFFER_INCOMPLETE_EXT  if this framebuffer is incomplete (programming error, will fail on all hardware)
                 GL_FRAMEBUFFER_COMPLETE_EXT    if it works
    */
    GLuint GLFBORenderTexture::tryFormat(GLenum depthFormat, GLenum stencilFormat)
    {
        GLuint status;
        if(depthFormat != GL_NONE)
        {
            /// Generate depth renderbuffer
            glGenRenderbuffersEXT(1, &mDepthRB);
            /// Bind it to FBO
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mDepthRB);
            
            /// Allocate storage for depth buffer
            glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, depthFormat,
                                mTexture->getWidth(), mTexture->getHeight());
            
            /// Attach depth
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                    GL_RENDERBUFFER_EXT, mDepthRB);
        }

        if(stencilFormat != GL_NONE)
        {
            /// Generate stencil renderbuffer
            glGenRenderbuffersEXT(1, &mStencilRB);
            /// Bind it to FBO
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mStencilRB);
            
            /// Allocate storage for stencil buffer
            glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, stencilFormat,
                                mTexture->getWidth(), mTexture->getHeight()); 
            
            /// Attach stencil
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                            GL_RENDERBUFFER_EXT, mStencilRB);
        }
        
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
#if 0
        /// Print some debug info
        std::stringstream str;
        str << std::hex << "depth=" << depthFormat << " stencil=" << stencilFormat << " -> status=" << status;
        LogManager::getSingleton().logMessage("[GL] : FBO try " + str.str());
#endif   
        /// If status is negative, clean up
        if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
        {
            // Detach and destroy
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_NONE, 0);
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_NONE, 0);
            glDeleteRenderbuffersEXT(1, &mDepthRB);
            mDepthRB = 0;
            glDeleteRenderbuffersEXT(1, &mStencilRB);
            mStencilRB = 0;
        }
        
        return status;
    }
    
    /// Stencil and depth formats to be tried
    GLenum stencilFormats[]={
        GL_STENCIL_INDEX,      // Default of driver first
        GL_STENCIL_INDEX_EXT,
        GL_STENCIL_INDEX16_EXT,
        GL_STENCIL_INDEX8_EXT,
        GL_STENCIL_INDEX4_EXT,
        GL_STENCIL_INDEX1_EXT,
        GL_NONE                    // No stencil
    };
    #define STENCILFORMAT_COUNT (sizeof(stencilFormats)/sizeof(GLenum))
    GLenum depthFormats[]={
        GL_DEPTH_COMPONENT,      // Default of driver first
        GL_DEPTH_COMPONENT24,    // Prefer 24 bit depth
        GL_DEPTH_COMPONENT32,
        GL_DEPTH_COMPONENT16
    };
    #define DEPTHFORMAT_COUNT (sizeof(depthFormats)/sizeof(GLenum))
        
    void GLFBORenderTexture::createFBO()
    {
        int depth=0, stencil=0;

        // Texture to bind to
        GLuint texture = static_cast<GLTexture*>(mTexture.get())->getGLID();
        
        /// Generate framebuffer object
        glGenFramebuffersEXT(1, &mFB);
        
        /// Bind it to context
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFB);
        
        // Attach texture level 0 [colour attachment 0]
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                             GL_TEXTURE_2D, texture, 0);

        // Check status
        GLuint status;
        bool done = false;
        while(!done)
        {
            status = tryFormat(depthFormats[depth], stencilFormats[stencil]);
            switch(status)
            {
            case GL_FRAMEBUFFER_COMPLETE_EXT:
                done = true;
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
                // Unsupported -- try next
                stencil++;
                if(stencil == STENCILFORMAT_COUNT)
                {
                    stencil = 0;
                    depth ++;
                    if(depth == DEPTHFORMAT_COUNT)
                    {
                        // Ran out of options
                        done = true;
                        LogManager::getSingleton().logMessage("[GL] : All framebuffer formats with this texture internal format unsupported");
                    }
                }
                break;
            default:
                LogManager::getSingleton().logMessage("[GL] : Framebuffer incomplete or other FBO status error "+StringConverter::toString(status));
                done = true;
                break;
            }
        }
        // Bind normal frame buffer
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        
        // Report status
        if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
        {
            // Clean up and throw exception
            glDeleteRenderbuffersEXT(1, &mDepthRB);
            glDeleteRenderbuffersEXT(1, &mStencilRB);
            glDeleteFramebuffersEXT(1, &mFB);
            OGRE_EXCEPT(
                Exception::ERR_RENDERINGAPI_ERROR, 
                "Could not create FBO with desired internal format",
                "GLFBORenderTexture::createFBO" );
        }
    }

}

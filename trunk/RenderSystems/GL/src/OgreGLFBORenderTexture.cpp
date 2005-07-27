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
#include "OgreGLPixelFormat.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreGLHardwarePixelBuffer.h"
namespace Ogre {

//-----------------------------------------------------------------------------    
    GLFBORenderTexture::GLFBORenderTexture(GLFBOManager *manager, const String &name,
        const GLSurfaceDesc &target):
        GLRenderTexture(name, target),
        mFB(manager)
    {
        // Bind target to surface 0 and initialise
        mFB.bindSurface(0, target);
        mFB.initialise();
        // Get attributes
        mWidth = mFB.getWidth();
        mHeight = mFB.getHeight();
    }

    void GLFBORenderTexture::getCustomAttribute(const String& name, void* pData)
    {
        if(name=="FBO")
        {
            *static_cast<GLFrameBufferObject **>(pData) = &mFB;
        }
    }
        
//-----------------------------------------------------------------------------
    GLFrameBufferObject::GLFrameBufferObject(GLFBOManager *manager):
        mManager(manager)
    {
        /// Generate framebuffer object
        glGenFramebuffersEXT(1, &mFB);
        /// Initialise state
        mDepth.buffer=0;
        mStencil.buffer=0;
        for(size_t x=0; x<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++x)
        {
            mColour[x].buffer=0;
        }
    }
    GLFrameBufferObject::~GLFrameBufferObject()
    {
        mManager->releaseRenderBuffer(mDepth);
        mManager->releaseRenderBuffer(mStencil);
        /// Delete framebuffer object
        glDeleteFramebuffersEXT(1, &mFB);        
    }
    void GLFrameBufferObject::bindSurface(size_t attachment, const GLSurfaceDesc &target)
    {
        assert(attachment < OGRE_MAX_MULTIPLE_RENDER_TARGETS);
        mColour[attachment] = target;
    }
    void GLFrameBufferObject::unbindSurface(size_t attachment)
    {
        assert(attachment < OGRE_MAX_MULTIPLE_RENDER_TARGETS);
        mColour[attachment].buffer = 0;
    }
    void GLFrameBufferObject::initialise()
    {
        mManager->releaseRenderBuffer(mDepth);
        mManager->releaseRenderBuffer(mStencil);
        /// First buffer must be bound
        if(!mColour[0].buffer)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
            "Attachment 0 must have surface attached",
		 	"GLFrameBufferObject::initialise");
        }
        /// Bind FBO to frame buffer
        bind();
        /// Store basic stats
        size_t width = mColour[0].buffer->getWidth();
        size_t height = mColour[0].buffer->getHeight();
        GLuint format = mColour[0].buffer->getGLFormat();
        PixelFormat ogreFormat = mColour[0].buffer->getFormat();
        /// Bind all attachment points to frame buffer
        for(size_t x=0; x<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++x)
        {
            if(mColour[x].buffer)
            {
                if(mColour[x].buffer->getWidth() != width || mColour[x].buffer->getHeight() != height)
                {
                    std::stringstream ss;
                    ss << "Attachment " << x << " has incompatible size ";
                    ss << mColour[x].buffer->getWidth() << "x" << mColour[x].buffer->getHeight();
                    ss << ". It must be of the same as the size of surface 0, ";
                    ss << width << "x" << height;
                    ss << ".";
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, ss.str(), "GLFrameBufferObject::initialise");
                }
                if(mColour[x].buffer->getGLFormat() != format)
                {
                    std::stringstream ss;
                    ss << "Attachment " << x << " has incompatible format.";
                    OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, ss.str(), "GLFrameBufferObject::initialise");
                }
                mColour[x].buffer->bindToFramebuffer(GL_COLOR_ATTACHMENT0_EXT+x, mColour[x].zoffset);
            }
            else
            {
                // Detach
                glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT+x,
                    GL_RENDERBUFFER_EXT, 0);
            }
        }
        /// Find suitable depth and stencil format that is compatible with colour format
        GLenum depthFormat, stencilFormat;
        mManager->getBestDepthStencil(ogreFormat, &depthFormat, &stencilFormat);
        
        /// Request surfaces
        mDepth = mManager->requestRenderBuffer(depthFormat, width, height);
        mStencil = mManager->requestRenderBuffer(stencilFormat, width, height);
        
        /// Attach/detach surfaces
        if(mDepth.buffer)
        {
            mDepth.buffer->bindToFramebuffer(GL_DEPTH_ATTACHMENT_EXT, mDepth.zoffset);
        }
        else
        {
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                GL_RENDERBUFFER_EXT, 0);
        }
        if(mStencil.buffer)
        {
            mStencil.buffer->bindToFramebuffer(GL_STENCIL_ATTACHMENT_EXT, mDepth.zoffset);
        }
        else
        {
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                GL_RENDERBUFFER_EXT, 0);
        }
        
        /// Check status
        GLuint status;
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        
        /// Do glDrawBuffer calls
        /// If MRT enabled
        /// TODO
        
        /// Bind main buffer
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        
        switch(status)
        {
        case GL_FRAMEBUFFER_COMPLETE_EXT:
            // All is good
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
            "All framebuffer formats with this texture internal format unsupported",
		 	"GLFrameBufferObject::initialise");
        default:
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
            "Framebuffer incomplete or other FBO status error",
		 	"GLFrameBufferObject::initialise");
        }
        
    }
    void GLFrameBufferObject::bind()
    {
        /// Bind it to FBO
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFB);
    }
    size_t GLFrameBufferObject::getWidth()
    {
        assert(mColour[0].buffer);
        return mColour[0].buffer->getWidth();
    }
    size_t GLFrameBufferObject::getHeight()
    {
        assert(mColour[0].buffer);
        return mColour[0].buffer->getHeight();
    }
    PixelFormat GLFrameBufferObject::getFormat()
    {
        assert(mColour[0].buffer);
        return mColour[0].buffer->getFormat();
    }
//-----------------------------------------------------------------------------    

/// Size of probe texture
#define PROBE_SIZE 256
/// Stencil and depth formats to be tried
GLenum stencilFormats[]={
    GL_NONE,                    // No stencil
    GL_STENCIL_INDEX1_EXT,
    GL_STENCIL_INDEX4_EXT,
    GL_STENCIL_INDEX8_EXT,
    GL_STENCIL_INDEX16_EXT
};
size_t stencilBits[] = {
    0, 1, 4, 8, 16
};
#define STENCILFORMAT_COUNT (sizeof(stencilFormats)/sizeof(GLenum))
GLenum depthFormats[]={
    GL_NONE,
    GL_DEPTH_COMPONENT16,
    GL_DEPTH_COMPONENT24,    // Prefer 24 bit depth
    GL_DEPTH_COMPONENT32
};
size_t depthBits[] = {
    0,16,24,32
};
#define DEPTHFORMAT_COUNT (sizeof(depthFormats)/sizeof(GLenum))
    
	GLFBOManager::GLFBOManager(bool atimode):
		mATIMode(atimode)
    {
        detectFBOFormats();
    }

	GLFBOManager::~GLFBOManager()
	{
		if(!mRenderBufferMap.empty())
		{
			LogManager::getSingleton().logMessage("GL: Warning! GLFBOManager destructor called, but not all renderbuffers were released.");
		}
	}

    /** Try a certain FBO format, and return the status. Also sets mDepthRB and mStencilRB.
        @returns true    if this combo is supported
                 false   if this combo is not supported
    */
    GLuint GLFBOManager::_tryFormat(GLenum depthFormat, GLenum stencilFormat)
    {
        GLuint status, depthRB, stencilRB;
        bool failed = false; // flag on GL errors
        if(depthFormat != GL_NONE)
        {
            /// Generate depth renderbuffer
            glGenRenderbuffersEXT(1, &depthRB);
            /// Bind it to FBO
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthRB);
            
            /// Allocate storage for depth buffer
            glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, depthFormat,
                                PROBE_SIZE, PROBE_SIZE);
            
            /// Attach depth
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                    GL_RENDERBUFFER_EXT, depthRB);
        }

        if(stencilFormat != GL_NONE)
        {
            /// Generate stencil renderbuffer
            glGenRenderbuffersEXT(1, &stencilRB);
            /// Bind it to FBO
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, stencilRB);
            glGetError(); // NV hack
            /// Allocate storage for stencil buffer
            glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, stencilFormat,
                                PROBE_SIZE, PROBE_SIZE); 
            if(glGetError() != GL_NO_ERROR) // NV hack
                failed = true;
            /// Attach stencil
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT,
                            GL_RENDERBUFFER_EXT, stencilRB);
            if(glGetError() != GL_NO_ERROR) // NV hack
                failed = true;
        }
        
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        /// If status is negative, clean up
        // Detach and destroy
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_NONE, 0);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_NONE, 0);
        glDeleteRenderbuffersEXT(1, &depthRB);
        glDeleteRenderbuffersEXT(1, &stencilRB);
        
        return status == GL_FRAMEBUFFER_COMPLETE_EXT && !failed;
    }
    
    /** Detect which internal formats are allowed as RTT
        Also detect what combinations of stencil and depth are allowed with this internal
        format.
    */
    void GLFBOManager::detectFBOFormats()
    {
        // Try all formats, and report which ones work as target
        GLuint fb, tid;
        GLenum target = GL_TEXTURE_2D;
        
        for(size_t x=0; x<PF_COUNT; ++x)
        {
            mProps[x].valid = false;

			// No test for compressed formats
			if(PixelUtil::isCompressed((PixelFormat)x))
				continue;

			// Buggy ATI cards *crash* on non-RGB(A) formats
			int depths[4];
			PixelUtil::getBitDepths((PixelFormat)x, depths);
			if(mATIMode && (!depths[0] || !depths[1] || !depths[2]))
				continue;

			// Fetch GL format token
			GLenum fmt = GLPixelUtil::getGLInternalFormat((PixelFormat)x);
            if(fmt == GL_NONE && x!=0)
                continue;
            
            // Create and attach texture
            glGenTextures(1, &tid);
		    glBindTexture(target, tid);
            glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
            if(x)
            {
                // 2D
                glTexImage2D(target, 0, fmt, PROBE_SIZE, PROBE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            }
            // Create and attach framebuffer
            glGenFramebuffersEXT(1, &fb);
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
            if(x)
            {
                // x==0 is the format without colour buffer. In this case, don't bind the 
                // texture.
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                                target, tid, 0);
            }
            // Check status
            GLuint status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
            //std::cerr << std::hex << fmt << " " << status << std::endl;
            if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
            {
                mProps[x].valid = true;
                //std::cerr << PixelUtil::getFormatName((PixelFormat)x) << " ";
                
                // Continue detection
                size_t depth=0, stencil=0;
                while(true)
                {
                    if(_tryFormat(depthFormats[depth], stencilFormats[stencil]))
                    {
                        /// Add mode to allowed modes
                        //std::cerr << "D" << depthBits[depth] << "S" << stencilBits[stencil] << " ";
                        FormatProperties::Mode mode;
                        mode.depth = depth;
                        mode.stencil = stencil;
                        mProps[x].modes.push_back(mode);
                    }
                    /// Try next combo
                    stencil++;
                    if(stencil == STENCILFORMAT_COUNT)
                    {
                        stencil = 0;
                        depth ++;
                        if(depth == DEPTHFORMAT_COUNT)
                        {
                            // We're done
                            break;
                        }
                    }
                }
                //std::cerr << std::endl;
            }
            // Delete texture and framebuffer
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
            glDeleteFramebuffersEXT(1, &fb);
            glDeleteTextures(1, &tid);
        }
        
        std::string fmtstring;
        for(size_t x=0; x<PF_COUNT; ++x)
        {
            if(mProps[x].valid)
                fmtstring += PixelUtil::getFormatName((PixelFormat)x)+" ";
        }
        LogManager::getSingleton().logMessage("[GL] : Valid FBO targets " + fmtstring);
    }
    void GLFBOManager::getBestDepthStencil(GLenum internalFormat, GLenum *depthFormat, GLenum *stencilFormat)
    {
        const FormatProperties &props = mProps[internalFormat];
        /// Decide what stencil and depth formats to use
        /// [best supported for internal format]
        size_t bestmode=0;
        int bestscore=-1;
        for(size_t mode=0; mode<props.modes.size(); mode++)
        {
#if 0
            /// Always prefer D24S8
            if(stencilBits[props.modes[mode].stencil]==8 &&
                depthBits[props.modes[mode].depth]==24)
            {
                bestmode = mode;
                break;
            }
#endif
            int desirability = 0;
            /// Find most desirable mode
            /// desirability == 0            if no depth, no stencil
            /// desirability == 1000...2000  if no depth, stencil
            /// desirability == 2000...3000  if depth, no stencil
            /// desirability == 3000+        if depth and stencil
            /// beyond this, the total numer of bits (stencil+depth) is maximised
            if(props.modes[mode].stencil)
                desirability += 1000;
            if(props.modes[mode].depth)
                desirability += 2000;
            if(depthBits[props.modes[mode].depth]==24) // Prefer 24 bit for now
                desirability += 500;
            desirability += stencilBits[props.modes[mode].stencil] + depthBits[props.modes[mode].depth];
            
            if(desirability>bestscore)
            {
                bestscore = desirability;
                bestmode = mode;
            }
        }
        *depthFormat = depthFormats[props.modes[bestmode].depth];
        *stencilFormat = stencilFormats[props.modes[bestmode].stencil];
    }

    GLFBORenderTexture *GLFBOManager::createRenderTexture(const String &name, const GLSurfaceDesc &target)
    {
        GLFBORenderTexture *retval = new GLFBORenderTexture(this, name, target);
        return retval;
    }
    GLFrameBufferObject *GLFBOManager::createFrameBufferObject()
    {
        return new GLFrameBufferObject(this);
    }

    void GLFBOManager::destroyFrameBufferObject(GLFrameBufferObject * x)
    {
        delete x;
    }
    void GLFBOManager::bind(RenderTarget *target)
    {
        /// Check if the render target is in the rendertarget->FBO map
        GLFrameBufferObject *fbo = 0;
        target->getCustomAttribute("FBO", &fbo);
        if(fbo)
            fbo->bind();
        else
            // Old style context (window/pbuffer) or copying render texture
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }
    
    GLSurfaceDesc GLFBOManager::requestRenderBuffer(GLenum format, size_t width, size_t height)
    {
        GLSurfaceDesc retval;
        retval.buffer = 0; // Return 0 buffer if GL_NONE is requested
        if(format != GL_NONE)
        {
            RBFormat key(format, width, height);
            RenderBufferMap::iterator it = mRenderBufferMap.find(key);
            if(it != mRenderBufferMap.end())
            {
                retval.buffer = it->second.buffer;
                retval.zoffset = 0;
                // Increase refcount
                ++it->second.refcount;
            }
            else
            {
                // New one
                GLRenderBuffer *rb = new GLRenderBuffer(format, width, height);
                mRenderBufferMap[key] = RBRef(rb);
                retval.buffer = rb;
                retval.zoffset = 0;
            }
        }
        //std::cerr << "Requested renderbuffer with format " << std::hex << format << std::dec << " of " << width << "x" << height << " :" << retval.buffer << std::endl;
        return retval;
    }
    void GLFBOManager::releaseRenderBuffer(const GLSurfaceDesc &surface)
    {
        if(surface.buffer == 0)
            return;
        RBFormat key(surface.buffer->getGLFormat(), surface.buffer->getWidth(), surface.buffer->getHeight());
        RenderBufferMap::iterator it = mRenderBufferMap.find(key);
        assert(it != mRenderBufferMap.end());
        // Decrease refcount
        --it->second.refcount;
        if(it->second.refcount==0)
        {
            // If refcount reaches zero, delete buffer and remove from map
            delete it->second.buffer;
            mRenderBufferMap.erase(it);
            //std::cerr << "Destroyed renderbuffer of format " << std::hex << key.format << std::dec
            //        << " of " << key.width << "x" << key.height << std::endl;
        }
    }
}

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2004 The OGRE Team
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

#include "OgreGLXRenderTexture.h"
#include "OgreException.h"

#include <iostream>

namespace Ogre
{
  
    GLXRenderTexture::GLXRenderTexture( const String & name, uint width, uint height, TextureType texType,  PixelFormat format):
        GLRenderTexture(name, width, height, texType, format),
        _hPBuffer(0)
    {
        // make sure it will be a RGBA texture
        // and double buffered PBuffer
        createPBuffer();
    }
//  Needed extensions:
// glXChooseFBConfigSGIX
// glXCreateGLXPbufferSGIX
// glXQueryGLXPbufferSGIX    
    void GLXRenderTexture::createPBuffer() {        
        bool bStencil = true;
           
           
        _pDpy = glXGetCurrentDisplay();
        GLXContext context = glXGetCurrentContext();
        int screen = DefaultScreen(_pDpy);
        XVisualInfo * visInfo = 0;
        int iFormat = 0;
        int iNumFormats;
        int attribs[50];
        int attrib = 0;
        // Attribs for glXChooseFBConfig
        int depth = 1;
        
        attribs[attrib++] = GLX_RENDER_TYPE;
        attribs[attrib++] = GLX_RGBA_BIT;
        
        attribs[attrib++] = GLX_RED_SIZE;
        attribs[attrib++] = depth;
        attribs[attrib++] = GLX_GREEN_SIZE;
        attribs[attrib++] = depth;
        attribs[attrib++] = GLX_BLUE_SIZE;
        attribs[attrib++] = depth;
        attribs[attrib++] = GLX_ALPHA_SIZE;
        if(Image::formatHasAlpha(mInternalFormat))
            attribs[attrib++] = depth;
        else
            attribs[attrib++] = 0; // No alpha
        attribs[attrib++] = GLX_DRAWABLE_TYPE;
        attribs[attrib++] = GLX_PBUFFER_BIT;
        attribs[attrib++] = GLX_STENCIL_SIZE;
        attribs[attrib++] = (bStencil) ? 8 : 0;
        attribs[attrib++] = GLX_DEPTH_SIZE;
        attribs[attrib++] = 16;
        attribs[attrib++] = GLX_DOUBLEBUFFER;
        attribs[attrib++] = 1;
        attribs[attrib++] = None;
        GLXFBConfig * fbConfigs;
        int nConfigs;
        fbConfigs =
            glXChooseFBConfig(_pDpy, screen, attribs, &nConfigs);
        if (nConfigs == 0 || !fbConfigs) {
            // TODO exception
            std::cerr << "RenderTexture::Initialize() creation error: Couldn't find a suitable pixel format" << std::endl;
        }
        // Attribs for CreatePbuffer
        attrib = 0;
        attribs[attrib++] = GLX_PBUFFER_WIDTH;
        attribs[attrib++] = mWidth;
        attribs[attrib++] = GLX_PBUFFER_HEIGHT;
        attribs[attrib++] = mHeight;
        attribs[attrib++] = GLX_PRESERVED_CONTENTS;
        attribs[attrib++] = 1;
        attribs[attrib++] = None;
        // Pick the first returned format that will return a pbuffer
        for (int i = 0; i < nConfigs; i++) {
            _hPBuffer =
                glXCreatePbuffer(_pDpy, fbConfigs[i], attribs);
    
            if (_hPBuffer) {
                visInfo = glXGetVisualFromFBConfig(_pDpy, fbConfigs[0]);
                if (!visInfo) {
                    // TODO exception
                    std::cerr << "Error: couldn't get an RGBA, double-buffered visual" << std::endl;
                }
            
                _hGLContext =
                    glXCreateContext(_pDpy, visInfo, context, True);
                break;
            }
    
        }
        if (!_hPBuffer) {
            // TODO exception
            std::cerr << "RenderTexture::Initialize() pbuffer creation error: glXCreatePbuffer() failed" << std::endl;
        }
        if (!_hGLContext) {
            // TODO exception
            std::cerr << "RenderTexture::Initialize() pbuffer creation error: Create context failed failed" << std::endl;
        }
        if(fbConfigs)
            XFree(fbConfigs);
        if(visInfo)
            XFree(visInfo);
      
        // Query real width and height
        GLuint iWidth, iHeight;
        glXQueryDrawable(_pDpy, _hPBuffer, GLX_WIDTH, &iWidth);
        glXQueryDrawable(_pDpy, _hPBuffer, GLX_HEIGHT, &iHeight);
        std::cerr << "Real dimensions " << iWidth << "x" << iHeight << std::endl;
        mWidth = iWidth;  
        mHeight = iHeight;

    }

    GLXRenderTexture::~GLXRenderTexture()
    {
        glXDestroyContext(_pDpy, _hGLContext);
        _hGLContext = 0;
        glXDestroyPbuffer(_pDpy, _hPBuffer);
        _hPBuffer = 0;
    }

    void GLXRenderTexture::_copyToTexture()
    {
        // Should do nothing
    }
    
    void GLXRenderTexture::firePreUpdate(void)
    {

        //std::cerr << "Pre" << std::endl;

        //GLXDrawable drawable = glXGetCurrentDrawable();
        //GLXContext context = glXGetCurrentContext();
        
        glXMakeCurrent(_pDpy, _hPBuffer, _hGLContext);
        //std::cerr << "Switch" << std::endl;

        //glXMakeCurrent(_pDpy, drawable, context);
        GLint tid = static_cast<GLTexture*>(mTexture)->getGLID();
        
        
        // Set context
        GLRenderTexture::firePreUpdate();
    }
    void GLXRenderTexture::firePostUpdate(void)
    {
        GLRenderTexture::firePostUpdate();
        // Unset, bind texture

        //std::cerr << "Post" << std::endl;
        glXSwapBuffers(_pDpy, _hPBuffer);
        GLRenderTexture::_copyToTexture();

        //glXMakeCurrent(_pDpy, None, 0);

    }
    
  
}

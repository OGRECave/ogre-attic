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

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"
#include "OgreStringConverter.h"

#include "OgreGLRenderSystem.h"

#include "OgreGLXRenderTexture.h"
#include "OgreGLXContext.h"
#include "OgreGLXUtils.h"

#include <iostream>

// Replace by arb ASAP
#ifndef GLX_ATI_pixel_format_float
#define GLX_ATI_pixel_format_float  1
#define GLX_RGBA_FLOAT_ATI_BIT                          0x00000100
#endif


namespace Ogre
{
  
    GLXRenderTexture::GLXRenderTexture( const String & name, unsigned int width, unsigned int height,
			TextureType texType, PixelFormat internalFormat, 
			const NameValuePairList *miscParams):
        GLRenderTexture(name, width, height, texType, internalFormat, miscParams),
        _hPBuffer(0),
        mContext(0)
    {
        createPBuffer();
        // Create context
        mContext = new GLXContext(_pDpy, _hPBuffer, _hGLContext);
        // Register the context with the rendersystem
        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        rs->_registerContext(this, mContext);
    }

    void GLXRenderTexture::createPBuffer() {        
        LogManager::getSingleton().logMessage(
        "GLXRenderTexture::Creating PBuffer"
        );
           
        _pDpy = glXGetCurrentDisplay();
        ::GLXContext context = glXGetCurrentContext();
        int screen = DefaultScreen(_pDpy);
        int iFormat = 0;
        int iNumFormats;
        int attribs[50], ideal[50];
        int attrib;
        // Attribs for glXChooseFBConfig
        // Get R,G,B,A depths
        int depths[4];
        PixelUtil::getBitDepths(mInternalFormat, depths);
        int renderType;
        if(PixelUtil::getFlags(mInternalFormat) & PFF_FLOAT)
            renderType = GLX_RGBA_FLOAT_ATI_BIT; // GLX_RGBA_FLOAT_BIT
        else
            renderType = GLX_RGBA_BIT;

        // Create base required format description
        attrib = 0;
        attribs[attrib++] = GLX_RENDER_TYPE;
        attribs[attrib++] = renderType;        
        attribs[attrib++] = GLX_DRAWABLE_TYPE;
        attribs[attrib++] = GLX_PBUFFER_BIT;
        attribs[attrib++] = GLX_DOUBLEBUFFER;
        attribs[attrib++] = 0;
        attribs[attrib++] = None;
        
        // Create "ideal" format description
        attrib = 0;
        ideal[attrib++] = GLX_RED_SIZE;
        ideal[attrib++] = depths[0];        
        ideal[attrib++] = GLX_GREEN_SIZE;
        ideal[attrib++] = depths[1];
        ideal[attrib++] = GLX_BLUE_SIZE;
        ideal[attrib++] = depths[2];        
        ideal[attrib++] = GLX_ALPHA_SIZE;
        ideal[attrib++] = depths[3];
        ideal[attrib++] = GLX_DEPTH_SIZE;
        ideal[attrib++] = 24;
        ideal[attrib++] = GLX_STENCIL_SIZE;
        ideal[attrib++] = 8;
        ideal[attrib++] = GLX_ACCUM_RED_SIZE;
        ideal[attrib++] = 0;    // Accumulation buffer not used
        ideal[attrib++] = GLX_ACCUM_GREEN_SIZE;
        ideal[attrib++] = 0;    // Accumulation buffer not used
        ideal[attrib++] = GLX_ACCUM_BLUE_SIZE;
        ideal[attrib++] = 0;    // Accumulation buffer not used
        ideal[attrib++] = GLX_ACCUM_ALPHA_SIZE;
        ideal[attrib++] = 0;    // Accumulation buffer not used
        ideal[attrib++] = None;

        // Create vector of existing config data formats        
        GLXFBConfig config = GLXUtils::findBestMatch(_pDpy, screen, attribs, ideal);

        // Create the pbuffer in the best matching format
        attrib = 0;
        attribs[attrib++] = GLX_PBUFFER_WIDTH;
        attribs[attrib++] = mWidth; // Get from texture?
        attribs[attrib++] = GLX_PBUFFER_HEIGHT;
        attribs[attrib++] = mHeight; // Get from texture?
        attribs[attrib++] = GLX_PRESERVED_CONTENTS;
        attribs[attrib++] = 1;
        attribs[attrib++] = None;

        FBConfigData configData(_pDpy, config);
        LogManager::getSingleton().logMessage(
                LML_NORMAL,
                "GLXRenderTexture::PBuffer chose format "+configData.toString());                   

        _hPBuffer = glXCreatePbuffer(_pDpy, config, attribs);
        if (!_hPBuffer) 
            Except(Exception::UNIMPLEMENTED_FEATURE, "glXCreatePbuffer() failed", "GLRenderTexture::createPBuffer");

        _hGLContext = glXCreateNewContext(_pDpy, config, GLX_RGBA_TYPE, context, True);
        if (!_hGLContext) 
            Except(Exception::UNIMPLEMENTED_FEATURE, "glXCreateContext() failed", "GLRenderTexture::createPBuffer");        

        // Query real width and height
        GLuint iWidth, iHeight;
        glXQueryDrawable(_pDpy, _hPBuffer, GLX_WIDTH, &iWidth);
        glXQueryDrawable(_pDpy, _hPBuffer, GLX_HEIGHT, &iHeight);

        LogManager::getSingleton().logMessage(
             LML_NORMAL,
                "GLXRenderTexture::PBuffer created -- Real dimensions "+
                StringConverter::toString(iWidth)+"x"+StringConverter::toString(iHeight)
        );
        mWidth = iWidth;  
        mHeight = iHeight;
    }

    GLXRenderTexture::~GLXRenderTexture()
    {
        // Unregister and destroy mContext
        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        rs->_unregisterContext(this);
        delete mContext;
        // Destroy GL context
        glXDestroyContext(_pDpy, _hGLContext);
        _hGLContext = 0;
        glXDestroyPbuffer(_pDpy, _hPBuffer);
        _hPBuffer = 0;
    }

/*
    void GLXRenderTexture::_copyToTexture()
    {
        // Should do nothing
    }
 */  
    
  
}

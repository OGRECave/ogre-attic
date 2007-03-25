/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
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

/// ATI: GLX_ATI_pixel_format_float
#ifndef GLX_RGBA_FLOAT_ATI_BIT
#define GLX_RGBA_FLOAT_ATI_BIT 0x00000100
#endif

/// ARB: GLX_ARB_fbconfig_float
#ifndef GLX_RGBA_FLOAT_BIT
#define GLX_RGBA_FLOAT_BIT 0x00000004
#endif

#ifndef GLX_RGBA_FLOAT_TYPE
#define GLX_RGBA_FLOAT_TYPE 0x20B9
#endif


namespace Ogre
{
    GLXPBuffer::GLXPBuffer (PixelComponentType format, size_t width, size_t height) :
        GLPBuffer (format, width, height), mContext (0)
    {
        createPBuffer ();
    }

    GLXPBuffer::~GLXPBuffer()
    {
        // Destroy and unregister context & PBuffer
        delete mContext;
    }

    GLContext *GLXPBuffer::getContext ()
    {
        return mContext;
    }

    void GLXPBuffer::createPBuffer ()
    {
	GLRenderSystem *rs = static_cast<GLRenderSystem *>(Root::getSingleton ().getRenderSystem ());
        GLXContext *ctx = static_cast<GLXContext *>(rs->_getMainContext ());

        int attribs [50], ideal [50];
        int attrib;

        // Process format
        int bits = 0;
        bool isFloat = false;
        switch (mFormat)
        {
            case PCT_BYTE:
                bits = 8; isFloat = false;
                break;
            case PCT_SHORT:
                bits = 16; isFloat = false;
                break;
            case PCT_FLOAT16:
                bits = 16; isFloat = true;
                break;
            case PCT_FLOAT32:
                bits = 32; isFloat = true;
                break;
            default:
                break;
        };

        RTFType floatBuffer = RTF_NONE;
        if (isFloat)
        {
            floatBuffer = detectRTFType (ctx->mDisplay);
            if (floatBuffer == RTF_NONE || floatBuffer == RTF_NV)
                OGRE_EXCEPT (Exception::ERR_NOT_IMPLEMENTED,
                             "Floating point PBuffers not supported on this hardware",
                             "GLRenderTexture::createPBuffer");
        }

        // Create base required format description
        attrib = 0;
        if (floatBuffer == RTF_ATI)
        {
            attribs[attrib++] = GLX_RENDER_TYPE;
            attribs[attrib++] = GLX_RGBA_FLOAT_ATI_BIT;
        }
        else if (floatBuffer == RTF_ARB)
        {
            attribs[attrib++] = GLX_RENDER_TYPE;
            attribs[attrib++] = GLX_RGBA_FLOAT_BIT;
        }
        else
        {
            attribs[attrib++] = GLX_RENDER_TYPE;
            attribs[attrib++] = GLX_RGBA_BIT;
        }
        attribs[attrib++] = GLX_DRAWABLE_TYPE;
        attribs[attrib++] = GLX_PBUFFER_BIT;
        attribs[attrib++] = GLX_DOUBLEBUFFER;
        attribs[attrib++] = 0;
#if 0
        if (floatBuffer == RTF_NV)
        {
            attribs[attrib++] = GLX_FLOAT_COMPONENTS_NV;
            attribs[attrib++] = 1;
        }
#endif
        attribs[attrib++] = None;

        // Create "ideal" format description
        attrib = 0;
        ideal[attrib++] = GLX_RED_SIZE;
        ideal[attrib++] = bits;
        ideal[attrib++] = GLX_GREEN_SIZE;
        ideal[attrib++] = bits;
        ideal[attrib++] = GLX_BLUE_SIZE;
        ideal[attrib++] = bits;
        ideal[attrib++] = GLX_ALPHA_SIZE;
        ideal[attrib++] = bits;
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
        GLXFBConfig fbc = GLXUtils::findBestMatch (
            ctx->mDisplay, DefaultScreen (ctx->mDisplay), attribs, ideal);

        // Create the pbuffer in the best matching format
        attrib = 0;
        attribs[attrib++] = GLX_PBUFFER_WIDTH;
        attribs[attrib++] = mWidth; // Get from texture?
        attribs[attrib++] = GLX_PBUFFER_HEIGHT;
        attribs[attrib++] = mHeight; // Get from texture?
        attribs[attrib++] = GLX_PRESERVED_CONTENTS;
        attribs[attrib++] = 1;
        attribs[attrib++] = GLX_CONFIG_CAVEAT;
        attribs[attrib++] = GLX_NONE;   // No caveats or slow configs
        attribs[attrib++] = GLX_LEVEL;
        attribs[attrib++] = 0;          // No overlays
        attribs[attrib++] = GLX_STEREO;
        attribs[attrib++] = GL_FALSE;   // No stereo contexts
        attribs[attrib++] = GLX_DOUBLEBUFFER;
        attribs[attrib++] = GL_FALSE;   // No double buffer
        attribs[attrib++] = GLX_DEPTH_SIZE;
        attribs[attrib++] = 16;         // We're going to render into it
        attribs[attrib++] = GLX_STENCIL_SIZE;
        attribs[attrib++] = 1;          // And even perhaps with shadows
        attribs[attrib++] = GLX_RENDER_TYPE;
        attribs[attrib++] = GLX_RGBA_BIT;      // No indexed contexts
        attribs[attrib++] = GLX_DRAWABLE_TYPE;
        attribs[attrib++] = GLX_PBUFFER_BIT;   // Compatible with PBuffer's
        attribs[attrib++] = None;

        ::GLXPbuffer pbuf = glXCreatePbuffer (ctx->mDisplay, fbc, attribs);
        if (!pbuf)
            OGRE_EXCEPT (Exception::ERR_NOT_IMPLEMENTED,
                         "glXCreatePbuffer() failed",
                         "GLRenderTexture::createPBuffer");

        ::GLXContext newctx = glXCreateNewContext (ctx->mDisplay, fbc, GLX_RGBA_TYPE, ctx->mContext, True);
        if (!newctx)
        {
            glXDestroyPbuffer (ctx->mDisplay, pbuf);
            OGRE_EXCEPT (Exception::ERR_NOT_IMPLEMENTED,
                         "glXCreateContext() failed",
                         "GLRenderTexture::createPBuffer");
        }

        // Query real width and height
        GLuint iWidth, iHeight;
        glXQueryDrawable (ctx->mDisplay, pbuf, GLX_WIDTH, &iWidth);
        glXQueryDrawable (ctx->mDisplay, pbuf, GLX_HEIGHT, &iHeight);

        LogManager::getSingleton ().logMessage (
            LML_TRIVIAL,
            "Created PBuffer: " + StringConverter::toString (iWidth) + "x" + StringConverter::toString (iHeight) +
            "x" + StringConverter::toString (bits) + " float: " + StringConverter::toString (isFloat) +
            " pixel format: " + GLXUtils::FBConfigToString (ctx->mDisplay, fbc));

        mWidth = iWidth;
        mHeight = iHeight;

        // Create context
        mContext = new GLXContext (ctx->mDisplay, pbuf, newctx, fbc, GLX_PBUFFER);
    }

    GLXPBuffer::RTFType GLXPBuffer::detectRTFType (::Display *dpy)
    {
        static struct { char *s; RTFType t; } rtf_types [] =
        {
            { "GLX_NV_float_buffer", RTF_NV },
            { "GLX_ATI_pixel_format_float", RTF_ATI },
            { "GLX_ARB_fbconfig_float", RTF_ARB }
        };

        /// Query supported float buffer extensions
        /// Choose the best one
        const char *s_ext = glXQueryExtensionsString (dpy, DefaultScreen (dpy));
        const char *c_ext = glXGetClientString (dpy, GLX_EXTENSIONS);

        RTFType floatBuffer = RTF_NONE;
        for (size_t i = 0; i < sizeof (rtf_types) / sizeof (rtf_types [0]); i++)
        {
            if (strstr (s_ext, rtf_types [i].s) ||
                strstr (c_ext, rtf_types [i].s))
                if (floatBuffer < rtf_types [i].t)
                    floatBuffer = rtf_types [i].t;
        }

        return floatBuffer;
    }
}

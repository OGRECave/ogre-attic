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
#include "OgreGLXUtils.h"

#include "OgreNoMemoryMacros.h"
#include <GL/glxext.h>

namespace Ogre {
    String GLXUtils::FBConfigToString (Display *dpy, GLXFBConfig config)
    {
        int configID, visualID, bufferSize, level, drawableType, caveat;
        int doubleBuffer, stereo, auxBuffers, renderType, depthSize, stencilSize;
        int redSize, greenSize, blueSize, alphaSize;
        int accumRedSize, accumGreenSize, accumBlueSize, accumAlphaSize;
        int maxPBufferWidth, maxPBufferHeight, maxPBufferPixels;
        int sampleBuffers, samples;

        glXGetFBConfigAttrib (dpy, config, GLX_FBCONFIG_ID, &configID);
        glXGetFBConfigAttrib (dpy, config, GLX_VISUAL_ID, &visualID);
        glXGetFBConfigAttrib (dpy, config, GLX_BUFFER_SIZE, &bufferSize);
        glXGetFBConfigAttrib (dpy, config, GLX_LEVEL, &level);
        glXGetFBConfigAttrib (dpy, config, GLX_DOUBLEBUFFER, &doubleBuffer);
        glXGetFBConfigAttrib (dpy, config, GLX_STEREO, &stereo);
        glXGetFBConfigAttrib (dpy, config, GLX_AUX_BUFFERS, &auxBuffers);
        glXGetFBConfigAttrib (dpy, config, GLX_RENDER_TYPE, &renderType);
        glXGetFBConfigAttrib (dpy, config, GLX_RED_SIZE, &redSize);
        glXGetFBConfigAttrib (dpy, config, GLX_GREEN_SIZE, &greenSize);
        glXGetFBConfigAttrib (dpy, config, GLX_BLUE_SIZE, &blueSize);
        glXGetFBConfigAttrib (dpy, config, GLX_ALPHA_SIZE, &alphaSize);
        glXGetFBConfigAttrib (dpy, config, GLX_DEPTH_SIZE, &depthSize);
        glXGetFBConfigAttrib (dpy, config, GLX_STENCIL_SIZE, &stencilSize);
        glXGetFBConfigAttrib (dpy, config, GLX_ACCUM_RED_SIZE, &accumRedSize);
        glXGetFBConfigAttrib (dpy, config, GLX_ACCUM_GREEN_SIZE, &accumGreenSize);
        glXGetFBConfigAttrib (dpy, config, GLX_ACCUM_BLUE_SIZE, &accumBlueSize);
        glXGetFBConfigAttrib (dpy, config, GLX_ACCUM_ALPHA_SIZE, &accumAlphaSize);
        glXGetFBConfigAttrib (dpy, config, GLX_DRAWABLE_TYPE, &drawableType);
        glXGetFBConfigAttrib (dpy, config, GLX_CONFIG_CAVEAT, &caveat);
        glXGetFBConfigAttrib (dpy, config, GLX_MAX_PBUFFER_WIDTH, &maxPBufferWidth);
        glXGetFBConfigAttrib (dpy, config, GLX_MAX_PBUFFER_HEIGHT, &maxPBufferHeight);
        glXGetFBConfigAttrib (dpy, config, GLX_MAX_PBUFFER_PIXELS, &maxPBufferPixels);
        glXGetFBConfigAttrib (dpy, config, GLX_SAMPLE_BUFFERS_ARB, &sampleBuffers);
        glXGetFBConfigAttrib (dpy, config, GLX_SAMPLES_ARB, &samples);

        char buff [400];

        snprintf (buff, sizeof (buff),
                  "ConfigID:0x%02x VisualID:0x%02x bufferSize:%d level:%d doubleBuffer:%d "
                  "stereo:%d caveat:%d renderType:%d drawableType:%d auxBuffers:%d "
                  "depth bits:%d stencil bits:%d R/G/B/A bits:%d/%d/%d/%d "
                  "Accum R/G/B/A bits:%d/%d/%d/%d Max PBuffer W/H/Size:%d/%d/%d "
                  "Sample buffers/samples:%d/%d",
                  configID, visualID, bufferSize, level, doubleBuffer, stereo,
                  caveat & ~GLX_NONE, renderType, drawableType, auxBuffers,
                  depthSize, stencilSize,
                  redSize, greenSize, blueSize, alphaSize,
                  accumRedSize, accumGreenSize, accumBlueSize, accumAlphaSize,
                  maxPBufferWidth, maxPBufferHeight, maxPBufferPixels,
                  sampleBuffers, samples);
        return String (buff);
    }

    GLXFBConfig GLXUtils::findBestMatch (Display *dpy, int scrnum,
                                         const int *attribs, const int *idealattribs)
    {
        // Create vector of existing config data formats
        int nConfigs;
        GLXFBConfig *fbConfigs = glXChooseFBConfig (dpy, scrnum, attribs, &nConfigs);
        if (nConfigs == 0 || !fbConfigs)
            OGRE_EXCEPT (Exception::ERR_NOT_IMPLEMENTED,
                         "glXChooseFBConfig() failed: Couldn't find a suitable pixel format",
                         "GLRenderTexture::createPBuffer");

        // Find the best fbconfig. GLX already sorted this for us according
        // to its own criteria, so just pick the first most suitable one.
        GLXFBConfig best = 0;
        uint best_score = 0xffffffff;
        for (int i = 0; i < nConfigs; i++)
        {
            // Compute the "matching score" for current visual.
            // The earliest attributes have most weight, since
            // we suppose the caller have specified them in
            // most-important to less-important order.
            uint score = 0;

            for (int j = 0; idealattribs [j] != None; j += 2)
            {
                int value;
                glXGetFBConfigAttrib (dpy, fbConfigs [i], idealattribs [j], &value);
                int delta = value - idealattribs [j + 1];
                score += delta * delta * (j / 2 + 1);
            }

            if (score < best_score)
            {
                best_score = score;
                best = fbConfigs [i];
            }
        }

        XFree (fbConfigs);

        return best;
    }

    bool GLXUtils::LoadIcon (Display *mDisplay, Window rootWindow,
                             const std::string &name, Pixmap *pix, Pixmap *mask)
    {
        Image img;
        int mWidth, mHeight;
        char *data, *bitmap;
        try
        {
            // Try to load image
            img.load (name, ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            mWidth = img.getWidth ();
            mHeight = img.getHeight ();
            if (img.getFormat () != PF_A8R8G8B8)
                // Image format must be RGBA
                return 0;
        }
        catch (Exception &e)
        {
            // Could not find image; never mind
            return false;
        }

        // Allocate space for image data
        data = (char*)malloc(mWidth * mHeight * 4); // Must be allocated with malloc
        // Allocate space for transparency bitmap
        int wbits = (mWidth+7)/8;
        bitmap = (char*)malloc(wbits * mHeight);

        // Convert and copy image
        const char *imgdata = (const char*)img.getData();
        int sptr = 0, dptr = 0;
        for(int y=0; y<mHeight; y++) {
            for(int x=0; x<mWidth; x++) {
                data[dptr + 0] = 0;
                data[dptr + 1] = imgdata[sptr + 0];
                data[dptr + 2] = imgdata[sptr + 1];
                data[dptr + 3] = imgdata[sptr + 2];
                // Alpha threshold
                if(((unsigned char)imgdata[sptr + 3])<128) {
                    bitmap[y*wbits+(x>>3)] &= ~(1<<(x&7));
                } else {
                    bitmap[y*wbits+(x>>3)] |= 1<<(x&7);
                }
                sptr += 4;
                dptr += 4;
            }
        }

        /* put my pixmap data into the client side X image data structure */
        XImage *image = XCreateImage (mDisplay, NULL, 24, ZPixmap, 0,
                                      data,
                                      mWidth, mHeight, 8,
                                      mWidth*4);
        image->byte_order = MSBFirst; // 0RGB format

        /* tell server to start managing my pixmap */
        Pixmap retval = XCreatePixmap(mDisplay, rootWindow, mWidth,
                                      mHeight, 24);

        /* copy from client to server */
        GC context = XCreateGC (mDisplay, rootWindow, 0, NULL);
        XPutImage(mDisplay, retval, context, image, 0, 0, 0, 0,
                  mWidth, mHeight);

        /* free up the client side pixmap data area */
        XDestroyImage(image); // also cleans data
        XFreeGC(mDisplay, context);

        *pix = retval;
        *mask = XCreateBitmapFromData(mDisplay, rootWindow, bitmap, mWidth, mHeight);
        free(bitmap);
        return true;
    }
};

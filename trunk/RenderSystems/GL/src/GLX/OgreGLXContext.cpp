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

#include "OgreGLRenderSystem.h"
#include "OgreRoot.h"
#include "OgreGLXContext.h"

namespace Ogre {

    GLXContext::GLXContext (::Display *dpy, ::GLXDrawable drawable,
        ::GLXContext ctx, ::GLXFBConfig fbconfig, uint32 drawableType) :
        mDisplay (dpy), mDrawable (drawable), mContext (ctx),
        mFBConfig (fbconfig), mDrawableType (drawableType)
    {
    }

    GLXContext::~GLXContext ()
    {
        // Unregister and destroy this context
        // This will disable it if it was still active
        // NB have to do this is subclass to ensure any methods called back
        // are on this subclass and not half-destructed superclass
        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        rs->_unregisterContext (this);

        if (mDrawableType != 0)
        {
            // Destroy GL context
            glXDestroyContext (mDisplay, mContext);

            if (mDrawableType == GLX_WINDOW)
                glXDestroyWindow (mDisplay, mDrawable);
            else if (mDrawableType == GLX_PBUFFER)
                glXDestroyPbuffer (mDisplay, mDrawable);
        }
    }

    void GLXContext::setCurrent ()
    {
        glXMakeCurrent (mDisplay, mDrawable, mContext);
    }

    void GLXContext::endCurrent ()
    {
        glXMakeCurrent (mDisplay, None, NULL);
    }

    GLContext *GLXContext::clone () const
    {
        // Create a new context, share lists with existing
        ::GLXContext newctx = glXCreateNewContext (mDisplay, mFBConfig, GLX_RGBA_TYPE, mContext, True);

        // Create an unused PBuffer, other contexts can't render to same window
        static const int pbuf_attr [] =
        {
            GLX_PBUFFER_WIDTH, 1,
            GLX_PBUFFER_HEIGHT, 1,
            GLX_PRESERVED_CONTENTS, False,
            GLX_DOUBLEBUFFER, False,
            None
        };
        GLXPbuffer pbuf = glXCreatePbuffer (mDisplay, mFBConfig, pbuf_attr);
        if (!pbuf)
            OGRE_EXCEPT (Exception::ERR_NOT_IMPLEMENTED,
                         "Failed to create a spare PBuffer",
                         "GLXContext::clone");

        return new GLXContext (mDisplay, pbuf, newctx, mFBConfig, GLX_PBUFFER);
    }
}

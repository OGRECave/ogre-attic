/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2004 The OGRE Team
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
#ifndef __GLXRenderTexture_H__
#define __GLXRenderTexture_H__

#include "OgrePrerequisites.h"

#include "OgreGLTexture.h"
#include "OgreGLXContext.h"

#include <X11/Xlib.h>
#include <GL/glx.h>

namespace Ogre
{
    class _OgreExport GLXRenderTexture : public GLRenderTexture
    {
    public:
        GLXRenderTexture( const String & name, uint width, uint height, TextureType texType,  PixelFormat format );
        ~GLXRenderTexture();
    protected:
        //virtual void _copyToTexture();

        void createPBuffer();

        ::Display      *_pDpy;
        ::GLXContext   _hGLContext;
        ::GLXPbuffer   _hPBuffer;
        GLXContext   *mContext;
    };
}

#endif

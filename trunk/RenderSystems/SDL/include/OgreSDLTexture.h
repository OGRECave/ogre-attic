/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/

#ifndef __SDLTexture_H__
#define __SDLTexture_H__

#include "OgrePlatform.h"
#include "OgreTexture.h"

#if OGRE_PLATFORM == PLATFORM_WIN32
#   include "gl.h"
#   define GL_GLEXT_PROTOTYPES
#   include "glprocs.h"
#else
#   include <GL/gl.h>
#endif

#include <GL/glu.h>

namespace Ogre {

    class SDLTexture : public Texture
    {
    public:
        // Constructor, called from SDLTextureManager
        SDLTexture(String name);
        virtual ~SDLTexture();
        /** Overridden - see Texture. */
        void loadRawRGB(void* buffer, int width, int height);
        void loadRawRGBA( void *buffer, int width, int height );
        void load(void);
        void unload(void);

        GLuint getGLID() const
        {
            return mTextureID;
        }
    protected:
        void generateMipMaps(void* data);
    private:
        GLuint mTextureID;
    };
}

#endif

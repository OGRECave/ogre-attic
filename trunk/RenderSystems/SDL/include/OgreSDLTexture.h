/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
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

#ifndef __SDLTexture_H__
#define __SDLTexture_H__

#include "OgrePlatform.h"
#include "OgreTexture.h"
#include "OgreRenderTexture.h"

#if OGRE_PLATFORM == PLATFORM_WIN32
#   include <windows.h>
#   include <wingdi.h>
#   include "gl.h"
#   define GL_GLEXT_PROTOTYPES
#   include "glprocs.h"
#   include <GL/glu.h>
#elif OGRE_PLATFORM == PLATFORM_LINUX
#   include <GL/gl.h>
#   include <GL/glu.h>
#elif OGRE_PLATFORM == PLATFORM_APPLE
#   include <OpenGL/gl.h>
#   define GL_EXT_texture_env_combine 1
#   include <OpenGL/glext.h>
#   include <OpenGL/glu.h>
#endif



namespace Ogre {

    class SDLTexture : public Texture
    {
    public:
        // Constructor, called from SDLTextureManager
        SDLTexture( String name );
        SDLTexture( String name, uint width, uint height, uint num_mips, 
            PixelFormat format, TextureUsage usage );

        virtual ~SDLTexture();        
        
        void load();
        void loadImage( const Image &img );

        void unload();

        void createRenderTexture();

        void blitToTexture( const Image& src, 
            unsigned uStartX, unsigned uStartY );

        GLuint getGLID() const
        {
            return mTextureID;
        }

    protected:
        void generateMipMaps( uchar *data );

    private:
        GLuint mTextureID;
    };

    class SDLRenderTexture : public RenderTexture
    {
    public:
        SDLRenderTexture(const String& name, uint width, uint height) 
            : RenderTexture(name, width, height) 
        {
        }

        void _copyToTexture(void);

        bool requiresTextureFlipping() const { return false; }
        virtual void writeContentsToFile( const String & filename ) {}
        virtual void outputText(int x, int y, const String& text) {}
    };
}

#endif

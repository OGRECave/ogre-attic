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

#include "OgreSDLTexture.h"
#include "OgreTextureManager.h"
#include "OgreDataChunk.h"
#include "OgreImage.h"
#include "OgreLogManager.h"

#if OGRE_PLATFORM == PLATFORM_WIN32
#   include <windows.h>
#   include <wingdi.h>
#endif

namespace Ogre {

    SDLTexture::SDLTexture(String name)
    {
        mName = name;
        enable32Bit(false);
    }

    SDLTexture::~SDLTexture()
    {
        unload();
    }

    void SDLTexture::blitToTexture( 
        const Image& src, 
        unsigned uStartX, unsigned uStartY )
    {
        Image img = src;
        img.flipAroundX();

        glBindTexture( GL_TEXTURE_2D, mTextureID );
        Image::applyGamma( img.getData(), mGamma, img.getSize(), img.getBPP() );
        glTexSubImage2D( 
            GL_TEXTURE_2D, 0, 
            uStartX, uStartY,
            img.getWidth(), img.getHeight(),
            img.getHasAlpha() ? GL_RGBA : GL_RGB, 
            GL_UNSIGNED_BYTE, img.getData() );
    }

    void SDLTexture::loadImage( const Image &src )
    {
        Image img = src;
        img.flipAroundX();

        if( mIsLoaded )
        {
            std::cout << "Unloading image" << std::endl;
            unload();
        }

        LogManager::getSingleton().logMessage( 
            LML_NORMAL,
            "SDLTexture: Loading %s with %d mipmaps from Image.", 
            mName.c_str(), mNumMipMaps );        

        PixelFormat pf = img.getFormat();

        mSrcBpp = Image::PF2BPP(pf);
        mHasAlpha = img.getHasAlpha();

        mSrcWidth = img.getWidth();
        mSrcHeight = img.getHeight();
        // Same dest dimensions for GL
        mWidth = mSrcWidth;
        mHeight = mSrcHeight;

        uchar *pTempData = new uchar[ img.getSize() ];
        memcpy( pTempData, img.getData(), img.getSize() );

        // Create the GL texture
        glGenTextures( 1, &mTextureID );
        glBindTexture( GL_TEXTURE_2D, mTextureID );

        Image::applyGamma( pTempData, mGamma, img.getSize(), mSrcBpp );

        glTexImage2D(
            GL_TEXTURE_2D, 0, mHasAlpha ? GL_RGBA : GL_RGB, 
            img.getWidth(), img.getHeight(), 0, 
            mHasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pTempData );

        generateMipMaps( pTempData );

        delete [] pTempData;

        // Update size (the final size, not including temp space)
        short bytesPerPixel = mFinalBpp >> 3;
        if( !mHasAlpha && mFinalBpp == 32 )
        {
            bytesPerPixel--;
        }
        mSize = mWidth * mHeight * bytesPerPixel;

        mIsLoaded = true;     
    }
    
    void SDLTexture::load()
    {        
        Image img;
        img.load( mName );

        loadImage( img );
    }
    
    void SDLTexture::unload()
    {
        if( mIsLoaded )
        {
            glDeleteTextures( 1, &mTextureID );
            mIsLoaded = false;
        }
    }
    
    void SDLTexture::generateMipMaps( uchar *data )
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mNumMipMaps );
        gluBuild2DMipmaps(
            GL_TEXTURE_2D, mHasAlpha ? GL_RGBA : GL_RGB, mSrcWidth, mSrcHeight, 
            mHasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data );
    }
}


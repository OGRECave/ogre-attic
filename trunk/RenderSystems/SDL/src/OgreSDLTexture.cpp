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

    void SDLTexture::loadRawRGB(void* buffer, int width, int height)
    {
        if (mIsLoaded)
        {
            unload();
            mIsLoaded = false;
        }

        char msg[100];
        sprintf(msg, "SDLTexture: Loading %s with %d mipmaps from raw RGB stream.", mName.c_str(), mNumMipMaps);
        LogManager::getSingleton().logMessage(msg);

        mSrcBpp = 24;
        mHasAlpha = false;

        mSrcWidth = width;
        mSrcHeight = height;

        // Create the GL texture
        glGenTextures(1, &mTextureID);

        glBindTexture(GL_TEXTURE_2D, mTextureID);

        applyGamma((unsigned char*)buffer,width*height*(mSrcBpp>>3), mSrcBpp);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);

        // Generate mipmaps
        generateMipMaps(buffer);

        // Update size (NB final size, not including temp space)
        short bytesPerPixel = mFinalBpp >> 3;
        if ((!mHasAlpha) && (mFinalBpp == 32))
        {
            bytesPerPixel--;
        }
        mSize = mWidth * mHeight * bytesPerPixel;

        mIsLoaded = true;

    }

    void SDLTexture::loadRawRGBA(void* buffer, int width, int height)
    {
        if (mIsLoaded)
        {
            unload();
            mIsLoaded = false;
        }

        char msg[100];
        sprintf(msg, "SDLTexture: Loading %s with %d mipmaps from raw RGB stream.", mName.c_str(), mNumMipMaps);
        LogManager::getSingleton().logMessage(msg);

        mSrcBpp = 32;
        mHasAlpha = true;

        mSrcWidth = width;
        mSrcHeight = height;

        // Create the GL texture
        glGenTextures(1, &mTextureID);

        glBindTexture(GL_TEXTURE_2D, mTextureID);

        applyGamma((unsigned char*)buffer,width*height*(mSrcBpp>>3), mSrcBpp);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

        // Generate mipmaps
        generateMipMaps(buffer);

        // Update size (NB final size, not including temp space)
        short bytesPerPixel = mFinalBpp >> 3;
        if ((!mHasAlpha) && (mFinalBpp == 32))
        {
            bytesPerPixel--;
        }
        mSize = mWidth * mHeight * bytesPerPixel;

        mIsLoaded = true;

    }

    void SDLTexture::load()
    {
        if (mIsLoaded)
        {
            unload();
            mIsLoaded = false;
        }

        char msg[100];
        sprintf(msg, "SDLTexture: Loading %s with %d mipmaps.", mName.c_str(), mNumMipMaps);
        LogManager::getSingleton().logMessage(msg);

        // Get extension
        // Manipulate using C-strings as case-insensitive compare is hard in STL?
        char extension[4];


        // Read data from whatever source
        DataChunk chunk;
        TextureManager::getSingleton()._findResourceData(mName, chunk);

        size_t pos = mName.find_last_of(".");
		if( pos == String::npos )
            Except( 
			Exception::ERR_INVALIDPARAMS, 
			"Unable to load texture - invalid extension.",
            "SDLTexture::load" );

        strcpy(extension, mName.substr(pos + 1, 3).c_str());

        // NB - BMP support removed
        // Load using Image object
        Image img;
        // Load from memory chunk
        img.load(chunk,extension);
        chunk.clear();        
        if (img.isGreyscale())
        {
            mSrcBpp = 8;
        }
        else
        {
            mSrcBpp = 24;
        }

        if (mHasAlpha = img.hasAlphaChannel())
        {
            mSrcBpp += 8;
        }
        mSrcWidth = img.getWidth();
        mSrcHeight = img.getHeight();

        // Set er up
        glGenTextures(1, &mTextureID);

        glBindTexture(GL_TEXTURE_2D, mTextureID);

        applyGamma((unsigned char*)img.getData(),mSrcWidth*mSrcHeight*(mSrcBpp>>3), mSrcBpp);

        if (mHasAlpha)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mSrcWidth, mSrcHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getData());
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mSrcWidth, mSrcHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img.getData());
        
        // Generate mipmaps
        generateMipMaps(img.getData());

        // Update size (NB final size, not including temp space)
        short bytesPerPixel = mFinalBpp >> 3;
        if ((!mHasAlpha) && (mFinalBpp == 32))
        {
            bytesPerPixel--;
        }
        mSize = mWidth * mHeight * bytesPerPixel;

        mIsLoaded = true;
    }
    
    void SDLTexture::unload()
    {
        if (mIsLoaded)
            glDeleteTextures(1, &mTextureID);
    }
    
    void SDLTexture::generateMipMaps(void* data)
    {
        GLint format = mHasAlpha ? GL_RGBA : GL_RGB;        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mNumMipMaps);
        gluBuild2DMipmaps(
            GL_TEXTURE_2D, format, mSrcWidth, mSrcHeight, format,
            GL_UNSIGNED_BYTE, data);
    }
}


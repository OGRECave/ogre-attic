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

#include "OgreGLTexture.h"
#include "OgreGLSupport.h"
#include "OgreTextureManager.h"
#include "OgreDataChunk.h"
#include "OgreImage.h"
#include "OgreLogManager.h"
#include "OgreCamera.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreCodec.h"
#include "OgreSDDataChunk.h"

#if OGRE_PLATFORM == PLATFORM_WIN32
#   include <windows.h>
#   include <wingdi.h>
#endif

namespace Ogre {

    unsigned int mostSignificantBitSet(unsigned int value)
	{
		unsigned int result = 0;
		while (value != 0) {
			++result;
			value >>= 1;
		}
		return result-1;
	}

    GLTexture::GLTexture(const String& name, GLSupport& support, TextureType texType) :
        mGLSupport(support)
    {
        mName = name;
        mTextureType = texType;

        mUsage = TU_DEFAULT;
        enable32Bit(false);
    }

    // XXX init rather than assign
    GLTexture::GLTexture(const String& name, GLSupport& support, TextureType texType, 
        uint width, uint height, uint num_mips, PixelFormat format, 
        TextureUsage usage) : mGLSupport(support)
    {
        mName = name;
        mTextureType = texType;

        mSrcWidth = width;
        mSrcHeight = height;
        // Same dest dimensions for GL
        mWidth = mSrcWidth;
        mHeight = mSrcHeight;
        mDepth = 1;

        mNumMipMaps = num_mips;

        mUsage = usage;
        mFormat = format;

        mSrcBpp = Image::PF2BPP(mFormat);

        enable32Bit(false);
    }

    GLTexture::~GLTexture()
    {
        unload();
    }

    GLenum GLTexture::getGLTextureType(void) const
    {
        switch(mTextureType)
        {
            case TEX_TYPE_1D:
                return GL_TEXTURE_1D;
            case TEX_TYPE_2D:
                return GL_TEXTURE_2D;
            case TEX_TYPE_3D:
                return GL_TEXTURE_3D;
            case TEX_TYPE_CUBE_MAP:
                return GL_TEXTURE_CUBE_MAP;
            default:
                return 0;
        };
    }

    GLenum GLTexture::getGLTextureFormat(void) const
    {
        switch(mFormat)
        {
            case PF_L8:
                return GL_LUMINANCE;
            case PF_R8G8B8:
                return GL_RGB;
            case PF_B8G8R8:
                return GL_BGR;
            case PF_B8G8R8A8:
                return GL_BGRA;
            case PF_A8R8G8B8:
                return GL_RGBA;
            case PF_DXT1:
                return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            case PF_DXT3:
                 return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            case PF_DXT5:
                 return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            default:
                return 0;
        }
    }

    void GLTexture::blitToTexture( 
        const Image& src, 
        unsigned uStartX, unsigned uStartY )
    {
		if (this->getTextureType() != TEX_TYPE_2D)
          Except( Exception::UNIMPLEMENTED_FEATURE, "**** Blit to texture implemented only for 2D textures!!! ****", "GLTexture::blitToTexture" );

        mGLSupport.begin_context();
        glBindTexture( GL_TEXTURE_2D, mTextureID );
        glTexSubImage2D( 
            GL_TEXTURE_2D, 0, 
            uStartX, uStartY,
            src.getWidth(), src.getHeight(),
            getGLTextureFormat(),
            GL_UNSIGNED_BYTE, src.getData() );
        mGLSupport.end_context();
    }

    uchar* GLTexture::rescaleNPower2( const Image& src ) 
    {
        // Scale image to n^2 dimensions
		unsigned int newWidth = (1 << mostSignificantBitSet(mSrcWidth));
		if (newWidth != mSrcWidth)
			newWidth <<= 1;

		unsigned int newHeight = (1 << mostSignificantBitSet(mSrcHeight));
		if (newHeight != mSrcHeight)
			newHeight <<= 1;

        uchar *pTempData;
        if(newWidth != mSrcWidth || newHeight != mSrcHeight)
        {
          unsigned int newImageSize = newWidth * newHeight * 
            (mHasAlpha ? 4 : 3);

          pTempData = new uchar[ newImageSize ];
          mGLSupport.begin_context();
          if(gluScaleImage(getGLTextureFormat(), mSrcWidth, mSrcHeight,
                GL_UNSIGNED_BYTE, src.getData(), newWidth, newHeight, 
                GL_UNSIGNED_BYTE, pTempData) != 0)
          {
            Except(Exception::ERR_INTERNAL_ERROR, 
                "Error while rescaling image!", "GLTexture::rescaleNPower2");
          }
          mGLSupport.end_context();

          Image::applyGamma( pTempData, mGamma, newImageSize, mSrcBpp );

          mSrcWidth = mWidth = newWidth; 
          mSrcHeight = mHeight = newHeight;
        }
        else
        {
          pTempData = new uchar[ src.getSize() ];
          memcpy( pTempData, src.getData(), src.getSize() );
          Image::applyGamma( pTempData, mGamma, src.getSize(), mSrcBpp );
        }

        return pTempData;
    }

    void GLTexture::loadImage( const Image& img )
    {
        std::vector<Image> images;
		
        images.push_back(img);
        loadImages(images);
        images.clear();
    }

    void GLTexture::loadImages( const std::vector<Image>& images )
    {
        bool useSoftwareMipmaps = true;

        if( mIsLoaded )
        {
            std::cout << "Unloading image" << std::endl;
            unload();
        }

        // Create the GL texture
        mGLSupport.begin_context();
        glGenTextures( 1, &mTextureID );
        glBindTexture( getGLTextureType(), mTextureID );

        if(mNumMipMaps && Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_AUTOMIPMAP))
        {
            glTexParameteri( getGLTextureType(), GL_GENERATE_MIPMAP, GL_TRUE );
            useSoftwareMipmaps = false;
        }

        for(size_t i = 0; i < images.size(); i++)
        {
            Image img = images[i];

            LogManager::getSingleton().logMessage( 
                LML_NORMAL,
                "GLTexture: Loading %s with %d mipmaps from Image.", 
                mName.c_str(), mNumMipMaps );        

            mFormat = img.getFormat();

            mSrcBpp = Image::PF2BPP(mFormat);
            mHasAlpha = img.getHasAlpha();

            mSrcWidth = img.getWidth();
            mSrcHeight = img.getHeight();
            // Same dest dimensions for GL
            mWidth = mSrcWidth;
            mHeight = mSrcHeight;

            mDepth = img.getDepth();

            unsigned short imageMipmaps = img.getNumMipmaps();
            if(imageMipmaps)
                mNumMipMaps = imageMipmaps;

            glTexParameteri(getGLTextureType(), GL_TEXTURE_MAX_LEVEL, mNumMipMaps);

            uchar *pTempData = rescaleNPower2(img);

            generateMipMaps( pTempData, useSoftwareMipmaps, img.hasFlag(IF_COMPRESSED), i );
            delete [] pTempData;
        }
        mGLSupport.end_context();

        // Update size (the final size, not including temp space)
        unsigned short bytesPerPixel = mFinalBpp >> 3;
        if( !mHasAlpha && mFinalBpp == 32 )
        {
            bytesPerPixel--;
        }
        mSize = mWidth * mHeight * bytesPerPixel;

        mIsLoaded = true;     
    }
    
    void GLTexture::createRenderTexture(void)
    {
        if (this->getTextureType() != TEX_TYPE_2D)
            Except( Exception::UNIMPLEMENTED_FEATURE, "**** Create render texture implemented only for 2D textures!!! ****", "GLTexture::createRenderTexture" );

        // Create the GL texture
        mGLSupport.begin_context();
        glGenTextures( 1, &mTextureID );
        glBindTexture( GL_TEXTURE_2D, mTextureID );

        glTexImage2D( GL_TEXTURE_2D, 0, getGLTextureFormat(),
            mWidth, mHeight, 0, getGLTextureFormat(), GL_UNSIGNED_BYTE, 0 );

        // This needs to be set otherwise the texture doesn't get rendered
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mNumMipMaps );
        mGLSupport.end_context();
    }

    void GLTexture::load()
    {
        if( mUsage == TU_RENDERTARGET )
        {
            createRenderTexture();
            mIsLoaded = true;     
        }
        else
        {
            if(mTextureType == TEX_TYPE_1D || mTextureType == TEX_TYPE_2D || 
                mTextureType == TEX_TYPE_3D)
            {
                Image img;
                img.load( mName );

                if (getName().endsWith(".dds") && img.hasFlag(IF_CUBEMAP))
                {
                    Image newImage;
                    std::vector<Image> images;
                    uint imageSize = img.getSize() / 6;

                    mTextureType = TEX_TYPE_CUBE_MAP;

                    uint offset = 0;
                    for(int i = 0; i < 6; i++)
                    {
                        DataChunk chunk(img.getData() + offset, imageSize);
                        newImage.loadRawData(chunk, img.getWidth(), 
                            img.getHeight(), img.getFormat());
                        offset += imageSize;
                        images.push_back(newImage);
                    }

                    loadImages( images );
                    images.clear();
                }
                else
                {
                    // If this is a dds volumetric texture set the texture type flag accordingly.
                    if(getName().endsWith(".dds") && img.getDepth() > 1)
                        mTextureType = TEX_TYPE_3D;

                    loadImage( img );
                }
            }
            else if (mTextureType == TEX_TYPE_CUBE_MAP)
            {
                Image img;
                String baseName, ext;
                std::vector<Image> images;
                static const String suffixes[6] = {"_rt", "_lf", "_up", "_dn", "_fr", "_bk"};

                for(size_t i = 0; i < 6; i++)
                {
                    size_t pos = mName.find_last_of(".");
                    baseName = mName.substr(0, pos);
                    ext = mName.substr(pos);
                    String fullName = baseName + suffixes[i] + ext;

                    img.load( fullName );
                    images.push_back(img);
                }

                loadImages( images );
                images.clear();
            }
            else
                Except( Exception::UNIMPLEMENTED_FEATURE, "**** Unknown texture type ****", "GLTexture::load" );
        }
    }
    
    void GLTexture::unload()
    {
        if( mIsLoaded )
        {
            glDeleteTextures( 1, &mTextureID );
            mIsLoaded = false;
        }
    }

    void GLTexture::generateMipMaps( uchar *data, bool useSoftware, 
        bool isCompressed, size_t faceNumber )
    {
        mGLSupport.begin_context();
        if(useSoftware && mNumMipMaps)
        {
            if(mTextureType == TEX_TYPE_1D)
            {
                gluBuild1DMipmaps(
                    getGLTextureType(), mHasAlpha ? GL_RGBA8 : GL_RGB8, 
                    mSrcWidth, getGLTextureFormat(), GL_UNSIGNED_BYTE, data);
            }
            else if (mTextureType == TEX_TYPE_3D)
            {
                /* Requires GLU 1.3 which is harder to come by
                   Most 3D textures don't need mipmaps?
                gluBuild3DMipmaps(
                    getGLTextureType(), mHasAlpha ? GL_RGBA8 : GL_RGB8, 
                    mSrcWidth, mSrcHeight, mDepth, getGLTextureFormat(), 
                    GL_UNSIGNED_BYTE, data);
                */
                glTexImage3D(
                    getGLTextureType(), 0, mHasAlpha ? GL_RGBA8 : GL_RGB8, 
                    mSrcWidth, mSrcHeight, mDepth, 0, getGLTextureFormat(), 
                    GL_UNSIGNED_BYTE, data );
            }
            else
            {
                gluBuild2DMipmaps(
                    mTextureType == TEX_TYPE_CUBE_MAP ? 
                        GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceNumber : 
                        getGLTextureType(), 
                    mHasAlpha ? GL_RGBA8 : GL_RGB8, mSrcWidth, mSrcHeight, 
                    getGLTextureFormat(), GL_UNSIGNED_BYTE, data);
            }
        }
        else
        {
            if(mTextureType == TEX_TYPE_1D)
            {
                glTexImage1D(
                    getGLTextureType(), 0, mHasAlpha ? GL_RGBA8 : GL_RGB8, 
                    mSrcWidth, 0, getGLTextureFormat(), GL_UNSIGNED_BYTE, data);
            }
            else if (mTextureType == TEX_TYPE_3D)
            {
                glTexImage3D(
                    getGLTextureType(), 0, mHasAlpha ? GL_RGBA8 : GL_RGB8, 
                    mSrcWidth, mSrcHeight, mDepth, 0, getGLTextureFormat(), 
                    GL_UNSIGNED_BYTE, data );
            }
            else
            {
			    if(isCompressed && Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability( RSC_TEXTURE_COMPRESSION_DXT ))
                {    
                    unsigned short blockSize = (mFormat == PF_DXT1) ? 8 : 16;
                    int size = ((mWidth+3)/4)*((mHeight+3)/4)*blockSize; 

                    glCompressedTexImage2DARB_ptr(
                        mTextureType == TEX_TYPE_CUBE_MAP ?
                            GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceNumber :
                            getGLTextureType(), 0,
                         getGLTextureFormat(), mSrcWidth, mSrcHeight, 0, 
                         size, data);
                }
                else
                {
                    glTexImage2D(
                        mTextureType == TEX_TYPE_CUBE_MAP ? 
                            GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceNumber : 
                            getGLTextureType(), 0, 
                        mHasAlpha ? GL_RGBA8 : GL_RGB8, mSrcWidth, mSrcHeight, 
                        0, getGLTextureFormat(), GL_UNSIGNED_BYTE, data );
                }
            }
        }

        mGLSupport.end_context();
    }

    void GLRenderTexture::_copyToTexture(void)
    {
		
        glBindTexture(GL_TEXTURE_2D,
            static_cast<GLTexture*>(mTexture)->getGLID());
			
        glCopyTexSubImage2D(GL_TEXTURE_2D, mTexture->getNumMipMaps(), 0, 0,
            0, 0, mWidth, mHeight);

    }
}


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
#include "OgreImage.h"
#include "OgreLogManager.h"
#include "OgreCamera.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreCodec.h"
#include "OgreImageCodec.h"

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

    GLTexture::GLTexture(ResourceManager* creator, const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader, GLSupport& support) 
        : Texture(creator, name, handle, group, isManual, loader),
        mGLSupport(support), mTextureID(0)
    {
    }


    GLTexture::~GLTexture()
    {
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload(); 
    }

    GLenum GLTexture::getGLTextureTarget(void) const
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

    GLenum GLTexture::getGLTextureOriginFormat(void) const
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
            case PF_FP_R16G16B16:
                return GL_RGB;
            case PF_FP_R16G16B16A16:
                return GL_RGBA;
            case PF_FP_R32G32B32:
                return GL_RGB;
            case PF_FP_R32G32B32A32:
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

    GLenum GLTexture::getGLTextureOriginDataType(void) const
    {
        switch(mFormat)
        {
            case PF_L8:
            case PF_R8G8B8:
            case PF_B8G8R8:
            case PF_B8G8R8A8:
            case PF_A8R8G8B8:
                return GL_UNSIGNED_BYTE;
            case PF_FP_R16G16B16:
            case PF_FP_R16G16B16A16:
                return 0; // GL_HALF_FLOAT_ARB -- nyi
            case PF_FP_R32G32B32:
            case PF_FP_R32G32B32A32:
                return GL_FLOAT;
            default:
                return 0;
        }
    }

    GLenum GLTexture::getGLTextureInternalFormat(void) const
    {
        switch(mFormat) {
            case PF_L8:
                return GL_LUMINANCE8;
            case PF_L16:
                return GL_LUMINANCE16;
            case PF_A8:
                return GL_ALPHA8;
            case PF_A4L4:
            case PF_L4A4:
                return GL_LUMINANCE4_ALPHA4;
            case PF_R5G6B5:
            case PF_B5G6R5:
                return GL_RGB5;
            case PF_A4R4G4B4:
            case PF_B4G4R4A4:
                return GL_RGBA4;
            case PF_R8G8B8:
            case PF_B8G8R8:
                return GL_RGB8;
            case PF_A8R8G8B8:
            case PF_B8G8R8A8:
                return GL_RGBA8;
            case PF_A2R10G10B10:
            case PF_B10G10R10A2:
                return GL_RGB10_A2;
            case PF_FP_R16G16B16:
                return GL_RGB_FLOAT16_ATI;
                //    return GL_RGB16F_ARB;
            case PF_FP_R16G16B16A16:
                return GL_RGBA_FLOAT16_ATI;
                //    return GL_RGBA16F_ARB;
            case PF_FP_R32G32B32:
                return GL_RGB_FLOAT32_ATI;
                //    return GL_RGB32F_ARB;
            case PF_FP_R32G32B32A32:
                return GL_RGBA_FLOAT32_ATI;
                //    return GL_RGBA32F_ARB;
            default:
                return GL_RGBA8; // 0?
        }
    }


    void GLTexture::blitToTexture( 
        const Image& src, 
        unsigned uStartX, unsigned uStartY )
    {
		if (this->getTextureType() != TEX_TYPE_2D)
          Except( Exception::UNIMPLEMENTED_FEATURE, "**** Blit to texture implemented only for 2D textures!!! ****", "GLTexture::blitToTexture" );

        glBindTexture( GL_TEXTURE_2D, mTextureID );
        glTexSubImage2D( 
            GL_TEXTURE_2D, 0, 
            uStartX, uStartY,
            src.getWidth(), src.getHeight(),
            getGLTextureOriginFormat(),
            getGLTextureOriginDataType(), src.getData() );
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

        uchar* pTempData;
        if(!Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_NON_POWER_OF_2_TEXTURES) && 
            (newWidth != mSrcWidth || newHeight != mSrcHeight))
        {
            unsigned int newImageSize = newWidth * newHeight * 
                Image::getNumElemBytes(mFormat);

            pTempData = new uchar[ newImageSize ];

            if(gluScaleImage(getGLTextureOriginFormat(), mSrcWidth, mSrcHeight,
                getGLTextureOriginDataType(), src.getData(), newWidth, newHeight, 
                getGLTextureOriginDataType(), pTempData) != 0)
            {
                Except(Exception::ERR_INTERNAL_ERROR, 
                    "Error while rescaling image!", 
                    "GLTexture::rescaleNPower2");
            }

            Image::applyGamma( pTempData, mGamma, newImageSize, mSrcBpp );

            mSrcWidth = mWidth = newWidth; 
            mSrcHeight = mHeight = newHeight;
        } else  {
            // The texture is already a power of two, or the RenderSystem supports non-power-of-2 textures
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

    void GLTexture::createInternalResources(void)
    {
        glGenTextures( 1, &mTextureID );
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
        createInternalResources();

        glBindTexture( getGLTextureTarget(), mTextureID );

        if(mNumMipMaps && Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_AUTOMIPMAP))
        {
            glTexParameteri( getGLTextureTarget(), GL_GENERATE_MIPMAP, GL_TRUE );
            useSoftwareMipmaps = false;
        }

        for(size_t i = 0; i < images.size(); i++)
        {
            const Image& img = images[i];

            StringUtil::StrStreamType str;
            str << "GLTexture: Loading " << mName << " with "
                << mNumMipMaps << " mipmaps from Image.";
            LogManager::getSingleton().logMessage( 
                LML_NORMAL, str.str());

            mFormat = img.getFormat();

            mSrcBpp = Image::PF2BPP(mFormat);
            mHasAlpha = img.getHasAlpha();

            mSrcWidth = img.getWidth();
            mSrcHeight = img.getHeight();
            // Same dest dimensions for GL
            mWidth = mSrcWidth;
            mHeight = mSrcHeight;
            mDepth = img.getDepth();

			// Never *generate* mipmaps for floating point textures. This is buggy in current
			// GLU implementations
			if(Image::formatIsFloat(mFormat))
				mNumMipMaps = 0;

			// The custom mipmaps in the image have priority over everything
            unsigned short imageMipmaps = img.getNumMipmaps();
            if(imageMipmaps)
                mNumMipMaps = imageMipmaps;
			glTexParameteri(getGLTextureTarget(), GL_TEXTURE_MAX_LEVEL, mNumMipMaps);

            uchar *pTempData = rescaleNPower2(img);

            generateMipMaps( pTempData, useSoftwareMipmaps, img.hasFlag(IF_COMPRESSED), i );
            delete [] pTempData;
        }

        // Update size (the final size, not including temp space)
        mSize = mWidth * mHeight * Image::getNumElemBytes(mFormat);

        mIsLoaded = true;     
    }
    
    void GLTexture::createRenderTexture(void)
    {
        if (this->getTextureType() != TEX_TYPE_2D)
            Except( Exception::UNIMPLEMENTED_FEATURE, "**** Create render texture implemented only for 2D textures!!! ****", "GLTexture::createRenderTexture" );

        // Create the GL texture
        createInternalResources();

        glBindTexture( GL_TEXTURE_2D, mTextureID );

        glTexImage2D( GL_TEXTURE_2D, 0, getGLTextureInternalFormat(),
            mWidth, mHeight, 0, getGLTextureOriginFormat(), getGLTextureOriginDataType(), 0 );

        // This needs to be set otherwise the texture doesn't get rendered
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mNumMipMaps );
    }

    void GLTexture::loadImpl()
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
                img.load(mName, mGroup);

                if (StringUtil::endsWith(getName(), "dds") && img.hasFlag(IF_CUBEMAP))
                {
                    Image newImage;
                    std::vector<Image> images;
                    uint imageSize = img.getSize() / 6;

                    mTextureType = TEX_TYPE_CUBE_MAP;

                    uint offset = 0;
                    for(int i = 0; i < 6; i++)
                    {
                        DataStreamPtr stream(
                            new MemoryDataStream(img.getData() + offset, imageSize, false));
                        newImage.loadRawData(stream, img.getWidth(), 
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
                    if(StringUtil::endsWith(getName(), ".dds") && img.getDepth() > 1)
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

                    img.load(fullName, mGroup);
                    images.push_back(img);
                }

                loadImages( images );
                images.clear();
            }
            else
                Except( Exception::UNIMPLEMENTED_FEATURE, "**** Unknown texture type ****", "GLTexture::load" );
        }
    }
    
    void GLTexture::unloadImpl()
    {
        glDeleteTextures( 1, &mTextureID );
    }

    void GLTexture::generateMipMaps( const uchar *data, bool useSoftware, 
        bool isCompressed, size_t faceNumber )
    {
        if(useSoftware && mNumMipMaps)
        {
            if(mTextureType == TEX_TYPE_1D)
            {
                gluBuild1DMipmaps(
                    getGLTextureTarget(), getGLTextureInternalFormat(),
                    mSrcWidth, getGLTextureOriginFormat(), getGLTextureOriginDataType(), data);
            }
            else if (mTextureType == TEX_TYPE_3D)
            {
                /* Requires GLU 1.3 which is harder to come by
                   Most 3D textures don't need mipmaps?
                gluBuild3DMipmaps(
                    getGLTextureTarget(), getGLTextureInternalFormat(), 
                    mSrcWidth, mSrcHeight, mDepth, getGLTextureOriginFormat(), 
                    getGLTextureOriginDataType(), data);
                */
                glTexImage3D(
                    getGLTextureTarget(), 0, getGLTextureInternalFormat(), 
                    mSrcWidth, mSrcHeight, mDepth, 0, getGLTextureOriginFormat(), 
                    getGLTextureOriginDataType(), data );
            }
            else
            {
                gluBuild2DMipmaps(
                    mTextureType == TEX_TYPE_CUBE_MAP ? 
                        GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceNumber : 
                        getGLTextureTarget(), 
                    getGLTextureInternalFormat(), mSrcWidth, mSrcHeight, 
                    getGLTextureOriginFormat(), getGLTextureOriginDataType(), data);
            }
        }
        else
        {
            if(mTextureType == TEX_TYPE_1D)
            {
                glTexImage1D(
                    getGLTextureTarget(), 0, getGLTextureInternalFormat(), 
                    mSrcWidth, 0, getGLTextureOriginFormat(), getGLTextureOriginDataType(), data);
            }
            else if (mTextureType == TEX_TYPE_3D)
            {
                glTexImage3D(
                    getGLTextureTarget(), 0, getGLTextureInternalFormat(), 
                    mSrcWidth, mSrcHeight, mDepth, 0, getGLTextureOriginFormat(), 
                    getGLTextureOriginDataType(), data );
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
                            getGLTextureTarget(), 0,
                         getGLTextureOriginFormat(), mSrcWidth, mSrcHeight, 0, 
                         size, data);
                }
                else
                {
                    glTexImage2D(
                        mTextureType == TEX_TYPE_CUBE_MAP ? 
                            GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceNumber : 
                            getGLTextureTarget(), 0, 
                        getGLTextureInternalFormat(), mSrcWidth, mSrcHeight, 
                        0, getGLTextureOriginFormat(), getGLTextureOriginDataType(), data );
                }
            }
        }

    }

    void GLRenderTexture::_copyToTexture(void)
    {		
        glBindTexture(GL_TEXTURE_2D, mGLTexture->getGLID());
			
        glCopyTexSubImage2D(GL_TEXTURE_2D, mGLTexture->getNumMipMaps(), 0, 0,
            0, 0, mWidth, mHeight);

    }
    
    void GLRenderTexture::writeContentsToFile( const String & filename ) 
    {
        ImageCodec::ImageData *imgData = new ImageCodec::ImageData();
        
        imgData->width = mGLTexture->getWidth();
        imgData->height = mGLTexture->getHeight();
        imgData->format = PF_R8G8B8;

        // Allocate buffer 
        uchar* pBuffer = new uchar[imgData->width * imgData->height * 3];

        // Read pixels
        // I love GL: it does all the locking & colour conversion for us
        glBindTexture(GL_TEXTURE_2D, mGLTexture->getGLID());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pBuffer);

        // Wrap buffer in a chunk
        DataStreamPtr stream(new MemoryDataStream(
            pBuffer, imgData->width * imgData->height * 3, false));

        // Need to flip the read data over in Y though
        Image img;
        img.loadRawData(stream, imgData->width, imgData->height, imgData->format );
        img.flipAroundX();

        MemoryDataStreamPtr streamFlipped(
            new MemoryDataStream(img.getData(), stream->size(), false));

        // Get codec 
        size_t pos = filename.find_last_of(".");
            String extension;
        if( pos == String::npos )
            Except(
                Exception::ERR_INVALIDPARAMS, 
            "Unable to determine image type for '" + filename + "' - invalid extension.",
                "GLRenderTexture::writeContentsToFile" );

        while( pos != filename.length() - 1 )
            extension += filename[++pos];

        // Get the codec
        Codec * pCodec = Codec::getCodec(extension);

        // Write out
        Codec::CodecDataPtr codecDataPtr(imgData);
        pCodec->codeToFile(streamFlipped, filename, codecDataPtr);

        delete [] pBuffer;
    }
}


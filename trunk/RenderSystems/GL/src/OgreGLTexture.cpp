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
#include "OgreGLPixelFormat.h"
#include "OgreGLHardwarePixelBuffer.h"

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

/// New image loading code using pixel buffers
#define EXPERIMENTAL

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
            GLPixelUtil::getGLOriginFormat(mFormat),
            GLPixelUtil::getGLOriginDataType(mFormat), src.getData() );
    }

	void GLTexture::generateMipmaps( const uchar *data, bool useSoftware, 
        bool isCompressed, size_t faceNumber )
    {
        if(useSoftware && mNumMipmaps)
        {
            if(mTextureType == TEX_TYPE_1D)
            {
                gluBuild1DMipmaps(
                    getGLTextureTarget(), GLPixelUtil::getClosestGLInternalFormat(mFormat),
                    mSrcWidth, GLPixelUtil::getGLOriginFormat(mFormat), GLPixelUtil::getGLOriginDataType(mFormat), data);
            }
            else if (mTextureType == TEX_TYPE_3D)
            {
                /* Requires GLU 1.3 which is harder to come by
                   Most 3D textures don't need mipmaps?
                gluBuild3DMipmaps(
                    getGLTextureTarget(), GLPixelUtil::getClosestGLInternalFormat(mFormat), 
                    mSrcWidth, mSrcHeight, mDepth, GLPixelUtil::getGLOriginFormat(mFormat), 
                    GLPixelUtil::getGLOriginDataType(mFormat), data);
                */
                glTexImage3D(
                    getGLTextureTarget(), 0, GLPixelUtil::getClosestGLInternalFormat(mFormat), 
                    mSrcWidth, mSrcHeight, mDepth, 0, GLPixelUtil::getGLOriginFormat(mFormat), 
                    GLPixelUtil::getGLOriginDataType(mFormat), data );
            }
            else
            {
                gluBuild2DMipmaps(
                    mTextureType == TEX_TYPE_CUBE_MAP ? 
                        GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceNumber : 
                        getGLTextureTarget(), 
                    GLPixelUtil::getClosestGLInternalFormat(mFormat), mSrcWidth, mSrcHeight, 
                    GLPixelUtil::getGLOriginFormat(mFormat), GLPixelUtil::getGLOriginDataType(mFormat), data);
            }
        }
        else
        {
            if(mTextureType == TEX_TYPE_1D)
            {
                glTexImage1D(
                    getGLTextureTarget(), 0, GLPixelUtil::getClosestGLInternalFormat(mFormat), 
                    mSrcWidth, 0, GLPixelUtil::getGLOriginFormat(mFormat), GLPixelUtil::getGLOriginDataType(mFormat), data);
            }
            else if (mTextureType == TEX_TYPE_3D)
            {
                glTexImage3D(
                    getGLTextureTarget(), 0, GLPixelUtil::getClosestGLInternalFormat(mFormat), 
                    mSrcWidth, mSrcHeight, mDepth, 0, GLPixelUtil::getGLOriginFormat(mFormat), 
                    GLPixelUtil::getGLOriginDataType(mFormat), data );
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
                         GLPixelUtil::getClosestGLInternalFormat(mFormat), mSrcWidth, mSrcHeight, 0, 
                         size, data);
                }
                else
                {
                    glTexImage2D(
                        mTextureType == TEX_TYPE_CUBE_MAP ? 
                            GL_TEXTURE_CUBE_MAP_POSITIVE_X + faceNumber : 
                            getGLTextureTarget(), 0, 
                        GLPixelUtil::getClosestGLInternalFormat(mFormat), mSrcWidth, mSrcHeight, 
                        0, GLPixelUtil::getGLOriginFormat(mFormat), GLPixelUtil::getGLOriginDataType(mFormat), data );
                }
            }
        }

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
                PixelUtil::getNumElemBytes(mFormat);

            pTempData = new uchar[ newImageSize ];

            if(gluScaleImage(GLPixelUtil::getGLOriginFormat(mFormat), mSrcWidth, mSrcHeight,
                GLPixelUtil::getGLOriginDataType(mFormat), src.getData(), newWidth, newHeight, 
                GLPixelUtil::getGLOriginDataType(mFormat), pTempData) != 0)
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

	
	//* Creation / loading methods ********************************************
#ifndef EXPERIMENTAL
	void GLTexture::createInternalResources(void)
    {	
		// Generate texture name
        glGenTextures( 1, &mTextureID );
	}
#else
	void GLTexture::createInternalResources(void)
    {
		// Adjust requested parameters to capabilities

		// Check power-of-two size if required
        unsigned int newWidth = (1 << mostSignificantBitSet(mWidth));
        if (newWidth != mWidth)
            newWidth <<= 1;

        unsigned int newHeight = (1 << mostSignificantBitSet(mHeight));
        if (newHeight != mHeight)
            newHeight <<= 1;
			
		unsigned int newDepth = (1 << mostSignificantBitSet(mDepth));
        if (newDepth != mDepth)
            newDepth <<= 1;

        if(!Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_NON_POWER_OF_2_TEXTURES))
		{
			mHeight = newHeight;
			mWidth = newWidth;
			mDepth = newDepth;
		}
		
		// Check compressed texture support
		// if a compressed format not supported, revert to PF_A8R8G8B8
		if(PixelUtil::isCompressed(mFormat) &&
		 !Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability( RSC_TEXTURE_COMPRESSION_DXT ))
		{
			mFormat = PF_A8R8G8B8;
		}
		// if floating point textures not supported, revert to PF_A8R8G8B8
		if(PixelUtil::isFloatingPoint(mFormat) &&
		 !Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability( RSC_TEXTURE_FLOAT ))
		{
			mFormat = PF_A8R8G8B8;
		}
		
		// Check requested number of mipmaps
		// Zero means create mip levels until 1x1
		size_t maxMips = GLPixelUtil::getMaxMipmaps(mWidth, mHeight, mDepth, mFormat);
		if(mNumMipmaps==0 || mNumMipmaps>maxMips)
			mNumMipmaps = maxMips;
		
		// Generate texture name
        glGenTextures( 1, &mTextureID );
		
		// Set texture type
		glBindTexture( getGLTextureTarget(), mTextureID );
        
		// This needs to be set otherwise the texture doesn't get rendered
        glTexParameteri( getGLTextureTarget(), GL_TEXTURE_MAX_LEVEL, mNumMipmaps - 1 );
		
		// If we can do automip generation, do so
		// TODO: make this a choice
		if(mNumMipmaps>1 &&
		 Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_AUTOMIPMAP))
        {
            glTexParameteri( getGLTextureTarget(), GL_GENERATE_MIPMAP, GL_TRUE );
        }
		
		// Allocate internal buffer so that glTexSubImageXD can be used
		// Internal format
		GLenum format = GLPixelUtil::getClosestGLInternalFormat(mFormat);
		size_t width = mWidth;
		size_t height = mHeight;
		size_t depth = mDepth;
		if(PixelUtil::isCompressed(mFormat))
		{
			// Compressed formats
			size_t size = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
			// Provide temporary buffer filled with zeroes as glCompressedTexImageXD does not
			// accept a 0 pointer like normal glTexImageXD
			// Run through this process for every mipmap to pregenerate mipmap piramid
			uint8 *tmpdata = new uint8[size];
			std::fill(tmpdata, tmpdata+size, 0);
			
			for(int mip=0; mip<mNumMipmaps; mip++)
			{
			//int mip = 0;
				size = PixelUtil::getMemorySize(width, height, depth, mFormat);
				switch(mTextureType)
				{
					case TEX_TYPE_1D:
						glCompressedTexImage1DARB_ptr(GL_TEXTURE_1D, mip, format, 
							width, 0, 
							size, tmpdata);
						break;
					case TEX_TYPE_2D:
						glCompressedTexImage2DARB_ptr(GL_TEXTURE_2D, mip, format,
							width, height, 0, 
							size, tmpdata);
						break;
					case TEX_TYPE_3D:
						glCompressedTexImage3DARB_ptr(GL_TEXTURE_3D, mip, format,
							width, height, depth, 0, 
							size, tmpdata);
						break;
					case TEX_TYPE_CUBE_MAP:
						for(int face=0; face<6; face++) {
							glCompressedTexImage2DARB_ptr(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, format,
								width, height, 0, 
								size, tmpdata);
						}
						break;
				};
				width = (width+1)/2;
				height = (height+1)/2;
				depth = (depth+1)/2;
			}
			delete tmpdata;
		}
		else
		{
			// Run through this process to pregenerate mipmap piramid
			for(int mip=0; mip<mNumMipmaps; mip++)
			{
				// Normal formats
				switch(mTextureType)
				{
					case TEX_TYPE_1D:
						glTexImage1D(GL_TEXTURE_1D, mip, format,
							width, 0, 
							GL_RGBA, GL_UNSIGNED_INT, 0);
	
						break;
					case TEX_TYPE_2D:
						glTexImage2D(GL_TEXTURE_2D, mip, format,
							width, height, 0, 
							GL_RGBA, GL_UNSIGNED_INT, 0);
						break;
					case TEX_TYPE_3D:
						glTexImage3D(GL_TEXTURE_3D, mip, format,
							width, height, depth, 0, 
							GL_RGBA, GL_UNSIGNED_INT, 0);
						break;
					case TEX_TYPE_CUBE_MAP:
						for(int face=0; face<6; face++) {
							glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, format,
								width, height, 0, 
								GL_RGBA, GL_UNSIGNED_INT, 0);
						}
						break;
				};
				width = (width+1)/2;
				height = (height+1)/2;
				depth = (depth+1)/2;
			}
		}
		_createSurfaceList();
		// Get final internal format
		mFormat = getBuffer(0,0)->getFormat();
	}
#endif	
	
    void GLTexture::loadImage( const Image& img )
    {
        std::vector<Image> images;
		
        images.push_back(img);
        loadImages(images);
        images.clear();
    }
#ifndef EXPERIMENTAL
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

        if(mNumMipmaps && Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_AUTOMIPMAP))
        {
            glTexParameteri( getGLTextureTarget(), GL_GENERATE_MIPMAP, GL_TRUE );
            useSoftwareMipmaps = false;
        }

        for(size_t i = 0; i < images.size(); i++)
        {
            const Image& img = images[i];

            StringUtil::StrStreamType str;
            str << "GLTexture: Loading " << mName 
                << "(" << PixelUtil::getFormatName(img.getFormat()) << ") with "
                << mNumMipmaps << " mipmaps from Image.";
            LogManager::getSingleton().logMessage( 
                LML_NORMAL, str.str());

            mFormat = img.getFormat();

            mSrcBpp = PixelUtil::getNumElemBits(mFormat);
            mHasAlpha = img.getHasAlpha();

            mSrcWidth = img.getWidth();
            mSrcHeight = img.getHeight();
            // Same dest dimensions for GL
            mWidth = mSrcWidth;
            mHeight = mSrcHeight;
            mDepth = img.getDepth();

			// Never *generate* mipmaps for floating point textures. This is buggy in current
			// GLU implementations
			if(PixelUtil::getFlags(mFormat) & PFF_FLOAT)
				mNumMipmaps = 0;

			// The custom mipmaps in the image have priority over everything
            unsigned short imageMipmaps = img.getNumMipmaps();
            if(imageMipmaps)
                mNumMipmaps = imageMipmaps;
			glTexParameteri(getGLTextureTarget(), GL_TEXTURE_MAX_LEVEL, mNumMipmaps);

            uchar *pTempData = rescaleNPower2(img);

            generateMipmaps( pTempData, useSoftwareMipmaps, img.hasFlag(IF_COMPRESSED), i );
            delete [] pTempData;
        }

        // Update size (the final size, not including temp space)
        mSize = mWidth * mHeight * PixelUtil::getNumElemBytes(mFormat);

        mIsLoaded = true;     
    }
#else
    void GLTexture::loadImages( const std::vector<Image>& images )
    {
		if(images.size() < 1)
			Except(Exception::ERR_INVALIDPARAMS, "Cannot load empty vector of images",
			 "GLTexture::loadImages");
        
 
        if( mIsLoaded )
        {
            std::cout << "Unloading image" << std::endl;
            unload();
        }

		// Set desired texture size and properties from images[0]
		mWidth = images[0].getWidth();
		mHeight = images[0].getHeight();
		mDepth = images[0].getDepth();
		mFormat = images[0].getFormat();
		mSrcBpp = PixelUtil::getNumElemBits(mFormat);
		mHasAlpha = PixelUtil::hasAlpha(mFormat);
		// The custom mipmaps in the image have priority over everything
		bool doCustomMipmaps = images[0].getNumMipmaps()>0;
		
		if(doCustomMipmaps) {
			mNumMipmaps = images[0].getNumMipmaps();
			// TODO: disable flag for auto mip generation
		}
		
        // Create the GL texture
        createInternalResources();

        for(size_t i = 0; i < images.size(); i++)
        {
            const Image& img = images[i];

            StringUtil::StrStreamType str;
            str << "GLTexture: Loading face=" << i << " filename=" << mName 
                << "(" << PixelUtil::getFormatName(img.getFormat()) << ") with "
                << mNumMipmaps << " mipmaps from Image.";
            LogManager::getSingleton().logMessage( 
                LML_NORMAL, str.str());

			// Destination: entire texture. blitFromMemory does the scaling to
			// a power of two for us when needed
        	PixelBox src = img.getPixelBox(0, 0);
			// TODO manual mips
			
			if(mGamma != 1.0f) {
				// Do gamma correction in temporary buffer
				PixelBox corrected = PixelBox(src.getWidth(), src.getHeight(), src.getDepth(), src.format);
				corrected.data = new uint8[ corrected.getConsecutiveSize() ];
				PixelUtil::bulkPixelConversion(src, corrected);
				
				Image::applyGamma(static_cast<uint8*>(corrected.data), mGamma, corrected.getConsecutiveSize(), 
					PixelUtil::getNumElemBits(src.format));
				
				getBuffer(i, 0)->blitFromMemory(corrected);
				delete [] static_cast<uint8*>(corrected.data);
			}
			else 
			{
            	getBuffer(i, 0)->blitFromMemory(src);
			}
			
        }

        // Update size (the final size, not including temp space)
        mSize = mWidth * mHeight * PixelUtil::getNumElemBytes(mFormat);

        mIsLoaded = true;
    }
#endif    
    void GLTexture::createRenderTexture(void)
    {
        if (this->getTextureType() != TEX_TYPE_2D)
            Except( Exception::UNIMPLEMENTED_FEATURE, "**** Create render texture implemented only for 2D textures!!! ****", "GLTexture::createRenderTexture" );

        // Create the GL texture
		// This already does everything neccessary
        createInternalResources();
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

                if (img.hasFlag(IF_CUBEMAP))
                {
                    // Split cubemap into six different images
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
                    // If this is a volumetric texture set the texture type flag accordingly.
                    if(img.getDepth() > 1)
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
	
	//*************************************************************************
    
    void GLTexture::unloadImpl()
    {
		mSurfaceList.clear();
        glDeleteTextures( 1, &mTextureID );
    }

	
	//---------------------------------------------------------------------------------------------
	void GLTexture::_createSurfaceList()
	{
		mSurfaceList.clear();
		// Make our understanding of the number of mips matches the GL one
		glBindTexture( getGLTextureTarget(), mTextureID );
		GLint value;
		glGetTexParameteriv( getGLTextureTarget(), GL_TEXTURE_MAX_LEVEL, &value );
		mNumMipmaps = value + 1;
		
		// For all faces and mipmaps, store surfaces as HardwarePixelBufferSharedPtr
		bool wantGeneratedMips = true; // TODO make option on texture
		bool canMip = Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_AUTOMIPMAP);
		
		// Do mipmapping in software? (uses GLU) For some cards, this is still needed. Of course,
		// only when mipmap generation is desired.
		bool doSoftware = wantGeneratedMips && !canMip && getNumMipmaps()>1; 
		
		for(int face=0; face<getNumFaces(); face++)
		{
			for(int mip=0; mip<getNumMipmaps(); mip++)
			{
				mSurfaceList.push_back(HardwarePixelBufferSharedPtr(
					// TODO provide real usage instead of HBU_STATIC
					new GLHardwarePixelBuffer(getGLTextureTarget(), mTextureID, face, mip,
						HardwareBuffer::HBU_STATIC, doSoftware && mip==0)
						//HardwareBuffer::HBU_DYNAMIC, doSoftware && mip==0)
				));
			}
		}
	}
	
	//---------------------------------------------------------------------------------------------
	HardwarePixelBufferSharedPtr GLTexture::getBuffer(int face, int mipmap)
	{
		if(getTextureType() != TEX_TYPE_CUBE_MAP && face != 0)
			Except(Exception::ERR_INVALIDPARAMS, "Normal textures have only face 0",
					"GLTexture::getBuffer");
		if(face < 0 || face >= getNumFaces())
			Except(Exception::ERR_INVALIDPARAMS, "Face index out of range",
					"GLTexture::getBuffer");
		if(mipmap < 0 || mipmap >= mNumMipmaps)
			Except(Exception::ERR_INVALIDPARAMS, "Mipmap index out of range",
					"GLTexture::getBuffer");
		unsigned int idx = face*mNumMipmaps + mipmap;
		assert(idx < mSurfaceList.size());
		return mSurfaceList[idx];
	}
	
	//---------------------------------------------------------------------------------------------
    void GLRenderTexture::_copyToTexture(void)
    {		
        glBindTexture(GL_TEXTURE_2D, mGLTexture->getGLID());
			
        glCopyTexSubImage2D(GL_TEXTURE_2D, mGLTexture->getNumMipmaps(), 0, 0,
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


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
        mTextureID(0), mGLSupport(support)
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

	//* Creation / loading methods ********************************************
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
		if(mNumMipmaps>maxMips)
			mNumMipmaps = maxMips;
		
		// Generate texture name
        glGenTextures( 1, &mTextureID );
		
		// Set texture type
		glBindTexture( getGLTextureTarget(), mTextureID );
        
		// This needs to be set otherwise the texture doesn't get rendered
        glTexParameteri( getGLTextureTarget(), GL_TEXTURE_MAX_LEVEL, mNumMipmaps );
		
		// If we can do automip generation and the user desires this, do so
		if((mUsage & TU_AUTOMIPMAP) &&
		    mNumMipmaps &&
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
			memset(tmpdata, 0, size);
			
			for(int mip=0; mip<=mNumMipmaps; mip++)
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
			for(int mip=0; mip<=mNumMipmaps; mip++)
			{
				// Normal formats
				switch(mTextureType)
				{
					case TEX_TYPE_1D:
						glTexImage1D(GL_TEXTURE_1D, mip, format,
							width, 0, 
							GL_RGBA, GL_UNSIGNED_BYTE, 0);
	
						break;
					case TEX_TYPE_2D:
						glTexImage2D(GL_TEXTURE_2D, mip, format,
							width, height, 0, 
							GL_RGBA, GL_UNSIGNED_BYTE, 0);
						break;
					case TEX_TYPE_3D:
						glTexImage3D(GL_TEXTURE_3D, mip, format,
							width, height, depth, 0, 
							GL_RGBA, GL_UNSIGNED_BYTE, 0);
						break;
					case TEX_TYPE_CUBE_MAP:
						for(int face=0; face<6; face++) {
							glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, format,
								width, height, 0, 
								GL_RGBA, GL_UNSIGNED_BYTE, 0);
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
		mIsLoaded = true;
	}
	
    void GLTexture::createRenderTexture(void)
    {
        if (this->getTextureType() != TEX_TYPE_2D)
            Except( Exception::UNIMPLEMENTED_FEATURE, "**** Create render texture implemented only for 2D textures!!! ****", "GLTexture::createRenderTexture" );

        // Create the GL texture
		// This already does everything neccessary
        createInternalResources();
    }
	
	void GLTexture::loadImage( const Image& img )
    {
        std::vector<const Image*> images;
		
        images.push_back(&img);
        _loadImages(images);
        images.clear();
    }

    void GLTexture::loadImpl()
    {
        if( mUsage & TU_RENDERTARGET )
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

				// If this is a cube map, set the texture type flag accordingly.
                if (img.hasFlag(IF_CUBEMAP))
					mTextureType = TEX_TYPE_CUBE_MAP;
				// If this is a volumetric texture set the texture type flag accordingly.
				if(img.getDepth() > 1)
					mTextureType = TEX_TYPE_3D;

				loadImage( img );
            }
            else if (mTextureType == TEX_TYPE_CUBE_MAP)
            {
				if(StringUtil::endsWith(getName(), ".dds"))
				{
					// XX HACK there should be a better way to specify wether 
					// all faces are in the same file or not
					Image img;
                	img.load(mName, mGroup);
					loadImage( img );
				}
				else
				{
					String baseName, ext;
					std::vector<Image> images(6);
					std::vector<const Image*> imagePtrs;
					static const String suffixes[6] = {"_rt", "_lf", "_up", "_dn", "_fr", "_bk"};
	
					for(size_t i = 0; i < 6; i++)
					{
						size_t pos = mName.find_last_of(".");
						baseName = mName.substr(0, pos);
						ext = mName.substr(pos);
						String fullName = baseName + suffixes[i] + ext;
	
						images[i].load(fullName, mGroup);
						imagePtrs.push_back(&images[i]);
					}
	
					_loadImages( imagePtrs );
				}
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
		mNumMipmaps = value;
		
		// For all faces and mipmaps, store surfaces as HardwarePixelBufferSharedPtr
		bool wantGeneratedMips = (mUsage & TU_AUTOMIPMAP)!=0;
		bool canMip = Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_AUTOMIPMAP);
		
		// Do mipmapping in software? (uses GLU) For some cards, this is still needed. Of course,
		// only when mipmap generation is desired.
		bool doSoftware = wantGeneratedMips && !canMip && getNumMipmaps(); 
		
		for(int face=0; face<getNumFaces(); face++)
		{
			for(int mip=0; mip<=getNumMipmaps(); mip++)
			{
				mSurfaceList.push_back(HardwarePixelBufferSharedPtr(
					new GLHardwarePixelBuffer(getGLTextureTarget(), mTextureID, face, mip,
						static_cast<HardwareBuffer::Usage>(mUsage), doSoftware && mip==0)
				));
			}
		}
	}
	
	//---------------------------------------------------------------------------------------------
	HardwarePixelBufferSharedPtr GLTexture::getBuffer(size_t face, size_t mipmap)
	{
		if(face >= getNumFaces())
			Except(Exception::ERR_INVALIDPARAMS, "Face index out of range",
					"GLTexture::getBuffer");
		if(mipmap > mNumMipmaps)
			Except(Exception::ERR_INVALIDPARAMS, "Mipmap index out of range",
					"GLTexture::getBuffer");
		unsigned int idx = face*(mNumMipmaps+1) + mipmap;
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
		imgData->depth = 1;
        imgData->format = PF_BYTE_RGB;

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


/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#include "OgreStableHeaders.h"
#include "OgreLogManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreImage.h"
#include "OgreTexture.h"
#include "OgreException.h"

namespace Ogre {
	//--------------------------------------------------------------------------
    Texture::Texture(ResourceManager* creator, const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader)
        : Resource(creator, name, handle, group, isManual, loader),
            // init defaults; can be overridden before load()
            mHeight(512),
            mWidth(512),
            mDepth(1),
            mNumMipmaps(0),
            mGamma(1.0f),
            mTextureType(TEX_TYPE_2D),            
            mFormat(PF_A8R8G8B8),
            mUsage(TU_DEFAULT),
            // mSrcBpp inited later on
            mSrcWidth(0),
            mSrcHeight(0)
            // mFinalBpp inited later on by enable32bit
            // mHasAlpha inited later on            
    {

        enable32Bit(false);

        if (createParamDictionary("Texture"))
        {
            // Define the parameters that have to be present to load
            // from a generic source; actually there are none, since when
            // predeclaring, you use a texture file which includes all the
            // information required.
        }

        
    }
    //--------------------------------------------------------------------------
	void Texture::loadRawData( DataStreamPtr& stream, 
		ushort uWidth, ushort uHeight, PixelFormat eFormat)
	{
		Image img;
		img.loadRawData(stream, uWidth, uHeight, eFormat);
		loadImage(img);
	}
    //--------------------------------------------------------------------------
    void Texture::setFormat(PixelFormat pf)
    {
        mFormat = pf;
        // This should probably change with new texture access methods, but
        // no changes made for now
        mSrcBpp = PixelUtil::getNumElemBytes(mFormat);
        mHasAlpha = PixelUtil::getFlags(mFormat) & PFF_HASALPHA;
    }
    //--------------------------------------------------------------------------
	size_t Texture::calculateSize(void) const
	{
		// TODO - how do we calculate real DDS texture size?
		return mWidth * mHeight * mDepth * mFinalBpp;
	}
	//--------------------------------------------------------------------------
	size_t Texture::getNumFaces(void) const
	{
		return getTextureType() == TEX_TYPE_CUBE_MAP ? 6 : 1;
	}
	//--------------------------------------------------------------------------
    void Texture::_loadImages( const std::vector<const Image*>& images )
    {
		if(images.size() < 1)
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot load empty vector of images",
			 "Texture::loadImages");
        
        if( mIsLoaded )
        {
			LogManager::getSingleton().logMessage( 
				LML_NORMAL, "Texture: "+mName+": Unloading Image");
            unload();
        }

		// Set desired texture size and properties from images[0]
		mSrcWidth = mWidth = images[0]->getWidth();
		mSrcHeight = mHeight = images[0]->getHeight();
		mDepth = images[0]->getDepth();
		mFormat = images[0]->getFormat();
		mSrcBpp = PixelUtil::getNumElemBits(mFormat);
		mHasAlpha = PixelUtil::hasAlpha(mFormat);
		
		// The custom mipmaps in the image have priority over everything
		bool doCustomMipmaps = images[0]->getNumMipmaps()>0;
		
		if(doCustomMipmaps) {
			mNumMipmaps = images[0]->getNumMipmaps();
			// Disable flag for auto mip generation
			mUsage &= ~TU_AUTOMIPMAP;
		}
		
        // Create the texture
        createInternalResources();
		// Check if we're loading one image with multiple faces
		// or a vector of images representing the faces
		size_t faces;
		bool multiImage; // Load from multiple images?
		if(images.size() > 1)
		{
			faces = images.size();
			multiImage = true;
		}
		else
		{
			faces = images[0]->getNumFaces();
			multiImage = false;
		}
		
		// Check wether number of faces in images exceeds number of faces
		// in this texture. If so, clamp it.
		if(faces > getNumFaces())
			faces = getNumFaces();
		
		// Say what we're doing
		StringUtil::StrStreamType str;
		str << "Texture: " << mName << ": Loading " << faces << " faces"
			<< "(" << PixelUtil::getFormatName(images[0]->getFormat()) << "," <<
			images[0]->getWidth() << "x" << images[0]->getHeight() << "x" << images[0]->getDepth() <<
			") with "
			<< mNumMipmaps;
		if(mUsage & TU_AUTOMIPMAP)
			str << " generated mipmaps";
		else
			str << " custom mipmaps";
 		if(multiImage)
			str << " from multiple Images.";
		else
			str << " from Image.";
		// Scoped
		{
			// Print data about first destination surface
			HardwarePixelBufferSharedPtr buf = getBuffer(0, 0); 
			str << " Internal format is " << PixelUtil::getFormatName(buf->getFormat()) << 
			"," << buf->getWidth() << "x" << buf->getHeight() << "x" << buf->getDepth() << ".";
		}
		LogManager::getSingleton().logMessage( 
				LML_NORMAL, str.str());
		
		// Main loading loop
        for(size_t i = 0; i < faces; i++)
        {
			PixelBox src;
			// TODO manual mips
			if(multiImage)
			{
				// Load from multiple images
				src = images[i]->getPixelBox();
			}
			else
			{
				// Load from faces of images[0]
				src = images[0]->getPixelBox(i, 0);
			}

			if(mGamma != 1.0f) {
				// Apply gamma correction
				// Do not overwrite original image but do gamma correction in temporary buffer
				MemoryDataStreamPtr buf; // for scoped deletion of conversion buffer
				buf.bind(new MemoryDataStream(
					PixelUtil::getMemorySize(
						src.getWidth(), src.getHeight(), src.getDepth(), src.format)));
				
				PixelBox corrected = PixelBox(src.getWidth(), src.getHeight(), src.getDepth(), src.format, buf->getPtr());
				PixelUtil::bulkPixelConversion(src, corrected);
				
				Image::applyGamma(static_cast<uint8*>(corrected.data), mGamma, corrected.getConsecutiveSize(), 
					PixelUtil::getNumElemBits(src.format));

				// Destination: entire texture. blitFromMemory does the scaling to
				// a power of two for us when needed
				getBuffer(i, 0)->blitFromMemory(corrected);
			}
			else 
			{
				// Destination: entire texture. blitFromMemory does the scaling to
				// a power of two for us when needed
            	getBuffer(i, 0)->blitFromMemory(src);
			}
			
        }
        // Update size (the final size, not including temp space)
        mSize = getNumFaces() * PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);

        mIsLoaded = true;
    }

}

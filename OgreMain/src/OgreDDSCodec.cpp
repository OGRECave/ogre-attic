/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 The OGRE Team
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

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreDDSCodec.h"
#include "OgreImage.h"
#include "OgreException.h"

#include "OgreLogManager.h"
#include "OgreStringConverter.h"


namespace Ogre {
	// Internal DDS structure definitions
#define FOURCC(c0, c1, c2, c3) (c0 | (c1 << 8) | (c2 << 16) | (c3 << 24))
	
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#pragma pack (push, 1)
#else
#pragma pack (1)
#endif

	// Nested structure
	struct DDSPixelFormat
	{
		uint32 size;
		uint32 flags;
		uint32 fourCC;
		uint32 rgbBits;
		uint32 redMask;
		uint32 greenMask;
		uint32 blueMask;
		uint32 alphaMask;
	};
	
	// Nested structure
	struct DDSCaps
	{
		uint32 caps1;
		uint32 caps2;
		uint32 reserved[2];
	};
	// Main header, note preceded by 'DDS '
	struct DDSHeader
	{
		uint32 size;		
		uint32 flags;
		uint32 height;
		uint32 width;
		uint32 sizeOrPitch;
		uint32 depth;
		uint32 mipMapCount;
		uint32 reserved1[11];
		DDSPixelFormat pixelFormat;
		DDSCaps caps;
		uint32 reserved2;
	};
	
#if OGRE_COMPILER == OGRE_COMPILER_MSVC
#pragma pack (pop)
#else
#pragma pack ()
#endif

	const uint32 DDS_PIXELFORMAT_SIZE = 8 * sizeof(uint32);
	const uint32 DDS_CAPS_SIZE = 4 * sizeof(uint32);
	const uint32 DDS_HEADER_SIZE = 19 * sizeof(uint32) + DDS_PIXELFORMAT_SIZE + DDS_CAPS_SIZE;

	const uint32 DDSD_CAPS = 0x00000001;
	const uint32 DDSD_HEIGHT = 0x00000002;
	const uint32 DDSD_WIDTH = 0x00000004;
	const uint32 DDSD_PITCH = 0x00000008;
	const uint32 DDSD_PIXELFORMAT = 0x00001000;
	const uint32 DDSD_MIPMAPCOUNT = 0x00020000;
	const uint32 DDSD_LINEARSIZE = 0x00080000;
	const uint32 DDSD_DEPTH = 0x00800000;
	const uint32 DDPF_ALPHAPIXELS = 0x00000001;
	const uint32 DDPF_FOURCC = 0x00000004;
	const uint32 DDPF_RGB = 0x00000040;
	const uint32 DDSCAPS_COMPLEX = 0x00000008;
	const uint32 DDSCAPS_TEXTURE = 0x00001000;
	const uint32 DDSCAPS_MIPMAP = 0x00400000;
	const uint32 DDSCAPS2_CUBEMAP = 0x00000200;
	const uint32 DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400;
	const uint32 DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800;
	const uint32 DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000;
	const uint32 DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000;
	const uint32 DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000;
	const uint32 DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000;
	const uint32 DDSCAPS2_VOLUME = 0x00200000;

	//---------------------------------------------------------------------
	DDSCodec* DDSCodec::msInstance = 0;
	//---------------------------------------------------------------------
	void DDSCodec::startup(void)
	{
		if (!msInstance)
		{

			LogManager::getSingleton().logMessage(
				LML_NORMAL,
				"DDS codec registering");

			msInstance = new DDSCodec();
			Codec::registerCodec(msInstance);
		}

	}
	//---------------------------------------------------------------------
	void DDSCodec::shutdown(void)
	{
		if(msInstance)
		{
			Codec::unRegisterCodec(msInstance);
			delete msInstance;
			msInstance = 0;
		}

	}
	//---------------------------------------------------------------------
    DDSCodec::DDSCodec():
        mType("dds")
    { 
    }
    //---------------------------------------------------------------------
    DataStreamPtr DDSCodec::code(MemoryDataStreamPtr& input, Codec::CodecDataPtr& pData) const
    {        
		OGRE_EXCEPT(Exception::UNIMPLEMENTED_FEATURE,
			"DDS encoding not supported",
			"DDSCodec::code" ) ;
    }
    //---------------------------------------------------------------------
    void DDSCodec::codeToFile(MemoryDataStreamPtr& input, 
        const String& outFileName, Codec::CodecDataPtr& pData) const
    {
		OGRE_EXCEPT(Exception::UNIMPLEMENTED_FEATURE,
			"DDS encoding not supported",
			"DDSCodec::codeToFile" ) ;
    }
	//---------------------------------------------------------------------
	PixelFormat DDSCodec::convertDXTFormat(uint32 fourcc, bool decompress) const
	{
		// convert dxt pixel format
		if (!decompress)
		{
			switch(fourcc)
			{
			case FOURCC('D','X','T','1'):
				return PF_DXT1;
			case FOURCC('D','X','T','2'):
				return PF_DXT2;
			case FOURCC('D','X','T','3'):
				return PF_DXT3;
			case FOURCC('D','X','T','4'):
				return PF_DXT4;
			case FOURCC('D','X','T','5'):
				return PF_DXT5;
			// We could support 3Dc here, but only ATI cards support it, not nVidia
			default:
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
					"Invalid FourCC format found in DDS file", 
					"DDSCodec::decode");
			};
		}
		else
		{
			switch(fourcc)
			{
			case FOURCC('D','X','T','1'):
				return PF_DXT1;
			case FOURCC('D','X','T','2'):
				return PF_DXT2;
			case FOURCC('D','X','T','3'):
				return PF_DXT3;
			case FOURCC('D','X','T','4'):
				return PF_DXT4;
			case FOURCC('D','X','T','5'):
				return PF_DXT5;
				// We could support 3Dc here, but only ATI cards support it, not nVidia
			default:
				OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, 
					"Invalid FourCC format found in DDS file", 
					"DDSCodec::decode");
			};

		}

	}
	//---------------------------------------------------------------------
	PixelFormat DDSCodec::convertPixelFormat(uint32 rgbBits, uint32 rMask, 
		uint32 gMask, uint32 bMask, uint32 aMask) const
	{
		// NB on big-endian machines the masks will have been flipped, so flip back
		flipEndian(&rMask, sizeof(uint32));
		flipEndian(&gMask, sizeof(uint32));
		flipEndian(&bMask, sizeof(uint32));
		flipEndian(&aMask, sizeof(uint32));

		// General search through pixel formats
		for (int i = PF_UNKNOWN + 1; i < PF_COUNT; ++i)
		{
			PixelFormat pf = static_cast<PixelFormat>(i);
			if (PixelUtil::getNumElemBits(pf) == rgbBits)
			{
				uint32 testMasks[4];
				PixelUtil::getBitMasks(pf, testMasks);
				int testBits[4];
				PixelUtil::getBitDepths(pf, testBits);
				if (testMasks[0] == rMask && testMasks[1] == gMask &&
					testMasks[2] == bMask && 
					// for alpha, deal with 'X8' formats by checking bit counts
					(testMasks[3] == aMask || (aMask == 0 && testBits[3] == 0)))
				{
					return pf;
				}
			}

		}

		OGRE_EXCEPT(Exception::ERR_ITEM_NOT_FOUND, "Cannot determine pixel format",
			"DDSCodec::convertPixelFormat");

	}
    //---------------------------------------------------------------------
    Codec::DecodeResult DDSCodec::decode(DataStreamPtr& stream) const
    {
		// Read 4 character code
		uint32 fileType;
		stream->read(&fileType, sizeof(uint32));
		flipEndian(&fileType, sizeof(uint32), 1);
		
		if (FOURCC('D', 'D', 'S', ' ') != fileType)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"This is not a DDS file!", "DDSCodec::decode");
		}
		
		// Read header in full
		DDSHeader header;
		stream->read(&header, sizeof(DDSHeader));

		// Endian flip if required, all 32-bit values
		flipEndian(&header, 4, sizeof(DDSHeader) / 4);

		// Check some sizes
		if (header.size != DDS_HEADER_SIZE)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"DDS header size mismatch!", "DDSCodec::decode");
		}
		if (header.pixelFormat.size != DDS_PIXELFORMAT_SIZE)
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
				"DDS header size mismatch!", "DDSCodec::decode");
		}

		ImageData* imgData = new ImageData();
		MemoryDataStreamPtr output;

		imgData->depth = 1; // (deal with volume later)
		imgData->width = header.width;
		imgData->height = header.height;
		size_t numFaces = 1; // assume one face until we know otherwise

		if (header.caps.caps1 & DDSCAPS_MIPMAP)
		{
	        imgData->num_mipmaps = header.mipMapCount - 1;
		}
		else
		{
			imgData->num_mipmaps = 0;
		}
		imgData->flags = 0;

		bool decompressDXT = false;
		// Figure out basic image type
		if (header.flags & DDSD_LINEARSIZE)
		{
			// compressed data
			if (Root::getSingleton().getRenderSystem()->getCapabilities()
				->hasCapability(RSC_TEXTURE_COMPRESSION_DXT))
			{
				// Keep DXT data compressed
				imgData->flags |= IF_COMPRESSED;
			}
			else
			{
				// We'll need to decompress
				decompressDXT = true;
			}
		}
		if (header.caps.caps2 & DDSCAPS2_CUBEMAP)
		{
			imgData->flags |= IF_CUBEMAP;
			numFaces = 6;
		}
		else if (header.caps.caps2 & DDSCAPS2_VOLUME)
		{
			imgData->flags |= IF_3D_TEXTURE;
			imgData->depth = header.depth;
		}
		// Pixel format
		if (header.pixelFormat.flags & DDPF_FOURCC)
		{
			imgData->format = convertDXTFormat(header.pixelFormat.fourCC, decompressDXT);
		}
		else //if (header.pixelFormat.flags & DDPF_RGB)
		{
			// Don't test against DDPF_RGB since greyscale DDS doesn't set this
			// just derive any other kind of format
			imgData->format = convertPixelFormat(header.pixelFormat.rgbBits, 
				header.pixelFormat.redMask, header.pixelFormat.greenMask, 
				header.pixelFormat.blueMask, 
				header.pixelFormat.flags & DDPF_ALPHAPIXELS ? 
					header.pixelFormat.alphaMask : 0);
		}

		// Calculate total size from number of mipmaps, faces and size
		imgData->size = Image::calculateSize(imgData->num_mipmaps, numFaces, 
			imgData->width, imgData->height, imgData->depth, imgData->format);

		// Bind output buffer
		output.bind(new MemoryDataStream(imgData->size));

		
		// Now deal with the data
		void* destPtr = output->getPtr();
		size_t width = imgData->width;
		size_t height = imgData->height;
		size_t depth = imgData->height;

		for(size_t mip = 0; mip <= imgData->num_mipmaps; ++mip)
		{   
			for(size_t i = 0; i < numFaces; ++i)
			{
				if (imgData->flags & IF_COMPRESSED)
				{
					// Compressed data
					if (decompressDXT)
					{
						// todo

					}
					else
					{
						// load directly
						size_t dxtSize = header.sizeOrPitch / 
							std::max((size_t)1, mip * 4);
						stream->read(destPtr, dxtSize);
						destPtr = static_cast<void*>(static_cast<uchar*>(destPtr) + dxtSize);
					}

				}
				else
				{
					// Final data - trim incoming pitch
					size_t dstPitch = imgData->width * PixelUtil::getNumElemBytes(imgData->format);
					size_t srcPitch;
					if (header.flags & DDSD_PITCH)
					{
						srcPitch = header.sizeOrPitch;
					}
					else
					{
						// assume same as final pitch
						srcPitch = dstPitch;
					}
					assert (dstPitch <= srcPitch);
					size_t srcAdvance = srcPitch - dstPitch;

					for (size_t z = 0; z < imgData->depth; ++z)
					{
						for (size_t y = 0; y < imgData->height; ++y)
						{
							stream->read(destPtr, dstPitch);
							if (srcAdvance > 0)
								stream->skip(srcAdvance);

							destPtr = static_cast<void*>(static_cast<uchar*>(destPtr) + dstPitch);
						}
					}

				}

				
			}

			/// Next mip
			if(width!=1) width /= 2;
			if(height!=1) height /= 2;
			if(depth!=1) depth /= 2;
		}

		DecodeResult ret;
		ret.first = output;
		ret.second = CodecDataPtr(imgData);
		return ret;
		


    }
    //---------------------------------------------------------------------    
    String DDSCodec::getType() const 
    {
        return mType;
    }
    //---------------------------------------------------------------------    
    void DDSCodec::flipEndian(void * pData, size_t size, size_t count) const
    {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
		for(unsigned int index = 0; index < count; index++)
        {
            flipEndian((void *)((long)pData + (index * size)), size);
        }
#endif
    }
    //---------------------------------------------------------------------    
    void DDSCodec::flipEndian(void * pData, size_t size) const
    {
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
        char swapByte;
        for(unsigned int byteIndex = 0; byteIndex < size/2; byteIndex++)
        {
            swapByte = *(char *)((long)pData + byteIndex);
            *(char *)((long)pData + byteIndex) = *(char *)((long)pData + size - byteIndex - 1);
            *(char *)((long)pData + size - byteIndex - 1) = swapByte;
        }
#endif
    }
	
}


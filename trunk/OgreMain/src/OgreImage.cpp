/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreStableHeaders.h"
#include "OgreImage.h"

#include "OgreArchiveManager.h"
#include "OgreException.h"
#include "OgreImageCodec.h"
#include "OgreColourValue.h"

// Dependency on IL/ILU for resize
#include <IL/il.h>
#include <IL/ilu.h>
namespace Ogre {
    ImageCodec::~ImageCodec() {
    }

    //-----------------------------------------------------------------------------
    Image::Image()
        : m_uDepth(0),
          m_uSize(0),
          m_uNumMipmaps(0),
          m_uFlags(0),
          m_pBuffer( NULL ),
		  m_bAutoDelete( true )
    {
    }

    //-----------------------------------------------------------------------------
    Image::Image( const Image &img )
    {
        // call assignment operator
        *this = img;
    }

    //-----------------------------------------------------------------------------
    Image::~Image()
    {
		//Only delete if this was not a dynamic image (meaning app holds & destroys buffer)
        if( m_pBuffer && m_bAutoDelete )
        {
            delete[] m_pBuffer;
            m_pBuffer = NULL;
        }
    }

    //-----------------------------------------------------------------------------
    Image & Image::operator = ( const Image &img )
    {
        m_uWidth = img.m_uWidth;
        m_uHeight = img.m_uHeight;
        m_uDepth = img.m_uDepth;
        m_eFormat = img.m_eFormat;
        m_uSize = img.m_uSize;
        m_uFlags = img.m_uFlags;
        m_ucPixelSize = img.m_ucPixelSize;
        m_uNumMipmaps = img.m_uNumMipmaps;
        m_bAutoDelete = img.m_bAutoDelete;

		//Only create/copy when previous data was not dynamic data
        if( m_bAutoDelete )
		{
			m_pBuffer = new uchar[ m_uSize ];
			memcpy( m_pBuffer, img.m_pBuffer, m_uSize );
		}
		else
		{
			m_pBuffer = img.m_pBuffer;
		}

        return *this;
    }

    //-----------------------------------------------------------------------------
    Image & Image::flipAroundY()
    {
        OgreGuard( "Image::flipAroundY" );

        if( !m_pBuffer )
        {
            Except( 
                Exception::ERR_INTERNAL_ERROR,
                "Can not flip an unitialized texture",
                "Image::flipAroundY" );
        }
        
		uchar	*pTempBuffer1 = NULL;
		ushort	*pTempBuffer2 = NULL;
		uchar	*pTempBuffer3 = NULL;
		uint	*pTempBuffer4 = NULL;

		uchar	*src1 = m_pBuffer, *dst1 = NULL;
		ushort	*src2 = (ushort *)m_pBuffer, *dst2 = NULL;
		uchar	*src3 = m_pBuffer, *dst3 = NULL;
		uint	*src4 = (uint *)m_pBuffer, *dst4 = NULL;

		ushort y;
		switch (m_ucPixelSize)
		{
		case 1:
			pTempBuffer1 = new uchar[m_uWidth * m_uHeight];
			for (y = 0; y < m_uHeight; y++)
			{
				dst1 = (pTempBuffer1 + ((y * m_uWidth) + m_uWidth - 1));
				for (ushort x = 0; x < m_uWidth; x++)
					memcpy(dst1--, src1++, sizeof(uchar));
			}

			memcpy(m_pBuffer, pTempBuffer1, m_uWidth * m_uHeight * sizeof(uchar));
			delete [] pTempBuffer1;
			break;

		case 2:
			pTempBuffer2 = new ushort[m_uWidth * m_uHeight];
			for (y = 0; y < m_uHeight; y++)
			{
				dst2 = (pTempBuffer2 + ((y * m_uWidth) + m_uWidth - 1));
				for (ushort x = 0; x < m_uWidth; x++)
					memcpy(dst2--, src2++, sizeof(ushort));
			}

			memcpy(m_pBuffer, pTempBuffer2, m_uWidth * m_uHeight * sizeof(ushort));
			delete [] pTempBuffer2;
			break;

		case 3:
			pTempBuffer3 = new uchar[m_uWidth * m_uHeight * 3];
			for (y = 0; y < m_uHeight; y++)
			{
				uint offset = ((y * m_uWidth) + (m_uWidth - 1)) * 3;
				dst3 = pTempBuffer3;
				dst3 += offset;
				for (ushort x = 0; x < m_uWidth; x++)
				{
					memcpy(dst3, src3, sizeof(uchar) * 3);
					dst3 -= 3; src3 += 3;
				}
			}

			memcpy(m_pBuffer, pTempBuffer3, m_uWidth * m_uHeight * sizeof(uchar) * 3);
			delete [] pTempBuffer3;
			break;

		case 4:
			pTempBuffer4 = new uint[m_uWidth * m_uHeight];
			for (y = 0; y < m_uHeight; y++)
			{
				dst4 = (pTempBuffer4 + ((y * m_uWidth) + m_uWidth - 1));
				for (ushort x = 0; x < m_uWidth; x++)
					memcpy(dst4--, src4++, sizeof(uint));
			}

			memcpy(m_pBuffer, pTempBuffer4, m_uWidth * m_uHeight * sizeof(uint));
			delete [] pTempBuffer4;
			break;

		default:
            Except( 
                Exception::ERR_INTERNAL_ERROR,
                "Unknown pixel depth",
                "Image::flipAroundY" );
			break;
		}

		OgreUnguardRet( *this );
    }

    //-----------------------------------------------------------------------------
    Image & Image::flipAroundX()
    {
        OgreGuard( "Image::flipAroundX" );

        if( !m_pBuffer )
        {
            Except( 
                Exception::ERR_INTERNAL_ERROR,
                "Can not flip an unitialized texture",
                "Image::flipAroundX" );
        }
        
        size_t rowSpan = m_uWidth * m_ucPixelSize;
        
        uchar *pTempBuffer = new uchar[ rowSpan * m_uHeight ];
        uchar *ptr1 = m_pBuffer, *ptr2 = pTempBuffer + ( ( m_uHeight - 1 ) * rowSpan );

        for( ushort i = 0; i < m_uHeight; i++ )
        {
            memcpy( ptr2, ptr1, rowSpan );
            ptr1 += rowSpan; ptr2 -= rowSpan;
        }

        memcpy( m_pBuffer, pTempBuffer, rowSpan * m_uHeight);

        delete [] pTempBuffer;

        OgreUnguardRet( *this );
    }

	//-----------------------------------------------------------------------------
	Image& Image::loadDynamicImage( uchar* pData, ushort uWidth, ushort uHeight, 
							 PixelFormat eFormat )
	{
        OgreGuard( "Image::loadDynamicImage" );

        m_uWidth = uWidth;
        m_uHeight = uHeight;
        m_eFormat = eFormat;
        m_ucPixelSize = PixelUtil::getNumElemBytes( m_eFormat );
        m_uSize = m_uWidth * m_uHeight * m_ucPixelSize;

        m_pBuffer = pData;
		
		m_bAutoDelete = false;
        
		OgreUnguardRet( *this );
	}

    //-----------------------------------------------------------------------------
    Image & Image::loadRawData(
        DataStreamPtr& stream,
        ushort uWidth, ushort uHeight,
        PixelFormat eFormat )
    {
        OgreGuard( "Image::loadRawData" );

        m_uWidth = uWidth;
        m_uHeight = uHeight;
        m_eFormat = eFormat;
        m_ucPixelSize = PixelUtil::getNumElemBytes( m_eFormat );
        m_uSize = m_uWidth * m_uHeight * m_ucPixelSize;

        if (m_uSize != stream->size())
        {
            Except(Exception::ERR_INVALIDPARAMS, 
                "Stream size does not match calculated image size! ", 
                "Image::loadRawData");
        }
        m_pBuffer = new uchar[ m_uSize ];
        stream->read(m_pBuffer, m_uSize);

        OgreUnguardRet( *this );
    }

    //-----------------------------------------------------------------------------
    Image & Image::load(const String& strFileName, const String& group)
    {
        OgreGuard( "Image::load" );

        if( m_pBuffer )
        {
            delete[] m_pBuffer;
            m_pBuffer = NULL;
        }

        String strExt;

        size_t pos = strFileName.find_last_of(".");
	    if( pos == String::npos )
            Except(
		    Exception::ERR_INVALIDPARAMS, 
		    "Unable to load image file '" + strFileName + "' - invalid extension.",
            "Image::load" );

        while( pos != strFileName.length() - 1 )
            strExt += strFileName[++pos];

        Codec * pCodec = Codec::getCodec(strExt);
        if( !pCodec )
            Except(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to load image file '" + strFileName + "' - invalid extension.",
            "Image::load" );

        DataStreamPtr encoded = 
            ResourceGroupManager::getSingleton().openResource(strFileName, group);

		Codec::DecodeResult res = pCodec->decode(encoded);

		ImageCodec::ImageData* pData = 
			static_cast<ImageCodec::ImageData*>(res.second.getPointer());

        // Get the format and compute the pixel size
        m_uWidth = pData->width;
        m_uHeight = pData->height;
        m_uDepth = pData->depth;
        m_uSize = pData->size;
        m_eFormat = pData->format;
        m_uNumMipmaps = pData->num_mipmaps;
        m_ucPixelSize = PixelUtil::getNumElemBytes( m_eFormat );
        m_uFlags = pData->flags;

		// re-use the decoded buffer
        m_pBuffer = res.first->getPtr();
		// ensure we don't delete when stream is closed
		res.first->setFreeOnClose(false);
        
        OgreUnguardRet( *this );
    }
    //-----------------------------------------------------------------------------
    void Image::save(const String& filename)
    {
        if( !m_pBuffer )
        {
            Except(Exception::ERR_INVALIDPARAMS, "No image data loaded", 
                "Image::save");
        }

        String strExt;
        size_t pos = filename.find_last_of(".");
        if( pos == String::npos )
            Except(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to save image file '" + filename + "' - invalid extension.",
            "Image::save" );

        while( pos != filename.length() - 1 )
            strExt += filename[++pos];

        Codec * pCodec = Codec::getCodec(strExt);
        if( !pCodec )
            Except(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to save image file '" + filename + "' - invalid extension.",
            "Image::save" );

        ImageCodec::ImageData* imgData = new ImageCodec::ImageData();
        imgData->format = m_eFormat;
        imgData->height = m_uHeight;
        imgData->width = m_uWidth;
		// Wrap in CodecDataPtr, this will delete
		Codec::CodecDataPtr codeDataPtr(imgData);
		// Wrap memory, be sure not to delete when stream destroyed
        MemoryDataStreamPtr wrapper(new MemoryDataStream(m_pBuffer, m_uSize, false));
		
        pCodec->codeToFile(wrapper, filename, codeDataPtr);
    }
    //-----------------------------------------------------------------------------
    Image & Image::load(DataStreamPtr& stream, const String& type )
    {
        OgreGuard( "Image::load" );

        String strType = type;

        Codec * pCodec = Codec::getCodec(strType);
        if( !pCodec )
            Except(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to load image - invalid extension.",
            "Image::load" );

		Codec::DecodeResult res = pCodec->decode(stream);

		ImageCodec::ImageData* pData = 
			static_cast<ImageCodec::ImageData*>(res.second.getPointer());

        m_uWidth = pData->width;
        m_uHeight = pData->height;
        m_uDepth = pData->depth;
        m_uSize = pData->size;
        m_uNumMipmaps = pData->num_mipmaps;
        m_uFlags = pData->flags;
        
        // Get the format and compute the pixel size
        m_eFormat = pData->format;
        m_ucPixelSize = PixelUtil::getNumElemBytes( m_eFormat );
		// Just use internal buffer of returned memory stream
        m_pBuffer = res.first->getPtr();
		// Make sure stream does not delete
		res.first->setFreeOnClose(false);

        OgreUnguardRet( *this );
    }

    //-----------------------------------------------------------------------------
    uchar* Image::getData()
    {
        return m_pBuffer;
    }

    //-----------------------------------------------------------------------------
    const uchar* Image::getData() const
    {
        assert( m_pBuffer );
        return m_pBuffer;
    }

    //-----------------------------------------------------------------------------
    size_t Image::getSize() const
    {
       return m_uSize;
    }

    //-----------------------------------------------------------------------------
    unsigned short Image::getNumMipmaps() const
    {
       return m_uNumMipmaps;
    }

    //-----------------------------------------------------------------------------
    bool Image::hasFlag(const ImageFlags imgFlag) const
    {
        if(m_uFlags & imgFlag)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    //-----------------------------------------------------------------------------
    ushort Image::getDepth() const
    {
        return m_uDepth;
    }
    //-----------------------------------------------------------------------------
    ushort Image::getWidth() const
    {
        return m_uWidth;
    }

    //-----------------------------------------------------------------------------
    ushort Image::getHeight() const
    {
        return m_uHeight;
    }

    //-----------------------------------------------------------------------------
    ushort Image::getRowSpan() const
    {
        return m_uWidth * m_ucPixelSize;
    }

    //-----------------------------------------------------------------------------
    PixelFormat Image::getFormat() const
    {
        return m_eFormat;
    }

    //-----------------------------------------------------------------------------
    uchar Image::getBPP() const
    {
        return m_ucPixelSize * 8;
    }

    //-----------------------------------------------------------------------------
    bool Image::getHasAlpha(void) const
    {
        return PixelUtil::getFlags(m_eFormat) & PFF_HASALPHA;
    }
    //-----------------------------------------------------------------------------
    void Image::applyGamma( unsigned char *buffer, Real gamma, size_t size, uchar bpp )
    {
        if( gamma == 1.0f )
            return;

        //NB only 24/32-bit supported
        if( bpp != 24 && bpp != 32 ) return;

        uint stride = bpp >> 3;

        for( size_t i = 0, j = size / stride; i < j; i++, buffer += stride )
        {
            float r, g, b;

            r = (float)buffer[0];
            g = (float)buffer[1];
            b = (float)buffer[2];

            r = r * gamma;
            g = g * gamma;
            b = b * gamma;

            float scale = 1.0f, tmp;

            if( r > 255.0f && (tmp=(255.0f/r)) < scale )
                scale = tmp;
            if( g > 255.0f && (tmp=(255.0f/g)) < scale )
                scale = tmp;
            if( b > 255.0f && (tmp=(255.0f/b)) < scale )
                scale = tmp;

            r *= scale; g *= scale; b *= scale;

            buffer[0] = (uchar)r;
            buffer[1] = (uchar)g;
            buffer[2] = (uchar)b;
        }
    }
    //-----------------------------------------------------------------------------
	// Local declaration of DevIL functions to prevent DevIL dependencies on header users
	ILenum getILFilter(Image::Filter filter)
	{
		switch (filter)
		{
			case Image::FILTER_NEAREST:
				return ILU_NEAREST;
			case Image::FILTER_LINEAR:
				return ILU_LINEAR;
			case Image::FILTER_BILINEAR:
				return ILU_BILINEAR;
			case Image::FILTER_BOX:
				return ILU_SCALE_BOX;
			case Image::FILTER_TRIANGLE:
				return ILU_SCALE_TRIANGLE;
			case Image::FILTER_BICUBIC:
				return ILU_SCALE_BSPLINE;
		};
        // keep compiler happy
        return ILU_NEAREST;
	}
    //-----------------------------------------------------------------------------
	void Image::resize(ushort width, ushort height, Filter filter)
	{
        ILuint ImageName;

        ilGenImages( 1, &ImageName );
        ilBindImage( ImageName );

        std::pair< int, int > fmt_bpp = ILUtil::OgreFormat2ilFormat( m_eFormat );
        ilTexImage( 
            m_uWidth, m_uHeight, 1, fmt_bpp.second, fmt_bpp.first, IL_UNSIGNED_BYTE, 
            static_cast< void * >( m_pBuffer ) );

		// set filter
		iluImageParameter(ILU_FILTER, getILFilter(filter));
		
		iluScale(width, height, 1);
		
		// delete existing buffer and recreate with new settings
		delete [] m_pBuffer;
		m_uWidth = width;
		m_uHeight = height;
		size_t dataSize = width * height * m_ucPixelSize;
		m_pBuffer = new uchar[dataSize];
		memcpy(m_pBuffer, ilGetData(), dataSize);

        ilDeleteImages(1, &ImageName);

		// return to default filter
		iluImageParameter(ILU_FILTER, ILU_NEAREST);
	}
    
    //-----------------------------------------------------------------------------    
    void Image::getColourAt(ColourValue *out, int x, int y, int z) 
    {
        PixelUtil::unpackColour(out, m_eFormat, &m_pBuffer[m_ucPixelSize * (z * m_uWidth * m_uHeight + m_uWidth * y + x)]);
    }

    PixelBox Image::getPixelBox(int cubeface, int mipmap) 
    {
        // TODO - do something with cubeface & mipmap
        PixelBox src;
        src.width = getWidth();
        src.height = getHeight();
        src.depth = 1;
        src.format = getFormat();
        src.data = getData();
        src.setConsecutive();
        return src;
    }

}

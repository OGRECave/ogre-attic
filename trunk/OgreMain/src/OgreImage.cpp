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
#include "OgreImage.h"

#include "OgreArchiveManager.h"
#include "OgreException.h"
#include "OgreImageCodec.h"
#include "OgreSDDataChunk.h"

namespace Ogre {

    //-----------------------------------------------------------------------------
    Image::Image()
        : m_pBuffer( NULL )
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
        if( m_pBuffer )
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
        m_eFormat = img.m_eFormat;
        m_ucPixelSize = img.m_ucPixelSize;

        m_pBuffer = new uchar[ m_uWidth * m_uHeight * m_ucPixelSize ];
        memcpy( m_pBuffer, img.m_pBuffer, m_uWidth * m_uHeight * m_ucPixelSize );

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
    Image & Image::loadRawData(
        const DataChunk &pData,
        ushort uWidth, ushort uHeight,
        PixelFormat eFormat )
    {
        OgreGuard( "Image::loadRawData" );

        m_uWidth = uWidth;
        m_uHeight = uHeight;
        m_eFormat = eFormat;
        m_ucPixelSize = PF2PS( m_eFormat );

        m_pBuffer = new uchar[ uWidth * uHeight * m_ucPixelSize ];
        memcpy( m_pBuffer, pData.getPtr(), uWidth * uHeight * m_ucPixelSize );

        OgreUnguardRet( *this );
    }

    //-----------------------------------------------------------------------------
    Image & Image::load( const String& strFileName )
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

        SDDataChunk encoded;
        DataChunk decoded;

        if( !ArchiveManager::getSingleton()._findResourceData( 
            strFileName, 
            encoded ) )
        {
            Except(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to find image file '" + strFileName + "'.",
            "Image::load" );
        }

        ImageCodec::ImageData * pData = static_cast< ImageCodec::ImageData * > (
            pCodec->decode( encoded, &decoded ) );

        // Get the format and compute the pixel size
        m_uWidth = pData->width;
        m_uHeight = pData->height;
        m_eFormat = pData->format;
        m_ucPixelSize = PF2PS( m_eFormat );

        delete pData;

        m_pBuffer = decoded.getPtr();
        
        OgreUnguardRet( *this );
    }

    //-----------------------------------------------------------------------------
    Image & Image::load( const DataChunk& chunk, const String& type )
    {
        OgreGuard( "Image::load" );

        String strType = type;

        Codec * pCodec = Codec::getCodec(strType);
        if( !pCodec )
            Except(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to load image - invalid extension.",
            "Image::load" );

        DataChunk decoded;

        ImageCodec::ImageData * pData = static_cast< ImageCodec::ImageData * >(
            pCodec->decode( chunk, &decoded ) );

        m_uWidth = pData->width;
        m_uHeight = pData->height;
        
        // Get the format and compute the pixel size
        m_eFormat = pData->format;
        m_ucPixelSize = PF2PS( m_eFormat );

        delete pData;

        m_pBuffer = decoded.getPtr();

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
        return m_uWidth * m_uHeight * m_ucPixelSize;
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
    bool Image::getHasAlpha() const
    {
        switch( m_eFormat )
		{
		case PF_A8:
		case PF_A4L4:
		case PF_L4A4:
		case PF_A4R4G4B4:
		case PF_B4G4R4A4:
		case PF_A8R8G8B8:
		case PF_B8G8R8A8:
		case PF_A2R10G10B10:
		case PF_B10G10R10A2:
			return true;

		case PF_UNKNOWN:
		case PF_L8:
		case PF_R5G6B5:
		case PF_B5G6R5:
		case PF_R8G8B8:
		case PF_B8R8G8:
		default:
			return false;
		}
    }

    void Image::applyGamma( unsigned char *buffer, Real gamma, uint size, uchar bpp )
    {
        if( gamma == 1.0f )
            return;

        //NB only 24/32-bit supported
        if( bpp != 24 && bpp != 32 ) return;

        uint stride = bpp >> 3;

        for( uint i = 0, j = size / stride; i < j; i++, buffer += stride )
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
}

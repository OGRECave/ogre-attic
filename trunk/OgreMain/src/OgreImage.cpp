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
        OgreGuard( "Image::flipAroundX" );
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

        Codec * pCodec = Codec::getColl()[ strExt.toLowerCase() ];
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
        m_uWidth = pData->ulWidth;
        m_uHeight = pData->ulHeight;
        m_eFormat = pData->eFormat;
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

        Codec * pCodec = Codec::getColl()[ strType.toLowerCase() ];
        if( !pCodec )
            Except(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to load image - invalid extension.",
            "Image::load" );

        DataChunk decoded;

        ImageCodec::ImageData * pData = static_cast< ImageCodec::ImageData * >(
            pCodec->decode( chunk, &decoded ) );

        m_uWidth = pData->ulWidth;
        m_uHeight = pData->ulHeight;
        
        // Get the format and compute the pixel size
        m_eFormat = pData->eFormat;
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
    const uchar* Image::getConstData() const
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
    Image::PixelFormat Image::getFormat() const
    {
        return m_eFormat;
    }

}

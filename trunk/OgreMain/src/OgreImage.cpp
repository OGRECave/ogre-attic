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
#include "OgrePNGCodec.h"
#include "OgreJPEGCodec.h"
#include "OgreTGACodec.h"

namespace Ogre {

    Bool Image::ms_bFlipX = false;
    Bool Image::ms_bFlipY = false;

    //-----------------------------------------------------------------------
    Image::Image()
    {
        mpBuffer = NULL;
    }

    //-----------------------------------------------------------------------
    Image::~Image()
    {
        if (mpBuffer)
        {
            delete[] mpBuffer;
            mpBuffer = NULL;
        }
    }

    void Image::flipX()
    {
    }

    //-----------------------------------------------------------------------
    void Image::flipY()
    {
        if( !mpBuffer )
            Except( 
                Exception::ERR_INTERNAL_ERROR,
                "Can not flip an unitialized texture",
                "Image::flipY" );

        size_t imgSize = mWidth * mHeight * ( mIsGreyscale ? ( mHasAlpha ? 2 : 1 ) : ( mHasAlpha ? 4 : 3 ) );
        size_t linSize = imgSize / mHeight;
        
        byte *pTempBuffer = new byte[ imgSize ];
        byte *ptr1 = mpBuffer, *ptr2 = pTempBuffer + ( ( mHeight - 1 ) * linSize );

        for( size_t i = 0; i < mHeight; i++ )
        {
            memcpy( ptr2, ptr1, linSize );
            ptr1 += linSize; ptr2 -= linSize;
        }

        memcpy( mpBuffer, pTempBuffer, imgSize );
    }

    //-----------------------------------------------------------------------
    Byte * Image::load( const String& strFileName )
    {
        if( mpBuffer )
        {
            delete[] mpBuffer;
            mpBuffer = NULL;
        }

        String strExt;

        size_t pos = strFileName.find_last_of(".");
		if( pos == String::npos )
            Except(
			Exception::ERR_INVALIDPARAMS, 
			"Unable to load image file '" + strFileName + "' - invalid extension.",
            "Image::load" );

        while( pos != strFileName.length() )
            strExt += strFileName[pos++];

        Codec * pCodec = Codec::getColl()[strExt.toLowerCase()];
        if( !pCodec )
            Except(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to load image file '" + strFileName + "' - invalid extension.",
            "Image::load" );

        SDDataChunk encoded;
        DataChunk decoded;

        if( !ArchiveManager::getSingleton()._findResourceData( strFileName, encoded ) )
            Except(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to find image file '" + strFileName + "'.",
            "Image::load" );

        ImageCodec::ImageData * pData = static_cast< ImageCodec::ImageData * > (
            pCodec->decode( encoded, &decoded ) );

        mWidth = pData->ulWidth;
        mHeight = pData->ulHeight;
        mIsGreyscale = pData->bGreyS;
        mHasAlpha = pData->b32Bit;

        delete pData;

        mpBuffer = const_cast< unsigned char * >( decoded.getPtr() );

        if( ms_bFlipY )
            flipY();
        
        // Return ptr
        return mpBuffer;
    }

    //-----------------------------------------------------------------------
    unsigned char* Image::load( DataChunk& chunk, const String& ext )
    {
        String strExt = ext;

        Codec * pCodec = Codec::getColl()[strExt.toLowerCase()];
        if( !pCodec )
            Except(
            Exception::ERR_INVALIDPARAMS, 
            "Unable to load image - invalid extension.",
            "Image::load" );

        DataChunk decoded;

        ImageCodec::ImageData * pData = static_cast< ImageCodec::ImageData * > (
            pCodec->decode( chunk, &decoded ) );

        mWidth = pData->ulWidth;
        mHeight = pData->ulHeight;
        mIsGreyscale = pData->bGreyS;
        mHasAlpha = pData->b32Bit;

        delete pData;

        mpBuffer = const_cast< unsigned char * >( decoded.getPtr() );

        if( ms_bFlipY )
            flipY();

        // Return ptr
        return mpBuffer;        
    }
    void Image::loadRawRGB( Byte *pData, UInt32 ulWidth, UInt32 ulHeight )
    {
        if( mpBuffer )
            delete [] mpBuffer;

        mpBuffer = new unsigned char[ ulHeight * ulWidth * 3 ];
        mWidth = ulWidth;
        mHeight = ulHeight;
        mHasAlpha = false;
        mIsGreyscale = false;

        memcpy( mpBuffer, pData, ulHeight * ulWidth * 3 );

        if( ms_bFlipY )
            flipY();
    }
    void Image::loadRawRGBA( Byte *pData, UInt32 ulWidth, UInt32 ulHeight )
    {
        if( mpBuffer )
            delete [] mpBuffer;

        mpBuffer = new unsigned char[ ulHeight * ulWidth * 4 ];
        mWidth = ulWidth;
        mHeight = ulHeight;
        mHasAlpha = true;
        mIsGreyscale = false;

        memcpy( mpBuffer, pData, ulHeight * ulWidth * 4 );

        if( ms_bFlipY )
            flipY();
    }
    //-----------------------------------------------------------------------
    unsigned char* Image::getData(void)
    {
        return mpBuffer;
    }

    //-----------------------------------------------------------------------
    unsigned short Image::getWidth(void)
    {
        return mWidth;
    }

    //-----------------------------------------------------------------------
    unsigned short Image::getHeight(void)
    {
        return mHeight;
    }

    //-----------------------------------------------------------------------
    unsigned short Image::getRowSpan(void)
    {
        return mRowSpan;
    }

    //-----------------------------------------------------------------------
    bool Image::isGreyscale(void)
    {
        return mIsGreyscale;

    }

    //-----------------------------------------------------------------------
    bool Image::hasAlphaChannel(void)
    {
        return mHasAlpha;
    }
}

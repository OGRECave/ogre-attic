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
#ifndef _Image_H__
#define _Image_H__

#include "OgrePrerequisites.h"

#include "OgreString.h"
#include "OgreDataChunk.h"

namespace Ogre {

    /** Class representing an image file.
        @remarks
            The Image class usually holds uncompressed image data and is the
            only object that can be loaded in a texture. Image  objects handle 
            image data decoding themselves by the means of locating the correct 
            Codec object for each data type.
        @par
            Typically, you would want to use an Image object to load a texture
            when extra processing needs to be done on an image before it is
            loaded or when you want to blit to an existing texture.
    */
    class _OgreExport Image
    {
    public:
        /** Pixel formats allowed by the Image class.
        */
        enum PixelFormat
        {
            FMT_UNKNOWN = 0,
            FMT_ALPHA = 1,
            FMT_GREY = 2,
            FMT_GREY_ALPHA = 3,
            FMT_RGB = 4,
            FMT_RGB_ALPHA = 5
        };
    public:

        /** Standard constructor.
        */
        Image();
        /** Copy-constructor - copies all the data from the target image.
        */
        Image( const Image &img );

        /** Standard destructor.
        */
        ~Image();

        /** Assignment operator - copies all the data from the target image.
        */
        Image & operator = ( const Image & img );

        /** Flips (mirrors) the image around the Y-axis. 
            @remarks
                An example of an original and flipped image:
                <pre>
                        flip axis
                            |
                originalimg|gmilanigiro
                00000000000|00000000000
                00000000000|00000000000
                00000000000|00000000000
                00000000000|00000000000
                00000000000|00000000000
                </pre>
        */
        Image & flipAroundY();

        /** Flips (mirrors) the image around the X-axis.
            @remarks
                An example of an original and flipped image:
                <pre>                
                originalimg
                00000000000
                00000000000
                00000000000
                00000000000
                00000000000
                ------------> flip axis
                00000000000
                00000000000
                00000000000
                00000000000
                00000000000
                originalimg
                </pre>
        */                 
        Image & flipAroundX();

        /** Returns the pixel size  for a given PixelFormat
            @param
                eFormat the PixelFormat to get the pixel size for
            @returns
                the number of bytes per pixel of data
        */
        static uchar PF2PS( PixelFormat eFormat )
        {
            uchar res = 0;

            if( eFormat & FMT_ALPHA )
            {
                res += 1;
            }
            if( eFormat & FMT_RGB )
            {
                res += 3;
            }
            else
            {
                res += 1;
            }

            return res;
        }

        /** Returns the BPP for a given PixelFormat
            @param
                eFormat the PixelFormat to get the BPP for
            @returns
                the number of bits per pixel of data
        */
        static uchar PF2BPP( PixelFormat fmt )
        {
            uchar res = 0;

            if( fmt & FMT_ALPHA )
                res = 8;

            if( fmt & FMT_GREY )
                res += 8;
            else
                res += 24;

            return res;
        }

        /** Converts a BPP value into a pixel format.
        */
        static PixelFormat BPP2PF( uchar bpp )
        {
            if( bpp == 8 )
                return FMT_GREY;
            if( bpp == 16 )
                return FMT_GREY_ALPHA;
            if( bpp == 24 )
                return FMT_RGB;
            if( bpp == 32 )
                return FMT_RGB_ALPHA;
            return FMT_UNKNOWN;
        }

        /** Converts a pixel size value into a pixel format.
        */
        static PixelFormat PS2PF( uchar bpp )
        {
            if( bpp == 1 )
                return FMT_GREY;
            if( bpp == 2 )
                return FMT_GREY_ALPHA;
            if( bpp == 3 )
                return FMT_RGB;
            if( bpp == 4 )
                return FMT_RGB_ALPHA;
            return FMT_UNKNOWN;
        }

        /** Loads raw data from memory. The pixel format has to be specified.
        */
        Image & loadRawData( 
            const DataChunk &pData, 
            ushort uWidth, ushort uHeight, 
            PixelFormat eFormat );

        /** Loads an image file.
            @remarks
                This method loads an image into memory held in the object. The 
                pixel format will be either greyscale or RGB with an optional
                Alpha component.
                The type can be determined by calling getFormat().             
            @param
                strFileName Name of a file file to load.
            @note
                The memory associated with this buffer is destroyed with the
                Image object.
        */
        Image & load( const String& strFileName );

        /** Loads an image file from a chunk of memory.
            @remarks
                This method works in the same way as the filename-based load 
                method except it loads the image from a DataChunk object, ie 
                a chunk of memory. This DataChunk is expected to contain the 
                encoded data as it would be held in a file. 
            @par
                This method is here to support loading from compressed archives
                where you decompress the data from the archive into memory 
                first. This method will then decode the data and return a raw 
                image data stream.
            @param
                chunk The source data.
            @param
                type The type of the image. Used to decide what decompression
                codec to use.
            @see
                Image::load( const String& strFileName )
        */
        Image & load( const DataChunk& chunk, const String& type );

        /** Returns a pointer to the internal image buffer.
        */
        uchar* getData(void);

        /** Returns a const pointer to the internal image buffer.
        */
        const uchar * getConstData() const;       

        /** REturns the size of the data buffer.
        */
        size_t getSize() const;

        /** Gets the width of the image in pixels.
        */
        ushort getWidth(void) const;

        /** Gets the height of the image in pixels.
        */
        ushort getHeight(void) const;

        /** Gets the physical width in bytes of each row of pixels.
        */
        ushort getRowSpan(void) const;

        /** Returns the image format.
        */
        PixelFormat getFormat() const;

    private:
        // The width of the image in pixels
        ushort m_uWidth;
        // The height of the image in pixels
        ushort m_uHeight;

        // The pixel format of the image
        PixelFormat m_eFormat;

        // The number of bytes per pixel
        uchar m_ucPixelSize;
        uchar* m_pBuffer;
    };

} // namespace

#endif

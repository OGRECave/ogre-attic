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
#ifndef __Image_H__
#define __Image_H__

#include "OgrePrerequisites.h"

#include "OgreString.h"
#include "OgreDataChunk.h"

// Library includes
#include "png.h"
// jpeglib hack to stop redefinition of INT32
#define XMD_H
extern "C" {
    #include "jpeglib.h"
}


namespace Ogre {
    /** Class representing an image file.
        @remarks
            This class can load & hold the data for image files. Currently it
            loads PNG and JPG files. I could have supported BMP but chose not to
            because it is not platform independant. Loading BMPs as textures is
            supported by Ogre on Win32 platforms, however, just not through this
            class (the D3DTexture class will load BMPs).
        @par
            I chose not to support GIF since a) it's copyrighted and b) for a 3D
            engine these days, 16-bit colour textures is really the accepted minimum.
            Palettised textures are rarely used anymore. It also simplifies my job!
        @note
            Image data is always in RGB(A) format (optional alpha channel, only in PNG)
    */
    class _OgreExport Image
    {
    public:

        Image();
        ~Image();
        /** Loads an image file and returns a pointer to the raw image data.
            @remarks
                This method loads an image into memory held in the object, and passes a
                pointer to it back to the caller. The data will be in one of 3 formats:
                Greyscale, i.e. one byte per pixel, RGB or RGBA. The type can be determined by
                calling isGreyscale() and hasAlphaChannel(). Note that the memory associated
                with this buffer is destroyed when the Image object is.
            @param
                filename Name of a PNG or JPG file to load.
        */
        unsigned char* load(String filename);
        /** Loads an image file from a chunk of memory and returns a pointer to the uncompressed image data.
            @remarks
                This method works in the same way as the filename-based load method except it loads the image from
                a DataChunk object, i.e. a chunk of memory. This DataChunk is expected to contain the encoded
                data as it would be held in a file. This method is here to support loading from compressed archives
                where you decompress the data from the archive into memory first. This method will then decode the
                data and return a raw image data stream.
            @par
                The data will be in one of 3 formats: Greyscale, i.e. one byte per pixel, RGB or RGBA. The type
                can be determined by calling isGreyscale() and hasAlphaChannel(). Note that the memory associated
                with this buffer is destroyed when the Image object is.
            @param
                chunk The source data.
            @param
                extension The extension associated with the file type, e.g. PNG, JPG, TGA
        */
        unsigned char* load(DataChunk& chunk, String extension);

        /** Returns a pointer to the image data.
        */
        unsigned char* getData(void);

        /** Gets the width of the image in pixels.
        */
        unsigned short getWidth(void);

        /** Gets the height of the image in pixels.
        */
        unsigned short getHeight(void);

        /** Gets the physical width in bytes of each row of pixels.
        */
        unsigned short getRowSpan(void);

        /** Returns true if colour data is greyscale i.e. 1 byte per pixel colour. Otherwise data is
            3 or 4 bytes per pixel (RGB(A)).
        */
        bool isGreyscale(void);

        /** Returns true if colour data has an embedded alpha channel. Alpha channel is
            only available in PNG files.
        */
        bool hasAlphaChannel(void);

    private:
        unsigned short mWidth, mHeight;
        unsigned short mRowSpan;
        unsigned char* mpBuffer;
        bool mIsGreyscale;
        bool mHasAlpha;

        /** Internal utility method for loading from a PNG file.
        */
        void loadFromPNG(String filename);

        /** Internal utility method for loading PNG data.
        */
        void loadFromPNGChunk(DataChunk& chunk);

        /** Internal utility method for loading from a JPG file.
        */
        void loadFromJPG(String filename);

        /** Internal utility method for loading JPG data.
        */
        void loadFromJPGChunk(DataChunk& chunk);

        /** Internal utility method for loading a TGA file.
        */
        void loadFromTGA(String filename);

        /** Internal utility method for loading TGA data.
        */
        void loadFromTGAChunk(DataChunk& chunk);

        /** Internal Jpeg decoder implementation.
        */
        void decodeJPG(jpeg_decompress_struct* cinfo);

        /** Internal PNG decoder implementation.
        */
        void decodePNG(png_structp png_ptr, png_infop info_ptr);
    };
}


#endif

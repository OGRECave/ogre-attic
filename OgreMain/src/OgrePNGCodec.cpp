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
#include "png.h"

#include "OgrePNGCodec.h"
#include "OgreImage.h"
#include "OgreException.h"

namespace Ogre {
    
    //---------------------------------------------------------------------
    void PNGCodec::pngChunkRead(png_structp png_ptr, png_bytep data, png_size_t length)
    {
        DataChunk* pChunk = (DataChunk*)png_ptr->io_ptr;

        if( pChunk->read((void*)data, static_cast< unsigned long >( length ) ) != length )
            png_error(png_ptr, "Read Error.");
    }
    //---------------------------------------------------------------------
    void PNGCodec::code( const DataChunk& input, DataChunk* output, ... ) const
    {        
        OgreGuard( "PNGCodec::code" );

        Except(Exception::UNIMPLEMENTED_FEATURE, "code to memory not implemented",
            "PNGCodec::code");

        OgreUnguard();

    }
    //---------------------------------------------------------------------
    void PNGCodec::codeToFile( const DataChunk& input, 
        const String& outFileName, Codec::CodecData* pData) const
    {
        OgreGuard( "PNGCodec::codeToFile" );

        ImageData* pImgData = static_cast<ImageData*>(pData);

        // Open file
        FILE *fp = fopen(outFileName, "wb");
        if (!fp)
        {
            Except(Exception::ERR_INTERNAL_ERROR, "Cannot open " + outFileName + 
                " for writing.", "PNGCodec::codeToFile");
        }
        
        // Create PNG structs
        png_structp png_ptr = png_create_write_struct
           (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
            NULL, NULL);
        if (!png_ptr)
        {
            Except(Exception::ERR_INTERNAL_ERROR, "Cannot create PNG write structure",
                "PNGCodec::codeToFile");
        }

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
        {
           png_destroy_write_struct(&png_ptr,
             (png_infopp)NULL);
            Except(Exception::ERR_INTERNAL_ERROR, "Cannot create PNG info structure",
                "PNGCodec::codeToFile");
        }

        /*
        // Set up libpng's freaky error handling
        if (setjmp(png_jmpbuf(png_ptr)))
        {
           png_destroy_write_struct(&png_ptr, &info_ptr);
           fclose(fp);
            Except(Exception::ERR_INTERNAL_ERROR, "Problems calling setjmp.",
                "PNGCodec::codeToFile");
        }
        */

        // init
        png_init_io(png_ptr, fp);

        // Set up info
        int bitsPerChannel, colourType;

        bitsPerChannel = 8; // Always use 8 bits per channel
        unsigned short rowspan;
        switch(pImgData->eFormat)
        {
        case Image::FMT_GREY:
            colourType = PNG_COLOR_TYPE_GRAY;
            rowspan = pImgData->ulWidth;
            break;
        case Image::FMT_ALPHA:
            colourType = PNG_COLOR_MASK_ALPHA;
            rowspan = pImgData->ulWidth;
            break;
        case Image::FMT_GREY_ALPHA:
            colourType = PNG_COLOR_TYPE_GRAY_ALPHA;
            rowspan = pImgData->ulWidth * 2;
            break;
        case Image::FMT_RGB:
            colourType = PNG_COLOR_TYPE_RGB;
            rowspan = pImgData->ulWidth * 3;
            break;
        case Image::FMT_RGB_ALPHA:
            colourType = PNG_COLOR_TYPE_RGB_ALPHA;
            rowspan = pImgData->ulWidth * 4;
            break;

        };

        png_set_IHDR(png_ptr, info_ptr, pImgData->ulWidth, pImgData->ulHeight,
            bitsPerChannel, colourType, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        png_bytep* rowPtr = new png_bytep[pImgData->ulHeight];
        const uchar* buffer = input.getPtr();
        for( unsigned i = 0; i < pImgData->ulHeight; i++ )
            rowPtr[i] = (png_bytep)&( buffer[i*rowspan] );

        png_set_rows(png_ptr, info_ptr, rowPtr);


        // Write
        png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
        png_write_end(png_ptr, info_ptr);


        // Clean
        png_destroy_write_struct(&png_ptr, &info_ptr);


        fclose(fp);

        OgreUnguard();
    }
    //---------------------------------------------------------------------
    Codec::CodecData * PNGCodec::decode( const DataChunk& input, DataChunk* output, ... ) const
    {
        OgreGuard( "PNGCodec::decode" );

        png_structp png_ptr = png_create_read_struct(
            PNG_LIBPNG_VER_STRING, 
            (png_voidp)NULL, NULL, NULL );
        if( !png_ptr )
            Except(
            Exception::ERR_INTERNAL_ERROR,
            "Unable to load image from PNG.",
            "PNGCodec::decode");

        png_infop info_ptr = png_create_info_struct( png_ptr );
        if( !info_ptr )
        {
            png_destroy_read_struct(
                &png_ptr,
                (png_infopp)NULL, 
                (png_infopp)NULL );
            Except(
                Exception::ERR_INTERNAL_ERROR,
                "Unable to load image from PNG.",
                "PNGCodec::decode" );
        }

        png_infop end_info = png_create_info_struct( png_ptr );
        if( !end_info )
        {
            png_destroy_read_struct(
                &png_ptr,
                &info_ptr, 
                (png_infopp)NULL );
            Except(
                Exception::ERR_INTERNAL_ERROR,
                "Unable to load image from PNG.",
                "PNGCodec::decode" );
        }

        // Set new read function (memory access)
        png_set_read_fn(
            png_ptr, 
            (png_voidp)&input, 
            pngChunkRead );

        // No need to reverse alpha channel (1.0 is opaque in
        //  PNG, D3D and OpenGL)

        // Read PNG info
        png_read_info( png_ptr, info_ptr );

        // Retrieve dimensions etc
        unsigned width  = png_get_image_width( png_ptr, info_ptr );
        unsigned height = png_get_image_height( png_ptr, info_ptr );

        // Get colour info
        int iBitDepth, iColourType;
        iBitDepth = png_get_bit_depth( png_ptr, info_ptr );
        iColourType = png_get_color_type( png_ptr, info_ptr );

        bool greyscale, has_alpha;

        switch( iColourType )
        {
        case PNG_COLOR_TYPE_PALETTE:
            Except(
                999, 
                "Palettised images not supported.",
                "Image::loadFromPNG");
            break;
        case PNG_COLOR_TYPE_RGB:
            greyscale = false;
            has_alpha = false;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            greyscale = false;
            has_alpha = true;
            break;
        case PNG_COLOR_TYPE_GRAY:
            greyscale = true;
            has_alpha = false;
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            greyscale = true;
            has_alpha = true;
            break;
        }

        unsigned short rowspan = png_get_rowbytes( png_ptr, info_ptr );

        // Allocate the right amount of memory for image
        output->allocate( height * rowspan );
        unsigned char *buffer = const_cast< unsigned char * >( output->getPtr() );

        // Create array of row pointers for PNG lib
        // (This allows it to deal with interlaced images)
        png_bytep* rowPtr = new png_bytep[height];
        for( unsigned i = 0; i < height; i++ )
            rowPtr[i] = (png_bytep)&( buffer[i*rowspan] );

        // Read the whole image
        png_read_image( png_ptr, rowPtr );

        delete[] rowPtr;

        // Clean up
        png_read_end( png_ptr, end_info );
        png_destroy_read_struct(
            &png_ptr, 
            &info_ptr,
            &end_info);

        ImageData * ret_data = new ImageData;

        ret_data->ulHeight = height;
        ret_data->ulWidth = width;

        uchar ucBpp = 0;
        if( has_alpha )
            ucBpp += 8;
        if( greyscale )
            ucBpp += 8;
        else
            ucBpp += 24;

        ret_data->eFormat = Image::BPP2PF( ucBpp );

        OgreUnguardRet( ret_data );
    }

}


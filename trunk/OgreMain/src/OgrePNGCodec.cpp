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
#include "png.h"

#include "OgrePNGCodec.h"
#include "OgreException.h"

BEGIN_OGRE_NAMESPACE

void PNGCodec::pngChunkRead(png_structp png_ptr, png_bytep data, png_size_t length)
{
    DataChunk* pChunk = (DataChunk*)png_ptr->io_ptr;

    if( pChunk->read((void*)data, static_cast< unsigned long >( length ) ) != length )
        png_error(png_ptr, "Read Error.");
}

void PNGCodec::code( const DataChunk& input, DataChunk* output, ... ) const
{        
}
Codec::CodecData * PNGCodec::decode( const DataChunk& input, DataChunk* output ) const
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

    ImageData * data = new ImageData;

    data->ulHeight = height;
    data->ulWidth = width;
    data->bGreyS = greyscale;
    data->b32Bit = has_alpha;

    OgreUnguardRet( data );
}

END_OGRE_NAMESPACE

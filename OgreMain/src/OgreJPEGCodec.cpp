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

#include "OgrePlatform.h"
#include "OgreStdHeaders.h"

// We might as well re-define boolean in this source file, as it doesn't change anything outside.
#ifdef boolean
    #undef boolean
#endif

#ifndef XMD_H
    #define XMD_H
#endif

#ifdef FAR
    #undef FAR
#endif

#define boolean unsigned char
#define HAVE_BOOLEAN

extern "C" {
#include "jpeglib.h"
}

#include "OgreJPEGCodec.h"
#include "OgreImage.h"
#include "OgreException.h"

namespace Ogre {

    // Non-member method because I want to use 'boolean' as defined by jpeglib
    // but I don't want to make the .h dependent on this type (which I'd have to do if I used
    // a static method). Hence the non-member hack.
    boolean dummy_fill_input_buffer( j_decompress_ptr cinfo )
    {
        return TRUE;
    }

    void JPEGCodec::init_source( j_decompress_ptr cinfo )
    {
    }

    void JPEGCodec::skip_input_data( j_decompress_ptr cinfo, long count )
    {
        jpeg_source_mgr * src = cinfo->src;
        if(count > 0) {
            src->bytes_in_buffer -= count;
            src->next_input_byte += count;
        }
    }

    void JPEGCodec::term_source( j_decompress_ptr cinfo )
    {
    }

    void JPEGCodec::code( const DataChunk& input, DataChunk* output, ... ) const
    {
        OgreGuard( "JPEGCodec::code" );
        OgreUnguard();
    }

    Codec::CodecData * JPEGCodec::decode( const DataChunk& input, DataChunk* output, ... ) const
    {
        OgreGuard( "JPEGCodec::decode" );

        struct jpeg_decompress_struct cinfo;

        // allocate and initialize JPEG decompression object
        struct jpeg_error_mgr jerr;

        /* We have to set up the error handler first, in case the initialization
        step fails.  (Unlikely, but it could happen if you are out of memory.)
        This routine fills in the contents of struct jerr, and returns jerr's
        address which we place into the link field in cinfo.
        */
        cinfo.err = jpeg_std_error(&jerr);
        /* Now we can initialize the JPEG decompression object. */
        jpeg_create_decompress(&cinfo);

        // specify data source
        jpeg_source_mgr jsrc;

        // Set up data pointer
        jsrc.bytes_in_buffer = input.getSize();
        jsrc.next_input_byte = (JOCTET*) input.getPtr();
        cinfo.src = &jsrc;

        jsrc.init_source = init_source;
        jsrc.fill_input_buffer = dummy_fill_input_buffer;
        jsrc.skip_input_data = skip_input_data;
        jsrc.resync_to_restart = jpeg_resync_to_restart;    // use default method
        jsrc.term_source = term_source;

        // Decodes JPG input from whatever source
        // Does everything AFTER jpeg_create_decompress
        //   and BEFORE jpeg_destroy_decompress
        // Caller is responsible for arranging these + setting up cinfo

        /* read file parameters with jpeg_read_header() */
        (void) jpeg_read_header(&cinfo, TRUE);

        /* Start decompressor */
        (void) jpeg_start_decompress(&cinfo);

        // Get image data
        unsigned short rowspan = cinfo.image_width * cinfo.num_components;
        unsigned width = cinfo.image_width;
        unsigned height = cinfo.image_height;

        bool has_alpha= false;  //(JPEG never has alpha)
        bool greyscale;

        if (cinfo.jpeg_color_space == JCS_GRAYSCALE)
        {
            greyscale = true;
        }
        else
        {
            greyscale = false;
        }

        // Allocate memory for buffer
        output->allocate( rowspan * height );
        uchar *buffer = const_cast< uchar * >( output->getPtr() );

        /* Here we use the library's state variable cinfo.output_scanline as the
        * loop counter, so that we don't have to keep track ourselves.
        */
        // Create array of row pointers for lib
        uchar **rowPtr = new uchar * [height];
        for( unsigned i = 0; i < height; i++ )
            rowPtr[i] = &buffer[ i * rowspan ];
        unsigned rowsRead = 0;
        while( cinfo.output_scanline < cinfo.output_height )
        {
            rowsRead += jpeg_read_scanlines( &cinfo, &rowPtr[rowsRead], cinfo.output_height - rowsRead );
        }

        delete [] rowPtr;
        /* Finish decompression */

        (void) jpeg_finish_decompress(&cinfo);

        // Release JPEG decompression object

        // This is an important step since it will release a good deal of memory.
        jpeg_destroy_decompress(&cinfo);

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


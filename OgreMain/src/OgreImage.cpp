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
// Ogre includes
#include "OgreImage.h"

#include "OgreException.h"

namespace Ogre {

    /* Crappy  callbacks required by jpeglib */
    static void
    init_source(j_decompress_ptr cinfo)
    {
    }

    static boolean
    fill_input_buffer(j_decompress_ptr cinfo)
    {
        return TRUE;
    }

    static void
    skip_input_data(j_decompress_ptr cinfo, long count)
    {
        jpeg_source_mgr * src = cinfo->src;
        if(count > 0) {
            src->bytes_in_buffer -= count;
            src->next_input_byte += count;
        }
    }

    static void
    term_source(j_decompress_ptr cinfo)
    {
    }

    //-----------------------------------------------------------------------
    Image::Image()
    {
        mpBuffer = 0;

    }

    //-----------------------------------------------------------------------
    Image::~Image()
    {
        if (mpBuffer)
        {
            delete[] mpBuffer;
            mpBuffer = 0;
        }

    }

    //-----------------------------------------------------------------------
    unsigned char* Image::load(String filename)
    {
        if (mpBuffer)
        {
            // Deallocate current image before loading
            delete[] mpBuffer;
            mpBuffer = 0;
        }

        // Determine type of file
        // Get extension
        // Manipulate using C-strings as case-insensitive compare is hard in STL?
        char extension[5];

        int pos = filename.find_last_of(".");
        if (pos == -1)
            Except(999, "Unable to load image - invalid extension.",
                "Image::load");

        strcpy(extension, filename.substr(pos + 1, filename.length() - pos).c_str());

        if (!stricmp(extension, "jpg") || !stricmp(extension, "jpeg"))
            loadFromJPG(filename);
        else if (!stricmp(extension, "png"))
            loadFromPNG(filename);
        else if (!stricmp(extension, "tga"))
            loadFromTGA(filename);
        else
            Except(999, "Unable to load image - invalid extension.",
                "Image::load");


        // Return ptr
        return mpBuffer;

    }

    //-----------------------------------------------------------------------
    unsigned char* Image::load(DataChunk& chunk, String extension)
    {
        if (!stricmp(extension.c_str(), "jpg") || !stricmp(extension.c_str(), "jpeg"))
            loadFromJPGChunk(chunk);
        else if (!stricmp(extension.c_str(), "png"))
            loadFromPNGChunk(chunk);
        else if (!stricmp(extension.c_str(), "tga"))
            loadFromTGAChunk(chunk);
        else
            Except(999, "Unable to load image - invalid extension.",
                "Image::load");

        return mpBuffer;

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

    //-----------------------------------------------------------------------
    // Private members
    //-----------------------------------------------------------------------
    void Image::loadFromPNG(String filename)
    {
        // Load PNG format image
        FILE* fp;
        fp = fopen(filename.c_str(), "rb");
        if (!fp)
            Except(999, "Unable to open PNG file.",
                "Image::loadFromPNG");

        png_structp png_ptr = png_create_read_struct
           (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
            NULL, NULL);
        if (!png_ptr)
            Except(999, "Unable to load image from PNG.",
                "Image::loadFromPNG");


        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
        {
            png_destroy_read_struct(&png_ptr,
               (png_infopp)NULL, (png_infopp)NULL);
            Except(999, "Unable to load image from PNG.",
                "Image::loadFromPNG");
        }

        png_infop end_info = png_create_info_struct(png_ptr);
        if (!end_info)
        {
            png_destroy_read_struct(&png_ptr, &info_ptr,
              (png_infopp)NULL);
            Except(999, "Unable to load image from PNG.",
                "Image::loadFromPNG");
        }

        // Init PNG file IO
        png_init_io(png_ptr, fp);

        // generic method
        decodePNG(png_ptr, info_ptr);

        // Clean up
        png_read_end(png_ptr, end_info);
        png_destroy_read_struct(&png_ptr, &info_ptr,
           &end_info);


        fclose(fp);
    }

    //-----------------------------------------------------------------------
    void Image::loadFromJPG(String filename)
    {

        struct jpeg_decompress_struct cinfo;
        FILE * infile;

        if ((infile = fopen(filename.c_str(), "rb")) == NULL) {
            Except(999, "Cannot open JPG file." , "Image::loadFromJPG");
        }

        /* allocate and initialize JPEG decompression object */
        struct jpeg_error_mgr jerr;
        /* We have to set up the error handler first, in case the initialization
         * step fails.  (Unlikely, but it could happen if you are out of memory.)
         * This routine fills in the contents of struct jerr, and returns jerr's
         * address which we place into the link field in cinfo.
         */
        cinfo.err = jpeg_std_error(&jerr);
        /* Now we can initialize the JPEG decompression object. */
        jpeg_create_decompress(&cinfo);

        /* specify data source (eg, a file) */

        jpeg_stdio_src(&cinfo, infile);

        // Call general-purpose decoder
        decodeJPG(&cinfo);


        /* Release JPEG decompression object */

        /* This is an important step since it will release a good deal of memory. */
        jpeg_destroy_decompress(&cinfo);

        fclose(infile);

    }

    //-----------------------------------------------------------------------
    static void
    pngChunkRead(png_structp png_ptr, png_bytep data, png_size_t length)
    {
        DataChunk* pChunk = (DataChunk*)png_ptr->io_ptr;

        if(pChunk->read((void*)data, length) != length)
            png_error(png_ptr, "Read Error.");

    }
    //-----------------------------------------------------------------------
    void Image::loadFromPNGChunk(DataChunk& chunk)
    {
        png_structp png_ptr = png_create_read_struct
           (PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
            NULL, NULL);
        if (!png_ptr)
            Except(Exception::ERR_INTERNAL_ERROR,
            "Unable to load image from PNG.",
                "Image::loadFromPNGChunk");


        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
        {
            png_destroy_read_struct(&png_ptr,
               (png_infopp)NULL, (png_infopp)NULL);
            Except(Exception::ERR_INTERNAL_ERROR,
                "Unable to load image from PNG.",
                "Image::loadFromPNGChunk");
        }

        png_infop end_info = png_create_info_struct(png_ptr);
        if (!end_info)
        {
            png_destroy_read_struct(&png_ptr, &info_ptr,
              (png_infopp)NULL);
            Except(Exception::ERR_INTERNAL_ERROR,
                "Unable to load image from PNG.",
                "Image::loadFromPNG");
        }

        // Set new read function (memory access)
        png_set_read_fn(png_ptr, (png_voidp)&chunk, pngChunkRead);

        // call general purpose function
        decodePNG(png_ptr, info_ptr);

        // Clean up
        png_read_end(png_ptr, end_info);
        png_destroy_read_struct(&png_ptr, &info_ptr,
           &end_info);



    }
    //-----------------------------------------------------------------------
    void Image::loadFromJPGChunk(DataChunk& chunk)
    {
        struct jpeg_decompress_struct cinfo;

        /* allocate and initialize JPEG decompression object */
        struct jpeg_error_mgr jerr;
        /* We have to set up the error handler first, in case the initialization
         * step fails.  (Unlikely, but it could happen if you are out of memory.)
         * This routine fills in the contents of struct jerr, and returns jerr's
         * address which we place into the link field in cinfo.
         */
        cinfo.err = jpeg_std_error(&jerr);
        /* Now we can initialize the JPEG decompression object. */
        jpeg_create_decompress(&cinfo);

        /* specify data source */
        jpeg_source_mgr jsrc;
        // Set up data pointer
        jsrc.bytes_in_buffer = chunk.getSize();
        jsrc.next_input_byte = (JOCTET*) chunk.getPtr();
        cinfo.src = &jsrc;

        jsrc.init_source = init_source;
        jsrc.fill_input_buffer = fill_input_buffer;
        jsrc.skip_input_data = skip_input_data;
        jsrc.resync_to_restart = jpeg_resync_to_restart;    // use default method
        jsrc.term_source = term_source;

        // Call general-purpose decoder
        decodeJPG(&cinfo);


        /* Release JPEG decompression object */

        /* This is an important step since it will release a good deal of memory. */
        jpeg_destroy_decompress(&cinfo);

    }

    //-----------------------------------------------------------------------
    void Image::decodeJPG(jpeg_decompress_struct* cinfo)
    {
        // Decodes JPG input from whatever source
        // Does everything AFTER jpeg_create_decompress
        //   and BEFORE jpeg_destroy_decompress
        // Caller is responsible for arranging these + setting up cinfo

        /* read file parameters with jpeg_read_header() */
        (void) jpeg_read_header(cinfo, TRUE);

        /* Start decompressor */
        (void) jpeg_start_decompress(cinfo);

        // Get image data
        mRowSpan = cinfo->image_width * cinfo->num_components;
        mWidth = cinfo->image_width;
        mHeight = cinfo->image_height;
        mHasAlpha = false;  //(JPEG never has alpha)
        if (cinfo->jpeg_color_space == JCS_GRAYSCALE)
        {
            mIsGreyscale = true;
        }
        else
        {
            mIsGreyscale = false;
        }

        // Allocate memory for buffer
        mpBuffer = new unsigned char[mRowSpan * mHeight];


        /* Here we use the library's state variable cinfo.output_scanline as the
         * loop counter, so that we don't have to keep track ourselves.
         */
        // Create array of row pointers for lib
        unsigned char** rowPtr = new unsigned char*[mHeight];
        for (int i = 0; i < mHeight; i++)
            rowPtr[i] = &mpBuffer[i*mRowSpan];
        int rowsRead = 0;
        while (cinfo->output_scanline < cinfo->output_height)
        {
            rowsRead += jpeg_read_scanlines(cinfo, &rowPtr[rowsRead], cinfo->output_height - rowsRead);
        }

        delete [] rowPtr;
        /* Finish decompression */

        (void) jpeg_finish_decompress(cinfo);
    }

    //-----------------------------------------------------------------------
    void Image::decodePNG(png_structp png_ptr, png_infop info_ptr)
    {
        // No need to reverse alpha channel (1.0 is opaque in
        //  PNG, D3D and OpenGL)

        // Read PNG info
        png_read_info(png_ptr, info_ptr);

        // Retrieve dimensions etc
        mWidth = png_get_image_width(png_ptr, info_ptr);
        mHeight = png_get_image_height(png_ptr, info_ptr);

        // Get colour info
        int iBitDepth, iColourType;
        iBitDepth = png_get_bit_depth(png_ptr, info_ptr);
        iColourType = png_get_color_type(png_ptr, info_ptr);

        switch(iColourType)
        {
        case PNG_COLOR_TYPE_PALETTE:
            Except(999, "Palettised images not supported.",
                "Image::loadFromPNG");
            break;
        case PNG_COLOR_TYPE_RGB:
            mIsGreyscale = false;
            mHasAlpha = false;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            mIsGreyscale = false;
            mHasAlpha = true;
            break;
        case PNG_COLOR_TYPE_GRAY:
            mIsGreyscale = true;
            mHasAlpha = false;
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            mIsGreyscale = true;
            mHasAlpha = true;
            break;


        }

        mRowSpan = png_get_rowbytes(png_ptr, info_ptr);

        // Allocate the right amount of memory for image
        mpBuffer = new unsigned char[mHeight * mRowSpan];

        // Create array of row pointers for PNG lib
        // (This allows it to deal with interlaced images)
        png_bytep* rowPtr = new png_bytep[mHeight];
        for (int i = 0; i < mHeight; i++)
            rowPtr[i] = (png_bytep)&(mpBuffer[i*mRowSpan]);

        // Read the whole image
        png_read_image(png_ptr, rowPtr);

        delete[] rowPtr;



    }
    //-----------------------------------------------------------------------
    void Image::loadFromTGA(String filename)
    {
        DataChunk chunk;

        FILE* fp = fopen(filename, "rb");
        if (!fp)
        {
            Except(Exception::ERR_FILE_NOT_FOUND,
                "Can't find file " + filename,
                "Image::loadFromTGA");
        }

        // Read in entire file
        fseek(fp, 0, SEEK_END);
        unsigned long size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        chunk.allocate(size);
        if (fread((void*)chunk.getPtr(), 1, size,fp) != size)
        {
            Except(Exception::ERR_INTERNAL_ERROR,
                "Error reading " + filename,
                "Image::loadFromTGA");
        }

        loadFromTGAChunk(chunk);


    }
    //-----------------------------------------------------------------------
#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#endif

    struct TgaHeader {
        unsigned char    id_len;
        unsigned char    cm_type;
        unsigned char    type;
        unsigned short  cm_start;
        unsigned short  cm_len;
        unsigned char    cm_bits;
        unsigned short  xorg;
        unsigned short  yorg;
        unsigned short  width;
        unsigned short  height;
        unsigned char    bpp;
        unsigned char    flags;
    };
#ifdef _MSC_VER
#pragma pack(pop)
#endif

    void Image::loadFromTGAChunk(DataChunk& chunk)
    {
        // Note that this implementation is modified from code
        // Copyright (c) 2000,2001 Bart Sekura

        // validate hdr
        const TgaHeader* hdr = (const TgaHeader*) chunk.getPtr();
        if(!(hdr->type == 2 || hdr->type == 10))
        {
            Except(Exception::ERR_INTERNAL_ERROR,
                "Unsupported TGA file type " + hdr->type,
                "Image::loadFromTGAChunk");
        }

        mWidth = hdr->width;
        mHeight = hdr->height;
        if (hdr->bpp == 32) // RGBA
            mHasAlpha = true;
        else
            mHasAlpha = false;
        mIsGreyscale = false;

        int offset = sizeof(TgaHeader);
        if(hdr->id_len) {
            offset += hdr->id_len;
        }
        if(hdr->cm_len) {
            offset += hdr->cm_len*(hdr->cm_bits>>3);
        }

        const unsigned char* data = chunk.getPtr()+offset;
        unsigned int size;
        bool rgb_swap = true;

        // the big ugly switch
        switch(hdr->type) {
        case 2: // rgb(a)
            switch(hdr->bpp) {
            case 16:
                {
                    // upgrade to RGB
                    size = mWidth*mHeight*3;
                    mpBuffer = new unsigned char[size];
                    const unsigned short* s = (const unsigned short*) data;

                    for(int i = 0; i < mWidth*mHeight; i++) {
                        unsigned short tmp = *s++;
                        int r,g,b;
                        b = tmp & 0x1f;
                        g = (tmp>>5) & 0x1f;
                        r = (tmp>>10) & 0x1f;

                        b<<=3,g<<=3,r<<=3;
                        mpBuffer[i*3+0]=r;
                        mpBuffer[i*3+1]=g;
                        mpBuffer[i*3+2]=b;
                    }
                    rgb_swap = false;
                }
                break;
            case 24:
            case 32:
                {
                    size = mWidth*mHeight*(hdr->bpp>>3);
                    mpBuffer = new unsigned char[size];
                    memcpy(mpBuffer,data,size);
                }
                break;
            };
            break;

        case 10: // RLE
            switch(hdr->bpp) {
            case 24:
            case 32:
                {
                    mpBuffer = new unsigned char[mWidth*mHeight*(hdr->bpp>>3)];
                    size = mWidth*mHeight;
                    unsigned int i = 0;
                    unsigned char* p = mpBuffer;
                    const unsigned char* s = data;
                    int r,g,b,a;

                    while(i<size) {
                        int rle = (int)*s++&0xff;
                        if(rle<0x80) {
                            rle++;
                            while(rle) {
                                b = (int)*s++&0xff;
                                g = (int)*s++&0xff;
                                r = (int)*s++&0xff;
                                if(hdr->bpp == 32) {
                                    a = (int)*s++&0xff;
                                }

                                *p++ = r;
                                *p++ = g;
                                *p++ = b;
                                if(hdr->bpp == 32) {
                                    *p++ = a;
                                }

                                rle--;
                                i++;

                                if(i>size) {
                                    Except(Exception::ERR_INTERNAL_ERROR,
                                        "Overrun in TGA RLE read.",
                                        "Image::loadFromTGAChunk");
                                }
                            }
                        }
                        else {
                            rle -= 127;

                            b = (int)*s++&0xff;
                            g = (int)*s++&0xff;
                            r = (int)*s++&0xff;
                            if(hdr->bpp == 32) {
                                a = (int)*s++&0xff;
                            }

                            while(rle) {
                                *p++ = r;
                                *p++ = g;
                                *p++ = b;
                                if(hdr->bpp == 32) {
                                    *p++ = a;
                                }

                                i++;

                                if(i>size) {
                                    Except(Exception::ERR_INTERNAL_ERROR,
                                        "Overrun in TGA RLE read.",
                                        "Image::loadFromTGAChunk");
                                }
                                rle--;
                            }
                        }
                    }
                }
                break;
            case 8:
            case 16:
                Except(Exception::ERR_INTERNAL_ERROR,
                    "Unsupported TGA RLE type.",
                    "Image::loadFromTGAChunk");
            }; // switch
            break;
        };

        // swap bgr to rgb for bpp={24,32}
        int bytes = hdr->bpp>>3;

        if(bytes > 1) {
            for(unsigned int c = 0; c < size; c += bytes) {
                unsigned char tmp = mpBuffer[c];
                mpBuffer[c] = mpBuffer[c+2];
                mpBuffer[c+2] = tmp;
            }
        }


        //swap_scanlines();
        unsigned char* tmp = new unsigned char[mWidth*bytes];
        for(int i = 0; i < mHeight/2; i++) {
            memcpy(tmp,    mpBuffer + i*mWidth*bytes, mWidth*bytes);
            memcpy(mpBuffer + i*mWidth*bytes, mpBuffer + (mHeight-i-1)*mWidth*bytes, mWidth*bytes);
            memcpy(mpBuffer + (mHeight-i-1)*mWidth*bytes, tmp, mWidth*bytes);
        }
        delete tmp;


    }

}

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright � 2000-2001 Steven J. Streeting
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
#include "OgreTGACodec.h"
#include "OgreImage.h"
#include "OgreException.h"

namespace Ogre {

    void TGACodec::code( const DataChunk& input, DataChunk* output, ... ) const
    {
        OgreGuard( "TGACodec::code" );
        OgreUnguard();
    }

    Codec::CodecData * TGACodec::decode( const DataChunk& input, DataChunk* output, ... ) const
    {
        OgreGuard( "TGACodec::decode" );
        // Note that this implementation is modified from code
        // Copyright (c) 2000,2001 Bart Sekura

        // validate hdr
        const TgaHeader* hdr = (const TgaHeader*) input.getPtr();
        if(!(hdr->type == 2 || hdr->type == 10))
        {
            Except(Exception::ERR_INTERNAL_ERROR,
                "Unsupported TGA file type " + hdr->type,
                "Image::loadFromTGAinput");
        }

        unsigned mWidth, mHeight;
        bool mHasAlpha, mIsGreyscale;
        uchar *mpBuffer;

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

        const unsigned char* data = input.getPtr()+offset;
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
                output->allocate( size );
                mpBuffer = const_cast< uchar * >( output->getPtr() );
                const unsigned short* s = (const unsigned short*) data;

                for( unsigned i = 0; i < mWidth*mHeight; i++) {
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
                output->allocate( size );
                mpBuffer = const_cast< uchar * >( output->getPtr() );
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
                output->allocate( mWidth*mHeight*(hdr->bpp>>3) );
                mpBuffer = const_cast< uchar * >( output->getPtr() );                    
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
                                    "Image::loadFromTGAinput");
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
                                    "Image::loadFromTGAinput");
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
                "Image::loadFromTGAinput");
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

        unsigned char* tmp = new unsigned char[mWidth*bytes];
        for( unsigned i = 0; i < mHeight/2; i++) {
            memcpy(tmp,    mpBuffer + i*mWidth*bytes, mWidth*bytes);
            memcpy(mpBuffer + i*mWidth*bytes, mpBuffer + (mHeight-i-1)*mWidth*bytes, mWidth*bytes);
            memcpy(mpBuffer + (mHeight-i-1)*mWidth*bytes, tmp, mWidth*bytes);
        }
        delete tmp;

        ImageData * ret_data = new ImageData;

        ret_data->ulHeight = mHeight;
        ret_data->ulWidth = mWidth;
        
        uchar ucBpp = 0;
        if( mHasAlpha )
            ucBpp += 8;
        if( mIsGreyscale )
            ucBpp += 8;
        else
            ucBpp += 24;

        ret_data->eFormat = Image::BPP2PF( ucBpp );

        OgreUnguardRet( ret_data );
    }

}
//-----------------------------------------------------------------------------
// This is the CVS log of the file. Do NOT modify beyond this point as this
// may cause inconsistencies between the actual log and what's written here.
// (for more info, see http://www.cvshome.org/docs/manual/cvs_12.html#SEC103 )
//
// $Log$
// Revision 1.5  2002/08/22 14:52:14  cearny
// Linux changes.
//
// Revision 1.4  2002/08/22 12:32:10  cearny
// Some more 'byte' removals.
//
//-----------------------------------------------------------------------------

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2005 The OGRE Team
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
#include "OgreStableHeaders.h"
#include "OgrePixelFormat.h"
#include "OgreBitwise.h"
#include "OgreColourValue.h"
#include "OgreException.h"
#include <IL/il.h>

namespace {
#include "OgrePixelConversions.h"
};

namespace Ogre {

    //-----------------------------------------------------------------------
    /**
    * A record that describes a pixel format in detail.
    */
    struct PixelFormatDescription {
        /* Name of the format, as in the enum */
        char *name;
        /* Number of bytes one element (colour value) takes. */
        int elemBytes;
        /* Pixel format flags, see enum PixelFormatFlags for the bit field
        * definitions 
        */
        unsigned int flags;
        /* Number of bits in one element. */
        // int elemBits; == elemBytes * 8
        /* Number of bits for red(or luminance), green, blue, alpha
        */
        int rbits,gbits,bbits,abits; /*, ibits, dbits, ... */

        /* Masks and shifts as used by packers/unpackers */
        uint32 rmask, gmask, bmask, amask;
        int rshift, gshift, bshift, ashift;
    };
    //-----------------------------------------------------------------------
    /** Pixel format database */
    PixelFormatDescription _pixelFormats[PF_COUNT] = {
        {"PF_UNKNOWN", 
            /* Bytes per element */ 
            0,  
            /* Flags */
            0,  
            /* rbits, gbits, bbits, abits */
            0, 0, 0, 0,
            /* Masks and shifts */
            0, 0, 0, 0, 0, 0, 0, 0 
        },
        {"PF_L8", 
        /* Bytes per element */ 
        1,  
        /* Flags */
        PFF_LUMINANCE | PFF_NATIVEENDIAN,
        /* rbits, gbits, bbits, abits */
        8, 0, 0, 0,
        /* Masks and shifts */
        0xFF, 0, 0, 0, 0, 0, 0, 0 
        },
        {"PF_L16", 
        /* Bytes per element */ 
        2,  
        /* Flags */
        PFF_LUMINANCE | PFF_NATIVEENDIAN,  
        /* rbits, gbits, bbits, abits */
        16, 0, 0, 0,
        /* Masks and shifts */
        0xFFFF, 0, 0, 0, 0, 0, 0, 0 
        },
        {"PF_A8", 
        /* Bytes per element */ 
        1,  
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 8,
        /* Masks and shifts */
        0, 0, 0, 0xFF, 0, 0, 0, 0 
        },
        {"PF_A4L4", 
        /* Bytes per element */ 
        1,  
        /* Flags */
        PFF_HASALPHA | PFF_LUMINANCE | PFF_NATIVEENDIAN,
        /* rbits, gbits, bbits, abits */
        4, 0, 0, 4,
        /* Masks and shifts */
        0x0F, 0, 0, 0xF0, 0, 0, 0, 4
        },
        {"PF_L4A4", 
        /* Bytes per element */ 
        1,  
        /* Flags */
        PFF_HASALPHA | PFF_LUMINANCE | PFF_NATIVEENDIAN,  
        /* rbits, gbits, bbits, abits */
        4, 0, 0, 4,
        /* Masks and shifts */
        0xF0, 0, 0, 0x0F, 4, 0, 0, 0 
        },
        {"PF_R5G6B5", 
        /* Bytes per element */ 
        2,  
        /* Flags */
        PFF_NATIVEENDIAN,  
        /* rbits, gbits, bbits, abits */
        5, 6, 5, 0,
        /* Masks and shifts */
        0xF800, 0x07E0, 0x001F, 0, 
        11, 5, 0, 0 
        },
        {"PF_B5G6R5", 
        /* Bytes per element */ 
        2,  
        /* Flags */
        PFF_NATIVEENDIAN,  
        /* rbits, gbits, bbits, abits */
        5, 6, 5, 0,
        /* Masks and shifts */
        0x001F, 0x07E0, 0xF800, 0, 
        0, 5, 11, 0 
        },
        {"PF_A4R4G4B4", 
        /* Bytes per element */ 
        2,  
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,  
        /* rbits, gbits, bbits, abits */
        4, 4, 4, 4,
        /* Masks and shifts */
        0x0F00, 0x00F0, 0x000F, 0xF000, 
        8, 4, 0, 12 
        },
        {"PF_B4G4R4A4", 
        /* Bytes per element */ 
        2,  
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,  
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 0,
        /* Masks and shifts */
        0x00F0, 0x0F00, 0xF000, 0x000F, 
        4, 8, 12, 0,
        },
        {"PF_R8G8B8", 
        /* Bytes per element */ 
        3,  // 24 bit integer -- special
        /* Flags */
        PFF_NATIVEENDIAN,  
        /* rbits, gbits, bbits, abits */
        8, 8, 8, 0,
        /* Masks and shifts */
        0xFF0000, 0x00FF00, 0x0000FF, 0, 
        16, 8, 0, 0 
        },
        {"PF_B8G8R8", 
        /* Bytes per element */ 
        3,  // 24 bit integer -- special
        /* Flags */
        PFF_NATIVEENDIAN,  
        /* rbits, gbits, bbits, abits */
        8, 8, 8, 0,
        /* Masks and shifts */
        0x0000FF, 0x00FF00, 0xFF0000, 0, 
        0, 8, 16, 0 
        },
        {"PF_A8R8G8B8", 
        /* Bytes per element */ 
        4,  
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,  
        /* rbits, gbits, bbits, abits */
        8, 8, 8, 8,
        /* Masks and shifts */
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000,
        16, 8, 0, 24
        },
        {"PF_A8B8G8R8", 
        /* Bytes per element */ 
        4,  
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,  
        /* rbits, gbits, bbits, abits */
        8, 8, 8, 8,
        /* Masks and shifts */
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000,
        0, 8, 16, 24,
        },
        {"PF_B8G8R8A8", 
        /* Bytes per element */ 
        4,  
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,  
        /* rbits, gbits, bbits, abits */
        8, 8, 8, 8,
        /* Masks and shifts */
        0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF,
        8, 16, 24, 0
        },
        {"PF_A2R10G10B10", 
        /* Bytes per element */ 
        4,  
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,  
        /* rbits, gbits, bbits, abits */
        10, 10, 10, 2,
        /* Masks and shifts */
        0x3FF00000, 0x000FFC00, 0x000003FF, 0xC0000000,
        20, 10, 0, 30
        },
        {"PF_A2B10G10R10", 
        /* Bytes per element */ 
        4,  
        /* Flags */
        PFF_HASALPHA | PFF_NATIVEENDIAN,  
        /* rbits, gbits, bbits, abits */
        10, 10, 10, 2,
        /* Masks and shifts */
        0x000003FF, 0x000FFC00, 0x3FF00000, 0xC0000000,
        0, 10, 20, 30
        },
        {"PF_DXT1", 
        /* Bytes per element */ 
        0,  
        /* Flags */
        PFF_COMPRESSED | PFF_HASALPHA,  
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0 
        },
        {"PF_DXT2", 
        /* Bytes per element */ 
        0,  
        /* Flags */
        PFF_COMPRESSED | PFF_HASALPHA,  
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0 
        },
        {"PF_DXT3", 
        /* Bytes per element */ 
        0,  
        /* Flags */
        PFF_COMPRESSED | PFF_HASALPHA,  
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0 
        },
        {"PF_DXT4", 
        /* Bytes per element */ 
        0,  
        /* Flags */
        PFF_COMPRESSED | PFF_HASALPHA,  
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0 
        },
        {"PF_DXT5", 
        /* Bytes per element */ 
        0,  
        /* Flags */
        PFF_COMPRESSED | PFF_HASALPHA,  
        /* rbits, gbits, bbits, abits */
        0, 0, 0, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0 
        },
        {"PF_FP_R16G16B16", 
        /* Bytes per element */ 
        6,  
        /* Flags */
        PFF_FLOAT,  
        /* rbits, gbits, bbits, abits */
        16, 16, 16, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0 
        },
        {"PF_FP_R16G16B16A16", 
        /* Bytes per element */ 
        8,  
        /* Flags */
        PFF_FLOAT,  
        /* rbits, gbits, bbits, abits */
        16, 16, 16, 16,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0 
        },
        {"PF_FP_R32G32B32", 
        /* Bytes per element */ 
        12,  
        /* Flags */
        PFF_FLOAT,  
        /* rbits, gbits, bbits, abits */
        32, 32, 32, 0,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0 
        },
        {"PF_FP_R32G32B32A32", 
        /* Bytes per element */ 
        16,  
        /* Flags */
        PFF_FLOAT,  
        /* rbits, gbits, bbits, abits */
        32, 32, 32, 32,
        /* Masks and shifts */
        0, 0, 0, 0, 0, 0, 0, 0 
        }
    };
    //-----------------------------------------------------------------------
    /**
    * Directly get the description record for provided pixel format. For debug builds,
    * this checks the bounds of fmt with an assertion.
    */    
    static inline const PixelFormatDescription &getDescriptionFor(PixelFormat fmt) 
    {
        int ord = (int)fmt;
        assert(ord>=0 && ord<PF_COUNT);

        return _pixelFormats[ord];
    }
    //-----------------------------------------------------------------------
    int PixelUtil::getNumElemBytes( PixelFormat format )
    {
        return getDescriptionFor(format).elemBytes;
    }
    //-----------------------------------------------------------------------
    int PixelUtil::getNumElemBits( PixelFormat format )
    {
        return getDescriptionFor(format).elemBytes * 8;
    }
    //-----------------------------------------------------------------------
    unsigned int PixelUtil::getFlags( PixelFormat format )
    {
        return getDescriptionFor(format).flags;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::hasAlpha(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_HASALPHA) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isFloatingPoint(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_FLOAT) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isCompressed(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_COMPRESSED) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isDepth(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_DEPTH) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isNativeEndian(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_NATIVEENDIAN) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isLuminance(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_LUMINANCE) > 0;
    }
    //-----------------------------------------------------------------------
    void PixelUtil::getBitDepths(PixelFormat format, int rgba[4])
    {
        const PixelFormatDescription &des = getDescriptionFor(format);
        rgba[0] = des.rbits;
        rgba[1] = des.gbits;
        rgba[2] = des.bbits;
        rgba[3] = des.abits;
    }
    //-----------------------------------------------------------------------
    String PixelUtil::getFormatName(PixelFormat srcformat)
    {
        return getDescriptionFor(srcformat).name;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isAccessible(PixelFormat srcformat) {
        unsigned int flags = getFlags(srcformat);
        return !((flags & PFF_COMPRESSED) || (flags & PFF_DEPTH));
    }
    //-----------------------------------------------------------------------
    /*************************************************************************
    * Pixel packing/unpacking utilities
    */
    void PixelUtil::packColour(const ColourValue &colour, PixelFormat pf,  void* dest)
    {
        packColour(colour.r, colour.g, colour.b, colour.a, pf, dest);
    }
    //-----------------------------------------------------------------------
    void PixelUtil::packColour(uint8 r, uint8 g, uint8 b, uint8 a, PixelFormat pf,  void* dest)
    {
        const PixelFormatDescription &des = getDescriptionFor(pf);
        if(des.flags & PFF_NATIVEENDIAN) {
            // Shortcut for integer formats packing
            unsigned int value = ((Bitwise::fixedToFixed(r, 8, des.rbits)<<des.rshift) & des.rmask) |
                ((Bitwise::fixedToFixed(g, 8, des.gbits)<<des.gshift) & des.gmask) |
                ((Bitwise::fixedToFixed(b, 8, des.bbits)<<des.bshift) & des.bmask) |
                ((Bitwise::fixedToFixed(a, 8, des.abits)<<des.ashift) & des.amask);
            // And write to memory
            Bitwise::intWrite(dest, des.elemBytes, value);
        } else {
            // Convert to float
            packColour((float)r/255.0f,(float)g/255.0f,(float)b/255.0f,(float)a/255.0f, pf, dest);
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::packColour(float r, float g, float b, float a, PixelFormat pf,  void* dest)
    {
        // Catch-it-all here
        const PixelFormatDescription &des = getDescriptionFor(pf);
        if(des.flags & PFF_NATIVEENDIAN) {
            // Do the packing
            //std::cerr << dest << " " << r << " " << g <<  " " << b << " " << a << std::endl;
            unsigned int value = ((Bitwise::floatToFixed(r, des.rbits)<<des.rshift) & des.rmask) |
                ((Bitwise::floatToFixed(g, des.gbits)<<des.gshift) & des.gmask) |
                ((Bitwise::floatToFixed(b, des.bbits)<<des.bshift) & des.bmask) |
                ((Bitwise::floatToFixed(a, des.abits)<<des.ashift) & des.amask);
            // And write to memory
            Bitwise::intWrite(dest, des.elemBytes, value);                                
        } else {
            switch(pf)
            {
            case PF_FP_R32G32B32:
                ((float*)dest)[0] = r;
                ((float*)dest)[1] = g;
                ((float*)dest)[2] = b;
                break;
            case PF_FP_R32G32B32A32:
                ((float*)dest)[0] = r;
                ((float*)dest)[1] = g;
                ((float*)dest)[2] = b;
                ((float*)dest)[3] = a;
                break;
            case PF_FP_R16G16B16:
                ((uint16*)dest)[0] = Bitwise::floatToHalf(r);
                ((uint16*)dest)[1] = Bitwise::floatToHalf(g);
                ((uint16*)dest)[2] = Bitwise::floatToHalf(b);
                break;
            case PF_FP_R16G16B16A16:
                ((uint16*)dest)[0] = Bitwise::floatToHalf(r);
                ((uint16*)dest)[1] = Bitwise::floatToHalf(g);
                ((uint16*)dest)[2] = Bitwise::floatToHalf(b);
                ((uint16*)dest)[3] = Bitwise::floatToHalf(a);
                break;
            default:
                // Not yet supported
                Except(
                    Exception::UNIMPLEMENTED_FEATURE, 
                    "pack to "+getFormatName(pf)+" not implemented", 
                    "PixelUtil::packColour");
                break;
            }
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::unpackColour(ColourValue *colour, PixelFormat pf,  const void* src)
    {
        unpackColour(&colour->r, &colour->g, &colour->b, &colour->a, pf, src);
    }
    //-----------------------------------------------------------------------
    void PixelUtil::unpackColour(uint8 *r, uint8 *g, uint8 *b, uint8 *a, PixelFormat pf,  const void* src)
    {
        const PixelFormatDescription &des = getDescriptionFor(pf);
        if(des.flags & PFF_NATIVEENDIAN) {
            // Shortcut for integer formats unpacking
            unsigned int value = Bitwise::intRead(src, des.elemBytes);
            if(des.flags & PFF_LUMINANCE) 
            {
                // Luminance format -- only rbits used
                *r = *g = *b = Bitwise::fixedToFixed(
                    (value & des.rmask)>>des.rshift, des.rbits, 8);
            } 
            else 
            {
                *r = Bitwise::fixedToFixed((value & des.rmask)>>des.rshift, des.rbits, 8);
                *g = Bitwise::fixedToFixed((value & des.gmask)>>des.gshift, des.gbits, 8);
                *b = Bitwise::fixedToFixed((value & des.bmask)>>des.bshift, des.bbits, 8);
            }
            if(des.flags & PFF_HASALPHA)
            {
                *a = Bitwise::fixedToFixed((value & des.amask)>>des.ashift, des.abits, 8);
            }
            else
            {
                *a = 255; // No alpha, default a component to full
            }
        } else {
            // Do the operation with the more generic floating point
            float rr, gg, bb, aa;
            unpackColour(&rr,&gg,&bb,&aa, pf, src);
            *r = Bitwise::floatToFixed(rr, 8);
            *g = Bitwise::floatToFixed(gg, 8);            
            *b = Bitwise::floatToFixed(bb, 8); 
            *a = Bitwise::floatToFixed(aa, 8);                    
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::unpackColour(float *r, float *g, float *b, float *a, 
        PixelFormat pf,  const void* src) 
    {
        const PixelFormatDescription &des = getDescriptionFor(pf);
        if(des.flags & PFF_NATIVEENDIAN) {
            // Shortcut for integer formats unpacking
            unsigned int value = Bitwise::intRead(src, des.elemBytes);
            if(des.flags & PFF_LUMINANCE) 
            {
                // Luminance format -- only rbits used
                *r = *g = *b = Bitwise::fixedToFloat(
                    (value & des.rmask)>>des.rshift, des.rbits);
            } 
            else
            { 
                *r = Bitwise::fixedToFloat((value & des.rmask)>>des.rshift, des.rbits);
                *g = Bitwise::fixedToFloat((value & des.gmask)>>des.gshift, des.gbits);
                *b = Bitwise::fixedToFloat((value & des.bmask)>>des.bshift, des.bbits);
            }
            if(des.flags & PFF_HASALPHA)
            {
                *a = Bitwise::fixedToFloat((value & des.amask)>>des.ashift, des.abits);
            }
            else
            {
                *a = 1.0f; // No alpha, default a component to full
            }
        } else {
            switch(pf)
            {
            case PF_FP_R32G32B32:
                *r = ((float*)src)[0];
                *g = ((float*)src)[1];
                *b = ((float*)src)[2];
                *a = 1.0f;
                break;
            case PF_FP_R32G32B32A32:
                *r = ((float*)src)[0];
                *g = ((float*)src)[1];
                *b = ((float*)src)[2];
                *a = ((float*)src)[3];
                break;
            case PF_FP_R16G16B16:
                *r = Bitwise::halfToFloat(((uint16*)src)[0]);
                *g = Bitwise::halfToFloat(((uint16*)src)[1]);
                *b = Bitwise::halfToFloat(((uint16*)src)[2]);
                *a = 1.0f;
                break;
            case PF_FP_R16G16B16A16:
                *r = Bitwise::halfToFloat(((uint16*)src)[0]);
                *g = Bitwise::halfToFloat(((uint16*)src)[1]);
                *b = Bitwise::halfToFloat(((uint16*)src)[2]);
                *a = Bitwise::halfToFloat(((uint16*)src)[3]);
                break;
            default:
                // Not yet supported
                Except(Exception::UNIMPLEMENTED_FEATURE, 
                    "unpack from "+getFormatName(pf)+" not implemented", 
                    "PixelUtil::unpackColour");
                break;
            }
        }    
    }
    //-----------------------------------------------------------------------
    /* Convert pixels from one format to another */
    void PixelUtil::bulkPixelConversion(void *srcp, PixelFormat srcFormat, 
        void *destp, PixelFormat dstFormat, unsigned int count)
    {
        PixelBox src, dst;
        src.data = srcp;
        src.format = srcFormat;
        src.width = count;
        src.height = 1;
        src.depth = 1;
        src.setConsecutive();

        dst.data = destp;
        dst.format = dstFormat;
        dst.width = count;
        dst.height = 1;
        dst.depth = 1;
        dst.rowPitch = 0;
        dst.slicePitch = 0;
        dst.setConsecutive();

        bulkPixelConversion(src, dst);
    }
    //-----------------------------------------------------------------------
    void PixelUtil::bulkPixelConversion(const PixelBox &src, const PixelBox &dst)
    {
        assert(src.width == dst.width && src.height==dst.height && src.depth==dst.depth);

        uint8 *srcptr = static_cast<uint8*>(src.data);
        uint8 *dstptr = static_cast<uint8*>(dst.data);
        unsigned int srcPixelSize = PixelUtil::getNumElemBytes(src.format);
        unsigned int dstPixelSize = PixelUtil::getNumElemBytes(dst.format);

        // Calculate pitches+skips in bytes
        int srcRowPitchBytes = src.rowPitch*srcPixelSize;
        int srcRowSkipBytes = src.getRowSkip()*srcPixelSize;
        int srcSliceSkipBytes = src.getSliceSkip()*srcPixelSize;

        int dstRowPitchBytes = dst.rowPitch*dstPixelSize;
        int dstRowSkipBytes = dst.getRowSkip()*dstPixelSize;
        int dstSliceSkipBytes = dst.getSliceSkip()*dstPixelSize;

        // The easy case
        if(src.format == dst.format) {
            // Everything consecutive?
            if(src.isConsecutive() && dst.isConsecutive()) 
            {
                std::copy(srcptr, srcptr+(src.width*src.height*src.depth*srcPixelSize), dstptr);
                return;
            }

            // Otherwise, copy per row
            unsigned int rowSize = src.width*srcPixelSize;
            for(unsigned int z=0; z<src.depth; z++) 
            {
                for(unsigned int y=0; y<src.height; y++)
                {
                    std::copy(srcptr, srcptr+rowSize, dstptr);
                    srcptr += srcRowPitchBytes;
                    dstptr += dstRowPitchBytes;
                }
                srcptr += srcSliceSkipBytes;
                dstptr += dstSliceSkipBytes;
            }
            return;
        }

        // Is there a specialized, inlined, conversion?
        if(doOptimizedConversion(src, dst))
        {
            // If so, good
            return;
        }

        // The brute force fallback
        float r,g,b,a;
        for(unsigned int z=0; z<src.depth; z++) 
        {
            for(unsigned int y=0; y<src.height; y++)
            {
                for(unsigned int x=0; x<src.width; x++)
                {
                    unpackColour(&r, &g, &b, &a, src.format, srcptr);
                    packColour(r, g, b, a, dst.format, dstptr);
                    srcptr += srcPixelSize; 
                    dstptr += dstPixelSize;
                }
                srcptr += srcRowSkipBytes;
                dstptr += dstRowSkipBytes;
            }
            srcptr += srcSliceSkipBytes;
            dstptr += dstSliceSkipBytes;
        }
    }
    //-----------------------------------------------------------------------
    /*************************************************************************
    * IL specific functions
    */
    PixelFormat ILUtil::ilFormat2OgreFormat( int ImageFormat, int ImageType ) {
        PixelFormat fmt = PF_UNKNOWN;
        switch( ImageFormat )
        {
            /* Compressed formats -- ignore type */
        case IL_DXT1:   fmt = PF_DXT1; break;
        case IL_DXT2:   fmt = PF_DXT2; break;
        case IL_DXT3:   fmt = PF_DXT3; break;
        case IL_DXT4:   fmt = PF_DXT4; break;
        case IL_DXT5:   fmt = PF_DXT5; break;
            /* Normal formats */
#if OGRE_ENDIAN == ENDIAN_BIG
        case IL_RGB:
            fmt = PF_R8G8B8;
            break;
        case IL_BGR:
            fmt = PF_B8G8R8;
            break;            
        case IL_RGBA:
            fmt = PF_A8R8G8B8;// No direct match
            break;
        case IL_BGRA:
            fmt = PF_B8G8R8A8;
            break;
#else
        case IL_RGB:
            fmt = PF_B8G8R8;
            break;
        case IL_BGR:
            fmt = PF_R8G8B8;
            break;            
        case IL_RGBA:
            fmt = PF_A8B8G8R8;
            break;
        case IL_BGRA:
            fmt = PF_A8R8G8B8;
            break;
#endif
        case IL_LUMINANCE:
            switch(ImageType)
            {
            case IL_BYTE:
            case IL_UNSIGNED_BYTE:
                fmt = PF_L8;
                break;
            default:
                fmt = PF_L16;
            }
            break;            
        case IL_LUMINANCE_ALPHA:
            fmt = PF_A4L4;
            break;
        }  
        return fmt;
    }
    //-----------------------------------------------------------------------
    std::pair< int, int > ILUtil::OgreFormat2ilFormat( PixelFormat format )
    {
        switch( format )
        {
        case PF_L8:
        case PF_A8:
            return std::pair< int, int >( IL_LUMINANCE, 1 );
        case PF_R5G6B5:
            return std::pair< int, int >( IL_RGB, 2 );
        case PF_B5G6R5:
            return std::pair< int, int >( IL_BGR, 2 );
        case PF_A4R4G4B4:
            return std::pair< int, int >( IL_RGBA, 2 );
        case PF_B4G4R4A4:
            return std::pair< int, int >( IL_BGRA, 2 );
        case PF_R8G8B8:
            return std::pair< int, int >( IL_RGB, 3 );
        case PF_B8G8R8:
            return std::pair< int, int >( IL_BGR, 3 );
        case PF_A8R8G8B8:
            return std::pair< int, int >( IL_BGRA, 4 );
        case PF_B8G8R8A8: // ?
            return std::pair< int, int >( IL_BGRA, 4 );
        case PF_A8B8G8R8:
            return std::pair< int, int >( IL_RGBA, 4 );
        default:
            return std::pair< int, int >( -1, -1 );
        }
    }
    //-----------------------------------------------------------------------
    template <typename T> void ilToOgreInternal(uint8 *tar, PixelFormat ogrefmt, 
        T r, T g, T b, T a)
    {
        int ilfmt = ilGetInteger( IL_IMAGE_FORMAT );
        T *src = (T*)ilGetData();
        T *srcend = (T*)((uint8*)ilGetData() + ilGetInteger( IL_IMAGE_SIZE_OF_DATA ));
        int elemSize = PixelUtil::getNumElemBytes(ogrefmt);
        while(src < srcend) {
            switch(ilfmt) {
    case IL_RGB:
        r = src[0];
        g = src[1];
        b = src[2];
        src += 3;
        break;
    case IL_BGR:
        b = src[0];
        g = src[1];
        r = src[2];
        src += 3;
        break;
    case IL_LUMINANCE:
        r = src[0];
        g = src[0];
        b = src[0];
        src += 1;
        break;
    case IL_LUMINANCE_ALPHA:
        r = src[0];
        g = src[0];
        b = src[0];
        a = src[1];
        src += 2;
        break;
    case IL_RGBA:
        r = src[0];
        g = src[1];
        b = src[2];
        a = src[3];
        src += 4;
        break;
    case IL_BGRA:
        b = src[0];
        g = src[1];
        r = src[2];
        a = src[3];
        src += 4;
        break;
    default:
        return;
            }
            PixelUtil::packColour(r, g, b, a, ogrefmt, tar);
            tar += elemSize;
        }

    }    
    //-----------------------------------------------------------------------
    void ILUtil::toOgre(uint8 *tar, PixelFormat ogrefmt) 
    {
        int ilfmt = ilGetInteger( IL_IMAGE_FORMAT );
        int iltp = ilGetInteger( IL_IMAGE_TYPE );

        // check for easy fast passthrough for RGBA and RGB formats if in-memory
        // layout matches       
        bool passThrough = false;
        if(iltp == IL_UNSIGNED_BYTE || iltp == IL_BYTE) 
        {
#if OGRE_ENDIAN == ENDIAN_BIG      
            if( (ogrefmt == PF_R8G8B8 && ilfmt == IL_RGB)||
                (ogrefmt == PF_B8G8R8 && ilfmt == IL_BGR)||
                (ogrefmt == PF_B8G8R8A8 && ilfmt == IL_BGRA)||
                (ogrefmt == PF_L8 && ilfmt == IL_LUMINANCE))
#else
            if( (ogrefmt == PF_B8G8R8 && ilfmt == IL_RGB)||
                (ogrefmt == PF_R8G8B8 && ilfmt == IL_BGR)||
                (ogrefmt == PF_A8B8G8R8 && ilfmt == IL_RGBA)||
                (ogrefmt == PF_A8R8G8B8 && ilfmt == IL_BGRA)||
                (ogrefmt == PF_L8 && ilfmt == IL_LUMINANCE))
#endif
            {
                passThrough = true;
            }
        }
        if(iltp == IL_UNSIGNED_SHORT || iltp == IL_SHORT) 
        {
            if( (ogrefmt == PF_L16 && ilfmt == IL_LUMINANCE)) {
                passThrough = true;
            }
        }
        
        if(passThrough)
        {
            std::copy((uint8*)ilGetData(), 
                (uint8*)ilGetData() + ilGetInteger( IL_IMAGE_SIZE_OF_DATA ), 
                tar);
            return;
        }
        
        if(iltp == IL_UNSIGNED_BYTE || iltp == IL_BYTE) 
        {
            ilToOgreInternal(tar, ogrefmt, (uint8)0x00,(uint8)0x00,(uint8)0x00,(uint8)0xFF);
        } 
        else if(iltp == IL_FLOAT)
        {
            ilToOgreInternal(tar, ogrefmt, 0.0f,0.0f,0.0f,1.0f);          
        }
        else 
        {
            Except( Exception::UNIMPLEMENTED_FEATURE,
                "Cannot convert this DevIL type",
                "ilToOgre" ) ;
        }
    }
    //-----------------------------------------------------------------------
    void ILUtil::fromOgre(uint8 *src, PixelFormat ogrefmt, int width, int height, int depth)
    {
        // TODO
    }


}

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
#ifndef _PixelFormat_H__
#define _PixelFormat_H__

#include "OgrePrerequisites.h"

namespace Ogre {
    /** The pixel format used for images, textures, and render surfaces */
    enum PixelFormat
    {
        /// Unknown pixel format.
        PF_UNKNOWN = 0,
        /// 8-bit pixel format, all bits luminace.
        PF_L8 = 1,
        /// 16-bit pixel format, all bits luminace.
        PF_L16 = 2,
        /// 8-bit pixel format, all bits alpha.
        PF_A8 = 3,
        /// 8-bit pixel format, 4 bits alpha, 4 bits luminace.
        PF_A4L4 = 4,
        /// 8-bit pixel format, 4 bits luminace, 4 bits alpha.
        PF_L4A4 = 5,
        /// 16-bit pixel format, 5 bits red, 6 bits green, 5 bits blue.
        PF_R5G6B5 = 6,
        /// 16-bit pixel format, 5 bits blue, 6 bits green, 5 bits red.
        PF_B5G6R5 = 7,
        /// 16-bit pixel format, 4 bits for alpha, red, green and blue.
        PF_A4R4G4B4 = 8,
        /// 16-bit pixel format, 4 bits for blue, green, red and alpha.
        PF_B4G4R4A4 = 9,
        /// 24-bit pixel format, 8 bits for red, green and blue.
        PF_R8G8B8 = 10,
        /// 24-bit pixel format, 8 bits for blue, green and red.
        PF_B8G8R8 = 11,
        /// 32-bit pixel format, 8 bits for alpha, red, green and blue.
        PF_A8R8G8B8 = 12,
        /// 32-bit pixel format, 8 bits for blue, green, red and alpha.
        PF_A8B8G8R8 = 13,
        /// 32-bit pixel format, 8 bits for blue, green, red and alpha.
        PF_B8G8R8A8 = 14,
        /// 32-bit pixel format, 2 bits for alpha, 10 bits for red, green and blue.
        PF_A2R10G10B10 = 15,
        /// 32-bit pixel format, 10 bits for blue, green and red, 2 bits for alpha.
        PF_A2B10G10R10 = 16,
        /// DDS (DirectDraw Surface) DXT1 format
        PF_DXT1 = 17,
        /// DDS (DirectDraw Surface) DXT2 format
        PF_DXT2 = 18,
        /// DDS (DirectDraw Surface) DXT3 format
        PF_DXT3 = 19,
        /// DDS (DirectDraw Surface) DXT4 format
        PF_DXT4 = 20,
        /// DDS (DirectDraw Surface) DXT5 format
        PF_DXT5 = 21,
        // 48-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue
        PF_FP_R16G16B16 = 22,
        // 64-bit pixel format, 16 bits (float) for red, 16 bits (float) for green, 16 bits (float) for blue, 16 bits (float) for alpha
        PF_FP_R16G16B16A16 = 23,
       // 96-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue
        PF_FP_R32G32B32 = 24,
        // 128-bit pixel format, 32 bits (float) for red, 32 bits (float) for green, 32 bits (float) for blue, 32 bits (float) for alpha
        PF_FP_R32G32B32A32 = 25,
        // Number of pixel formats currently defined
        PF_COUNT = 26
    };

    /**
     * Flags defining some on/off properties of pixel formats
     */
    enum PixelFormatFlags {
        // This format has an alpha channel
        PFF_HASALPHA        = 0x00000001,      
        // This format is compressed. This invalidates the values in elemBytes,
        // elemBits and the bit counts as these might not be fixed in a compressed format.
        PFF_COMPRESSED    = 0x00000002,
        // This is a floating point format
        PFF_FLOAT           = 0x00000004,         
        // This is a depth format (for depth textures)
        PFF_DEPTH           = 0x00000008,
        // Format is in native endian. Generally true for the 16, 24 and 32 bits
        // formats which can be represented as machine integers.
        PFF_NATIVEENDIAN    = 0x00000010,
        // This is an intensity format instead of a RGB one. The luminance
        // replaces R,G and B. (but not A)
        PFF_LUMINANCE       = 0x00000020
    };

    /**
     * Some utility functions for packing and unpacking pixel data
     */
    class _OgreExport PixelUtil {
    public:
        /** Returns the size in bytes of an element of the given pixel format.
         @returns
               The size in bytes of an element. See Remarks.
         @remarks
               Passing PF_UNKNOWN will result in returning a size of 0 bytes.
        */
        static int getNumElemBytes( PixelFormat format );

        /** Returns the size in bits of an element of the given pixel format.
          @returns
               The size in bits of an element. See Remarks.
           @remarks
               Passing PF_UNKNOWN will result in returning a size of 0 bits.
        */
        static int getNumElemBits( PixelFormat format );

        /** Returns the property flags for this pixel format
          @returns
               A bitfield combination of PFF_HASALPHA, PFF_ISCOMPRESSED,
               PFF_FLOAT, PFF_DEPTH, PFF_NATIVEENDIAN, PFF_INTENSITY
          @remarks
               This replaces the seperate functions for formatHasAlpha, formatIsFloat, ...
        */
        static unsigned int getFlags( PixelFormat format );

        /** Gives the number of bits (RGBA) for a format. See remarks.          
          @remarks      For non-colour formats (dxt, depth) this returns [0,0,0,0].
        */
        static void getBitDepths(PixelFormat format, int rgba[4]);

        /** Gets the name of an image format
        */
        static String getFormatName(PixelFormat srcformat);

        /** Returns wether the format can be packed or unpacked with the packColour()
        and unpackColour() functions. This is generally not true for compessed and
        depth formats as they are special. It can only be true for formats with a
        fixed element size.
          @returns 
               true if yes, otherwise false
        */
        static bool isAccessible(PixelFormat srcformat);
        
        /* Pack a colour value to memory location dest */
        static void packColour(const ColourValue &colour, PixelFormat pf,  void* dest);
        static void packColour(uint8 r, uint8 g, uint8 b, uint8 a, PixelFormat pf,  void* dest);
        // 32 bit fixed point
        //static void packColour(uint32 r, uint32 g, uint32 b, uint32 a, PixelFormat pf,  void* dest);        
        // 32 bit floating point
        static void packColour(float r, float g, float b, float a, PixelFormat pf,  void* dest); 

        /* Unpack a colour value from memory location src */        
        static void unpackColour(ColourValue *colour, PixelFormat pf,  const void* src);
        static void unpackColour(uint8 *r, uint8 *g, uint8 *b, uint8 *a, PixelFormat pf,  const void* src);
        static void unpackColour(float *r, float *g, float *b, float *a, PixelFormat pf,  const void* src); 
        
        /* Convert pixels from one format to another */
        static void bulkPixelConversion(void *src, PixelFormat srcFormat, void *dest, PixelFormat dstFormat, unsigned int count);
    };

    /* 
     * DevIL specific utility class
     **/    
    class ILUtil {
    public:
        /**
         * Get OGRE format to which a given IL format can be most optimally converted.
         */
        static PixelFormat ilFormat2OgreFormat( int ImageFormat, int ImageType );
        /**
         * Get best IL format to convert a given OGRE format to.
         */
        static std::pair< int, int > OgreFormat2ilFormat( PixelFormat format );      
        /**
         * Convert current IL image to an OGRE format. The size of the target will be
         * PixelUtil::getNumElemBytes(fmt) * ilGetInteger( IL_IMAGE_WIDTH ) * ilGetInteger( IL_IMAGE_HEIGHT ) * ilGetInteger( IL_IMAGE_DEPTH )
         * The IL image type must be IL(_UNSIGNED_)BYTE or IL_FLOAT.
         * The IL image format must be IL_RGBA, IL_BGRA, IL_RGB, IL_BGR, IL_LUMINANCE or IL_LUMINANCE_ALPHA
         * 
         * @param tar       Target pointer
         * @param ogrefmt   Ogre pixel format to employ
         */
        static void toOgre(uint8 *tar, PixelFormat ogrefmt);

        /**
         * Convert an OGRE format image to current IL image.
         * @param src       Source pointer
         * @param ogrefmt   Ogre pixel format to employ
         * @param width     Image width (x)
         * @param height    Image height (y)
         * @param depth     Image depth (z)
         */
        static void fromOgre(uint8 *src, PixelFormat ogrefmt, int width, int height, int depth);
    };

}

#endif

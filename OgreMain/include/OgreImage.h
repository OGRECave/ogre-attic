/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#ifndef _Image_H__
#define _Image_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgrePixelFormat.h"
#include "OgreDataStream.h"

namespace Ogre {

    enum ImageFlags
    {
        IF_COMPRESSED = 0x00000001,
        IF_CUBEMAP    = 0x00000002,
        IF_3D_TEXTURE = 0x00000004
    };
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
        /** Structure used to define a rectangle in a 2-D integer space.
        */
        struct Rect
        {
            long left, top, right, bottom;

            Rect()
            {
            }
            Rect( long l, long t, long r, long b )
            {
                left = l;
                top = t;   
                right = r;
                bottom = b;                
            }
            Rect& operator = ( const Rect& other )
            {
                left = other.left;
                top = other.top;
                right = other.right;
                bottom = other.bottom;       

                return *this;
            }
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
        virtual ~Image();

        /** Assignment operator - copies all the data from the target image.
        */
        Image & operator = ( const Image & img );

        /** Flips (mirrors) the image around the Y-axis. 
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
        Image & flipAroundY();

        /** Flips (mirrors) the image around the X-axis.
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
        Image & flipAroundX();

        /** Stores a pointer to raw data in memory. The pixel format has to be specified.
            @remarks
                This method loads an image into memory held in the object. The 
                pixel format will be either greyscale or RGB with an optional
                Alpha component.
                The type can be determined by calling getFormat().             
            @param
                The data pointer
            @param
				Width of image
            @param
				Height of image
            @param
				Pixel Format
            @note
                The memory associated with this buffer is NOT destroyed with the
                Image object.
        */
		Image& loadDynamicImage( uchar* pData, ushort uWidth, 
								 ushort uHeight, PixelFormat eFormat );

        /** Loads raw data from a stream. The pixel format has to be specified.
        */
        Image & loadRawData( 
            DataStreamPtr& stream, 
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
            @param
                groupName Name of the resource group to search for the image
            @note
                The memory associated with this buffer is destroyed with the
                Image object.
        */
        Image & load( const String& strFileName, const String& groupName );

        /** Loads an image file from a stream.
            @remarks
                This method works in the same way as the filename-based load 
                method except it loads the image from a DataStream object. 
				This DataStream is expected to contain the 
                encoded data as it would be held in a file. 
            @param
                stream The source data.
            @param
                type The type of the image. Used to decide what decompression
                codec to use.
            @see
                Image::load( const String& strFileName )
        */
        Image & load(DataStreamPtr& stream, const String& type );
        
        /** Save the image as a file. */
        void save(const String& filename);

        /** Returns a pointer to the internal image buffer.
        */
        uchar* getData(void);

        /** Returns a const pointer to the internal image buffer.
        */
        const uchar * getData() const;       

        /** Returns the size of the data buffer.
        */
        size_t getSize() const;

        /** Returns the number of mipmaps contained in the image.
        */
        unsigned short getNumMipmaps() const;

        /** Returns true if the image has the appropriate flag set.
        */
        bool hasFlag(const ImageFlags imgFlag) const;

        /** Gets the width of the image in pixels.
        */
        ushort getWidth(void) const;

        /** Gets the height of the image in pixels.
        */
        ushort getHeight(void) const;

        /** Gets the depth of the image.
        */
        ushort getDepth(void) const;

        /** Gets the physical width in bytes of each row of pixels.
        */
        ushort getRowSpan(void) const;

        /** Returns the image format.
        */
        PixelFormat getFormat() const;

        /** Returns the number of bits per pixel.
        */
        uchar getBPP() const;

        /** Returns true if the image has an alpha component.
        */
        bool getHasAlpha() const;

        /** Does gamma adjustment.
            @note
                Basic algo taken from Titan Engine, copyright (c) 2000 Ignacio 
                Castano Iguado
        */
        static void applyGamma( uchar *buffer, Real gamma, size_t size, uchar bpp );
        
        /**
         * Get colour value from a certain location in the image. The z coordinate
         * is only valid for cubemaps and volume textures. This uses the first (largest)
         * mipmap.
         */
        void getColourAt(ColourValue *out, int x, int y, int z);
        
        /**
         * Get a PixelBox encapsulating the image data of a mipmap
         */
        PixelBox getPixelBox(int mipmap);

		enum Filter
		{
			FILTER_NEAREST,
			FILTER_LINEAR,
			FILTER_BILINEAR,
			FILTER_BOX,
			FILTER_TRIANGLE,
			FILTER_BICUBIC
		};
		/** Resize a 2D image, applying the appropriate filter. */
		void resize(ushort width, ushort height, Filter filter = FILTER_BILINEAR);
		

    private:
        // The width of the image in pixels
        ushort m_uWidth;
        // The height of the image in pixels
        ushort m_uHeight;
        // The depth of the image
        ushort m_uDepth;
        // The size of the image buffer
        uint m_uSize;
        // The number of mipmaps the image contains
        ushort m_uNumMipmaps;
        // Image specific flags.
        int m_uFlags;

        // The pixel format of the image
        PixelFormat m_eFormat;

        // The number of bytes per pixel
        uchar m_ucPixelSize;
        uchar* m_pBuffer;

		// A bool to determine if we delete the buffer or the calling app does
		bool m_bAutoDelete;
    };

} // namespace

#endif

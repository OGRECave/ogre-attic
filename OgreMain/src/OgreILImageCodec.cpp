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

#include "OgreStableHeaders.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreILImageCodec.h"
#include "OgreImage.h"
#include "OgreException.h"

#include <IL/il.h>
#include <IL/ilu.h>

namespace Ogre {

    bool ILImageCodec::_is_initialised = false;    

    //---------------------------------------------------------------------
    void ILImageCodec::code( const DataChunk& input, DataChunk* output, ... ) const
    {        
        OgreGuard( "ILCodec::code" );

        Except(Exception::UNIMPLEMENTED_FEATURE, "code to memory not implemented",
            "ILCodec::code");

        OgreUnguard();

    }


    //---------------------------------------------------------------------
    void ILImageCodec::codeToFile( const DataChunk& input, 
        const String& outFileName, Codec::CodecData* pData) const
    {
        OgreGuard( "ILImageCodec::codeToFile" );

        ILuint ImageName;

        ilGenImages( 1, &ImageName );
        ilBindImage( ImageName );

        ImageData* pImgData = static_cast< ImageData * >( pData );
        std::pair< int, int > fmt_bpp = OgreFormat2ilFormat( pImgData->format );
        ilTexImage( 
            pImgData->width, pImgData->height, 1, fmt_bpp.second, fmt_bpp.first, IL_UNSIGNED_BYTE, 
            static_cast< void * >( const_cast< uchar * >( ( input.getPtr() ) ) ) );
        iluFlipImage();

        // Implicitly pick DevIL codec
        ilSaveImage(const_cast< char * >( outFileName.c_str() ) );

        ilDeleteImages(1, &ImageName);

        OgreUnguard();
    }
    //---------------------------------------------------------------------
    Codec::CodecData * ILImageCodec::decode( const DataChunk& input, DataChunk* output, ... ) const
    {
        OgreGuard( "ILImageCodec::decode" );

        // DevIL variables
        ILuint ImageName;
        ILint ImageFormat, BytesPerPixel;
        ImageData * ret_data = new ImageData;

        // Load the image
        ilGenImages( 1, &ImageName );
        ilBindImage( ImageName );

        // Put it right side up
        ilEnable(IL_ORIGIN_SET);
        ilSetInteger(IL_ORIGIN_MODE, IL_ORIGIN_UPPER_LEFT);

        // Keep DXTC(compressed) data if present
        ilSetInteger(IL_KEEP_DXTC_DATA, IL_TRUE);

        // Load image from disk
        ilLoadL( 
            getILType(), 
            ( void * )const_cast< uchar * >( input.getPtr() ), 
            static_cast< ILuint >( input.getSize() ) );

        // Check if everything was ok
        ILenum PossibleError = ilGetError() ;
        if( PossibleError != IL_NO_ERROR ) {
            Except( Exception::UNIMPLEMENTED_FEATURE,
                "IL Error",
                iluErrorString(PossibleError) ) ;
        }

        // Format conversion to RGB or RGBA
        ImageFormat = ilGetInteger( IL_IMAGE_FORMAT );
        if(ImageFormat==IL_BGR || ImageFormat==IL_BGRA) {
            // New image format
            if(ImageFormat==IL_BGR)
                ImageFormat = IL_RGB;
            else
                ImageFormat = IL_RGBA;

            // Doing this with IL_FORMAT_SET/IL_FORMAT_MODE would have
            // to be done before loading, and would always
            // produce images with alpha channel.
            iluSwapColours();
        }

        // Now sets some variables
        BytesPerPixel = ilGetInteger( IL_IMAGE_BYTES_PER_PIXEL ); 

        ret_data->format = ilFormat2OgreFormat( ImageFormat, BytesPerPixel );
        ret_data->width = ilGetInteger( IL_IMAGE_WIDTH );
        ret_data->height = ilGetInteger( IL_IMAGE_HEIGHT );
        ret_data->depth = ilGetInteger( IL_IMAGE_DEPTH );
        ret_data->num_mipmaps = ilGetInteger ( IL_NUM_MIPMAPS );
        ret_data->flags = 0;

        // Check for cubemap
        ILuint cubeflags = ilGetInteger ( IL_IMAGE_CUBEFLAGS );
        if(cubeflags)
            ret_data->flags |= IF_CUBEMAP;

        // Keep DXT data (if present at all)
        ILuint dxtFormat = ilGetInteger( IL_DXTC_DATA_FORMAT );
        if(dxtFormat != IL_DXT_NO_COMP && Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability( RSC_TEXTURE_COMPRESSION_DXT ))
        {
            ILuint dxtSize = ilGetDXTCData(NULL, 0, dxtFormat);
            output->allocate( dxtSize );
            ilGetDXTCData(output->getPtr(), dxtSize, dxtFormat);

            ret_data->size = dxtSize;
            ret_data->format = ilFormat2OgreFormat( dxtFormat, BytesPerPixel );
            ret_data->flags |= IF_COMPRESSED;
        }
        else
        {
            uint numImagePasses = cubeflags ? 6 : 1;
            uint imageSize = ilGetInteger(IL_IMAGE_SIZE_OF_DATA);
            output->allocate( imageSize * numImagePasses );

            unsigned int i = 0, offset = 0;
            for(i = 0; i < numImagePasses; i++)
            {
                if(cubeflags)
                {
                    ilBindImage(ImageName);
                    ilActiveImage(i);
                }

                // Move the image data to the output buffer
                memcpy( output->getPtr() + offset, ilGetData(), imageSize );
                offset += imageSize;
            }

            ret_data->size = imageSize * numImagePasses;
            ret_data->format = ilFormat2OgreFormat( ImageFormat, BytesPerPixel );
        }

        // Restore IL state
        ilDisable(IL_ORIGIN_SET);
        ilDisable(IL_FORMAT_SET);

        ilDeleteImages( 1, &ImageName );

        OgreUnguardRet( ret_data );
    }
    //---------------------------------------------------------------------
    void ILImageCodec::initialiseIL(void)
    {
        if( !_is_initialised )
        {
            ilInit();
            ilEnable( IL_FILE_OVERWRITE );
            _is_initialised = true;
        }
    }
}

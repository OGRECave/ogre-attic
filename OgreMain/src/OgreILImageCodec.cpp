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

#include "OgreILImageCodec.h"
#include "OgreImage.h"
#include "OgreException.h"

#include <IL/il.h>
#include <IL/ilu.h>

namespace Ogre {

	bool ILImageCodec::_is_initialised = false;    
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
		ILint Imagformat, BytesPerPixel;
		ImageData * ret_data = new ImageData;

		// Load the image
		ilGenImages( 1, &ImageName );
		ilBindImage( ImageName );

		ilLoadL( 
			getILType(), 
			( void * )const_cast< uchar * >( input.getPtr() ), 
			static_cast< ILuint >( input.getSize() ) );

                // Check if everything was ok
                ILenum PossibleError = ilGetError() ;
                if( PossibleError != IL_NO_ERROR )
                {
                        Except( Exception::UNIMPLEMENTED_FEATURE,
                                        "IL Error",
                                        iluErrorString(PossibleError) ) ;
                }

		// Now sets some variables
		ret_data->width = ilGetInteger( IL_IMAGE_WIDTH );
		ret_data->height = ilGetInteger( IL_IMAGE_HEIGHT );

		Imagformat = ilGetInteger( IL_IMAGE_FORMAT );
		BytesPerPixel = ilGetInteger( IL_IMAGE_BYTES_PER_PIXEL ); 

		ret_data->format = ilFormat2OgreFormat( Imagformat, BytesPerPixel );
		ret_data->width = ilGetInteger( IL_IMAGE_WIDTH );
		ret_data->height = ilGetInteger( IL_IMAGE_HEIGHT );

		uint ImageSize = ilGetInteger( IL_IMAGE_WIDTH ) * ilGetInteger( IL_IMAGE_HEIGHT ) * ilGetInteger( IL_IMAGE_BYTES_PER_PIXEL );

		// Move the image data to the output buffer
		output->allocate( ImageSize );
		memcpy( output->getPtr(), ilGetData(), ImageSize );

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

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
#include "OgreStableHeaders.h"
#include "OgreTGACodec.h"
#include "OgreImage.h"
#include "OgreException.h"

#include <IL/il.h>
#include <IL/ilu.h>

namespace Ogre {
    //---------------------------------------------------------------------
    void TGACodec::code( const DataChunk& input, DataChunk* output, ... ) const
    {
        OgreGuard( "TGACodec::code" );
        Except(Exception::UNIMPLEMENTED_FEATURE, "code to memory not implemented",
            "TGACodec::code");
        OgreUnguard();
    }
    //---------------------------------------------------------------------
    Codec::CodecData * TGACodec::decode( const DataChunk& input, DataChunk* output, ... ) const
    {
        OgreGuard( "TGACodec::decode" );

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
        ILenum PossibleError = ilGetError();
        if( PossibleError != IL_NO_ERROR )
        {
            Except( Exception::UNIMPLEMENTED_FEATURE,
                    "IL Error",
                    iluErrorString(PossibleError) );
        }

		Imagformat = ilGetInteger( IL_IMAGE_FORMAT );
		BytesPerPixel = ilGetInteger( IL_IMAGE_BYTES_PER_PIXEL ); 

		// Now sets some variables
		ret_data->width = ilGetInteger( IL_IMAGE_WIDTH );
		ret_data->height = ilGetInteger( IL_IMAGE_HEIGHT );

		uint ImageSize = ilGetInteger( IL_IMAGE_WIDTH ) * ilGetInteger( IL_IMAGE_HEIGHT ) * ilGetInteger( IL_IMAGE_BYTES_PER_PIXEL );
		
		output->allocate( ImageSize );
		
		//check to see whether the image is upside down
		if(ilGetInteger(IL_ORIGIN_MODE) == IL_ORIGIN_LOWER_LEFT)
		{
			//if so (probably) put it right side up
			ilEnable(IL_ORIGIN_SET);
			ilSetInteger(IL_ORIGIN_MODE, IL_ORIGIN_UPPER_LEFT);
		}
        if( Imagformat==IL_BGR || Imagformat==IL_BGRA)
        {
            //if so (probably) reverse the b and the r, this is slower but at least it works
            ILint newIF = Imagformat==IL_BGR ? IL_RGB : IL_RGBA;
            ilCopyPixels(0, 0, 0, ret_data->width , ret_data->height, 1, newIF, IL_UNSIGNED_BYTE, output->getPtr());
            Imagformat = newIF;
        }
        else
        {
            memcpy( output->getPtr(), ilGetData(), ImageSize );
        }

		ret_data->format = ilFormat2OgreFormat( Imagformat, BytesPerPixel );
		memcpy( output->getPtr(), ilGetData(), ImageSize );

		ilDeleteImages( 1, &ImageName );

		OgreUnguardRet( ret_data );
    }
    //---------------------------------------------------------------------
    unsigned int TGACodec::getILType(void) const
    { 
        return IL_TGA;
    }
}

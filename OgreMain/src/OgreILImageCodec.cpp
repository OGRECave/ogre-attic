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

    ILImageCodec::ILImageCodec(const String &type, unsigned int ilType):
        mType(type),
        mIlType(ilType)
    { 
        initialiseIL();
    }

    //---------------------------------------------------------------------
    DataStreamPtr ILImageCodec::code(MemoryDataStreamPtr& input, Codec::CodecDataPtr& pData) const
    {        
        OgreGuard( "ILCodec::code" );

        Except(Exception::UNIMPLEMENTED_FEATURE, "code to memory not implemented",
            "ILCodec::code");

        OgreUnguard();

    }
    //---------------------------------------------------------------------
    void ILImageCodec::codeToFile(MemoryDataStreamPtr& input, 
        const String& outFileName, Codec::CodecDataPtr& pData) const
    {
        OgreGuard( "ILImageCodec::codeToFile" );

        ILuint ImageName;

        ilGenImages( 1, &ImageName );
        ilBindImage( ImageName );

        ImageData* pImgData = static_cast< ImageData * >( pData.getPointer() );
        std::pair< int, int > fmt_bpp = ILUtil::OgreFormat2ilFormat( pImgData->format );

        ilTexImage( 
            pImgData->width, pImgData->height, 1, fmt_bpp.second, fmt_bpp.first, 
            IL_UNSIGNED_BYTE, input->getPtr());
        iluFlipImage();

        // Implicitly pick DevIL codec
        ilSaveImage(const_cast< char * >( outFileName.c_str() ) );

        ilDeleteImages(1, &ImageName);

        OgreUnguard();
    }
    //---------------------------------------------------------------------
    Codec::DecodeResult ILImageCodec::decode(DataStreamPtr& input) const
    {
        OgreGuard( "ILImageCodec::decode" );

        // DevIL variables
        ILuint ImageName;

        ILint ImageFormat, BytesPerPixel, ImageType;
        ImageData* imgData = new ImageData();
        MemoryDataStreamPtr output;

        // Load the image
        ilGenImages( 1, &ImageName );
        ilBindImage( ImageName );

        // Put it right side up
        ilEnable(IL_ORIGIN_SET);
        ilSetInteger(IL_ORIGIN_MODE, IL_ORIGIN_UPPER_LEFT);

        // Keep DXTC(compressed) data if present
        ilSetInteger(IL_KEEP_DXTC_DATA, IL_TRUE);

        // Load image from stream, cache into memory
        MemoryDataStream memInput(input);
        ilLoadL( 
            mIlType, 
            memInput.getPtr(), 
            static_cast< ILuint >(memInput.size()));

        // Check if everything was ok
        ILenum PossibleError = ilGetError() ;
        if( PossibleError != IL_NO_ERROR ) {
            Except( Exception::UNIMPLEMENTED_FEATURE,
                "IL Error",
                iluErrorString(PossibleError) ) ;
        }

        ImageFormat = ilGetInteger( IL_IMAGE_FORMAT );
        ImageType = ilGetInteger( IL_IMAGE_TYPE );

        // Convert image if ImageType is != IL_*BYTE and != IL_FLOAT
        if(ImageType != IL_BYTE && ImageType != IL_UNSIGNED_BYTE && ImageType != IL_FLOAT) {
            ilConvertImage(ImageFormat, IL_FLOAT);
        }

        // Now sets some variables
        BytesPerPixel = ilGetInteger( IL_IMAGE_BYTES_PER_PIXEL ); 

        imgData->format = ILUtil::ilFormat2OgreFormat( ImageFormat, ImageType );
        imgData->width = ilGetInteger( IL_IMAGE_WIDTH );
        imgData->height = ilGetInteger( IL_IMAGE_HEIGHT );
        imgData->depth = ilGetInteger( IL_IMAGE_DEPTH );
        imgData->num_mipmaps = ilGetInteger ( IL_NUM_MIPMAPS );
        imgData->flags = 0;

        // Check for cubemap
        ILuint cubeflags = ilGetInteger ( IL_IMAGE_CUBEFLAGS );
        if(cubeflags)
            imgData->flags |= IF_CUBEMAP;

        // Keep DXT data (if present at all)
        ILuint dxtFormat = ilGetInteger( IL_DXTC_DATA_FORMAT );
        if(dxtFormat != IL_DXT_NO_COMP && Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability( RSC_TEXTURE_COMPRESSION_DXT ))
        {
            ILuint dxtSize = ilGetDXTCData(NULL, 0, dxtFormat);
            output.bind(new MemoryDataStream(dxtSize));
            ilGetDXTCData(output->getPtr(), dxtSize, dxtFormat);

            imgData->size = dxtSize;
            imgData->format = ILUtil::ilFormat2OgreFormat( dxtFormat, ImageType );
            imgData->flags |= IF_COMPRESSED;
        }
        else
        {
            uint numImagePasses = cubeflags ? 6 : 1;
            uint imageSize = PixelUtil::getNumElemBytes(imgData->format) * ilGetInteger( IL_IMAGE_WIDTH ) * ilGetInteger( IL_IMAGE_HEIGHT ) * ilGetInteger( IL_IMAGE_DEPTH );
            output.bind(new MemoryDataStream(imageSize * numImagePasses));

            unsigned int i = 0, offset = 0;
            for(i = 0; i < numImagePasses; i++)
            {
                if(cubeflags)
                {
                    ilBindImage(ImageName);
                    ilActiveImage(i);
                }

                ILUtil::toOgre((unsigned char*)output->getPtr()+offset, imgData->format);

                offset += imageSize;
            }

            imgData->size = imageSize * numImagePasses;
        }

        // Restore IL state
        ilDisable(IL_ORIGIN_SET);
        ilDisable(IL_FORMAT_SET);

        ilDeleteImages( 1, &ImageName );

        DecodeResult ret;
        ret.first = output;
        ret.second = CodecDataPtr(imgData);


        OgreUnguardRet( ret );
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
    //---------------------------------------------------------------------    
    String ILImageCodec::getType() const 
    {
        return mType;
    }
}

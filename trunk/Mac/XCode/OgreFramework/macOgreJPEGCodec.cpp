/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General  License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General  License for more details.

You should have received a copy of the GNU Lesser General  License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/

#include <ApplicationServices/ApplicationServices.h>

#include "OgrePlatform.h"
#include "OgreStdHeaders.h"

#include "OgreJPEGCodec.h"
#include "OgreImage.h"
#include "OgreException.h"

#include "macDataProvider.h"

namespace Ogre {


    //---------------------------------------------------------------------
    void JPEGCodec::code( const DataChunk& input, DataChunk* output, ... ) const
    {
        OgreGuard( "JPEGCodec::code" );

        Except(Exception::UNIMPLEMENTED_FEATURE, "code to memory not implemented",
            "JPEGCodec::code");

        OgreUnguard();
    }
    
    //---------------------------------------------------------------------
    Codec::CodecData * JPEGCodec::decode( const DataChunk& input, DataChunk* output, ... ) const
    {
        OgreGuard( "JPEGCodec::decode" );
        ImageData * ret_data = new ImageData;
    
        CGDataProviderCallbacks providerCallbacks;
        providerCallbacks.getBytes = mac_CGgetBytes;
        providerCallbacks.skipBytes = mac_CGskipBytes;
        providerCallbacks.rewind = mac_CGrewind;
        providerCallbacks.releaseProvider = mac_CGreleaseProvider;
    
        CGDataProviderRef data = CGDataProviderCreate((void*)&input , &providerCallbacks);
    
        CGImageRef image = CGImageCreateWithJPEGDataProvider(data, NULL, false, kCGRenderingIntentDefault);
        if(image == NULL) {
            CFRelease ( data );
            Except ( Exception::UNIMPLEMENTED_FEATURE,
                    "Image loading error",
                    "Could not initalize JPEG loader.");
        }
    
        ret_data->width = CGImageGetHeight(image);
        ret_data->height = CGImageGetWidth(image);
    
        uint components;
        CGColorSpaceRef colorSpace;
        CGImageAlphaInfo alphaInfo;
    
        if(CGColorSpaceGetNumberOfComponents(CGImageGetColorSpace(image)) == 1) {
            ret_data->format = PF_L8;
            components = 1;
            colorSpace = CGColorSpaceCreateDeviceGray();
            alphaInfo = kCGImageAlphaNone;
        }
        else {
            ret_data->format = PF_A8R8G8B8;
            components = 4;
            colorSpace = CGColorSpaceCreateDeviceRGB();
            alphaInfo = kCGImageAlphaPremultipliedLast;
        }
    
        uint bitsPerComponent = 8;
        uint bytesPerRow = ret_data->width * components;
        uint imageSize = bytesPerRow * ret_data->height;
    
        output->allocate(imageSize);
    
        CGContextRef destContext = CGBitmapContextCreate(output->getPtr(), ret_data->width, ret_data->height,
                                                        bitsPerComponent, bytesPerRow, colorSpace,
                                                        alphaInfo);
        if(destContext == NULL) {
            CFRelease ( colorSpace );
            CFRelease ( image );
            CFRelease ( data );
            Except ( Exception::UNIMPLEMENTED_FEATURE,
                    "Image loading error",
                    "Could not create bitmap context.");
        }
    
        CGRect destRect = CGRectMake(0, 0, ret_data->width, ret_data->height);
        CGContextDrawImage(destContext, destRect, image);
    
        CFRelease( destContext );
        CFRelease( colorSpace );
        CFRelease( image );
        CFRelease( data );
    
    
        OgreUnguardRet( ret_data );
    }
    
    unsigned int JPEGCodec::getILType(void) const
    { return 0; }
}
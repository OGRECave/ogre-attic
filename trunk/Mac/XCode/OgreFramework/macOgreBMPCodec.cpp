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

#include "OgreBMPCodec.h"
#include "OgreImage.h"
#include "OgreException.h"

#include "macDataProvider.h"

namespace Ogre {

    //---------------------------------------------------------------------
    void BMPCodec::code( const DataChunk& input, DataChunk* output, ... ) const
    {
        OgreGuard( "BMPCodec::code" );

        Except(Exception::UNIMPLEMENTED_FEATURE, "code to memory not implemented",
            "BMPCodec::code");

        OgreUnguard();
    }
    
    //---------------------------------------------------------------------
    Codec::CodecData * BMPCodec::decode( const DataChunk& input, DataChunk* output, ... ) const
    {
        OgreGuard( "BMPCodec::decode" );
        
        Except(Exception::UNIMPLEMENTED_FEATURE, "BMP decoding is not yet implemented", "BMPCodec::decode");
    
        OgreUnguard();
    }
    
    unsigned int BMPCodec::getILType(void) const
    { return 0; }
}
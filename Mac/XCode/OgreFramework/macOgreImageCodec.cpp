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

#include "OgreImageCodec.h"
#include "OgreException.h"

namespace Ogre {

//	bool ImageCodec::_is_initialized = false;    
    //---------------------------------------------------------------------
    void ImageCodec::codeToFile( const DataChunk& input, 
        const String& outFileName, Codec::CodecData* pData) const
    {
        OgreGuard( "ImageCodec::codeToFile" );
        Except(Exception::UNIMPLEMENTED_FEATURE, "code to file not implemented",
                "ImageCodec::codeToFile");
        OgreUnguard();
    }
    
    Codec::CodecData* ImageCodec::decode(const DataChunk& input, DataChunk* ouput, ...) const
    {
        Except(Exception::ERR_INTERNAL_ERROR, "ImageCodec::decode was called directly", "ImageCodec::decode");
        return NULL;
    }
    
    void ImageCodec::initialiseIL(void)
    {
        return;
    }
}

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

#include "OgreImage.h"
#include "OgreCodec.h"
#include "OgreException.h"
#include "OgreILImageCodec.h"
#include "OgreILCodecs.h"

#include <IL/il.h>
#include <IL/ilu.h>

namespace Ogre {

    Codec *ILCodecs::mPNGCodec;
    Codec *ILCodecs::mJPGCodec;
    Codec *ILCodecs::mJPEGCodec;
    Codec *ILCodecs::mTGACodec;
    Codec *ILCodecs::mDDSCodec;
    Codec *ILCodecs::mBMPCodec;

    void ILCodecs::registerCodecs(void) {
        mPNGCodec = new PNGCodec;
        Codec::registerCodec( mPNGCodec );
        mJPEGCodec = new JPEGCodec;
        Codec::registerCodec( mJPEGCodec );
        mTGACodec = new TGACodec;
        Codec::registerCodec( mTGACodec );
        mDDSCodec = new DDSCodec;
        Codec::registerCodec( mDDSCodec );
        mJPGCodec = new JPGCodec;
        Codec::registerCodec( mJPGCodec );
        mBMPCodec = new BMPCodec;
        Codec::registerCodec( mBMPCodec );
    }

    void ILCodecs::deleteCodecs(void) {
        delete mBMPCodec;
        delete mDDSCodec;
        delete mTGACodec;
        delete mJPGCodec;
        delete mJPEGCodec;
        delete mPNGCodec;
    }

    //---------------------------------------------------------------------
    unsigned int BMPCodec::getILType(void) const
    { 
        return IL_BMP;
    }

    //---------------------------------------------------------------------
    unsigned int TGACodec::getILType(void) const
    { 
        return IL_TGA;
    }

    //---------------------------------------------------------------------
    unsigned int JPEGCodec::getILType(void) const
    { 
        return IL_JPG;
    }

    //---------------------------------------------------------------------
    unsigned int DDSCodec::getILType(void) const
    {
        return IL_DDS;
    }

    //---------------------------------------------------------------------
    unsigned int PNGCodec::getILType(void) const
    {
        return IL_PNG;
    }

}


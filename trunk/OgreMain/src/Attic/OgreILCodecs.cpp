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
#include "OgreLogManager.h"
#include "OgreILImageCodec.h"
#include "OgreILCodecs.h"

#include <IL/il.h>
#include <IL/ilu.h>

namespace Ogre {
    std::list<ILImageCodec*> ILCodecs::codeclist;

    void ILCodecs::registerCodecs(void) {
        const char *il_extensions = ilGetString ( IL_LOAD_EXT );
        LogManager::getSingleton().logMessage(
         LML_NORMAL,
            "DevIL image formats: %s", il_extensions);
        std::stringstream ext;
        String str;
        ext << il_extensions;
        while(ext >> str)
        {
            ILImageCodec *codec = new ILImageCodec(str, IL_TYPE_UNKNOWN);
            Codec::registerCodec(codec);
            codeclist.push_back(codec);
        }
    }

    void ILCodecs::deleteCodecs(void) {
        for(std::list<ILImageCodec*>::const_iterator i = codeclist.begin(); i != codeclist.end(); ++i)
        {
            Codec::unRegisterCodec(*i);   
            delete *i;
        }
        codeclist.clear();
    }

    //---------------------------------------------------------------------
    /*
        return IL_BMP;
        return IL_TGA;
        return IL_JPG;
        return IL_DDS;
        return IL_PNG;
    */

}


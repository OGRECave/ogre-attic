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
#include <IL/devil_internal_exports.h>

namespace Ogre {
    std::list<ILImageCodec*> ILCodecs::codeclist;

    void ILCodecs::registerCodecs(void) {
		const char *il_version = ilGetString ( IL_VERSION_NUM );
		LogManager::getSingleton().logMessage(
         LML_NORMAL,
            "DevIL version: " + String(il_version));
        const char *il_extensions = ilGetString ( IL_LOAD_EXT );
        
        std::stringstream ext;
        String str, all;
        ext << il_extensions;
        while(ext >> str)
        {
			String fileName = "dummy." + str;
			int ilType = ilTypeFromExt(const_cast<char*>(fileName.c_str()));
            ILImageCodec *codec = new ILImageCodec(str, ilType);
            Codec::registerCodec(codec);
            codeclist.push_back(codec);
			all += str+String("(")+StringConverter::toString(ilType)+String(") ");
        }
		// Raw format, missing in image formats string
		ILImageCodec *cod = new ILImageCodec("raw", IL_RAW);
		Codec::registerCodec(cod);
		codeclist.push_back(cod);
		all += String("raw")+"("+StringConverter::toString(IL_RAW)+String(") ");
		LogManager::getSingleton().logMessage(
         LML_NORMAL,
            "DevIL image formats: " + all);
    }

    void ILCodecs::deleteCodecs(void) {
        for(std::list<ILImageCodec*>::const_iterator i = codeclist.begin(); i != codeclist.end(); ++i)
        {
            Codec::unRegisterCodec(*i);   
            delete *i;
        }
        codeclist.clear();
    }

}


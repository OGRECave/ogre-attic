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
#include "OgreCodec.h"
#include "OgreException.h"


namespace Ogre {

    std::map< String, Codec * > Codec::ms_mapCodecs;


    Codec* Codec::getCodec(const String& extension)
    {
        String lwrcase = extension;
        CodecList::const_iterator i = ms_mapCodecs.find(lwrcase.toLowerCase());
        if (i == ms_mapCodecs.end())
        {
            Except(Exception::ERR_ITEM_NOT_FOUND, 
                "Cannot find codec for extension " + extension,
                "Codec::getCodec");
        }

        return i->second;

    }

}

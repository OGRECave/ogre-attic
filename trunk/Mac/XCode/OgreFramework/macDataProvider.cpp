/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
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

#include "OgrePrerequisites.h"
#include "OgreDataChunk.h"

#include "macDataProvider.h"

//This is the implementation for a CFDataProvider of image data from 
//an Ogre DataChunk

namespace Ogre {

    size_t mac_CGgetBytes(void *chunk, void *buffer, size_t count) {
        DataChunk *dataChunk = (DataChunk*) chunk;
        return (size_t) dataChunk->read(buffer, count);
    }
    
    void mac_CGskipBytes(void *chunk, size_t count) {
        DataChunk *dataChunk = (DataChunk*) chunk;
        dataChunk->skip(count);
    }
    
    void mac_CGrewind(void *chunk) {
        DataChunk *dataChunk = (DataChunk*) chunk;
        dataChunk->seek(0);
    }
    
    void mac_CGreleaseProvider(void *chunk) {
        //nothing to do, this provider allocates no memory
    }
    
    
}
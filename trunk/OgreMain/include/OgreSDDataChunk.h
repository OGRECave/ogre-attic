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
#ifndef _SDDataChunk_H__
#define _SDDataChunk_H__

#include "OgrePrerequisites.h"
#include "OgreDataChunk.h"

namespace Ogre {

    /** Wraps a chunk of memory, storing both size and a pointer to the data.
        @remarks
            This class simply wraps a chunk of memory. It provides extra info
            about the size of the data contained within it, simple allocation
            and free methods, and simple stream-like 'read' methods for
            parsing through the memory chunk rather like a file.
        @par
            This class contains all the functionality of the DataChunk
            superclass, only that when an object of this class gets deleted,
            it also deletes the allocated memory (SD stands for Self-Delete).
    */
    class _OgreExport SDDataChunk : public DataChunk
    {
    public:
        SDDataChunk();
        SDDataChunk( void *pData, size_t size );
        /** Overloaded destructor.
            @note
                Deletes the allocated chunk
        */        
        ~SDDataChunk();
    };

}


#endif

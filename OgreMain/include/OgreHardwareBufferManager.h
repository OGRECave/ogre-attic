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
#ifndef __HardwareBufferManager__
#define __HardwareBufferManager__

// Precompiler options
#include "OgrePrerequisites.h"

#include "OgreSingleton.h"
#include "OgreHardwareVertexBuffer.h"
#include "OgreHardwareIndexBuffer.h"

namespace Ogre {

    /** Abstract singleton class for managing hardware buffers, a concrete instance
    of this will be created by the RenderSystem. */
    class _OgreExport HardwareBufferManager : public Singleton<HardwareBufferManager>
    {
	    public:	
		    /// Create a hardware vertex buffer
		    VertexBuffer* createVertexBuffer(const HardwareVertexBuffer::VertexFormat& vf, size_t numVerts, HardwareBuffer::Usage usage);
		    /// Destroy a hardware index buffer
		    void destroyVertexBuffer(VertexBuffer* buf);
		    /// Create a hardware vertex buffer
		    IndexBuffer* createIndexBuffer(IndexFormat if, size_t numIndexes, HardwareBuffer::HardwareBufferUsage usage);
		    /// Destroy a hardware vertex buffer
		    void destroyIndexBuffer(VertexBuffer* buf);
    };

}

#endif


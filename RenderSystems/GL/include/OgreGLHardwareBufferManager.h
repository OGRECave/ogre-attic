/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
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
#ifndef __GLHARWAREBUFFERMANAGER_H__
#define __GLHARWAREBUFFERMANAGER_H__

#include "OgreGLPrerequisites.h"
#include "OgreHardwareBufferManager.h"

namespace Ogre {

    /** Implementation of HardwareBufferManager for OpenGL. */
    class GLHardwareBufferManager : public HardwareBufferManager
    {
    public:
        GLHardwareBufferManager();
        ~GLHardwareBufferManager();
        /// Creates a vertex buffer
        HardwareVertexBuffer* createVertexBuffer(size_t vertexSize, 
            size_t numVerts, HardwareBuffer::Usage usage);
        /// Destroy a hardware index buffer
        void destroyVertexBuffer(HardwareVertexBuffer* buf);
        /// Create a hardware vertex buffer
        HardwareIndexBuffer* createIndexBuffer(
            HardwareIndexBuffer::IndexType itype, size_t numIndexes, 
            HardwareBuffer::Usage usage);
        /// Destroy a hardware vertex buffer
        void destroyIndexBuffer(HardwareIndexBuffer* buf);

        /// Creates a vertex declaration, may be overridden by certain rendering APIs
        VertexDeclaration* createVertexDeclaration(void);
        /// Destroys a vertex declaration, may be overridden by certain rendering APIs
        void destroyVertexDeclaration(VertexDeclaration* decl);


    };

}

#endif // __GLHARWAREBUFFERMANAGER_H__

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
#include "OgreGLHardwareBufferManager.h"
#include "OgreGLHardwareVertexBuffer.h"
#include "OgreGLHardwareIndexBuffer.h"
#include "OgreHardwareBuffer.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    GLHardwareBufferManager::GLHardwareBufferManager()
    {
    }
    //-----------------------------------------------------------------------
    GLHardwareBufferManager::~GLHardwareBufferManager()
    {
        destroyAllDeclarations();
        destroyAllBindings();
        destroyAllBuffers();
    }
    //-----------------------------------------------------------------------
    HardwareVertexBufferSharedPtr GLHardwareBufferManager::createVertexBuffer(
        size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage, bool useShadowBuffer)
    {
		return HardwareVertexBufferSharedPtr(
			new GLHardwareVertexBuffer(vertexSize, numVerts, usage, useShadowBuffer) );
    }
    //-----------------------------------------------------------------------
    void GLHardwareBufferManager::destroyVertexBuffer(HardwareVertexBuffer* buf)
    {
        delete buf;
    }
    //-----------------------------------------------------------------------
    HardwareIndexBufferSharedPtr 
    GLHardwareBufferManager:: createIndexBuffer(
        HardwareIndexBuffer::IndexType itype, size_t numIndexes, 
        HardwareBuffer::Usage usage, bool useShadowBuffer)
    {
		return HardwareIndexBufferSharedPtr(
			new GLHardwareIndexBuffer(itype, numIndexes, usage, useShadowBuffer) );
    }
    //-----------------------------------------------------------------------
    void GLHardwareBufferManager::destroyIndexBuffer(HardwareIndexBuffer* buf)
    {
        delete buf;
    }
    //-----------------------------------------------------------------------
    VertexDeclaration* GLHardwareBufferManager::createVertexDeclaration(void)
    {
        VertexDeclaration* decl = new VertexDeclaration();
        mVertexDeclarations.push_back(decl);
        return decl;
    }
    //-----------------------------------------------------------------------
    void GLHardwareBufferManager::destroyVertexDeclaration(VertexDeclaration* decl)
    {
        mVertexDeclarations.remove(decl);
        delete decl;
    }
    //-----------------------------------------------------------------------
    void GLHardwareBufferManager::destroyAllBuffers(void)
    {
        VertexBufferList::iterator vb;
        for (vb = mVertexBuffers.begin(); vb != mVertexBuffers.end(); ++vb)
        {
            delete *vb;
        }
        mVertexBuffers.clear();

        IndexBufferList::iterator ib;
        for (ib = mIndexBuffers.begin(); ib != mIndexBuffers.end(); ++ib)
        {
            delete *ib;
        }
        mIndexBuffers.clear();
    }
    //---------------------------------------------------------------------
    GLenum GLHardwareBufferManager::getGLUsage(unsigned int usage)
    {
        switch(usage)
        {
        case HardwareBuffer::HBU_STATIC:
            return GL_STATIC_DRAW_ARB;
        case HardwareBuffer::HBU_DYNAMIC:
        case HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY:
        default:
            return GL_DYNAMIC_DRAW_ARB;
        };
    }
    //---------------------------------------------------------------------
    GLenum GLHardwareBufferManager::getGLType(unsigned int type)
    {
        switch(type)
        {
            case VET_FLOAT1:
            case VET_FLOAT2:
            case VET_FLOAT3:
            case VET_FLOAT4:
                return GL_FLOAT;
            case VET_SHORT1:
            case VET_SHORT2:
            case VET_SHORT3:
            case VET_SHORT4:
                return GL_SHORT;
            case VET_COLOUR:
                return GL_UNSIGNED_BYTE;
            default:
                return 0;
        };
    }
}

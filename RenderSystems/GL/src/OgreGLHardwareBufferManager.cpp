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
#include "OgreGLVertexDeclaration.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    GLHardwareBufferManager::GLHardwareBufferManager()
    {
    }
    //-----------------------------------------------------------------------
    GLHardwareBufferManager::~GLHardwareBufferManager()
    {
        for (VertexBufferList::iterator vi = mVertexBuffers.begin();
            vi != mVertexBuffers.end(); ++vi)
        {
            delete *vi;
        }
        mVertexBuffers.clear();

        for (IndexBufferList::iterator ii = mIndexBuffers.begin();
            ii != mIndexBuffers.end(); ++ii)
        {
            delete *ii;
        }
        mIndexBuffers.clear();

    }
    //-----------------------------------------------------------------------
    HardwareVertexBuffer* GLHardwareBufferManager::createVertexBuffer(
        size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage)
    {
        HardwareVertexBuffer* ret = new GLHardwareVertexBuffer(vertexSize, 
            numVerts, usage);
        mVertexBuffers.push_back(ret);
        return ret;
    }
    //-----------------------------------------------------------------------
    void GLHardwareBufferManager::destroyVertexBuffer(HardwareVertexBuffer* buf)
    {
        mVertexBuffers.remove(buf);
        delete buf;
    }
    //-----------------------------------------------------------------------
    HardwareIndexBuffer* 
    GLHardwareBufferManager:: createIndexBuffer(
        HardwareIndexBuffer::IndexType itype, size_t numIndexes, 
        HardwareBuffer::Usage usage)
    {
        HardwareIndexBuffer* ret = new GLHardwareIndexBuffer(itype, numIndexes, 
            usage);
        mIndexBuffers.push_back(ret);
        return ret;
    }
    //-----------------------------------------------------------------------
    void GLHardwareBufferManager::destroyIndexBuffer(HardwareIndexBuffer* buf)
    {
        mIndexBuffers.remove(buf);
        delete buf;
    }
    //-----------------------------------------------------------------------
    VertexDeclaration* GLHardwareBufferManager::createVertexDeclaration(void)
    {
        VertexDeclaration* decl = new GLVertexDeclaration();
        mVertexDeclarations.push_back(decl);
        return decl;
    }
    //-----------------------------------------------------------------------
    void GLHardwareBufferManager::destroyVertexDeclaration(VertexDeclaration* decl)
    {
        mVertexDeclarations.remove(decl);
        delete decl;
    }

}

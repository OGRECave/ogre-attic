/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright � 2000-2003 The OGRE Team
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
#include "OgreD3D9HardwareBufferManager.h"
#include "OgreD3D9HardwareVertexBuffer.h"
#include "OgreD3D9HardwareIndexBuffer.h"
#include "OgreD3D9VertexDeclaration.h"

namespace Ogre {
    //-----------------------------------------------------------------------
    D3D9HardwareBufferManager::D3D9HardwareBufferManager(LPDIRECT3DDEVICE9 device)
        : mlpD3DDevice(device)
    {
    }
    //-----------------------------------------------------------------------
    D3D9HardwareBufferManager::~D3D9HardwareBufferManager()
    {
        destroyAllDeclarations();
        destroyAllBindings();
        destroyAllBuffers();
    }
    //-----------------------------------------------------------------------
    HardwareVertexBufferSharedPtr 
    D3D9HardwareBufferManager::
    createVertexBuffer(size_t vertexSize, size_t numVerts, HardwareBuffer::Usage usage,
		bool useShadowBuffer)
    {
        return HardwareVertexBufferSharedPtr(
            new D3D9HardwareVertexBuffer(vertexSize, 
            numVerts, usage, mlpD3DDevice, false, useShadowBuffer) );
    }
    //-----------------------------------------------------------------------
	void D3D9HardwareBufferManager::destroyVertexBuffer(HardwareVertexBuffer* buf)
    {
        delete buf;
    }
    //-----------------------------------------------------------------------
    void D3D9HardwareBufferManager::destroyAllBuffers(void)
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
    //-----------------------------------------------------------------------
    void D3D9HardwareBufferManager::destroyAllDeclarations(void)
    {
        VertexDeclarationList::iterator decl;
        for (decl = mVertexDeclarations.begin(); decl != mVertexDeclarations.end(); ++decl)
        {
            delete *decl;
        }
        mVertexDeclarations.clear();
    }
    //-----------------------------------------------------------------------
	HardwareIndexBufferSharedPtr 
    D3D9HardwareBufferManager::
    createIndexBuffer(HardwareIndexBuffer::IndexType itype, size_t numIndexes, 
        HardwareBuffer::Usage usage, bool useShadowBuffer)
    {
        // NB no longer store the buffer in a local list since reference counted
        return HardwareIndexBufferSharedPtr(
                new D3D9HardwareIndexBuffer(itype, numIndexes, 
                usage, mlpD3DDevice, false, useShadowBuffer) );
            
    }
    //-----------------------------------------------------------------------
	void D3D9HardwareBufferManager::destroyIndexBuffer(HardwareIndexBuffer* buf)
    {
        delete buf;
    }
    //-----------------------------------------------------------------------
    VertexDeclaration* D3D9HardwareBufferManager::createVertexDeclaration(void)
    {
        VertexDeclaration* decl = new D3D9VertexDeclaration(mlpD3DDevice);
        mVertexDeclarations.push_back(decl);
        return decl;
        
    }
    //-----------------------------------------------------------------------
    void D3D9HardwareBufferManager::destroyVertexDeclaration(VertexDeclaration* decl)
    {
        mVertexDeclarations.remove(decl);
        delete decl;
    }

}

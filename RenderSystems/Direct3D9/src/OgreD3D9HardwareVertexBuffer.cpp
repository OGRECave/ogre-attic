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
#include "OgreD3D9HardwareVertexBuffer.h"
#include "OgreD3D9Mappings.h"
#include "OgreException.h"

namespace Ogre {

	//---------------------------------------------------------------------
    D3D9HardwareVertexBuffer::D3D9HardwareVertexBuffer(size_t vertexSize, 
        size_t numVertices, HardwareBuffer::Usage usage, LPDIRECT3DDEVICE9 pDev)
        : HardwareVertexBuffer(vertexSize, numVertices, usage)
    {
        // Create the vertex buffer
        HRESULT hr = pDev->CreateVertexBuffer(
            mSizeInBytes, 
            D3D9Mappings::get(usage), 
            0, // No FVF here, thankyou
            D3DPOOL_DEFAULT, 
            &mlpD3DBuffer,
            NULL);
        if (FAILED(hr))
        {
            Except(hr, "Cannot create D3D9 vertex buffer", 
                "D3D9HardwareVertexBuffer::D3D9HardwareVertexBuffer");
        }

    }
	//---------------------------------------------------------------------
    D3D9HardwareVertexBuffer::~D3D9HardwareVertexBuffer()
    {
        SAFE_RELEASE(mlpD3DBuffer);
    }
	//---------------------------------------------------------------------
    void* D3D9HardwareVertexBuffer::lock(size_t offset, 
        size_t length, LockOptions options)
    {
        void* pBuf;
        HRESULT hr = mlpD3DBuffer->Lock(
            offset, 
            length, 
            &pBuf,
            D3D9Mappings::get(options));

        if (FAILED(hr))
        {
            Except(hr, "Cannot lock D3D9 vertex buffer", 
                "D3D9HardwareVertexBuffer::lock");
        }

        mIsLocked = true;

        return pBuf;



    }
	//---------------------------------------------------------------------
	void D3D9HardwareVertexBuffer::unlock(void)
    {
        HRESULT hr = mlpD3DBuffer->Unlock();

        mIsLocked = false;
    }
	//---------------------------------------------------------------------
    void D3D9HardwareVertexBuffer::readData(size_t offset, size_t length, 
        void* pDest)
    {
        // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        void* pSrc = this->lock(offset, length, HardwareBuffer::HBL_READ_ONLY);
        memcpy(pDest, pSrc, length);
        this->unlock();

    }
	//---------------------------------------------------------------------
    void D3D9HardwareVertexBuffer::writeData(size_t offset, size_t length, 
            const void* pSource,
			bool discardWholeBuffer)
    {
        // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        void* pDst = this->lock(offset, length, 
            discardWholeBuffer ? HardwareBuffer::HBL_DISCARD : HardwareBuffer::HBL_NORMAL);
        memcpy(pDst, pSource, length);
        this->unlock();
    }
	//---------------------------------------------------------------------

}

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreD3D9HardwareIndexBuffer.h"
#include "OgreD3D9Mappings.h"
#include "OgreException.h"

namespace Ogre {

	//---------------------------------------------------------------------
    D3D9HardwareIndexBuffer::D3D9HardwareIndexBuffer(HardwareIndexBuffer::IndexType idxType, 
        size_t numIndexes, HardwareBuffer::Usage usage, LPDIRECT3DDEVICE9 pDev, 
        bool useSystemMemory, bool useShadowBuffer)
        : HardwareIndexBuffer(idxType, numIndexes, usage, useSystemMemory, useShadowBuffer)
    {
        // Create the Index buffer
        HRESULT hr = pDev->CreateIndexBuffer(
            static_cast<UINT>(mSizeInBytes),
            D3D9Mappings::get(usage),
            D3D9Mappings::get(idxType),
			mSystemMemory ? D3DPOOL_SYSTEMMEM : D3DPOOL_DEFAULT,
            &mlpD3DBuffer,
            NULL
            );
            
        if (FAILED(hr))
        {
			String msg = DXGetErrorDescription9(hr);
            Except(hr, "Cannot create D3D9 Index buffer: " + msg, 
                "D3D9HardwareIndexBuffer::D3D9HardwareIndexBuffer");
        }

    }
	//---------------------------------------------------------------------
    D3D9HardwareIndexBuffer::~D3D9HardwareIndexBuffer()
    {
        SAFE_RELEASE(mlpD3DBuffer);
    }
	//---------------------------------------------------------------------
    void* D3D9HardwareIndexBuffer::lockImpl(size_t offset, 
        size_t length, LockOptions options)
    {
        void* pBuf;
		DWORD lockOpts;
		if (!(mUsage & HBU_DYNAMIC) && options == HBL_DISCARD)
		{
			// D3D doesn't like discard on non-dynamic buffers
			lockOpts = 0;
		}
		else
		{
			lockOpts= D3D9Mappings::get(options);
		} 
        HRESULT hr = mlpD3DBuffer->Lock(
            static_cast<UINT>(offset), 
            static_cast<UINT>(length), 
            &pBuf,
            lockOpts);

        if (FAILED(hr))
        {
            Except(hr, "Cannot lock D3D9 Index buffer", 
                "D3D9HardwareIndexBuffer::lock");
        }


        return pBuf;


    }
	//---------------------------------------------------------------------
	void D3D9HardwareIndexBuffer::unlockImpl(void)
    {
        HRESULT hr = mlpD3DBuffer->Unlock();
    }
	//---------------------------------------------------------------------
    void D3D9HardwareIndexBuffer::readData(size_t offset, size_t length, 
        void* pDest)
    {
       // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        void* pSrc = this->lock(offset, length, HardwareBuffer::HBL_READ_ONLY);
        memcpy(pDest, pSrc, length);
        this->unlock();

    }
	//---------------------------------------------------------------------
    void D3D9HardwareIndexBuffer::writeData(size_t offset, size_t length, 
            const void* pSource,
			bool discardWholeBuffer)
    {
       // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        void* pDst = this->lock(offset, length, 
            discardWholeBuffer ? HardwareBuffer::HBL_DISCARD : HardwareBuffer::HBL_NORMAL);
        memcpy(pDst, pSource, length);
        this->unlock();    }
	//---------------------------------------------------------------------

}

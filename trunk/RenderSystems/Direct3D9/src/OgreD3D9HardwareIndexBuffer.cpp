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
#include "OgreD3D9HardwareIndexBuffer.h"
#include "OgreD3D9Mappings.h"
#include "OgreException.h"

namespace Ogre {

	//---------------------------------------------------------------------
    D3D9HardwareIndexBuffer::D3D9HardwareIndexBuffer(HardwareIndexBuffer::IndexType idxType, 
        size_t numIndexes, HardwareBuffer::Usage usage, LPDIRECT3DDEVICE9 pDev)
        : HardwareIndexBuffer(idxType, numIndexes, usage)
    {
        // Create the Index buffer
        HRESULT hr = pDev->CreateIndexBuffer(
            mSizeInBytes,
            D3D9Mappings::get(usage),
            D3D9Mappings::get(idxType),
            D3DPOOL_DEFAULT,
            &mlpD3DBuffer,
            NULL
            );
            
        if (FAILED(hr))
        {
            Except(hr, "Cannot create D3D9 Index buffer", 
                "D3D9HardwareIndexBuffer::D3D9HardwareIndexBuffer");
        }

    }
	//---------------------------------------------------------------------
    D3D9HardwareIndexBuffer::~D3D9HardwareIndexBuffer()
    {
        SAFE_RELEASE(mlpD3DBuffer);
    }
	//---------------------------------------------------------------------
    unsigned char* D3D9HardwareIndexBuffer::lock(size_t offset, 
        size_t length, LockOptions options)
    {
        unsigned char* pBuf;
        HRESULT hr = mlpD3DBuffer->Lock(
            offset, 
            length, 
            (void**)&pBuf,
            D3D9Mappings::get(options));

        if (FAILED(hr))
        {
            Except(hr, "Cannot lock D3D9 Index buffer", 
                "D3D9HardwareIndexBuffer::lock");
        }


        mIsLocked = true;

        return pBuf;


    }
	//---------------------------------------------------------------------
	void D3D9HardwareIndexBuffer::unlock(void)
    {
        HRESULT hr = mlpD3DBuffer->Unlock();

        mIsLocked = false;
    }
	//---------------------------------------------------------------------
    void D3D9HardwareIndexBuffer::readData(size_t offset, size_t length, 
        unsigned char* pDest)
    {
       // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        unsigned char* pSrc = this->lock(offset, length, HardwareBuffer::HBL_READ_ONLY);
        memcpy(pDest, pSrc, length);
        this->unlock();

    }
	//---------------------------------------------------------------------
    void D3D9HardwareIndexBuffer::writeData(size_t offset, size_t length, 
            const unsigned char* pSource,
			bool discardWholeBuffer)
    {
       // There is no functional interface in D3D, just do via manual 
        // lock, copy & unlock
        unsigned char* pDst = this->lock(offset, length, 
            discardWholeBuffer ? HardwareBuffer::HBL_DISCARD : HardwareBuffer::HBL_NORMAL);
        memcpy(pDst, pSource, length);
        this->unlock();    }
	//---------------------------------------------------------------------

}

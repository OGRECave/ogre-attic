/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2003 The OGRE Team
Also see acknowledgements in Readme.html

This program is free software you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#include "OgreDefaultHardwareBufferManager.h"

namespace Ogre {

	DefaultHardwareVertexBuffer::DefaultHardwareVertexBuffer(size_t vertexSize, size_t numVertices, 
		HardwareBuffer::Usage usage)
        : HardwareVertexBuffer(vertexSize, numVertices, usage, true, false) // always software, never shadowed
	{
		mpData = new unsigned char[mSizeInBytes];
	}
	//-----------------------------------------------------------------------
    DefaultHardwareVertexBuffer::~DefaultHardwareVertexBuffer()
	{
		delete [] mpData;
	}
	//-----------------------------------------------------------------------
    void* DefaultHardwareVertexBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
	{
        // Nothing to do
        return NULL;
	}
	//-----------------------------------------------------------------------
	void DefaultHardwareVertexBuffer::unlockImpl(void)
	{
        // Nothing to do
	}
	//-----------------------------------------------------------------------
    void* DefaultHardwareVertexBuffer::lock(size_t offset, size_t length, LockOptions options)
	{
		return mpData + offset;
	}
	//-----------------------------------------------------------------------
	void DefaultHardwareVertexBuffer::unlock(void)
	{
        // Nothing to do
	}
	//-----------------------------------------------------------------------
    void DefaultHardwareVertexBuffer::readData(size_t offset, size_t length, void* pDest)
	{
		assert((offset + length) <= mSizeInBytes);
		memcpy(pDest, mpData + offset, length);
	}
	//-----------------------------------------------------------------------
    void DefaultHardwareVertexBuffer::writeData(size_t offset, size_t length, const void* pSource,
			bool discardWholeBuffer)
	{
		assert((offset + length) <= mSizeInBytes);
		// ignore discard, memory is not guaranteed to be zeroised
		memcpy(mpData + offset, pSource, length);

	}
	//-----------------------------------------------------------------------

	DefaultHardwareIndexBuffer::DefaultHardwareIndexBuffer(IndexType idxType, 
		size_t numIndexes, HardwareBuffer::Usage usage) 
		: HardwareIndexBuffer(idxType, numIndexes, usage, true, false) // always software, never shadowed
	{
		mpData = new unsigned char[mSizeInBytes];
	}
	//-----------------------------------------------------------------------
    DefaultHardwareIndexBuffer::~DefaultHardwareIndexBuffer()
	{
		delete [] mpData;
	}
	//-----------------------------------------------------------------------
    void* DefaultHardwareIndexBuffer::lockImpl(size_t offset, size_t length, LockOptions options)
	{
        // Nothing to do
        return NULL;
	}
	//-----------------------------------------------------------------------
	void DefaultHardwareIndexBuffer::unlockImpl(void)
	{
        // Nothing to do
	}
	//-----------------------------------------------------------------------
    void* DefaultHardwareIndexBuffer::lock(size_t offset, size_t length, LockOptions options)
	{
		return mpData + offset;
	}
	//-----------------------------------------------------------------------
	void DefaultHardwareIndexBuffer::unlock(void)
	{
        // Nothing to do
	}
	//-----------------------------------------------------------------------
    void DefaultHardwareIndexBuffer::readData(size_t offset, size_t length, void* pDest)
	{
		assert((offset + length) <= mSizeInBytes);
		memcpy(pDest, mpData + offset, length);
	}
	//-----------------------------------------------------------------------
    void DefaultHardwareIndexBuffer::writeData(size_t offset, size_t length, const void* pSource,
			bool discardWholeBuffer)
	{
		assert((offset + length) <= mSizeInBytes);
		// ignore discard, memory is not guaranteed to be zeroised
		memcpy(mpData + offset, pSource, length);

	}
	//-----------------------------------------------------------------------
	
	
	//-----------------------------------------------------------------------
	void DefaultHardwareBufferManager::destroyVertexBuffer(HardwareVertexBuffer* buf)
	{
		delete buf;
	}
    //-----------------------------------------------------------------------
	void DefaultHardwareBufferManager::destroyIndexBuffer(HardwareIndexBuffer* buf)
	{
		delete buf;
	}
    //-----------------------------------------------------------------------
    DefaultHardwareBufferManager::DefaultHardwareBufferManager()
	{
	}
    //-----------------------------------------------------------------------
    DefaultHardwareBufferManager::~DefaultHardwareBufferManager()
	{
	}
    //-----------------------------------------------------------------------
	HardwareVertexBufferSharedPtr 
        DefaultHardwareBufferManager::createVertexBuffer(size_t vertexSize, 
		size_t numVerts, HardwareBuffer::Usage usage, bool useShadowBuffer)
	{
		return HardwareVertexBufferSharedPtr(
			new DefaultHardwareVertexBuffer(vertexSize, numVerts, usage));
	}
    //-----------------------------------------------------------------------
	HardwareIndexBufferSharedPtr 
        DefaultHardwareBufferManager::createIndexBuffer(HardwareIndexBuffer::IndexType itype, 
		size_t numIndexes, HardwareBuffer::Usage usage, bool useShadowBuffer)
	{
		return HardwareIndexBufferSharedPtr(
			new DefaultHardwareIndexBuffer(itype, numIndexes, usage) );
	}
}

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
#ifndef __D3D9HWBUFFERS_H__
#define __D3D9HWBUFFERS_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreException.h"

#include "OgreNoMemoryMacros.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include "OgreMemoryMacros.h"

/// minimal Vertex buffers vertex count
#define MIN_VB_VERTEX_COUNT 100
/// minimal Index buffers indices count 
#define MIN_IB_INDEX_COUNT 100

namespace Ogre 
{
	/// dynamic hardware vertex buffer template class
	template < class VertexBufferType >
	class D3D9DynamicVertexBuffer
	{
	private : 
		/// the buffer
		IDirect3DVertexBuffer9 *mpBuff;
		/// vertex count
		uint mVCount;
		/// vertex stride
		uint mVStride;
		/// the size of the buffer
		uint mSize;
		/// dimensions of one vertex (ie. 1, 2, 3, 4 floats)
		uchar mTypeCount;
		/// the type of the buffer, a D3D9 FVF code
		DWORD mType;
		/// is the buffer locked
		bool mIsLocked;
		/// is the buffer in use
		bool mIsInUse;

	public :
		/// constructor
		/// defaults to no vertex stride and 0 for format
		D3D9DynamicVertexBuffer(
			IDirect3DDevice9 *pDevice, 
			uint numVertices, 
			uchar typeCount, 
			uint vertexStride = 0, 
			DWORD fvf = 0)
		{
			assert(pDevice);
			assert(numVertices);
			assert(typeCount);
			// zero member vars first
			mpBuff = NULL;
			mIsLocked = false;
			mIsInUse = false;
			mVCount = 0;
			mVStride = 0;
			mType = 0;
			mTypeCount = 0;
			mSize = 0;
			
			// calculate size based on data stride if present
			uint size;
			if (vertexStride)
				size = vertexStride + (sizeof(VertexBufferType) * typeCount);
			else
				size = (sizeof(VertexBufferType) * typeCount);
			// create a default sized if vertex count < MIN_VB_VERTEX_COUNT
			if (numVertices < MIN_VB_VERTEX_COUNT)
				numVertices = MIN_VB_VERTEX_COUNT;
			// create the buffer
			HRESULT hr = pDevice->CreateVertexBuffer(
				size * numVertices, 
				D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 
				fvf,
				D3DPOOL_DEFAULT, 
				&mpBuff,
				NULL);
			// complain if failed :(
			if (FAILED(hr))
				Except( hr, "Failed to create dynamic vertex buffer", "D3D9DynamicVertexBuffer::D3D9DynamicVertexBuffer" );

			// just to be sure ;)
			assert(mpBuff);
			// set the member vars now when the buffer is created
			mSize = size * numVertices;
			mType = fvf;
			mTypeCount = typeCount;
			mVCount = numVertices;
			mVStride = vertexStride;
		}
		/// destructor
		~D3D9DynamicVertexBuffer()
		{
			// first unlock if locked
			unlock();
			// release the buffer if created
			if (mpBuff)
				mpBuff->Release();
		}
		/// lock the buffer and return a pointer to the locked data buffer
		/// defaults to 0,0 wich means the whole buffer will be locked
		VertexBufferType *lock(uint numVertices = 0, uint startVertex = 0)
		{
			assert(mpBuff);

			VertexBufferType* pData = NULL;
			uint offset = 0;
			uint size = 0;
			uint stride = 0;

			// calculate stride if present
			if (mVStride)
				stride = mVStride + (sizeof(VertexBufferType) * mTypeCount);
			else
				stride = sizeof(VertexBufferType) * mTypeCount;

			// calculate start offset based on stride
			if (startVertex)
				offset = startVertex * stride;
			// calculate size based on stride
			if (numVertices)
				size = numVertices * stride;

			// lock the buffer
			HRESULT hr = mpBuff->Lock(offset, size, (void **)&pData, D3DLOCK_DISCARD);
			if (FAILED(hr))
				Except( hr, "Failed to lock dynamic vertex buffer", "D3D9DynamicVertexBuffer::Lock" );

			// say 'it's locked man!!'
			mIsLocked = true;
			// set the in use flag
			// this is needed because in one RenderOperation can be for example 2 tex.coord.
			// buffers, and the buffer manager can return twice the same buffer to be used
			// wich is not what we want, so this flag will say to the buffer manager not to use this buffer ;)
			mIsInUse = true;
			// return locked buffer
			return pData;
		}
		/// unlock a locked buffer
		void unlock()
		{
			// unlock only if locked and created
			if (mIsLocked && mpBuff)
			{
				HRESULT hr = mpBuff->Unlock();
				// complain if failed
				if (FAILED(hr))
					Except( hr, "Failed to unlock dynamic vertex buffer", "D3D9DynamicVertexBuffer::Unlock" );
				mIsLocked = false;
			}
		}
		/// copy data to the buffer, lock and unlock after
		void setData(const void *src, unsigned int numVertices = 0, unsigned int startVertex = 0)
		{
			assert(mpBuff);
			if (mIsLocked)
				unlock();

			// lock the buffer and obtain a pointer to the locked data
			VertexBufferType *dst = lock(numVertices, startVertex);
			// calculate stride if present
			uint stride = 0;
			if (mVStride)
				stride = mVStride + (sizeof(VertexBufferType) * mTypeCount);
			else
				stride = sizeof(VertexBufferType) * mTypeCount;
			// calculate size based on stride
			uint size = 0;
			if (numVertices)
				size = numVertices * stride;
			// copy the memory
			// TODO : maybe use SSE/MMX if available ??
			memcpy(dst, src, size);
			// unlock the buffer
			unlock();
		}
		/// return true if the 'inUse' flag is not cleared
		bool isInUse() const
		{ return mIsInUse;}
		/// clear the 'inUse' flag
		void clearInUseFlag()
		{ mIsInUse = false; }
		/// is the buffer locked
		bool isLocked() const
		{ return mIsLocked;}
		/// return the size of the buffer
		uint getSize() const
		{ assert(mSize); return mSize; }
		/// return the number of vertices in the buffer
		uint getVertexCount() const
		{ assert(mVCount); return mVCount; }
		/// return vertex stride
		uint getVertexStride() const
		{ return mVStride; }
		/// return a pointer to the D3D9 vertex buffer interface
		IDirect3DVertexBuffer9 *getBuffer() const
		{ assert(mpBuff); return mpBuff; }
	};

	/// dynamic hardware index buffer template class
	template < class IndexBufferType >
	class D3D9DynamicIndexBuffer
	{
	private : 
		/// the D3D9 index buffer interface pointer
		IDirect3DIndexBuffer9 *mpBuff;
		/// number of indices in buffer
		uint mICount;
		/// buffer size
		uint mSize;
		/// format of the buffer
		D3DFORMAT mFormat;
		/// is the buffer locked ???
		bool mIsLocked;

	public :
		/// constructor
		/// defaults to D3DFMT_INDEX16 format
		D3D9DynamicIndexBuffer(
			IDirect3DDevice9 *pDevice, 
			uint numIndices, 
			D3DFORMAT format = D3DFMT_INDEX16)
		{
			assert(pDevice);
			assert(numIndices);
			// zero member vars first
			mpBuff = NULL;
			mIsLocked = false;
			mICount = 0;
			mFormat = D3DFMT_UNKNOWN;
			mSize = 0;

			// calculate size of buffer based on stride if present
			uint size = sizeof(IndexBufferType);
			// create a default sized if indixes count < MIN_IB_INDEX_COUNT
			if (numIndices < MIN_IB_INDEX_COUNT)
				numIndices = MIN_IB_INDEX_COUNT;
			// create the buffer
			HRESULT hr = pDevice->CreateIndexBuffer(
				size * numIndices, 
				D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 
				format,
				D3DPOOL_DEFAULT, 
				&mpBuff,
				NULL);
			// complaint if failed :(
			if (FAILED(hr))
				Except( hr, "Failed to create dynamic index buffer", "D3D9DynamicIndexBuffer::D3D9DynamicIndexBuffer" );
			// just in case...
			assert(mpBuff);

			// set member vals to real values now that the buffer is there
			mSize = size * numIndices;
			mFormat = format;
			mICount = numIndices;
		}
		/// destructor		
		~D3D9DynamicIndexBuffer()
		{
			// unlock if locked
			unlock();
			// release if created
			if (mpBuff)
				mpBuff->Release();
		}
		/// lock the buffer
		/// defaults to 0 vertices and 0 as the start index (offset), this means the whole buffer
		IndexBufferType *lock(unsigned int numIndices = 0, unsigned int startIndex = 0)
		{
			assert(mpBuff);

			IndexBufferType* pData = NULL;
			uint offset = 0;
			uint size = 0;

			// calculate start offset
			if (startIndex)
				offset = startIndex * sizeof(IndexBufferType);
			// calculate size
			if (numIndices)
				size = numIndices * sizeof(IndexBufferType);

			// lock IT!!
			HRESULT hr = mpBuff->Lock(offset, size, (void **)&pData, D3DLOCK_DISCARD);
			// cry ;(
			if (FAILED(hr))
				Except( hr, "Failed to lock dynamic index buffer", "D3D9DynamicIndexBuffer::Lock" );

			// say it's locked
			mIsLocked = true;
			// return locked data pointer
			return pData;
		}
		/// unlock a locked buffer
		void unlock()
		{
			// unlock only if locked and defined
			if (mIsLocked && mpBuff)
			{
				// do it..
				HRESULT hr = mpBuff->Unlock();
				// what the f..k ???
				if (FAILED(hr))
					Except( hr, "Failed to unlock dynamic index buffer", "D3D9DynamicIndexBuffer::Unlock" );
				// that's better...
				mIsLocked = false;
			}
		}
		/// copy data to the buffer, lock and unlock after
		void setData(const void *src, unsigned int numIndices = 0, unsigned int startIndex = 0)
		{
			assert(mpBuff);
			// first unlock if locked
			if (mIsLocked)
				unlock();
			// lock the buffer and obtain a pointer to the locked data
			IndexBufferType *dst = lock(numIndices, startIndex);
			// calculate size
			uint size = 0;
			if (numIndices)
				size = numIndices * sizeof(IndexBufferType);
			// copy the data to the buffer
			memcpy(dst, src, size);
			// unlock it
			unlock();
		}
		/// return the buffer format
		D3DFORMAT getFormat() const
		{ return mFormat;}
		/// is the buffer locked ???
		bool isLocked() const
		{ return mIsLocked;}
		/// returns the size of the buffer
		uint getSize() const
		{ assert(mSize); return mSize; }
		/// return number of indices in the buffer
		uint getIndexCount() const
		{ assert(mICount); return mICount; }
		/// return the pointer to the D3D9 index buffer interface
		IDirect3DIndexBuffer9 *getBuffer() const
		{ assert(mpBuff); return mpBuff; }
	};

	/// Dynamic vertex buffers manager class inplementation
	class D3D9DynVBManager
	{
	public:
		/// float type of vertex buffer, for position, normals, tex.coords
		typedef D3D9DynamicVertexBuffer< float >	FloatVB;
		/// color type of vertex buffer, for diffuse/specular colors
		typedef D3D9DynamicVertexBuffer< D3DCOLOR > ColorVB;
	private:
		/// type of position buffer pool
		typedef std::vector< FloatVB * >	PositionBuffers;
		/// type of normals buffer pool
		typedef std::vector< FloatVB * >	NormalBuffers;
		/// type of diffuse color buffer pool
		typedef std::vector< ColorVB * >	DiffuseBuffers;
		/// type of specular color buffer pool
		typedef std::vector< ColorVB * >	SpecularBuffers;
		/// type of tex.coord of size 1 float buffer pool
		typedef std::vector< FloatVB * >	TexCoord1Buffers;
		/// type of tex.coord of size 2 float buffer pool
		typedef std::vector< FloatVB * >	TexCoord2Buffers;
		/// type of tex.coord of size 3 float buffer pool
		typedef std::vector< FloatVB * >	TexCoord3Buffers;
		/// type of tex.coord of size 4 float buffer pool
		typedef std::vector< FloatVB * >	TexCoord4Buffers;

		/// our position buffers pool
		PositionBuffers		mPos;
		/// our normals buffers pool
		NormalBuffers		mNor;
		/// our diffuse colors buffers pool
		DiffuseBuffers		mDif;
		/// our specular colors buffers pool
		SpecularBuffers		mSpe;
		/// our tex.coords float 1 buffers pool
		TexCoord1Buffers	mTex1;
		/// our tex.coords float 2 buffers pool
		TexCoord2Buffers	mTex2;
		/// our tex.coords float 3 buffers pool
		TexCoord3Buffers	mTex3;
		/// our tex.coords float 4 pool
		TexCoord4Buffers	mTex4;

		/// I'll declare itterators here also, so no time is spent every time to construct them
		PositionBuffers::iterator	mPosIT;
		NormalBuffers::iterator		mNorIT;
		DiffuseBuffers::iterator	mDifIT;
		SpecularBuffers::iterator	mSpeIT;
		TexCoord1Buffers::iterator	mTex1IT;
		TexCoord2Buffers::iterator	mTex2IT;
		TexCoord3Buffers::iterator	mTex3IT;
		TexCoord4Buffers::iterator	mTex4IT;

		/// Direct3D9 device pointer
		IDirect3DDevice9 *mpDev;
	
	public:
		/// constructor
		D3D9DynVBManager(IDirect3DDevice9 *pDevice);
		/// destructor
		~D3D9DynVBManager();
		/// returns a Position VB
		FloatVB *getPositionBuffer(			
			uint numVertices, 
			uint vertexStride = 0);
		/// returns a Normals VB
		FloatVB *getNormalBuffer(			
			uint numVertices, 
			uint vertexStride = 0);
		/// returns a tex.coord float 1 VB
		FloatVB *getTexCoord1Buffer(			
			uint numVertices, 
			uint vertexStride = 0);
		/// returns a tex.coord float 2 VB
		FloatVB *getTexCoord2Buffer(			
			uint numVertices, 
			uint vertexStride = 0);
		/// returns a tex.coord float 3 VB
		FloatVB *getTexCoord3Buffer(			
			uint numVertices, 
			uint vertexStride = 0);
		/// returns a tex.coord float 4 VB
		FloatVB *getTexCoord4Buffer(			
			uint numVertices, 
			uint vertexStride = 0);
		/// returns a diffuse color VB
		ColorVB *getDiffuseBuffer(			
			uint numVertices, 
			uint vertexStride = 0);
		/// returns a specular color VB
		ColorVB *getSpecularBuffer(			
			uint numVertices, 
			uint vertexStride = 0);
		/// clear 'inUse' flags from all tex.coord buffers
		/// only tex.coords VB can be used more that one in one RenderOperation
		void clearInUseFlagedBuffers();
	};

	/// Dynamic index buffers manager class inplementation
	class D3D9DynIBManager
	{
	public:
		/// type of our index buffers (D3DFMT_INDEX16)
		typedef D3D9DynamicIndexBuffer< unsigned short > IndexBuffer;
	private:
		/// type of our index buffers pool
		typedef std::vector< IndexBuffer * > IndexBuffers;
		/// the index buffer pool
		IndexBuffers mPool;	
		/// I'll declare itterators here also, so no time is spent every time to construct them
		IndexBuffers::iterator mPoolIT;
		/// the D3D9 device
		IDirect3DDevice9 *mpDev;

	public:
		/// constructor
		D3D9DynIBManager(IDirect3DDevice9 *pDevice);
		/// destructor
		~D3D9DynIBManager();
		/// return a index buffer
		IndexBuffer *getBuffer(uint numIndices);
	};
}
#endif
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
#include "OgreD3D9HWBuffers.h"
#include "OgreString.h"
#include "OgreStringConverter.h"
#include "OgreLogManager.h"

namespace Ogre 
{
	/************************************************************************/
	/********************DYNAMIC VERTEX BUFFER MANAGER***********************/
	/************************************************************************/
	D3D9DynVBManager::D3D9DynVBManager(IDirect3DDevice9 *pDevice) 
	{
		assert(pDevice);
		mpDev = pDevice;
	}
	/************************************************************************/
	D3D9DynVBManager::~D3D9DynVBManager() 
	{
		LogManager::getSingleton().logMessage( "D3D9 : Used " + StringConverter::toString((uint)mPos.size()) + " position dynamic VB's " );
		for (mPosIT = mPos.begin(); mPosIT != mPos.end(); mPosIT++)
			delete (*mPosIT);
		mPos.clear();

		LogManager::getSingleton().logMessage( "D3D9 : Used " + StringConverter::toString((uint)mNor.size()) + " normals dynamic VB's " );
		for (mNorIT = mNor.begin(); mNorIT != mNor.end(); mNorIT++)
			delete (*mNorIT);
		mNor.clear();

		LogManager::getSingleton().logMessage( "D3D9 : Used " + StringConverter::toString((uint)mDif.size()) + " diffuse dynamic VB's " );
		for (mDifIT = mDif.begin(); mDifIT != mDif.end(); mDifIT++)
			delete (*mDifIT);
		mDif.clear();

		LogManager::getSingleton().logMessage( "D3D9 : Used " + StringConverter::toString((uint)mSpe.size()) + " specular dynamic VB's " );
		for (mSpeIT = mSpe.begin(); mSpeIT != mSpe.end(); mSpeIT++)
			delete (*mSpeIT);
		mSpe.clear();

		LogManager::getSingleton().logMessage( "D3D9 : Used " + StringConverter::toString((uint)mTex1.size()) + " tex.coord. size 1 dynamic VB's " );
		for (mTex1IT = mTex1.begin(); mTex1IT != mTex1.end(); mTex1IT++)
			delete (*mTex1IT);
		mTex1.clear();

		LogManager::getSingleton().logMessage( "D3D9 : Used " + StringConverter::toString((uint)mTex2.size()) + " tex.coord. size 2 dynamic VB's " );
		for (mTex2IT = mTex2.begin(); mTex2IT != mTex2.end(); mTex2IT++)
			delete (*mTex2IT);
		mTex2.clear();

		LogManager::getSingleton().logMessage( "D3D9 : Used " + StringConverter::toString((uint)mTex3.size()) + " tex.coord. size 3 dynamic VB's " );
		for (mTex3IT = mTex3.begin(); mTex3IT != mTex3.end(); mTex3IT++)
			delete (*mTex3IT);
		mTex3.clear();

		LogManager::getSingleton().logMessage( "D3D9 : Used " + StringConverter::toString((uint)mTex4.size()) + " tex.coord. size 4 dynamic VB's " );
		for (mTex4IT = mTex4.begin(); mTex4IT != mTex4.end(); mTex4IT++)
			delete (*mTex4IT);
		mTex4.clear();
	}
	/************************************************************************/
	D3D9DynVBManager::FloatVB *D3D9DynVBManager::getPositionBuffer(			
		uint numVertices, 
		uint vertexStride) 
	{
		for (mPosIT = mPos.begin(); mPosIT != mPos.end(); mPosIT++)
		{
			if ((*mPosIT)->getVertexCount() >= numVertices &&
				(*mPosIT)->getVertexStride() == vertexStride)
				return (*mPosIT);
		}

		FloatVB *newFB = new FloatVB(mpDev, numVertices, 3, vertexStride, D3DFVF_XYZ);
		mPos.push_back(newFB);
		return newFB;
	}
	/************************************************************************/
	D3D9DynVBManager::FloatVB *D3D9DynVBManager::getNormalBuffer(			
		uint numVertices, 
		uint vertexStride) 
	{
		for (mNorIT = mNor.begin(); mNorIT != mNor.end(); mNorIT++)
		{
			if ((*mNorIT)->getVertexCount() >= numVertices &&
				(*mNorIT)->getVertexStride() == vertexStride)
				return (*mNorIT);
		}

		FloatVB *newFB = new FloatVB(mpDev, numVertices, 3, vertexStride, D3DFVF_NORMAL);
		mNor.push_back(newFB);
		return newFB;
	}
	/************************************************************************/
	D3D9DynVBManager::FloatVB *D3D9DynVBManager::getTexCoord1Buffer(			
		uint numVertices, 
		uint vertexStride) 
	{
		for (mTex1IT = mTex1.begin(); mTex1IT != mTex1.end(); mTex1IT++)
		{
			if ((*mTex1IT)->getVertexCount() >= numVertices &&
				(*mTex1IT)->getVertexStride() == vertexStride &&
				(*mTex1IT)->isInUse() != true)
				return (*mTex1IT);
		}

		FloatVB *newFB = new FloatVB(mpDev, numVertices, 1, vertexStride, 0);
		mTex1.push_back(newFB);
		return newFB;
	}
	/************************************************************************/
	D3D9DynVBManager::FloatVB *D3D9DynVBManager::getTexCoord2Buffer(			
		uint numVertices, 
		uint vertexStride) 
	{
		for (mTex2IT = mTex2.begin(); mTex2IT != mTex2.end(); mTex2IT++)
		{
			if ((*mTex2IT)->getVertexCount() >= numVertices &&
				(*mTex2IT)->getVertexStride() == vertexStride &&
				(*mTex2IT)->isInUse() != true)
				return (*mTex2IT);
		}

		FloatVB *newFB = new FloatVB(mpDev, numVertices, 2, vertexStride, 0);
		mTex2.push_back(newFB);
		return newFB;
	}
	/************************************************************************/
	D3D9DynVBManager::FloatVB *D3D9DynVBManager::getTexCoord3Buffer(			
		uint numVertices, 
		uint vertexStride) 
	{
		for (mTex3IT = mTex3.begin(); mTex3IT != mTex3.end(); mTex3IT++)
		{
			if ((*mTex3IT)->getVertexCount() >= numVertices &&
				(*mTex3IT)->getVertexStride() == vertexStride &&
				(*mTex3IT)->isInUse() != true)
				return (*mTex3IT);
		}

		FloatVB *newFB = new FloatVB(mpDev, numVertices, 3, vertexStride, 0);
		mTex3.push_back(newFB);
		return newFB;
	}
	/************************************************************************/
	D3D9DynVBManager::FloatVB *D3D9DynVBManager::getTexCoord4Buffer(			
		uint numVertices, 
		uint vertexStride) 
	{
		for (mTex4IT = mTex4.begin(); mTex4IT != mTex4.end(); mTex4IT++)
		{
			if ((*mTex4IT)->getVertexCount() >= numVertices &&
				(*mTex4IT)->getVertexStride() == vertexStride &&
				(*mTex4IT)->isInUse() != true)
				return (*mTex4IT);
		}

		FloatVB *newFB = new FloatVB(mpDev, numVertices, 4, vertexStride, 0);
		mTex4.push_back(newFB);
		return newFB;
	}
	/************************************************************************/
	D3D9DynVBManager::ColorVB *D3D9DynVBManager::getDiffuseBuffer(			
		uint numVertices, 
		uint vertexStride) 
	{
		for (mDifIT = mDif.begin(); mDifIT != mDif.end(); mDifIT++)
		{
			if ((*mDifIT)->getVertexCount() >= numVertices &&
				(*mDifIT)->getVertexStride() == vertexStride)
				return (*mDifIT);
		}

		ColorVB *newCB = new ColorVB(mpDev, numVertices, 1, vertexStride, D3DFVF_DIFFUSE);
		mDif.push_back(newCB);
		return newCB;
	}
	/************************************************************************/
	D3D9DynVBManager::ColorVB *D3D9DynVBManager::getSpecularBuffer(			
		uint numVertices, 
		uint vertexStride) 
	{
		for (mSpeIT = mSpe.begin(); mSpeIT != mSpe.end(); mSpeIT++)
		{
			if ((*mSpeIT)->getVertexCount() >= numVertices &&
				(*mSpeIT)->getVertexStride() == vertexStride)
				return (*mSpeIT);
		}

		ColorVB *newCB = new ColorVB(mpDev, numVertices, 1, vertexStride, D3DFVF_SPECULAR);
		mSpe.push_back(newCB);
		return newCB;
	}
	/************************************************************************/
	void D3D9DynVBManager::clearInUseFlagedBuffers()
	{
		for (mTex1IT = mTex1.begin(); mTex1IT != mTex1.end(); mTex1IT++)
			(*mTex1IT)->clearInUseFlag();

		for (mTex2IT = mTex2.begin(); mTex2IT != mTex2.end(); mTex2IT++)
			(*mTex2IT)->clearInUseFlag();

		for (mTex3IT = mTex3.begin(); mTex3IT != mTex3.end(); mTex3IT++)
			(*mTex3IT)->clearInUseFlag();

		for (mTex4IT = mTex4.begin(); mTex4IT != mTex4.end(); mTex4IT++)
			(*mTex4IT)->clearInUseFlag();
	}
	/************************************************************************/
	/********************DYNAMIC INDEX BUFFER MANAGER************************/
	/************************************************************************/
	D3D9DynIBManager::D3D9DynIBManager(IDirect3DDevice9 *pDevice)
	{
		assert(pDevice);
		mpDev = pDevice;
	}
	/************************************************************************/
	D3D9DynIBManager::~D3D9DynIBManager() 
	{
		LogManager::getSingleton().logMessage( "D3D9 : Used " + StringConverter::toString((uint)mPool.size()) + " dynamic IndexBuffers " );
		for (mPoolIT = mPool.begin(); mPoolIT != mPool.end(); mPoolIT++)
			delete (*mPoolIT);
		mPool.clear();
	}
	/************************************************************************/
	D3D9DynIBManager::IndexBuffer *D3D9DynIBManager::getBuffer(uint numIndices)
	{
		for (mPoolIT = mPool.begin(); mPoolIT != mPool.end(); mPoolIT++)
		{
			if ((*mPoolIT)->getIndexCount() >= numIndices)
				return (*mPoolIT);
		}

		IndexBuffer *newIB = new IndexBuffer(mpDev, numIndices, D3DFMT_INDEX16);
		mPool.push_back(newIB);
		return newIB;
	}
	/************************************************************************/
}

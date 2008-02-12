/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#include "OgreD3D10HardwarePixelBuffer.h"
#include "OgreD3D10Texture.h"
#include "OgreD3D10Mappings.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"

#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreD3D10Texture.h"
#include "OgreD3D10Device.h"

namespace Ogre {

	//-----------------------------------------------------------------------------  

	D3D10HardwarePixelBuffer::D3D10HardwarePixelBuffer(D3D10Texture * parentTexture, D3D10Device & device, size_t subresourceIndex,
		size_t width, size_t height, size_t depth,PixelFormat format, HardwareBuffer::Usage usage):
	HardwarePixelBuffer(width, height, depth, format, usage, false, false),
		//mSurface(0), /*mVolume(0),*/ /*mTempSurface(0),*/ /*mTempVolume(0),*/
		//mDoMipmapGen(0), mHWMipmaps(0), mMipTex(0)
		mParentTexture(parentTexture),
		mDevice(device),
		mSubresourceIndex(subresourceIndex)
	{
	}
	D3D10HardwarePixelBuffer::~D3D10HardwarePixelBuffer()
	{
		destroyRenderTextures();
	}
	//-----------------------------------------------------------------------------  
	/*void D3D10HardwarePixelBuffer::bind(D3D10Driver *dev, IDXGISurface *surface, bool update)
	{
	mDevice = dev;
	mSurface = surface;
	/*
	DXGI_SURFACE_DESC desc;
	if(FAILED(mSurface->GetDesc(&desc)))
	OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Could not get surface information",
	"D3D10HardwarePixelBuffer::D3D10HardwarePixelBuffer");
	mWidth = desc.Width;
	mHeight = desc.Height;
	mDepth = 1;
	mFormat = D3D10Mappings::_getPF(desc.Format);
	// Default
	mRowPitch = mWidth;
	mSlicePitch = mHeight*mWidth;
	mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);

	if(mUsage & TU_RENDERTARGET)
	createRenderTextures(update);
	}*/
	//-----------------------------------------------------------------------------

	//-----------------------------------------------------------------------------  
	// Util functions to convert a D3D locked box to a pixel box
	void fromD3DLock(PixelBox &rval, const DXGI_MAPPED_RECT &lrect)
	{
		rval.rowPitch = lrect.Pitch / PixelUtil::getNumElemBytes(rval.format);
		rval.slicePitch = rval.rowPitch * rval.getHeight();
		assert((lrect.Pitch % PixelUtil::getNumElemBytes(rval.format))==0);
		rval.data = lrect.pBits;
	}
	/*void fromD3DLock(PixelBox &rval, const D3DLOCKED_BOX &lbox)
	{
	rval.rowPitch = lbox.RowPitch / PixelUtil::getNumElemBytes(rval.format);
	rval.slicePitch = lbox.SlicePitch / PixelUtil::getNumElemBytes(rval.format);
	assert((lbox.RowPitch % PixelUtil::getNumElemBytes(rval.format))==0);
	assert((lbox.SlicePitch % PixelUtil::getNumElemBytes(rval.format))==0);
	rval.data = lbox.pBits;
	}*/
	// Convert Ogre integer Box to D3D rectangle
	/*RECT toD3DRECT(const Box &lockBox)
	{
	RECT prect;
	assert(lockBox.getDepth() == 1);
	prect.left = lockBox.left;
	prect.right = lockBox.right;
	prect.top = lockBox.top;
	prect.bottom = lockBox.bottom;
	return prect;
	}
	// Convert Ogre integer Box to D3D box
	D3D10_BOX toD3DBOX(const Box &lockBox)
	{
	D3D10_BOX pbox;
	pbox.left = lockBox.left;
	pbox.right = lockBox.right;
	pbox.top = lockBox.top;
	pbox.bottom = lockBox.bottom;
	pbox.front = lockBox.front;
	pbox.back = lockBox.back;
	return pbox;
	}
	// Convert Ogre pixelbox extent to D3D rectangle
	RECT toD3DRECTExtent(const PixelBox &lockBox)
	{
	RECT prect;
	assert(lockBox.getDepth() == 1);
	prect.left = 0;
	prect.right = lockBox.getWidth();
	prect.top = 0;
	prect.bottom = lockBox.getHeight();
	return prect;
	}
	// Convert Ogre pixelbox extent to D3D box
	D3D10_BOX toD3DBOXExtent(const PixelBox &lockBox)
	{
	D3D10_BOX pbox;
	pbox.left = 0;
	pbox.right = lockBox.getWidth();
	pbox.top = 0;
	pbox.bottom = lockBox.getHeight();
	pbox.front = 0;
	pbox.back = lockBox.getDepth();
	return pbox;
	}*/
	//-----------------------------------------------------------------------------  
	PixelBox D3D10HardwarePixelBuffer::lockImpl(const Image::Box lockBox,  LockOptions options)
	{
		// Check for misuse
		if(mUsage & TU_RENDERTARGET)
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "DirectX does not allow locking of or directly writing to RenderTargets. Use blitFromMemory if you need the contents.",
			"D3D10HardwarePixelBuffer::lockImpl");	
		// Set extents and format
		PixelBox rval(lockBox, mFormat);
		// Set locking flags according to options
		D3D10_MAP  flags = D3D10_MAP_WRITE ;
		switch(options)
		{
		case HBL_DISCARD:
			// D3D only likes D3DLOCK_DISCARD if you created the texture with D3DUSAGE_DYNAMIC
			// debug runtime flags this up, could cause problems on some drivers
			if (mUsage & HBU_DYNAMIC)
				flags = D3D10_MAP_WRITE_DISCARD;
			break;
		case HBL_READ_ONLY:
			flags = D3D10_MAP_READ;
			break;
		default: 
			break;
		};

		// TODO - check return values here
		switch(mParentTexture->getTextureType()) {
		case TEX_TYPE_1D:
			{

				mParentTexture->GetTex1D()->Map(static_cast<UINT>(mSubresourceIndex), flags, 0, &rval.data);
			}
			break;
		case TEX_TYPE_2D:
			{
				D3D10_MAPPED_TEXTURE2D mappedTex2D;
				mParentTexture->GetTex2D()->Map(static_cast<UINT>(mSubresourceIndex), flags, 0, &mappedTex2D);
				rval.data = mappedTex2D.pData;
			}
			break;
		case TEX_TYPE_3D:
			{
				D3D10_MAPPED_TEXTURE3D mappedTex3D;
				mParentTexture->GetTex3D()->Map(static_cast<UINT>(mSubresourceIndex), flags, 0, &mappedTex3D);
				rval.data = mappedTex3D.pData;
			}
			break;
		}




		return rval;
	}
	//-----------------------------------------------------------------------------  
	void D3D10HardwarePixelBuffer::unlockImpl(void)
	{
		switch(mParentTexture->getTextureType()) {
		case TEX_TYPE_1D:
			{
				mParentTexture->GetTex1D()->Unmap(static_cast<UINT>(mSubresourceIndex));
			}
			break;
		case TEX_TYPE_2D:
			{
				mParentTexture->GetTex2D()->Unmap(static_cast<UINT>(mSubresourceIndex));
			}
			break;
		case TEX_TYPE_3D:
			{
				mParentTexture->GetTex3D()->Unmap(static_cast<UINT>(mSubresourceIndex));
			}
			break;
		}
	}
	//-----------------------------------------------------------------------------  

	void D3D10HardwarePixelBuffer::blit(const HardwarePixelBufferSharedPtr &rsrc, const Image::Box &srcBox, const Image::Box &dstBox)
	{
		/*
		D3D10HardwarePixelBuffer *src = static_cast<D3D10HardwarePixelBuffer*>(rsrc.getPointer());
		if(mSurface && src->mSurface)
		{
		// Surface-to-surface
		RECT dsrcRect = toD3DRECT(srcBox);
		RECT ddestRect = toD3DRECT(dstBox);
		// D3DXLoadSurfaceFromSurface
		if(D3DXLoadSurfaceFromSurface(
		mSurface, NULL, &ddestRect, 
		src->mSurface, NULL, &dsrcRect,
		D3DX_DEFAULT, 0) != D3D_OK)
		{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromSurface failed",
		"D3D10HardwarePixelBuffer::blit");
		}
		}
		else if(mVolume && src->mVolume)
		{
		// Volume-to-volume
		D3D10_BOX dsrcBox = toD3DBOX(srcBox);
		D3D10_BOX ddestBox = toD3DBOX(dstBox);

		// D3DXLoadVolumeFromVolume
		if(D3DXLoadVolumeFromVolume(
		mVolume, NULL, &ddestBox, 
		src->mVolume, NULL, &dsrcBox,
		D3DX_DEFAULT, 0) != D3D_OK)
		{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadVolumeFromVolume failed",
		"D3D10HardwarePixelBuffer::blit");
		}
		}
		else
		{
		// Software fallback   
		HardwarePixelBuffer::blit(rsrc, srcBox, dstBox);
		}
		*/
	}
	//-----------------------------------------------------------------------------  
	void D3D10HardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Image::Box &dstBox)
	{
		// for scoped deletion of conversion buffer
		MemoryDataStreamPtr buf;
		PixelBox converted = src;

		// convert to pixelbuffer's native format if necessary
		if (src.format != mFormat)
		{
			buf.bind(new MemoryDataStream(
				PixelUtil::getMemorySize(src.getWidth(), src.getHeight(), src.getDepth(),
				mFormat)));
			converted = PixelBox(src.getWidth(), src.getHeight(), src.getDepth(), mFormat, buf->getPtr());
			PixelUtil::bulkPixelConversion(src, converted);
		}


		switch(mParentTexture->getTextureType()) {
		case TEX_TYPE_1D:
			{

				//mParentTexture->GetTex1D()->Map(mSubresourceIndex, flags, 0, &rval.data);
			}
			break;
		case TEX_TYPE_2D:
			{
				mDevice->UpdateSubresource( 
					mParentTexture->GetTex2D(), 
					static_cast<UINT>(mSubresourceIndex),
					NULL,
					converted.data,
					static_cast<UINT>(converted.rowPitch),
					0 );
				if (mDevice.isError())
				{
					String errorDescription = mDevice.getErrorDescription();
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
						"D3D10 device cannot update subresource\nError Description:" + errorDescription,
						"D3D10HardwarePixelBuffer::blitFromMemory");
				}
			}
			break;
		case TEX_TYPE_3D:
			{
				//D3D10_MAPPED_TEXTURE3D mappedTex3D;
				//mParentTexture->GetTex3D()->Map(mSubresourceIndex, flags, 0, &mappedTex3D);
				//rval.data = mappedTex3D.pData;
			}
			break;
		}

		mDevice->GenerateMips(mParentTexture->getTexture()); // TODO - DO WE NEED THIS?
		if (mDevice.isError())
		{
			String errorDescription = mDevice.getErrorDescription();
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
				"D3D10 device cannot generate mips\nError Description:" + errorDescription,
				"D3D10HardwarePixelBuffer::blitFromMemory");
		}

		/*if(mDoMipmapGen)
		{
		mDevice->GenerateMips(mParentTexture->getTexture();

		}*/


		//D3DX10CreateTextureFromMemory
		//PixelBox lockedBuffer = lockImpl(dstBox, HBL_DISCARD);
		//	memcpy(lockedBuffer.data, src.data, src.slicePitch);
		//	unlockImpl();
		/*
		// for scoped deletion of conversion buffer
		MemoryDataStreamPtr buf;
		PixelBox converted = src;

		// convert to pixelbuffer's native format if necessary
		if (D3D10Mappings::_getPF(src.format) == D3DFMT_UNKNOWN)
		{
		buf.bind(new MemoryDataStream(
		PixelUtil::getMemorySize(src.getWidth(), src.getHeight(), src.getDepth(),
		mFormat)));
		converted = PixelBox(src.getWidth(), src.getHeight(), src.getDepth(), mFormat, buf->getPtr());
		PixelUtil::bulkPixelConversion(src, converted);
		}

		if(mSurface)
		{
		RECT destRect, srcRect;
		srcRect = toD3DRECTExtent(converted);
		destRect = toD3DRECT(dstBox);

		if(D3DXLoadSurfaceFromMemory(mSurface, NULL, &destRect, 
		converted.data, D3D10Mappings::_getPF(converted.format),
		converted.rowPitch * PixelUtil::getNumElemBytes(converted.format),
		NULL, &srcRect, D3DX_DEFAULT, 0) != D3D_OK)
		{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromMemory failed",
		"D3D10HardwarePixelBuffer::blitFromMemory");
		}
		}
		else
		{
		D3D10_BOX destBox, srcBox;
		srcBox = toD3DBOXExtent(converted);
		destBox = toD3DBOX(dstBox);

		if(D3DXLoadVolumeFromMemory(mVolume, NULL, &destBox, 
		converted.data, D3D10Mappings::_getPF(converted.format),
		converted.rowPitch * PixelUtil::getNumElemBytes(converted.format),
		converted.slicePitch * PixelUtil::getNumElemBytes(converted.format),
		NULL, &srcBox, D3DX_DEFAULT, 0) != D3D_OK)
		{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromMemory failed",
		"D3D10HardwarePixelBuffer::blitFromMemory");
		}
		}
		if(mDoMipmapGen)
		_genMipmaps();
		*/
	}
	//-----------------------------------------------------------------------------  
	void D3D10HardwarePixelBuffer::blitToMemory(const Image::Box &srcBox, const PixelBox &dst)
	{
		/*
		// Decide on pixel format of temp surface
		PixelFormat tmpFormat = mFormat; 
		if(D3D10Mappings::_getPF(dst.format) != D3DFMT_UNKNOWN)
		{
		tmpFormat = dst.format;
		}
		if(mSurface)
		{
		assert(srcBox.getDepth() == 1 && dst.getDepth() == 1);
		// Create temp texture
		ID3D10Resource  *tmp;
		IDirect3DSurface9 *surface;

		if(D3DXCreateTexture(
		mDevice,
		dst.getWidth(), dst.getHeight(), 
		1, // 1 mip level ie topmost, generate no mipmaps
		0, D3D10Mappings::_getPF(tmpFormat), D3DPOOL_SCRATCH,
		&tmp
		) != D3D_OK)
		{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Create temporary texture failed",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		if(tmp->GetSurfaceLevel(0, &surface) != D3D_OK)
		{
		tmp->Release();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Get surface level failed",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		// Copy texture to this temp surface
		RECT destRect, srcRect;
		srcRect = toD3DRECT(srcBox);
		destRect = toD3DRECTExtent(dst);

		if(D3DXLoadSurfaceFromSurface(
		surface, NULL, &destRect, 
		mSurface, NULL, &srcRect,
		D3DX_DEFAULT, 0) != D3D_OK)
		{
		surface->Release();
		tmp->Release();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromSurface failed",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		// Lock temp surface and copy it to memory
		DXGI_MAPPED_RECT lrect; // Filled in by D3D
		if(surface->LockRect(&lrect, NULL,  D3DLOCK_READONLY) != D3D_OK)
		{
		surface->Release();
		tmp->Release();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "surface->LockRect",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		// Copy it
		PixelBox locked(dst.getWidth(), dst.getHeight(), dst.getDepth(), tmpFormat);
		fromD3DLock(locked, lrect);
		PixelUtil::bulkPixelConversion(locked, dst);
		surface->UnlockRect();
		// Release temporary surface and texture
		surface->Release();
		tmp->Release();
		}
		else
		{
		// Create temp texture
		IDirect3DVolumeTexture9 *tmp;
		IDirect3DVolume9 *surface;

		if(D3DXCreateVolumeTexture(
		mDevice,
		dst.getWidth(), dst.getHeight(), dst.getDepth(), 0,
		0, D3D10Mappings::_getPF(tmpFormat), D3DPOOL_SCRATCH,
		&tmp
		) != D3D_OK)
		{
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Create temporary texture failed",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		if(tmp->GetVolumeLevel(0, &surface) != D3D_OK)
		{
		tmp->Release();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Get volume level failed",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		// Volume
		D3D10_BOX ddestBox, dsrcBox;
		ddestBox = toD3DBOXExtent(dst);
		dsrcBox = toD3DBOX(srcBox);

		if(D3DXLoadVolumeFromVolume(
		surface, NULL, &ddestBox, 
		mVolume, NULL, &dsrcBox,
		D3DX_DEFAULT, 0) != D3D_OK)
		{
		surface->Release();
		tmp->Release();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadVolumeFromVolume failed",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		// Lock temp surface and copy it to memory
		D3DLOCKED_BOX lbox; // Filled in by D3D
		if(surface->LockBox(&lbox, NULL,  D3DLOCK_READONLY) != D3D_OK)
		{
		surface->Release();
		tmp->Release();
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "surface->LockBox",
		"D3D10HardwarePixelBuffer::blitToMemory");
		}
		// Copy it
		PixelBox locked(dst.getWidth(), dst.getHeight(), dst.getDepth(), tmpFormat);
		fromD3DLock(locked, lbox);
		PixelUtil::bulkPixelConversion(locked, dst);
		surface->UnlockBox();
		// Release temporary surface and texture
		surface->Release();
		tmp->Release();
		}
		*/
	}

	//-----------------------------------------------------------------------------  
	void D3D10HardwarePixelBuffer::_genMipmaps()
	{
		//	assert(mMipTex);
		// Mipmapping
		//if (mHWMipmaps)
		{
			// Hardware mipmaps
			//	mDevice->getD3DDevice()->CreateTexture2D()  (mMipTex);
		}
		/*else
		{
		// Software mipmaps
		if( D3DXFilterTexture( mMipTex, NULL, D3DX_DEFAULT, D3DX_DEFAULT ) != D3D_OK )
		{
		OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, 
		"Failed to filter texture (generate mipmaps)",
		"D3D10HardwarePixelBuffer::_genMipmaps" );
		}
		}*/

	}
	//----------------------------------------------------------------------------- 
	void D3D10HardwarePixelBuffer::_setMipmapping(bool doMipmapGen, bool HWMipmaps, ID3D10Resource *mipTex)
	{
		//	mDoMipmapGen = doMipmapGen;
		//	mHWMipmaps = HWMipmaps;
		//	mMipTex = mipTex;
	}
	//-----------------------------------------------------------------------------    
	RenderTexture *D3D10HardwarePixelBuffer::getRenderTarget(size_t zoffset)
	{
		assert(mUsage & TU_RENDERTARGET);
		assert(zoffset < mDepth);
		return mSliceTRT[zoffset];
	}
	//-----------------------------------------------------------------------------    
	void D3D10HardwarePixelBuffer::createRenderTextures(bool update)
	{
		/*    if (update)
		{
		assert(mSliceTRT.size() == mDepth);
		for (SliceTRT::const_iterator it = mSliceTRT.begin(); it != mSliceTRT.end(); ++it)
		{
		D3D10RenderTexture *trt = static_cast<D3D10RenderTexture*>(*it);
		trt->rebind(this);
		}
		return;
		}

		destroyRenderTextures();
		if(!mSurface)
		{
		OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, 
		"Rendering to 3D slices not supported yet for Direct3D",
		"D3D10HardwarePixelBuffer::createRenderTexture");
		}
		// Create render target for each slice
		mSliceTRT.reserve(mDepth);
		assert(mDepth==1);
		for(size_t zoffset=0; zoffset<mDepth; ++zoffset)
		{
		String name;
		name = "rtt/"+Ogre::StringConverter::toString((size_t)mSurface);

		RenderTexture *trt = new D3D10RenderTexture(name, this);
		mSliceTRT.push_back(trt);
		Root::getSingleton().getRenderSystem()->attachRenderTarget(*trt);
		}
		*/
	}
	//-----------------------------------------------------------------------------    
	void D3D10HardwarePixelBuffer::destroyRenderTextures()
	{
		/*if(mSliceTRT.empty())
		return;
		// Delete all render targets that are not yet deleted via _clearSliceRTT
		for(size_t zoffset=0; zoffset<mDepth; ++zoffset)
		{
		if(mSliceTRT[zoffset])
		Root::getSingleton().getRenderSystem()->destroyRenderTarget(mSliceTRT[zoffset]->getName());
		}*/
	}

	D3D10Texture * D3D10HardwarePixelBuffer::getParentTexture() const
	{
		return mParentTexture;
	}
};

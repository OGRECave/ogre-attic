/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright © 2000-2005 The OGRE Team
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
#include "OgreD3D9HardwarePixelBuffer.h"
#include "OgreD3D9Texture.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"

#include "OgreNoMemoryMacros.h"
#include <d3dx9.h>
#include <dxerr9.h>
#include "OgreMemoryMacros.h"

namespace Ogre {

//-----------------------------------------------------------------------------  

D3D9HardwarePixelBuffer::D3D9HardwarePixelBuffer(HardwareBuffer::Usage usage):
	HardwarePixelBuffer(0, 0, 0, PF_UNKNOWN, usage, false, false),
	mSurface(0), mVolume(0), mTempSurface(0), mTempVolume(0),
	mDoMipmapGen(0), mHWMipmaps(0), mMipTex(0)
{
}
D3D9HardwarePixelBuffer::~D3D9HardwarePixelBuffer()
{
}
//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::bind(IDirect3DSurface9 *surface)
{
	mSurface = surface;
	D3DSURFACE_DESC desc;
	if(mSurface->GetDesc(&desc) != D3D_OK)
		Except(Exception::ERR_RENDERINGAPI_ERROR, "Could not get surface information",
		 "D3D9HardwarePixelBuffer::D3D9HardwarePixelBuffer");
	mWidth = desc.Width;
	mHeight = desc.Height;
	mDepth = 1;
	mFormat = D3D9Texture::_getPF(desc.Format);
	// Default
	mRowPitch = mWidth;
	mSlicePitch = mHeight*mWidth;
	mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
}
//-----------------------------------------------------------------------------
void D3D9HardwarePixelBuffer::bind(IDirect3DVolume9 *volume)
{
	mVolume = volume;
	D3DVOLUME_DESC desc;
	if(mVolume->GetDesc(&desc) != D3D_OK)
		Except(Exception::ERR_RENDERINGAPI_ERROR, "Could not get volume information",
		 "D3D9HardwarePixelBuffer::D3D9HardwarePixelBuffer");
	mWidth = desc.Width;
	mHeight = desc.Height;
	mDepth = desc.Depth;
	mFormat = D3D9Texture::_getPF(desc.Format);
	// Default
	mRowPitch = mWidth;
	mSlicePitch = mHeight*mWidth;
	mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);
}
//-----------------------------------------------------------------------------  
PixelBox D3D9HardwarePixelBuffer::lockImpl(const Image::Box lockBox,  LockOptions options)
{
	// Set extents and format
	PixelBox rval(lockBox, mFormat);
	// Set locking flags according to options
	DWORD flags = 0;
	switch(options)
	{
	case HBL_DISCARD:
		// D3D only likes D3DLOCK_DISCARD if you created the texture with D3DUSAGE_DYNAMIC
		// debug runtime flags this up, could cause problems on some drivers
		if (mUsage & HBU_DYNAMIC)
			flags |= D3DLOCK_DISCARD;
		break;
	case HBL_READ_ONLY:
		flags |= D3DLOCK_READONLY;
		break;
	default: 
		break;
	};
	
	if(mSurface) 
	{
		// Surface
		D3DLOCKED_RECT lrect; // Filled in by D3D
		HRESULT hr;

		if (lockBox.left == 0 && lockBox.top == 0 
			&& lockBox.right == mWidth && lockBox.bottom == mHeight)
		{
			// Lock whole surface
			hr = mSurface->LockRect(&lrect, NULL, flags);
		}
		else
		{
			RECT prect; // specify range to lock
			prect.left = lockBox.left;
			prect.right = lockBox.right;
			prect.top = lockBox.top;
			prect.bottom = lockBox.bottom;
			hr = mSurface->LockRect(&lrect, &prect, flags);
		}
		if (FAILED(hr))		
			Except(Exception::ERR_RENDERINGAPI_ERROR, "Surface locking failed",
		 		"D3D9HardwarePixelBuffer::lockImpl");
		
		rval.rowPitch = lrect.Pitch / PixelUtil::getNumElemBytes(mFormat);
		rval.slicePitch = rval.rowPitch * mHeight;
		rval.data = lrect.pBits;
	} 
	else 
	{
		// Volume
		D3DBOX pbox; // specify range to lock
		D3DLOCKED_BOX lbox; // Filled in by D3D
		pbox.Left = lockBox.left;
		pbox.Right = lockBox.right;
		pbox.Top = lockBox.top;
		pbox.Bottom = lockBox.bottom;
		pbox.Front = lockBox.front;
		pbox.Back = lockBox.back;
		
		if(mVolume->LockBox(&lbox, &pbox, flags) != D3D_OK)
			Except(Exception::ERR_RENDERINGAPI_ERROR, "Volume locking failed",
		 		"D3D9HardwarePixelBuffer::lockImpl");
		
		rval.rowPitch = lbox.RowPitch / PixelUtil::getNumElemBytes(mFormat);
		rval.slicePitch = lbox.SlicePitch / PixelUtil::getNumElemBytes(mFormat);
		rval.data = lbox.pBits;
	}

	return rval;
}
//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::unlockImpl(void)
{
	if(mSurface) 
	{
		// Surface
		mSurface->UnlockRect();
	} else {
		// Volume
		mVolume->UnlockBox();
	}
	if(mDoMipmapGen)
		_genMipmaps();
}
//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::blit(HardwarePixelBuffer *src, const Image::Box &srcBox, const Image::Box &dstBox)
{
	// TODO
	// D3DXLoadSurfaceFromSurface
	// D3DXLoadVolumeFromVolume
	HardwarePixelBuffer::blit(src, srcBox, dstBox);

}
//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::blitFromMemory(const PixelBox &src, const Image::Box &dstBox)
{
	// for scoped deletion of conversion buffer
	MemoryDataStreamPtr buf;
	PixelBox converted = src;

	// convert to pixelbuffer's native format if necessary
	if (D3D9Texture::_getPF(src.format) == D3DFMT_UNKNOWN)
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
		srcRect.left = 0;
		srcRect.top = 0;
		srcRect.right = converted.getWidth();
		srcRect.bottom = converted.getHeight();
		
		destRect.left = dstBox.left;
		destRect.top = dstBox.top;
		destRect.right = dstBox.right;
		destRect.bottom = dstBox.bottom;
		
		if(D3DXLoadSurfaceFromMemory(mSurface, NULL, &destRect, 
			converted.data, D3D9Texture::_getPF(converted.format),
			converted.rowPitch * PixelUtil::getNumElemBytes(converted.format),
			NULL, &srcRect, D3DX_DEFAULT, 0) != D3D_OK)
		{
			Except(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromMemory failed",
		 		"D3D9HardwarePixelBuffer::blitFromMemory");
		}
	}
	else
	{
		D3DBOX destBox, srcBox;
		srcBox.Left = 0;
		srcBox.Top = 0;
		srcBox.Front = 0;
		srcBox.Right = converted.getWidth();
		srcBox.Bottom = converted.getHeight();
		srcBox.Back = converted.getDepth();
		
		destBox.Left = dstBox.left;
		destBox.Top = dstBox.top;
		destBox.Right = dstBox.right;
		destBox.Bottom = dstBox.bottom;
		destBox.Front = dstBox.front;
		destBox.Back = dstBox.back;
		
		if(D3DXLoadVolumeFromMemory(mVolume, NULL, &destBox, 
			converted.data, D3D9Texture::_getPF(converted.format),
			converted.rowPitch * PixelUtil::getNumElemBytes(converted.format),
			converted.slicePitch * PixelUtil::getNumElemBytes(converted.format),
			NULL, &srcBox, D3DX_DEFAULT, 0) != D3D_OK)
		{
			Except(Exception::ERR_RENDERINGAPI_ERROR, "D3DXLoadSurfaceFromMemory failed",
		 		"D3D9HardwarePixelBuffer::blitFromMemory");
		}
	}
	if(mDoMipmapGen)
		_genMipmaps();
}
//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::blitToMemory(const Image::Box &srcBox, const PixelBox &dst)
{
	// TODO
}
//-----------------------------------------------------------------------------  
void D3D9HardwarePixelBuffer::_genMipmaps()
{
	assert(mMipTex);
	// Mipmapping
	if (mHWMipmaps)
	{
		// Hardware mipmaps
		mMipTex->GenerateMipSubLevels();
	}
	else
	{
		// Software mipmaps
		if( D3DXFilterTexture( mMipTex, NULL, D3DX_DEFAULT, D3DX_DEFAULT ) != D3D_OK )
		{
			Except( Exception::ERR_RENDERINGAPI_ERROR, 
			"Failed to filter texture (generate mip maps)",
			 "D3D9HardwarePixelBuffer::_genMipmaps" );
		}
	}

}
//----------------------------------------------------------------------------- 
void D3D9HardwarePixelBuffer::_setMipmapping(bool doMipmapGen, bool HWMipmaps, IDirect3DBaseTexture9 *mipTex)
{
	mDoMipmapGen = doMipmapGen;
	mHWMipmaps = HWMipmaps;
	mMipTex = mipTex;
}


};

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
#ifndef __D3D9PIXELBUFFER_H__
#define __D3D9PIXELBUFFER_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreHardwarePixelBuffer.h"

#include <d3d9.h>
#include <d3dx9.h>
namespace Ogre {

	class D3D9HardwarePixelBuffer: public HardwarePixelBuffer
	{
	protected:
		/// Lock a box
		PixelBox lockImpl(const Image::Box lockBox,  LockOptions options);

		/// Unlock a box
		void unlockImpl(void);
		
		/// Surface abstracted by this buffer
		IDirect3DSurface9 *mSurface;
		/// Volume abstracted by this buffer
		IDirect3DVolume9 *mVolume;
		/// Temporary surface in main memory if direct locking of mSurface is not possible
		IDirect3DSurface9 *mTempSurface;
		/// Temporary volume in main memory if direct locking of mVolume is not possible
		IDirect3DVolume9 *mTempVolume;
	public:
		D3D9HardwarePixelBuffer(IDirect3DSurface9 *pSurface);
		D3D9HardwarePixelBuffer(IDirect3DVolume9 *pVolume);
		
		/// @copydoc HardwarePixelBuffer::blit
		void blit(const Image::Box &srcBox, HardwarePixelBuffer *dst, const Image::Box &dstBox);
		
		/// @copydoc HardwarePixelBuffer::blitFromMemory
		void blitFromMemory(const PixelBox &src, const Image::Box &dstBox);
		
		/// @copydoc HardwarePixelBuffer::blitToMemory
		void blitToMemory(const Image::Box &srcBox, const PixelBox &dst);
		
		~D3D9HardwarePixelBuffer();
	};

};

#endif

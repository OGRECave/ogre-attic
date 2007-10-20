/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#ifndef __D3D10PIXELBUFFER_H__
#define __D3D10PIXELBUFFER_H__

#include "OgreD3D10Prerequisites.h"
#include "OgreHardwarePixelBuffer.h"

#include <d3d10.h>
#include <d3dx10.h>
namespace Ogre {

	class D3D10HardwarePixelBuffer: public HardwarePixelBuffer
	{
	protected:
		/// Lock a box
		PixelBox lockImpl(const Image::Box lockBox,  LockOptions options);

		/// Unlock a box
		void unlockImpl(void);

		/// Create (or update) render textures for slices
		void createRenderTextures(bool update);
		/// Destroy render textures for slices
		void destroyRenderTextures();
		
		/// D3DDevice pointer
		ID3D10Device *mpDev;
		
		/// 1D texture
		ID3D10Texture1D *mTex1D;
		/// 2D texture (inc cubemap surfaces)
		ID3D10Texture2D *mTex2D;
		/// 3D texture
		ID3D10Texture3D *mTex3D;
		// Index of the subresource to use (mipmap, cube face)
		uint mSubresourceIndex;
		
		/// Mipmapping
		bool mDoMipmapGen;
		bool mHWMipmaps;
		ID3D10Resource *mMipTex;

		/// Render targets
		typedef std::vector<RenderTexture*> SliceTRT;
        SliceTRT mSliceTRT;
	public:
		D3D10HardwarePixelBuffer(HardwareBuffer::Usage usage);
		
		/// Call this to associate a D3D surface or volume with this pixel buffer
		void bind(ID3D10Device *dev, ID3D10Texture1D *tex, uint subresindex, bool update);
		void bind(ID3D10Device *dev, ID3D10Texture2D *tex, uint subresindex, bool update);
		void bind(ID3D10Device *dev, ID3D10Texture3D *tex, uint subresindex, bool update);
		
		/// @copydoc HardwarePixelBuffer::blit
        void blit(const HardwarePixelBufferSharedPtr &src, const Image::Box &srcBox, const Image::Box &dstBox);
		
		/// @copydoc HardwarePixelBuffer::blitFromMemory
		void blitFromMemory(const PixelBox &src, const Image::Box &dstBox);
		
		/// @copydoc HardwarePixelBuffer::blitToMemory
		void blitToMemory(const Image::Box &srcBox, const PixelBox &dst);
		
		/// Internal function to update mipmaps on update of level 0
		void _genMipmaps();
		
		/// Function to set mipmap generation
		void _setMipmapping(bool doMipmapGen, bool HWMipmaps, ID3D10Resource *mipTex);
		
		~D3D10HardwarePixelBuffer();

		/// Get rendertarget for z slice
		RenderTexture *getRenderTarget(size_t zoffset);

		/// Accessor for surface
		ID3D10Texture1D *getTexture1D() { return mTex1D; }
		ID3D10Texture2D *getTexture2D() { return mTex2D; }
		ID3D10Texture3D *getTexture3D() { return mTex3D; }

		/// Notify TextureBuffer of destruction of render target
        virtual void _clearSliceRTT(size_t zoffset)
        {
            mSliceTRT[zoffset] = 0;
        }
	};
};
#endif

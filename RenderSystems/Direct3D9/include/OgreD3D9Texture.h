/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2002 The OGRE Team
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
#ifndef __D3D8TEXTURE_H__
#define __D3D8TEXTURE_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreTexture.h"
#include "OgreRenderTexture.h"
#include "OgreImage.h"
#include "OgreException.h"

#include "OgreNoMemoryMacros.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include "OgreMemoryMacros.h"

namespace Ogre {
	class D3D9Texture : public Texture
	{
	protected:
		/// D3DDevice pointer
		IDirect3DDevice9		*mpDev;		
		/// D3D9 pointer
		IDirect3D9				*mpD3D;
		/// 1D/2D normal texture pointer
		IDirect3DTexture9		*mpNormTex;	
		/// cubic texture pointer
		IDirect3DCubeTexture9	*mpCubeTex;	
        /// Volume texture
        IDirect3DVolumeTexture9 *mpVolumeTex;
        /// z-buffer for the render surface pointer
		IDirect3DSurface9		*mpZBuff;	
		/// actual texture pointer
		IDirect3DBaseTexture9	*mpTex;		

		/// cube texture individual face names
		String							mCubeFaceNames[6];
		/// device creation parameters
		D3DDEVICE_CREATION_PARAMETERS	mDevCreParams;
		/// back buffer pixel format
		D3DFORMAT						mBBPixelFormat;
		/// device capabilities pointer
		D3DCAPS9						mDevCaps;
        // Auto-generated mipmaps?
        bool                            mAutoGenMipMaps;
	
        /// Initialise the device and get formats
        void _initDevice(void);
		/// internal method, load a cube texture
		void _loadCubeTex();
		/// internal method, load a normal texture
		void _loadNormTex();
		/// internal method, load a volume texture
		void _loadVolumeTex();

		/// internal method, create a blank normal 1D/2D texture
		void _createNormTex();
		/// internal method, create a blank cube texture
		void _createCubeTex();

		/// internal method, return a D3D pixel format for texture creation
		D3DFORMAT _chooseD3DFormat();
		/// internal method, return the color masks for a given format
		void _getColorMasks(D3DFORMAT format, DWORD *pdwRed, DWORD *pdwGreen, DWORD *pdwBlue, DWORD *pdwAlpha, DWORD *pdwRGBBitCount);
		/// internal method, copy a memory block to the given surface
		void _copyMemoryToSurface( const unsigned char *pBuffer, IDirect3DSurface9 *pSurface );
		/// internal method, blits a given image to normal textures
		void _blitImageToNormTex(const Image &srcImage);
		/// internal method, blits images to cube textures
		void _blitImagesToCubeTex(const Image srcImages[]);

		/// internal method, convert D3D9 pixel format to Ogre pixel format
		static PixelFormat _getPF(D3DFORMAT d3dPF);
		/// internal method, convert Ogre pixel format to D3D9 pixel format
		static D3DFORMAT _getPF(PixelFormat ogrePF);

		/// internal method, free D3D9 resources
		void _freeResources();
		/// internal method, construct full cube texture face names from a given string
		void _constructCubeFaceNames(const String& name);
		/// internal method, set Texture class source image protected attributes
		void _setSrcAttributes(unsigned long width, unsigned long height, unsigned long depth, PixelFormat format);
		/// internal method, set Texture class final texture protected attributes
		void _setFinalAttributes(unsigned long width, unsigned long height, unsigned long depth, PixelFormat format);
		/// internal method, return the best by hardware supported filter method
		D3DTEXTUREFILTERTYPE _getBestFilterMethod();
		/// internal method, return true if the device/texture combination can auto gen. mip maps
		bool _canAutoGenMipMaps(DWORD srcUsage, D3DRESOURCETYPE srcType, D3DFORMAT srcFormat);
		/// internal method, create a depth stencil for the render target texture
		void _createDepthStencil();

		/// internal method, the cube map face name for the spec. face index
		String _getCubeFaceName(unsigned char face) const
		{ assert(face < 6); return mCubeFaceNames[face]; }
		/// internal method, return the BPP for the specified format
		static unsigned short _getPFBpp(PixelFormat ogrePF)
		{ return Image::getNumElemBits(ogrePF); }

        /// overriden from Resource
        void loadImpl();
        /// overriden from Resource
        void unloadImpl();
	public:
		/// constructor 
        D3D9Texture(ResourceManager* creator, const String& name, ResourceHandle handle,
            const String& group, bool isManual, ManualResourceLoader* loader, 
            IDirect3DDevice9 *pD3DDevice);
		/// destructor
		~D3D9Texture();

		/// overriden from Texture
		void blitImage(const Image& src, const Image::Rect imgRect, const Image::Rect texRect );
		/// overriden from Texture
        void blitToTexture( const Image &src, unsigned uStartX, unsigned uStartY );
		/// overriden from Texture
		void copyToTexture( TexturePtr& target );
		/// overriden from Texture
		void loadImage( const Image &img );

        /// @copydoc Texture::createInternalResources
        void createInternalResources(void);

		/// retrieves a pointer to the actual texture
		IDirect3DBaseTexture9 *getTexture() 
		{ assert(mpTex); return mpTex; }
		/// retrieves a pointer to the normal 1D/2D texture
		IDirect3DTexture9 *getNormTexture()
		{ assert(mpNormTex); return mpNormTex; }
		/// retrieves a pointer to the cube texture
		IDirect3DCubeTexture9 *getCubeTexture()
		{ assert(mpCubeTex); return mpCubeTex; }
		/// retrieves a pointer to the Depth stencil
		IDirect3DSurface9 *getDepthStencil() 
		{ assert(mpZBuff); return mpZBuff; }
    };

    /** Specialisation of SharedPtr to allow SharedPtr to be assigned to D3D9TexturePtr 
    @note Has to be a subclass since we need operator=.
    We could templatise this instead of repeating per Resource subclass, 
    except to do so requires a form VC6 does not support i.e.
    ResourceSubclassPtr<T> : public SharedPtr<T>
    */
    class _OgreExport D3D9TexturePtr : public SharedPtr<D3D9Texture> 
    {
    public:
        D3D9TexturePtr() : SharedPtr<D3D9Texture>() {}
        D3D9TexturePtr(D3D9Texture* rep) : SharedPtr<D3D9Texture>(rep) {}
        D3D9TexturePtr(const D3D9TexturePtr& r) : SharedPtr<D3D9Texture>(r) {} 
        D3D9TexturePtr(const ResourcePtr& r) : SharedPtr<D3D9Texture>()
        {
            pRep = static_cast<D3D9Texture*>(r.getPointer());
            pUseCount = r.useCountPointer();
            if (pUseCount)
            {
                ++(*pUseCount);
            }
        }

        /// Operator used to convert a ResourcePtr to a D3D9TexturePtr
        D3D9TexturePtr& operator=(const ResourcePtr& r)
        {
            if (pRep == static_cast<D3D9Texture*>(r.getPointer()))
                return *this;
            release();
            pRep = static_cast<D3D9Texture*>(r.getPointer());
            pUseCount = r.useCountPointer();
            if (pUseCount)
            {
                ++(*pUseCount);
            }
            return *this;
        }
    };


    /// RenderTexture implementation for D3D9
    class D3D9RenderTexture : public RenderTexture
    {
    public:
        D3D9RenderTexture( const String & name, uint width, uint height, TextureType texType = TEX_TYPE_2D ) : RenderTexture( name, width, height, texType )
        {
            mPrivateTex = TextureManager::getSingleton().createManual( mName + "_PRIVATE##", texType, mWidth, mHeight, 0, PF_R8G8B8, TU_RENDERTARGET );
        }
		
        ~D3D9RenderTexture()
        {
			mPrivateTex->unload();
        }

		virtual void getCustomAttribute( const String& name, void *pData )
        {
			if( name == "DDBACKBUFFER" )
            {
                IDirect3DSurface9 ** pSurf = (IDirect3DSurface9 **)pData;
				if (mPrivateTex->getTextureType() == TEX_TYPE_2D)
					mPrivateTex->getNormTexture()->GetSurfaceLevel( 0, &(*pSurf) );
				else if (mPrivateTex->getTextureType() == TEX_TYPE_CUBE_MAP)
					mPrivateTex->getCubeTexture()->GetCubeMapSurface( (D3DCUBEMAP_FACES)0, 0, &(*pSurf) );
				else
				{
					Except( Exception::UNIMPLEMENTED_FEATURE, 
							"getCustomAttribute is implemented only for 2D and cube textures !!!", 
							"D3D9RenderTexture::getCustomAttribute" );
				}
                (*pSurf)->Release();
                return;
            }
            else if( name == "D3DZBUFFER" )
            {
                IDirect3DSurface9 ** pSurf = (IDirect3DSurface9 **)pData;
                *pSurf = mPrivateTex->getDepthStencil();
                return;
            }
            else if( name == "DDFRONTBUFFER" )
            {
                IDirect3DSurface9 ** pSurf = (IDirect3DSurface9 **)pData;
				if (mPrivateTex->getTextureType() == TEX_TYPE_2D)
					mPrivateTex->getNormTexture()->GetSurfaceLevel( 0, &(*pSurf) );
				else if (mPrivateTex->getTextureType() == TEX_TYPE_CUBE_MAP)
					mPrivateTex->getCubeTexture()->GetCubeMapSurface( (D3DCUBEMAP_FACES)0, 0, &(*pSurf) );
				else
				{
					Except( Exception::UNIMPLEMENTED_FEATURE, 
							"getCustomAttribute is implemented only for 2D and cube textures !!!", 
							"D3D9RenderTexture::getCustomAttribute" );
				}
                (*pSurf)->Release();
                return;
            }
            else if( name == "HWND" )
            {
                HWND *pHwnd = (HWND*)pData;
                *pHwnd = NULL;
                return;
            }
            else if( name == "isTexture" )
            {
                bool *b = reinterpret_cast< bool * >( pData );
                *b = true;
                return;
            }
        }

		bool requiresTextureFlipping() const { return false; }
        virtual void writeContentsToFile( const String & filename ) {}

    protected:
        /// The texture to which rendering takes place.
        D3D9TexturePtr mPrivateTex;
		
        virtual void _copyToTexture()
        {
            // Copy the newly-rendered data to the public texture surface.
            mPrivateTex->copyToTexture( mTexture );
        }
    };
}

#endif

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
#include "OgreD3D9Texture.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreBitwise.h"

#include "OgreNoMemoryMacros.h"
#include <d3dx9.h>
#include <dxerr9.h>
#include "OgreMemoryMacros.h"

namespace Ogre 
{
	/****************************************************************************************/
	D3D9Texture::D3D9Texture( const String& name, TextureType texType, IDirect3DDevice9 *pD3DDevice, TextureUsage usage )
	{
		// normal constructor
		this->_initMembers();
		// set the device and caps/formats
		this->_setDevice(pD3DDevice);

		mName = name;
		mTextureType = texType;
		mUsage = usage;

		if (this->getTextureType() == TEX_TYPE_CUBE_MAP)
			_constructCubeFaceNames(mName);
	}
	/****************************************************************************************/
	D3D9Texture::D3D9Texture( const String& name, TextureType texType, IDirect3DDevice9 *pD3DDevice, uint width, uint height, uint numMips, PixelFormat format, TextureUsage usage )
	{
		// this constructor is mainly used for RTT
		this->_initMembers();
		// set the device and caps/formats
		this->_setDevice(pD3DDevice);

		mName = name;
		mTextureType = texType;
		mUsage = usage;
		mNumMipMaps = numMips;

		if (this->getTextureType() == TEX_TYPE_CUBE_MAP)
			_constructCubeFaceNames(mName);

		this->_setSrcAttributes(width, height, format);
		// if it's a render target we must 
		// create it right now, don't know why ???
		if (mUsage == TU_RENDERTARGET)
		{
			this->_createTex();
			mIsLoaded = true;
		}
	}
	/****************************************************************************************/
	D3D9Texture::~D3D9Texture()
	{
		if (this->isLoaded())
			unload();
		SAFE_RELEASE(mpD3D);
	}
	/****************************************************************************************/
	void D3D9Texture::blitImage(const Image& src, const Image::Rect imgRect, const Image::Rect texRect )
	{
		Except(	Exception::UNIMPLEMENTED_FEATURE, "**** Blit image called but not implemented!!! ****", "D3D9Texture::blitImage" );
	}
	/****************************************************************************************/
	void D3D9Texture::blitToTexture( const Image &src, unsigned uStartX, unsigned uStartY )
	{
		Except( Exception::UNIMPLEMENTED_FEATURE, "**** Blit to texture called but not implemented!!! ****", "D3D9Texture::blitToTexture" );
	}
	/****************************************************************************************/
	void D3D9Texture::copyToTexture(Texture *target)
	{
        // check if this & target are the same format and type
		// blitting from or to cube textures is not supported yet
		if (target->getUsage() != this->getUsage() ||
			target->getTextureType() != this->getTextureType())
		{
			Except( Exception::ERR_INVALIDPARAMS, 
					"Src. and dest. textures must be of same type and must have the same usage !!!", 
					"D3D9Texture::copyToTexture" );
		}

        HRESULT hr;
        D3D9Texture *other;
		// get the target
		other = reinterpret_cast< D3D9Texture * >( target );
		// target rectangle (whole surface)
		RECT dstRC = {0, 0, other->getWidth(), other->getHeight()};

		// do it plain for normal texture
		if (this->getTextureType() == TEX_TYPE_2D)
		{
			// get our source surface
			IDirect3DSurface9 *pSrcSurface;
			if( FAILED( hr = mpNormTex->GetSurfaceLevel(0, &pSrcSurface) ) )
			{
				String msg = DXGetErrorDescription9(hr);
				Except( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
			}

			// get our target surface
			IDirect3DSurface9 *pDstSurface;
			IDirect3DTexture9 *pOthTex = other->getNormTexture();
			if( FAILED( hr = pOthTex->GetSurfaceLevel(0, &pDstSurface) ) )
			{
				String msg = DXGetErrorDescription9(hr);
				Except( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
				SAFE_RELEASE(pSrcSurface);
			}

			// target rectangle (whole surface)
			RECT dstRC = {0, 0, other->getWidth(), other->getHeight()};
			// do the blit, it's called StretchRect in D3D9 :)
			if( FAILED( hr = mpDev->StretchRect( pSrcSurface, NULL, pDstSurface, &dstRC, D3DTEXF_NONE) ) )
			{
				String msg = DXGetErrorDescription9(hr);
				Except( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
				SAFE_RELEASE(pSrcSurface);
				SAFE_RELEASE(pDstSurface);
			}

			// release temp. surfaces
			SAFE_RELEASE(pSrcSurface);
			SAFE_RELEASE(pDstSurface);
		}
		else if (this->getTextureType() == TEX_TYPE_CUBE_MAP)
		{
			// get the target cube texture
			IDirect3DCubeTexture9 *pOthTex = other->getCubeTexture();
			// blit to 6 cube faces
			for (int face = 0; face < 6; face++)
			{
				// get our source surface
				IDirect3DSurface9 *pSrcSurface;
				if( FAILED( hr = mpCubeTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, 0, &pSrcSurface) ) )
				{
					String msg = DXGetErrorDescription9(hr);
					Except( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
				}

				// get our target surface
				IDirect3DSurface9 *pDstSurface;
				if( FAILED( hr = pOthTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, 0, &pDstSurface) ) )
				{
					String msg = DXGetErrorDescription9(hr);
					Except( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
					SAFE_RELEASE(pSrcSurface);
				}

				// do the blit, it's called StretchRect in D3D9 :)
				if( FAILED( hr = mpDev->StretchRect( pSrcSurface, NULL, pDstSurface, &dstRC, D3DTEXF_NONE) ) )
				{
					String msg = DXGetErrorDescription9(hr);
					Except( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
					SAFE_RELEASE(pSrcSurface);
					SAFE_RELEASE(pDstSurface);
				}

				// release temp. surfaces
				SAFE_RELEASE(pSrcSurface);
				SAFE_RELEASE(pDstSurface);
			}
		}
		else
		{
			Except( Exception::UNIMPLEMENTED_FEATURE, 
					"Copy to texture is implemented only for 2D and cube textures !!!", 
					"D3D9Texture::copyToTexture" );
		}
	}
	/****************************************************************************************/
	void D3D9Texture::loadImage( const Image &img )
	{
		assert(this->getTextureType() == TEX_TYPE_1D || this->getTextureType() == TEX_TYPE_2D);

		Image tImage(img); // our temp image
		HRESULT hr = S_OK; // D3D9 methods result

		// we need src image info
		this->_setSrcAttributes(tImage.getWidth(), tImage.getHeight(), tImage.getFormat());
		// create a blank texture
		this->_createNormTex();
		// set gamma prior to blitting
        Image::applyGamma(tImage.getData(), this->getGamma(), (uint)tImage.getSize(), tImage.getBPP());
		this->_blitImageToNormTex(tImage);
		// get topLevel surface description
		mpNormTex->GetLevelDesc(0, &mTexDesc);
		mIsLoaded = true;
	}
	/****************************************************************************************/
	void D3D9Texture::load()
	{
		// unload if loaded
		if (this->isLoaded())
			unload();
		
		if (mUsage == TU_RENDERTARGET)
		{
			this->_createTex();
			mIsLoaded = true;
			return;
		}

		// load based on tex.type
		switch (this->getTextureType())
		{
		case TEX_TYPE_1D:
		case TEX_TYPE_2D:
			this->_loadNormTex();
			break;
		case TEX_TYPE_CUBE_MAP:
			this->_loadCubeTex();
			break;
		default:
			Except( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D9Texture::load" );
			this->_freeResources();
		}

	}
	/****************************************************************************************/
	void D3D9Texture::unload()
	{
		// unload if it's loaded
		if (this->isLoaded())
		{
			this->_freeResources();
			mIsLoaded = false;
		}
	}
	/****************************************************************************************/
	void D3D9Texture::_freeResources()
	{
		SAFE_RELEASE(mpTex);
		SAFE_RELEASE(mpNormTex);
		SAFE_RELEASE(mpCubeTex);
		SAFE_RELEASE(mpZBuff);
	}
	/****************************************************************************************/
	void D3D9Texture::_loadCubeTex()
	{
		assert(this->getTextureType() == TEX_TYPE_CUBE_MAP);

		HRESULT hr = S_OK; // D3D9 methods result
		Image tImages[6]; // temp. images we'll use

		// we need src. image info so load first face
		// we assume that all faces are of the same dimensions
		// if they are not, they will be automatically 
		// resized when blitting
		tImages[0].load(this->_getCubeFaceName(0));
		this->_setSrcAttributes(tImages[0].getWidth(), tImages[0].getHeight(), tImages[0].getFormat());
		// now create the texture
		this->_createCubeTex();

		// prepare faces
		for (int face = 0; face < 6; face++)
		{
			// first face is already loaded
			if (face > 0)
				tImages[face].load(this->_getCubeFaceName(face)); // load the cube face
			// set gamma prior to blitting
			Image::applyGamma(
					tImages[face].getData(), 
					this->getGamma(), 
					(uint)tImages[face].getSize(), 
					tImages[face].getBPP());
		}
		// blit them all :(
		this->_blitImagesToCubeTex(tImages);
		// get topLevel surface description
		mpCubeTex->GetLevelDesc(0, &mTexDesc);
		// say IT'S loaded loud ;) 
		mIsLoaded = true;
	}
	/****************************************************************************************/
	void D3D9Texture::_loadNormTex()
	{
		assert(this->getTextureType() == TEX_TYPE_1D || this->getTextureType() == TEX_TYPE_2D);

		Image tImage; // temp. image we'll use
		HRESULT hr = S_OK; // D3D9 methods result

		// we need src image info
		tImage.load(this->getName());
		this->_setSrcAttributes(tImage.getWidth(), tImage.getHeight(), tImage.getFormat());
		// now create it
		this->_createNormTex();
		// set gamma prior to blitting
        Image::applyGamma(tImage.getData(), this->getGamma(), (uint)tImage.getSize(), tImage.getBPP());
		// make the BLT !!!
		this->_blitImageToNormTex(tImage);
		// get topLevel surface description
		mpNormTex->GetLevelDesc(0, &mTexDesc);
		// say IT'S loaded loud ;) 
		mIsLoaded = true;
	}
	/****************************************************************************************/
	void D3D9Texture::_createTex()
	{
		// if we are there then the source image dim. and format must already be set !!!
		assert(mSrcWidth > 0 || mSrcHeight > 0);

		// load based on tex.type
		switch (this->getTextureType())
		{
		case TEX_TYPE_1D:
		case TEX_TYPE_2D:
			this->_createNormTex();
			break;
		case TEX_TYPE_CUBE_MAP:
			this->_createCubeTex();
			break;
		default:
			Except( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D9Texture::_createTex" );
			this->_freeResources();
		}
	}
	/****************************************************************************************/
	void D3D9Texture::_createNormTex()
	{
		// we must have those defined here
		assert(mSrcWidth > 0 || mSrcHeight > 0);

		// determine wich D3D9 pixel format we'll use
		HRESULT hr;
		D3DFORMAT d3dPF = (mUsage == TU_RENDERTARGET) ? mBBPixelFormat : this->_chooseD3DFormat();

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		DWORD usage = (mUsage == TU_RENDERTARGET) ? D3DUSAGE_RENDERTARGET : 0;
		UINT numMips = (mNumMipMaps ? mNumMipMaps : 1);
		// check if mip maps are supported on hardware
		if (mDevCaps.TextureCaps & D3DPTEXTURECAPS_MIPMAP)
		{
			// use auto.gen. if available
			if (this->_canAutoGenMipMaps(usage, D3DRTYPE_TEXTURE, d3dPF))
			{
				usage |= D3DUSAGE_AUTOGENMIPMAP;
				numMips = 0;
			}
			else
			{
				if (mUsage != TU_RENDERTARGET)
				{
					// we must create a temp. texture in SYSTEM MEMORY if no auto gen. mip map is present
					hr = D3DXCreateTexture(	
						mpDev,								// device
						mSrcWidth,							// width
						mSrcHeight,							// height
						numMips,							// number of mip map levels
						usage,								// usage
						d3dPF,								// pixel format
						D3DPOOL_SYSTEMMEM,					// memory pool
						&mpTmpNormTex);						// data pointer
					// check result and except if failed
					if (FAILED(hr))
					{
						Except( hr, "Error creating texture", "D3D9Texture::_createNormTex" );
						this->_freeResources();
					}
				}
			}
		}
		else
		{
			// device don't support mip maps ???
			mNumMipMaps = 0;
			numMips = 1;
		}

		// create the texture
		hr = D3DXCreateTexture(	
				mpDev,								// device
				mSrcWidth,							// width
				mSrcHeight,							// height
				numMips,							// number of mip map levels
				usage,								// usage
				d3dPF,								// pixel format
				D3DPOOL_DEFAULT,					// memory pool
				&mpNormTex);						// data pointer
		// check result and except if failed
		if (FAILED(hr))
		{
			Except( hr, "Error creating texture", "D3D9Texture::_createNormTex" );
			this->_freeResources();
		}

		// set final tex. attributes from tex. description
		// they may differ from the source image !!!
		D3DSURFACE_DESC desc;
		hr = mpNormTex->GetLevelDesc(0, &desc);
		if (FAILED(hr))
		{
			Except( hr, "Can't get texture description", "D3D9Texture::_createNormTex" );
			this->_freeResources();
		}
		this->_setFinalAttributes(desc.Width, desc.Height, this->_getPF(desc.Format));
		
		// set the base texture we'll use in the render system
		hr = mpNormTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);
		if (FAILED(hr))
		{
			Except( hr, "Can't get base texture", "D3D9Texture::_createNormTex" );
			this->_freeResources();
		}
		
		// create a depth stencil if this is a render target
		if (mUsage == TU_RENDERTARGET)
			this->_createDepthStencil();

		// get topLevel surface description
		mpNormTex->GetLevelDesc(0, &mTexDesc);
	}
	/****************************************************************************************/
	void D3D9Texture::_createCubeTex()
	{
		// we must have those defined here
		assert(mSrcWidth > 0 || mSrcHeight > 0);

		// determine wich D3D9 pixel format we'll use
		HRESULT hr;
		D3DFORMAT d3dPF = (mUsage == TU_RENDERTARGET) ? mBBPixelFormat : this->_chooseD3DFormat();

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		DWORD usage = (mUsage == TU_RENDERTARGET) ? D3DUSAGE_RENDERTARGET : 0;
		UINT numMips = (mNumMipMaps ? mNumMipMaps : 1);
		// check if mip map cube textures are supported
		if (mDevCaps.TextureCaps & D3DPTEXTURECAPS_MIPCUBEMAP)
		{
			// use auto.gen. if available
			if (this->_canAutoGenMipMaps(usage, D3DRTYPE_CUBETEXTURE, d3dPF))
			{
				usage |= D3DUSAGE_AUTOGENMIPMAP;
				numMips = 0;
			}
			else
			{
				if (mUsage != TU_RENDERTARGET)
				{
					// we must create a temp. texture in SYSTEM MEMORY if no auto gen. mip map is present
					hr = D3DXCreateCubeTexture(	
						mpDev,								// device
						mSrcWidth,							// dimension
						numMips,							// number of mip map levels
						usage,								// usage
						d3dPF,								// pixel format
						D3DPOOL_SYSTEMMEM,					// memory pool
						&mpTmpCubeTex);						// data pointer
					// check result and except if failed
					if (FAILED(hr))
					{
						Except( hr, "Error creating texture", "D3D9Texture::_createCubeTex" );
						this->_freeResources();
					}
				}
			}
		}
		else
		{
			// no mip map support for this kind of textures :(
			mNumMipMaps = 0;
			numMips = 1;
		}

		// create the texture
		hr = D3DXCreateCubeTexture(	
				mpDev,								// device
				mSrcWidth,							// dimension
				numMips,							// number of mip map levels
				usage,								// usage
				d3dPF,								// pixel format
				D3DPOOL_DEFAULT,					// memory pool
				&mpCubeTex);						// data pointer
		// check result and except if failed
		if (FAILED(hr))
		{
			Except( hr, "Error creating texture", "D3D9Texture::_createCubeTex" );
			this->_freeResources();
		}

		// set final tex. attributes from tex. description
		// they may differ from the source image !!!
		D3DSURFACE_DESC desc;
		hr = mpCubeTex->GetLevelDesc(0, &desc);
		if (FAILED(hr))
		{
			Except( hr, "Can't get texture description", "D3D9Texture::_createCubeTex" );
			this->_freeResources();
		}
		this->_setFinalAttributes(desc.Width, desc.Height, this->_getPF(desc.Format));

		// set the base texture we'll use in the render system
		hr = mpCubeTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);
		if (FAILED(hr))
		{
			Except( hr, "Can't get base texture", "D3D9Texture::_createCubeTex" );
			this->_freeResources();
		}
		
		// create a depth stencil if this is a render target
		if (mUsage == TU_RENDERTARGET)
			this->_createDepthStencil();

		// get topLevel surface description
		mpCubeTex->GetLevelDesc(0, &mTexDesc);
	}
	/****************************************************************************************/
	void D3D9Texture::_initMembers()
	{
		mpDev = NULL;
		mpD3D = NULL;
		mpNormTex = NULL;
		mpCubeTex = NULL;
		mpZBuff = NULL;
		mpTex = NULL;

		mpTmpNormTex = NULL;
		mpTmpCubeTex = NULL;

		for (int i = 0; i < 6; ++i)
			mCubeFaceNames[i] = "";

		mWidth = mHeight = mSrcWidth = mSrcHeight = 0;
		mIsLoaded = false;
	}
	/****************************************************************************************/
	void D3D9Texture::_setDevice(IDirect3DDevice9 *pDev)
	{ 
		assert(pDev);
		mpDev = pDev;
		HRESULT hr;

		// get device caps
		hr = mpDev->GetDeviceCaps(&mDevCaps);
		if (FAILED(hr))
			Except( Exception::ERR_INTERNAL_ERROR, "Can't get device description", "D3D9Texture::_setDevice" );

		// get D3D pointer
		hr = mpDev->GetDirect3D(&mpD3D);
		if (FAILED(hr))
			Except( hr, "Failed to get D3D9 pointer", "D3D9Texture::_setDevice" );

		// get our device creation parameters
		hr = mpDev->GetCreationParameters(&mDevCreParams);
		if (FAILED(hr))
			Except( hr, "Failed to get D3D9 device creation parameters", "D3D9Texture::_setDevice" );

		// get our back buffer pixel format
		IDirect3DSurface9 *pSrf;
		D3DSURFACE_DESC srfDesc;
		hr = mpDev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSrf);
		if (FAILED(hr))
			Except( hr, "Failed to get D3D9 device pixel format", "D3D9Texture::_setDevice" );

		hr = pSrf->GetDesc(&srfDesc);
		if (FAILED(hr))
		{
			Except( hr, "Failed to get D3D9 device pixel format", "D3D9Texture::_setDevice" );
			SAFE_RELEASE(pSrf);
		}

		mBBPixelFormat = srfDesc.Format;
		SAFE_RELEASE(pSrf);
	}
	/****************************************************************************************/
	void D3D9Texture::_constructCubeFaceNames(const String& name)
	{
		// the suffixes
		String suffixes[6] = {"_rt", "_lf", "_up", "_dn", "_fr", "_bk"};
		size_t pos = -1;

		String ext; // the extension
		String baseName; // the base name
		String fakeName = name; // the 'fake' name, temp. holder

		// first find the base name
		pos = fakeName.find_last_of(".");
		if (pos == -1)
		{
			Except( Exception::ERR_INTERNAL_ERROR, "Invalid cube texture base name", "D3D9Texture::_constructCubeFaceNames" );
			this->_freeResources();
		}

		baseName = fakeName.substr(0, pos);
		ext = fakeName.substr(pos);

		// construct the full 6 faces file names from the baseName, suffixes and extension
		for (int i = 0; i < 6; ++i)
			mCubeFaceNames[i] = baseName + suffixes[i] + ext;
	}
	/****************************************************************************************/
	void D3D9Texture::_setFinalAttributes(unsigned long width, unsigned long height, PixelFormat format)
	{ 
		// set target texture attributes
		mHeight = height; 
		mWidth = width; 
		mFormat = format; 

		// Update size (the final size, not including temp space)
		// this is needed in Resource class
		short bytesPerPixel = mFinalBpp >> 3;
		if( !mHasAlpha && mFinalBpp == 32 )
			bytesPerPixel--;
		mSize = mWidth * mHeight * bytesPerPixel;

		// say to the world what we are doing
		if (mWidth != mSrcWidth ||
			mHeight != mSrcHeight)
		{
			LogManager::getSingleton().logMessage("D3D9 : ***** Dimensions altered by the render system");
			LogManager::getSingleton().logMessage("D3D9 : ***** Source image dimensions : " + StringConverter::toString(mSrcWidth) + "x" + StringConverter::toString(mSrcHeight));
			LogManager::getSingleton().logMessage("D3D9 : ***** Texture dimensions : " + StringConverter::toString(mWidth) + "x" + StringConverter::toString(mHeight));
		}
	}
	/****************************************************************************************/
	void D3D9Texture::_setSrcAttributes(unsigned long width, unsigned long height, PixelFormat format)
	{ 
		// set source image attributes
		mSrcWidth = width; 
		mSrcHeight = height; 
		mSrcBpp = _getPFBpp(format); 
        mHasAlpha = Image::formatHasAlpha(format); 
		// say to the world what we are doing
		switch (this->getTextureType())
		{
		case TEX_TYPE_1D:
			if (mUsage == TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating 1D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipMaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading 1D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipMaps) + " mip map levels");
			break;
		case TEX_TYPE_2D:
			if (mUsage == TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating 2D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipMaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading 2D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipMaps) + " mip map levels");
			break;
		case TEX_TYPE_CUBE_MAP:
			if (mUsage == TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating Cube map RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipMaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading Cube Texture, base image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipMaps) + " mip map levels");
			break;
		default:
			Except( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D9Texture::_createTex" );
			this->_freeResources();
		}
	}
	/****************************************************************************************/
	D3DTEXTUREFILTERTYPE D3D9Texture::_getBestFilterMethod()
	{
		// those MUST be initialized !!!
		assert(mpDev);
		assert(mpD3D);
		assert(mpTex);

		// TODO : do it really :)
		return D3DTEXF_POINT;
	}
	/****************************************************************************************/
	bool D3D9Texture::_canAutoGenMipMaps(DWORD srcUsage, D3DRESOURCETYPE srcType, D3DFORMAT srcFormat)
	{
		// those MUST be initialized !!!
		assert(mpDev);
		assert(mpD3D);

		if (mDevCaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP)
		{
			HRESULT hr;
			// check for auto gen. mip maps support
			hr = mpD3D->CheckDeviceFormat(
					mDevCreParams.AdapterOrdinal, 
					mDevCreParams.DeviceType, 
					mBBPixelFormat, 
					srcUsage | D3DUSAGE_AUTOGENMIPMAP,
					srcType,
					srcFormat);
			// this HR could a SUCCES
			// but mip maps will not be generated
			if (hr == D3D_OK)
				return true;
			else
				return false;
		}
		else
			return false;
	}
	/****************************************************************************************/
	void D3D9Texture::_getColorMasks(D3DFORMAT format, DWORD *pdwRed, DWORD *pdwGreen, DWORD *pdwBlue, DWORD *pdwAlpha, DWORD *pdwRGBBitCount)
	{
		// we choose the format of the D3D texture so check only for our pf types...
		switch (format)
		{
		case D3DFMT_X8R8G8B8:
			*pdwRed = 0x00FF0000; *pdwGreen = 0x0000FF00; *pdwBlue = 0x000000FF; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 32;
			break;
		case D3DFMT_R8G8B8:
			*pdwRed = 0x00FF0000; *pdwGreen = 0x0000FF00; *pdwBlue = 0x000000FF; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 24;
			break;
		case D3DFMT_A8R8G8B8:
			*pdwRed = 0x00FF0000; *pdwGreen = 0x0000FF00; *pdwBlue = 0x000000FF; *pdwAlpha = 0xFF000000;
			*pdwRGBBitCount = 32;
			break;
		case D3DFMT_X1R5G5B5:
			*pdwRed = 0x00007C00; *pdwGreen = 0x000003E0; *pdwBlue = 0x0000001F; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_R5G6B5:
			*pdwRed = 0x0000F800; *pdwGreen = 0x000007E0; *pdwBlue = 0x0000001F; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_A4R4G4B4:
			*pdwRed = 0x00000F00; *pdwGreen = 0x000000F0; *pdwBlue = 0x0000000F; *pdwAlpha = 0x0000F000;
			*pdwRGBBitCount = 16;
			break;
		default:
			Except( Exception::ERR_INTERNAL_ERROR, "Unknown D3D pixel format, this should not happen !!!", "D3D9Texture::_getColorMasks" );
		}
	}
	/****************************************************************************************/
	void D3D9Texture::_copyMemoryToSurface(const unsigned char *pBuffer, IDirect3DSurface9 *pSurface)
	{
		assert(pBuffer);
		assert(pSurface);
		// Copy the image from the buffer to the temporary surface.
		// We have to do our own colour conversion here since we don't 
		// have a DC to do it for us
		// NOTE - only non-palettised surfaces supported for now
		HRESULT hr;
		D3DSURFACE_DESC desc;
		D3DLOCKED_RECT rect;
		BYTE *pSurf8;
		BYTE *pBuf8;
		DWORD data32;
		DWORD out32;
		DWORD temp32;
		DWORD srcPattern;
		unsigned iRow, iCol;
		// NOTE - dimensions of surface may differ from buffer
		// dimensions (e.g. power of 2 or square adjustments)
		// Lock surface
		pSurface->GetDesc(&desc);
		DWORD aMask, rMask, gMask, bMask, rgbBitCount;
		this->_getColorMasks(desc.Format, &rMask, &gMask, &bMask, &aMask, &rgbBitCount);
		// lock our surface to acces raw memory
		if( FAILED( hr = pSurface->LockRect(&rect, NULL, D3DLOCK_NOSYSLOCK) ) )
		{
			Except( hr, "Unable to lock temp texture surface", "D3D9Texture::_copyMemoryToSurface" );
			this->_freeResources();
			SAFE_RELEASE(pSurface);
		}
		else
			pBuf8 = (BYTE*)pBuffer;
		// loop through data and do conv.
		for( iRow = 0; iRow < mSrcHeight; iRow++ )
		{
			// NOTE: Direct3D used texture coordinates where (0,0) is the TOP LEFT corner of texture
			// Everybody else (OpenGL, 3D Studio, etc) uses (0,0) as BOTTOM LEFT corner
			// So whilst we load, flip the texture in the Y-axis to compensate
			pSurf8 = (BYTE*)rect.pBits + ((mSrcHeight - iRow - 1) * rect.Pitch);
			for( iCol = 0; iCol < mSrcWidth; iCol++ )
			{
				// Read RGBA values from buffer
				data32 = 0;
				if( mSrcBpp >= 24 )
				{
					// Data in buffer is in RGB(A) format
					// Read into a 32-bit structure
					// Uses bytes for 24-bit compatibility
					// NOTE: buffer is big-endian
					data32 |= *pBuf8++ << 24;
					data32 |= *pBuf8++ << 16;
					data32 |= *pBuf8++ << 8;
				}
				else if( mSrcBpp == 8 ) // Greyscale, not palettised (palettised NOT supported)
				{
					// Duplicate same greyscale value across R,G,B
					data32 |= *pBuf8 << 24;
					data32 |= *pBuf8 << 16;
					data32 |= *pBuf8++ << 8;
				}
				// check for alpha
				if( mHasAlpha )
					data32 |= *pBuf8++;
				else
					data32 |= 0xFF;	// Set opaque
				// Write RGBA values to surface
				// Data in surface can be in varying formats
				// Use bit concersion function
				// NOTE: we use a 32-bit value to manipulate
				// Will be reduced to size later
				out32 = 0;
				// Red
				srcPattern = 0xFF000000;
				Bitwise::convertBitPattern( &data32, &srcPattern, 32, &temp32, &rMask, 32 );
				out32 |= temp32;
				// Green
				srcPattern = 0x00FF0000;
				Bitwise::convertBitPattern( &data32, &srcPattern, 32, &temp32, &gMask, 32 );
				out32 |= temp32;
				// Blue
				srcPattern = 0x0000FF00;
				Bitwise::convertBitPattern( &data32, &srcPattern, 32, &temp32, &bMask, 32 );
				out32 |= temp32;
				// Alpha
				if( aMask > 0 )
				{
					srcPattern = 0x000000FF;
					Bitwise::convertBitPattern( &data32, &srcPattern, 32, &temp32, &aMask, 32 );
					out32 |= temp32;
				}
				// Assign results to surface pixel
				// Write up to 4 bytes
				// Surfaces are little-endian (low byte first)
				if( rgbBitCount >= 8 )
					*pSurf8++ = (BYTE)out32;
				if( rgbBitCount >= 16 )
					*pSurf8++ = (BYTE)(out32 >> 8);
				if( rgbBitCount >= 24 )
					*pSurf8++ = (BYTE)(out32 >> 16);
				if( rgbBitCount >= 32 )
					*pSurf8++ = (BYTE)(out32 >> 24);
			} // for( iCol...
		} // for( iRow...
		// unlock the surface
		pSurface->UnlockRect();
	}
	/****************************************************************************************/
	void D3D9Texture::_blitImageToNormTex(const Image &srcImage)
	{
		HRESULT hr;
		D3DFORMAT srcFormat = this->_getPF(srcImage.getFormat());
		D3DFORMAT dstFormat = _chooseD3DFormat();
		RECT tmpDataRect = {0, 0, srcImage.getWidth(), srcImage.getHeight()}; // the rectangle representing the src. image dim.

		// this surface will hold our temp conversion image
		// We need this in all cases because we can't lock 
		// the main texture surfaces in all cards
		// Also , this cannot be the temp texture because we'd like D3DX to resize it for us
		// with the D3DxLoadSurfaceFromSurface
		IDirect3DSurface9 *pSrcSurface = NULL;
		hr = mpDev->CreateOffscreenPlainSurface( 
						srcImage.getWidth(),
						srcImage.getHeight(),
						dstFormat, 
						D3DPOOL_SCRATCH, 
						&pSrcSurface, 
						NULL);
		// check result and except if failed
		if (FAILED(hr))
		{
			this->_freeResources();
			Except( hr, "Error loading surface from memory", "D3D9Texture::_blitImageToTexture" );
		}
		
		// copy the buffer to our surface, 
		// _copyMemoryToSurface will do color conversion and flipping
		this->_copyMemoryToSurface(srcImage.getData(), pSrcSurface);

		// Now we need to copy the source surface (where our image is) to the texture
		// This will be a temp texture for s/w filtering and the final one for h/w filtering
		// This will perform any size conversion (inc stretching)
		IDirect3DSurface9 *pDstSurface; 
		if (mpTmpNormTex)
		{
			// s/w mipmaps, use temp texture
			hr = mpTmpNormTex->GetSurfaceLevel(0, &pDstSurface);
		}
		else
		{
			// h/w mipmaps, use the final texture
			hr = mpNormTex->GetSurfaceLevel(0, &pDstSurface);
		}

		// check result and except if failed
		if (FAILED(hr))
		{
			SAFE_RELEASE(pSrcSurface);
			this->_freeResources();
			Except( hr, "Error getting level 0 surface from dest. texture", "D3D9Texture::_blitImageToTexture" );
		}

		// copy surfaces
		hr = D3DXLoadSurfaceFromSurface(pDstSurface, NULL, NULL, pSrcSurface, NULL, NULL, D3DX_DEFAULT, 0);
		// check result and except if failed
		if (FAILED(hr))
		{
			SAFE_RELEASE(pSrcSurface);
			SAFE_RELEASE(pDstSurface);
			this->_freeResources();
			Except( hr, "Error copying original surface to texture", "D3D9Texture::_blitImageToTexture" );
		}

		if (mpTmpNormTex)
		{
			// Software filtering
			// Now update the texture & filter the results
			// we will use D3DX to create the mip map levels
			if( FAILED( hr = D3DXFilterTexture( mpTmpNormTex, NULL, D3DX_DEFAULT, D3DX_DEFAULT ) ) )
			{
				SAFE_RELEASE(pSrcSurface);
				SAFE_RELEASE(pDstSurface);
				this->_freeResources();
				Except( hr, "Failed to filter texture (generate mip maps)", "D3D9Texture::_blitImageToTexture" );
			}
			if( FAILED( hr = mpDev->UpdateTexture( mpTmpNormTex, mpNormTex ) ) )
			{
				SAFE_RELEASE(pSrcSurface);
				SAFE_RELEASE(pDstSurface);
				this->_freeResources();
				Except( hr, "Failed to update texture", "D3D9Texture::_blitImageToTexture" );
			}
		}
		else
		{
			// Hardware mipmapping
			// use best filtering method supported by hardware
			hr = mpTex->SetAutoGenFilterType(_getBestFilterMethod());
			if (FAILED(hr))
			{
				SAFE_RELEASE(pSrcSurface);
				SAFE_RELEASE(pDstSurface);
				this->_freeResources();
				Except( hr, "Error generating mip maps", "D3D9Texture::_blitImageToNormTex" );
			}
			mpNormTex->GenerateMipSubLevels();

			
		}

		SAFE_RELEASE(pDstSurface);
		SAFE_RELEASE(pSrcSurface);
		SAFE_RELEASE(mpTmpNormTex);
	}
	/****************************************************************************************/
	void D3D9Texture::_blitImagesToCubeTex(const Image srcImages[])
	{
		HRESULT hr;
		D3DFORMAT dstFormat = _chooseD3DFormat();

		// we must loop through all 6 cube map faces :(
		for (int face = 0; face < 6; face++)
		{
			D3DFORMAT srcFormat = this->_getPF(srcImages[face].getFormat());
			RECT tmpDataRect = {0, 0, srcImages[face].getWidth(), srcImages[face].getHeight()}; // the rectangle representing the src. image dim.

			// this surface will hold our temp conversion image
			IDirect3DSurface9 *pSrcSurface = NULL;
			// We need this in all cases because we can't lock 
			// the main texture surfaces in all cards
			// Also , this cannot be the temp texture because we'd like D3DX to resize it for us
			// with the D3DxLoadSurfaceFromSurface
			hr = mpDev->CreateOffscreenPlainSurface( 
							srcImages[face].getWidth(), 
							srcImages[face].getHeight(), 
							dstFormat, 
							D3DPOOL_SCRATCH, 
							&pSrcSurface, 
							NULL);
			// check result and except if failed
			if (FAILED(hr))
			{
				Except( hr, "Error loading surface from memory", "D3D9Texture::_blitImagesToCubeTex" );
				this->_freeResources();
			}

			// don't know why but cube textures don't require fliping
			// _copyMemoryToSurface flips all around x, so we'll flip the
			// src.image first, then 'reflip' it :(, and we need a temp. image for this :(
			Image tmpImg(srcImages[face]);
			tmpImg.flipAroundX();
			// copy the buffer to our surface, 
			// _copyMemoryToSurface will do color conversion and flipping
			this->_copyMemoryToSurface(tmpImg.getData(), pSrcSurface);

			// Now we need to copy the source surface (where our image is) to 
			// either the the temp. texture level 0 surface (for s/w mipmaps)
			// or the final texture (for h/w mipmaps)
			IDirect3DSurface9 *pDstSurface; 
			if (mpTmpCubeTex)
			{
				// copy into temp
                hr = mpTmpCubeTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, 0, &pDstSurface);
			}
			else
			{
				// copy into temp
                hr = mpCubeTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, 0, &pDstSurface);
			}
			// check result and except if failed
			if (FAILED(hr))
			{
				Except( hr, "Error getting level dest cube map surface", "D3D9Texture::_blitImagesToCubeTex" );
				SAFE_RELEASE(pSrcSurface);
				this->_freeResources();
			}

			// load the surface with an in memory buffer
			hr = D3DXLoadSurfaceFromSurface(pDstSurface, NULL, NULL, pSrcSurface, NULL, NULL, D3DX_DEFAULT, 0);
			// check result and except if failed
			if (FAILED(hr))
			{
				Except( hr, "Error loading in temporary surface", "D3D9Texture::_blitImagesToCubeTex" );
				SAFE_RELEASE(pSrcSurface);
				SAFE_RELEASE(pDstSurface);
				this->_freeResources();
			}

			SAFE_RELEASE(pDstSurface);
			SAFE_RELEASE(pSrcSurface);
		}

		// After doing all the faces, we generate mipmaps
		// For s/w mipmaps this involves an extra copying step
		if (mpTmpCubeTex)
		{
			// Filter
			if( FAILED( hr = D3DXFilterTexture( mpTmpCubeTex, NULL, D3DX_DEFAULT, D3DX_DEFAULT ) ) )
			{
				this->_freeResources();
				Except( hr, "Failed to filter texture (generate mip maps)", "D3D9Texture::_blitImageToCubeTex" );
			}
			// Update main texture
			if( FAILED( hr = mpDev->UpdateTexture( mpTmpCubeTex, mpCubeTex ) ) )
			{
				this->_freeResources();
				Except( hr, "Failed to update texture", "D3D9Texture::_blitImageToCubeTex" );
			}
		}
		else
		{
			// Hardware filter
			// use best filtering method supported by hardware
			hr = mpTex->SetAutoGenFilterType(_getBestFilterMethod());
			if (FAILED(hr))
			{
				this->_freeResources();
				Except( hr, "Error generating mip maps", "D3D9Texture::_blitImageToCubeTex" );
			}
			mpCubeTex->GenerateMipSubLevels();

		}

		SAFE_RELEASE(mpTmpCubeTex);

	}
	/****************************************************************************************/
	D3DFORMAT D3D9Texture::_chooseD3DFormat()
	{
		// choose wise wich D3D format we'll use ;)
		if( mFinalBpp > 16 && mHasAlpha )
			return D3DFMT_A8R8G8B8;
		else if( mFinalBpp > 16 && !mHasAlpha )
			return D3DFMT_R8G8B8;
		else if( mFinalBpp == 16 && mHasAlpha )
			return D3DFMT_A4R4G4B4;
		else if( mFinalBpp == 16 && !mHasAlpha )
			return D3DFMT_R5G6B5;
		else
			Except( Exception::ERR_INVALIDPARAMS, "Unknown pixel format", "D3D9Texture::_chooseD3DFormat" );
	}
	/****************************************************************************************/
	void D3D9Texture::_createDepthStencil()
	{
		IDirect3DSurface9 *pSrf;
		D3DSURFACE_DESC srfDesc;
		HRESULT hr;

		/* Get the format of the depth stencil surface of our main render target. */
		hr = mpDev->GetDepthStencilSurface(&pSrf);
		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Error GetDepthStencilSurface : " + msg, "D3D9Texture::_createDepthStencil" );
			this->_freeResources();
		}
		// get it's description
		hr = pSrf->GetDesc(&srfDesc);
		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Error GetDesc : " + msg, "D3D9Texture::_createDepthStencil" );
			SAFE_RELEASE(pSrf);
			this->_freeResources();
		}
		// release the temp. surface
		SAFE_RELEASE(pSrf);
		/** Create a depth buffer for our render target, it must be of
		    the same format as other targets !!!
		. */
		hr = mpDev->CreateDepthStencilSurface( 
			mSrcWidth, 
			mSrcHeight, 
			srfDesc.Format, 
			srfDesc.MultiSampleType, 
			NULL, 
			FALSE, 
			&mpZBuff, 
			NULL);
		// cry if failed 
		if (FAILED(hr))
		{
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Error CreateDepthStencilSurface : " + msg, "D3D9Texture::_createDepthStencil" );
			this->_freeResources();
		}
	}
	/****************************************************************************************/
	PixelFormat D3D9Texture::_getPF(D3DFORMAT d3dPF)
	{
		switch(d3dPF)
		{
		case D3DFMT_A8:
			return PF_A8;
		case D3DFMT_A4L4:
			return PF_A4L4;
		case D3DFMT_A4R4G4B4:
			return PF_A4R4G4B4;
		case D3DFMT_A8R8G8B8:
			return PF_A8R8G8B8;
		case D3DFMT_A2R10G10B10:
			return PF_A2R10G10B10;
		case D3DFMT_L8:
			return PF_L8;
		case D3DFMT_X1R5G5B5:
		case D3DFMT_R5G6B5:
			return PF_R5G6B5;
		case D3DFMT_X8R8G8B8:
		case D3DFMT_R8G8B8:
			return PF_R8G8B8;
		default:
			return PF_UNKNOWN;
		}
	}
	/****************************************************************************************/
	D3DFORMAT D3D9Texture::_getPF(PixelFormat ogrePF)
	{
		switch(ogrePF)
		{
		case PF_L8:
			return D3DFMT_L8;
		case PF_A8:
			return D3DFMT_A8;
		case PF_B5G6R5:
		case PF_R5G6B5:
			return D3DFMT_R5G6B5;
		case PF_B4G4R4A4:
		case PF_A4R4G4B4:
			return D3DFMT_A4R4G4B4;
		case PF_B8G8R8:
		case PF_R8G8B8:
			return D3DFMT_R8G8B8;
		case PF_B8G8R8A8:
		case PF_A8R8G8B8:
			return D3DFMT_A8R8G8B8;
		case PF_L4A4:
		case PF_A4L4:
			return D3DFMT_A4L4;
		case PF_B10G10R10A2:
		case PF_A2R10G10B10:
			return D3DFMT_A2R10G10B10;
		case PF_UNKNOWN:
		default:
			return D3DFMT_UNKNOWN;
		}
	}
	/****************************************************************************************/
}

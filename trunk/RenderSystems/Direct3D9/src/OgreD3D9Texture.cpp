/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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
#include "OgreD3D9HardwarePixelBuffer.h"
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
    D3D9Texture::D3D9Texture(ResourceManager* creator, const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader, IDirect3DDevice9 *pD3DDevice)
        :Texture(creator, name, handle, group, isManual, loader),
        mpDev(pD3DDevice), 
        mpD3D(NULL), 
        mpNormTex(NULL),
        mpCubeTex(NULL),
        mpZBuff(NULL),
        mpTex(NULL),
        mAutoGenMipmaps(false)
	{
        _initDevice();
	}
	/****************************************************************************************/
	D3D9Texture::~D3D9Texture()
	{
		SAFE_RELEASE(mpD3D);
        // have to call this here reather than in Resource destructor
        // since calling virtual methods in base destructors causes crash
        unload(); 
	}

	/****************************************************************************************/
	void D3D9Texture::copyToTexture(TexturePtr& target)
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
		other = reinterpret_cast< D3D9Texture * >( target.get() );
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
			for (size_t face = 0; face < 6; face++)
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
		this->_setSrcAttributes(tImage.getWidth(), tImage.getHeight(), 1, tImage.getFormat());
		// create a blank texture
		createInternalResources();
		// set gamma prior to blitting
        Image::applyGamma(tImage.getData(), this->getGamma(), (uint)tImage.getSize(), tImage.getBPP());
		this->_blitImageToNormTex(tImage);
		mIsLoaded = true;
	}
	/****************************************************************************************/
	void D3D9Texture::loadImpl()
	{
		// load based on tex.type
		switch (this->getTextureType())
		{
		case TEX_TYPE_1D:
		case TEX_TYPE_2D:
			this->_loadNormTex();
			break;
		case TEX_TYPE_3D:
            this->_loadVolumeTex();
            break;
		case TEX_TYPE_CUBE_MAP:
			this->_loadCubeTex();
			break;
		default:
			Except( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D9Texture::loadImpl" );
			this->_freeResources();
		}

	}
	/****************************************************************************************/
	void D3D9Texture::unloadImpl()
	{
		_freeResources();
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

        // DDS load?
		if (StringUtil::endsWith(getName(), ".dds"))
        {
            // find & load resource data
			DataStreamPtr dstream = ResourceGroupManager::getSingleton().openResource(mName, mGroup);
            MemoryDataStream stream( dstream );

            HRESULT hr = D3DXCreateCubeTextureFromFileInMemory(
                mpDev,
                stream.getPtr(),
                stream.size(),
                &mpCubeTex);

            if (FAILED(hr))
		    {
			    Except( hr, "Can't create cube texture", "D3D9Texture::_loadCubeTex" );
			    this->_freeResources();
		    }

            hr = mpCubeTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);

            if (FAILED(hr))
		    {
			    Except( hr, "Can't get base texture", "D3D9Texture::_loadCubeTex" );
			    this->_freeResources();
		    }

            D3DSURFACE_DESC texDesc;
            mpCubeTex->GetLevelDesc(0, &texDesc);
            // set src and dest attributes to the same, we can't know
            _setSrcAttributes(texDesc.Width, texDesc.Height, 1, _getPF(texDesc.Format));
            _setFinalAttributes(texDesc.Width, texDesc.Height, 1,  _getPF(texDesc.Format));

        	
        }
        else
        {

            // Load from 6 separate files
            _constructCubeFaceNames(mName);
            // First create the base surface, for that we need to know the size
            Image img;
            img.load(this->_getCubeFaceName(0), mGroup);
            _setSrcAttributes(img.getWidth(), img.getHeight(), 1, img.getFormat());
            // now create the texture
            createInternalResources();

		    HRESULT hr; // D3D9 methods result

		    // load faces
		    for (size_t face = 0; face < 6; face++)
		    {
				DataStreamPtr dstream = 
					ResourceGroupManager::getSingleton().openResource(
                        _getCubeFaceName(face), mGroup);
                MemoryDataStream stream(dstream);
                    
                
                LPDIRECT3DSURFACE9 pDstSurface;
                hr = mpCubeTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, 0, &pDstSurface);
                if (FAILED(hr))
                {
                    Except(Exception::ERR_INTERNAL_ERROR, "Cannot get cubemap surface", 
                        "D3D9Texture::_loadCubeTex");
                }

                // Load directly using D3DX
                hr = D3DXLoadSurfaceFromFileInMemory(
                    pDstSurface,
                    NULL,                       // no palette
                    NULL,                       // entire surface
                    stream.getPtr(),
                    stream.size(),
                    NULL,                       // entire source
                    D3DX_DEFAULT,               // default filtering
                    0,                          // No colour key
                    NULL);                      // no src info
                        
                if (FAILED(hr))
                {
                    Except(Exception::ERR_INTERNAL_ERROR, "Cannot load cubemap surface", 
                        "D3D9Texture::_loadCubeTex");
                }

		    }

            // Mipmaps
            if (mAutoGenMipmaps)
            {
                // use best filtering method supported by hardware
                hr = mpTex->SetAutoGenFilterType(_getBestFilterMethod());
                if (FAILED(hr))
                {
                    this->_freeResources();
                    Except( hr, "Error generating mip maps", "D3D9Texture::_blitImageToCubeTex" );
                }
                mpCubeTex->GenerateMipSubLevels();
            }
            else
            {
                // Software mipmaps
                if( FAILED( hr = D3DXFilterTexture( mpCubeTex, NULL, D3DX_DEFAULT, D3DX_DEFAULT ) ) )
                {
                    this->_freeResources();
                    Except( hr, "Failed to filter texture (generate mip maps)", "D3D9Texture::_blitImageToCubeTex" );
                }

            }

            // Do final attributes
            D3DSURFACE_DESC texDesc;
            mpCubeTex->GetLevelDesc(0, &texDesc);
            _setFinalAttributes(texDesc.Width, texDesc.Height, 1,  _getPF(texDesc.Format));
        }
		mIsLoaded = true;
	}
	/****************************************************************************************/
	void D3D9Texture::_loadVolumeTex()
	{
		assert(this->getTextureType() == TEX_TYPE_3D);

        // find & load resource data
		DataStreamPtr dstream = ResourceGroupManager::getSingleton().openResource(mName, mGroup);
		MemoryDataStream stream(dstream);

        HRESULT hr = D3DXCreateVolumeTextureFromFileInMemory(
            mpDev,
            stream.getPtr(),
            stream.size(),
            &mpVolumeTex);

        if (FAILED(hr))
        {
            Except(Exception::ERR_INTERNAL_ERROR, 
                "Unable to load volume texture from " + this->getName(),
                "D3D9Texture::_loadVolumeTex");
        }

        hr = mpVolumeTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);

        if (FAILED(hr))
		{
			Except( hr, "Can't get base texture", "D3D9Texture::_loadVolumeTex" );
			this->_freeResources();
		}

        D3DVOLUME_DESC texDesc;
        hr = mpVolumeTex->GetLevelDesc(0, &texDesc);

        // set src and dest attributes to the same, we can't know
        _setSrcAttributes(texDesc.Width, texDesc.Height, texDesc.Depth, _getPF(texDesc.Format));
        _setFinalAttributes(texDesc.Width, texDesc.Height, texDesc.Depth, _getPF(texDesc.Format));
        
		mIsLoaded = true;
    }
	/****************************************************************************************/
	void D3D9Texture::_loadNormTex()
	{
		assert(this->getTextureType() == TEX_TYPE_1D || this->getTextureType() == TEX_TYPE_2D);

		// Use D3DX
        // find & load resource data
		DataStreamPtr dstream = 
			ResourceGroupManager::getSingleton().openResource(mName, mGroup);
        MemoryDataStream stream(dstream);

        HRESULT hr = D3DXCreateTextureFromFileInMemory(
            mpDev,
            stream.getPtr(),
            stream.size(),
            &mpNormTex);

        if (FAILED(hr))
        {
            Except(Exception::ERR_INTERNAL_ERROR, 
                "Unable to load texture from " + this->getName(),
                "D3D9Texture::_loadNormTex");
        }

        hr = mpNormTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);

        if (FAILED(hr))
		{
			Except( hr, "Can't get base texture", "D3D9Texture::_loadNormTex" );
			this->_freeResources();
		}

        D3DSURFACE_DESC texDesc;
        mpNormTex->GetLevelDesc(0, &texDesc);
        // set src and dest attributes to the same, we can't know
        _setSrcAttributes(texDesc.Width, texDesc.Height, 1, _getPF(texDesc.Format));
        _setFinalAttributes(texDesc.Width, texDesc.Height, 1, _getPF(texDesc.Format));
        
		mIsLoaded = true;
	}
	/****************************************************************************************/
    void D3D9Texture::createInternalResources(void)
	{
		// If mSrcWidth and mSrcHeight are zero, the requested extents have probably been set
		// through setWidth and setHeight, which set mWidth and mHeight. Take those values.
		if(mSrcWidth == 0 || mSrcHeight == 0) {
			mSrcWidth = mWidth;
			mSrcHeight = mHeight;
		}
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
		case TEX_TYPE_3D:
			this->_createVolumeTex();
			break;
		default:
			Except( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D9Texture::createInternalResources" );
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
		D3DFORMAT d3dPF = (mUsage & TU_RENDERTARGET) ? mBBPixelFormat : this->_chooseD3DFormat();

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		DWORD usage = (mUsage & TU_RENDERTARGET) ? D3DUSAGE_RENDERTARGET : 0;
		UINT numMips = mNumMipmaps + 1;
		// check if mip maps are supported on hardware
		if ((mDevCaps.TextureCaps & D3DPTEXTURECAPS_MIPMAP) && mNumMipmaps > 0)
		{
			// use auto.gen. if available, and if desired
            mAutoGenMipmaps = this->_canAutoGenMipmaps(usage, D3DRTYPE_TEXTURE, d3dPF);
			if ((mUsage & TU_AUTOMIPMAP) && mAutoGenMipmaps)
			{
				usage |= D3DUSAGE_AUTOGENMIPMAP;
				numMips = 0;
			}
		}
		else
		{
			// device don't support mip maps, or zero mipmaps requested
			mNumMipmaps = 0;
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
                (mUsage & TU_RENDERTARGET)?
                D3DPOOL_DEFAULT : D3DPOOL_MANAGED,	// memory pool
				&mpNormTex);						// data pointer
		// check result and except if failed
		if (FAILED(hr))
		{
			this->_freeResources();
			Except( hr, "Error creating texture", "D3D9Texture::_createNormTex" );
		}
		
		// set the base texture we'll use in the render system
		hr = mpNormTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);
		if (FAILED(hr))
		{
			Except( hr, "Can't get base texture", "D3D9Texture::_createNormTex" );
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
		this->_setFinalAttributes(desc.Width, desc.Height, 1, this->_getPF(desc.Format));
		
		// create a depth stencil if this is a render target
		if (mUsage & TU_RENDERTARGET)
			this->_createDepthStencil();

		// Set best filter type
		if(mAutoGenMipmaps)
		{
			hr = mpTex->SetAutoGenFilterType(_getBestFilterMethod());
			if(FAILED(hr))
			{
				Except( hr, "Could not set best autogen filter type", "D3D9Texture::_createNormTex" );
			}
		}
	}
	/****************************************************************************************/
	void D3D9Texture::_createCubeTex()
	{
		// we must have those defined here
		assert(mSrcWidth > 0 || mSrcHeight > 0);

		// determine wich D3D9 pixel format we'll use
		HRESULT hr;
		D3DFORMAT d3dPF = (mUsage & TU_RENDERTARGET) ? mBBPixelFormat : this->_chooseD3DFormat();

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		DWORD usage = (mUsage & TU_RENDERTARGET) ? D3DUSAGE_RENDERTARGET : 0;
		UINT numMips = mNumMipmaps + 1;
		// check if mip map cube textures are supported
		if (mDevCaps.TextureCaps & D3DPTEXTURECAPS_MIPCUBEMAP)
		{
			// use auto.gen. if available
            mAutoGenMipmaps = this->_canAutoGenMipmaps(usage, D3DRTYPE_CUBETEXTURE, d3dPF);
			if (mAutoGenMipmaps)
			{
				usage |= D3DUSAGE_AUTOGENMIPMAP;
				numMips = 0;
			}
		}
		else
		{
			// no mip map support for this kind of textures :(
			mNumMipmaps = 0;
			numMips = 1;
		}

		// create the texture
		hr = D3DXCreateCubeTexture(	
				mpDev,								// device
				mSrcWidth,							// dimension
				numMips,							// number of mip map levels
				usage,								// usage
				d3dPF,								// pixel format
                (mUsage & TU_RENDERTARGET)?
                D3DPOOL_DEFAULT : D3DPOOL_MANAGED,	// memory pool
				&mpCubeTex);						// data pointer
		// check result and except if failed
		if (FAILED(hr))
		{
			this->_freeResources();
			Except( hr, "Error creating texture", "D3D9Texture::_createCubeTex" );
		}

		// set the base texture we'll use in the render system
		hr = mpCubeTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);
		if (FAILED(hr))
		{
			Except( hr, "Can't get base texture", "D3D9Texture::_createCubeTex" );
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
		this->_setFinalAttributes(desc.Width, desc.Height, 1, this->_getPF(desc.Format));
		
		// create a depth stencil if this is a render target
		if (mUsage & TU_RENDERTARGET)
			this->_createDepthStencil();

		// Set best filter type
		if(mAutoGenMipmaps)
		{
			hr = mpTex->SetAutoGenFilterType(_getBestFilterMethod());
			if(FAILED(hr))
			{
				Except( hr, "Could not set best autogen filter type", "D3D9Texture::_createCubeTex" );
			}
		}
	}
	/****************************************************************************************/
	void D3D9Texture::_createVolumeTex()
	{
		// we must have those defined here
		assert(mWidth > 0 && mHeight > 0 && mDepth>0);

		// determine wich D3D9 pixel format we'll use
		HRESULT hr;
		D3DFORMAT d3dPF = (mUsage & TU_RENDERTARGET) ? mBBPixelFormat : this->_chooseD3DFormat();

		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		DWORD usage = (mUsage & TU_RENDERTARGET) ? D3DUSAGE_RENDERTARGET : 0;
		UINT numMips = mNumMipmaps + 1;
		// check if mip map cube textures are supported
		if (mDevCaps.TextureCaps & D3DPTEXTURECAPS_MIPVOLUMEMAP)
		{
			// use auto.gen. if available
            mAutoGenMipmaps = this->_canAutoGenMipmaps(usage, D3DRTYPE_VOLUMETEXTURE, d3dPF);
			if (mAutoGenMipmaps)
			{
				usage |= D3DUSAGE_AUTOGENMIPMAP;
				numMips = 0;
			}
		}
		else
		{
			// no mip map support for this kind of textures :(
			mNumMipmaps = 0;
			numMips = 1;
		}

		// create the texture
		hr = D3DXCreateVolumeTexture(	
				mpDev,								// device
				mWidth,								// dimension
				mHeight,
				mDepth,
				numMips,							// number of mip map levels
				usage,								// usage
				d3dPF,								// pixel format
                (mUsage & TU_RENDERTARGET)?
                D3DPOOL_DEFAULT : D3DPOOL_MANAGED,	// memory pool
				&mpVolumeTex);						// data pointer
		// check result and except if failed
		if (FAILED(hr))
		{
			this->_freeResources();
			Except( hr, "Error creating texture", "D3D9Texture::_createVolumeTex" );
		}

		// set the base texture we'll use in the render system
		hr = mpVolumeTex->QueryInterface(IID_IDirect3DBaseTexture9, (void **)&mpTex);
		if (FAILED(hr))
		{
			Except( hr, "Can't get base texture", "D3D9Texture::_createVolumeTex" );
			this->_freeResources();
		}
		
		// set final tex. attributes from tex. description
		// they may differ from the source image !!!
		D3DVOLUME_DESC desc;
		hr = mpVolumeTex->GetLevelDesc(0, &desc);
		if (FAILED(hr))
		{
			Except( hr, "Can't get texture description", "D3D9Texture::_createVolumeTex" );
			this->_freeResources();
		}
		this->_setFinalAttributes(desc.Width, desc.Height, desc.Depth, this->_getPF(desc.Format));
		
		// create a depth stencil if this is a render target
		if (mUsage & TU_RENDERTARGET)
			this->_createDepthStencil();

		// Set best filter type
		if(mAutoGenMipmaps)
		{
			hr = mpTex->SetAutoGenFilterType(_getBestFilterMethod());
			if(FAILED(hr))
			{
				Except( hr, "Could not set best autogen filter type", "D3D9Texture::_createCubeTex" );
			}
		}
	}
	/****************************************************************************************/
	void D3D9Texture::_initDevice(void)
	{ 
		assert(mpDev);
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
		static const String suffixes[6] = {"_rt", "_lf", "_up", "_dn", "_fr", "_bk"};
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
		for (size_t i = 0; i < 6; ++i)
			mCubeFaceNames[i] = baseName + suffixes[i] + ext;
	}
	/****************************************************************************************/
	void D3D9Texture::_setFinalAttributes(unsigned long width, unsigned long height, 
        unsigned long depth, PixelFormat format)
	{ 
		// set target texture attributes
		mHeight = height; 
		mWidth = width; 
        mDepth = depth;
		mFormat = format; 

		// Update size (the final size, not including temp space)
		// this is needed in Resource class
		unsigned short bytesPerPixel = mFinalBpp >> 3;
		if( !mHasAlpha && mFinalBpp == 32 )
			bytesPerPixel--;
		mSize = mWidth * mHeight * mDepth * bytesPerPixel 
            * (mTextureType == TEX_TYPE_CUBE_MAP)? 6 : 1;

		// say to the world what we are doing
		if (mWidth != mSrcWidth ||
			mHeight != mSrcHeight)
		{
			LogManager::getSingleton().logMessage("D3D9 : ***** Dimensions altered by the render system");
			LogManager::getSingleton().logMessage("D3D9 : ***** Source image dimensions : " + StringConverter::toString(mSrcWidth) + "x" + StringConverter::toString(mSrcHeight));
			LogManager::getSingleton().logMessage("D3D9 : ***** Texture dimensions : " + StringConverter::toString(mWidth) + "x" + StringConverter::toString(mHeight));
		}
		
		// Create list of subsurfaces for getBuffer()
		_createSurfaceList();
	}
	/****************************************************************************************/
	void D3D9Texture::_setSrcAttributes(unsigned long width, unsigned long height, 
        unsigned long depth, PixelFormat format)
	{ 
		// set source image attributes
		mSrcWidth = width; 
		mSrcHeight = height; 
		mSrcBpp = PixelUtil::getNumElemBits(format); 
        mHasAlpha = PixelUtil::getFlags(format) & PFF_HASALPHA; 
		// say to the world what we are doing
		switch (this->getTextureType())
		{
		case TEX_TYPE_1D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating 1D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading 1D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_2D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating 2D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading 2D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_3D:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating 3D RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading 3D Texture, image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		case TEX_TYPE_CUBE_MAP:
			if (mUsage & TU_RENDERTARGET)
				LogManager::getSingleton().logMessage("D3D9 : Creating Cube map RenderTarget, name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			else
				LogManager::getSingleton().logMessage("D3D9 : Loading Cube Texture, base image name : '" + this->getName() + "' with " + StringConverter::toString(mNumMipmaps) + " mip map levels");
			break;
		default:
			Except( Exception::ERR_INTERNAL_ERROR, "Unknown texture type", "D3D9Texture::_setSrcAttributes" );
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
		
		DWORD filterCaps = 0;
		// Minification filter is used for mipmap generation
		// Pick the best one supported for this tex type
		switch (this->getTextureType())
		{
		case TEX_TYPE_1D:		// Same as 2D
		case TEX_TYPE_2D:		filterCaps = mDevCaps.TextureFilterCaps;	break;
		case TEX_TYPE_3D:		filterCaps = mDevCaps.VolumeTextureFilterCaps;	break;
		case TEX_TYPE_CUBE_MAP:	filterCaps = mDevCaps.CubeTextureFilterCaps;	break;
		}
		if(filterCaps & D3DPTFILTERCAPS_MINFGAUSSIANQUAD)
			return D3DTEXF_GAUSSIANQUAD;
		
		if(filterCaps & D3DPTFILTERCAPS_MINFPYRAMIDALQUAD)
			return D3DTEXF_PYRAMIDALQUAD;
		
		if(filterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC)
			return D3DTEXF_ANISOTROPIC;
		
		if(filterCaps & D3DPTFILTERCAPS_MINFLINEAR)
			return D3DTEXF_LINEAR;
		
		if(filterCaps & D3DPTFILTERCAPS_MINFPOINT)
			return D3DTEXF_POINT;
		
		return D3DTEXF_POINT;
	}
	/****************************************************************************************/
	bool D3D9Texture::_canAutoGenMipmaps(DWORD srcUsage, D3DRESOURCETYPE srcType, D3DFORMAT srcFormat)
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
			pSurf8 = (BYTE*)rect.pBits + (iRow * rect.Pitch);
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

				// Red
				out32 = Bitwise::convertBitPattern( (DWORD)data32, (DWORD)0xFF000000, (DWORD)rMask );
				// Green
				out32 |= Bitwise::convertBitPattern( (DWORD)data32, (DWORD)0x00FF0000, (DWORD)gMask );
				// Blue
				out32 |= Bitwise::convertBitPattern( (DWORD)data32, (DWORD)0x0000FF00, (DWORD)bMask );
				// Alpha
				if( aMask > 0 )
				{
					out32 |= Bitwise::convertBitPattern( (DWORD)data32, (DWORD)0x000000FF, (DWORD)aMask );
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
		// This will perform any size conversion (inc stretching)
		IDirect3DSurface9 *pDstSurface; 
		hr = mpNormTex->GetSurfaceLevel(0, &pDstSurface);

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

        // Generate mipmaps
        if (mAutoGenMipmaps)
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
        else
        {
            // Software mipmaps
            if( FAILED( hr = D3DXFilterTexture( mpNormTex, NULL, D3DX_DEFAULT, D3DX_DEFAULT ) ) )
            {
                this->_freeResources();
                Except( hr, "Failed to filter texture (generate mip maps)", "D3D9Texture::_blitImageToNormTex" );
            }
        }

			
		SAFE_RELEASE(pDstSurface);
		SAFE_RELEASE(pSrcSurface);
	}
	/****************************************************************************************/
	void D3D9Texture::_blitImagesToCubeTex(const Image srcImages[])
	{
		HRESULT hr;
		D3DFORMAT dstFormat = _chooseD3DFormat();

		// we must loop through all 6 cube map faces :(
		for (size_t face = 0; face < 6; face++)
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
			//tmpImg.flipAroundX();
			// copy the buffer to our surface, 
			// _copyMemoryToSurface will do color conversion and flipping
			this->_copyMemoryToSurface(tmpImg.getData(), pSrcSurface);

			// Now we need to copy the source surface (where our image is) to 
			// either the the temp. texture level 0 surface (for s/w mipmaps)
			// or the final texture (for h/w mipmaps)
			IDirect3DSurface9 *pDstSurface; 
            hr = mpCubeTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, 0, &pDstSurface);
			// check result and except if failed
			if (FAILED(hr))
			{
				Except( hr, "Error getting level dest cube map surface", "D3D9Texture::_blitImagesToCubeTex" );
				SAFE_RELEASE(pSrcSurface);
				this->_freeResources();
			}

			// load the surface with an in memory buffer
			hr = D3DXLoadSurfaceFromSurface(
                pDstSurface, 
                NULL, 
                NULL, 
                pSrcSurface, 
                NULL, 
                NULL, 
                D3DX_DEFAULT, 
                0);
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

        // Mipmaps
        if (mAutoGenMipmaps)
        {
			// use best filtering method supported by hardware
			hr = mpTex->SetAutoGenFilterType(_getBestFilterMethod());
			if (FAILED(hr))
			{
				this->_freeResources();
				Except( hr, "Error generating mip maps", "D3D9Texture::_blitImageToCubeTex" );
			}
            mpCubeTex->GenerateMipSubLevels();
        }
        else
        {
            // Software mipmaps
            if( FAILED( hr = D3DXFilterTexture( mpCubeTex, NULL, D3DX_DEFAULT, D3DX_DEFAULT ) ) )
            {
                this->_freeResources();
                Except( hr, "Failed to filter texture (generate mip maps)", "D3D9Texture::_blitImageToCubeTex" );
            }

        }


	}
	/****************************************************************************************/
	D3DFORMAT D3D9Texture::_chooseD3DFormat()
	{
		// Choose closest supported D3D format as a D3D format
		return _getPF(_getClosestSupportedPF(mFormat));
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
	void D3D9Texture::_createSurfaceList()
	{
		IDirect3DSurface9 *surface;
		IDirect3DVolume9 *volume;
		D3D9HardwarePixelBuffer *buffer;
		assert(mpTex);
		// Make sure number of mips is right
		mNumMipmaps = mpTex->GetLevelCount() - 1;
			
		mSurfaceList.clear();
		switch(getTextureType()) {
		case TEX_TYPE_2D:
		case TEX_TYPE_1D:
			assert(mpNormTex);
			// For all mipmaps, store surfaces as HardwarePixelBufferSharedPtr
			for(int mip=0; mip<=mNumMipmaps; mip++)
			{
				if(mpNormTex->GetSurfaceLevel(mip, &surface) != D3D_OK)
					Except(Exception::ERR_RENDERINGAPI_ERROR, "Get surface level failed",
		 				"D3D9Texture::_createSurfaceList");

				buffer = new D3D9HardwarePixelBuffer(surface);
				if(mNumMipmaps>0 && mip == 0 && (mUsage & TU_AUTOMIPMAP)) 
					buffer->_setMipmapping(true, mAutoGenMipmaps, mpTex);
					
				mSurfaceList.push_back(HardwarePixelBufferSharedPtr(buffer));
			}
			break;
		case TEX_TYPE_CUBE_MAP:
			assert(mpCubeTex);
			// For all faces and mipmaps, store surfaces as HardwarePixelBufferSharedPtr
			for(int face=0; face<6; face++)
			{
				for(int mip=0; mip<=mNumMipmaps; mip++)
				{
					if(mpCubeTex->GetCubeMapSurface((D3DCUBEMAP_FACES)face, mip, &surface) != D3D_OK)
						Except(Exception::ERR_RENDERINGAPI_ERROR, "Get cubemap surface failed",
		 				"D3D9Texture::getBuffer");
						
					buffer = new D3D9HardwarePixelBuffer(surface);
					if(mNumMipmaps>0 && mip == 0 && (mUsage & TU_AUTOMIPMAP)) 
						buffer->_setMipmapping(true, mAutoGenMipmaps, mpTex);
						
					mSurfaceList.push_back(HardwarePixelBufferSharedPtr(buffer));
				}
			}
			break;
		case TEX_TYPE_3D:
			assert(mpVolumeTex);
			// For all mipmaps, store surfaces as HardwarePixelBufferSharedPtr
			for(int mip=0; mip<=mNumMipmaps; mip++)
			{
				if(mpVolumeTex->GetVolumeLevel(mip, &volume) != D3D_OK)
					Except(Exception::ERR_RENDERINGAPI_ERROR, "Get volume level failed",
		 				"D3D9Texture::getBuffer");	
						
				buffer = new D3D9HardwarePixelBuffer(volume);
				if(mNumMipmaps>0 && mip == 0 && (mUsage & TU_AUTOMIPMAP)) 
					buffer->_setMipmapping(true, mAutoGenMipmaps, mpTex);
					
				mSurfaceList.push_back(HardwarePixelBufferSharedPtr(buffer));
			}
			break;
		};
	}
	/****************************************************************************************/
	HardwarePixelBufferSharedPtr D3D9Texture::getBuffer(int face, int mipmap) 
	{
		if(getTextureType() != TEX_TYPE_CUBE_MAP && face != 0)
			Except(Exception::ERR_INVALIDPARAMS, "Normal textures have only face 0",
					"D3D9Texture::getBuffer");
		if(face < 0 || face >= 6)
			Except(Exception::ERR_INVALIDPARAMS, "A three dimensional cube has six faces",
					"D3D9Texture::getBuffer");
		if(mipmap < 0 || mipmap > mNumMipmaps)
			Except(Exception::ERR_INVALIDPARAMS, "Mipmap index out of range",
					"D3D9Texture::getBuffer");
		int idx = face*(mNumMipmaps+1) + mipmap;
		assert(idx < mSurfaceList.size());
		return mSurfaceList[idx];
	}
	
	/****************************************************************************************/
	PixelFormat D3D9Texture::_getPF(D3DFORMAT d3dPF)
	{
		switch(d3dPF)
		{
		case D3DFMT_A8:
			return PF_A8;
		case D3DFMT_L8:
			return PF_L8;
		case D3DFMT_L16:
			return PF_L16;
		case D3DFMT_A4L4:
			return PF_A4L4;
		case D3DFMT_A4R4G4B4:
			return PF_A4R4G4B4;
		case D3DFMT_R5G6B5:
			return PF_R5G6B5;
		case D3DFMT_R8G8B8:
			return PF_R8G8B8;
		case D3DFMT_X8R8G8B8:
			return PF_X8R8G8B8;
		case D3DFMT_A8R8G8B8:
			return PF_A8R8G8B8;
		case D3DFMT_X8B8G8R8:
			return PF_X8B8G8R8;
		case D3DFMT_A8B8G8R8:
			return PF_A8B8G8R8;
		case D3DFMT_A2R10G10B10:
			return PF_A2R10G10B10;
        case D3DFMT_A2B10G10R10:
           return PF_A2B10G10R10;
		case D3DFMT_A16B16G16R16F: // verify?
			return PF_FLOAT16_RGBA;
		case D3DFMT_A32B32G32R32F: // verify?
			return PF_FLOAT32_RGBA;
		case D3DFMT_DXT1:
			return PF_DXT1;
		case D3DFMT_DXT2:
			return PF_DXT2;
		case D3DFMT_DXT3:
			return PF_DXT3;
		case D3DFMT_DXT4:
			return PF_DXT4;
		case D3DFMT_DXT5:
			return PF_DXT5;
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
		case PF_L16:
			return D3DFMT_L16;
		case PF_A8:
			return D3DFMT_A8;
		case PF_A4L4:
			return D3DFMT_A4L4;
		case PF_R5G6B5:
			return D3DFMT_R5G6B5;
		case PF_A4R4G4B4:
			return D3DFMT_A4R4G4B4;
		case PF_R8G8B8:
			return D3DFMT_R8G8B8;
		case PF_A8R8G8B8:
			return D3DFMT_A8R8G8B8;
		case PF_A8B8G8R8:
			return D3DFMT_A8B8G8R8;
		case PF_X8R8G8B8:
			return D3DFMT_X8R8G8B8;
		case PF_X8B8G8R8:
			return D3DFMT_X8B8G8R8;
		case PF_A2B10G10R10:
            return D3DFMT_A2B10G10R10;
		case PF_A2R10G10B10:
			return D3DFMT_A2R10G10B10;
		case PF_FLOAT16_RGBA:
			return D3DFMT_A16B16G16R16F;
		case PF_FLOAT32_RGBA:
			return D3DFMT_A32B32G32R32F;
		case PF_DXT1:
			return D3DFMT_DXT1;
		case PF_DXT2:
			return D3DFMT_DXT2;
		case PF_DXT3:
			return D3DFMT_DXT3;
		case PF_DXT4:
			return D3DFMT_DXT4;
		case PF_DXT5:
			return D3DFMT_DXT5;
		case PF_UNKNOWN:
		default:
			return D3DFMT_UNKNOWN;
		}
	}
	/****************************************************************************************/
	PixelFormat D3D9Texture::_getClosestSupportedPF(PixelFormat ogrePF)
	{
		if (_getPF(ogrePF) != D3DFMT_UNKNOWN)
		{
			return ogrePF;
		}
		switch(ogrePF)
		{
		case PF_L4A4:
			return PF_A4L4;
		case PF_B5G6R5:
			return PF_R5G6B5;
		case PF_B4G4R4A4:
			return PF_A4R4G4B4;
		case PF_B8G8R8:
			return PF_R8G8B8;
		case PF_B8G8R8A8:
			return PF_A8R8G8B8;
		case PF_FLOAT16_RGB:
			return PF_FLOAT16_RGBA;
		case PF_FLOAT32_RGB:
			return PF_FLOAT32_RGBA;
		case PF_UNKNOWN:
		default:
			return PF_A8R8G8B8;
		}
	}

	/****************************************************************************************/
}

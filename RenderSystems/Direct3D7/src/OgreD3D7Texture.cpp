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
#include "OgreD3D7Texture.h"
#include "OgreException.h"
#include "OgreBitwise.h"
#include "OgreImage.h"
#include "OgreLogManager.h"

#include "png.h"
#include "d3dutil.h"
#include "d3dxcore.h"
#include "OgreRoot.h"

namespace Ogre {

    //---------------------------------------------------------------------------------------------
    D3DTexture::D3DTexture(String name, LPDIRECT3DDEVICE7 lpDirect3dDevice)
    {
        mName = name;
        mD3DDevice = lpDirect3dDevice; mD3DDevice->AddRef();

        // Default to 16-bit texture
        enable32Bit( false );
    }

    //---------------------------------------------------------------------------------------------
    D3DTexture::~D3DTexture()
    {
        if( mIsLoaded )
            unload();

        __safeRelease( &mD3DDevice );
    }

    //---------------------------------------------------------------------------------------------
    void D3DTexture::blitToTexture( 
        const Image &src, unsigned uStartX, unsigned uStartY )
    {
		blitImage( src, Image::Rect( uStartX, uStartY, src.getWidth(), src.getHeight() ),
			Image::Rect( 0, 0, getWidth(), getHeight() ) );
    }

    //---------------------------------------------------------------------------------------------
    void D3DTexture::blitImage( const Image& src, 
            const Image::Rect imgRect, const Image::Rect texRect )
    {
        /* We need a temporary surface in which to load the image data. */
        LPDIRECTDRAWSURFACE7 pddsTempSurface;
        HRESULT hr;
        D3DX_SURFACEFORMAT surfFmt, texFmt;

        /* Compute the pixel format for the image. */
        if( src.getBPP() == 16 )
        {
            if( src.getHasAlpha() )
                surfFmt = D3DX_SF_A4R4G4B4;
            else
                surfFmt = D3DX_SF_R5G6B5;
        }
        else
        {
            if( src.getHasAlpha() )
                surfFmt = D3DX_SF_A8R8G8B8;
            else
                surfFmt = D3DX_SF_R8G8B8;
        }

        /* Compute the current pixel format of the texture. */
        if( mFinalBpp == 16 )
        {
            if( mHasAlpha )
                texFmt = D3DX_SF_A4R4G4B4;
            else
                texFmt = D3DX_SF_R5G6B5;
        }
        else
        {
            if( mHasAlpha )
                texFmt = D3DX_SF_A8R8G8B8;
            else
                texFmt = D3DX_SF_R8G8B8;
        }

        /* Oh, you, endianness! How thy beauty never ceases to amaze me. NOT! */
        Image tempImg = src;
        {
            /* Scoping in order to get rid of the local vars. */
            uchar *c = tempImg.getData();
            for( uint i = 0; i < src.getSize(); i+= src.getBPP() >> 3, c += src.getBPP() >> 3 )
            {
                uchar tmp;

                if( src.getBPP() == 16 )
                {
                    tmp = c[0]; c[0] = c[1]; c[1] = tmp;
                }
                else
                {
                    tmp = c[0]; c[0] = c[2]; c[2] = tmp;
                }
            }
        }

        /* We generate the mip-maps by hand. */
        DWORD mipFlag, numMips;
		mipFlag = D3DX_TEXTURE_NOMIPMAP;

        /* Set the width and height. */
        DWORD dwWidth = src.getWidth(), dwHeight = src.getHeight();

        /* Create the temporary surface. */
        if( FAILED( hr = D3DXCreateTexture(
            mD3DDevice,
            NULL,
            &dwWidth,
            &dwHeight,
            &texFmt,
            NULL,
            &pddsTempSurface,
            &numMips ) ) )
        {
            Except( hr, "Error during blit operation.", "D3DTexture::blitImage" );
        }

        /* Load the image into the temporary surface. */
        if( FAILED( hr = D3DXLoadTextureFromMemory(
            mD3DDevice,
            pddsTempSurface,
            D3DX_DEFAULT,
            tempImg.getData(),
            NULL,
            surfFmt,
            D3DX_DEFAULT,
            NULL,
            D3DX_FT_LINEAR ) ) )
        {
            pddsTempSurface->Release();
            Except( hr, "Error during blit operation.", "D3DTexture::blitImage" );
        }

        /* We have to make sure that the source image wasn't stretched during the loading. */
        Image::Rect finalRect;

        Real fHeightFactor = Real( dwHeight ) / Real( src.getHeight() );
        Real fWidthFactor = Real( dwWidth ) / Real( src.getWidth() );

        finalRect.bottom = Real( imgRect.bottom ) * fHeightFactor;
        finalRect.top    = Real( imgRect.top )    * fHeightFactor;
        finalRect.left   = Real( imgRect.left )   * fWidthFactor;
        finalRect.right  = Real( imgRect.right )  * fWidthFactor;

		/* We have to use a mirror up/down (around the X axis) effect since in DirectX the
		   positive Y is downward, which is different from OGRE's way. */
        DDBLTFX  ddbltfx;
        ZeroMemory(&ddbltfx, sizeof(ddbltfx));

        ddbltfx.dwSize = sizeof(ddbltfx);
        ddbltfx.dwDDFX = DDBLTFX_MIRRORUPDOWN;

        if( FAILED( hr = mSurface->Blt(
            (RECT*)&texRect,
            pddsTempSurface, 
            (RECT*)&finalRect,
            DDBLT_WAIT | DDBLT_DDFX,
            &ddbltfx ) ) )
        {
            pddsTempSurface->Release();
            Except( hr, "Error during blit operation.", "D3DTexture::blitImage" );
        }

        /* Load the image in all the mip-maps (if there are any, that is). */
        LPDIRECTDRAWSURFACE7 ddsMipLevel, ddsNextLevel;
        DDSCAPS2 ddsCaps;
        HRESULT mipRes = DD_OK;
        uint mipLevel = 1;

        ZeroMemory(&ddsCaps, sizeof(DDSCAPS2));
        ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

        /* Get the base level and increae the reference count. */
        ddsMipLevel = mSurface;
        ddsMipLevel->AddRef();

        /* While we can get a next level in the mip-map chain. */
        while( ddsMipLevel->GetAttachedSurface( &ddsCaps, &ddsNextLevel ) == DD_OK )
        {
            /* Calculate the destination rect. */
            RECT mipRect = { 
                texRect.left >> mipLevel,
                texRect.top  >> mipLevel,
                texRect.right >> mipLevel,
                texRect.bottom >> mipLevel
            };

            /* And do a blit from the base level into the next level. */
            ddsNextLevel->Blt(
                &mipRect,
                mSurface,
                (RECT*)&texRect,
                DDBLT_WAIT,
                NULL );

            /* Release the current level and get the next one, incrementing the mip depth. */
            ddsMipLevel->Release();
            ddsMipLevel = ddsNextLevel;
            mipLevel++;
        }

        /* Release the last mip-map level surface. */
        ddsMipLevel->Release();
    }

    //---------------------------------------------------------------------------------------------
    void D3DTexture::loadImage( const Image & img )
    {
        if( mIsLoaded )
        {
            unload();
        }

        LogManager::getSingleton().logMessage( 
            LML_TRIVIAL,
            "D3DTexture: Loading %s with %d mipmaps from Image.", 
            mName.c_str(), mNumMipMaps );

        Image *tempImg = NULL;

        mHasAlpha = img.getHasAlpha();
        mSrcWidth = img.getWidth();
        mSrcHeight = img.getHeight();
        mSrcBpp = img.getBPP();

        /* Now that we have the image's parameters, create the D3D surface based on them. */
        createSurface();

        /* Since calculating gamma requires an extra copy of the data, we may want to skip that. */
        if( mGamma == 1.0 )
        {
            blitImage( 
                const_cast< Image & >( img ), 
                Image::Rect( 0, 0, mSrcWidth, mSrcHeight ),
                Image::Rect( 0, 0, mWidth, mHeight ) );
        }
        else
        {
            tempImg = new Image( img );
            Image::applyGamma( 
                tempImg->getData(), mGamma, 
                static_cast< uint >( tempImg->getSize() ), mSrcBpp );

            blitImage( 
                *tempImg, 
                Image::Rect( 0, 0, mSrcWidth, mSrcHeight ),
                Image::Rect( 0, 0, mWidth, mHeight ) );
        }
        
        short bytesPerPixel = mFinalBpp >> 3;
        if( !mHasAlpha && mFinalBpp == 32 )
        {
            bytesPerPixel--;
        }
        mSize = mWidth * mHeight * bytesPerPixel;

        mIsLoaded = true;

        if( tempImg )
            delete tempImg;
    }
    
    //---------------------------------------------------------------------------------------------
    void D3DTexture::load(void)
    {
        Image img;
        img.load( mName );

        loadImage( img );
    }

    //---------------------------------------------------------------------------------------------
    void D3DTexture::unload()
    {
        if( mIsLoaded )
        {
            __safeRelease( &mSurface );
            mIsLoaded = false;
        }
    }

    //---------------------------------------------------------------------------------------------
    void D3DTexture::createSurface(void)
    {
        D3DDEVICEDESC7 ddDesc;
        ZeroMemory( &ddDesc, sizeof(D3DDEVICEDESC7) );

        if( FAILED( mD3DDevice->GetCaps( &ddDesc ) ) )
            Except( Exception::ERR_RENDERINGAPI_ERROR, 
                    "Could not retrieve Direct3D Device caps.",
                    "D3DTexture::createSurfaces" );

        // Create a surface descriptor.
        DDSURFACEDESC2 ddsd;
        D3DUtil_InitSurfaceDesc( ddsd );

        ddsd.dwSize          = sizeof(DDSURFACEDESC2);
        ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH |
                               DDSD_PIXELFORMAT | DDSD_TEXTURESTAGE;
        ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
        ddsd.dwWidth         = mSrcWidth;
        ddsd.dwHeight        = mSrcHeight;

        /* If we want to have mip-maps, set the flags. Note that if the
           texture is the render target type mip-maps are automatically 
           disabled. */
        if( mNumMipMaps && !isRenderTarget() )
        {
            ddsd.dwFlags |= DDSD_MIPMAPCOUNT;
            ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;

            ddsd.dwMipMapCount = mNumMipMaps;
        }
        /* If the texture is a render target, set the corresponding flags */
        else if( isRenderTarget() )
        {
            ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE;
        }

        /* The pixel format is always RGB */
        ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
        ddsd.ddpfPixelFormat.dwRGBBitCount = mFinalBpp;
        
        if( mHasAlpha )
        {
            ddsd.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
        }

        /* Compute the bit masks */
        if( mFinalBpp == 16 )
        {
            if( mHasAlpha )
            {
                /* 4-4-4-4 ARGB */
                ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xf000;
                ddsd.ddpfPixelFormat.dwRBitMask        = 0x0f00;
                ddsd.ddpfPixelFormat.dwGBitMask        = 0x00f0;
                ddsd.ddpfPixelFormat.dwBBitMask        = 0x000f;
            }
            else
            {
                /* 5-6-5 RGB */
                ddsd.ddpfPixelFormat.dwRBitMask = 0xf800;
                ddsd.ddpfPixelFormat.dwGBitMask = 0x07e0;
                ddsd.ddpfPixelFormat.dwBBitMask = 0x001f;
            }
        }
        else
        {
            if( mHasAlpha )
            {
                /* 8-8-8 RGB */
                ddsd.ddpfPixelFormat.dwRBitMask        = 0xff0000;
                ddsd.ddpfPixelFormat.dwGBitMask        = 0x00ff00;
                ddsd.ddpfPixelFormat.dwBBitMask        = 0x0000ff;
            }
            else
            {
                /* 8-8-8-8 ARGB */
                ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
                ddsd.ddpfPixelFormat.dwRBitMask        = 0x00ff0000;
                ddsd.ddpfPixelFormat.dwGBitMask        = 0x0000ff00;
                ddsd.ddpfPixelFormat.dwBBitMask        = 0x000000ff;
            }
        }

        /* Try and get the texture managed by DirectX. This only happens for true 
           hardware devices (since software devices don't have a video memory). */
        if( ddDesc.deviceGUID == IID_IDirect3DHALDevice || ddDesc.deviceGUID == IID_IDirect3DTnLHalDevice )
        {
            ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
        }
        else
        {
            ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
        }

        /* Change texture size so that it is a power of 2, if needed. */
        if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2 )
        {
            for( ddsd.dwWidth = 1; mSrcWidth > ddsd.dwWidth; ddsd.dwWidth <<= 1 );
            for( ddsd.dwHeight = 1; mSrcHeight > ddsd.dwHeight; ddsd.dwHeight <<= 1 );
        }

        /* Change texture size so that it is square, if needed. Note that we made it a
           power of 2 in the above test, so here we just have to find the bigger dimension
           and make it the side length. */
        if( ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY )
        {
            if( ddsd.dwWidth > ddsd.dwHeight ) 
                ddsd.dwHeight = ddsd.dwWidth;
            else                               
                ddsd.dwWidth  = ddsd.dwHeight;
        }

        /* Register the possibly modified dimensions. */
        mWidth = ddsd.dwWidth;
        mHeight = ddsd.dwHeight;

        LPDIRECTDRAWSURFACE7 pddsRender;
        LPDIRECTDRAW7        pDD;

        /* Get a DirectDraw interface. */
        mD3DDevice->GetRenderTarget( &pddsRender );
        pddsRender->GetDDInterface( (VOID**)&pDD );
        pddsRender->Release();

        HRESULT hr;

        /* Now create the surface. */
        if( FAILED( hr = pDD->CreateSurface( &ddsd, &mSurface, NULL ) ) )
        {
            pDD->Release();
            Except( Exception::ERR_RENDERINGAPI_ERROR,
                "Could not create DirectDraw surface.",
                "D3DTexture::createSurfaces" );
        }

        /* Release the DirectDraw interface used in the surface creation */
        pDD->Release();
    }

    LPDIRECTDRAWSURFACE7 D3DTexture::getDDSurface(void)
    {
        return mSurface;
    }

    D3D7RenderTargetTexture::D3D7RenderTargetTexture( String name, LPDIRECT3DDEVICE7 lpDirect3dDevice )
        : D3DTexture( name, lpDirect3dDevice )
    {
    }
}


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

#include "d3dutil.h"
#include "d3dxcore.h"
#include "OgreRoot.h"

namespace Ogre {

    static bool OgreFormatRequiresEndianFlipping( PixelFormat format )
    {
        switch( format )
        {
        case PF_L4A4:
        case PF_B5G6R5:
        case PF_B4G4R4A4:
        case PF_B8G8R8:
        case PF_B8G8R8A8:
        case PF_B10G10R10A2:
            return true;

        case PF_L8:
        case PF_A8:
        case PF_R5G6B5:
        case PF_A4R4G4B4:
        case PF_R8G8B8:
        case PF_A8R8G8B8:
        case PF_A2R10G10B10:
        case PF_A4L4:
        case PF_UNKNOWN:
        default:
            return false;
        }
    }
    static D3DX_SURFACEFORMAT OgreFormat_to_D3DXFormat( PixelFormat format )
    {
        switch( format )
        {
        case PF_L8:
            return D3DX_SF_L8;
        case PF_A8:
            return D3DX_SF_A8;
        case PF_R5G6B5:
        case PF_B5G6R5:
            return D3DX_SF_R5G6B5;
        case PF_A4R4G4B4:
        case PF_B4G4R4A4:
            return D3DX_SF_A4R4G4B4;
        case PF_R8G8B8:
        case PF_B8G8R8:
            return D3DX_SF_R8G8B8;
        case PF_A8R8G8B8:
        case PF_B8G8R8A8:
            return D3DX_SF_A8R8G8B8;
        case PF_UNKNOWN:
        case PF_A4L4:
        case PF_L4A4:
        case PF_A2R10G10B10:
        case PF_B10G10R10A2:
        default:
            return D3DX_SF_UNKNOWN;
        }
    }

    static void OgreFormat_to_DDPixelFormat( PixelFormat format, DDPIXELFORMAT & out )
    {
        memset( &out, 0, sizeof( DDPIXELFORMAT ) );
        out.dwSize = sizeof( DDPIXELFORMAT );

        switch( format )
        {
        case PF_A8:
            out.dwFlags = DDPF_ALPHA;
            out.dwAlphaBitDepth = 8;
            
            break;

        case PF_L8:
            out.dwFlags = DDPF_LUMINANCE ;
            out.dwLuminanceBitCount = 8;

            break;

        case PF_A4L4:
        case PF_L4A4:
            out.dwFlags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
            out.dwLuminanceBitCount = 4;

            if( format == PF_A4L4 )
            {
                out.dwLuminanceAlphaBitMask = 0xf0;
                out.dwLuminanceBitMask = 0x0f;
            }
            else
            {
                out.dwLuminanceAlphaBitMask = 0x0f;
                out.dwLuminanceBitMask = 0xf0;
            }

            break;

        case PF_R5G6B5:
        case PF_B5G6R5:
            out.dwFlags = DDPF_RGB;
            out.dwRGBBitCount = 16;

            if( format == PF_R5G6B5 )
            {
                out.dwRBitMask = 0xf800;
                out.dwGBitMask = 0x07e0;
                out.dwBBitMask = 0x001f;
            }
            else
            {
                out.dwRBitMask = 0x001f;
                out.dwGBitMask = 0x07e0;
                out.dwBBitMask = 0xf800;
            }

            break;

        case PF_A4R4G4B4:
        case PF_B4G4R4A4:
            out.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
            out.dwRGBBitCount = 12;

            if( format == PF_A4R4G4B4 )
            {
                out.dwRGBAlphaBitMask = 0xf000;
                out.dwRBitMask        = 0x0f00;
                out.dwGBitMask        = 0x00f0;
                out.dwBBitMask        = 0x000f;
            }
            else
            {
                out.dwRGBAlphaBitMask = 0x000f;
                out.dwRBitMask          = 0x00f0;
                out.dwGBitMask          = 0x0f00;
                out.dwBBitMask          = 0xf000;
            }

            break;

        case PF_R8G8B8:
        case PF_B8G8R8:
            out.dwFlags = DDPF_RGB;
            out.dwRGBBitCount = 24;

            if( format == PF_R8G8B8 )
            {
                out.dwRBitMask = 0xff0000;
                out.dwGBitMask = 0x00ff00;
                out.dwBBitMask = 0x0000ff;
            }
            else
            {
                out.dwRBitMask = 0x0000ff;
                out.dwGBitMask = 0x00ff00;
                out.dwBBitMask = 0xff0000;
            }

            break;

        case PF_A8R8G8B8:
        case PF_B8G8R8A8:
            out.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
            out.dwRGBBitCount = 24;

            if( format == PF_A8R8G8B8 )
            {
                out.dwRGBAlphaBitMask = 0xff000000;
                out.dwRBitMask        = 0x00ff0000;
                out.dwGBitMask        = 0x0000ff00;
                out.dwBBitMask        = 0x000000ff;
            }
            else
            {
                out.dwRGBAlphaBitMask = 0x000000ff;
                out.dwRBitMask        = 0x0000ff00;
                out.dwGBitMask        = 0x00ff0000;
                out.dwBBitMask        = 0xff000000;
            }

            break;
        }
    }

    //---------------------------------------------------------------------------------------------
    D3DTexture::D3DTexture(const String& name, TextureType texType, LPDIRECT3DDEVICE7 lpDirect3dDevice, TextureUsage usage )
    {
        mD3DDevice = lpDirect3dDevice; mD3DDevice->AddRef();

		mName = name;
		mTextureType = texType;
		mUsage = usage;

        // Default to 16-bit texture
        enable32Bit( false );
    }
    //---------------------------------------------------------------------------------------------
    D3DTexture::D3DTexture( 
        const String& name, 
		TextureType texType, 
        IDirect3DDevice7 * lpDirect3dDevice, 
        uint width, 
        uint height, 
        uint num_mips,
        PixelFormat format,
        TextureUsage usage )
    {
        mD3DDevice = lpDirect3dDevice; mD3DDevice->AddRef();

		mName = name;
		mTextureType = texType;
		mSrcWidth = width;
        mSrcHeight = height;
        mNumMipMaps = num_mips;

        mUsage = usage;
        mFormat = format;
        mSrcBpp = mFinalBpp = Image::getNumElemBits( mFormat );
        mHasAlpha = Image::formatHasAlpha(mFormat);

        createSurface();
        mIsLoaded = true;
    }
    //---------------------------------------------------------------------------------------------
    D3DTexture::~D3DTexture()
    {
        if( mIsLoaded )
            unload();

        __safeRelease( &mD3DDevice );
    }
	/****************************************************************************************/
    void D3DTexture::blitToTexture( 
        const Image &src, unsigned uStartX, unsigned uStartY )
    {
        blitImage( src, Image::Rect( uStartX, uStartY, src.getWidth(), src.getHeight() ),
            Image::Rect( 0, 0, Texture::getWidth(), Texture::getHeight() ) );
    }

    //---------------------------------------------------------------------------------------------
    void D3DTexture::blitImage( const Image& src, 
            const Image::Rect imgRect, const Image::Rect texRect )
    {
        OgreGuard( "D3DTexture::blitImage" );

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

        /* Oh, you, endianness! How thy beauty never ceases to amaze me. NOT! 
           In other words we need to create a temporary image in which we can
           convert the data (and also apply gamma transformation). */
        Image tempImg( src );
        Image::applyGamma( tempImg.getData(), mGamma, static_cast< uint >( tempImg.getSize() ), tempImg.getBPP() );
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

            /* Blit using D3DX in order to use bilinear filtering. */
            D3DXLoadTextureFromSurface(
                mD3DDevice,
                ddsNextLevel,
                0,
                mSurface,
                (RECT*)&texRect,
                (RECT*)&mipRect,
                D3DX_FT_LINEAR );

            /* Release the current level and get the next one, incrementing the mip depth. */
            ddsMipLevel->Release();
            ddsMipLevel = ddsNextLevel;
            mipLevel++;
        }

        /* Release the last mip-map level surface. */
        ddsMipLevel->Release();

        /* Release the temporary surface. */
        pddsTempSurface->Release();

		OgreUnguard();
    }
    //---------------------------------------------------------------------------------------------
HRESULT WINAPI testEnumAtt( 
  LPDIRECTDRAWSURFACE7 lpDDSurface,  
  LPDDSURFACEDESC2 desc,  
  LPVOID lpContext                  
)
{
    if (desc->dwWidth == 512)
    {
        int i = 0;
        i = 1;
    }
    return DDENUMRET_OK;

}

    void D3DTexture::blitImage3D( const Image src[], 
            const Image::Rect imgRect, const Image::Rect texRect )
    {
        OgreGuard( "D3DTexture::blitImage3D" );

        HRESULT hr;

        //mSurface->EnumAttachedSurfaces(NULL, testEnumAtt);


        for (int face = 0; face < 6; ++face)
		{
			/* We need a temporary surface in which to load the image data. */
			LPDIRECTDRAWSURFACE7 pddsTempSurface;
			D3DX_SURFACEFORMAT surfFmt, texFmt;
			/* Compute the pixel format for the image. */
			if( src[face].getBPP() == 16 )
			{
				if( src[face].getHasAlpha() )
					surfFmt = D3DX_SF_A4R4G4B4;
				else
					surfFmt = D3DX_SF_R5G6B5;
			}
			else
			{
				if( src[face].getHasAlpha() )
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

			/* Oh, you, endianness! How thy beauty never ceases to amaze me. NOT! 
			In other words we need to create a temporary image in which we can
			convert the data (and also apply gamma transformation). */
			Image tempImg( src[face] );
			Image::applyGamma( tempImg.getData(), mGamma, static_cast< uint >( tempImg.getSize() ), tempImg.getBPP() );
			{
				/* Scoping in order to get rid of the local vars. */
				uchar *c = tempImg.getData();
				for( uint i = 0; i < src[face].getSize(); i+= src[face].getBPP() >> 3, c += src[face].getBPP() >> 3 )
				{
					uchar tmp;

					if( src[face].getBPP() == 16 )
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
			DWORD dwWidth = src[face].getWidth(), dwHeight = src[face].getHeight();

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

			Real fHeightFactor = Real( dwHeight ) / Real( src[face].getHeight() );
			Real fWidthFactor = Real( dwWidth ) / Real( src[face].getWidth() );

			finalRect.bottom = Real( imgRect.bottom ) * fHeightFactor;
			finalRect.top    = Real( imgRect.top )    * fHeightFactor;
			finalRect.left   = Real( imgRect.left )   * fWidthFactor;
			finalRect.right  = Real( imgRect.right )  * fWidthFactor;

			// Get cube map face to blit to
			LPDIRECTDRAWSURFACE7 pCubeFace;
			DDSCAPS2 cubeCaps;
			ZeroMemory(&cubeCaps, sizeof(DDSCAPS2));
			switch (face)
			{
			case 0:
				// left

                // WTF???
                // If we try to retrieve DDSCAPS2_CUBEMAP_POSITIVEX we actually get
                // the FIRST MIPMAP of the +X cubemap surface and we fail on exit
                // the size is wrong
                // IS THIS A D3D7 BUG??
                // The below hack to get around it for now but leaves surface blank
                //continue;
                //
				cubeCaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX;
				break;
			case 1:
				// right
				cubeCaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX;
				break;
			case 2:
				// up
				cubeCaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY;
				break;
			case 3:
				// down
				cubeCaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY;
				break;
			case 4:
				// front - NB DirectX is backwards
				cubeCaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ;
				break;
			case 5:
				// back - NB DirectX is backwards
				cubeCaps.dwCaps2 = DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ;
				break;
			}

            if (FAILED(hr = mSurface->GetAttachedSurface( &cubeCaps, &pCubeFace)))
			{
				pddsTempSurface->Release();
				Except( hr, "Error getting cube face surface.", "D3DTexture::blitImage" );
			}

            if( FAILED( hr = pCubeFace->Blt(
				NULL,
				pddsTempSurface, 
				NULL,
				DDBLT_WAIT,
				NULL ) ) )
			{
				pddsTempSurface->Release();
                char msg[256];
                D3DXGetErrorString(hr, 256, msg);
                Except( hr, String("Error during blit operation: ") + msg, "D3DTexture::blitImage" );
			}

			/// Load the image in all the mip-maps (if there are any, that is). 
            
			LPDIRECTDRAWSURFACE7 ddsMipLevel, ddsNextLevel;
			DDSCAPS2 ddsCaps;
			HRESULT mipRes = DD_OK;
			uint mipLevel = 1;

			ZeroMemory(&ddsCaps, sizeof(DDSCAPS2));
			ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

			// Get the base level and increae the reference count. 
			ddsMipLevel = pCubeFace;
			ddsMipLevel->AddRef();

			/// While we can get a next level in the mip-map chain. 
			while( ddsMipLevel->GetAttachedSurface( &ddsCaps, &ddsNextLevel ) == DD_OK )
			{
				// Calculate the destination rect. 
				RECT mipRect = { 
					texRect.left >> mipLevel,
					texRect.top  >> mipLevel,
					texRect.right >> mipLevel,
					texRect.bottom >> mipLevel
				};

				// Blit using D3DX in order to use bilinear filtering. 
				D3DXLoadTextureFromSurface(
					mD3DDevice,
					ddsNextLevel,
					0,
					pCubeFace,
					(RECT*)&texRect,
					(RECT*)&mipRect,
					D3DX_FT_LINEAR );

				// Release the current level and get the next one, incrementing the mip depth. 
				ddsMipLevel->Release();
				ddsMipLevel = ddsNextLevel;
				mipLevel++;
			}

			// Release the last mip-map level surface. 
			ddsMipLevel->Release();
            
		}
        OgreUnguard();
    }

    //---------------------------------------------------------------------------------------------
    void D3DTexture::copyToTexture( Texture * target )
    {
        HRESULT hr;
        D3DTexture * other;

        if( target->getUsage() != mUsage )
            return;

        other = reinterpret_cast< D3DTexture * >( target );

        //if( FAILED( hr = other->getDDSurface()->BltFast( 0, 0, mSurface, NULL, DDBLTFAST_WAIT ) ) )
        if( FAILED( hr = other->getDDSurface()->Blt( NULL, mSurface, NULL, DDBLT_WAIT, NULL ) ) )
        {
            Except( Exception::ERR_RENDERINGAPI_ERROR, "Couldn't blit!", "" );
        }
    }

    //---------------------------------------------------------------------------------------------
    void D3DTexture::loadImage( const Image & img )
    {
        OgreGuard( "D3DTexture::loadImage" );

        if( mIsLoaded )
            unload();

        LogManager::getSingleton().logMessage( 
            LML_TRIVIAL,
            "D3DTexture: Loading %s with %d mipmaps from Image.", 
            Texture::mName.c_str(), mNumMipMaps );

        /* Get parameters from the Image */
        mHasAlpha = img.getHasAlpha();
        mSrcWidth = img.getWidth();
        mSrcHeight = img.getHeight();
        mSrcBpp = img.getBPP();
        mFormat = img.getFormat();

        /* Now that we have the image's parameters, create the D3D surface based on them. */
        createSurface();

        /* Blit to the image. This also creates the mip-maps and applies gamma. */
        blitImage( 
            img, 
            Image::Rect( 0, 0, mSrcWidth, mSrcHeight ),
            Image::Rect( 0, 0, Texture::mWidth, Texture::mHeight ) );
        
        short bytesPerPixel = mFinalBpp >> 3;
        if( !mHasAlpha && mFinalBpp == 32 )
        {
            bytesPerPixel--;
        }
        mSize = Texture::mWidth * Texture::mHeight * bytesPerPixel;

        mIsLoaded = true;

        OgreUnguard();
    }
    //---------------------------------------------------------------------------------------------
    void D3DTexture::loadImage3D( const Image imgs[] )
    {
        OgreGuard( "D3DTexture::loadImage" );

        if( mIsLoaded )
            unload();

        LogManager::getSingleton().logMessage( 
            LML_TRIVIAL,
            "D3DTexture: Loading cubemap %s with %d mipmaps from Image.", 
            Texture::mName.c_str(), mNumMipMaps );

        /* Get parameters from Image[0] (they are all the same) */
        mHasAlpha = imgs[0].getHasAlpha();
        mSrcWidth = imgs[0].getWidth();
        mSrcHeight = imgs[0].getHeight();
        mSrcBpp = imgs[0].getBPP();
        mFormat = imgs[0].getFormat();

        /* Now that we have the image's parameters, create the D3D surface based on them. */
        createSurface();

        /* Blit to the image. This also creates the mip-maps and applies gamma. */
        blitImage3D( 
            imgs, 
            Image::Rect( 0, 0, mSrcWidth, mSrcHeight ),
            Image::Rect( 0, 0, Texture::mWidth, Texture::mHeight ) );
        
        short bytesPerPixel = mFinalBpp >> 3;
        if( !mHasAlpha && mFinalBpp == 32 )
        {
            bytesPerPixel--;
        }
        mSize = Texture::mWidth * Texture::mHeight * bytesPerPixel;

        mIsLoaded = true;

        OgreUnguard();
	}
    //---------------------------------------------------------------------------------------------
    void D3DTexture::load(void)
    {
        if( mIsLoaded )
            return;

        if( mUsage == TU_DEFAULT )
        {
			if (mTextureType == TEX_TYPE_CUBE_MAP)
			{
				_constructCubeFaceNames(mName);
				Image imgs[6];
				for (int face = 0; face < 6; ++face)
				{
					imgs[face].load(mCubeFaceNames[face]);
				}
				loadImage3D(imgs);

			}
			else
			{
				Image img;
				img.load( Texture::mName );
				
				loadImage( img );
			}
        }
        else if( mUsage == TU_RENDERTARGET )
        {
            mSrcWidth = mSrcHeight = 512;
            mSrcBpp = mFinalBpp;
            createSurface();
        }

        mIsLoaded = true;
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
	void D3DTexture::_chooseD3DFormat(DDPIXELFORMAT &ddpf)
	{
		/* The pixel format is always RGB */
        ddpf.dwFlags = DDPF_RGB;
        ddpf.dwRGBBitCount = mFinalBpp;
        
        if( mHasAlpha )
        {
            ddpf.dwFlags |= DDPF_ALPHAPIXELS;
        }

        /* Compute the bit masks */
        if( mFinalBpp == 16 )
        {
            if( mHasAlpha )
            {
                /* 4-4-4-4 ARGB */
                ddpf.dwRGBAlphaBitMask = 0xf000;
                ddpf.dwRBitMask        = 0x0f00;
                ddpf.dwGBitMask        = 0x00f0;
                ddpf.dwBBitMask        = 0x000f;
            }
            else
            {
                /* 5-6-5 RGB */
                ddpf.dwRBitMask = 0xf800;
                ddpf.dwGBitMask = 0x07e0;
                ddpf.dwBBitMask = 0x001f;
            }
        }
        else
        {
            if( mHasAlpha )
            {
                /* 8-8-8-8 ARGB */
                ddpf.dwRGBAlphaBitMask = 0xff000000;
                ddpf.dwRBitMask        = 0x00ff0000;
                ddpf.dwGBitMask        = 0x0000ff00;
                ddpf.dwBBitMask        = 0x000000ff;
            }
            else
            {
                /* 8-8-8 RGB */
                ddpf.dwRBitMask        = 0xff0000;
                ddpf.dwGBitMask        = 0x00ff00;
                ddpf.dwBBitMask        = 0x0000ff;
            }
        }
	}
	//---------------------------------------------------------------------------------------------
    void D3DTexture::createSurface(void)
    {

		if (mTextureType == TEX_TYPE_CUBE_MAP)
		{
			createSurface3D();
		}
		else
		{
			createSurface2D();
		}
	}
	//---------------------------------------------------------------------------------------------
	void D3DTexture::createSurface2D(void)
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
        ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
        ddsd.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
        ddsd.ddsCaps.dwCaps2 = 0;
        ddsd.dwWidth         = mSrcWidth;
        ddsd.dwHeight        = mSrcHeight;

        /* If the texture is a render target, set the corresponding flags */
        if( mUsage == TU_RENDERTARGET )
        {
            ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE | DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY;
            ddsd.ddsCaps.dwCaps2 = 0;
            mNumMipMaps = 0;
        }
        else
        {
            ddsd.ddsCaps.dwCaps2 = DDSCAPS2_D3DTEXTUREMANAGE;
        }
        
        /* If we want to have mip-maps, set the flags. Note that if the
           texture is the render target type mip-maps are automatically 
           disabled. */
        if( mNumMipMaps )
        {
            ddsd.dwFlags |= DDSD_MIPMAPCOUNT;
            ddsd.dwMipMapCount = mNumMipMaps;

            ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;            
        }

        _chooseD3DFormat(ddsd.ddpfPixelFormat);

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
        mIsLoaded = true;

        LPDIRECTDRAWSURFACE7 pddsRender;
        LPDIRECTDRAW7        pDD;

        /* Get a DirectDraw interface. */
        mD3DDevice->GetRenderTarget( &pddsRender );
        pddsRender->GetDDInterface( (VOID**)&pDD );
        pddsRender->Release();

        if( mUsage == TU_RENDERTARGET )
        {
            /* Get the pixel format of the primary surface - we need it because
               render target surfaces need to have the same pixel format. */
            IDirectDrawSurface7 * pddsPrimarySurface;
            pDD->GetGDISurface( &pddsPrimarySurface );
            pddsPrimarySurface->GetPixelFormat( &( ddsd.ddpfPixelFormat ) );
            pddsPrimarySurface->Release();
        }

        HRESULT hr;

        /* Now create the surface. */
        if( FAILED( hr = pDD->CreateSurface( &ddsd, &mSurface, NULL ) ) )
        {
            pDD->Release();
            Except( hr,
                "Could not create DirectDraw surface.",
                "D3DTexture::createSurfaces" );
        }

        if( mUsage == TU_RENDERTARGET )
        {
            LPDIRECTDRAWSURFACE7 pddsZBuffer, pddsBackBuffer, pddsTexZBuffer;

            DDSCAPS2 ZBuffDDSCaps;
            DDSURFACEDESC2 ZBuffDDSD;

            ZBuffDDSCaps.dwCaps = DDSCAPS_ZBUFFER;
            ZBuffDDSCaps.dwCaps2 = ZBuffDDSCaps.dwCaps3 = ZBuffDDSCaps.dwCaps4 = 0;

            memset( &ZBuffDDSD, 0, sizeof( DDSURFACEDESC2 ) );
            ZBuffDDSD.dwSize = sizeof( DDSURFACEDESC2 );

            /* If the primary surface has an attached z-buffer, we need one for our texture
               too. Here, we get the primary surface's z-buffer format and we create a new
               Z-buffer that we attach to the our texture. */
            if( SUCCEEDED( mD3DDevice->GetRenderTarget( &pddsBackBuffer ) ) )
            {
                if( SUCCEEDED( pddsBackBuffer->GetAttachedSurface( &ZBuffDDSCaps, &pddsZBuffer ) ) )
                {
                    pddsZBuffer->GetSurfaceDesc( &ZBuffDDSD );

                    /* Our new Z-buffer should have the size of the new render target. */
                    ZBuffDDSD.dwWidth = ddsd.dwWidth;
                    ZBuffDDSD.dwHeight = ddsd.dwHeight;

                    hr = pDD->CreateSurface( &ZBuffDDSD, &pddsTexZBuffer, NULL );
                    hr = mSurface->AddAttachedSurface( pddsTexZBuffer );

                    pddsBackBuffer->Release();
                    pddsZBuffer->Release();
                    pddsTexZBuffer->Release();
                }
            }
        }

        /* Release the DirectDraw interface used in the surface creation */
        pDD->Release();
    }
	//---------------------------------------------------------------------------------------------
	void D3DTexture::createSurface3D(void)
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
        ddsd.dwFlags         = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
        ddsd.ddsCaps.dwCaps  = DDSCAPS_COMPLEX | DDSCAPS_TEXTURE;
        ddsd.ddsCaps.dwCaps2 = DDSCAPS2_CUBEMAP|DDSCAPS2_CUBEMAP_ALLFACES;
        ddsd.dwWidth         = mSrcWidth;
        ddsd.dwHeight        = mSrcHeight;

        /* If the texture is a render target, set the corresponding flags */
        if( mUsage == TU_RENDERTARGET )
        {
            ddsd.ddsCaps.dwCaps |= DDSCAPS_3DDEVICE | DDSCAPS_LOCALVIDMEM | DDSCAPS_VIDEOMEMORY;
            mNumMipMaps = 0;
        }
        else
        {
            ddsd.ddsCaps.dwCaps2 |= DDSCAPS2_D3DTEXTUREMANAGE;
        }
        
        /* If we want to have mip-maps, set the flags. Note that if the
           texture is the render target type mip-maps are automatically 
           disabled. */
        if( mNumMipMaps )
        {
            ddsd.dwFlags |= DDSD_MIPMAPCOUNT;
            ddsd.dwMipMapCount = mNumMipMaps;

            ddsd.ddsCaps.dwCaps |= DDSCAPS_MIPMAP;            
        }

        _chooseD3DFormat(ddsd.ddpfPixelFormat);

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
        mIsLoaded = true;


        LPDIRECTDRAWSURFACE7 pddsRender;
        LPDIRECTDRAW7        pDD;

        /* Get a DirectDraw interface. */
        mD3DDevice->GetRenderTarget( &pddsRender );
        pddsRender->GetDDInterface( (VOID**)&pDD );
        pddsRender->Release();

        if( mUsage == TU_RENDERTARGET )
        {
            /* Get the pixel format of the primary surface - we need it because
               render target surfaces need to have the same pixel format. */
            IDirectDrawSurface7 * pddsPrimarySurface;
            pDD->GetGDISurface( &pddsPrimarySurface );
            pddsPrimarySurface->GetPixelFormat( &( ddsd.ddpfPixelFormat ) );
            pddsPrimarySurface->Release();
        }

        HRESULT hr;

        /* Now create the surface. */
        if( FAILED( hr = pDD->CreateSurface( &ddsd, &mSurface, NULL ) ) )
        {
            pDD->Release();
            Except( hr,
                "Could not create DirectDraw surface.",
                "D3DTexture::createSurfaces" );
        }


        if( mUsage == TU_RENDERTARGET )
        {
            LPDIRECTDRAWSURFACE7 pddsZBuffer, pddsBackBuffer, pddsTexZBuffer;

            DDSCAPS2 ZBuffDDSCaps;
            DDSURFACEDESC2 ZBuffDDSD;

            ZBuffDDSCaps.dwCaps = DDSCAPS_ZBUFFER;
            ZBuffDDSCaps.dwCaps2 = ZBuffDDSCaps.dwCaps3 = ZBuffDDSCaps.dwCaps4 = 0;

            memset( &ZBuffDDSD, 0, sizeof( DDSURFACEDESC2 ) );
            ZBuffDDSD.dwSize = sizeof( DDSURFACEDESC2 );

            /* If the primary surface has an attached z-buffer, we need one for our texture
               too. Here, we get the primary surface's z-buffer format and we create a new
               Z-buffer that we attach to the our texture. */
            if( SUCCEEDED( mD3DDevice->GetRenderTarget( &pddsBackBuffer ) ) )
            {
                if( SUCCEEDED( pddsBackBuffer->GetAttachedSurface( &ZBuffDDSCaps, &pddsZBuffer ) ) )
                {
                    pddsZBuffer->GetSurfaceDesc( &ZBuffDDSD );

                    /* Our new Z-buffer should have the size of the new render target. */
                    ZBuffDDSD.dwWidth = ddsd.dwWidth;
                    ZBuffDDSD.dwHeight = ddsd.dwHeight;

                    hr = pDD->CreateSurface( &ZBuffDDSD, &pddsTexZBuffer, NULL );
                    hr = mSurface->AddAttachedSurface( pddsTexZBuffer );

                    pddsBackBuffer->Release();
                    pddsZBuffer->Release();
                    pddsTexZBuffer->Release();
                }
            }
        }

        /* Release the DirectDraw interface used in the surface creation */
        pDD->Release();
	}
    //---------------------------------------------------------------------------------------------
    LPDIRECTDRAWSURFACE7 D3DTexture::getDDSurface(void)
    {
        return mSurface;
    }
	void D3DTexture::_constructCubeFaceNames(const String name)
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
		}

		baseName = fakeName.substr(0, pos);
		ext = fakeName.substr(pos);

		// construct the full 6 faces file names from the baseName, suffixes and extension
		for (int i = 0; i < 6; ++i)
			mCubeFaceNames[i] = baseName + suffixes[i] + ext;
	}


}


#include "OgreD3D9Texture.h"
#include "OgreException.h"
#include "OgreBitwise.h"
#include "OgreImage.h"
#include "OgreLogManager.h"
#include "OgreImageCodec.h"

#include "dxutil.h"
#include "OgreRoot.h"

#include "OgreNoMemoryMacros.h"
#include <d3dx9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include "OgreMemoryMacros.h"

namespace Ogre 
{
	D3D9Texture::D3D9Texture( String name, LPDIRECT3DDEVICE9 pD3DDevice, TextureUsage usage )
	{
        mName = name;

		mpD3DDevice = pD3DDevice;
		if( !mpD3DDevice )
			Except( 999, "Invalid Direct3D9 Device passed in", "D3D9Texture::D3D9Texture" );
		mpD3DDevice->AddRef();
        HRESULT hr;
		if (FAILED(hr = mpD3DDevice->GetDeviceCaps(&mCaps)))
		{
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Failed GetDeviceCaps : " + msg, "D3D9Texture::D3D9Texture" );
		}
		enable32Bit( false );

		mUsage = usage;
		if( mUsage == TU_RENDERTARGET )
        {
            mSrcWidth = mSrcHeight = 512;
            mSrcBpp = mFinalBpp;
		}

		mpTexture = NULL;
		mpTempTexture = NULL;       
        mpRenderZBuffer = NULL;
		mIsLoaded = false;
	}

	D3D9Texture::D3D9Texture( String name, IDirect3DDevice9 * device, uint width, uint height, uint num_mips, PixelFormat format, TextureUsage usage )
	{
		mName = name;

		mpD3DDevice = device;
		if( !mpD3DDevice )
			Except( 999, "Invalid Direct3D9 Device passed in", "D3D9Texture::D3D9Texture" );
		mpD3DDevice->AddRef();
        HRESULT hr;
        if (FAILED(hr = mpD3DDevice->GetDeviceCaps(&mCaps)))
		{
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Failed GetDeviceCaps : " + msg, "D3D9Texture::D3D9Texture" );
		}
		enable32Bit( false );

		mpTexture = NULL;
		mpTempTexture = NULL;       
        mpRenderZBuffer = NULL;

		mSrcWidth = width;
		mSrcHeight = height;
		mNumMipMaps = num_mips;

		mUsage = usage;
		mFormat = format;
		mSrcBpp = mFinalBpp = Image::getNumElemBits( mFormat );

		if( mUsage == TU_RENDERTARGET )
        {
            mSrcWidth = mSrcHeight = 512;
            mSrcBpp = mFinalBpp;
		}

		createTexture();
		mIsLoaded = true;
	}

	D3D9Texture::~D3D9Texture()
	{
		if( mIsLoaded )
			unload();

		SAFE_RELEASE( mpD3DDevice );
        SAFE_RELEASE( mpTempTexture );
        SAFE_RELEASE( mpTexture );
        SAFE_RELEASE( mpRenderZBuffer );
	}

	void D3D9Texture::load()
	{
		if( mIsLoaded )
			return;

        if( mUsage == TU_RENDERTARGET )
        {
            mSrcWidth = mSrcHeight = 512;
            mSrcBpp = mFinalBpp;
            createTexture();
        }
        else
        {
		    Image img;
            img.load( Texture::mName );
		    loadImage( img );
        }

		mIsLoaded = true;
	}

	void D3D9Texture::loadImage( const Image& img )
	{
		if( mIsLoaded )
			unload();

		LogManager::getSingleton().logMessage( LML_TRIVIAL, "D3D9Texture: Loading %s iwth %d mipmaps from Image.",
            Texture::mName.c_str(), mNumMipMaps );

		PixelFormat pf = img.getFormat();
		mSrcBpp = Image::PF2BPP( pf );
        mHasAlpha = img.getHasAlpha();

		mSrcWidth = img.getWidth();
		mSrcHeight = img.getHeight();

		uchar *pTempData = new uchar[ img.getSize() ];
		memcpy( pTempData, img.getData(), img.getSize() );

		createTexture();
        Image::applyGamma( pTempData, mGamma, uint( img.getSize() ), mSrcBpp );
		copyMemoryToTexture( pTempData );

		SAFE_DELETE_ARRAY( pTempData );
	}

	void D3D9Texture::copyToTexture( Texture * target )
	{
        if( target->getUsage() != mUsage )
            return;

        HRESULT hr;
        D3D9Texture *other;

		other = reinterpret_cast< D3D9Texture * >( target );
		RECT dstRC = {0, 0, other->getWidth(), other->getHeight()};
        LPDIRECT3DSURFACE9 src, dst;
		
		if( FAILED( hr = mpTexture->GetSurfaceLevel(0, &src) ) )
        {
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
        }

		if( FAILED( hr = other->getD3DTexture()->GetSurfaceLevel(0, &dst) ) )
        {
			SAFE_RELEASE(src);
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
        }

		if( FAILED( hr = mpD3DDevice->StretchRect( src, NULL, dst, &dstRC, D3DTEXF_NONE) ) )
        {
			SAFE_RELEASE(src);
			SAFE_RELEASE(dst);
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Couldn't blit : " + msg, "D3D9Texture::copyToTexture" );
        }

		SAFE_RELEASE(src);
		SAFE_RELEASE(dst);
	}

	void D3D9Texture::unload()
	{
		SAFE_RELEASE( mpTexture );
		SAFE_RELEASE( mpTempTexture );
		SAFE_RELEASE( mpRenderZBuffer);
	}

	void D3D9Texture::blitImage(const Image& src, const Image::Rect imgRect, const Image::Rect texRect )
	{
		Except( 999, "**** Blit image called but not implemented!!! ****", "D3D9Texture" );
	}

    void D3D9Texture::blitToTexture( const Image &src, unsigned uStartX, unsigned uStartY )
	{
		Except( 999, "**** Blit to texture called but not implemented!!! ****", "D3D9Texture" );
	}

	void D3D9Texture::createTexture()
	{
		D3DFORMAT format = D3DFMT_A8R8G8B8;
		if( mFinalBpp > 16 && mHasAlpha )
			format = D3DFMT_A8R8G8B8;
		else if( mFinalBpp > 16 && !mHasAlpha )
			format = D3DFMT_R8G8B8;
		else if( mFinalBpp == 16 && mHasAlpha )
			format = D3DFMT_A4R4G4B4;
		else if( mFinalBpp == 16 && !mHasAlpha )
			format = D3DFMT_R5G6B5;

		HRESULT hr;
		// Use D3DX to help us create the texture, this way it can adjust any relevant sizes
		if( FAILED( hr = D3DXCreateTexture( 
			mpD3DDevice, 
			mSrcWidth, 
			mSrcHeight, 
			(mNumMipMaps ? mNumMipMaps : 1), 
			0, 
			format, 
			D3DPOOL_SYSTEMMEM, 
			&mpTempTexture ) ) )
		{
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Error creating temp Direct3D texture D3DXCreateTexture : " + msg, "D3DXTexture::createTexture" );
		}

        if( mUsage == TU_DEFAULT )
        {
			if( FAILED( hr = D3DXCreateTexture( 
                mpD3DDevice, 
                mSrcWidth, 
                mSrcHeight, 
                (mNumMipMaps ? mNumMipMaps : 1), 
                0, 
				format, 
				D3DPOOL_DEFAULT, 
				&mpTexture ) ) )
		    {
				String msg = DXGetErrorDescription9(hr);
				Except( hr, "Error creating Direct3D texture D3DXCreateTexture : " + msg, "D3DXTexture::createTexture" );
		    }
        }
        else
        {
            /* We need to set the pixel format of the render target texture to be the same as the
			   one of the front buffer. */
			IDirect3DSurface9 * tempSurf;
			D3DSURFACE_DESC tempDesc;

			hr = mpD3DDevice->GetRenderTarget(0, &tempSurf );
            if( FAILED( hr ) )
		    {
				String msg = DXGetErrorDescription9(hr);
				Except( hr, "Error GetRenderTarget : " + msg, "D3D9Texture::createTexture" );
		    }

			hr = tempSurf->GetDesc( & tempDesc );
            if( FAILED( hr ) )
		    {
				SAFE_RELEASE(tempSurf);
				String msg = DXGetErrorDescription9(hr);
				Except( hr, "Error GetDesc : " + msg, "D3D9Texture::createTexture" );
		    }
			format = tempDesc.Format;
			SAFE_RELEASE(tempSurf);

			/** Now we create the texture. */
            if( FAILED( hr = D3DXCreateTexture( mpD3DDevice, mSrcWidth, mSrcHeight, 1, 
                D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &mpTexture ) ) )
		    {
				String msg = DXGetErrorDescription9(hr);
				Except( hr, "Error, D3DXCreateTexture : " + msg, "D3D9Texture::createTexture" );
		    }

			/* Now get the format of the depth stencil surface. */
			hr = mpD3DDevice->GetDepthStencilSurface( & tempSurf );
            if( FAILED( hr ) )
		    {
				String msg = DXGetErrorDescription9(hr);
				Except( hr, "Error GetDepthStencilSurface : " + msg, "D3D9Texture::createTexture" );
		    }

			hr = tempSurf->GetDesc( & tempDesc );
            if( FAILED( hr ) )
		    {
				SAFE_RELEASE(tempSurf);
				String msg = DXGetErrorDescription9(hr);
				Except( hr, "Error GetDesc : " + msg, "D3D9Texture::createTexture" );
		    }
			SAFE_RELEASE(tempSurf);

			/** We also create a depth buffer for our render target. */
            if( FAILED( hr = mpD3DDevice->CreateDepthStencilSurface( 
				mSrcWidth, 
				mSrcHeight, 
				tempDesc.Format, 
				tempDesc.MultiSampleType, 
				NULL, 
				FALSE, 
				&mpRenderZBuffer, 
				NULL ) ) )
		    {
				String msg = DXGetErrorDescription9(hr);
				Except( hr, "Error CreateDepthStencilSurface : " + msg, "D3D9Texture::createTexture" );
		    }
        }

		// Check the actual dimensions vs requested
		D3DSURFACE_DESC desc;
		if( FAILED( hr = mpTexture->GetLevelDesc( 0, &desc ) ) )
		{
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Error GetLevelDesc : " + msg, "D3D9Texture::createTexture" );
		}

		if( desc.Width != mSrcWidth || desc.Height != mSrcHeight )
		{
			char msg[255];
            sprintf(msg, "Surface dimensions for requested texture %s have been altered by "
                "the renderer.", Texture::mName.c_str());
            LogManager::getSingleton().logMessage(msg);

            sprintf(msg,"   Requested: %dx%d Actual: %dx%d",
				mSrcWidth, mSrcHeight, desc.Width, desc.Height);
            LogManager::getSingleton().logMessage(msg);

            LogManager::getSingleton().logMessage("   Likely cause is that requested dimensions are not a power of 2, "
                "or device requires square textures.");
		}

		mWidth = desc.Width;
        mHeight = desc.Height;
	}

	void D3D9Texture::getColourMasks( D3DFORMAT format, DWORD* pdwRed, DWORD* pdwGreen, DWORD* pdwBlue, DWORD* pdwAlpha, DWORD* pdwRGBBitCount )
	{
		switch( format )
		{
		case D3DFMT_R8G8B8:
			*pdwRed = 0x00FF0000; *pdwGreen = 0x0000FF00; *pdwBlue = 0x000000FF; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 24;
			break;
		case D3DFMT_A8R8G8B8:
			*pdwRed = 0x00FF0000; *pdwGreen = 0x0000FF00; *pdwBlue = 0x000000FF; *pdwAlpha = 0xFF000000;
			*pdwRGBBitCount = 32;
			break;
		case D3DFMT_X8R8G8B8:
			*pdwRed = 0x00FF0000; *pdwGreen = 0x0000FF00; *pdwBlue = 0x000000FF; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 32;
			break;
		case D3DFMT_R5G6B5:
			*pdwRed = 0x0000F800; *pdwGreen = 0x000007E0; *pdwBlue = 0x0000001F; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_X1R5G5B5:
			*pdwRed = 0x00007C00; *pdwGreen = 0x000003E0; *pdwBlue = 0x0000001F; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_A1R5G5B5:
			*pdwRed = 0x00007C00; *pdwGreen = 0x000003E0; *pdwBlue = 0x0000001F; *pdwAlpha = 0x00008000;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_A4R4G4B4:
			*pdwRed = 0x00000F00; *pdwGreen = 0x000000F0; *pdwBlue = 0x0000000F; *pdwAlpha = 0x0000F000;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_A8:
			*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x000000FF;
			*pdwRGBBitCount = 8;
			break;
		case D3DFMT_R3G3B2:
			*pdwRed = 0x000000E0; *pdwGreen = 0x0000001C; *pdwBlue = 0x00000003; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 8;
			break;
		case D3DFMT_A8R3G3B2:
			*pdwRed = 0x000000E0; *pdwGreen = 0x0000001C; *pdwBlue = 0x00000003; *pdwAlpha = 0x0000FF00;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_X4R4G4B4:
			*pdwRed = 0x00000F00; *pdwGreen = 0x000000F0; *pdwBlue = 0x0000000F; *pdwAlpha = 0x0000F000;
			*pdwRGBBitCount = 16;
			break;
		case D3DFMT_A2B10G10R10:
			*pdwRed = 0x3FF00000; *pdwGreen = 0x000FFC00; *pdwBlue = 0x000003FF; *pdwAlpha = 0xC0000000;
			*pdwRGBBitCount = 32;
			break;
		case D3DFMT_G16R16:
			*pdwRed = 0xFFFF0000; *pdwGreen = 0x0000FFFF; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 32;
			break;
		// Not implemented
		//case D3DFMT_A8P8:
		//	*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
		//	*pdwRGBBitCount = 8;
		//	break;
		//case D3DFMT_P8:
		//	*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
		//	*pdwRGBBitCount = 8;
		//	break;
		//case D3DFMT_L8:
		//	*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
		//	*pdwRGBBitCount = 8;
		//	break;
		//case D3DFMT_A8L8:
		//	*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
		//	*pdwRGBBitCount = 16;
		//	break;
		//case D3DFMT_A4L4 :
		//	*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
		//	*pdwRGBBitCount = 8;
		//	break;

		default:
			*pdwRed = 0x00000000; *pdwGreen = 0x00000000; *pdwBlue = 0x00000000; *pdwAlpha = 0x00000000;
			*pdwRGBBitCount = 0;
			break;
		}
	}

	void D3D9Texture::copyMemoryToTexture( unsigned char* pBuffer )
	{
		HRESULT hr;
		// Create a tempoary texture at the same format as final texture
		D3DSURFACE_DESC desc;
		if( FAILED( hr = mpTempTexture->GetLevelDesc( 0, &desc ) ) )
		{
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Failed to get texture level 0 descripition GetLevelDesc : " + msg, "D3D9Texture::copyMemoryToTexture" );
		}

		// Create a temporary surface for the texture
		LPDIRECT3DSURFACE9 pTempSurface;
		if( FAILED( hr = mpD3DDevice->CreateOffscreenPlainSurface( 
						mSrcWidth, 
						mSrcHeight, 
						desc.Format, 
						D3DPOOL_SCRATCH, 
						&pTempSurface, 
						NULL ) ) )
		{
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Failed to create tempoarary surface CreateOffscreenPlainSurface : " + msg, "D3D9Texture::copyMemoryToTexture" );
		}

		// Copy the image from the buffer to the temporary surface.
		// We have to do our own colour conversion here since we don't 
		// have a DC to do it for us

		// NOTE - only non-palettised surfaces supported for now
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
		pTempSurface->GetDesc( &desc );
		DWORD aMask, rMask, gMask, bMask, rgbBitCount;
		getColourMasks( desc.Format, &rMask, &gMask, &bMask, &aMask, &rgbBitCount );

		D3DLOCKED_RECT rect;
		if( FAILED( hr = pTempSurface->LockRect( &rect, NULL, D3DLOCK_NOSYSLOCK ) ) )
		{
			SAFE_RELEASE(pTempSurface);
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Unable to lock temp texture surface : " + msg, "D3D9Texture::copyMemoryToTexture" );
		}
		else
			pBuf8 = (BYTE*)pBuffer;

		for( iRow = 0; iRow < mSrcHeight; iRow++ )
		{
			// NOTE: Direct3D used texture coordinates where (0,0) is the TOP LEFT corner of texture
			// Everybody else (OpenGL, 3D Studio, etc) uses (0,0) as BOTTOM LEFT corner
			// So whilst we load, flip the texture in the Y-axis to compensate
			pSurf8 = (BYTE*)rect.pBits + ((mSrcHeight-iRow-1) * rect.Pitch);

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

		pTempSurface->UnlockRect();

		// Now we need to copy the surface to the texture level 0 surface
		LPDIRECT3DSURFACE9 pDestSurface;
		if( FAILED( hr = mpTempTexture->GetSurfaceLevel( 0, &pDestSurface ) ) )
		{
			SAFE_RELEASE(pTempSurface);
			SAFE_RELEASE(pDestSurface);
			SAFE_RELEASE( mpTempTexture );
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Failed to get level 0 surface for texture : " + msg, "D3D9Texture::copyMemoryToTexture" );
		}

		if( FAILED( hr = D3DXLoadSurfaceFromSurface( pDestSurface, NULL, NULL, pTempSurface, NULL, NULL, D3DX_DEFAULT, 0 ) ) )
		{
			SAFE_RELEASE(pTempSurface);
			SAFE_RELEASE(pDestSurface);
			SAFE_RELEASE( mpTempTexture );
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Failed to copy temp surface to texture : " + msg, "D3D9Texture::copyMemoryToTexture" );
		}

		if( FAILED( hr = D3DXFilterTexture( mpTempTexture, NULL, D3DX_DEFAULT, D3DX_DEFAULT ) ) )
		{
			SAFE_RELEASE(pTempSurface);
			SAFE_RELEASE(pDestSurface);
			SAFE_RELEASE( mpTempTexture );
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Failed to filter temporary texture (generate mip maps) : " + msg, "D3D9Texture::copyMemoryToTexture" );
		}

		if( FAILED( hr = mpD3DDevice->UpdateTexture(mpTempTexture, mpTexture) ) )
		{
			SAFE_RELEASE(pTempSurface);
			SAFE_RELEASE(pDestSurface);
			SAFE_RELEASE( mpTempTexture );
			String msg = DXGetErrorDescription9(hr);
			Except( hr, "Failed to stretch texture : " + msg, "D3D9Texture::copyMemoryToTexture" );
		}
		
		SAFE_RELEASE( pTempSurface );
		SAFE_RELEASE( pDestSurface );
		SAFE_RELEASE( mpTempTexture );
	}

    void D3D9Texture::getCustomAttribute( String name, void* pData )
    {
        // Valid attributes and their equivalent native functions:
        // D3DDEVICE            : getD3DDeviceDriver
        // DDBACKBUFFER         : getDDBackBuffer
        // DDFRONTBUFFER        : getDDFrontBuffer
        // HWND                 : getWindowHandle

        if( name == "D3DDEVICE" )
        {
            LPDIRECT3DDEVICE9 *pDev = (LPDIRECT3DDEVICE9*)pData;
            *pDev = mpD3DDevice;
            return;
        }
        else if( name == "DDBACKBUFFER" )
        {
            LPDIRECT3DSURFACE9 *pSurf = (LPDIRECT3DSURFACE9*)pData;
            mpTexture->GetSurfaceLevel( 0, &(*pSurf) );
            (*pSurf)->Release();
            return;
        }
        else if( name == "D3DZBUFFER" )
        {
            LPDIRECT3DSURFACE9 *pSurf = (LPDIRECT3DSURFACE9*)pData;
            *pSurf = mpRenderZBuffer;
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
}
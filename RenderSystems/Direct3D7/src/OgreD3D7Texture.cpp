/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
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

    D3DTexture::D3DTexture(String name, LPDIRECT3DDEVICE7 lpDirect3dDevice)
    {
        mName = name;
        mD3DDevice = lpDirect3dDevice;
        // Default to 16-bit texture
        enable32Bit(false);
    }

    D3DTexture::~D3DTexture()
    {
        if( mIsLoaded )
            unload();
    }

    void D3DTexture::blitToTexture( 
        const Image &src, unsigned uStartX, unsigned uStartY )
    {
        // I have no FREAKING idea how this is done in DX7 and besides, I don't
        // have the API docs installed. Sorry, guys :(
    }

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

        Image::PixelFormat pf = img.getFormat();

        mSrcBpp = Image::PF2BPP(pf);

        if( pf & Image::FMT_ALPHA )
        {
            mHasAlpha = true;
        }
        else
        {
            mHasAlpha = false;
        }

        mSrcWidth = img.getWidth();
        mSrcHeight = img.getHeight();

        uchar *pTempData = new uchar[ img.getSize() ];
        memcpy( pTempData, img.getConstData(), img.getSize() );

        createSurfaces();
        applyGamma( pTempData, img.getSize(), mSrcBpp );
        copyMemoryToSurface( pTempData );
        generateMipMaps();

        delete [] pTempData;

        // Update size (the final size, not including temp space)
        short bytesPerPixel = mFinalBpp >> 3;
        if( !mHasAlpha && mFinalBpp == 32 )
        {
            bytesPerPixel--;
        }
        mSize = mWidth * mHeight * bytesPerPixel;        

        mIsLoaded = true;
    }    
    
    // -------------------------------------------------------------------
    void D3DTexture::load(void)
    {
        Image img;
        img.load( mName );

        loadImage( img );
    }

    void D3DTexture::unload(void)
    {
        if( mIsLoaded )
        {
            releaseSurfaces();
            mIsLoaded = false;
        }
    }

    // -------------------------------------------------------------------
    void D3DTexture::loadFromBMP(void)
    {
        HBITMAP hBitmap;

        hBitmap = (HBITMAP)LoadImage( NULL, mName.c_str(),
                                IMAGE_BITMAP, 0, 0,
                                LR_LOADFROMFILE|LR_CREATEDIBSECTION );

        if (!hBitmap)
            throw Exception(999, "Unable to load texture from BMP.",
                "D3DTexture::loadFromBMP");

        // Get the bitmap structure (to extract width, height, and bpp)
        BITMAP bm;
        GetObject( hBitmap, sizeof(BITMAP), &bm );
        mSrcWidth  = bm.bmWidth;
        mSrcHeight = bm.bmHeight;
        mHasAlpha = false;            // No embedded transparency
        mSrcBpp = bm.bmBitsPixel*3;  // This is per-plane

        // Create DD surfaces
        createSurfaces();

        // Blt bitmap to surface
        copyBitmapToSurface(hBitmap);


    }

    // -------------------------------------------------------------------
    void D3DTexture::createSurfaces(void)
    {
        // Create new texture using D3DX
        // Determine requested surface format
        // Note assumption is that partial transparency is
        //  always required is there is some alpha i.e.
        //  even if 1-bit alpha is available it is not used
        D3DX_SURFACEFORMAT d3dxSurf;
        // 32-bit with alpha
        if (mFinalBpp > 16 && mHasAlpha)
            d3dxSurf = D3DX_SF_A8R8G8B8;
        // 32-bit, no alpha
        else if (mFinalBpp > 16 && !mHasAlpha)
            d3dxSurf = D3DX_SF_R8G8B8;
        // 16-bit, with alpha
        else if (mFinalBpp == 16 && mHasAlpha)
            d3dxSurf = D3DX_SF_A4R4G4B4;
        // 16-bit, no alpha
        else if (mFinalBpp == 16 && !mHasAlpha)
            d3dxSurf = D3DX_SF_R5G6B5;

        // Set up copies of core information
        // D3DX required pointers to these, & I want to know if they change
        unsigned long d3dxMipMaps, d3dxHeight, d3dxWidth;
        d3dxMipMaps = mNumMipMaps;
        d3dxHeight = mSrcHeight;
        d3dxWidth = mSrcWidth;
        unsigned long flags = 0;

        if (getNumMipMaps() == 0)
            flags |= D3DX_TEXTURE_NOMIPMAP;

        HRESULT hr = D3DXCreateTexture(
            mD3DDevice, &flags, &d3dxWidth, &d3dxHeight,
            &d3dxSurf, 0, &mSurface, &d3dxMipMaps);

        if (FAILED(hr))
            throw new Exception(hr, "Error creating Direct3D texture", "D3DTexture::createSurfaces");

        // If actual dimensions differ, log
        if (d3dxWidth != mSrcWidth || d3dxHeight != mSrcHeight)
        {
            char msg[255];

            sprintf(msg, "Surface dimensions for requested texture %s have been altered by "
                "the renderer.", mName.c_str());
            LogManager::getSingleton().logMessage(msg);

            sprintf(msg,"   Requested: %dx%d Actual: %dx%d",
                mSrcWidth, mSrcHeight, d3dxWidth, d3dxHeight);
            LogManager::getSingleton().logMessage(msg);

            LogManager::getSingleton().logMessage("   Likely cause is that requested dimensions are not a power of 2, "
                "or device requires square textures.");

        }
        // Record actual width/height
        mWidth = d3dxWidth;
        mHeight = d3dxHeight;
    }
    // -------------------------------------------------------------------
    void D3DTexture::releaseSurfaces(void)
    {
        mSurface->Release();
        mSurface = 0;
    }


    // -------------------------------------------------------------------
    void D3DTexture::copyBitmapToSurface(HBITMAP hBitmap)
    {
        // Get a DDraw object to create a temporary surface
        LPDIRECTDRAW7 pDD;
        mSurface->GetDDInterface( (VOID**)&pDD );
        pDD->Release();

        // Setup the new surface desc
        // Start same size as source
        DDSURFACEDESC2 ddsd;
        D3DUtil_InitSurfaceDesc( ddsd );
        mSurface->GetSurfaceDesc( &ddsd );
        ddsd.dwFlags          = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|
                                DDSD_TEXTURESTAGE;
        ddsd.ddsCaps.dwCaps   = DDSCAPS_TEXTURE|DDSCAPS_SYSTEMMEMORY;
        ddsd.ddsCaps.dwCaps2  = 0L;
        ddsd.dwWidth          = mSrcWidth;
        ddsd.dwHeight         = mSrcHeight;

        // Create a new surface for the texture
        LPDIRECTDRAWSURFACE7 pddsTempSurface;
        HRESULT hr;
        if( FAILED( hr = pDD->CreateSurface( &ddsd, &pddsTempSurface, NULL ) ) )
            throw Exception(999, "Internal error creating surface for bitmap copy", "D3DTexture::copyBitmapToSurface");

        // Get a DC for the bitmap
        HDC hdcBitmap = CreateCompatibleDC( NULL );
        if( NULL == hdcBitmap )
        {
            pddsTempSurface->Release();
            throw Exception(999, "Internal error creating DC for bitmap copy", "D3DTexture::copyBitmapToSurface");
        }
        SelectObject( hdcBitmap, hBitmap );

        // Copy the bitmap image to the temporary surface.
        HDC hdcSurface;
        if( SUCCEEDED( pddsTempSurface->GetDC( &hdcSurface ) ) )
        {
            BitBlt( hdcSurface, 0, 0, mSrcWidth, mSrcHeight, hdcBitmap, 0, 0,
                    SRCCOPY );
            pddsTempSurface->ReleaseDC( hdcSurface );
        }
        DeleteDC( hdcBitmap );

        // Copy the temp surface to the real texture surface (first mipmap level if mipmap)
        // At this point, blit from source size to dest size happens, stretching if required
        mSurface->Blt( NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL );

        pddsTempSurface->Release();


    }


    // -------------------------------------------------------------------
    void D3DTexture::copyMemoryToSurface(unsigned char* pBuffer)
    {
        // Get a DDraw object to create a temporary surface
        LPDIRECTDRAW7 pDD;
        mSurface->GetDDInterface( (VOID**)&pDD );
        pDD->Release();

        // Setup the new surface desc
        DDSURFACEDESC2 ddsd;
        D3DUtil_InitSurfaceDesc( ddsd );
        // Create temp surface at same format as final texture
        // Make same size a source for now, stretching will happen later
        mSurface->GetSurfaceDesc(&ddsd);
        ddsd.dwFlags          = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT|
                                DDSD_TEXTURESTAGE;
        ddsd.ddsCaps.dwCaps   = DDSCAPS_TEXTURE|DDSCAPS_SYSTEMMEMORY;
        ddsd.ddsCaps.dwCaps2  = 0L;
        // Make source size
        ddsd.dwWidth = mSrcWidth;
        ddsd.dwHeight = mSrcHeight;



        // Create a new temporary surface for the texture
        // This is because we may not be able to lock a
        // final texture (video memory) surface e.g. Voodoo
        LPDIRECTDRAWSURFACE7 pddsTempSurface;
        HRESULT hr;
        if( FAILED( hr = pDD->CreateSurface( &ddsd, &pddsTempSurface, NULL ) ) )
            throw Exception(hr, "Error creating temp surface", "Texture module");

        // Copy the image from the buffer to the temporary surface.
        // We have to do our own colour conversion here since we don't
        // have a DC to do it for us (could use D3DX, but it's not that hard)

        // Note - only non-palettised surfaces supported for now

        BYTE *pSurf8;
        BYTE *pBuf8;
        DWORD data32;
        DWORD out32;
        DWORD temp32;
        DWORD srcPattern;

        unsigned iRow, iCol;

        // Note - dimensions of surface may differ from buffer
        // dimensions (e.g. power of 2 or square adjustments)

        // Lock surface
        hr = pddsTempSurface->Lock(NULL, &ddsd, 0, NULL);
        if (FAILED(hr))
            throw Exception(hr, "Unable to lock temp texture surface", "D3DTexture::copyMemoryToSurface");

        else
            pBuf8 = (BYTE*)pBuffer;

        for( iRow = 0; iRow < mSrcHeight; iRow++ )
        {
            // NOTE: Direct3D used texture coordinates where (0,0) is the TOP LEFT corner of texture
            // Everybody else (OpenGL, 3D Studio etc) uses (0,0) as the BOTTOM LEFT corner
            // So whilst we load, flip the texture in the Y-axis to compensate
            pSurf8 = (BYTE*)ddsd.lpSurface + ((mSrcHeight-iRow-1) * ddsd.lPitch);

            for (iCol = 0; iCol < mSrcWidth; iCol++)
            {
                // Read RGBA values from buffer
                data32 = 0;
                if (mSrcBpp >= 24)
                {
                    // Data in buffer is in RGB(A) format
                    // Read into a 32-bit structure
                    // Use bytes for 24-bit compatibility
                    // Note - buffer is big-endian
                    data32 |= *pBuf8++ << 24;
                    data32 |= *pBuf8++ << 16;
                    data32 |= *pBuf8++ << 8;
                }
                else if (mSrcBpp == 8) // Greyscale, not palettised (palettised not supported)
                {
                    // Duplicate same greyscale value across R,G,B
                    data32 |= *pBuf8 << 24;
                    data32 |= *pBuf8 << 16;
                    data32 |= *pBuf8++ << 8;
                }

                if (mHasAlpha)
                    data32 |= *pBuf8++;
                else
                    data32 |= 0xFF; // Set opaque


                // Write RGBA values to surface
                // Data in surface can be in varying formats

                // Use bit conversion function
                // Note we use a 32-bit value to manipulate
                // Will be reduced to size later
                out32 = 0;

                // Red
                srcPattern = 0xFF000000;
                Bitwise::convertBitPattern(&data32, &srcPattern, 32,
                    &temp32, &(ddsd.ddpfPixelFormat.dwRBitMask), 32);
                out32 |= temp32;

                // Green
                srcPattern = 0x00FF0000;
                Bitwise::convertBitPattern(&data32, &srcPattern, 32,
                    &temp32, &(ddsd.ddpfPixelFormat.dwGBitMask), 32);
                out32 |= temp32;

                // Blue
                srcPattern = 0x0000FF00;
                Bitwise::convertBitPattern(&data32, &srcPattern, 32,
                    &temp32, &(ddsd.ddpfPixelFormat.dwBBitMask), 32);
                out32 |= temp32;

                // Alpha
                if (ddsd.ddpfPixelFormat.dwRGBAlphaBitMask > 0)
                {
                    srcPattern = 0x000000FF;
                    Bitwise::convertBitPattern(&data32, &srcPattern, 32,
                        &temp32, &(ddsd.ddpfPixelFormat.dwRGBAlphaBitMask), 32);
                    out32 |= temp32;
                }


                // Asign results to surface pixel
                // Write up to 4 bytes
                // Surfaces are little-endian (low byte first)
                if (ddsd.ddpfPixelFormat.dwRGBBitCount >= 8)
                    *pSurf8++ = (BYTE)out32;
                if (ddsd.ddpfPixelFormat.dwRGBBitCount >= 16)
                    *pSurf8++ = (BYTE)(out32 >> 8);
                if (ddsd.ddpfPixelFormat.dwRGBBitCount >= 24)
                    *pSurf8++ = (BYTE)(out32 >> 16);
                if (ddsd.ddpfPixelFormat.dwRGBBitCount == 32)
                    *pSurf8++ = (BYTE)(out32 >> 24);


            }

        }

        pddsTempSurface->Unlock(NULL);


        // Copy the temp surface to the real texture surface (first mipmap level if mipmap)
        // This will stretch the texture if required (if dest is larger because of powers of 2)
        hr = mSurface->Blt( NULL, pddsTempSurface, NULL, DDBLT_WAIT, NULL );
        if (FAILED(hr))
            throw Exception(hr, "Can't blit from temp texture to final texture", "D3DTexture::copyMemoryToSurface");


        pddsTempSurface->Release();

    }


    // -------------------------------------------------------------------
    void D3DTexture::generateMipMaps(void)
    {
        // Takes a surface with the mip map surfaces attached
        // and writes filtered minified versions of the texture
        // in the primary surface to the mipmaps.

        int iMipLvl;
        LPDIRECTDRAWSURFACE7 lpddsSource;
        LPDIRECTDRAWSURFACE7 lpddsDest;
        DDSCAPS2 ddsCaps;
        DDSURFACEDESC2 ddsd;
        DDSURFACEDESC2 ddsdSource, ddsdDest;
        DWORD totR;
        DWORD totG;
        DWORD totB;
        DWORD totA;
        DWORD avgR;
        DWORD avgG;
        DWORD avgB;
        DWORD avgA;
        HRESULT hr;

        // Get surface desc
        D3DUtil_InitSurfaceDesc( ddsd );
        hr = mSurface->GetSurfaceDesc(&ddsd);
        if (FAILED(hr))
            throw Exception(hr, "Cannot get surface desc of first mipmap level.",
                "GenerateMipMaps");


        int rBitShift = Bitwise::getBitShift(ddsd.ddpfPixelFormat.dwRBitMask);
        int gBitShift = Bitwise::getBitShift(ddsd.ddpfPixelFormat.dwGBitMask);
        int bBitShift = Bitwise::getBitShift(ddsd.ddpfPixelFormat.dwBBitMask);
        int aBitShift = Bitwise::getBitShift(ddsd.ddpfPixelFormat.dwRGBAlphaBitMask);

        ZeroMemory(&ddsCaps, sizeof(DDSCAPS2));
        ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

        // Start at first level, keep going until we run out of mipmaps
        lpddsSource = mSurface;
        for(iMipLvl = 0; ; iMipLvl++)
        {
            // Get next mipmap level
            hr = lpddsSource->GetAttachedSurface(&ddsCaps, &lpddsDest);
            if (hr == DDERR_NOTFOUND) break;

            lpddsDest->Release();
            if (FAILED(hr))
                throw Exception(hr, "Cannot get mipmap surface.",
                    "GenerateMipMaps");

            // Lock source & destination
            D3DUtil_InitSurfaceDesc( ddsdSource );
            D3DUtil_InitSurfaceDesc( ddsdDest );
            hr = lpddsSource->Lock( NULL, &ddsdSource, DDLOCK_WAIT, NULL );
            if (FAILED(hr))
                throw Exception(hr, "Cannot lock source surface in mipmap creation.",
                    "GenerateMipMaps");
            hr = lpddsDest->Lock( NULL, &ddsdDest, DDLOCK_WAIT, NULL );
            if (FAILED(hr))
                throw Exception(hr, "Cannot lock destination surface in mipmap creation.",
                    "GenerateMipMaps");


            // Loop around destination
            // Read pixels from source in groups of 4
            for (DWORD y = 0; y < ddsdDest.dwHeight; y++)
            {
                WORD*  psrc16 =  (WORD*)((BYTE*)ddsdSource.lpSurface + (y*2)*ddsdSource.lPitch);
                DWORD* psrc32 = (DWORD*)((BYTE*)ddsdSource.lpSurface + (y*2)*ddsdSource.lPitch);
                WORD*  pdest16 =  (WORD*)((BYTE*)ddsdDest.lpSurface + y*ddsdDest.lPitch);
                DWORD* pdest32 = (DWORD*)((BYTE*)ddsdDest.lpSurface + y*ddsdDest.lPitch);

                for (DWORD x = 0; x < ddsdDest.dwWidth; x++)
                {
                    // Get 4 pixels from source
                    // Reuse masks from previous alpha work
                    totR = 0;
                    totG = 0;
                    totB = 0;
                    totA = 0;
                    if (ddsdSource.ddpfPixelFormat.dwRGBBitCount == 16)
                    {
                        totR += (*psrc16 & ddsdSource.ddpfPixelFormat.dwRBitMask) >> rBitShift;
                        totG += (*psrc16 & ddsdSource.ddpfPixelFormat.dwGBitMask) >> gBitShift;
                        totB += (*psrc16 & ddsdSource.ddpfPixelFormat.dwBBitMask) >> bBitShift;
                        totA += (*psrc16 & ddsdSource.ddpfPixelFormat.dwRGBAlphaBitMask) >> aBitShift;
                        // one to the right
                        psrc16++;
                        totR += (*psrc16 & ddsdSource.ddpfPixelFormat.dwRBitMask) >> rBitShift;
                        totG += (*psrc16 & ddsdSource.ddpfPixelFormat.dwGBitMask) >> gBitShift;
                        totB += (*psrc16 & ddsdSource.ddpfPixelFormat.dwBBitMask) >> bBitShift;
                        totA += (*psrc16 & ddsdSource.ddpfPixelFormat.dwRGBAlphaBitMask) >> aBitShift;
                        // Back to the left and down one line
                        psrc16--;
                        psrc16 = (WORD*)((BYTE*)psrc16 + ddsdSource.lPitch);
                        totR += (*psrc16 & ddsdSource.ddpfPixelFormat.dwRBitMask) >> rBitShift;
                        totG += (*psrc16 & ddsdSource.ddpfPixelFormat.dwGBitMask) >> gBitShift;
                        totB += (*psrc16 & ddsdSource.ddpfPixelFormat.dwBBitMask) >> bBitShift;
                        totA += (*psrc16 & ddsdSource.ddpfPixelFormat.dwRGBAlphaBitMask) >> aBitShift;
                        // One to the right
                        psrc16++;
                        totR += (*psrc16 & ddsdSource.ddpfPixelFormat.dwRBitMask) >> rBitShift;
                        totG += (*psrc16 & ddsdSource.ddpfPixelFormat.dwGBitMask) >> gBitShift;
                        totB += (*psrc16 & ddsdSource.ddpfPixelFormat.dwBBitMask) >> bBitShift;
                        totA += (*psrc16 & ddsdSource.ddpfPixelFormat.dwRGBAlphaBitMask) >> aBitShift;
                        // Back up to the previous line, along one ready for next
                        psrc16 = (WORD*)((BYTE*)psrc16 - ddsdSource.lPitch);
                        psrc16++;


                        // Assign pixel
                        avgR = ((totR / 4) << rBitShift) & ddsdSource.ddpfPixelFormat.dwRBitMask;
                        avgG = ((totG / 4) << gBitShift) & ddsdSource.ddpfPixelFormat.dwGBitMask;
                        avgB = ((totB / 4) << bBitShift) & ddsdSource.ddpfPixelFormat.dwBBitMask;
                        avgA = ((totA / 4) << aBitShift) & ddsdSource.ddpfPixelFormat.dwRGBAlphaBitMask;
                        *pdest16 = (WORD)(avgR | avgG | avgB | avgA);
                        pdest16++;
                    }
                    else if (ddsdSource.ddpfPixelFormat.dwRGBBitCount == 32)
                    {
                        totR += (*psrc32 & ddsdSource.ddpfPixelFormat.dwRBitMask) >> rBitShift;
                        totG += (*psrc32 & ddsdSource.ddpfPixelFormat.dwGBitMask) >> gBitShift;
                        totB += (*psrc32 & ddsdSource.ddpfPixelFormat.dwBBitMask) >> bBitShift;
                        totA += (*psrc32 & ddsdSource.ddpfPixelFormat.dwRGBAlphaBitMask) >> aBitShift;
                        // one to the right
                        psrc32++;
                        totR += (*psrc32 & ddsdSource.ddpfPixelFormat.dwRBitMask) >> rBitShift;
                        totG += (*psrc32 & ddsdSource.ddpfPixelFormat.dwGBitMask) >> gBitShift;
                        totB += (*psrc32 & ddsdSource.ddpfPixelFormat.dwBBitMask) >> bBitShift;
                        totA += (*psrc32 & ddsdSource.ddpfPixelFormat.dwRGBAlphaBitMask) >> aBitShift;
                        // Back to the left and down one line
                        psrc32--;
                        psrc32 = (DWORD*)((BYTE*)psrc32 + ddsdSource.lPitch);
                        totR += (*psrc32 & ddsdSource.ddpfPixelFormat.dwRBitMask) >> rBitShift;
                        totG += (*psrc32 & ddsdSource.ddpfPixelFormat.dwGBitMask) >> gBitShift;
                        totB += (*psrc32 & ddsdSource.ddpfPixelFormat.dwBBitMask) >> bBitShift;
                        totA += (*psrc32 & ddsdSource.ddpfPixelFormat.dwRGBAlphaBitMask) >> aBitShift;
                        // One to the right
                        psrc32++;
                        totR += (*psrc32 & ddsdSource.ddpfPixelFormat.dwRBitMask) >> rBitShift;
                        totG += (*psrc32 & ddsdSource.ddpfPixelFormat.dwGBitMask) >> gBitShift;
                        totB += (*psrc32 & ddsdSource.ddpfPixelFormat.dwBBitMask) >> bBitShift;
                        totA += (*psrc32 & ddsdSource.ddpfPixelFormat.dwRGBAlphaBitMask) >> aBitShift;
                        // Back up to the previous line, along one ready for next
                        psrc32 = (DWORD*)((BYTE*)psrc32 - ddsdSource.lPitch);
                        psrc32++;


                        // Assign pixel
                        avgR = ((totR / 4) << rBitShift) & ddsdSource.ddpfPixelFormat.dwRBitMask;
                        avgG = ((totG / 4) << gBitShift) & ddsdSource.ddpfPixelFormat.dwGBitMask;
                        avgB = ((totB / 4) << bBitShift) & ddsdSource.ddpfPixelFormat.dwGBitMask;
                        avgA = ((totA / 4) << aBitShift) & ddsdSource.ddpfPixelFormat.dwRGBAlphaBitMask;
                        *pdest32 = (DWORD)(avgR | avgG | avgB | avgA);
                        pdest32++;
                    }
                } // for x

            } // for y

            // Unlock surfaces
            lpddsSource->Unlock(NULL);
            lpddsDest->Unlock(NULL);

            // Move to next level
            lpddsSource = lpddsDest;

        } // for iMipLvl


    }

    LPDIRECTDRAWSURFACE7 D3DTexture::getDDSurface(void)
    {
        return mSurface;
    }
}


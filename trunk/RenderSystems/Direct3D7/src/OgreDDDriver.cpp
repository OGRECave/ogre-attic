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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreD3D7RenderSystem.h"
#include "OgreDDDriver.h"
#include "OgreDDVideoModeList.h"
#include "OgreDDVideoMode.h"
#include "OgreD3D7DeviceList.h"
#include "OgreD3D7Device.h"
#include "png.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreBitwise.h"

namespace Ogre {
    DDDriver DDDriver::operator=(const DDDriver &orig)
    {


        mGuid = orig.mGuid;
        //mDriverDesc = new char(strlen(orig.mDriverDesc) + 1);
        //strcpy(mDriverDesc, orig.mDriverDesc);
        mDriverDesc = orig.mDriverDesc;
        //mDriverName = new char(strlen(orig.mDriverName)+1);
        //strcpy(mDriverName, orig.mDriverName);
        mDriverName = orig.mDriverName;
        mPrimaryDisplay = orig.mPrimaryDisplay;
        active3DDevice = orig.active3DDevice;
        activeVideoMode = orig.activeVideoMode;
        lpD3D = orig.lpD3D;
        lpDD7 = orig.lpDD7;
        lpDDClipper = orig.lpDDClipper;
        lpDDSBack = orig.lpDDSBack;
        lpDDSPrimary = orig.lpDDSPrimary;
        mSWCaps = orig.mSWCaps;
        mHWCaps = orig.mHWCaps;
        mDeviceList = orig.mDeviceList;
        mVideoModeList = orig.mVideoModeList;


        return *this;

    }

    // Default constructor
    DDDriver::DDDriver()
    {
        // Init pointers
        active3DDevice = NULL;
        activeHWnd = 0;
        activeVideoMode = NULL;
        lpD3D = NULL;
        lpDD7 = NULL;
        lpDDClipper = NULL;
        lpDDSBack = NULL;
        lpDDSPrimary = NULL;
        mDeviceList = NULL;
        mVideoModeList = NULL;


    }

    DDDriver::~DDDriver()
    {
        // Delete related system objects
        if (mDeviceList)
            delete mDeviceList;
        if (mVideoModeList)
            delete mVideoModeList;

    }


    // Copy Constructor
    DDDriver::DDDriver(const DDDriver &ob)
    {


        mGuid = ob.mGuid;
        //mDriverDesc = new char(strlen(ob.mDriverDesc) + 1);
        //strcpy(mDriverDesc, ob.mDriverDesc);
        mDriverDesc = ob.mDriverDesc;
        //mDriverName = new char(strlen(ob.mDriverName)+1);
        //strcpy(mDriverName, ob.mDriverName);
        mDriverName = ob.mDriverName;
        mPrimaryDisplay = ob.mPrimaryDisplay;
        active3DDevice = ob.active3DDevice;
        activeVideoMode = ob.activeVideoMode;
        lpD3D = ob.lpD3D;
        lpDD7 = ob.lpDD7;
        lpDDClipper = ob.lpDDClipper;
        lpDDSBack = ob.lpDDSBack;
        lpDDSPrimary = ob.lpDDSPrimary;
        mSWCaps = ob.mSWCaps;
        mHWCaps = ob.mHWCaps;
        mDeviceList = ob.mDeviceList;
        mVideoModeList = ob.mVideoModeList;

    }

    // Constructor with enumeration details
    DDDriver::DDDriver(GUID FAR *lpGuid,
                                LPSTR lpDriverDescription,
                                LPSTR lpDriverName)
    {
        HRESULT hr;

        // Init pointers
        active3DDevice = NULL;
        activeHWnd = 0;
        activeVideoMode = NULL;
        lpD3D = NULL;
        lpDD7 = NULL;
        lpDDClipper = NULL;
        lpDDSBack = NULL;
        lpDDSPrimary = NULL;
        mDeviceList = NULL;
        mVideoModeList = NULL;

        // Copy GUID, Description and Name
        // Deal with NULL (default display driver)
        if (lpGuid)
        {
            memcpy(&mGuid, lpGuid, sizeof(GUID));
            mPrimaryDisplay = false;
        }
        else
            mPrimaryDisplay = true;


        mDriverDesc = String(lpDriverDescription);
        //mDriverDesc = new char(strlen(lpDriverDescription)+1);
        //strcpy(mDriverDesc, lpDriverDescription);

        mDriverName = String(lpDriverName);
        //mDriverName = new char(strlen(lpDriverName)+1);
        //strcpy(mDriverName, lpDriverName);


        // Get capabilities
        mHWCaps.dwSize = sizeof(DDCAPS);
        mSWCaps.dwSize = sizeof(DDCAPS);

        hr = directDraw()->GetCaps(&mHWCaps, &mSWCaps);
        if (FAILED(hr))
            throw Exception(hr, "Cannot get direct draw driver capabilities.",
                "DIM_DDDriver - directDraw()");


    }

    void DDDriver::createWindowSurfaces(HWND hWnd, int width, int height, int colourDepth, bool fullScreen,
            LPDIRECTDRAWSURFACE7 *front, LPDIRECTDRAWSURFACE7 *back)
    {

        char msg[150];
        DWORD dwFlags;
        DDSURFACEDESC2 ddsd;
        HRESULT hr;
        DDSCAPS2 ddscaps;
        LPDIRECTDRAWCLIPPER clip;

        LogManager::getSingleton().logMessage("Creating DirectDraw surfaces for window with dimensions:");

        if (fullScreen)
            sprintf(msg, "  FULLSCREEN w:%i h:%i bpp:%i", width, height, colourDepth);
        else
            sprintf(msg, "  WINDOWED w:%i h:%i", width, height);

        LogManager::getSingleton().logMessage(msg);

        // Set co-op level
        if (fullScreen)
        {
            dwFlags = DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE;
        }
        else
        {
            dwFlags = DDSCL_NORMAL;
        }

        // Also set FPU cooperative mode (optimisation)
        dwFlags = dwFlags | DDSCL_FPUSETUP;

        hr = directDraw()->SetCooperativeLevel(hWnd, dwFlags);
        if (FAILED(hr))
            throw Exception(hr, "Error setting cooperative mode",
                "DDDriver - createWindowSurfaces");


        // Create surfaces
        // If we're running fullscreen, create a complex flipping
        // chain surface (implicit back buffer)
        // Otherwise we need separate back buffers

        if (fullScreen)
        {
            // Set Video Mode
            hr = lpDD7->SetDisplayMode(width, height, colourDepth, 0, 0);
            if (FAILED(hr))
                throw Exception(hr, "Unable to set fullScreen display mode.", "DDDriver - createWindowSurfaces");

            // Set up surfaces
            ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
            ddsd.dwSize = sizeof(DDSURFACEDESC2);
            ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
            ddsd.ddsCaps.dwCaps = DDSCAPS_COMPLEX |
                DDSCAPS_FLIP | DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;

            // Create a single back buffer
            ddsd.dwBackBufferCount = 1;
            hr = directDraw()->CreateSurface(&ddsd,front,NULL);
            if (FAILED(hr))
                throw Exception(hr, "Error creating linked surface buffers",
                    "DDDriver - createWindowSurfaces");

            // Get back buffer
            ZeroMemory(&ddscaps, sizeof(DDSCAPS2));
            ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
            hr = (*front)->GetAttachedSurface(&ddscaps, back);

            if (FAILED(hr))
                throw Exception(hr, "Error retrieving linked back buffer",
                    "DDDriver - createWindowSurfaces");

            LogManager::getSingleton().logMessage("Successfully created full screen rendering surface / flipping chain.");
        }
        else
        {
            // Explicitly create front and back buffers, and create
            // a clipper object

            // Standard non-auto-flip primary
            ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
            ddsd.dwSize  = sizeof(DDSURFACEDESC2);
            ddsd.dwFlags = DDSD_CAPS;
            ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
            hr = directDraw()->CreateSurface(&ddsd,front,NULL);
            if (FAILED(hr))
                throw Exception(hr, "Error creating primary surface buffer",
                    "DDDriver - createWindowSurfaces");


            // Create offscreen buffer
            ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
            ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |
                DDSCAPS_3DDEVICE;
            // Set the dimensions of the back buffer. Note that if our window changes
            // size, we need to destroy this surface and create a new one.
            ddsd.dwWidth  = width;
            ddsd.dwHeight = height;
            // Create the back buffer. The most likely reason for failure is running
            // out of video memory. (A more sophisticated app should handle this.)
            hr = directDraw()->CreateSurface( &ddsd, back, NULL );
            if( FAILED( hr ) )
                throw Exception(hr, "Error creating back surface buffer",
                    "DDDriver - createWindowSurfaces");

            LogManager::getSingleton().logMessage("Windowed mode rendering & display surfaces created.");

            // Create clipper
            hr = directDraw()->CreateClipper(0, &clip, NULL);
            if (FAILED(hr))
                throw Exception(hr, "Error creating clipper",
                    "DDDriver - createWindowSurfaces");

            // Assign it to the window
            clip->SetHWnd(0,hWnd);
            // Assign it to primary surface
            (*front)->SetClipper(clip);
            clip->Release();

            LogManager::getSingleton().logMessage("Window clipper created.");

        }



    }


    String DDDriver::DriverName(void)
    {
        return mDriverName;
    }

    String DDDriver::DriverDescription(void)
    {
        return mDriverDesc;
    }

    D3DDeviceList* DDDriver::get3DDeviceList(void)
    {
        if (!mDeviceList)
            mDeviceList = new D3DDeviceList(lpD3D);

        return mDeviceList;
    }

    DDVideoModeList* DDDriver::getVideoModeList(void)
    {
        if (!mVideoModeList)
            mVideoModeList = new DDVideoModeList(lpDD7);

        return mVideoModeList;
    }

    DDVideoMode* DDDriver::getActiveVideoMode(void)
    {
        return activeVideoMode;
    }


    D3DDevice* DDDriver::get3DDevice(void)
    {
        return active3DDevice;
    }


    LPDIRECTDRAW7 DDDriver::directDraw()
    {
        HRESULT rVal;

        if (!lpDD7)
        {
            // We need to create a direct draw object
            // Create with GUID for this driver
            if (mPrimaryDisplay)
                rVal = DirectDrawCreateEx( NULL, (VOID**)&lpDD7, IID_IDirectDraw7, NULL );
            else
                rVal = DirectDrawCreateEx( &mGuid, (VOID**)&lpDD7, IID_IDirectDraw7, NULL );
            if (rVal != DD_OK)
                throw Exception(rVal, "Cannot create direct draw interface.",
                    "DIM_DDDriver - directDraw()");

            // Get Direct3D interface too
            lpDD7->QueryInterface(IID_IDirect3D7, (VOID**)&lpD3D);


        }

        return lpDD7;
    }

    void DDDriver::Cleanup(void)
    {
        // Tell 3D Device to clean itself up
        if (active3DDevice)
        {
            active3DDevice->Cleanup();
        }

        if (activeHWnd && lpDD7)
            lpDD7->SetCooperativeLevel(activeHWnd, DDSCL_NORMAL);

        // Release DirectX system objects

        if (lpDDSBack)
        {
            lpDDSBack->Release();
            lpDDSBack = NULL;
        }

        if (lpDDSPrimary)
        {
            lpDDSPrimary->Release();
            lpDDSPrimary = NULL;
        }

        if (lpD3D)
        {
            lpD3D->Release();
            lpD3D = NULL;
        }

        if (lpDD7)
        {
            lpDD7->Release();
            lpDD7 = NULL;
        }




    }


    void DDDriver::CheckWindow(void)
    {
        // If windowed mode, check window size & position
        RECT rcCheck;

        if (RunningFullScreen())
            return;

        GetClientRect( activeHWnd, &rcCheck );
        ClientToScreen( activeHWnd, (POINT*)&rcCheck.left );
        ClientToScreen( activeHWnd, (POINT*)&rcCheck.right );

        // Has the window resized? If so, we need to recreate surfaces
        if ((rcCheck.right - rcCheck.left !=
                rcViewport.right - rcViewport.left) ||
            (rcCheck.bottom - rcCheck.top !=
                rcViewport.bottom - rcViewport.top))
        {
            // The window has changed size
            //g_DIMSystem->CleanupRenderer();
            //g_DIMSystem->Reinitialise();
        }
        else if (rcCheck.left != rcViewport.left ||
            rcCheck.top != rcViewport.top)
        {
            // Window has only moved
            // Just alter the blit location
            rcViewport = rcCheck;
        }


    }


    void DDDriver::FlipBuffers(void)
    {
        HRESULT hr;
        if (runningFullScreen)
        {
            // Use flipping chain
            hr = lpDDSPrimary->Flip(NULL,DDFLIP_WAIT);


        }
        else
        {
            // Ordinary Blit
            RECT srcRect;
            srcRect.left = 0;
            srcRect.top = 0;
            srcRect.right = rcViewport.right - rcViewport.left;
            srcRect.bottom = rcViewport.bottom - rcViewport.top;
            hr = lpDDSPrimary->Blt(&rcViewport,lpDDSBack,&srcRect,DDBLT_WAIT,NULL);
        }

        if (hr == DDERR_SURFACELOST)
        {
            // Restore surfaces
            RestoreSurfaces();
        }
        else
        {
            if (FAILED(hr))
             throw Exception(hr,"Error flipping surfaces", "DDDriver::FlipBuffers");
        }

    }

    void DDDriver::RestoreSurfaces(void)
    {
        HRESULT hr;

        if (lpDDSPrimary->IsLost())
        {
            hr = lpDDSPrimary->Restore();
            if (FAILED(hr))
                throw Exception(hr, "Error restoring lost primary surface.", "DDDriver - RestoreSurfaces");
        }

        if (lpDDSBack->IsLost())
        {
            hr = lpDDSBack->Restore();
            if (FAILED(hr))
                throw Exception(hr, "Error restoring lost back buffer surface.", "DDDriver - RestoreSurfaces");
        }

    }

    void DDDriver::OutputText(int x, int y, char* text)
    {
        HDC hDC;

        // Get a DC for the surface. Then, write out the buffer
        if( lpDDSBack )
        {
            if( SUCCEEDED( lpDDSBack->GetDC(&hDC) ) )
            {
                SetTextColor( hDC, RGB(255,255,0) );
                SetBkMode( hDC, TRANSPARENT );
                ExtTextOut( hDC, x, y, 0, NULL, text, lstrlen(text), NULL );
                lpDDSBack->ReleaseDC(hDC);
            }
        }
    }

    bool DDDriver::RunningFullScreen(void)
    {
        return runningFullScreen;
    }

    RECT DDDriver::ViewportRect(void)
    {
        return rcViewport;
    }

    bool DDDriver::CanRenderWindowed(void)
    {
        return (mHWCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED) > 0;
    }


    bool DDDriver::Has3DAcceleration(void)
    {
        return (mHWCaps.dwCaps & DDCAPS_3D);
    }



    void DDDriver::GetDisplayDetails(int& width, int& height, int& colourDepth)
    {
        // Get details from primary surface
        // This works for both windowed and fullscreen modes

        DDSURFACEDESC2 ddsd;


        if (lpDDSPrimary)
        {
            ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
            ddsd.dwSize = sizeof(DDSURFACEDESC2);
            lpDDSPrimary->GetSurfaceDesc(&ddsd);

            width = ddsd.dwWidth;
            height = ddsd.dwHeight;
            colourDepth = ddsd.ddpfPixelFormat.dwRGBBitCount;
        }
    }

    void DDDriver::logCaps(void)
    {
        // Sends capabilities of this driver to the log
        char msg[255];

        LogManager::getSingleton().logMessage("DirectDraw Driver Capabilities:");

        sprintf(msg, "  3D Acceleration: %i", Has3DAcceleration());
        LogManager::getSingleton().logMessage(msg);

        sprintf(msg, "  Render in a window: %i", CanRenderWindowed());
        LogManager::getSingleton().logMessage(msg);



    }

    void DDDriver::takeScreenShot(void)
    {
        HRESULT hr;
        DDSURFACEDESC2 ddsd;
        static int numPics = 0;
        char filename[30];
        BYTE *pBuffer;
        BYTE **rowPointers;
        WORD *in16;
        DWORD *in32;
        BYTE *out8;
        FILE *fp;
        unsigned long lRow, lCol;


        // Lock front buffer
        ddsd.dwSize = sizeof(DDSURFACEDESC2);
        hr = lpDDSPrimary->Lock(NULL, &ddsd, NULL, NULL);

        if (FAILED(hr))
            throw Exception(hr, "Cannot lock surface.",
                "DDDriver::takeScreenShot");

        // Copy screen contents into a buffer, doing translation
        // to PNG-compatible bit depth (8-bits per channel)
        // Also create row pointers that PNG encoding requires
        pBuffer = (BYTE*)malloc(ddsd.ddpfPixelFormat.dwRGBBitCount *
                            ddsd.dwHeight * ddsd.dwWidth);
        rowPointers = (BYTE**)malloc(sizeof(BYTE*) * ddsd.dwHeight);
        out8 = pBuffer;
        for (lRow = 0; lRow < ddsd.dwHeight; lRow++)
        {
            rowPointers[lRow] = pBuffer + (3 * ddsd.dwWidth * lRow);
            if(ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
                in16 = (WORD*)ddsd.lpSurface + (ddsd.lPitch/2 * lRow);
            else
                in32 = (DWORD*)ddsd.lpSurface + (ddsd.lPitch/4 * lRow);

            for (lCol = 0; lCol < ddsd.dwWidth; lCol++)
            {
                // Convert each colour component to it's 8-bit
                //   counterpart
                BYTE mask8 = 0xFF;
                if(ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
                {
                    Bitwise::convertBitPattern(in16, &ddsd.ddpfPixelFormat.dwRBitMask, 16,
                                    out8++, &mask8, 8);
                    Bitwise::convertBitPattern(in16, &ddsd.ddpfPixelFormat.dwGBitMask, 16,
                                    out8++, &mask8, 8);
                    Bitwise::convertBitPattern(in16, &ddsd.ddpfPixelFormat.dwBBitMask, 16,
                                    out8++, &mask8, 8);
                    in16++;
                }
                else
                {
                    Bitwise::convertBitPattern(in32, &ddsd.ddpfPixelFormat.dwRBitMask, 32,
                                    out8++, &mask8, 8);
                    Bitwise::convertBitPattern(in32, &ddsd.ddpfPixelFormat.dwGBitMask, 32,
                                    out8++, &mask8, 8);
                    Bitwise::convertBitPattern(in32, &ddsd.ddpfPixelFormat.dwBBitMask, 32,
                                    out8++, &mask8, 8);
                    in32++;
                }
            }
        }

        // Unlock surface
        lpDDSPrimary->Unlock(NULL);

        numPics++;
        sprintf(filename, "screenshot%3.3d.png", numPics);

        fp = fopen(filename,"wb");

        if (!fp)
            throw Exception(999,"Cannot open file for writing.",
                "DDDriver::takeScreenShot");

        png_structp png_ptr = png_create_write_struct
           (PNG_LIBPNG_VER_STRING, NULL,
            NULL, NULL);
        if (!png_ptr)
            throw Exception(999,"Error initialising PNG library.",
                "DDDriver::takeScreenShot");

        png_infop info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
        {
           png_destroy_write_struct(&png_ptr,
             (png_infopp)NULL);
            throw Exception(999,"Error initialising PNG library.",
                "DDDriver::takeScreenShot");
        }

        png_init_io(png_ptr, fp);

        // Set image size, and channel bit depth (8),
        //  colour type (RGB), interlace type (NONE),
        //  compression & filter types (DEFAULT),
        png_set_IHDR(png_ptr, info_ptr, ddsd.dwWidth, ddsd.dwHeight,
           8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
           PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        // Omit all other info like gamma, description etc

        // Write info
        png_write_info(png_ptr, info_ptr);

        // Write data
        png_write_image(png_ptr, rowPointers);

        png_write_end(png_ptr, info_ptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);


        free(pBuffer);

    }

}



/*-------------------------------------------------------------------------
This source file is a part of OGRE
(Object-oriented Graphics Rendering Engine)

For the latest info, see http://ogre.sourceforge.net/

Copyright © 2000-2002 The OGRE Team
Also see acknowledgements in Readme.html

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License (LGPL) as 
published by the Free Software Foundation; either version 2.1 of the 
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public 
License for more details.

You should have received a copy of the GNU Lesser General Public License 
along with this library; if not, write to the Free Software Foundation, 
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA or go to
http://www.gnu.org/copyleft/lesser.txt
-------------------------------------------------------------------------*/
#include "OgreD3D7RenderWindow.h"

#include "OgreLogManager.h"
#include "OgreViewport.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreD3D7TextureManager.h"
#include "OgreBitwise.h"
#include "OgreImageCodec.h"

namespace Ogre {


    // Window procedure callback
    // This is a static member, so applies to all windows but we store the 
    // D3D7RenderWindow instance in the window data GetWindowLong/SetWindowLong.
    LRESULT D3D7RenderWindow::WndProc( 
        HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
    {
        LPCREATESTRUCT lpcs;
        D3D7RenderWindow* win;

        // look up window instance
        if (uMsg != WM_CREATE)
        {
            // Get window pointer
            win = (D3D7RenderWindow*)GetWindowLong(hWnd, 0);
        }

        switch( uMsg )
        {
            case WM_ACTIVATE:
                if( LOWORD( wParam ) == WA_INACTIVE )
                    win->mActive = false;
                else
                    win->mActive = true;
                break;

            case WM_CREATE:
                // Log the new window
                // Get CREATESTRUCT
                lpcs = (LPCREATESTRUCT)lParam;
                win = (D3D7RenderWindow*)(lpcs->lpCreateParams);
                // Store pointer in window user data area
                SetWindowLong(hWnd, 0, (long)win);
                win->mActive = true;

                return 0;

            case WM_KEYDOWN:
                // TEMPORARY CODE
                // TODO - queue up keydown / keyup events with
                //  window name and timestamp to be processed
                //  by main loop

                // ESCAPE closes window
                //if (wParam == VK_ESCAPE)
                //{
                //    win->mClosed = true;
                //    return 0L;
                //}
                break;

            case WM_PAINT:
                // If we get WM_PAINT messages, it usually means our window was
                // covered up, so we need to refresh it by re-showing the contents
                // of the current frame.
                if (win->mActive && win->mReady)
                    win->update();
                break;

            case WM_MOVE:
                // Move messages need to be tracked to update the screen rects
                // used for blitting the backbuffer to the primary.
                if(win->mActive && win->mReady)
                    win->windowMovedOrResized();
                break;

            case WM_ENTERSIZEMOVE:
                // Prevent rendering while moving / sizing
                win->mReady = false;
                break;

            case WM_EXITSIZEMOVE:
                win->windowMovedOrResized();
                win->mReady = true;
                break;

            case WM_SIZE:
                // Check to see if we are losing or gaining our window. Set the
                // active flag to match.
                if( SIZE_MAXHIDE==wParam || SIZE_MINIMIZED==wParam )
                    win->mActive = false;
                else
                {
                    win->mActive = true;
                    if (win->mReady)
                        win->windowMovedOrResized();
                }

                break;

            case WM_GETMINMAXINFO:
                // Prevent the window from going smaller than some minimum size
                ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
                ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
                break;

            case WM_CLOSE:
                DestroyWindow( win->mHWnd );
                Root::getSingleton().getRenderSystem()->detachRenderTarget( win->getName() );
                win->mClosed = true;
                return 0;

            /*
            case WM_DESTROY:
                g_DIMSystem->CleanupRenderer();
                PostQuitMessage(0);
                return 0L;
            */
        }



        return DefWindowProc( hWnd, uMsg, wParam, lParam );
    }

    // -------------------------------------------
    // D3D7RenderWindow Implementation
    // -------------------------------------------
    D3D7RenderWindow::D3D7RenderWindow()
    {
        mIsUsingDirectDraw = false;
        mIsFullScreen = false;
        mlpDDDriver = 0;
        mHWnd = 0;
        mActive = false;
        mReady = false;
        mClosed = false;
    }

    D3D7RenderWindow::~D3D7RenderWindow()
    {
    }




    void D3D7RenderWindow::create(const String& name, int width, int height, int colourDepth,
            bool fullScreen, int left, int top, bool depthBuffer, void* miscParam, ...)
    {

        HWND parentHWnd;
        HINSTANCE hInst;
        DDDriver* drv;
        long tempPtr;


        // Get variable-length params
        // miscParam[0] = HINSTANCE
        // miscParam[1] = DDDriver
        // miscParam[2] = parent HWND
        va_list marker;
        va_start(marker, depthBuffer);

        tempPtr = va_arg(marker, long);
        hInst = *(HINSTANCE*)tempPtr;

        tempPtr = va_arg(marker, long);
        drv = (DDDriver*)tempPtr;

        tempPtr = va_arg(marker, long);
        D3D7RenderWindow* parentRW = (D3D7RenderWindow*)tempPtr;
        if (parentRW == 0)
        {
            parentHWnd = 0;
        }
        else
        {
            parentHWnd = parentRW->getWindowHandle();
        }

        va_end(marker);

        // Destroy current window if any
        if (mHWnd)
        {
            destroy();
        }

        // TODO: deal with child windows
        mParentHWnd = parentHWnd;


        // Register the window class
        // NB Allow 4 bytes of window data for D3D7RenderWindow pointer
        WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW, WndProc, 0, 4, hInst,
                              LoadIcon( NULL, "IDI_ICON1" ),
                              LoadCursor(NULL, IDC_ARROW),
                              (HBRUSH)GetStockObject(BLACK_BRUSH), NULL,
                              TEXT(name.c_str()) };
        RegisterClass( &wndClass );

        // Create our main window
        // Pass pointer to self
        HWND hWnd = CreateWindow( TEXT(name.c_str()),
                                  TEXT(name.c_str()),
                                  WS_OVERLAPPEDWINDOW, left, top,
                                  width, height, 0L, 0L, hInst, this );
        ShowWindow( hWnd, SW_SHOWNORMAL );
        UpdateWindow( hWnd );

        mHWnd = hWnd;

        // Store info
        mName = name;
        mWidth = width;
        mHeight = height;
        mIsDepthBuffered = depthBuffer;
        mIsFullScreen = fullScreen;

        if (fullScreen)
        {
            mColourDepth = colourDepth;
            mLeft = 0;
            mTop = 0;
        }
        else
        {
            // Get colour depth from display
            HDC hdc = GetDC( mHWnd );
            mColourDepth = GetDeviceCaps( hdc, BITSPIXEL );
            ReleaseDC( mHWnd, hdc );
            mTop = top;
            mLeft = left;

            // Record the dimensions of the blit location
            GetClientRect( mHWnd, &rcBlitDest );
            ClientToScreen( mHWnd, (POINT*)&rcBlitDest.left );
            ClientToScreen( mHWnd, (POINT*)&rcBlitDest.right );
        }

        LogManager::getSingleton().logMessage( 
            LML_NORMAL, 
            "Created Win32 Rendering Window '%s': %i x %i @ %ibpp", 
            mName.c_str(), mWidth, mHeight, mColourDepth );

        // Set up DirectDraw if appropriate
        // NB devices & surfaces set up for root window only
        if (drv && mParentHWnd == NULL)
        {
            // DD Driver object passed
            mlpDDDriver = drv;
            mIsUsingDirectDraw = true;

            createDDSurfaces();
            // Get best device based on render bit depth
            D3DDevice* dev = mlpDDDriver->get3DDeviceList()->getBest(mColourDepth);

            // create D3D device for this window
            mlpD3DDevice = dev->createDevice(mlpDDSBack);
            createDepthBuffer();
        }
        else
            mIsUsingDirectDraw = false;

        mReady = true;
    }

    void D3D7RenderWindow::destroy(void)
    {
        if (mIsUsingDirectDraw)
            releaseDDSurfaces();

        DestroyWindow(mHWnd);

    }

    bool D3D7RenderWindow::isActive(void)
    {
        return mActive;
    }

    bool D3D7RenderWindow::isClosed(void)
    {
        return mClosed;
    }

    void D3D7RenderWindow::reposition(int left, int top)
    {

    }

    void D3D7RenderWindow::resize(int width, int height)
    {
        mWidth = width;
        mHeight = height;

        // Notify viewports of resize
        ViewportList::iterator it = mViewportList.begin();
        while (it != mViewportList.end())
            (*it).second->_updateDimensions();

        // TODO - resize window

    }

    void D3D7RenderWindow::swapBuffers(bool waitForVSync)
    {
        HRESULT hr;
        DWORD flags;
        if (mIsUsingDirectDraw)
        {
            if (mIsFullScreen)
            {
                // Use flipping chain
                if (waitForVSync)
                {
                    flags = DDFLIP_WAIT;
                }
                else
                {
                    flags = DDFLIP_WAIT | DDFLIP_NOVSYNC;
                }

                hr = mlpDDSFront->Flip(NULL,flags);
            }
            else
            {
                // Ordinary Blit
                RECT srcRect;
                srcRect.left = 0;
                srcRect.top = 0;
                srcRect.right = rcBlitDest.right - rcBlitDest.left;
                srcRect.bottom = rcBlitDest.bottom - rcBlitDest.top;
                hr = mlpDDSFront->Blt(&rcBlitDest,mlpDDSBack,&srcRect,DDBLT_WAIT,NULL);
            }

            if (hr == DDERR_SURFACELOST)
            {
                // Restore surfaces
                restoreDDSurfaces();
            }
            else if (FAILED(hr))
            {
                 Except( 
                     hr,
                     "Error flipping surfaces", 
                     "D3D7RenderWindow::swapBuffers" );
            }
        }

    }

    HWND D3D7RenderWindow::getWindowHandle(void)
    {
        return mHWnd;
    }

    HWND D3D7RenderWindow::getParentWindowHandle(void)
    {
        return mParentHWnd;
    }

    bool D3D7RenderWindow::isUsingDirectDraw(void)
    {
        return mIsUsingDirectDraw;
    }

    // -------------------------------------------------------
    //   DirectDraw specific methods
    // -------------------------------------------------------
    DDDriver* D3D7RenderWindow::getDirectDrawDriver(void)
    {
        return mlpDDDriver;
    }

    LPDIRECTDRAWSURFACE7 D3D7RenderWindow::getDDFrontBuffer(void)
    {
        return mlpDDSFront;

    }

    LPDIRECTDRAWSURFACE7 D3D7RenderWindow::getDDBackBuffer(void)
    {
        return mlpDDSBack;
    }
    LPDIRECT3DDEVICE7 D3D7RenderWindow::getD3DDevice(void)
    {
        return mlpD3DDevice;
    }

    void D3D7RenderWindow::createDDSurfaces(void)
    {
        // Use DirectDraw wrapper object to create surfaces
        if( !mlpDDDriver )
        {
            Except(
            Exception::ERR_INVALIDPARAMS, 
            "Cannot create surfaces because of no valid DirectDraw object",
            "D3D7RenderWindow::createDDSurfaces" );
        }

        if( mIsFullScreen )
        {
            mlpDDDriver->createWindowSurfaces(
                mHWnd, 
                mWidth, mHeight, mColourDepth,
                true, &mlpDDSFront, &mlpDDSBack);
        }
        else
        {
            // Windowed mode - need to use client rect for surface dimensions
            // I.e. we need to ignore menu bars, borders, title bar etc
            RECT rcClient;
            int cWidth, cHeight;

            GetClientRect( mHWnd, &rcClient );
            ClientToScreen( mHWnd, (POINT*)&rcClient.left );
            ClientToScreen( mHWnd, (POINT*)&rcClient.right );

            cWidth  = rcClient.right - rcClient.left;
            cHeight = rcClient.bottom - rcClient.top;

            // Create surfaces (AND clipper)
            mlpDDDriver->createWindowSurfaces(
                mHWnd, 
                cWidth, cHeight, 0,
                false, &mlpDDSFront, &mlpDDSBack);

            // Update own dimensions since target width / height is this
            mWidth = cWidth;
            mHeight = cHeight;
        }
    }

    void D3D7RenderWindow::createDepthBuffer(void)
    {
        // Get best device based on render bit depth
        D3DDevice* dev = mlpDDDriver->get3DDeviceList()->getBest(mColourDepth);
        if (mIsDepthBuffered && dev->NeedsZBuffer())
        {
            // Create Z buffer
            dev->createDepthBuffer(mlpDDSBack);
            // Don't set z buffer params here, leave to material
        }
    }

    void D3D7RenderWindow::releaseDDSurfaces(void)
    {
        // Release Z-buffer
        HRESULT hr;
        DDSCAPS2 ddscaps;

        ZeroMemory(&ddscaps, sizeof(DDSCAPS2));
        ddscaps.dwCaps = DDSCAPS_ZBUFFER;

        LPDIRECTDRAWSURFACE7 zBufSurface;

        hr = mlpDDSBack->GetAttachedSurface( &ddscaps, &zBufSurface );

        // Release twice as this method has increased
        zBufSurface->Release();
        zBufSurface->Release();

        // Release std buffers
        mlpDDSBack->Release();
        mlpDDSFront->Release();

        mlpDDSBack = mlpDDSFront = 0;
    }

    void D3D7RenderWindow::restoreDDSurfaces(void)
    {
        HRESULT hr;

        if( mlpDDSFront->IsLost() )
        {
            hr = mlpDDSFront->Restore();

            if( FAILED( hr ) )
                Except( 
                    Exception::ERR_INTERNAL_ERROR, 
                    "Error restoring lost primary surface.", 
                    "D3D7RenderWindow - restoreDDSurfaces" );
        }

        if( mlpDDSBack->IsLost() )
        {
            hr = mlpDDSBack->Restore();

            if( FAILED( hr ) )
                Except( 
                    Exception::ERR_INTERNAL_ERROR, 
                    "Error restoring lost back buffer surface.", 
                    "D3D7RenderWindow - restoreDDSurfaces" );
        }
    }

    void D3D7RenderWindow::windowMovedOrResized(void)
    {
        // If windowed mode, check window size & position
        RECT rcCheck;

        if( mIsFullScreen )
            return; // Nothing to check

        GetClientRect( mHWnd, &rcCheck );
        ClientToScreen( mHWnd, (POINT*)&rcCheck.left );
        ClientToScreen( mHWnd, (POINT*)&rcCheck.right );

        // Has the window resized? If so, we need to recreate surfaces
        if( ( rcCheck.right - rcCheck.left != 
                rcBlitDest.right - rcBlitDest.left ) ||
            ( rcCheck.bottom - rcCheck.top !=
                rcBlitDest.bottom - rcBlitDest.top ) )
        {
            // The window has changed size - DD surfaces must be recreated
            releaseDDSurfaces();
            createDDSurfaces();
            createDepthBuffer();

            // Update Viewport Sizes
            
            for( 
                ViewportList::iterator it = mViewportList.begin();
                it != mViewportList.end(); 
                ++it )
            {
                it->second->_updateDimensions();
            }

            rcBlitDest = rcCheck;
        }
        else if( 
            rcCheck.left != rcBlitDest.left ||
            rcCheck.top  != rcBlitDest.top )
        {
            // Window has only moved
            // Just alter the blit location
            rcBlitDest = rcCheck;
        }
    }

    //-----------------------------------------------------------------------
    void D3D7RenderWindow::outputText( int x, int y, const String& text )
    {
        HDC hDC;

        if( mIsUsingDirectDraw && mlpDDSBack )
        {
            // Get a DC for the surface. Then, write out the buffer
            if( SUCCEEDED( mlpDDSBack->GetDC(&hDC) ) )
            {
                SetTextColor( hDC, RGB(255,255,0) );
                SetBkMode( hDC, TRANSPARENT );
                ExtTextOut( hDC, x, y, 0, NULL, text, lstrlen(text), NULL );
                mlpDDSBack->ReleaseDC(hDC);
            }
        }
    }

    //-----------------------------------------------------------------------
    void D3D7RenderWindow::getCustomAttribute( const String& name, void* pData )
    {
        // Valid attributes and their equivalent native functions:
        // D3DDEVICE            : getD3DDeviceDriver
        // DDBACKBUFFER         : getDDBackBuffer
        // DDFRONTBUFFER        : getDDFrontBuffer
        // HWND                 : getWindowHandle

        if( name == "D3DDEVICE" )
        {
            LPDIRECT3DDEVICE7 *pDev = (LPDIRECT3DDEVICE7*)pData;

            *pDev = getD3DDevice();
            return;
        }
        else if( name == "DDBACKBUFFER" )
        {
            LPDIRECTDRAWSURFACE7 *pSurf = (LPDIRECTDRAWSURFACE7*)pData;

            *pSurf = getDDBackBuffer();
            return;
        }
        else if( name == "DDFRONTBUFFER" )
        {
            LPDIRECTDRAWSURFACE7 *pSurf = (LPDIRECTDRAWSURFACE7*)pData;

            *pSurf = getDDFrontBuffer();
            return;
        }
        else if( name == "HWND" )
        {
            HWND *pHwnd = (HWND*)pData;

            *pHwnd = getWindowHandle();
            return;
        }
        else if( name == "isTexture" )
        {
            bool *b = reinterpret_cast< bool * >( pData );
            *b = false;

            return;
        }
    }


    void D3D7RenderWindow::writeContentsToFile(const String& filename)
    {
         HRESULT hr;
         LPDIRECTDRAWSURFACE7 pTempSurf;
         DDSURFACEDESC2 desc;
         RECT srcRect;
 
         // Cannot lock surface direct, so create temp surface and blit
         memset(&desc, 0, sizeof (DDSURFACEDESC2));
		 desc.dwSize = sizeof(DDSURFACEDESC2);
         if (FAILED(hr = mlpDDSBack->GetSurfaceDesc(&desc)))
         {
	         Except(hr, "Error getting description of back buffer!", 
                 "D3D7RenderWindow::writeContentsToFile");
         }

         desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
         desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;

		 if (FAILED(hr = mlpDDDriver->directDraw()->CreateSurface(&desc, &pTempSurf, NULL)))
		 {
             Except(hr, "Error creating temporary surface!", 
                 "D3D7RenderWindow::writeContentsToFile");
		 }

         if (mIsFullScreen)
		 {
         	pTempSurf->Blt(NULL, mlpDDSFront, NULL, NULL, NULL);
		 }
         else
         {
         	GetWindowRect(mHWnd, &srcRect);
            srcRect.left += GetSystemMetrics(SM_CXSIZEFRAME);
            srcRect.top += GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CYCAPTION);
            srcRect.right = srcRect.left + desc.dwWidth;
            srcRect.bottom = srcRect.top + desc.dwHeight;

            pTempSurf->Blt(NULL, mlpDDSFront, &srcRect, NULL, NULL);
         }

		 if (FAILED(hr = pTempSurf->Lock(NULL, &desc,  
             DDLOCK_WAIT | DDLOCK_READONLY, NULL)))
         {
             Except(hr, "Cannot lock surface!", 
                 "D3D7RenderWindow::writeContentsToFile");
         }
 
 
         ImageCodec::ImageData imgData;
         imgData.width = desc.dwWidth;
         imgData.height = desc.dwHeight;
         imgData.format = PF_R8G8B8;
 
         // Allocate contiguous buffer (surfaces aren't necessarily contiguous)
         uchar* pBuffer = new uchar[desc.dwWidth * desc.dwHeight * 3];
 
         uint x, y;
         uchar *pData, *pDest;
 
         pData = (uchar*)desc.lpSurface;
         pDest = pBuffer;
         for (y = 0; y < desc.dwHeight; ++y)
         {
             uchar *pRow = pData;
 
             for (x = 0; x < desc.dwWidth; ++x)
             {
				 if (desc.ddpfPixelFormat.dwRGBBitCount == 16)
				 {
                     WORD val;
					 BYTE result;
                     ushort srcMask;
                     BYTE destMask;
 
                     destMask = 0xFF;
                     val = *((WORD*)pRow);
					 pRow += 2;

					 srcMask = 0xF800;
                     Bitwise::convertBitPattern(&val, &srcMask, 16, &result, &destMask, 8);
					 *pDest++ = result;
                     srcMask = 0x07E0;
                     Bitwise::convertBitPattern(&val, &srcMask, 16, &result, &destMask, 8);
					 *pDest++ = result;
                     srcMask = 0x1F;
                     Bitwise::convertBitPattern(&val, &srcMask, 16, &result, &destMask, 8);
					 *pDest++ = result;
				 }
				 else
				 {
					 // Actual format is BRGA for some reason
                     *pDest++ = pRow[2]; // R
                     *pDest++ = pRow[1]; // G
                     *pDest++ = pRow[0]; // B

                     pRow += 3; // skip alpha / dummy

					 if (desc.ddpfPixelFormat.dwRGBBitCount == 32)
					 {
						 ++pRow; // Skip alpha
					 }
                 }
 
                 
             }
             // increase by one line
             pData += desc.lPitch;
 
         }
 
 
         // Wrap buffer in a chunk
         DataChunk chunk(pBuffer, desc.dwWidth * desc.dwHeight * 3);
 
         // Get codec 
         size_t pos = filename.find_last_of(".");
         String extension;
 	    if( pos == String::npos )
             Except(
 		    Exception::ERR_INVALIDPARAMS, 
 		    "Unable to determine image type for '" + filename + "' - invalid extension.",
             "D3D8RenderWindow::writeContentsToFile" );
 
         while( pos != filename.length() - 1 )
             extension += filename[++pos];
 
         // Get the codec
         Codec * pCodec = Codec::getCodec(extension);
 
         // Write out
         pCodec->codeToFile(chunk, filename, &imgData);
 
         delete [] pBuffer;
		 pTempSurf->Release();
 
     }

}


#include "OgreD3D8RenderWindow.h"

#include "OgreLogManager.h"
#include "OgreViewport.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

#include "OgreNoMemoryMacros.h"
#include <d3d8.h>
#include "OgreMemoryMacros.h"
#include "dxutil.h"
#include "OgreBitwise.h"
#include "OgreImageCodec.h"

namespace Ogre
{

	// Window procedure callback
	// This is a static member, so applies to all windows but we store the
	// D3D8RenderWindow instance in the window data GetWindowLog/SetWindowLog
	LRESULT D3D8RenderWindow::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		LPCREATESTRUCT lpcs;
		D3D8RenderWindow* win;

		// look up window instance
		if( WM_CREATE != uMsg )
		{
			// Get window pointer
			win = (D3D8RenderWindow*)GetWindowLong( hWnd, 0 );
		}

		switch( uMsg )
		{
		case WM_ACTIVATE:
			if( WA_INACTIVE == LOWORD( wParam ) )
				win->mActive = false;
			else
				win->mActive = true;
			break;

		case WM_CREATE:
			// Log the new window
			// Get CREATESTRUCT
			lpcs = (LPCREATESTRUCT)lParam;
			win = (D3D8RenderWindow*)(lpcs->lpCreateParams);
			// Store pointer in window user data area
			SetWindowLong( hWnd, 0, (long)win );
			win->mActive = true;

			return 0;
			break;

		case WM_KEYDOWN:
            // TEMPORARY CODE
            // TODO - queue up keydown / keyup events with
            //  window name and timestamp to be processed
            //  by main loop

            // ESCAPE closes window
            /*
            if (wParam == VK_ESCAPE)
            {
                win->mClosed = true;
                return 0L;
            }
            */
            break;

		case WM_PAINT:
			// If we get WM_PAINT messges, it usually means our window was
			// comvered up, so we need to refresh it by re-showing the contents
			// of the current frame.
			if( win->mActive && win->mReady )
				win->update();
			break;

		case WM_MOVE:
			// Move messages need to be tracked to update the screen rects
			// used for blitting the backbuffer to the primary
			// *** This doesn't need to be used to Direct3D8 ***
			break;

		case WM_ENTERSIZEMOVE:
			// Previent rendering while moving / sizing
			win->mReady = false;
			break;

		case WM_EXITSIZEMOVE:
			win->WindowMovedOrResized();
			win->mReady = true;
			break;

		case WM_SIZE:
			// Check to see if we are losing or gaining our window.  Set the 
			// active flag to match
			if( SIZE_MAXHIDE == wParam || SIZE_MINIMIZED == wParam )
				win->mActive = false;
			else
			{
				win->mActive = true;
				if( win->mReady )
					win->WindowMovedOrResized();
			}
			break;

		case WM_GETMINMAXINFO:
			// Prevent the window from going smaller than some minimu size
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
			break;

		case WM_CLOSE:
            DestroyWindow( win->mHWnd );
            Root::getSingleton().getRenderSystem()->detachRenderTarget( win->getName() );
            win->mClosed = true;
            return 0;
		}

		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}

	D3D8RenderWindow::D3D8RenderWindow()
	{
		mIsFullScreen = false;
		mpD3DDriver= NULL;
		mpD3DDevice = NULL;
		mHWnd = 0;
		mActive = false;
		mReady = false;
		mClosed = false;
	}

	D3D8RenderWindow::~D3D8RenderWindow()
	{
		SAFE_RELEASE( mpD3DDevice );
	}

	void D3D8RenderWindow::create( String name, int width, int height, int colourDepth, 
		bool fullScreen, int left, int top, bool depthBuffer, void* miscParam, ... )
	{
		HWND parentHWnd;
		HINSTANCE hInst;
		D3D8Driver* driver;
		long tempPtr;

		// Get cariable-length params
		// miscParam[0] = HINSTANCE
		// miscParam[1] = D3D8Driver
		// miscParam[2] = parent HWND
		va_list marker;
		va_start( marker, depthBuffer );

		tempPtr = va_arg( marker, long );
		hInst = *(HINSTANCE*)tempPtr;

		tempPtr = va_arg( marker, long );
		driver = (D3D8Driver*)tempPtr;

		tempPtr = va_arg( marker, long );
		D3D8RenderWindow* parentRW = (D3D8RenderWindow*)tempPtr;
		if( parentRW == NULL )
			parentHWnd = 0;
		else
			parentHWnd = parentRW->getWindowHandle();

		va_end( marker );

		// Destroy current window if any
		if( mHWnd )
			destroy();

		mParentHWnd = parentHWnd;

		// Register the window class
		// NB allow 4 bytes of window data for D3D8RenderWindow pointer
		WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW, WndProc, 0, 4, hInst,
			LoadIcon( NULL, "IDI_ICON1" ),
			LoadCursor( NULL, IDC_ARROW ),
			(HBRUSH)GetStockObject( WHITE_BRUSH ), NULL,
			TEXT(name.c_str()) };
		RegisterClass( &wndClass );

		// Create our main window
		// Pass pointer to self
		HWND hWnd = CreateWindow( TEXT(name.c_str()), TEXT(name.c_str()),
			WS_OVERLAPPEDWINDOW, left, top, width, height, 0L, 0L, hInst, this );
		ShowWindow( hWnd, SW_SHOWNORMAL );
		UpdateWindow( hWnd );

		mHWnd = hWnd;

		// store info
		mName = name;
		mWidth = width;
		mHeight = height;
		mIsDepthBuffered = depthBuffer;
		mIsFullScreen = fullScreen;
		if( fullScreen )
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
		}

		LogManager::getSingleton().logMessage(
			LML_NORMAL, "Created D3D8 Rendering Window '%s' : %i x %i @ %ibpp",
			mName.c_str(), mWidth, mHeight, mColourDepth );

		if( driver && mParentHWnd == NULL )
		{
			mpD3DDriver = driver;
			HRESULT hr;
			LPDIRECT3D8 pD3D = mpD3DDriver->getD3D();

			ZeroMemory( &md3dpp, sizeof(D3DPRESENT_PARAMETERS) );
			md3dpp.Windowed					= !fullScreen;
			md3dpp.BackBufferCount			= 1;
			md3dpp.MultiSampleType			= D3DMULTISAMPLE_NONE ;
			md3dpp.SwapEffect				= D3DSWAPEFFECT_DISCARD;
			md3dpp.EnableAutoDepthStencil	= depthBuffer;
			md3dpp.hDeviceWindow			= mHWnd;
			md3dpp.BackBufferWidth			= mWidth;
			md3dpp.BackBufferHeight			= mHeight;

			// Don't wait for vsync in fullscreen mode
			if (fullScreen)
			{
				md3dpp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
			}

			if( !fullScreen )
				md3dpp.BackBufferFormat		= mpD3DDriver->getDesktopMode().Format;
			else
			{
				md3dpp.BackBufferFormat		= D3DFMT_R5G6B5;
				if( mColourDepth > 16 )
					md3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
			}
            // Depth-stencil format
            if (mColourDepth > 16 )
            {
                // Try to create a 32-bit depth, 8-bit stencil
                if( FAILED( pD3D->CheckDeviceFormat(mpD3DDriver->getAdapterNumber(),
                     D3DDEVTYPE_HAL,  md3dpp.BackBufferFormat,  D3DUSAGE_DEPTHSTENCIL, 
                     D3DRTYPE_SURFACE, D3DFMT_D24S8 )))
                {
                    // Bugger, no 8-bit hardware stencil, just try 32-bit zbuffer 
                    if( FAILED( pD3D->CheckDeviceFormat(mpD3DDriver->getAdapterNumber(),
                        D3DDEVTYPE_HAL,  md3dpp.BackBufferFormat,  D3DUSAGE_DEPTHSTENCIL, 
                        D3DRTYPE_SURFACE, D3DFMT_D32 )))
                    {
                        // Jeez, what a naff card. Fall back on 16-bit depth buffering
                        md3dpp.AutoDepthStencilFormat = D3DFMT_D16;
                    }
                    else
                    {
                        md3dpp.AutoDepthStencilFormat = D3DFMT_D32;
                    }

                }
                else
                {
                    // Woohoo!

					if( SUCCEEDED( pD3D->CheckDepthStencilMatch( mpD3DDriver->getAdapterNumber(), D3DDEVTYPE_HAL,
						md3dpp.BackBufferFormat, md3dpp.BackBufferFormat, D3DFMT_D24X8 ) ) )
					{
						md3dpp.AutoDepthStencilFormat = D3DFMT_D24X8; 
					} 
					else 
					{ 
						md3dpp.AutoDepthStencilFormat = D3DFMT_D24S8; 
					} 
				}



            }
            else
            {
                // 16-bit depth, software stencil
                md3dpp.AutoDepthStencilFormat	= D3DFMT_D16;
            }

			hr = pD3D->CreateDevice( mpD3DDriver->getAdapterNumber(), D3DDEVTYPE_HAL, mHWnd,
				D3DCREATE_HARDWARE_VERTEXPROCESSING, &md3dpp, &mpD3DDevice );
			if( SUCCEEDED( hr ) )
			{
				OutputDebugStr( "Created Direct3D device using hardware vertex processing" );

                mpD3DDevice->GetRenderTarget( &mpRenderSurface );
                mpD3DDevice->GetDepthStencilSurface( &mpRenderZBuffer );
			}
			else
			{
				hr = pD3D->CreateDevice( mpD3DDriver->getAdapterNumber(), D3DDEVTYPE_HAL, mHWnd,
					D3DCREATE_MIXED_VERTEXPROCESSING, &md3dpp, &mpD3DDevice );
				if( SUCCEEDED( hr ) )
				{
					OutputDebugStr( "Created Direct3D device using mixed vertex processing" );

                    mpD3DDevice->GetRenderTarget( &mpRenderSurface );
                    mpD3DDevice->GetDepthStencilSurface( &mpRenderZBuffer );
				}
				else
				{
					hr = pD3D->CreateDevice( mpD3DDriver->getAdapterNumber(), D3DDEVTYPE_HAL, mHWnd,
						D3DCREATE_SOFTWARE_VERTEXPROCESSING, &md3dpp, &mpD3DDevice );
					if( SUCCEEDED( hr ) )
					{
						OutputDebugString( "Created Direct3D device using software vertex processing" );

                        mpD3DDevice->GetRenderTarget( &mpRenderSurface );
                        mpD3DDevice->GetDepthStencilSurface( &mpRenderZBuffer );
					}
				}
			}

			// TODO: make this a bit better e.g. go from pure vertex processing to software
			if( FAILED( hr ) )
				Except( hr, "Failed to create Direct3D8 Device", "D3D8RenderWindow::create" );

				
		}
		else
			mpD3DDevice = NULL;

		mReady = true;
	}

	void D3D8RenderWindow::destroy()
	{
		SAFE_RELEASE( mpD3DDevice );

		DestroyWindow( mHWnd );
	}

	void D3D8RenderWindow::resize( int width, int height )
	{
		mWidth = width;
		mHeight = height;

		// Notify viewports of resize
		ViewportList::iterator it = mViewportList.begin();
		while( it != mViewportList.end() )
			(*it).second->_updateDimensions();

		// TODO - resize window
	}

	void D3D8RenderWindow::swapBuffers( bool waitForVSync )
	{
		if( mpD3DDevice )
		{

			HRESULT hr = mpD3DDevice->Present( NULL, NULL, 0, NULL );
			if( D3DERR_DEVICELOST == hr )
			{
				// TODO: Restore surfaces
				// restoreD3DSurfaces();
			}
			else if( FAILED(hr) )
			{
				Except( hr, "Error Presenting surfaces", "D3D8RenderWindow::swapBuffers" );
			}
		}
	}

	void D3D8RenderWindow::getCustomAttribute( String name, void* pData )
	{
		// Valid attributes and their equvalent native functions:
		// D3DDEVICE			: getD3DDevice
		// HWND					: getWindowHandle

		if( name == "D3DDEVICE" )
		{
			LPDIRECT3DDEVICE8 *pDev = (LPDIRECT3DDEVICE8*)pData;

			*pDev = getD3DDevice();
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
        else if( name == "D3DZBUFFER" )
        {
            LPDIRECT3DSURFACE8 *pSurf = (LPDIRECT3DSURFACE8*)pData;

            *pSurf = mpRenderZBuffer;
            return;
        }
        else if( name == "DDBACKBUFFER" )
        {
            LPDIRECT3DSURFACE8 *pSurf = (LPDIRECT3DSURFACE8*)pData;

            *pSurf = mpRenderSurface;
            return;
        }
        else if( name == "DDFRONTBUFFER" )
        {
            LPDIRECT3DSURFACE8 *pSurf = (LPDIRECT3DSURFACE8*)pData;

            *pSurf = mpRenderSurface;
            return;
        }
	}

	void D3D8RenderWindow::outputText( int x, int y, const String& text )
	{
        // Deprecated
	}

	void D3D8RenderWindow::WindowMovedOrResized()
	{
		// TODO
	}
 
     void D3D8RenderWindow::writeContentsToFile(const String& filename)
     {
         HRESULT hr;
         LPDIRECT3DSURFACE8 pSurf=NULL, pTempSurf=NULL;
         D3DSURFACE_DESC desc;
                D3DDISPLAYMODE dm;

                // get display dimensions
                // this will be the dimensions of the front buffer
                mpD3DDevice->GetDisplayMode(&dm);

                desc.Width = dm.Width;
                desc.Height = dm.Height;
                desc.Format = D3DFMT_A8R8G8B8;
                if (FAILED(hr = mpD3DDevice->CreateImageSurface(desc.Width, desc.Height, desc.Format, &pTempSurf)))
                {
                        Except(hr, "Cannot create offscreen buffer!",
                                "D3D8RenderWindow::writeContentsToFile");
                }

                if (FAILED(hr = mpD3DDevice->GetFrontBuffer(pTempSurf)))
                {
                        SAFE_RELEASE(pTempSurf);
                        Except(hr, "Cannot copy front buffer!",
                                "D3D8RenderWindow::writeContentsToFile");
                }

                if (!mIsFullScreen)
                {
                        D3DVIEWPORT8 vp;

                        mpD3DDevice->GetViewport(&vp);

                        desc.Width = vp.Width;
                        desc.Height = vp.Height;
                        desc.Format = D3DFMT_A8R8G8B8;         // this is what we get from the screen, so stick with it
 
                        // NB we can't lock the back buffer direct because it's no created that way
                        // and to do so hits performance, so copy to another surface
                        // Must be the same format as the source surface
                        if (FAILED(hr = mpD3DDevice->CreateImageSurface(desc.Width, desc.Height, desc.Format, &pSurf)))
                        {
                                SAFE_RELEASE(pSurf);
                                Except(hr, "Cannot create offscreen buffer!",
                                        "D3D8RenderWindow::writeContentsToFile");
                        }

                        POINT pt={0, 0};
                        RECT srcRect;

                        srcRect.left = vp.X;
                        srcRect.top = vp.Y;
                        srcRect.right = srcRect.left + vp.Width;
                        srcRect.bottom = srcRect.top + vp.Height;
 
                        // Copy
                        if (FAILED(hr = mpD3DDevice->CopyRects(pTempSurf, &srcRect, 1, pSurf, &pt)))
                        {
                                SAFE_RELEASE(pTempSurf);
                                SAFE_RELEASE(pSurf);
                                Except(hr, "Cannot copy back buffer!",
                                        "D3D8RenderWindow::writeContentsToFile");
                        }
                        SAFE_RELEASE(pTempSurf);
                        pTempSurf = pSurf;
                        pSurf = NULL;
                }
  
         D3DLOCKED_RECT lockedRect;

         if (FAILED(hr = pTempSurf->LockRect(&lockedRect, NULL, 
             D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK | D3DLOCK_NO_DIRTY_UPDATE)))
         {
             Except(hr, String("Cannot lock surface: ") + DXGetErrorDescription8( hr ),
                 "D3D8RenderWindow::writeContentsToFile");
         } 
 
         ImageCodec::ImageData imgData;
         imgData.width = desc.Width;
         imgData.height = desc.Height;
         imgData.format = PF_R8G8B8;
 
         // Allocate contiguous buffer (surfaces aren't necessarily contiguous)
         uchar* pBuffer = new uchar[desc.Width * desc.Height * 3];
 
         uint x, y;
         uchar *pData, *pDest;
 
         pData = (uchar*)lockedRect.pBits;
         pDest = pBuffer;
         for (y = 0; y < desc.Height; ++y)
         {
             uchar *pRow = pData;
 
             for (x = 0; x < desc.Width; ++x)
             {
                 switch(desc.Format)
                 {
                 case D3DFMT_R5G6B5:
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
                     break;
                 case D3DFMT_A8R8G8B8:
                 case D3DFMT_X8R8G8B8:
					 // Actual format is BRGA for some reason
                     *pDest++ = pRow[2]; // R
                     *pDest++ = pRow[1]; // G
                     *pDest++ = pRow[0]; // B
                     pRow += 4; // skip alpha / dummy
                     break;
				 case D3DFMT_R8G8B8:
					 // Actual format is BRGA for some reason
                     *pDest++ = pRow[2]; // R
                     *pDest++ = pRow[1]; // G
                     *pDest++ = pRow[0]; // B
                     pRow += 3; 
                     break;
                 }
 
                 
             }
             // increase by one line
             pData += lockedRect.Pitch;
 
         }
 
 
         // Wrap buffer in a chunk
         DataChunk chunk(pBuffer, desc.Width * desc.Height * 3);
 
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
 
         SAFE_RELEASE(pTempSurf);
         SAFE_RELEASE(pSurf);
 
     }
}
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
#include "OgreD3D9RenderWindow.h"
#include "OgreLogManager.h"
#include "OgreViewport.h"
#include "OgreException.h"
#include "OgreRenderSystem.h"
#include "OgreBitwise.h"
#include "OgreImageCodec.h"

#include "OgreNoMemoryMacros.h"
#include <d3d9.h>
#include "OgreMemoryMacros.h"
#include "OgreRoot.h"

namespace Ogre
{
	// Window procedure callback
	// This is a static member, so applies to all windows but we store the
	// D3D9RenderWindow instance in the window data GetWindowLog/SetWindowLog
	LRESULT D3D9RenderWindow::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		LPCREATESTRUCT lpcs;
		D3D9RenderWindow* win;

		// look up window instance
		if( WM_CREATE != uMsg )
			// Get window pointer
			win = (D3D9RenderWindow*)GetWindowLong( hWnd, 0 );

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
			win = (D3D9RenderWindow*)(lpcs->lpCreateParams);
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
			// *** This doesn't need to be used to Direct3D9 ***
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

	D3D9RenderWindow::D3D9RenderWindow()
	{
		mIsFullScreen = false;
		mpD3DDriver= NULL;
		mpD3DDevice = NULL;
		mHWnd = 0;
		mActive = false;
		mReady = false;
		mClosed = false;
		mExternalHandle = NULL;
	}

	D3D9RenderWindow::~D3D9RenderWindow()
	{
		SAFE_RELEASE( mpRenderSurface );
		SAFE_RELEASE( mpRenderZBuffer );
		SAFE_RELEASE( mpD3DDevice );
	}

	bool D3D9RenderWindow::_checkMultiSampleQuality(D3DMULTISAMPLE_TYPE type, DWORD *outQuality, D3DFORMAT format, UINT adapterNum, D3DDEVTYPE deviceType, BOOL fullScreen)
	{
		LPDIRECT3D9 pD3D = mpD3DDriver->getD3D();

		if (SUCCEEDED(pD3D->CheckDeviceMultiSampleType(
				adapterNum, 
				deviceType, format, 
				fullScreen, type, outQuality)))
			return true;
		else
			return false;
	}

	void D3D9RenderWindow::create( const String& name, unsigned int width, unsigned int height, unsigned int colourDepth, 
		bool fullScreen, int left, int top, bool depthBuffer, void* miscParam, ... )
	{
		HWND parentHWnd;
		HINSTANCE hInst;
		D3D9Driver* driver;
		long tempPtr;
		
		D3DMULTISAMPLE_TYPE mFSAAType = D3DMULTISAMPLE_NONE;
		DWORD mFSAAQuality = 0;
		// Get variable-length params
		// miscParam[0] = HINSTANCE
		// miscParam[1] = D3D9Driver
		// miscParam[2] = parent HWND
		// miscParam[3] = multisample type
		// miscParam[4] = multisample quality
		// miscParam[5] = vsync

		va_list marker;
		va_start( marker, depthBuffer );

		tempPtr = va_arg( marker, long );
		hInst = *(HINSTANCE*)tempPtr;

		tempPtr = va_arg( marker, long );
		driver = (D3D9Driver*)tempPtr;

		tempPtr = va_arg( marker, long );
		D3D9RenderWindow* parentRW = (D3D9RenderWindow*)tempPtr;
		if( parentRW == NULL )
			parentHWnd = 0;
		else
			parentHWnd = parentRW->getWindowHandle();

		tempPtr = va_arg( marker, long );
		mFSAAType = (D3DMULTISAMPLE_TYPE)tempPtr;

		tempPtr = va_arg( marker, long );
		mFSAAQuality = (DWORD)tempPtr;

		tempPtr = va_arg( marker, long );
		bool vsync = tempPtr ? true : false;

		va_end( marker );

		// Destroy current window if any
		if( mHWnd )
			destroy();

		// track the parent window handle
		mParentHWnd = parentHWnd;

		if (!mExternalHandle)
		{
			mWidth = width;
			mHeight = height;
			if (!fullScreen)
			{
				if (!left && (unsigned)GetSystemMetrics(SM_CXSCREEN) > mWidth)
					mLeft = (GetSystemMetrics(SM_CXSCREEN) / 2) - (mWidth / 2);
				else
					mLeft = left;
				if (!top && (unsigned)GetSystemMetrics(SM_CYSCREEN) > mHeight)
					mTop = (GetSystemMetrics(SM_CYSCREEN) / 2) - (mHeight / 2);
				else
					mTop = top;
			}
			else
				mTop = mLeft = 0;

			// Register the window class
			// NB allow 4 bytes of window data for D3D9RenderWindow pointer
            WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW, WndProc, 0, 4, hInst,
				LoadIcon( NULL, IDI_APPLICATION ),
				LoadCursor( NULL, IDC_ARROW ),
				(HBRUSH)GetStockObject( BLACK_BRUSH ), NULL,
				TEXT(name.c_str()) };
			RegisterClass( &wndClass );

			// Create our main window
			// Pass pointer to self
			HWND hWnd = CreateWindow(TEXT(name.c_str()),
									 TEXT(name.c_str()),
									 WS_OVERLAPPEDWINDOW, mLeft, mTop,
									 mWidth, mHeight, 0L, 0L, hInst, this);
			ShowWindow(hWnd, SW_SHOWNORMAL);
			UpdateWindow(hWnd);

			mHWnd = hWnd;
			// Store info
			mName = name;
			mIsDepthBuffered = depthBuffer;
			mIsFullScreen = fullScreen;
		}
		else
		{
			mHWnd = mExternalHandle;
			ShowWindow(mHWnd, SW_SHOWNORMAL);
			UpdateWindow(mHWnd);
			RECT rc;
			GetClientRect(mHWnd,&rc);
			mWidth = rc.right;
			mHeight = rc.bottom;
			mLeft = rc.left;
			mTop = rc.top;
			mName = name;
			mIsDepthBuffered = depthBuffer;
			mIsFullScreen = fullScreen;
		}

		// track colour depth
		mColourDepth = colourDepth;

		LogManager::getSingleton().logMessage(
			LML_NORMAL, "D3D9 : Created D3D9 Rendering Window '%s' : %ix%i, %ibpp",
			mName.c_str(), mWidth, mHeight, mColourDepth );

		if( driver && mParentHWnd == NULL )
		{
			mpD3DDriver = driver;
			HRESULT hr;
			LPDIRECT3D9 pD3D = mpD3DDriver->getD3D();
			D3DDEVTYPE devType = D3DDEVTYPE_HAL;

			ZeroMemory( &md3dpp, sizeof(D3DPRESENT_PARAMETERS) );
			md3dpp.Windowed					= !fullScreen;
			md3dpp.SwapEffect				= D3DSWAPEFFECT_DISCARD;
			md3dpp.BackBufferCount			= 1;
			md3dpp.EnableAutoDepthStencil	= depthBuffer;
			md3dpp.hDeviceWindow			= mHWnd;
			md3dpp.BackBufferWidth			= mWidth;
			md3dpp.BackBufferHeight			= mHeight;

			if (vsync)
				md3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
			else
				md3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

			md3dpp.BackBufferFormat		= D3DFMT_R5G6B5;
			if( mColourDepth > 16 )
				md3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;

			if (mColourDepth > 16 )
			{
				// Try to create a 32-bit depth, 8-bit stencil
				if( FAILED( pD3D->CheckDeviceFormat(mpD3DDriver->getAdapterNumber(),
					devType,  md3dpp.BackBufferFormat,  D3DUSAGE_DEPTHSTENCIL, 
					D3DRTYPE_SURFACE, D3DFMT_D24S8 )))
				{
					// Bugger, no 8-bit hardware stencil, just try 32-bit zbuffer 
					if( FAILED( pD3D->CheckDeviceFormat(mpD3DDriver->getAdapterNumber(),
						devType,  md3dpp.BackBufferFormat,  D3DUSAGE_DEPTHSTENCIL, 
						D3DRTYPE_SURFACE, D3DFMT_D32 )))
					{
						// Jeez, what a naff card. Fall back on 16-bit depth buffering
						md3dpp.AutoDepthStencilFormat = D3DFMT_D16;
					}
					else
						md3dpp.AutoDepthStencilFormat = D3DFMT_D32;
				}
				else
				{
					// Woohoo!
					if( SUCCEEDED( pD3D->CheckDepthStencilMatch( mpD3DDriver->getAdapterNumber(), devType,
						md3dpp.BackBufferFormat, md3dpp.BackBufferFormat, D3DFMT_D24S8 ) ) )
					{
						md3dpp.AutoDepthStencilFormat = D3DFMT_D24S8; 
					} 
					else 
						md3dpp.AutoDepthStencilFormat = D3DFMT_D24X8; 
				}
			}
			else
				// 16-bit depth, software stencil
				md3dpp.AutoDepthStencilFormat	= D3DFMT_D16;

			md3dpp.MultiSampleType = mFSAAType;
			md3dpp.MultiSampleQuality = (mFSAAQuality == 0) ? NULL : mFSAAQuality;

			hr = pD3D->CreateDevice( mpD3DDriver->getAdapterNumber(), devType, mHWnd,
				D3DCREATE_HARDWARE_VERTEXPROCESSING, &md3dpp, &mpD3DDevice );
			if( SUCCEEDED( hr ) )
			{
				mpD3DDevice->GetRenderTarget( 0, &mpRenderSurface );
				mpD3DDevice->GetDepthStencilSurface( &mpRenderZBuffer );
			}
			else
			{
				hr = pD3D->CreateDevice( mpD3DDriver->getAdapterNumber(), devType, mHWnd,
					D3DCREATE_MIXED_VERTEXPROCESSING, &md3dpp, &mpD3DDevice );
				if( SUCCEEDED( hr ) )
				{
					mpD3DDevice->GetRenderTarget( 0, &mpRenderSurface );
					mpD3DDevice->GetDepthStencilSurface( &mpRenderZBuffer );
				}
				else
				{
					hr = pD3D->CreateDevice( mpD3DDriver->getAdapterNumber(), devType, mHWnd,
						D3DCREATE_SOFTWARE_VERTEXPROCESSING, &md3dpp, &mpD3DDevice );
					if( SUCCEEDED( hr ) )
					{
						mpD3DDevice->GetRenderTarget( 0, &mpRenderSurface );
						mpD3DDevice->GetDepthStencilSurface( &mpRenderZBuffer );
					}
				}
			}

			// TODO: make this a bit better e.g. go from pure vertex processing to software
			if( FAILED( hr ) )
            {
                destroy();
                Except( hr, "Failed to create Direct3D9 Device: " + 
                    Root::getSingleton().getErrorDescription(hr), 
                    "D3D9RenderWindow::create" );
            }
		}
		else
			mpD3DDevice = NULL;

		mReady = true;
	}

	void D3D9RenderWindow::destroy()
	{
		SAFE_RELEASE( mpRenderSurface );
		SAFE_RELEASE( mpRenderZBuffer );
		SAFE_RELEASE( mpD3DDevice );
		SAFE_RELEASE(mpRenderSurface);
		SAFE_RELEASE(mpRenderZBuffer);
		DestroyWindow( mHWnd );
	}

	void D3D9RenderWindow::resize( unsigned int width, unsigned int height )
	{
		mWidth = width;
		mHeight = height;

		// Notify viewports of resize
		ViewportList::iterator it = mViewportList.begin();
		while( it != mViewportList.end() )
			(*it++).second->_updateDimensions();
		// TODO - resize window
	}

	void D3D9RenderWindow::swapBuffers( bool waitForVSync )
	{
		if( mpD3DDevice )
		{
			HRESULT hr = mpD3DDevice->Present( NULL, NULL, 0, NULL );
			if( D3DERR_DEVICELOST == hr )
				// TODO: Restore surfaces
				// restoreD3DSurfaces();
				Except( hr, "Device lost and not restored", "D3D9RenderWindow::swapBuffers" );
			else if( FAILED(hr) )
				Except( hr, "Error Presenting surfaces", "D3D9RenderWindow::swapBuffers" );
		}
	}

	void D3D9RenderWindow::getCustomAttribute( const String& name, void* pData )
	{
		// Valid attributes and their equvalent native functions:
		// D3DDEVICE			: getD3DDevice
		// HWND					: getWindowHandle

		if( name == "D3DDEVICE" )
		{
			LPDIRECT3DDEVICE9 *pDev = (LPDIRECT3DDEVICE9*)pData;
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
			LPDIRECT3DSURFACE9 *pSurf = (LPDIRECT3DSURFACE9*)pData;
			*pSurf = mpRenderZBuffer;
			return;
		}
		else if( name == "DDBACKBUFFER" )
		{
			LPDIRECT3DSURFACE9 *pSurf = (LPDIRECT3DSURFACE9*)pData;
			*pSurf = mpRenderSurface;
			return;
		}
		else if( name == "DDFRONTBUFFER" )
		{
			LPDIRECT3DSURFACE9 *pSurf = (LPDIRECT3DSURFACE9*)pData;
			*pSurf = mpRenderSurface;
			return;
		}
	}

	void D3D9RenderWindow::outputText( int x, int y, const String& text )
	{
		// Deprecated
	}

	void D3D9RenderWindow::WindowMovedOrResized()
	{
		// TODO
	}

	void D3D9RenderWindow::writeContentsToFile(const String& filename)
	{
		HRESULT hr;
		LPDIRECT3DSURFACE9 pSurf=NULL, pTempSurf=NULL;
		D3DSURFACE_DESC desc;
		D3DDISPLAYMODE dm;

		// get display dimensions
		// this will be the dimensions of the front buffer
		if (FAILED(hr = mpD3DDevice->GetDisplayMode(0, &dm)))
			Except(hr, "Can't get display mode!", "D3D9RenderWindow::writeContentsToFile");

		desc.Width = dm.Width;
		desc.Height = dm.Height;
		desc.Format = D3DFMT_A8R8G8B8;
		if (FAILED(hr = mpD3DDevice->CreateOffscreenPlainSurface(
						desc.Width, 
						desc.Height, 
						desc.Format, 
						D3DPOOL_SYSTEMMEM, 
						&pTempSurf, 
						NULL)))
		{
			Except(hr, "Cannot create offscreen buffer 1!", "D3D9RenderWindow::writeContentsToFile");
		}

		if (FAILED(hr = mpD3DDevice->GetFrontBufferData(0, pTempSurf)))
		{
			SAFE_RELEASE(pTempSurf);
			Except(hr, "Can't get front buffer!", "D3D9RenderWindow::writeContentsToFile");
		}

		if (!mIsFullScreen)
		{
			POINT pt={0, 0};
			RECT srcRect;
			GetWindowRect(mHWnd, &srcRect);

			desc.Width = srcRect.right - srcRect.left;
			desc.Height = srcRect.bottom - srcRect.top;
			desc.Format = D3DFMT_A8R8G8B8;         // this is what we get from the screen, so stick with it

			// NB we can't lock the back buffer direct because it's no created that way
			// and to do so hits performance, so copy to another surface
			// Must be the same format as the source surface
			if (FAILED(hr = mpD3DDevice->CreateOffscreenPlainSurface(
							desc.Width, 
							desc.Height, 
							desc.Format, 
							D3DPOOL_DEFAULT, 
							&pSurf,
							NULL)))
			{
				SAFE_RELEASE(pSurf);
				Except(hr, "Cannot create offscreen buffer 2!", "D3D9RenderWindow::writeContentsToFile");
			}

			// Copy
			if (FAILED(hr = mpD3DDevice->UpdateSurface(pTempSurf, &srcRect, pSurf, &pt)))
			{
				SAFE_RELEASE(pTempSurf);
				SAFE_RELEASE(pSurf);
				Except(hr, "Cannot update surface!", "D3D9RenderWindow::writeContentsToFile");
			}

			SAFE_RELEASE(pTempSurf);
			pTempSurf = pSurf;
			pSurf = NULL;
		}

		D3DLOCKED_RECT lockedRect;
		if (FAILED(hr = pTempSurf->LockRect(&lockedRect, NULL, 
			D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK)))
		{
			Except(hr, "can't lock rect!", "D3D9RenderWindow::writeContentsToFile");
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

					val = *((WORD*)pRow);
					pRow += 2;

					*pDest++ = Bitwise::convertBitPattern((WORD)val, (WORD)0xF800, (BYTE)0xFF);
					*pDest++ = Bitwise::convertBitPattern((WORD)val, (WORD)0x07E0, (BYTE)0xFF);
					*pDest++ = Bitwise::convertBitPattern((WORD)val, (WORD)0x001F, (BYTE)0xFF);
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
			"D3D9RenderWindow::writeContentsToFile" );

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

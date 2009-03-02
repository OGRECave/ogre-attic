/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/
#include "OgreD3D9RenderWindow.h"
#include "OgreLogManager.h"
#include "OgreViewport.h"
#include "OgreException.h"
#include "OgreD3D9RenderSystem.h"
#include "OgreRenderSystem.h"
#include "OgreBitwise.h"
#include "OgreImageCodec.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"
#include "OgreWindowEventUtilities.h"
#include "OgreD3D9DeviceManager.h"

namespace Ogre
{
	D3D9RenderWindow::D3D9RenderWindow(HINSTANCE instance)
        : mInstance(instance)        
	{
		mDevice = NULL;
		mIsFullScreen = false;		
		mIsExternal = false;
		mHWnd = 0;
		mActive = false;		
		mClosed = false;
		mSwitchingFullscreen = false;
		mDisplayFrequency = 0;
		mDeviceValid = false;
		mUseNVPerfHUD = false;
	}

	D3D9RenderWindow::~D3D9RenderWindow()
	{
		destroy();
	}

	void D3D9RenderWindow::create(const String& name, unsigned int width, unsigned int height,
		bool fullScreen, const NameValuePairList *miscParams)
	{
		HINSTANCE hInst = mInstance;
	
		HWND parentHWnd = 0;
		HWND externalHandle = 0;
		mFSAAType = D3DMULTISAMPLE_NONE;
		mFSAAQuality = 0;
		mFSAA = 0;
		mVSync = false;
		String title = name;
		unsigned int colourDepth = 32;
		int left = -1; // Defaults to screen center
		int top = -1; // Defaults to screen center
		bool depthBuffer = true;
		String border = "";
		bool outerSize = false;
		mUseNVPerfHUD = false;
		size_t fsaaSamples = 0;
		String fsaaHint;
		int monitorIndex = -1;	//Default by detecting the adapter from left / top position

		if(miscParams)
		{
			// Get variable-length params
			NameValuePairList::const_iterator opt;
			// left (x)
			opt = miscParams->find("left");
			if(opt != miscParams->end())
				left = StringConverter::parseInt(opt->second);
			// top (y)
			opt = miscParams->find("top");
			if(opt != miscParams->end())
				top = StringConverter::parseInt(opt->second);
			// Window title
			opt = miscParams->find("title");
			if(opt != miscParams->end())
				title = opt->second;
			// parentWindowHandle		-> parentHWnd
			opt = miscParams->find("parentWindowHandle");
			if(opt != miscParams->end())
				parentHWnd = (HWND)StringConverter::parseUnsignedInt(opt->second);
			// externalWindowHandle		-> externalHandle
			opt = miscParams->find("externalWindowHandle");
			if(opt != miscParams->end())
				externalHandle = (HWND)StringConverter::parseUnsignedInt(opt->second);
			// vsync	[parseBool]
			opt = miscParams->find("vsync");
			if(opt != miscParams->end())
				mVSync = StringConverter::parseBool(opt->second);
			// displayFrequency
			opt = miscParams->find("displayFrequency");
			if(opt != miscParams->end())
				mDisplayFrequency = StringConverter::parseUnsignedInt(opt->second);
			// colourDepth
			opt = miscParams->find("colourDepth");
			if(opt != miscParams->end())
				colourDepth = StringConverter::parseUnsignedInt(opt->second);
			// depthBuffer [parseBool]
			opt = miscParams->find("depthBuffer");
			if(opt != miscParams->end())
				depthBuffer = StringConverter::parseBool(opt->second);
			// FSAA settings
			opt = miscParams->find("FSAA");
			if(opt != miscParams->end())
			{
				mFSAA = StringConverter::parseUnsignedInt(opt->second);
			}
			opt = miscParams->find("FSAAHint");
			if(opt != miscParams->end())
			{
				mFSAAHint = opt->second;
			}

			// window border style
			opt = miscParams->find("border");
			if(opt != miscParams->end())
				border = opt->second;
			// set outer dimensions?
			opt = miscParams->find("outerDimensions");
			if(opt != miscParams->end())
				outerSize = StringConverter::parseBool(opt->second);
			// NV perf HUD?
			opt = miscParams->find("useNVPerfHUD");
			if(opt != miscParams->end())
				mUseNVPerfHUD = StringConverter::parseBool(opt->second);
			// sRGB?
			opt = miscParams->find("gamma");
			if(opt != miscParams->end())
				mHwGamma = StringConverter::parseBool(opt->second);
			// monitor index
			opt = miscParams->find("monitorIndex");
			if(opt != miscParams->end())
				monitorIndex = StringConverter::parseInt(opt->second);

		}

		// Destroy current window if any
		if( mHWnd )
			destroy();

		if (!externalHandle)
		{
			DWORD		dwStyle = WS_VISIBLE | WS_CLIPCHILDREN;
			DWORD		dwStyleEx = 0;
			HMONITOR    hMonitor = NULL;		
			MONITORINFO monitorInfo;
			RECT		rc;


			// If we specified which adapter we want to use - find it's monitor.
			if (monitorIndex != -1)
			{
				IDirect3D9* direct3D9 = D3D9RenderSystem::getDirect3D9();

				for (uint i=0; i < direct3D9->GetAdapterCount(); ++i)
				{
					if (i == monitorIndex)
					{
						hMonitor = direct3D9->GetAdapterMonitor(i);
						break;
					}
				}				
			}

			// If we didn't specified the adapter index, or if it didn't find it
			if (hMonitor == NULL)
			{
				POINT windowAnchorPoint;

				// Fill in anchor point.
				windowAnchorPoint.x = left;
				windowAnchorPoint.y = top;


				// Get the nearest monitor to this window.
				hMonitor = MonitorFromPoint(windowAnchorPoint, MONITOR_DEFAULTTONEAREST);
			}

			// Get the target monitor info		
			memset(&monitorInfo, 0, sizeof(MONITORINFO));
			monitorInfo.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(hMonitor, &monitorInfo);


			// No specified top left -> Center the window in the middle of the monitor
			if (left == -1 || top == -1)
			{				
				int screenw = monitorInfo.rcMonitor.right  - monitorInfo.rcMonitor.left;
				int screenh = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;

				SetRect(&rc, 0, 0, width, height);
				AdjustWindowRect(&rc, dwStyle, false);

				// clamp window dimensions to screen size
				int outerw = (rc.right-rc.left < screenw)? rc.right-rc.left : screenw;
				int outerh = (rc.bottom-rc.top < screenh)? rc.bottom-rc.top : screenh;

				if (left == -1)
					left = monitorInfo.rcMonitor.left + (screenw - outerw) / 2;
				else if (monitorIndex != -1)
					left += monitorInfo.rcMonitor.left;

				if (top == -1)
					top = monitorInfo.rcMonitor.top + (screenh - outerh) / 2;
				else if (monitorIndex != -1)
					top += monitorInfo.rcMonitor.top;
			}
			else if (monitorIndex != -1)
			{
				left += monitorInfo.rcMonitor.left;
				top += monitorInfo.rcMonitor.top;
			}

			mWidth = width;
			mHeight = height;
			mTop = top;
			mLeft = left;

			if (fullScreen)
			{
				dwStyleEx |= WS_EX_TOPMOST;
				dwStyle |= WS_POPUP;
				mTop = monitorInfo.rcMonitor.top;
				mLeft = monitorInfo.rcMonitor.left;				
			}
			else
			{
				if (parentHWnd)
				{
					dwStyle |= WS_CHILD;
				}
				else
				{
					if (border == "none")
						dwStyle |= WS_POPUP;
					else if (border == "fixed")
						dwStyle |= WS_OVERLAPPED | WS_BORDER | WS_CAPTION |
						WS_SYSMENU | WS_MINIMIZEBOX;
					else
						dwStyle |= WS_OVERLAPPEDWINDOW;
				}

				if (!outerSize)
				{
					// Calculate window dimensions required
					// to get the requested client area
					SetRect(&rc, 0, 0, mWidth, mHeight);
					AdjustWindowRect(&rc, dwStyle, false);
					mWidth = rc.right - rc.left;
					mHeight = rc.bottom - rc.top;

					// Clamp window rect to the nearest display monitor.
					if (mLeft < monitorInfo.rcMonitor.left)
						mLeft = monitorInfo.rcMonitor.left;		

					if (mTop < monitorInfo.rcMonitor.top)					
						mTop = monitorInfo.rcMonitor.top;					
				
					if (static_cast<int>(mWidth) > monitorInfo.rcMonitor.right - mLeft)					
						mWidth = monitorInfo.rcMonitor.right - mLeft;	

					if (static_cast<int>(mHeight) > monitorInfo.rcMonitor.bottom - mTop)					
						mHeight = monitorInfo.rcMonitor.bottom - mTop;										
				}
			}
			

			// Register the window class
			// NB allow 4 bytes of window data for D3D9RenderWindow pointer
			WNDCLASS wc = { 0, WindowEventUtilities::_WndProc, 0, 0, hInst,
				LoadIcon(0, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
				(HBRUSH)GetStockObject(BLACK_BRUSH), 0, "OgreD3D9Wnd" };
			RegisterClass(&wc);

			// Create our main window
			// Pass pointer to self
			mIsExternal = false;
			mHWnd = CreateWindowEx(dwStyleEx, "OgreD3D9Wnd", title.c_str(), dwStyle,
				mLeft, mTop, mWidth, mHeight, parentHWnd, 0, hInst, this);

			WindowEventUtilities::_addRenderWindow(this);
		}
		else
		{
			mHWnd = externalHandle;
			mIsExternal = true;
		}

		RECT rc;
		// top and left represent outer window coordinates
		GetWindowRect(mHWnd, &rc);
		mTop = rc.top;
		mLeft = rc.left;
		// width and height represent interior drawable area
		GetClientRect(mHWnd, &rc);
		mWidth = rc.right;
		mHeight = rc.bottom;

		mName = name;
		mIsDepthBuffered = depthBuffer;
		mIsFullScreen = fullScreen;
		mColourDepth = colourDepth;

		LogManager::getSingleton().stream()
			<< "D3D9 : Created D3D9 Rendering Window '"
			<< mName << "' : " << mWidth << "x" << mHeight 
			<< ", " << mColourDepth << "bpp";
									
		mActive = true;
		mClosed = false;
	}

	void D3D9RenderWindow::setFullscreen(bool fullScreen, unsigned int width, unsigned int height)
	{
		if (fullScreen != mIsFullScreen || width != mWidth || height != mHeight)
		{
			if (fullScreen != mIsFullScreen)
				mSwitchingFullscreen = true;

			DWORD dwStyle = WS_VISIBLE | WS_CLIPCHILDREN;

			bool oldFullscreen = mIsFullScreen;
			mIsFullScreen = fullScreen;

			if (fullScreen)
			{
				dwStyle |= WS_POPUP;
				
				// Get the nearest monitor to this window.
				HMONITOR hMonitor = MonitorFromWindow(mHWnd, MONITOR_DEFAULTTONEAREST);

				// Get monitor info	
				MONITORINFO monitorInfo;

				memset(&monitorInfo, 0, sizeof(MONITORINFO));
				monitorInfo.cbSize = sizeof(MONITORINFO);
				GetMonitorInfo(hMonitor, &monitorInfo);

				mTop = monitorInfo.rcMonitor.top;
				mLeft = monitorInfo.rcMonitor.left;				
				mWidth = width;
				mHeight = height;
				
				// need different ordering here

				if (oldFullscreen)
				{
					// was previously fullscreen, just changing the resolution
					SetWindowPos(mHWnd, HWND_TOPMOST, mLeft, mTop, width, height, SWP_NOACTIVATE);
				}
				else
				{
					SetWindowPos(mHWnd, HWND_TOPMOST, mLeft, mTop, width, height, SWP_NOACTIVATE);
					//MoveWindow(mHWnd, mLeft, mTop, mWidth, mHeight, FALSE);
					SetWindowLong(mHWnd, GWL_STYLE, dwStyle);
					SetWindowPos(mHWnd, 0, 0,0, 0,0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
				}
			}
			else
			{
				dwStyle |= WS_OVERLAPPEDWINDOW;
				// Calculate window dimensions required
				// to get the requested client area
				RECT rc;
				SetRect(&rc, 0, 0, width, height);
				AdjustWindowRect(&rc, dwStyle, false);
				unsigned int winWidth = rc.right - rc.left;
				unsigned int winHeight = rc.bottom - rc.top;

				SetWindowLong(mHWnd, GWL_STYLE, dwStyle);
				SetWindowPos(mHWnd, HWND_NOTOPMOST, 0, 0, winWidth, winHeight,
					SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOACTIVATE);
				// Note that we also set the position in the restoreLostDevice method
				// via _finishSwitchingFullScreen
			}
						
			// Have to release & trigger device reset
			// NB don't use windowMovedOrResized since Win32 doesn't know
			// about the size change yet				
			mDevice->invalidate();
			// Notify viewports of resize
			ViewportList::iterator it = mViewportList.begin();
			while(it != mViewportList.end()) (*it++).second->_updateDimensions();
		}
	} 

	void D3D9RenderWindow::_finishSwitchingFullscreen()
	{		
		if(mIsFullScreen)
		{
			// Need to reset the region on the window sometimes, when the 
			// windowed mode was constrained by desktop 
			HRGN hRgn = CreateRectRgn(0,0,mWidth, mHeight);
			SetWindowRgn(mHWnd, hRgn, FALSE);
		}
		else
		{
			// When switching back to windowed mode, need to reset window size 
			// after device has been restored
			RECT rc;
			SetRect(&rc, 0, 0, mWidth, mHeight);
			AdjustWindowRect(&rc, GetWindowLong(mHWnd, GWL_STYLE), false);
			unsigned int winWidth = rc.right - rc.left;
			unsigned int winHeight = rc.bottom - rc.top;
			unsigned int screenw = GetSystemMetrics(SM_CXSCREEN);
			unsigned int screenh = GetSystemMetrics(SM_CYSCREEN);

			// deal with overflow when switching down to smaller resolution
			int left = screenw > winWidth ? ((screenw - winWidth) / 2) : 0;
			int top = screenh > winHeight ? ((screenh - winHeight) / 2) : 0;
			SetWindowPos(mHWnd, HWND_NOTOPMOST, left, top, winWidth, winHeight,
				SWP_DRAWFRAME | SWP_FRAMECHANGED | SWP_NOACTIVATE);

		}
		mSwitchingFullscreen = false;
	}
	
	void D3D9RenderWindow::buildPresentParameters(D3DPRESENT_PARAMETERS* presentParams)
	{		
		// Set up the presentation parameters		
		IDirect3D9* pD3D = D3D9RenderSystem::getDirect3D9();
		D3DDEVTYPE devType = D3DDEVTYPE_HAL;

		ZeroMemory( presentParams, sizeof(D3DPRESENT_PARAMETERS) );
		presentParams->Windowed					= !mIsFullScreen;
		presentParams->SwapEffect				= D3DSWAPEFFECT_DISCARD;
		// triple buffer if VSync is on
		presentParams->BackBufferCount			= mVSync ? 2 : 1;
		presentParams->EnableAutoDepthStencil	= mIsDepthBuffered;
		presentParams->hDeviceWindow			= mHWnd;
		presentParams->BackBufferWidth			= mWidth;
		presentParams->BackBufferHeight			= mHeight;
		presentParams->FullScreen_RefreshRateInHz = mIsFullScreen ? mDisplayFrequency : 0;
		
		if (presentParams->BackBufferWidth == 0)		
			presentParams->BackBufferWidth = 1;					

		if (presentParams->BackBufferHeight == 0)	
			presentParams->BackBufferHeight = 1;					


		if (mVSync)
		{
			presentParams->PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		}
		else
		{
			// NB not using vsync in windowed mode in D3D9 can cause jerking at low 
			// frame rates no matter what buffering modes are used (odd - perhaps a
			// timer issue in D3D9 since GL doesn't suffer from this) 
			// low is < 200fps in this context
			if (!mIsFullScreen)
			{
				LogManager::getSingleton().logMessage("D3D9 : WARNING - "
					"disabling VSync in windowed mode can cause timing issues at lower "
					"frame rates, turn VSync on if you observe this problem.");
			}
			presentParams->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
		}

		presentParams->BackBufferFormat		= D3DFMT_R5G6B5;
		if( mColourDepth > 16 )
			presentParams->BackBufferFormat = D3DFMT_X8R8G8B8;

		if (mColourDepth > 16 )
		{
			// Try to create a 32-bit depth, 8-bit stencil
			if( FAILED( pD3D->CheckDeviceFormat(mDevice->getAdapterNumber(),
				devType,  presentParams->BackBufferFormat,  D3DUSAGE_DEPTHSTENCIL, 
				D3DRTYPE_SURFACE, D3DFMT_D24S8 )))
			{
				// Bugger, no 8-bit hardware stencil, just try 32-bit zbuffer 
				if( FAILED( pD3D->CheckDeviceFormat(mDevice->getAdapterNumber(),
					devType,  presentParams->BackBufferFormat,  D3DUSAGE_DEPTHSTENCIL, 
					D3DRTYPE_SURFACE, D3DFMT_D32 )))
				{
					// Jeez, what a naff card. Fall back on 16-bit depth buffering
					presentParams->AutoDepthStencilFormat = D3DFMT_D16;
				}
				else
					presentParams->AutoDepthStencilFormat = D3DFMT_D32;
			}
			else
			{
				// Woohoo!
				if( SUCCEEDED( pD3D->CheckDepthStencilMatch( mDevice->getAdapterNumber(), devType,
					presentParams->BackBufferFormat, presentParams->BackBufferFormat, D3DFMT_D24S8 ) ) )
				{
					presentParams->AutoDepthStencilFormat = D3DFMT_D24S8; 
				} 
				else 
					presentParams->AutoDepthStencilFormat = D3DFMT_D24X8; 
			}
		}
		else
			// 16-bit depth, software stencil
			presentParams->AutoDepthStencilFormat	= D3DFMT_D16;


		D3D9RenderSystem* rsys = static_cast<D3D9RenderSystem*>(Root::getSingleton().getRenderSystem());
		
		rsys->determineFSAASettings(mDevice->getD3D9Device(),
			mFSAA, mFSAAHint, presentParams->BackBufferFormat, mIsFullScreen, 
			&mFSAAType, &mFSAAQuality);

		presentParams->MultiSampleType = mFSAAType;
		presentParams->MultiSampleQuality = (mFSAAQuality == 0) ? 0 : mFSAAQuality;

		// Check sRGB
		if (mHwGamma)
		{
			/* hmm, this never succeeds even when device does support??
			if(FAILED(pD3D->CheckDeviceFormat(mDriver->getAdapterNumber(),
				devType, presentParams->BackBufferFormat, D3DUSAGE_QUERY_SRGBWRITE, 
				D3DRTYPE_SURFACE, presentParams->BackBufferFormat )))
			{
				// disable - not supported
				mHwGamma = false;
			}
			*/

		}
	}
	
	void D3D9RenderWindow::destroy()
	{
		if (mDevice != NULL)
		{
			mDevice->detachRenderWindow(this);
			mDevice = NULL;
		}
		
		if (mHWnd && !mIsExternal)
		{
			WindowEventUtilities::_removeRenderWindow(this);
			DestroyWindow(mHWnd);
		}

		mHWnd = 0;
		mActive = false;
		mClosed = true;
	}

	bool D3D9RenderWindow::isActive() const
	{
		if (isFullScreen())
			return isVisible();

		return mActive && isVisible();
	}

	bool D3D9RenderWindow::isVisible() const
	{
		return (mHWnd && !IsIconic(mHWnd));
	}

	void D3D9RenderWindow::reposition(int top, int left)
	{
		if (mHWnd && !mIsFullScreen)
		{
			SetWindowPos(mHWnd, 0, top, left, 0, 0,
				SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	void D3D9RenderWindow::resize(unsigned int width, unsigned int height)
	{
		if (mHWnd && !mIsFullScreen)
		{
			RECT rc = { 0, 0, width, height };
			AdjustWindowRect(&rc, GetWindowLong(mHWnd, GWL_STYLE), false);
			width = rc.right - rc.left;
			height = rc.bottom - rc.top;
			SetWindowPos(mHWnd, 0, 0, 0, width, height,
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	void D3D9RenderWindow::windowMovedOrResized()
	{
		if (!mHWnd || IsIconic(mHWnd))
			return;
	
		updateWindowRect();
	}

	void D3D9RenderWindow::swapBuffers( bool waitForVSync )
	{
		if (mDeviceValid)
			mDevice->present(this);		
	}

	void D3D9RenderWindow::getCustomAttribute( const String& name, void* pData )
	{
		// Valid attributes and their equvalent native functions:
		// D3DDEVICE			: getD3DDevice
		// WINDOW				: getWindowHandle

		if( name == "D3DDEVICE" )
		{
			IDirect3DDevice9* *pDev = (IDirect3DDevice9**)pData;
			*pDev = getD3D9Device();
			return;
		}		
		else if( name == "WINDOW" )
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
			IDirect3DSurface9* *pSurf = (IDirect3DSurface9**)pData;
			*pSurf = mDevice->getDepthBuffer(this);
			return;
		}
		else if( name == "DDBACKBUFFER" )
		{
			IDirect3DSurface9* *pSurf = (IDirect3DSurface9**)pData;
			*pSurf = mDevice->getBackBuffer(this);
			return;
		}
		else if( name == "DDFRONTBUFFER" )
		{
			IDirect3DSurface9* *pSurf = (IDirect3DSurface9**)pData;
			*pSurf = mDevice->getBackBuffer(this);
			return;
		}
	}

	void D3D9RenderWindow::copyContentsToMemory(const PixelBox &dst, FrameBuffer buffer)
	{
		if ((dst.left < 0) || (dst.right > mWidth) ||
			(dst.top < 0) || (dst.bottom > mHeight) ||
			(dst.front != 0) || (dst.back != 1))
		{
			OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
						"Invalid box.",
						"D3D9RenderWindow::copyContentsToMemory" );
		}

		HRESULT hr;
		IDirect3DSurface9* pSurf = NULL;
		IDirect3DSurface9* pTempSurf = NULL;
		D3DSURFACE_DESC desc;
		D3DLOCKED_RECT lockedRect;

		IDirect3DDevice9* pD3DDevice = getD3D9Device();

		if (buffer == FB_AUTO)
		{
			//buffer = mIsFullScreen? FB_FRONT : FB_BACK;
			buffer = FB_FRONT;
		}

		if (buffer == FB_FRONT)
		{
			D3DDISPLAYMODE dm;
			
			if (FAILED(hr = pD3DDevice->GetDisplayMode(0, &dm)))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't get display mode: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}

			desc.Width = dm.Width;
			desc.Height = dm.Height;
			desc.Format = D3DFMT_A8R8G8B8;
			if (FAILED(hr = pD3DDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height,
																	 desc.Format,
																	 D3DPOOL_SYSTEMMEM,
																	 &pTempSurf,
																	 0)))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't create offscreen buffer: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}

			if (FAILED(hr = pD3DDevice->GetFrontBufferData(0, pTempSurf)))
			{
				SAFE_RELEASE(pTempSurf);
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't get front buffer: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}

			if(mIsFullScreen)
			{
				if ((dst.left == 0) && (dst.right == mWidth) && (dst.top == 0) && (dst.bottom == mHeight))
				{
					hr = pTempSurf->LockRect(&lockedRect, 0, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
				}
				else
				{
					RECT rect;

					rect.left = static_cast<LONG>(dst.left);
					rect.right = static_cast<LONG>(dst.right);
					rect.top = static_cast<LONG>(dst.top);
					rect.bottom = static_cast<LONG>(dst.bottom);

					hr = pTempSurf->LockRect(&lockedRect, &rect, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
				}
				if (FAILED(hr))
				{
					SAFE_RELEASE(pTempSurf);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
								"Can't lock rect: " + Root::getSingleton().getErrorDescription(hr),
								"D3D9RenderWindow::copyContentsToMemory");
				} 
			}
			else
			{
				RECT srcRect;
				//GetClientRect(mHWnd, &srcRect);
				srcRect.left = static_cast<LONG>(dst.left);
				srcRect.top = static_cast<LONG>(dst.top);
				srcRect.right = static_cast<LONG>(dst.right);
				srcRect.bottom = static_cast<LONG>(dst.bottom);
				POINT point;
				point.x = srcRect.left;
				point.y = srcRect.top;
				ClientToScreen(mHWnd, &point);
				srcRect.top = point.y;
				srcRect.left = point.x;
				srcRect.bottom += point.y;
				srcRect.right += point.x;

				desc.Width = srcRect.right - srcRect.left;
				desc.Height = srcRect.bottom - srcRect.top;

				if (FAILED(hr = pTempSurf->LockRect(&lockedRect, &srcRect, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK)))
				{
					SAFE_RELEASE(pTempSurf);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
								"Can't lock rect: " + Root::getSingleton().getErrorDescription(hr),
								"D3D9RenderWindow::copyContentsToMemory");
				} 
			}
		}
		else
		{
			SAFE_RELEASE(pSurf);
			if(FAILED(hr = pD3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurf)))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't get back buffer: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}

			if(FAILED(hr = pSurf->GetDesc(&desc)))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't get description: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}

			if (FAILED(hr = pD3DDevice->CreateOffscreenPlainSurface(desc.Width, desc.Height,
																	 desc.Format,
																	 D3DPOOL_SYSTEMMEM,
																	 &pTempSurf,
																	 0)))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't create offscreen surface: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}

			if (desc.MultiSampleType == D3DMULTISAMPLE_NONE)
			{
				if (FAILED(hr = pD3DDevice->GetRenderTargetData(pSurf, pTempSurf)))
				{
					SAFE_RELEASE(pTempSurf);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
								"Can't get render target data: " + Root::getSingleton().getErrorDescription(hr),
								"D3D9RenderWindow::copyContentsToMemory");
				}
			}
			else
			{
				IDirect3DSurface9* pStretchSurf = 0;

				if (FAILED(hr = pD3DDevice->CreateRenderTarget(desc.Width, desc.Height,
																desc.Format,
																D3DMULTISAMPLE_NONE,
																0,
																false,
																&pStretchSurf,
																0)))
				{
					SAFE_RELEASE(pTempSurf);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
								"Can't create render target: " + Root::getSingleton().getErrorDescription(hr),
								"D3D9RenderWindow::copyContentsToMemory");
				}

				if (FAILED(hr = pD3DDevice->StretchRect(pSurf, 0, pStretchSurf, 0, D3DTEXF_NONE)))
				{
					SAFE_RELEASE(pTempSurf);
					SAFE_RELEASE(pStretchSurf);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
								"Can't stretch rect: " + Root::getSingleton().getErrorDescription(hr),
								"D3D9RenderWindow::copyContentsToMemory");
				}
				if (FAILED(hr = pD3DDevice->GetRenderTargetData(pStretchSurf, pTempSurf)))
				{
					SAFE_RELEASE(pTempSurf);
					SAFE_RELEASE(pStretchSurf);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
								"Can't get render target data: " + Root::getSingleton().getErrorDescription(hr),
								"D3D9RenderWindow::copyContentsToMemory");
				}
				SAFE_RELEASE(pStretchSurf);
			}

			if ((dst.left == 0) && (dst.right == mWidth) && (dst.top == 0) && (dst.bottom == mHeight))
			{
				hr = pTempSurf->LockRect(&lockedRect, 0, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
			}
			else
			{
				RECT rect;

				rect.left = static_cast<LONG>(dst.left);
				rect.right = static_cast<LONG>(dst.right);
				rect.top = static_cast<LONG>(dst.top);
				rect.bottom = static_cast<LONG>(dst.bottom);

				hr = pTempSurf->LockRect(&lockedRect, &rect, D3DLOCK_READONLY | D3DLOCK_NOSYSLOCK);
			}
			if (FAILED(hr))
			{
				SAFE_RELEASE(pTempSurf);
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
							"Can't lock rect: " + Root::getSingleton().getErrorDescription(hr),
							"D3D9RenderWindow::copyContentsToMemory");
			}
		}

		PixelFormat format = Ogre::D3D9Mappings::_getPF(desc.Format);

		if (format == PF_UNKNOWN)
		{
			SAFE_RELEASE(pTempSurf);
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR,
						"Unsupported format", "D3D9RenderWindow::copyContentsToMemory");
		}

		PixelBox src(dst.getWidth(), dst.getHeight(), 1, format, lockedRect.pBits);
		src.rowPitch = lockedRect.Pitch / PixelUtil::getNumElemBytes(format);
		src.slicePitch = desc.Height * src.rowPitch;

		PixelUtil::bulkPixelConversion(src, dst);

		SAFE_RELEASE(pTempSurf);
		SAFE_RELEASE(pSurf);
	}
	//-----------------------------------------------------------------------------
	void D3D9RenderWindow::update(bool swap)
	{		
		// External windows should update per frame
		// since it dosen't get the window resize/move messages.
		if (mIsExternal)
		{		
			updateWindowRect();
		}

		if (mWidth == 0 || mHeight == 0)
		{
			mDeviceValid = false;
			return;
		}

		D3D9RenderSystem::getDeviceManager()->setActiveRenderTargetDevice(mDevice);
		
		// Check that device can be used for rendering operations.
		mDeviceValid = mDevice->validate(this);
		if (mDeviceValid)
		{
			// Finish window / fullscreen mode switch.
			if (_getSwitchingFullscreen())		
				_finishSwitchingFullscreen();		

			
			RenderWindow::update(swap);					
		}

		D3D9RenderSystem::getDeviceManager()->setActiveRenderTargetDevice(NULL);	
	}

	//-----------------------------------------------------------------------------
	IDirect3DDevice9* D3D9RenderWindow::getD3D9Device()
	{
		return mDevice->getD3D9Device();
	}

	//-----------------------------------------------------------------------------
	IDirect3DSurface9* D3D9RenderWindow::getRenderSurface()
	{
		return mDevice->getBackBuffer(this);
	}

	//-----------------------------------------------------------------------------
	bool D3D9RenderWindow::_getSwitchingFullscreen() const
	{
		return mSwitchingFullscreen;
	}

	//-----------------------------------------------------------------------------
	D3D9Device* D3D9RenderWindow::getDevice()
	{
		return mDevice;
	}

	//-----------------------------------------------------------------------------
	void D3D9RenderWindow::setDevice(D3D9Device* device)
	{
		mDevice = device;
		mDeviceValid = false;
	}

	//-----------------------------------------------------------------------------
	bool D3D9RenderWindow::isDepthBuffered() const
	{
		return mIsDepthBuffered;
	}

	//-----------------------------------------------------------------------------
	void D3D9RenderWindow::updateWindowRect()
	{
		RECT rc;
		BOOL result;

		// Update top left parameters
		result = GetWindowRect(mHWnd, &rc);
		if (result == FALSE)
		{
			mTop = 0;
			mLeft = 0;
			mWidth = 0;
			mHeight = 0;
			return;
		}
		
		mTop = rc.top;
		mLeft = rc.left;

		// width and height represent drawable area only
		result = GetClientRect(mHWnd, &rc);
		if (result == FALSE)
		{
			mTop = 0;
			mLeft = 0;
			mWidth = 0;
			mHeight = 0;
			return;
		}
		unsigned int width = rc.right - rc.left;
		unsigned int height = rc.bottom - rc.top;

		// Case window resized.
		if (width != mWidth || height != mHeight)
		{
			mWidth  = rc.right - rc.left;
			mHeight = rc.bottom - rc.top;

			// Notify viewports of resize
			ViewportList::iterator it = mViewportList.begin();
			while( it != mViewportList.end() )
				(*it++).second->_updateDimensions();			
		}			
	}

	//-----------------------------------------------------------------------------
	bool D3D9RenderWindow::isNvPerfHUDEnable() const
	{
		return mUseNVPerfHUD;
	}
}

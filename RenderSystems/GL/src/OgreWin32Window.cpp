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

#include "OgreWin32Window.h"
#include "OgreLogManager.h"
#include "OgreRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreException.h"


namespace Ogre {

    Win32Window::Win32Window()
    {
		mIsFullScreen = false;
		mHWnd = 0;
		mActive = false;
		mReady = false;
		mClosed = false;
		mExternalHandle = NULL;
    }

    Win32Window::~Win32Window()
    {
		destroy();
    }

    void Win32Window::create(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth,
                           bool fullScreen, int left, int top, bool depthBuffer,
                           void* miscParam, ...)
    {
        HWND parentHWnd;
		HINSTANCE hInst = GetModuleHandle("RenderSystem_GL.dll");
		long tempPtr;

		// Get variable-length params
		// miscParam[0] = parent HWND
		// miscParam[1] = bool vsync
		// miscParam[2] = int displayFrequency

		va_list marker;
		va_start( marker, depthBuffer );

		tempPtr = va_arg( marker, long );
		Win32Window* parentRW = reinterpret_cast<Win32Window*>(tempPtr);
		if( parentRW == NULL )
			parentHWnd = 0;
		else
			parentHWnd = parentRW->getWindowHandle();

		tempPtr = va_arg( marker, long );
		bool vsync = (tempPtr != 0);

		tempPtr = va_arg( marker, long );
		unsigned int displayFrequency = static_cast<unsigned int>(tempPtr);

		va_end( marker );

		// Destroy current window if any
		if( mHWnd )
			destroy();

        if (fullScreen)
        {
			mColourDepth = colourDepth;
        }
		else 
		{
			// Get colour depth from display
			mColourDepth = GetDeviceCaps(GetDC(0), BITSPIXEL);
		}

		if (!mExternalHandle) {
			mWidth = width;
			mHeight = height;
			if (!fullScreen)
			{
				if (!left && (unsigned)GetSystemMetrics(SM_CXSCREEN) > mWidth)
                {
					mLeft = (GetSystemMetrics(SM_CXSCREEN) / 2) - (mWidth / 2);
                }
				else
                {
					mLeft = left;
                }
				if (!top && (unsigned)GetSystemMetrics(SM_CYSCREEN) > mHeight)
                {
					mTop = (GetSystemMetrics(SM_CYSCREEN) / 2) - (mHeight / 2);
                }
				else
                {
                    mTop = top;
                }
			}
			else
            {
				mTop = mLeft = 0;
            }

			// Register the window class

			WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW, WndProc, 0, 4, hInst,
				LoadIcon( NULL, IDI_APPLICATION ),
				LoadCursor( NULL, IDC_ARROW ),
				(HBRUSH)GetStockObject( BLACK_BRUSH ), NULL,
				TEXT(name.c_str()) };
			RegisterClass( &wndClass );

			// Create our main window
			// Pass pointer to self
			HWND hWnd = CreateWindowEx(fullScreen?WS_EX_TOPMOST:0, TEXT(name.c_str()), TEXT(name.c_str()),
				(fullScreen?WS_POPUP:WS_OVERLAPPEDWINDOW)|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, mLeft, mTop,
				width, height, 0L, 0L, hInst, this);
			mHWnd = hWnd;

            RECT rc;
			GetClientRect(mHWnd,&rc);
			mWidth = rc.right;
			mHeight = rc.bottom;

            if (fullScreen) {
			    DEVMODE DevMode;
			    DevMode.dmSize = sizeof(DevMode);
			    DevMode.dmBitsPerPel = mColourDepth;
			    DevMode.dmPelsWidth = mWidth;
			    DevMode.dmPelsHeight = mHeight;
			    DevMode.dmDisplayFrequency = displayFrequency;
			    DevMode.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY;
			    if (ChangeDisplaySettings(&DevMode, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				    LogManager::getSingleton().logMessage(LML_CRITICAL, "ChangeDisplaySettingsEx");

				
		    }

		}
		else {
			mHWnd = mExternalHandle;
			RECT rc;
			GetClientRect(mHWnd, &rc);
			mWidth = rc.right;
			mHeight = rc.bottom;
			mLeft = rc.left;
			mTop = rc.top;
		}
		ShowWindow(mHWnd, SW_SHOWNORMAL);
		UpdateWindow(mHWnd);
		mName = name;
		mIsDepthBuffered = depthBuffer;
		mIsFullScreen = fullScreen;

		
		HDC hdc = GetDC(mHWnd);

		LogManager::getSingleton().logMessage(
			LML_NORMAL, "Created Win32Window '%s' : %ix%i, %ibpp",
			mName.c_str(), mWidth, mHeight, mColourDepth );

		
        PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW |
			PFD_SUPPORT_OPENGL |
			PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA,
			mColourDepth,
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			32, 			     // 32-bit depth-buffer (will be emulated in 16-bit colour mode)
			8,				     // 8-bit stencil buffer
			0,
			PFD_MAIN_PLANE,
			0,
			0, 0, 0};
		int iPixelFormat = ChoosePixelFormat(hdc, &pfd);
		if (!iPixelFormat)
			Except(0, "ChoosePixelFormat failed", "Win32Window::create");
		if (!SetPixelFormat(hdc, iPixelFormat, &pfd))
			Except(0, "SetPixelFormat failed", "Win32Window::create");

		HGLRC glrc = wglCreateContext(hdc);
		if (!glrc)
			Except(0, "wglCreateContext", "Win32Window::create");
		if (!wglMakeCurrent(hdc, glrc))
			Except(0, "wglMakeCurrent", "Win32Window::create");
		
		mGlrc = glrc;
		mHDC = hdc;

		mOldSwapIntervall = wglGetSwapIntervalEXT();
		if (vsync) 
			wglSwapIntervalEXT(1);
		else
			wglSwapIntervalEXT(0);

		mReady = true;
    }

    void Win32Window::destroy(void)
    {
        wglSwapIntervalEXT(mOldSwapIntervall);
		if (mGlrc) {
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(mGlrc);
			mGlrc = NULL;
		}
		if (mHDC) {
			ReleaseDC(mHWnd, mHDC);
			mHDC = NULL;
		}
		if (mIsFullScreen)
		{
			ChangeDisplaySettings(NULL, 0);
		}
	DestroyWindow(mHWnd);
        mActive = false;
    }

    bool Win32Window::isActive() const
    {
        return mActive;
    }

    bool Win32Window::isClosed() const
    {
        return mClosed;
    }

    void Win32Window::reposition(int left, int top)
    {
        // XXX FIXME
    }

    void Win32Window::resize(unsigned int width, unsigned int height)
    {

		mWidth = width;
		mHeight = height;

		// Notify viewports of resize
		ViewportList::iterator it, itend;
        itend = mViewportList.end();
		for( it = mViewportList.begin(); it != itend; ++it )
			(*it).second->_updateDimensions();
		// TODO - resize window
    }

	void Win32Window::windowMovedOrResized()
	{
		RECT temprect;
		::GetWindowRect(getWindowHandle(),&temprect);
		resize(temprect.right-temprect.left,temprect.bottom-temprect.top);
		// TODO
	}

    void Win32Window::swapBuffers(bool waitForVSync)
    {
		SwapBuffers(mHDC);
    }

    void Win32Window::outputText(int x, int y, const String& text)
    {
        //deprecated
    }
	void Win32Window::writeContentsToFile(const String& filename)
	{
		ImageCodec::ImageData imgData;
		imgData.width = mWidth;
		imgData.height = mHeight;
		imgData.format = PF_R8G8B8;

		// Allocate buffer 
		uchar* pBuffer = new uchar[mWidth * mHeight * 3];

		// Read pixels
		// I love GL: it does all the locking & colour conversion for us
		glReadPixels(0,0, mWidth-1, mHeight-1, GL_RGB, GL_UNSIGNED_BYTE, pBuffer);

		// Wrap buffer in a chunk
		DataChunk chunk(pBuffer, mWidth * mHeight * 3);

		// Need to flip the read data over in Y though
		Image img;
		img.loadRawData(chunk, mWidth, mHeight, PF_R8G8B8 );
		img.flipAroundX();

		DataChunk chunkFlipped(img.getData(), chunk.getSize());

		// Get codec 
		size_t pos = filename.find_last_of(".");
		String extension;
		if( pos == String::npos )
			Except(
			Exception::ERR_INVALIDPARAMS, 
			"Unable to determine image type for '" + filename + "' - invalid extension.",
			"Win32Window::writeContentsToFile" );

		while( pos != filename.length() - 1 )
			extension += filename[++pos];

		// Get the codec
		Codec * pCodec = Codec::getCodec(extension);

		// Write out
		pCodec->codeToFile(chunkFlipped, filename, &imgData);

		delete [] pBuffer;
	}

	// Window procedure callback
	// This is a static member, so applies to all windows but we store the
	// Win32Window instance in the window data GetWindowLog/SetWindowLog
	LRESULT Win32Window::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
	{
		Win32Window* win;

		// look up window instance
		if( WM_CREATE != uMsg )
			win = (Win32Window*)GetWindowLong( hWnd, 0 );

		switch( uMsg )
		{
		case WM_ACTIVATE:
			if( WA_INACTIVE == LOWORD( wParam ) )
				win->mActive = false;
			else
				win->mActive = true;
			break;

		case WM_CREATE: {
			// Log the new window
			// Get CREATESTRUCT
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			win = (Win32Window*)(lpcs->lpCreateParams);
			// Store pointer in window user data area
			SetWindowLong( hWnd, 0, (long)win );
			win->mActive = true;

			return 0; }
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
			win->windowMovedOrResized();
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
					win->windowMovedOrResized();
			}
			break;

		case WM_GETMINMAXINFO:
			// Prevent the window from going smaller than some minimu size
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
			break;

		case WM_CLOSE:
			DestroyWindow( win->mHWnd );
			win->mClosed = true;
			return 0;
		}

		return DefWindowProc( hWnd, uMsg, wParam, lParam );
	}
}

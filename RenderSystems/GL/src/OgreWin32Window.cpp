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
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreStringConverter.h"
#include "OgreException.h"
#include "OgreWin32GLSupport.h"
#include "OgreWin32Context.h"

namespace Ogre {

	Win32Window::Win32Window(Win32GLSupport &glsupport):
		mGLSupport(glsupport),
        mContext(0)
    {
		mIsFullScreen = false;
		mHWnd = 0;
		mActive = false;
		mReady = false;
		mClosed = false;
    }

    Win32Window::~Win32Window()
    {
		destroy();
    }

	void Win32Window::create(const String& name, unsigned int width, unsigned int height,
	            bool fullScreen, const NameValuePairList *miscParams)
    {
        HWND parentHWnd = 0;
		HWND externalHandle = 0;
		HINSTANCE hInst = GetModuleHandle("RenderSystem_GL.dll");
		long tempPtr;
		bool vsync = false;
		unsigned int displayFrequency = 0;
		String title = name;
		unsigned int colourDepth = 32;
		unsigned int left = 0; // Defaults to screen center
		unsigned int top = 0; // Defaults to screen center
		bool depthBuffer = true;

		if(miscParams)
		{
			// Get variable-length params
			NameValuePairList::const_iterator opt;
			// left (x)
			opt = miscParams->find("left");
			if(opt != miscParams->end())
				left = StringConverter::parseUnsignedInt(opt->second);
			// top (y)
			opt = miscParams->find("top");
			if(opt != miscParams->end())
				top = StringConverter::parseUnsignedInt(opt->second);
			// Window title
			opt = miscParams->find("title");
			if(opt != miscParams->end())
				title = opt->second;
			// externalWindowHandle		-> externalHandle
			opt = miscParams->find("externalWindowHandle");
			if(opt != miscParams->end())
				externalHandle = (HWND)StringConverter::parseUnsignedInt(opt->second);
			// parentWindowHandle -> parentHWnd
			opt = miscParams->find("parentWindowHandle");
			if(opt != miscParams->end()) 
				parentHWnd = (HWND)StringConverter::parseUnsignedInt(opt->second);
			// vsync	[parseBool]
			opt = miscParams->find("vsync");
			if(opt != miscParams->end())
				vsync = StringConverter::parseBool(opt->second);
			// displayFrequency
			opt = miscParams->find("displayFrequency");
			if(opt != miscParams->end())
				displayFrequency = StringConverter::parseUnsignedInt(opt->second);
			// colourDepth
			opt = miscParams->find("colourDepth");
			if(opt != miscParams->end())
				colourDepth = StringConverter::parseUnsignedInt(opt->second);
			// depthBuffer [parseBool]
			opt = miscParams->find("depthBuffer");
			if(opt != miscParams->end())
				depthBuffer = StringConverter::parseBool(opt->second);
		}
		
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

		if (!externalHandle) {
			DWORD dwStyle = (fullScreen ? WS_POPUP : WS_OVERLAPPEDWINDOW) | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
			RECT rc;

			mWidth = width;
			mHeight = height;

			if (!fullScreen)
			{
				// Calculate window dimensions required to get the requested client area
				SetRect(&rc, 0, 0, mWidth, mHeight);
				AdjustWindowRect(&rc, dwStyle, false);
				mWidth = rc.right - rc.left;
				mHeight = rc.bottom - rc.top;

				// Clamp width and height to the desktop dimensions
				if (mWidth > (unsigned)GetSystemMetrics(SM_CXSCREEN))
					mWidth = (unsigned)GetSystemMetrics(SM_CXSCREEN);
				if (mHeight > (unsigned)GetSystemMetrics(SM_CYSCREEN))
					mHeight = (unsigned)GetSystemMetrics(SM_CYSCREEN);

				if (!left)
                {
					mLeft = (GetSystemMetrics(SM_CXSCREEN) / 2) - (mWidth / 2);
                }
				else
                {
					mLeft = left;
                }
				if (!top)
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
				TEXT(title.c_str()) };
			RegisterClass( &wndClass );

			// Create our main window
			// Pass pointer to self
			HWND hWnd = CreateWindowEx(fullScreen?WS_EX_TOPMOST:0, TEXT(title.c_str()), TEXT(title.c_str()),
				dwStyle, mLeft, mTop, mWidth, mHeight, 0L, 0L, hInst, this);
			mHWnd = hWnd;

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
			mHWnd = externalHandle;
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

        StringUtil::StrStreamType str;
        str << "Created Win32Window '"
            << mName << "' : " << mWidth << "x" << mHeight
            << ", " << mColourDepth << "bpp";
        LogManager::getSingleton().logMessage(LML_NORMAL, str.str());

		
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

        // Create RenderSystem context
        mContext = new Win32Context(mHDC, mGlrc);
        // Register the context with the rendersystem and associate it with this window
        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        rs->_registerContext(this, mContext);
    }

    void Win32Window::destroy(void)
    {
        // Unregister and destroy OGRE GLContext
        GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton().getRenderSystem());
        rs->_unregisterContext(this);
        delete mContext;    

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
		::GetClientRect(getWindowHandle(),&temprect);
		resize(temprect.right-temprect.left,temprect.bottom-temprect.top);
		// TODO
	}

    void Win32Window::swapBuffers(bool waitForVSync)
    {
		SwapBuffers(mHDC);
    }

	void Win32Window::writeContentsToFile(const String& filename)
	{
		ImageCodec::ImageData *imgData = new ImageCodec::ImageData();
		imgData->width = mWidth;
		imgData->height = mHeight;
		imgData->depth = 1;
		imgData->format = PF_BYTE_RGB;

		// Allocate buffer 
		uchar* pBuffer = new uchar[mWidth * mHeight * 3];

		// Read pixels
		// I love GL: it does all the locking & colour conversion for us
		glReadPixels(0,0, mWidth-1, mHeight-1, GL_RGB, GL_UNSIGNED_BYTE, pBuffer);

		// Wrap buffer in a memory stream
		DataStreamPtr stream(new MemoryDataStream(pBuffer, mWidth * mHeight * 3, false));

		// Need to flip the read data over in Y though
		Image img;
		img.loadRawData(stream, mWidth, mHeight, imgData->format );
		img.flipAroundX();

		MemoryDataStreamPtr streamFlipped(new MemoryDataStream(img.getData(), stream->size(), false));

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
		Codec::CodecDataPtr ptr(imgData);
        pCodec->codeToFile(streamFlipped, filename, ptr);

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
	
	void Win32Window::getCustomAttribute( const String& name, void* pData )
	{
		if( name == "HWND" )
		{
			HWND *pHwnd = (HWND*)pData;
			*pHwnd = getWindowHandle();
			return;
		}
	}

}

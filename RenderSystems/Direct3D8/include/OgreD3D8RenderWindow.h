#pragma once

#ifndef __D3D8RENDERWINDOW_H__
#define __D3D8RENDERWINDOW_H__

// Precompiler options
#include "OgreD3D8Prerequisites.h"

#include "OgreRenderWindow.h"
#include "OgreD3D8Driver.h"

#include "OgreNoMemoryMacros.h"
#include "d3dfont.h"
#include "OgreMemoryMacros.h"


namespace Ogre {

	class D3D8RenderWindow : public RenderWindow
	{
	public:
		D3D8RenderWindow();
		~D3D8RenderWindow();

		void create( String name, int width, int height, int colourDepth,
			bool fullScreen, int left, int top, bool depthBuffer, void* miscParam, ... );

		void destroy(void);
		bool isActive() { return mActive; }
		bool isClosed() { return mClosed; }
		void reposition( int left, int top ) {}
		void resize( int width, int height );
		void swapBuffers( bool waitForVSync = true );
		HWND getWindowHandle() { return mHWnd; }
		HWND getParentWindowHandle() { return mParentHWnd; }

		D3D8Driver* getDirectD3DDriver() { return mpD3DDriver; }
		LPDIRECT3DDEVICE8 getD3DDevice() { return mpD3DDevice; }

		void outputText( int x, int ty, const String& text );
		void getCustomAttribute( String name, void* pData );
        /** Overridden - see RenderTarget.
        */
        void writeContentsToFile(const String& filename);

		bool requiresTextureFlipping() const { return false; }

	protected:
		HWND	mHWnd;					// Win32 Window handle
		HWND	mParentHWnd;			// Parent Win32 window handle
		bool	mActive;				// Is active i.e. visible
		bool	mReady;					// Is ready i.e. available for update
		bool	mClosed;

		static LRESULT CALLBACK WndProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam );

		// -------------------------------------------------------
		// DirectX-specific
		// -------------------------------------------------------

		// Pointer to D3DDriver encapsulating Direct3D driver
		D3D8Driver* mpD3DDriver;

		// Pointer to the 3D device specific for this window
		LPDIRECT3DDEVICE8	mpD3DDevice;
		D3DPRESENT_PARAMETERS md3dpp;
        LPDIRECT3DSURFACE8 mpRenderSurface;
        LPDIRECT3DSURFACE8 mpRenderZBuffer;

		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized(void);
	};

}

#endif
#ifndef __D3D9RENDERWINDOW_H__
#define __D3D9RENDERWINDOW_H__

// Precompiler options
#include "OgreD3D9Prerequisites.h"

#include "OgreRenderWindow.h"
#include "OgreD3D9Driver.h"

#include "OgreNoMemoryMacros.h"
//#include "d3dfont.h"
#include "OgreMemoryMacros.h"

namespace Ogre 
{
	class D3D9RenderWindow : public RenderWindow
	{
	public:
		D3D9RenderWindow();
		~D3D9RenderWindow();

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

		D3D9Driver* getDirectD3DDriver() { return mpD3DDriver; }
		LPDIRECT3DDEVICE9 getD3DDevice() { return mpD3DDevice; }

		void outputText( int x, int ty, const String& text );
		void getCustomAttribute( String name, void* pData );
		/** Overridden - see RenderTarget.
		*/
		void writeContentsToFile(const String& filename);
		bool requiresTextureFlipping() const { return false; }

		// Method for getting multisample (anti aliasing) quality currently in use
		DWORD getMultiSampleQuality(){return mMultiSampleQuality;};
		// Method for dealing with resize / move & 3d library
		void WindowMovedOrResized(void);
		// Method for passing a external window handle before creation ;)
		void SetExternalWindowHandle(HWND externalHandle) {mExternalHandle = externalHandle;};

		bool isReady() { return mReady; }
		void setReady(bool set) { mReady = set; }
		void setActive(bool set) { mActive = set; }

	protected:
		HWND	mExternalHandle;		// External Win32 window handle
		HWND	mHWnd;					// Win32 Window handle
		HWND	mParentHWnd;			// Parent Win32 window handle
		bool	mActive;				// Is active i.e. visible
		bool	mReady;					// Is ready i.e. available for update
		bool	mClosed;
		DWORD	mMultiSampleQuality;

		static LRESULT CALLBACK WndProc(
			HWND hWnd,
			UINT uMsg,
			WPARAM wParam,
			LPARAM lParam );

		// -------------------------------------------------------
		// DirectX-specific
		// -------------------------------------------------------

		// Pointer to D3DDriver encapsulating Direct3D driver
		D3D9Driver* mpD3DDriver;

		// Pointer to the 3D device specific for this window
		LPDIRECT3DDEVICE9	mpD3DDevice;
		D3DPRESENT_PARAMETERS md3dpp;
		LPDIRECT3DSURFACE9 mpRenderSurface;
		LPDIRECT3DSURFACE9 mpRenderZBuffer;

		// just check if the multisampling requested is supported by the device
		bool _checkMultiSampleQuality(D3DMULTISAMPLE_TYPE type, DWORD *outQuality, D3DFORMAT fBack, D3DFORMAT fDepth, UINT adapterNum, D3DDEVTYPE deviceType, BOOL fullScreen);
	};
}
#endif
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
http://www.gnu.org/copyleft/lesser.txt.
-----------------------------------------------------------------------------
*/
#ifndef __D3D9RENDERWINDOW_H__
#define __D3D9RENDERWINDOW_H__

#include "OgreD3D9Prerequisites.h"
#include "OgreRenderWindow.h"
#include "OgreD3D9Driver.h"

namespace Ogre 
{
	class D3D9RenderWindow : public RenderWindow
	{
	public:
		D3D9RenderWindow();
		~D3D9RenderWindow();

		void create( const String& name, int width, int height, int colourDepth,
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
		void getCustomAttribute( const String& name, void* pData );
		/** Overridden - see RenderTarget.
		*/
		void writeContentsToFile(const String& filename);
		bool requiresTextureFlipping() const { return false; }

		// Method for dealing with resize / move & 3d library
		virtual void WindowMovedOrResized(void);
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
		bool _checkMultiSampleQuality(D3DMULTISAMPLE_TYPE type, DWORD *outQuality, D3DFORMAT format, UINT adapterNum, D3DDEVTYPE deviceType, BOOL fullScreen);
	};
}
#endif
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2002 The OGRE Team
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

#ifndef __Win32Window_H__
#define __Win32Window_H__

#include "OgreWin32Prerequisites.h"
#include "OgreRenderWindow.h"

namespace Ogre {
    class Win32Window : public RenderWindow
    {
    public:
        Win32Window(Win32GLSupport &glsupport);
        ~Win32Window();

        void create(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth,
            bool fullScreen, int left, int top, bool depthBuffer, void* miscParam, ...);
        void destroy(void);
        bool isActive(void) const;
        bool isClosed(void) const;
        void reposition(int left, int top);
        void resize(unsigned int width, unsigned int height);
        void swapBuffers(bool waitForVSync);

        /** Overridden - see RenderTarget.
        */
        void writeContentsToFile(const String& filename);

		bool requiresTextureFlipping() const { return false; }

		HWND getWindowHandle() const { return mHWnd; }
		
		// Method for dealing with resize / move & 3d library
		virtual void windowMovedOrResized(void);
		// Method for passing a external window handle before creation ;)
		void setExternalWindowHandle(HWND externalHandle) {mExternalHandle = externalHandle;};

		bool isReady() const { return mReady; }
		void setReady(bool set) { mReady = set; }
		void setActive(bool set) { mActive = set; }

	protected:
		Win32GLSupport &mGLSupport;
		HWND	mExternalHandle;		// External Win32 window handle
		HWND	mHWnd;					// Win32 Window handle
		HDC		mHDC;
		HGLRC	mGlrc;
		int		mOldSwapIntervall;
		bool	mActive;				// Is active i.e. visible
		bool	mReady;					// Is ready i.e. available for update
		bool	mClosed;
        Win32Context *mContext;

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    };
}

#endif

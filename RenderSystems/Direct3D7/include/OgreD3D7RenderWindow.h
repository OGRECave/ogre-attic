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
http://www.gnu.org/copyleft/lgpl.html.
-----------------------------------------------------------------------------
*/
#ifndef __Win32Window_H__
#define __Win32Window_H__

// Precompiler options
#include "OgreD3D7Prerequisites.h"

#include "OgreRenderWindow.h"
#include "OgreDDDriver.h"
#include "OgreD3D7DeviceList.h"
#include "OgreD3D7Device.h"

namespace Ogre {
    /** Defines the rendering window on Win32 systems
        @remarks
            This class represents the rendering window on the
            Windows family of operating systems.
     */
    class Win32Window : public RenderWindow
    {
    public:
        Win32Window();
        ~Win32Window();


        /** Overridden - see RenderWindow.
          
            @remarks
                In this case there should be 3 parameters in the miscParam
                group.
            @par
                miscParam[0] A pointer to a HINSTANCE representing the Win32
                instance the window is owned by.
            @par
                miscParam[1] A pointer to a DDDriver instance representing 
                the DirectDraw driver to be used to manage the window 
                contents. Alternatively, supply 0 to use conventional GDI.
            @par
                miscParam[2] A pointer to a RenderWindow representing the 
                window that is to be parent to this window (a MDI or child 
                window). Supply 0 to make the window a top-level window.
        */
        void create(
            String name, 
            int width, int height, int colourDepth,
            bool fullScreen, 
            int left, int top, 
            bool depthBuffer, void* miscParam, ... );
        
        /** Overridden - see RenderWindow 
        */
        void destroy(void);

        /** Overridden - see RenderWindow
        */
        bool isActive(void);

        /** Overridden - see RenderWindow
        */
        bool isClosed(void);

        /** Overridden - see RenderWindow
        */
        void reposition(int left, int top);

        /** Overridden - see RenderWindow
        */
        void resize(int width, int height);

        /** Overridden - see RenderWindow
        */
        void swapBuffers(bool waitForVSync);

        /** Retrieves a handle to the window (Win32 specific)
        */
        HWND getWindowHandle(void);

        /** Retrieves a handle to the window's parent, or 0 if there is no parent.
        */
        HWND getParentWindowHandle(void);

        /** Returns true if this window is using DirectDraw.
        */
        bool isUsingDirectDraw(void);
        /** Retrieves a pointer to the DirectDraw wrapper object used to 
            manage the window's surface. (Win32 specific)
            @note
                Only applicable for windows using DirectDraw.
        */
        DDDriver* getDirectDrawDriver(void);

        /** Returns a pointer to a DirectDraw surface containing the
            front buffer.
            @note
                Only applicable for windows using DirectDraw.
        */
        LPDIRECTDRAWSURFACE7 getDDFrontBuffer(void);

        /** Returns a pointer to a DirectDraw surface containing the back 
            buffer. Only applicable if using DirectDraw.
        */
        LPDIRECTDRAWSURFACE7 getDDBackBuffer(void);

        /** Returns a pointer to the Direct3D device that works with this
            window
            @note
                Only applicable for windows using DirectDraw.
        */
        LPDIRECT3DDEVICE7 getD3DDevice(void);

        /** Overridden - see RenderTarget.
        */
        void outputText(int x, int y, const String& text);

        /** Overridden - see RenderTarget.
        */
        void getCustomAttribute(String name, void* pData);

    protected:
        // Win32-specific data members
        bool mIsUsingDirectDraw;
        HWND mHWnd;                    // Win32 Window handle
        HWND mParentHWnd;
        bool mActive;                // Is active i.e. visible
        bool mReady;                // Is ready i.e. available for update
        bool mClosed;

        RECT rcBlitDest;            // Target for blit operation
                                    // == window client area


        static LRESULT CALLBACK WndProc( 
            HWND hWnd, 
            UINT uMsg, 
            WPARAM wParam, 
            LPARAM lParam);

        // ---------------------------------------------------------
        // DirectX-specific
        // ---------------------------------------------------------

        // Pointer to DDDriver encapsulating DirectDraw driver (if DirectX in use)
        DDDriver* mlpDDDriver;

        // Pointers to DX surfaces
        LPDIRECTDRAWSURFACE7 mlpDDSFront; // Pointer to primary (front) buffer surface
        LPDIRECTDRAWSURFACE7 mlpDDSBack; // Pointer to back buffer

        // Pointer to the 3D Device specific for this window
        LPDIRECT3DDEVICE7 mlpD3DDevice;

        // DirectDraw Methods
        void createDDSurfaces(void);
        void releaseDDSurfaces(void);
        void restoreDDSurfaces(void);
        void createDepthBuffer(void);

        // Method for dealing with resize / move & 3d library
        void windowMovedOrResized(void);
        
        // ---------------------------------------------------------
        // OpenGL-specific details
        // ---------------------------------------------------------

        // TODO
    };
}
#endif

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/
#include "OgreWin32Input.h"
#include "OgreRenderWindow.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"

#define DINPUT_BUFFERSIZE  16

namespace Ogre {
    //-----------------------------------------------------------------------
    Win32Input::Win32Input()
    {
        mlpDI = 0;
        mlpDIKeyboard = 0;
        mlpDIMouse = 0;

        memset(mKeyboardBuffer,0,256);


    }
    //-----------------------------------------------------------------------
    Win32Input::~Win32Input()
    {
        // Shutdown
        if (mlpDIKeyboard)
        {
            mlpDIKeyboard->Unacquire();
            mlpDIKeyboard->Release();
            mlpDIKeyboard = 0;
        }
        if (mlpDIMouse)
        {
            mlpDIMouse->Unacquire();
            mlpDIMouse->Release();
            mlpDIMouse = 0;
        }
        if (mlpDI)
        {
            mlpDI->Release();
            mlpDI = 0;
        }

    }

    //-----------------------------------------------------------------------
    void Win32Input::initialise(RenderWindow* pWindow, bool useKeyboard, bool useMouse, bool useGameController)
    {
        HRESULT hr;

        LogManager::getSingleton().logMessage("Win32Input: DirectInput Activation Starts");

        // Get HINST
        HINSTANCE hInst = GetModuleHandle("OgrePlatform.dll");


        // Get HWND
        HWND hWnd;
        //pWindow->getCustomAttribute("HWND", &hWnd);
        // Decouple from Win32Window
        hWnd = GetActiveWindow();

        mHWnd = hWnd;
        RECT rect;
        GetClientRect(mHWnd, &rect);
        mMouseCenterX = (rect.right - rect.left) / 2;
        mMouseCenterY = (rect.bottom - rect.top) / 2;
        POINT p;
        p.x = mMouseCenterX;
        p.y = mMouseCenterY;
        ClientToScreen(mHWnd, &p);
        SetCursorPos(p.x, p.y);
        // hide cursor
        ShowCursor(FALSE);



        // Create direct input
        hr = DirectInputCreateEx(hInst, DIRECTINPUT_VERSION,
            IID_IDirectInput7, (void**)&mlpDI, NULL);

        if (FAILED(hr))
            throw Exception(hr, "Unable to initialise DirectInput.",
                "Win32Input - initialise");

        if (useKeyboard)
        {
            LogManager::getSingleton().logMessage("Win32Input: Establishing keyboard input.");

            // Create keyboard device
            hr = mlpDI->CreateDeviceEx(GUID_SysKeyboard, IID_IDirectInputDevice7,
                (void**)&mlpDIKeyboard, NULL);


            if (FAILED(hr))
                throw Exception(hr, "Unable to create DirectInput keyboard device.",
                    "Win32Input - initialise");

            // Set data format
            hr = mlpDIKeyboard->SetDataFormat(&c_dfDIKeyboard);
            if (FAILED(hr))
                throw Exception(hr, "Unable to set DirectInput keyboard device data format.",
                    "Win32Input - initialise");

            // Make the window grab keyboard behaviour when foreground
            // NB Keyboard is never exclusive
            hr = mlpDIKeyboard->SetCooperativeLevel(hWnd,
                       DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
            if (FAILED(hr))
                throw Exception(hr, "Unable to set DirectInput keyboard device co-operative level.",
                    "Win32Input - initialise");


            // Acquire input
            hr = mlpDIKeyboard->Acquire();
            if (FAILED(hr))
                throw Exception(hr, "Unable to set aquire DirectInput keyboard device.",
                    "Win32Input - initialise");

            LogManager::getSingleton().logMessage("Win32Input: Keyboard input established.");
        }
        if (useMouse)
        {
            /* don't use DI
            LogManager::getSingleton().logMessage("Win32Input: Establishing mouse input.");

            // Create mouse device
            hr = mlpDI->CreateDeviceEx(GUID_SysMouse, IID_IDirectInputDevice7,
                (void**)&mlpDIMouse, NULL);


            if (FAILED(hr))
                throw Exception(hr, "Unable to create DirectInput mouse device.",
                    "Win32Input - initialise");

            // Set data format
            hr = mlpDIMouse->SetDataFormat(&c_dfDIMouse);
            if (FAILED(hr))
                throw Exception(hr, "Unable to set DirectInput mouse device data format.",
                    "Win32Input - initialise");

            // Make the window grab mouse behaviour when foreground
            hr = mlpDIMouse->SetCooperativeLevel(hWnd,
                       DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
            if (FAILED(hr))
                throw Exception(hr, "Unable to set DirectInput mouse device co-operative level.",
                    "Win32Input - initialise");

            // Acquire input
            hr = mlpDIKeyboard->Acquire();
            if (FAILED(hr))
                throw Exception(hr, "Unable to set aquire DirectInput mouse device.",
                    "Win32Input - initialise");

            LogManager::getSingleton().logMessage("Win32Input: Mouse input established.");
            */

        }


        LogManager::getSingleton().logMessage("Win32Input: DirectInput OK.");

    }

    //-----------------------------------------------------------------------
    void Win32Input::capture(void)
    {

        HRESULT  hr;

        // Get keyboard state
        hr = mlpDIKeyboard->GetDeviceState(256,(LPVOID)&mKeyboardBuffer);
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
        {
            hr = mlpDIKeyboard->Acquire();
            if (hr == DIERR_OTHERAPPHASPRIO)
            {
                hr = 0;
            }
            else
            {
                hr = mlpDIKeyboard->GetDeviceState(256,(LPVOID)&mKeyboardBuffer);
            }
        }
        else if (hr == DIERR_OTHERAPPHASPRIO)
        {
            // We've gone into the background - ignore
            hr = 0;
        }
        else if (hr == DIERR_NOTINITIALIZED)
        {
            hr = 0;
        }
        else if (hr == E_PENDING)
        {
            hr = 0;
        }
        else if (FAILED(hr))
        {
            // Ignore for now
            // TODO - sort this out
            hr = 0;
        }

        /*
        DIMOUSESTATE diMouseState;

        if (mlpDIMouse)
        {
            hr = mlpDIMouse->GetDeviceState(sizeof(DIMOUSESTATE),(LPVOID)&diMouseState);
            if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
            {
                hr = mlpDIMouse->Acquire();
                if (hr == DIERR_OTHERAPPHASPRIO)
                {
                    hr = 0;
                }
                else
                {
                    hr = mlpDIKeyboard->GetDeviceState(sizeof(DIMOUSESTATE),(LPVOID)&diMouseState);
                }
            }
            else if (hr == DIERR_OTHERAPPHASPRIO)
            {
                // We've gone into the background - ignore
                hr = 0;
            }
            else if (hr == DIERR_NOTINITIALIZED)
            {
                hr = 0;
            }
            else if (hr == E_PENDING)
            {
                hr = 0;
            }
            else if (FAILED(hr))
            {
                // Ignore for now
                // TODO - sort this out
                hr = 0;
            }
            else
            {
                mMouseRelX = diMouseState.lX;
                mMouseRelY = diMouseState.lY;

            }
        }
        */
        /*
            Only update mouse position if the window has the focus
         */
        if( mHWnd == GetForegroundWindow() )
        {
            POINT p;
            GetCursorPos(&p);
            ScreenToClient(mHWnd,&p);
            mMouseX = (Real)p.x;
            mMouseY = (Real)p.y;
            p.x = mMouseCenterX;
            p.y = mMouseCenterY;
            ClientToScreen(mHWnd, &p);
            if( IsWindowVisible( mHWnd ) )
            SetCursorPos(p.x, p.y);
        }
    }
    //-----------------------------------------------------------------------
    bool Win32Input::isKeyDown(KeyCode kc)
    {
        if (mKeyboardBuffer[kc] & 0x80)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    //-----------------------------------------------------------------------
    int Win32Input::getMouseRelativeX(void)
    {
        return mMouseX - mMouseCenterX;
    }
    //-----------------------------------------------------------------------
    int Win32Input::getMouseRelativeY(void)
    {
        return mMouseY - mMouseCenterY;
    }


} // namespace

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
#include "OgreWin32Input8.h"
#include "OgreRenderWindow.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreMouseEvent.h"
#include "OgreInputEvent.h"
#include "OgreEventQueue.h"
#include "OgreCursor.h"
#include <dxerr8.h>

#define DINPUT_BUFFERSIZE  16
//#define DIPROP_BUFFERSIZE 256

namespace Ogre {
    //-----------------------------------------------------------------------
    Win32Input8::Win32Input8() :
		InputReader()
    {
        mlpDI = 0;
        mlpDIKeyboard = 0;
        mlpDIMouse = 0;
		mEventQueue = 0;
		mScale = 0.001;

        memset(mKeyboardBuffer,0,256);
    }
    //-----------------------------------------------------------------------
    Win32Input8::~Win32Input8()
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
    void Win32Input8::initialiseBufferedKeyboard()
	{
		// not implemented yet
		initialiseImmediateKeyboard();	// HACK
	}

    //-----------------------------------------------------------------------
    void Win32Input8::initialiseImmediateKeyboard()
	{
        HRESULT hr;
        LogManager::getSingleton().logMessage("Win32Input8: Establishing keyboard input.");

        // Create keyboard device
        hr = mlpDI->CreateDevice(GUID_SysKeyboard, &mlpDIKeyboard, NULL);


        if (FAILED(hr))
            throw Exception(hr, "Unable to create DirectInput keyboard device.",
                "Win32Input8 - initialise");

        // Set data format
        hr = mlpDIKeyboard->SetDataFormat(&c_dfDIKeyboard);
        if (FAILED(hr))
            throw Exception(hr, "Unable to set DirectInput keyboard device data format.",
                "Win32Input8 - initialise");

        // Make the window grab keyboard behaviour when foreground
        // NB Keyboard is never exclusive
        hr = mlpDIKeyboard->SetCooperativeLevel(mHWnd,
                   DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
        if (FAILED(hr))
            throw Exception(hr, "Unable to set DirectInput keyboard device co-operative level.",
                "Win32Input8 - initialise");

		// IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
		//
		// DirectInput uses unbuffered I/O (buffer size = 0) by default.
		// If you want to read buffered data, you need to set a nonzero
		// buffer size.
		//
		// Set the buffer size to DINPUT_BUFFERSIZE (defined above) elements.
		//
		// The buffer size is a DWORD property associated with the device.
		DIPROPDWORD dipdw;

		dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj        = 0;
		dipdw.diph.dwHow        = DIPH_DEVICE;
		dipdw.dwData            = DINPUT_BUFFERSIZE ; // Arbitary buffer size

		hr = mlpDIKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ;

        if (FAILED(hr))
            throw Exception(hr, "Unable to create DirectInput keyboard buffer.",
                "Win32Input8 - initialise");


        // Acquire input
        hr = mlpDIKeyboard->Acquire();
        if (FAILED(hr))
            throw Exception(hr, "Unable to set aquire DirectInput keyboard device.",
                "Win32Input8 - initialise");

        LogManager::getSingleton().logMessage("Win32Input8: Keyboard input established.");
	}
    //-----------------------------------------------------------------------
    void Win32Input8::initialiseImmediateMouse()
	{
        OgreGuard( "Win32Input8::initialiseImmediateMouse" );

        HRESULT hr;
        DIPROPDWORD dipdw;
        LogManager::getSingleton().logMessage( "Win32Input8: Initializing mouse input in buffered mode." );

        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj        = 0;
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.dwData            = DIPROPAXISMODE_ABS;

        if( /* Create the DI Device. */
            FAILED( hr = mlpDI->CreateDevice( GUID_SysMouse, &mlpDIMouse, NULL ) ) ||
            /* Set the data format so that it knows it's a mouse. */
            FAILED( hr = mlpDIMouse->SetDataFormat( &c_dfDIMouse2 ) ) ||
            /* Absolute mouse input. We can derive the relative input from this. */
            FAILED( hr = mlpDIMouse->SetProperty( DIPROP_AXISMODE, &dipdw.diph ) ) ||
            /* Exclusive when in foreground, steps back when in background. */
            FAILED( hr = mlpDIMouse->SetCooperativeLevel( mHWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE ) ) ||
            /* Get the device. */
            FAILED( hr = mlpDIMouse->Acquire() ) )
        {
            Except( hr, "Unable to initialise mouse", "Win32Input8::initialiseImmediateMouse" );
        }

        /* Get initial mouse data. */
        captureMouse();

        /* Clear any mouse data. */
        mMouseState.Xrel = mMouseState.Yrel = mMouseState.Zrel = 0;

        LogManager::getSingleton().logMessage( "Win32Input8: Mouse input in buffered mode initialized." );

        OgreUnguard();
	}

    //-----------------------------------------------------------------------
    void Win32Input8::initialiseBufferedMouse()
	{
        HRESULT hr;
        LogManager::getSingleton().logMessage("Win32Input8: Establishing mouse input.");

        // Create mouse device
        hr = mlpDI->CreateDevice(GUID_SysMouse, &mlpDIMouse, NULL);


        if (FAILED(hr))
            throw Exception(hr, "Unable to create DirectInput mouse device.",
                "Win32Input8 - initialise");

        // Set data format
        hr = mlpDIMouse->SetDataFormat(&c_dfDIMouse);
        if (FAILED(hr))
            throw Exception(hr, "Unable to set DirectInput mouse device data format.",
                "Win32Input8 - initialise");

        // Make the window grab mouse behaviour when foreground
        hr = mlpDIMouse->SetCooperativeLevel(mHWnd,
                   DISCL_FOREGROUND | DISCL_EXCLUSIVE);
        if (FAILED(hr))
            throw Exception(hr, "Unable to set DirectInput mouse device co-operative level.",
                "Win32Input8 - initialise");


		// IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
		//
		// DirectInput uses unbuffered I/O (buffer size = 0) by default.
		// If you want to read buffered data, you need to set a nonzero
		// buffer size.
		//
		// Set the buffer size to SAMPLE_BUFFER_SIZE (defined above) elements.
		//
		// The buffer size is a DWORD property associated with the device.
		DIPROPDWORD dipdw;
		dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj        = 0;
		dipdw.diph.dwHow        = DIPH_DEVICE;
		dipdw.dwData            = DINPUT_BUFFERSIZE; // Arbitary buffer size

		hr = mlpDIMouse->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );

        if (FAILED(hr))
            throw Exception(hr, "Unable to create DirectInput mouse buffer.",
                "Win32Input8 - initialise");

        // Acquire input
        hr = mlpDIMouse->Acquire();
        if (FAILED(hr))
            throw Exception(hr, "Unable to set aquire DirectInput mouse device.",
                "Win32Input8 - initialise");

        LogManager::getSingleton().logMessage("Win32Input8: Mouse input established.");

	}

    //-----------------------------------------------------------------------
    void Win32Input8::initialise(RenderWindow* pWindow, bool useKeyboard, bool useMouse, bool useGameController)
    {
        HRESULT hr;

        LogManager::getSingleton().logMessage("Win32Input8: DirectInput Activation Starts");

        // Get HINST
        HINSTANCE hInst = GetModuleHandle("OgrePlatform.dll");

        // Get HWND
        HWND hWnd = GetActiveWindow();

        mHWnd = hWnd;

        ShowCursor(FALSE);


    // Register with the DirectInput subsystem and get a pointer
    // to a IDirectInput interface we can use.
    // Create a DInput object
		hr = DirectInput8Create( hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&mlpDI, NULL );
        if (FAILED(hr))
            throw Exception(hr, "Unable to initialise DirectInput.",
                "Win32Input8 - initialise");


		// If a queue exists, then use buffered mode, not immediate

		if (mUseBuffered)
		{
			if (useKeyboard)
			{
				initialiseBufferedKeyboard();
			}
			if (useMouse)
			{
				initialiseBufferedMouse();
			}
		}
		else
		{
			if (useKeyboard)
			{
				initialiseImmediateKeyboard();
			}
			if (useMouse)
			{
				initialiseImmediateMouse();
			}

		}

 

        LogManager::getSingleton().logMessage("Win32Input8: DirectInput OK.");

    }

    //-----------------------------------------------------------------------
    void Win32Input8::capture(void)
    {
		if (mUseBuffered)
		{
			mModifiers = getKeyModifiers();
			readBufferedKeyboardData();
			readBufferedMouseData();
		}
		else
		{
			captureKeyboard();
			captureMouse();
		}

	}
    //-----------------------------------------------------------------------
    void Win32Input8::captureKeyboard(void)
    {
        HRESULT  hr;

        // Get keyboard state
        hr = mlpDIKeyboard->GetDeviceState(sizeof(mKeyboardBuffer),(LPVOID)&mKeyboardBuffer);
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
        {
            hr = mlpDIKeyboard->Acquire();
            if (hr == DIERR_OTHERAPPHASPRIO)
            {
                hr = 0;
            }
            else
            {
                hr = mlpDIKeyboard->GetDeviceState(sizeof(mKeyboardBuffer),(LPVOID)&mKeyboardBuffer);
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

	}

    //-----------------------------------------------------------------------
    void Win32Input8::captureMouse(void)
    {
        DIMOUSESTATE2 mouseState;
        HRESULT hr;

        // Get mouse state
        hr = mlpDIMouse->GetDeviceState( sizeof( DIMOUSESTATE2 ), (LPVOID)&mouseState );

        if( SUCCEEDED( hr ) ||
            ( ( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED ) &&
              SUCCEEDED( mlpDIMouse->Acquire() ) && 
              SUCCEEDED( mlpDIMouse->GetDeviceState( sizeof( DIMOUSESTATE2 ), (LPVOID)&mouseState ) ) ) )
        {
            /* Register the new 'origin'. */
            mMouseCenterX = mMouseState.Xabs;
            mMouseCenterY = mMouseState.Yabs;
            mMouseCenterZ = mMouseState.Zabs;

            /* Get the new absolute position. */
            mMouseState.Xabs = mouseState.lX;
            mMouseState.Yabs = mouseState.lY;
            mMouseState.Zabs = mouseState.lZ;            

            /* Compute the new relative position. */
            mMouseState.Xrel = mMouseState.Xabs - mMouseCenterX;
            mMouseState.Yrel = mMouseState.Yabs - mMouseCenterY;
            mMouseState.Zrel = mMouseState.Zabs - mMouseCenterZ;

            /* Get the mouse buttons. This for loop can be unwrapped for speed. */
            mMouseState.Buttons = 0;
            for( int i = 0; i < 8; i++ )
                if( mouseState.rgbButtons[ i ] & 0x80 )
                    mMouseState.Buttons |= ( 1 << i );
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
 
   }



	//-----------------------------------------------------------------------------
	// Name: readBufferedData()
	// Desc: Read the input device's state when in buffered mode and display it.
	//-----------------------------------------------------------------------------
	bool Win32Input8::readBufferedKeyboardData()
	{
		captureKeyboard();		// HACK - buffered not implemented yet - use immediate
		return true;
	}

	//-----------------------------------------------------------------------------
	// Name: readBufferedData()
	// Desc: Read the input device's state when in buffered mode and display it.
	//-----------------------------------------------------------------------------
	bool Win32Input8::readBufferedMouseData()
	{
		DIDEVICEOBJECTDATA didod[ DINPUT_BUFFERSIZE ];  // Receives buffered data 
		DWORD              dwElements;
		HRESULT            hr;

		if( NULL == mlpDIMouse ) 
			return true;
    
		dwElements = DINPUT_BUFFERSIZE;

		hr = mlpDIMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
										 didod, &dwElements, 0 );
		if( hr != DI_OK ) 
		{
			// We got an error or we got DI_BUFFEROVERFLOW.
			//
			// Either way, it means that continuous contact with the
			// device has been lost, either due to an external
			// interruption, or because the buffer overflowed
			// and some events were lost.
			//
			// Consequently, if a button was pressed at the time
			// the buffer overflowed or the connection was broken,
			// the corresponding "up" message might have been lost.
			//
			// But since our simple sample doesn't actually have
			// any state associated with button up or down events,
			// there is no state to reset.  (In a real game, ignoring
			// the buffer overflow would result in the game thinking
			// a key was held down when in fact it isn't; it's just
			// that the "up" event got lost because the buffer
			// overflowed.)
			//
			// If we want to be cleverer, we could do a
			// GetDeviceState() and compare the current state
			// against the state we think the device is in,
			// and process all the states that are currently
			// different from our private state.
			hr = mlpDIMouse->Acquire();
			while( hr == DIERR_INPUTLOST ) 
				hr = mlpDIMouse->Acquire();

			// Update the dialog text 
	/*        if( hr == DIERR_OTHERAPPHASPRIO || 
				hr == DIERR_NOTACQUIRED ) 
				SetDlgItemText( hDlg, IDC_DATA, TEXT("Unacquired") );
	*/
			// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
			// may occur when the app is minimized or in the process of 
			// switching, so just try again later 
			return S_OK; 
		}

		if( FAILED(hr) )  
			return false;

		bool xSet = false;
		bool ySet = false;
		bool zSet = false;

		for(unsigned int i = 0; i < dwElements; i++ ) 
		{
			int nMouseCode = -1;		// not set

			// this will display then scan code of the key
			// plus a 'D' - meaning the key was pressed 
			//   or a 'U' - meaning the key was released
			switch( didod [ i ].dwOfs )
			{
				case DIMOFS_BUTTON0:
					nMouseCode = InputEvent::BUTTON0_MASK;
					break;

				case DIMOFS_BUTTON1:
					nMouseCode = InputEvent::BUTTON1_MASK;
					break;

				case DIMOFS_BUTTON2:
					nMouseCode = InputEvent::BUTTON2_MASK;
					break;

				case DIMOFS_BUTTON3:
					nMouseCode = InputEvent::BUTTON3_MASK;
					break;

				case DIMOFS_X:
					if (xSet) 
					{	// process the last X move since we have a new one
						mouseMoved(); 
						xSet = false;
					}
					mCursor->addToX(getScaled(didod[i].dwData));
					xSet = true;
					break;

				case DIMOFS_Y:
					if (ySet) 
					{
						mouseMoved(); 
						ySet = false;
					}
					mCursor->addToY(getScaled(didod[i].dwData));  
					ySet = true;
					break;

				case DIMOFS_Z:
					if (zSet) 
					{
						mouseMoved(); 
						zSet = false;
					}
					mCursor->addToZ(getScaled(didod[i].dwData));
					zSet = true;
					break;

				default:
					break;
			}
			if (xSet && ySet)	// don't create 2 mousemove events for an single X and Y move, just create 1.
			{
				mouseMoved(); 
				ySet = false;
				xSet = false;
			}

			if (nMouseCode != -1)
			{
				triggerMouseButton(nMouseCode, (didod [ i ].dwData & 0x80) != 0);
			}

		}
		if (zSet || xSet || ySet) // check for last moved at end
		{
			mouseMoved(); 
		}

		return true;
	}
    //-----------------------------------------------------------------------

	Real Win32Input8::getScaled(DWORD dwVal)
	{
		return (Real)((int)dwVal) * mScale;
	}

    //-----------------------------------------------------------------------
    bool Win32Input8::isKeyDown(KeyCode kc) const
    {
        return ( mKeyboardBuffer[ kc ] & 0x80 ) != 0;
    }

    //---------------------------------------------------------------------------------------------
    long Win32Input8::getMouseRelX() const
    {
        return mMouseState.Xrel;
    }

    //---------------------------------------------------------------------------------------------
    long Win32Input8::getMouseRelY() const
    {
        return mMouseState.Yrel;
    }

    //---------------------------------------------------------------------------------------------
    long Win32Input8::getMouseRelZ() const
    {
        return mMouseState.Zrel;
    }

    long Win32Input8::getMouseAbsX() const
    {
        return mMouseState.Xabs;
    }

    long Win32Input8::getMouseAbsY() const
    {
        return mMouseState.Yabs;
    }

    long Win32Input8::getMouseAbsZ() const
    {
        return mMouseState.Zabs;
    }

    //---------------------------------------------------------------------------------------------
    bool Win32Input8::getMouseButton( uchar button ) const
    {
        return mMouseState.isButtonDown( button ) != 0;
    }

    //---------------------------------------------------------------------------------------------
    void Win32Input8::getMouseState( MouseState& state ) const
    {
        memcpy( &state, &mMouseState, sizeof( MouseState ) );
    }

    //---------------------------------------------------------------------------------------------
	long Win32Input8::getKeyModifiers()
	{
		int ret = 0;

		if (isKeyDown(KC_LMENU) || isKeyDown(KC_RMENU))
		{
			ret |= InputEvent::ALT_MASK;
		}

		if (isKeyDown(KC_LSHIFT) || isKeyDown(KC_RSHIFT))
		{
			ret |= InputEvent::SHIFT_MASK;
		}

		if (isKeyDown(KC_LCONTROL) || isKeyDown(KC_LCONTROL))
		{
			ret |= InputEvent::CTRL_MASK;
		}

		return ret;
	}

} // namespace

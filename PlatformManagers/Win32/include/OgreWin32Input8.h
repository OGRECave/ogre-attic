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
#ifndef __Win32Input8_H__
#define __Win32Input8_H__

#include "OgreWin32Prerequisites.h"
#include "OgreInput.h"
#include "OgreInputEvent.h"

#include <dinput.h>

namespace Ogre {

    /** Utility class for dealing with user input on a Win32 system.
        Note that this is a basic implementation only at the moment.
    */
    class Win32Input8 : public InputReader
    {
    public:

        Win32Input8();
        ~Win32Input8();

        /** Initialise the input system.
            @param pWindow The window to capture input for
            @param useKeyboard If true, keyboard input will be supported.
            @param useMouse If true, mouse input will be supported.
            @param useGameController If true, joysticks/gamepads will be supported.
        */
	    void initialise(RenderWindow* pWindow, bool useKeyboard = true, bool useMouse = true, bool useGameController = true);

        /** Captures the state of all the input devices.
            This method captures the state of all input devices and stores it internally for use when
            the enquiry methods are next called. This is done to ensure that all input is captured at once
            and therefore combinations of input are not subject to time differences when methods are called.

        */
        void capture(void);

        /** Determines if the specified key is currently depressed.
            Note that this enquiry method uses the state of the keyboard at the last 'capture' call.
        */
        bool isKeyDown(KeyCode kc);

        /** Retrieves the relative position of the mouse when capture was called relative to the last time. */
        int getMouseRelativeX(void);
        /** Retrieves the relative position of the mouse when capture was called relative to the last time. */
        int getMouseRelativeY(void);


	    bool getMouseButton(bool leftButton);


    private:
        // Input device details
        LPDIRECTINPUT8 mlpDI;
        LPDIRECTINPUTDEVICE8 mlpDIKeyboard;
        LPDIRECTINPUTDEVICE8 mlpDIMouse;

        HWND mHWnd;




		/** specialised initialisation routines */		
	    void initialiseBufferedKeyboard();
	    void initialiseImmediateKeyboard();
	    void initialiseBufferedMouse();
		void initialiseImmediateMouse();

		/* immediate mode */
	    void captureKeyboard(void);
	    void captureMouse(void);

		/* buffered mode */
		bool readBufferedKeyboardData();
		bool readBufferedMouseData();

        /* State of modifiers at last 'capture' call 
		   NOTE this doesn't support keyboard buffering yet */
		int getKeyModifiers();


		/* for mouse immediate mode */
        Real mMouseX, mMouseY;
		Real mMouseCenterX, mMouseCenterY;
		bool mLMBDown, mRMBDown;

		/* for mouse buffered mode */
		Real mScale;
		Real getScaled(DWORD dwVal);


		/* for keyboard immediate mode */
        char mKeyboardBuffer[256];
    };



}


#endif

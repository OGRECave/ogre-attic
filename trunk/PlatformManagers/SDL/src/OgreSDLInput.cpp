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

#include "OgreSDLInput.h"
#include "OgreLogManager.h"

namespace Ogre {
    
    SDLInput::SDLInput()
    {
    }

    SDLInput::~SDLInput()
    {
        SDL_ShowCursor(1);
    }

    void SDLInput::initialise(RenderWindow* pWindow, bool useKeyboard, bool useMouse, bool useGameController)
    {
        // Hide the cursor
        SDL_ShowCursor(0);

        // Get the center and put the mouse there
        int width, height, depth, left, top;
        pWindow->getMetrics(width, height, depth, left, top);

        mMouseCenterX = width / 2;
        mMouseCenterY = height / 2;

        SDL_WarpMouse(mMouseCenterX, mMouseCenterY);

        
        // XXX Anything to do?
    }

    void SDLInput::capture()
    {
        SDL_PumpEvents();

        // Get Keyboard state
        mKeyboardBuffer = SDL_GetKeyState(NULL);

        // Get mouse info
        if( SDL_GetAppState() & SDL_APPMOUSEFOCUS )
        {
            mMouseKeys = SDL_GetMouseState( &mMouseX, &mMouseY );
            SDL_WarpMouse( mMouseCenterX, mMouseCenterY );
        }

        // XXX Fix me up
        // Game controller state
    }

    bool SDLInput::isKeyDown(KeyCode kc)
    {
        if (kc == KC_ESCAPE)
            return mKeyboardBuffer[SDLK_ESCAPE];

        if (kc == KC_S)
            return mKeyboardBuffer[SDLK_s];
        
        if (kc == KC_W)
            return mKeyboardBuffer[SDLK_w];

        if (kc == KC_A)
            return mKeyboardBuffer[SDLK_a];

        if (kc == KC_S)
            return mKeyboardBuffer[SDLK_s];
        
        if (kc == KC_D)
            return mKeyboardBuffer[SDLK_d];

        if (kc == KC_RIGHT)
            return mKeyboardBuffer[SDLK_RIGHT];

        if (kc == KC_LEFT)
            return mKeyboardBuffer[SDLK_LEFT];

        return false;
    }

    int SDLInput::getMouseRelativeX()
    {
        return mMouseX - mMouseCenterX;
    }

    int SDLInput::getMouseRelativeY()
    {
        return mMouseY - mMouseCenterY;
    }
}

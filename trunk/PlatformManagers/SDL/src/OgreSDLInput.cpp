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
        switch(kc)
        {
        case KC_ESCAPE:
            return mKeyboardBuffer[SDLK_ESCAPE];
        case KC_1:
            return mKeyboardBuffer[SDLK_1];
        case KC_2:
            return mKeyboardBuffer[SDLK_2];
        case KC_3:
            return mKeyboardBuffer[SDLK_3];
        case KC_4:
            return mKeyboardBuffer[SDLK_4];
        case KC_5:
            return mKeyboardBuffer[SDLK_5];
        case KC_6:
            return mKeyboardBuffer[SDLK_6];
        case KC_7:
            return mKeyboardBuffer[SDLK_7];
        case KC_8:
            return mKeyboardBuffer[SDLK_8];
        case KC_9:
            return mKeyboardBuffer[SDLK_9];
        case KC_0:
            return mKeyboardBuffer[SDLK_0];
        case KC_MINUS:  /* - on main keyboard */
            return mKeyboardBuffer[SDLK_MINUS];
        case KC_EQUALS:
            return mKeyboardBuffer[SDLK_EQUALS];
        case KC_BACK:  /* backspace */
            return mKeyboardBuffer[SDLK_BACKSPACE];
        case KC_TAB:
            return mKeyboardBuffer[SDLK_TAB];
        case KC_Q:
            return mKeyboardBuffer[SDLK_q];
        case KC_W:
            return mKeyboardBuffer[SDLK_w];
        case KC_E:
            return mKeyboardBuffer[SDLK_e];
        case KC_R:
            return mKeyboardBuffer[SDLK_r];
        case KC_T:
            return mKeyboardBuffer[SDLK_t];
        case KC_Y:
            return mKeyboardBuffer[SDLK_y];
        case KC_U:
            return mKeyboardBuffer[SDLK_u];
        case KC_I:
            return mKeyboardBuffer[SDLK_i];
        case KC_O:
            return mKeyboardBuffer[SDLK_o];
        case KC_P:
            return mKeyboardBuffer[SDLK_p];
        case KC_LBRACKET:
        case KC_RBRACKET:
            break;
        case KC_RETURN: /* Enter on main keyboard */
            return mKeyboardBuffer[SDLK_RETURN];
        case KC_LCONTROL:
            return mKeyboardBuffer[SDLK_LCTRL];
        case KC_A:
            return mKeyboardBuffer[SDLK_a];
        case KC_S:
            return mKeyboardBuffer[SDLK_s];
        case KC_D:
            return mKeyboardBuffer[SDLK_d];
        case KC_F:
            return mKeyboardBuffer[SDLK_f];
        case KC_G:
            return mKeyboardBuffer[SDLK_g];
        case KC_H:
            return mKeyboardBuffer[SDLK_h];
        case KC_J:
            return mKeyboardBuffer[SDLK_j];
        case KC_K:
            return mKeyboardBuffer[SDLK_k];
        case KC_L:
            return mKeyboardBuffer[SDLK_l];
        case KC_SEMICOLON:
        case KC_APOSTROPHE:
        case KC_GRAVE:  /* accent grave */
            break;
        case KC_LSHIFT:
            return mKeyboardBuffer[SDLK_LSHIFT];
        case KC_BACKSLASH:
            return mKeyboardBuffer[SDLK_BACKSLASH];
        case KC_Z:
            return mKeyboardBuffer[SDLK_z];
        case KC_X:
            return mKeyboardBuffer[SDLK_x];
        case KC_C:
            return mKeyboardBuffer[SDLK_c];
        case KC_V:
            return mKeyboardBuffer[SDLK_v];
        case KC_B:
            return mKeyboardBuffer[SDLK_b];
        case KC_N:
            return mKeyboardBuffer[SDLK_n];
        case KC_M:
            return mKeyboardBuffer[SDLK_m];
        case KC_COMMA:
            return mKeyboardBuffer[SDLK_COMMA];
        case KC_PERIOD: /* . on main keyboard */
            return mKeyboardBuffer[SDLK_PERIOD];
        case KC_SLASH: /* '/' on main keyboard */
            break;
        case KC_RSHIFT:
            return mKeyboardBuffer[SDLK_RSHIFT];
        case KC_MULTIPLY: /* * on numeric keypad */
            return mKeyboardBuffer[SDLK_KP_MULTIPLY];
        case KC_LMENU: /* left Alt */
            return mKeyboardBuffer[SDLK_LALT];
        case KC_SPACE:
            return mKeyboardBuffer[SDLK_SPACE];
        case KC_CAPITAL:
            return mKeyboardBuffer[SDLK_CAPSLOCK];
        case KC_F1:
            return mKeyboardBuffer[SDLK_F1];
        case KC_F2:
            return mKeyboardBuffer[SDLK_F2];
        case KC_F3:
            return mKeyboardBuffer[SDLK_F3];
        case KC_F4:
            return mKeyboardBuffer[SDLK_F4];
        case KC_F5:
            return mKeyboardBuffer[SDLK_F5];
        case KC_F6:
            return mKeyboardBuffer[SDLK_F6];
        case KC_F7:
            return mKeyboardBuffer[SDLK_F7];
        case KC_F8:
            return mKeyboardBuffer[SDLK_F8];
        case KC_F9:
            return mKeyboardBuffer[SDLK_F9];
        case KC_F10:
            return mKeyboardBuffer[SDLK_F10];
        case KC_NUMLOCK:
            return mKeyboardBuffer[SDLK_NUMLOCK];
        case KC_SCROLL: /* Scroll Lock */
            return mKeyboardBuffer[SDLK_SCROLLOCK];
        case KC_NUMPAD7:
            return mKeyboardBuffer[SDLK_KP7];
        case KC_NUMPAD8:
            return mKeyboardBuffer[SDLK_KP8];
        case KC_NUMPAD9:
            return mKeyboardBuffer[SDLK_KP9];
        case KC_SUBTRACT: /* - on numeric keypad */
            return mKeyboardBuffer[SDLK_KP_MINUS];
        case KC_NUMPAD4:
            return mKeyboardBuffer[SDLK_KP4];
        case KC_NUMPAD5:
            return mKeyboardBuffer[SDLK_KP5];
        case KC_NUMPAD6:
            return mKeyboardBuffer[SDLK_KP6];
        case KC_ADD: /* + on numeric keypad */
            return mKeyboardBuffer[SDLK_KP_PLUS];
        case KC_NUMPAD1:
            return mKeyboardBuffer[SDLK_KP1];
        case KC_NUMPAD2:
            return mKeyboardBuffer[SDLK_KP2];
        case KC_NUMPAD3:
            return mKeyboardBuffer[SDLK_KP3];
        case KC_NUMPAD0:
            return mKeyboardBuffer[SDLK_KP0];
        case KC_DECIMAL:  /* . on numeric keypad */
            return mKeyboardBuffer[SDLK_KP_PERIOD];
        case KC_OEM_102:  /* < > | on UK/Germany keyboards */
            break;
        case KC_F11:
            return mKeyboardBuffer[SDLK_F11];
        case KC_F12:
            return mKeyboardBuffer[SDLK_F12];
        case KC_F13: /* (NEC PC98) */
            return mKeyboardBuffer[SDLK_F13];
        case KC_F14: /* (NEC PC98) */
            return mKeyboardBuffer[SDLK_F14];
        case KC_F15: /* (NEC PC98) */
            return mKeyboardBuffer[SDLK_F15];
        case KC_KANA: /* (Japanese keyboard) */
        case KC_ABNT_C1: /* / ? on Portugese (Brazilian) keyboards */
        case KC_CONVERT: /* (Japanese keyboard) */
        case KC_NOCONVERT: /* (Japanese keyboard) */
        case KC_YEN: /* (Japanese keyboard) */
        case KC_ABNT_C2: /* Numpad . on Portugese (Brazilian) keyboards */
            break;
        case KC_NUMPADEQUALS: /* = on numeric keypad (NEC PC98) */
            return mKeyboardBuffer[SDLK_KP_EQUALS];
        case KC_PREVTRACK: /* Previous Track (KC_CIRCUMFLEX on Japanese keyboard) */
        case KC_AT: /* (NEC PC98) */
        case KC_COLON: /* (NEC PC98) */
        case KC_UNDERLINE: /* (NEC PC98) */
        case KC_KANJI: /* (Japanese keyboard)            */
        case KC_STOP: /* (NEC PC98) */
        case KC_AX:  /*  (Japan AX) */
        case KC_UNLABELED:  /* (J3100) */
        case KC_NEXTTRACK:  /* Next Track */
        case KC_NUMPADENTER:  /* Enter on numeric keypad */
        case KC_RCONTROL:
        case KC_MUTE:  /* Mute */
        case KC_CALCULATOR:  /* Calculator */
        case KC_PLAYPAUSE:  /* Play / Pause */
        case KC_MEDIASTOP:  /* Media Stop */
        case KC_VOLUMEDOWN:  /* Volume - */
        case KC_VOLUMEUP:  /* Volume + */
        case KC_WEBHOME:  /* Web home */
        case KC_NUMPADCOMMA:  /* , on numeric keypad (NEC PC98) */
            break;
        case KC_DIVIDE:  /* / on numeric keypad */
            return mKeyboardBuffer[SDLK_KP_DIVIDE];
        case KC_SYSRQ:
            return mKeyboardBuffer[SDLK_SYSREQ];
        case KC_RMENU:  /* right Alt */
            return mKeyboardBuffer[SDLK_RALT];
        case KC_PAUSE:  /* Pause */
            break;
        case KC_HOME:  /* Home on arrow keypad */
            return mKeyboardBuffer[SDLK_HOME];
        case KC_UP:  /* UpArrow on arrow keypad */
            return mKeyboardBuffer[SDLK_UP];
        case KC_PGUP:  /* PgUp on arrow keypad */
            return mKeyboardBuffer[SDLK_PAGEUP];
        case KC_LEFT:  /* LeftArrow on arrow keypad */
            return mKeyboardBuffer[SDLK_LEFT];
        case KC_RIGHT:  /* RightArrow on arrow keypad */
            return mKeyboardBuffer[SDLK_RIGHT];
        case KC_END:  /* End on arrow keypad */
            return mKeyboardBuffer[SDLK_END];
        case KC_DOWN:  /* DownArrow on arrow keypad */
            return mKeyboardBuffer[SDLK_DOWN];
        case KC_PGDOWN:  /* PgDn on arrow keypad */
            return mKeyboardBuffer[SDLK_PAGEDOWN];
        case KC_INSERT:  /* Insert on arrow keypad */
            return mKeyboardBuffer[SDLK_INSERT];
        case KC_DELETE:  /* Delete on arrow keypad */
            return mKeyboardBuffer[SDLK_DELETE];
        case KC_LWIN:  /* Left Windows key */
            return mKeyboardBuffer[SDLK_LSUPER];
        case KC_RWIN:  /* Right Windows key */
            return mKeyboardBuffer[SDLK_RSUPER];
        case KC_APPS:  /* AppMenu key */
        case KC_POWER:  /* System Power */
        case KC_SLEEP:  /* System Sleep */
        case KC_WAKE:  /* System Wake */
        case KC_WEBSEARCH:  /* Web Search */
        case KC_WEBFAVORITES:  /* Web Favorites */
        case KC_WEBREFRESH:  /* Web Refresh */
        case KC_WEBSTOP:  /* Web Stop */
        case KC_WEBFORWARD:  /* Web Forward */
        case KC_WEBBACK:  /* Web Back */
        case KC_MYCOMPUTER:  /* My Computer */
        case KC_MAIL:  /* Mail */
        case KC_MEDIASELECT:  /* Media Select */
            break;
        default:
            break;
        };
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

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

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

#include "OgreSDLInput.h"
#include "OgreLogManager.h"
#include "OgreMouseEvent.h"
#include "OgreCursor.h"

namespace Ogre {
    
    SDLInput::SDLInput() 
        : InputReader(), mMouseX(0), mMouseY(0), mMouseCenterX(0), 
          mMouseCenterY(0), mScale(0.002), _visible(true)
    {
        mEventQueue = 0;

        _key_map.insert(InputKeyMap::value_type(SDLK_ESCAPE,KC_ESCAPE));
        _key_map.insert(InputKeyMap::value_type(SDLK_1, KC_1));
        _key_map.insert(InputKeyMap::value_type(SDLK_2, KC_2));
        _key_map.insert(InputKeyMap::value_type(SDLK_3, KC_3));
        _key_map.insert(InputKeyMap::value_type(SDLK_4, KC_4));
        _key_map.insert(InputKeyMap::value_type(SDLK_5, KC_5));
        _key_map.insert(InputKeyMap::value_type(SDLK_6, KC_6));
        _key_map.insert(InputKeyMap::value_type(SDLK_7, KC_7));
        _key_map.insert(InputKeyMap::value_type(SDLK_8, KC_8));
        _key_map.insert(InputKeyMap::value_type(SDLK_9, KC_9));
        _key_map.insert(InputKeyMap::value_type(SDLK_0, KC_0));
        _key_map.insert(InputKeyMap::value_type(SDLK_MINUS, KC_MINUS));
        _key_map.insert(InputKeyMap::value_type(SDLK_EQUALS, KC_EQUALS));
        _key_map.insert(InputKeyMap::value_type(SDLK_BACKSPACE, KC_BACK));
        _key_map.insert(InputKeyMap::value_type(SDLK_TAB, KC_TAB));
        _key_map.insert(InputKeyMap::value_type(SDLK_q, KC_Q));
        _key_map.insert(InputKeyMap::value_type(SDLK_w, KC_W));
        _key_map.insert(InputKeyMap::value_type(SDLK_e, KC_E));
        _key_map.insert(InputKeyMap::value_type(SDLK_r, KC_R));
        _key_map.insert(InputKeyMap::value_type(SDLK_t, KC_T));
        _key_map.insert(InputKeyMap::value_type(SDLK_y, KC_Y));
        _key_map.insert(InputKeyMap::value_type(SDLK_u, KC_U));
        _key_map.insert(InputKeyMap::value_type(SDLK_i, KC_I));
        _key_map.insert(InputKeyMap::value_type(SDLK_o, KC_O));
        _key_map.insert(InputKeyMap::value_type(SDLK_p, KC_P));
        _key_map.insert(InputKeyMap::value_type(SDLK_RETURN, KC_RETURN));
        _key_map.insert(InputKeyMap::value_type(SDLK_LCTRL, KC_LCONTROL));
        _key_map.insert(InputKeyMap::value_type(SDLK_a, KC_A));
        _key_map.insert(InputKeyMap::value_type(SDLK_s, KC_S));
        _key_map.insert(InputKeyMap::value_type(SDLK_d, KC_D));
        _key_map.insert(InputKeyMap::value_type(SDLK_f, KC_F));
        _key_map.insert(InputKeyMap::value_type(SDLK_g, KC_G));
        _key_map.insert(InputKeyMap::value_type(SDLK_h, KC_H));
        _key_map.insert(InputKeyMap::value_type(SDLK_j, KC_J));
        _key_map.insert(InputKeyMap::value_type(SDLK_k, KC_K));
        _key_map.insert(InputKeyMap::value_type(SDLK_l, KC_L));
        _key_map.insert(InputKeyMap::value_type(SDLK_SEMICOLON, KC_SEMICOLON));
        _key_map.insert(InputKeyMap::value_type(SDLK_COLON, KC_COLON));
        _key_map.insert(InputKeyMap::value_type(SDLK_QUOTE, KC_APOSTROPHE));
        _key_map.insert(InputKeyMap::value_type(SDLK_BACKQUOTE, KC_GRAVE));
        _key_map.insert(InputKeyMap::value_type(SDLK_LSHIFT, KC_LSHIFT));
        _key_map.insert(InputKeyMap::value_type(SDLK_BACKSLASH, KC_BACKSLASH));
        _key_map.insert(InputKeyMap::value_type(SDLK_z, KC_Z));
        _key_map.insert(InputKeyMap::value_type(SDLK_x, KC_X));
        _key_map.insert(InputKeyMap::value_type(SDLK_c, KC_C));
        _key_map.insert(InputKeyMap::value_type(SDLK_v, KC_V));
        _key_map.insert(InputKeyMap::value_type(SDLK_b, KC_B));
        _key_map.insert(InputKeyMap::value_type(SDLK_n, KC_N));
        _key_map.insert(InputKeyMap::value_type(SDLK_m, KC_M));
        _key_map.insert(InputKeyMap::value_type(SDLK_COMMA, KC_COMMA));
        _key_map.insert(InputKeyMap::value_type(SDLK_PERIOD, KC_PERIOD));
        _key_map.insert(InputKeyMap::value_type(SDLK_RSHIFT, KC_RSHIFT));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP_MULTIPLY, KC_MULTIPLY));
        _key_map.insert(InputKeyMap::value_type(SDLK_LALT, KC_LMENU));
        _key_map.insert(InputKeyMap::value_type(SDLK_SPACE, KC_SPACE));
        _key_map.insert(InputKeyMap::value_type(SDLK_CAPSLOCK, KC_CAPITAL));
        _key_map.insert(InputKeyMap::value_type(SDLK_F1, KC_F1));
        _key_map.insert(InputKeyMap::value_type(SDLK_F2, KC_F2));
        _key_map.insert(InputKeyMap::value_type(SDLK_F3, KC_F3));
        _key_map.insert(InputKeyMap::value_type(SDLK_F4, KC_F4));
        _key_map.insert(InputKeyMap::value_type(SDLK_F5, KC_F5));
        _key_map.insert(InputKeyMap::value_type(SDLK_F6, KC_F6));
        _key_map.insert(InputKeyMap::value_type(SDLK_F7, KC_F7));
        _key_map.insert(InputKeyMap::value_type(SDLK_F8, KC_F8));
        _key_map.insert(InputKeyMap::value_type(SDLK_F9, KC_F9));
        _key_map.insert(InputKeyMap::value_type(SDLK_F10, KC_F10));
        _key_map.insert(InputKeyMap::value_type(SDLK_NUMLOCK, KC_NUMLOCK));
        _key_map.insert(InputKeyMap::value_type(SDLK_SCROLLOCK, KC_SCROLL));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP7, KC_NUMPAD7));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP8, KC_NUMPAD8));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP9, KC_NUMPAD9));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP_MINUS, KC_SUBTRACT));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP4, KC_NUMPAD4));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP5, KC_NUMPAD5));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP6, KC_NUMPAD6));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP_PLUS, KC_ADD));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP1, KC_NUMPAD1));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP2, KC_NUMPAD2));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP3, KC_NUMPAD3));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP0, KC_NUMPAD0));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP_PERIOD, KC_DECIMAL));
        _key_map.insert(InputKeyMap::value_type(SDLK_F11, KC_F11));
        _key_map.insert(InputKeyMap::value_type(SDLK_F12, KC_F12));
        _key_map.insert(InputKeyMap::value_type(SDLK_F13, KC_F13));
        _key_map.insert(InputKeyMap::value_type(SDLK_F14, KC_F14));
        _key_map.insert(InputKeyMap::value_type(SDLK_F15, KC_F15));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP_EQUALS, KC_NUMPADEQUALS));
        _key_map.insert(InputKeyMap::value_type(SDLK_KP_DIVIDE, KC_DIVIDE));
        _key_map.insert(InputKeyMap::value_type(SDLK_SYSREQ, KC_SYSRQ));
        _key_map.insert(InputKeyMap::value_type(SDLK_RALT, KC_RMENU));
        _key_map.insert(InputKeyMap::value_type(SDLK_HOME, KC_HOME));
        _key_map.insert(InputKeyMap::value_type(SDLK_UP, KC_UP));
        _key_map.insert(InputKeyMap::value_type(SDLK_PAGEUP, KC_PGUP));
        _key_map.insert(InputKeyMap::value_type(SDLK_LEFT, KC_LEFT));
        _key_map.insert(InputKeyMap::value_type(SDLK_RIGHT, KC_RIGHT));
        _key_map.insert(InputKeyMap::value_type(SDLK_END, KC_END));
        _key_map.insert(InputKeyMap::value_type(SDLK_DOWN, KC_DOWN));
        _key_map.insert(InputKeyMap::value_type(SDLK_PAGEDOWN, KC_PGDOWN));
        _key_map.insert(InputKeyMap::value_type(SDLK_INSERT, KC_INSERT));
        _key_map.insert(InputKeyMap::value_type(SDLK_DELETE, KC_DELETE));
        _key_map.insert(InputKeyMap::value_type(SDLK_LSUPER, KC_LWIN));
        _key_map.insert(InputKeyMap::value_type(SDLK_RSUPER, KC_RWIN));
    }

    SDLInput::~SDLInput()
    {
        SDL_WM_GrabInput(SDL_GRAB_OFF);
        SDL_ShowCursor(1);
    }

    void SDLInput::initialise(RenderWindow* pWindow, bool useKeyboard, bool useMouse, bool useGameController)
    {
        if(useMouse) {
          // Hide the cursor
          SDL_ShowCursor(0);
          SDL_WM_GrabInput(SDL_GRAB_ON);
        }
        warpMouse=useMouse;

        // Get the center and put the mouse there
        int width, height, depth, left, top;
        pWindow->getMetrics(width, height, depth, left, top);

        mMouseX = mMouseCenterX = width / 2;
        mMouseY = mMouseCenterY = height / 2;

        if(warpMouse)
          SDL_WarpMouse(mMouseCenterX, mMouseCenterY);
    }

    void SDLInput::capture()
    {
        // Wait until we're visible again
        if (!_visible)
        {
            SDL_Event event;
            while (SDL_WaitEvent(&event))
            {
                if (event.type == SDL_ACTIVEEVENT && event.active.gain == 1)
                {
                    break;
                }
            }
        }

        if (mUseBufferedKeys)
        {
            processBufferedKeyboard();
        }

        if (mUseBufferedMouse)
        {
            processBufferedMouse();
        }

        SDL_PumpEvents();

        if (!mUseBufferedKeys)
        {
            // Get Keyboard state
            mKeyboardBuffer = SDL_GetKeyState(NULL);
        }

		// NB buffered keyboard not yet supported!
		// TODO
        if (!mUseBufferedMouse)
        {
            mMouseKeys = 0;

            // Get mouse info
            if( SDL_GetAppState() & SDL_APPMOUSEFOCUS )
            {
                mMouseKeys = SDL_GetMouseState( &mMouseX, &mMouseY );
            }

            mMouseState.Xabs = mMouseX;
            mMouseState.Yabs = mMouseY;
            mMouseState.Zabs = 0;

            mMouseState.Xrel = mMouseX - mMouseCenterX;
            mMouseState.Yrel = mMouseY - mMouseCenterY;
            mMouseState.Zrel = 0;

            mMouseState.Buttons = 0;

            for(int i = 0; i < 3; i++ )
                if( mMouseKeys & SDL_BUTTON( i ) )
                    mMouseState.Buttons |= ( 1 << i );

            // XXX Fix me up
            // Game controller state
        }
        if(warpMouse)
          SDL_WarpMouse( mMouseCenterX, mMouseCenterY );
    }

    bool SDLInput::isKeyDown(KeyCode kc) const
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
            return mKeyboardBuffer[SDLK_SEMICOLON];
        case KC_APOSTROPHE:
            return mKeyboardBuffer[SDLK_QUOTE];
        case KC_GRAVE:  /* accent grave */
            return mKeyboardBuffer[SDLK_BACKQUOTE];
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
            return mKeyboardBuffer[SDLK_COLON];
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
            return mKeyboardBuffer[SDLK_SYSREQ] | mKeyboardBuffer[SDLK_PRINT];
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

        return 0;
    }

    long SDLInput::getMouseRelX() const
    {
        return mMouseX - mMouseCenterX;
    }

    long SDLInput::getMouseRelY() const
    {
        return mMouseY - mMouseCenterY;
    }

    long SDLInput::getMouseRelZ() const
    {
        return 0;
    }

    long SDLInput::getMouseAbsX() const
    {
        return mMouseX;
    }

    long SDLInput::getMouseAbsY() const
    {
        return mMouseY;
    }

    long SDLInput::getMouseAbsZ() const
    {
        return 0;
    }

    bool SDLInput::getMouseButton( uchar button ) const
    {
        return mMouseState.isButtonDown( button );
    }

    void SDLInput::processBufferedKeyboard()
    {
        // XXX Arbitrarily picked 16 
        SDL_Event events[16];

        int count = SDL_PeepEvents(events, 16, SDL_GETEVENT,
                (SDL_KEYDOWNMASK | SDL_KEYUPMASK));
        if (!count)
        {
            return;
        }

        for (int i = 0; i < count; i++)
        {
            bool down = false;
            if (events[i].type == SDL_KEYDOWN)
                down = true;
            keyChanged(_key_map[events[i].key.keysym.sym], down);
        }
    }

    void SDLInput::processBufferedMouse()
    {
        // XXX Arbitrarily picked 16 
        SDL_Event events[16];

        int count = SDL_PeepEvents(events, 16, SDL_GETEVENT,
                (SDL_MOUSEMOTIONMASK | SDL_MOUSEBUTTONDOWNMASK |
                 SDL_MOUSEBUTTONUPMASK | SDL_ACTIVEEVENTMASK));
        if (!count)
        {
            return;
        }

        bool Xset, Yset, Zset;
        Xset = Yset = Zset = false;
        for (int i = 0; i < count; i++)
        {
            int button_mask = -1;
            bool button_down = false;
            switch (events[i].type)
            {
            case SDL_ACTIVEEVENT:
                _visible = events[i].active.gain ? true : false;
                break;
            case SDL_MOUSEMOTION:
                if (events[i].motion.x == mMouseCenterX && 
                        events[i].motion.y == mMouseCenterY)
                {
                    // it moved to the center, probably a warp, ignore it
                    continue;
                }

                if (events[i].motion.xrel)
                {
                    if (Xset)
                    {
                        mouseMoved();
                    }
                    mCursor->addToX(events[i].motion.xrel * mScale);
                    Xset = true;
                }
                if (events[i].motion.yrel)
                {
                    if (Yset)
                    {
                        mouseMoved();
                    }
                    mCursor->addToY(events[i].motion.yrel * mScale);
                    Yset = true;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                button_down = true;
            case SDL_MOUSEBUTTONUP:
                switch(events[i].button.button)
                {
                case SDL_BUTTON_LEFT:
                    button_mask = InputEvent::BUTTON0_MASK;
                    break;
                case SDL_BUTTON_RIGHT:
                    button_mask = InputEvent::BUTTON1_MASK;
                    break;
                case SDL_BUTTON_MIDDLE:
                    button_mask = InputEvent::BUTTON2_MASK;
                    break;
                };
                triggerMouseButton(button_mask, button_down);
                break;
            };
            
            if (Xset && Yset)
            {
                mouseMoved();
                Xset = Yset = false;
            }
        }

        if (Xset || Yset)
        {
            mouseMoved();
        }
                    
    }

    void SDLInput::getMouseState( MouseState& state ) const
    {
        memcpy( &state, &mMouseState, sizeof( MouseState ) );
    }

}

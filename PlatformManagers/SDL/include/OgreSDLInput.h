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


#ifndef __SDLInputReader_H__
#define __SDLInputReader_H__

#include "OgreInput.h"
#include "OgreInputEvent.h"
#include "OgreRenderWindow.h"

#include <map>

#include <SDL.h>

namespace Ogre {
    class SDLInput : public InputReader
    {
    public:
        SDLInput();
        virtual ~SDLInput();

        void initialise( RenderWindow* pWindow, bool useKeyboard = true, bool useMouse = true, bool useGameController = false );
        void capture();

        bool isKeyDown( KeyCode kc ) const;

        /*
         * Mouse getters
         */
        virtual long getMouseRelX() const;
        virtual long getMouseRelY() const;
        virtual long getMouseRelZ() const;

        virtual long getMouseAbsX() const;
        virtual long getMouseAbsY() const;
        virtual long getMouseAbsZ() const;

        virtual void getMouseState( MouseState& state ) const;

        virtual bool getMouseButton( uchar button ) const;

    private:
        // State at last 'capture' call
        Uint8* mKeyboardBuffer;
        int mMaxKey;
        int mMouseX, mMouseY;
        int mMouseRelativeX, mMouseRelativeY, mMouseRelativeZ;
        Real mScale;
        Uint8 mMouseKeys;
        bool _visible;
        typedef std::map<SDLKey, KeyCode> InputKeyMap;
        InputKeyMap _key_map;
        bool warpMouse;

        static const unsigned int mWheelStep = 60;
        void processBufferedKeyboard();
        void processBufferedMouse();
    };
}

#endif


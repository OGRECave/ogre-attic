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


#ifndef __SDLInputReader_H__
#define __SDLInputReader_H__

#include "OgreInput.h"
#include "OgreRenderWindow.h"

#include <SDL.h>

namespace Ogre {
    class SDLInput : public InputReader
    {
    public:
        SDLInput();
        virtual ~SDLInput();

        void initialise(RenderWindow* pWindow, bool useKeyboard = true, bool useMouse = true, bool useGameController = false);
        void capture(void);
        bool isKeyDown(KeyCode kc);
        int getMouseRelativeX(void);
        int getMouseRelativeY(void);
    private:
        // State at last 'capture' call
        Uint8* mKeyboardBuffer;
        int mMaxKey;
        int mMouseX, mMouseY;
        int mMouseCenterX, mMouseCenterY;
        Uint8 mMouseKeys;
    };
}

#endif


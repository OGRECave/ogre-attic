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

#ifndef __SDLWindow_H__
#define __SDLWindow_H__

#include "OgreSDLPrerequisites.h"
#include "OgreRenderWindow.h"

#include <SDL.h>

namespace Ogre {
    class SDLWindow : public RenderWindow
    {
    private:
        SDL_Surface* mScreen;
        bool mActive;
        bool mClosed;
            
        // Process pending events
        void processEvents(void);
    public:
        SDLWindow();
        ~SDLWindow();

        void create(const String& name, int width, int height, int colourDepth,
            bool fullScreen, int left, int top, bool depthBuffer, void* miscParam, ...);
        /** Overridden - see RenderWindow */
        void destroy(void);
        /** Overridden - see RenderWindow */
        bool isActive(void);
        /** Overridden - see RenderWindow */
        bool isClosed(void);
        /** Overridden - see RenderWindow */
        void reposition(int left, int top);
        /** Overridden - see RenderWindow */
        void resize(int width, int height);
        /** Overridden - see RenderWindow */
        void swapBuffers(bool waitForVSync);

        void outputText(int x, int y, const String& text);
        /** Overridden - see RenderTarget.
        */
        void writeContentsToFile(const String& filename);

		bool requiresTextureFlipping() const { return false; }
    };
}

#endif


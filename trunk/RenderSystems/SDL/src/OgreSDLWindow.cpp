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

#include "OgreSDLWindow.h"

namespace Ogre {

    SDLWindow::SDLWindow() :
        mScreen(NULL), mActive(false), mClosed(false)
    {
    }

    SDLWindow::~SDLWindow()
    {
        if (mScreen != NULL)
            SDL_FreeSurface(mScreen);
    }

    void SDLWindow::create(String name, int width, int height, int colourDepth,
                           bool fullScreen, int left, int top, bool depthBuffer,
                           void* miscParam, ...)
    {
        fprintf(stderr, "SDLWindow::create\n");
        SDL_Surface* screen;
        int flags = SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE;

        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

        if (fullScreen)
            flags |= SDL_FULLSCREEN;

        fprintf(stderr, "Create window\n");
        screen = SDL_SetVideoMode(width, height, colourDepth, flags);
        if (!screen)
        {
            fprintf(stderr, "Could not make screen\n");
            exit(1);
        }
        fprintf(stderr, "screen is valid\n");
        mScreen = screen;

        mWidth = width;
        mHeight = height;

        mActive = true;

        if (!fullScreen)
            SDL_WM_SetCaption("OGRE Render Window", 0);

    }

    void SDLWindow::destroy(void)
    {
        SDL_FreeSurface(mScreen);
        mScreen = NULL;
    }

    bool SDLWindow::isActive()
    {
        return mActive;
    }

    bool SDLWindow::isClosed()
    {
        return mClosed;
    }

    void SDLWindow::reposition(int left, int top)
    {
        // XXX FIXME
    }

    void SDLWindow::resize(int width, int height)
    {
        ViewportList::iterator it = mViewportList.begin();
        while (it != mViewportList.end())
            (*it).second->_updateDimensions();
        // XXX FIXME
    }

    void SDLWindow::swapBuffers(bool waitForVSync)
    {
        SDL_GL_SwapBuffers();
        // XXX More?
    }

    void SDLWindow::outputText(int x, int y, const String& text)
    {
        //XXX FIXME
    }
}


/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright � 2000-2002 The OGRE Team
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
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreException.h"
#include "OgreLogManager.h"

#if OGRE_PLATFORM == PLATFORM_WIN32
#   include <windows.h>
#   include <wingdi.h>
#   include <GL/gl.h>
#   define GL_GLEXT_PROTOTYPES
#   include "glprocs.h"
#   include <GL/glu.h>
#elif OGRE_PLATFORM == PLATFORM_LINUX
#   include <GL/gl.h>
#   include <GL/glu.h>
#elif OGRE_PLATFORM == PLATFORM_APPLE
#   include <OpenGL/gl.h>
#   define GL_EXT_texture_env_combine 1
#   include <OpenGL/glext.h>
#   include <OpenGL/glu.h>
#endif

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

    void SDLWindow::create(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth,
                           bool fullScreen, int left, int top, bool depthBuffer,
                           void* miscParam, ...)
    {
        LogManager::getSingleton().logMessage("SDLWindow::create", LML_TRIVIAL);
        SDL_Surface* screen;
        int flags = SDL_OPENGL | SDL_HWPALETTE;
		
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
        // request good stencil size if 32-bit colour
        if (colourDepth == 32 && depthBuffer)
        {
            SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8);
        }
		
        if (fullScreen)
            flags |= SDL_FULLSCREEN;

        LogManager::getSingleton().logMessage("Create window", LML_TRIVIAL);
        screen = SDL_SetVideoMode(width, height, colourDepth, flags);
        if (!screen)
        {
            LogManager::getSingleton().logMessage(LML_CRITICAL, "Could not make screen: %s.", SDL_GetError());
            exit(1);
        }
        LogManager::getSingleton().logMessage("screen is valid", LML_TRIVIAL);
        mScreen = screen;

        mName = name;

        mWidth = width;
        mHeight = height;

        mActive = true;

        if (!fullScreen)
            SDL_WM_SetCaption(name.c_str(), 0);

    }

    void SDLWindow::destroy(void)
    {
        SDL_FreeSurface(mScreen);
        mScreen = NULL;
        mActive = false;

        Root::getSingleton().getRenderSystem()->detachRenderTarget( this->getName() );
    }

    bool SDLWindow::isActive() const
    {
        return mActive;
    }

    bool SDLWindow::isClosed() const
    {
        return mClosed;
    }

    void SDLWindow::reposition(int left, int top)
    {
        // XXX FIXME
    }

    void SDLWindow::resize(unsigned int width, unsigned int height)
    {
        for (ViewportList::iterator it = mViewportList.begin();
             it != mViewportList.end(); ++it)
        {
            (*it).second->_updateDimensions();
        }
    }

    void SDLWindow::swapBuffers(bool waitForVSync)
    {
        SDL_GL_SwapBuffers();
        // XXX More?
    }

	void SDLWindow::writeContentsToFile(const String& filename)
	{
		ImageCodec::ImageData imgData;
		imgData.width = mWidth;
		imgData.height = mHeight;
		imgData.format = PF_R8G8B8;

		// Allocate buffer 
		uchar* pBuffer = new uchar[mWidth * mHeight * 3];

		// Read pixels
		// I love GL: it does all the locking & colour conversion for us
		glReadPixels(0,0, mWidth-1, mHeight-1, GL_RGB, GL_UNSIGNED_BYTE, pBuffer);

		// Wrap buffer in a chunk
		DataChunk chunk(pBuffer, mWidth * mHeight * 3);

		// Need to flip the read data over in Y though
		Image img;
		img.loadRawData(chunk, mWidth, mHeight, PF_R8G8B8 );
		img.flipAroundX();

		DataChunk chunkFlipped(img.getData(), chunk.getSize());

		// Get codec 
		size_t pos = filename.find_last_of(".");
		String extension;
		if( pos == String::npos )
			Except(
			Exception::ERR_INVALIDPARAMS, 
			"Unable to determine image type for '" + filename + "' - invalid extension.",
			"SDLWindow::writeContentsToFile" );

		while( pos != filename.length() - 1 )
			extension += filename[++pos];

		// Get the codec
		Codec * pCodec = Codec::getCodec(extension);

		// Write out
		pCodec->codeToFile(chunkFlipped, filename, &imgData);

		delete [] pBuffer;


	}
}

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreGLXWindow.h"
#include "OgreRoot.h"
#include "OgreGLRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreGLXUtils.h"
#include "OgreWindowEventUtilities.h"

#include <iostream>
#include <algorithm>
#include <sys/time.h>
#include <climits>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#ifndef NO_XRANDR
#include <X11/extensions/Xrandr.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>


namespace Ogre
{

//-------------------------------------------------------------------------------------------------//
GLXWindow::GLXWindow (Display *display) :
	mDisplay (display), mWindow (0), mGlxContext (0), mFBConfig (0),
	mGlxWindow (0), mDelWindow (false), mDelContext (false), mDelGlxWindow (false),
	mClosed (false), mVisible (true), mFullScreen (false),
	mOldMode (-1), mOldRefreshRate (0), mContext (0)
{
	mActive = false;
}

//-------------------------------------------------------------------------------------------------//
GLXWindow::~GLXWindow()
{
	destroy ();
}

//-------------------------------------------------------------------------------------------------//
void GLXWindow::create (const String& name, unsigned int width, unsigned int height,
						bool fullScreen, const NameValuePairList *miscParams)
{
	String title = name;
	size_t fsaa_samples = 0;

	// Create the window on the default screen of the display,
	// unless a pre-created window has been passed
	Screen *screen = DefaultScreenOfDisplay (mDisplay);
	// Unless parentWindowHandle is given in miscParams we're top-level
	mTopLevel = true;
	// And unless told otherwise, we're the owners of the window
	mDelWindow = true;
	// Same about our GLXContext
	mDelContext = true;
	// Same about the drawable
	mDelGlxWindow = true;
	// No explicit parent window for now
	Window parentWindow = 0;
	// Window position - undefined for now
	size_t left = size_t (-1), top = size_t (-1);
	// The fbconfig id to use
	int fbconfig_id = GLX_DONT_CARE;
	// Refresh rate for fullscreen videomode
	int refresh_rate = 0;

	if (miscParams)
	{
		// Parse miscellenous parameters
		NameValuePairList::const_iterator opt;
		opt = miscParams->find ("displayFrequency");
		if (opt != miscParams->end ())
			refresh_rate = StringConverter::parseUnsignedInt (opt->second);
		// Full screen anti aliasing
		opt = miscParams->find ("FSAA");
		if (opt != miscParams->end ())
			fsaa_samples = StringConverter::parseUnsignedInt (opt->second);
		// left (x)
		opt = miscParams->find ("left");
		if (opt != miscParams->end ())
			left = StringConverter::parseUnsignedInt (opt->second);
		// top (y)
		opt = miscParams->find ("top");
		if (opt != miscParams->end ())
			top = StringConverter::parseUnsignedInt (opt->second);
		// Window title
		opt = miscParams->find ("title");
		if (opt != miscParams->end ())
			title = opt->second;

		opt = miscParams->find("parentWindowHandle");
		if (opt != miscParams->end ())
		{
			// Creating OGRE window in a parent window
			std::vector<String> tokens = StringUtil::split (opt->second, " :");

			// Now set things to their correct values
			// This must be the ugliest line of code I have ever written :P
			mDisplay = reinterpret_cast<Display*> (StringConverter::parseUnsignedLong (tokens [0]));
			parentWindow = StringConverter::parseUnsignedLong (tokens [1]);

			XWindowAttributes wa;
			if (XGetWindowAttributes (mDisplay, parentWindow, &wa))
				screen = wa.screen;
		}

		opt = miscParams->find("externalWindowHandle");
		if (opt != miscParams->end ()) // embedding OGRE in already created window
		{
			std::vector<String> tokens = StringUtil::split(opt->second, " :");

			mDisplay = reinterpret_cast<Display*> (StringConverter::parseUnsignedLong (tokens [0]));
			mWindow = StringConverter::parseUnsignedLong (tokens [1]);
			mDelWindow = false;

			XWindowAttributes wa;
			if (XGetWindowAttributes (mDisplay, mWindow, &wa))
				screen = wa.screen;
		}

		// The target fbconfig id
		opt = miscParams->find ("fbconfigid");
		if (opt != miscParams->end ())
			fbconfig_id = StringConverter::parseUnsignedInt (opt->second);
		// The GLXContext has been already created
		opt = miscParams->find ("glxcontext");
		if (opt != miscParams->end ())
		{
			std::vector<String> tokens = StringUtil::split (opt->second, " :");

			mGlxContext = reinterpret_cast< ::GLXContext> (StringConverter::parseUnsignedLong (tokens [0]));
			if (mGlxContext)
			{
				mDelContext = false;
				// User can also specify the drawable, in which case we'll use it
				if (tokens.size () > 1)
				{
					mGlxWindow = StringConverter::parseUnsignedLong (tokens [1]);
					if (mGlxWindow)
					{
						mDelGlxWindow = false;
						// If user has't specified a window, query the correct screen for the drawable
						if (!mWindow)
						{
							XWindowAttributes wa;
							if (XGetWindowAttributes (mDisplay, mGlxWindow, &wa))
								screen = wa.screen;
						}
					}
				}
				// Get the right fbconfig from the context
				glXQueryContext (mDisplay, mGlxContext, GLX_FBCONFIG_ID, &fbconfig_id);
			}
		}
	}

	// Fullscreen windows is always in the top-left corner
	if (fullScreen)
		left = top = 0;

	int depth = DefaultDepthOfScreen (screen);
	Window rootWindow = RootWindowOfScreen (screen);

	if (!mWindow && !parentWindow)
	{
		// Make sure the window is centered if no left and top in parameters
		if (left == size_t (-1))
			left = (WidthOfScreen (screen) - width) / 2;
		if (top == size_t (-1))
			top = (HeightOfScreen (screen) - height) / 2;
	}
	else
	{
		fullScreen = false; // Can't be full screen if embedded in an app!
		mTopLevel = false;  // Can't be top-level if embedded
	}

	if (!parentWindow)
		parentWindow = rootWindow;

	// Find out the ordinal number of the screen
	int screen_no = 0;
	while (screen_no < ScreenCount (mDisplay))
		if (ScreenOfDisplay (mDisplay, screen_no++) == screen)
			break;

	if (screen_no >= ScreenCount (mDisplay))
		OGRE_EXCEPT (Exception::ERR_NOT_IMPLEMENTED,
					 "The specified window is not on the specified display",
					 "GLXWindow::create");

#ifndef NO_XRANDR
	// Attempt mode switch for fullscreen -- only if RANDR extension is there
	int dummy;
	if (fullScreen && !XQueryExtension (mDisplay, "RANDR", &dummy, &dummy, &dummy))
	{
		LogManager::getSingleton().logMessage("GLXWindow::create -- Could not switch to full screen mode: No XRANDR extension found");
	}
	else if (fullScreen)
	{
		// Use Xrandr extension to switch video modes. This is much better than
		// XVidMode as you can't scroll away from the full-screen applications.

		// Get current screen info
		XRRScreenConfiguration *config = XRRGetScreenInfo (mDisplay, rootWindow);
		if (config)
		{
			// Get available sizes
			int nsizes;
			XRRScreenSize *sizes = XRRConfigSizes (config, &nsizes);
			// Get current size and rotation
			Rotation current_rotation;
			mOldMode = XRRConfigCurrentConfiguration (config, &current_rotation);
			mOldRefreshRate = XRRConfigCurrentRate (config);
			// Find smallest matching mode
			int mode = -1;
			int mode_width = INT_MAX;
			int mode_height = INT_MAX;
			for (int i = 0; i < nsizes; i++)
			{
				if(sizes[i].width >= static_cast<int>(width) &&
				   sizes[i].height >= static_cast<int>(height) &&
				   (sizes[i].width < mode_width ||
					sizes[i].height < mode_height))
				{
					mode = i;
					mode_width = sizes[i].width;
					mode_height = sizes[i].height;
				}
			}
			if(mode >= 0)
			{
				// Finally, set the screen configuration
				LogManager::getSingleton().logMessage("GLXWindow::create -- Entering full screen mode");
				if (refresh_rate)
					XRRSetScreenConfigAndRate (mDisplay, config, rootWindow, mode, current_rotation, refresh_rate, CurrentTime);
				else
					XRRSetScreenConfig (mDisplay, config, rootWindow, mode, current_rotation, CurrentTime);
			}
			else
				LogManager::getSingleton().logMessage("GLXWindow::create -- Could not switch to full screen mode: No conforming mode was found");

			// Free configuration data
			XRRFreeScreenConfigInfo (config);
		}
		else
			LogManager::getSingleton().logMessage("GLXWindow::create -- Could not switch to full screen mode: XRRGetScreenInfo failed");
	}
#endif

	// The minimum requirements for a valie Ogre GL context
	static int reqattr [] =
	{
		// This is used if we need a specific fbid
		GLX_FBCONFIG_ID, GLX_DONT_CARE,
		// No overlays or stereo contexts as well
		GLX_LEVEL, 0, GLX_STEREO, GL_FALSE,
		// Double buffering is a basic requirement for 3D graphics
		GLX_DOUBLEBUFFER, GL_TRUE,
		// At least 16-bit z-buffer and 1-bit stencil
		GLX_DEPTH_SIZE, 16, GLX_STENCIL_SIZE, 1,
		// No indexed visuals
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		// Make sure the visual can be used in a X window
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT | GLX_PBUFFER_BIT,
		GLX_X_RENDERABLE, GL_TRUE,
		None
	};

	/* The desired ideal requirements for the GL context */
	static int idealattr [] =
	{
		// This is used to enable antialiasing
		GLX_SAMPLE_BUFFERS_ARB, 0, GLX_SAMPLES_ARB, 0,
		// R8G8B8A8
		GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_ALPHA_SIZE, 8,
		// A 24-bit depth buffer
		GLX_DEPTH_SIZE, 24,
		// 256 distinct stencil values
		GLX_STENCIL_SIZE, 8,
		// No accumulation buffers
		GLX_ACCUM_RED_SIZE, 0, GLX_ACCUM_GREEN_SIZE, 0,
		GLX_ACCUM_BLUE_SIZE, 0, GLX_ACCUM_ALPHA_SIZE, 0,
		None
	};

	// Use a specific fbconfig if requested
	reqattr [1] = fbconfig_id;
	idealattr [1] = fsaa_samples ? GL_TRUE : GL_FALSE;
	idealattr [3] = fsaa_samples;
	mFBConfig = GLXUtils::findBestMatch (mDisplay, screen_no, reqattr, idealattr);
	if (!mFBConfig)
		OGRE_EXCEPT (Exception::ERR_RENDERINGAPI_ERROR,
					 "Failed to find a suitable GLXFBConfig",
					 "GLXWindow::create");

	LogManager::getSingleton().logMessage (
		LML_TRIVIAL, "Using GL context format: " + GLXUtils::FBConfigToString (mDisplay, mFBConfig));

	if (mWindow)
		// Don't create the window if user provided a window id
		LogManager::getSingleton().logMessage("GLXWindow::create -- using pre-created window");
	else
	{
		XVisualInfo *xvi = glXGetVisualFromFBConfig (mDisplay, mFBConfig);
		if (!xvi)
			OGRE_EXCEPT (Exception::ERR_RENDERINGAPI_ERROR,
						 "No XVisualInfo corresponding to GLXFBConfig",
						 "GLXWindow::create");

		XSetWindowAttributes attr;
		unsigned long mask;
		attr.background_pixel = 0;
		attr.border_pixel = 0;
		attr.colormap = XCreateColormap (mDisplay, rootWindow, xvi->visual, AllocNone);
		attr.event_mask = StructureNotifyMask | VisibilityChangeMask;
		if (fullScreen)
		{
			mask = CWBackPixel | CWColormap | CWOverrideRedirect | CWSaveUnder | CWBackingStore | CWEventMask;
			attr.override_redirect = True;
			attr.backing_store = NotUseful;
			attr.save_under = False;
		}
		else
			mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

		// Create window on server
		mWindow = XCreateWindow (mDisplay, parentWindow,
								 left, top, width, height,
								 0, xvi->depth, InputOutput,
								 xvi->visual, mask, &attr);
		if(!mWindow)
			OGRE_EXCEPT (Exception::ERR_RENDERINGAPI_ERROR,
						 "XCreateWindow failed",
						 "GLXWindow::create");

		// Make sure the window is in normal state
		XWMHints *wm_hints;
		if ((wm_hints = XAllocWMHints ()) != NULL)
		{
			wm_hints->initial_state = NormalState;
			wm_hints->input = True;
			wm_hints->flags = StateHint | InputHint;

			// Check if we can give it an icon
			if (depth == 24 || depth == 32)
			{
				// Woot! The right bit depth, we can load an icon
				static const String icon ("GLX_icon.png");
				if (ResourceGroupManager::getSingleton ().resourceExists (
					ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, icon))
					if (GLXUtils::LoadIcon (mDisplay, rootWindow, icon,
											&wm_hints->icon_pixmap, &wm_hints->icon_mask))
						wm_hints->flags |= IconPixmapHint | IconMaskHint;
			}
		}

		// Set size and location hints
		XSizeHints *size_hints;
		if ((size_hints = XAllocSizeHints ()) != NULL)
		{
			// Otherwise some window managers ignore our position request
			size_hints->flags = USPosition;
		}

		// Make text property from title
		XTextProperty titleprop;
		char *lst = (char *)title.c_str ();
		XStringListToTextProperty((char **)&lst, 1, &titleprop);

		XSetWMProperties (mDisplay, mWindow, &titleprop, NULL, NULL, 0, size_hints, wm_hints, NULL);

		// We don't like memory leaks. Free the clientside storage, but not the
		// pixmaps as they're still being used by the server.
		XFree (titleprop.value);
		XFree (wm_hints);
		XFree (size_hints);
		XFree (xvi);

		// Map window unto screen and focus it.
		XMapWindow (mDisplay, mWindow);

		// Acquire atom to recognize window close events
		mAtomDeleteWindow = XInternAtom (mDisplay, "WM_DELETE_WINDOW", False);
		XSetWMProtocols (mDisplay, mWindow, &mAtomDeleteWindow, 1);

		// Register only Ogre created windows (users can register their own)
		WindowEventUtilities::_addRenderWindow (this);

		// Make sure the server is up to date and focus the window
		XFlush (mDisplay);
	}

	if (!mGlxContext)
	{
		GLRenderSystem *rs = static_cast<GLRenderSystem*>(Root::getSingleton ().getRenderSystem ());
		GLXContext *mainContext = static_cast<GLXContext*>(rs->_getMainContext ());
		// Finally, create a GL context: we want to share it with main
		mGlxContext = glXCreateNewContext (mDisplay, mFBConfig, GLX_RGBA_TYPE,
										   mainContext ? mainContext->mContext : NULL, True);
		if (!mGlxContext)
			OGRE_EXCEPT (Exception::ERR_RENDERINGAPI_ERROR,
						 "glXCreateNewContext failed", "GLXWindow::create");
	}

	mName = name;
	mWidth = width;
	mHeight = height;
	mFullScreen = fullScreen;

	// Get the real window attributes
	XWindowAttributes temp;
	XGetWindowAttributes (mDisplay, mWindow, &temp);
	mLeft = temp.x;
	mTop = temp.y;

	if (!mGlxWindow)
	{
		// Create the GLX Window drawable
		mGlxWindow = glXCreateWindow (mDisplay, mFBConfig, mWindow, NULL);
		if (!mGlxWindow)
			OGRE_EXCEPT (Exception::ERR_RENDERINGAPI_ERROR,
						 "glXCreateWindow failed", "GLXWindow::create");
	}

	// Create OGRE GL context
	mContext = new GLXContext (mDisplay, mGlxWindow, mGlxContext, mFBConfig);

	mActive = true;
}

//-------------------------------------------------------------------------------------------------//
void GLXWindow::destroy ()
{
	WindowEventUtilities::_removeRenderWindow (this);

	// Unregister and destroy OGRE GLContext
	delete mContext;

	if (mGlxWindow && mDelGlxWindow)
		glXDestroyWindow (mDisplay, mGlxWindow);

	if (mGlxContext && mDelContext)
		glXDestroyContext (mDisplay, mGlxContext);

	if (mWindow && mDelWindow)
		XDestroyWindow (mDisplay, mWindow);

	mContext = 0;
	mWindow = 0;
	mGlxWindow = 0;
	mGlxContext = 0;
	mDelContext = mDelWindow = false;
	mActive = false;
	mVisible = false;
	mClosed = true;

	Root::getSingleton ().getRenderSystem ()->detachRenderTarget (this->getName ());

#ifndef NO_XRANDR
	if (mFullScreen)
	{
		// Restore original video mode.
		Window rootWindow = DefaultRootWindow (mDisplay);
		XRRScreenConfiguration *config;

		// Get current screen info
		config = XRRGetScreenInfo (mDisplay, rootWindow);
		if (config)
		{
			Rotation current_rotation;
			XRRConfigCurrentConfiguration (config, &current_rotation);
			LogManager::getSingleton ().logMessage (
				"Leaving full screen mode");
			if (mOldRefreshRate)
				XRRSetScreenConfigAndRate (mDisplay, config, rootWindow, mOldMode, current_rotation, mOldRefreshRate, CurrentTime);
			else
				XRRSetScreenConfig (mDisplay, config, rootWindow, mOldMode, current_rotation, CurrentTime);
			XRRFreeScreenConfigInfo (config);
		}
		else
			LogManager::getSingleton ().logMessage (
				"GLXWindow::destroy -- Could not switch from full screen mode: XRRGetScreenInfo failed");
	}
#endif
}

//-------------------------------------------------------------------------------------------------//
bool GLXWindow::isActive () const
{
	return mActive;
}

//-------------------------------------------------------------------------------------------------//
bool GLXWindow::isClosed () const
{
	return mClosed;
}

//-------------------------------------------------------------------------------------------------//
bool GLXWindow::isVisible () const
{
	return mVisible;
}

//-------------------------------------------------------------------------------------------------//
void GLXWindow::setVisible (bool visible)
{
	mVisible = visible;
}

//-------------------------------------------------------------------------------------------------//
void GLXWindow::reposition (int left, int top)
{
	XMoveWindow (mDisplay, mWindow, left, top);
}

//-------------------------------------------------------------------------------------------------//
void GLXWindow::resize (unsigned int width, unsigned int height)
{
	// Check if the window size really changed
	if (mWidth == width && mHeight == height)
		return;

	mWidth = width;
	mHeight = height;

	if (!mTopLevel)
	{
		for (ViewportList::iterator it = mViewportList.begin (); it != mViewportList.end (); ++it)
			(*it).second->_updateDimensions ();
	}
	else
	{
		XResizeWindow (mDisplay, mWindow, width, height); /// Ogre handles window
	}
}

//-------------------------------------------------------------------------------------------------//
void GLXWindow::windowMovedOrResized()
{
	//Get the new attributes of the screen
	XWindowAttributes temp;
	XGetWindowAttributes(mDisplay, mWindow, &temp);
	mLeft = temp.x;
	mTop  = temp.y;

	//Only update viewport dimensions if they did actually change
	if (static_cast<int>(mWidth) == temp.width && static_cast<int>(mHeight) == temp.height)
		return;

	mWidth = temp.width;
	mHeight = temp.height;

	// Notify viewports of resize
	ViewportList::iterator it, itend;
	itend = mViewportList.end();
	for( it = mViewportList.begin(); it != itend; ++it )
		(*it).second->_updateDimensions();
}

//-------------------------------------------------------------------------------------------------//
void GLXWindow::swapBuffers(bool waitForVSync)
{
	glXSwapBuffers (mDisplay, mGlxWindow);
}

//-------------------------------------------------------------------------------------------------//
void GLXWindow::getCustomAttribute( const String& name, void* pData )
{
	if (name == "DISPLAY")
	{
		*static_cast<Display**>(pData) = mDisplay;
		return;
	}
	else if (name == "ATOM")
	{
		*static_cast< ::Atom* >(pData) = mAtomDeleteWindow;
		return;
	}
	else if (name == "WINDOW")
	{
		*static_cast<Window*>(pData) = mWindow;
		return;
	}
	else if (name == "GLXWINDOW")
	{
		*static_cast< ::GLXWindow*>(pData) = mGlxWindow;
		return;
	}
	else if (name == "GLXCONTEXT")
	{
		*static_cast< ::GLXContext*>(pData) = mGlxContext;
		return;
	}
	else if (name == "GLCONTEXT")
	{
		*static_cast<GLXContext**>(pData) = mContext;
		return;
	}
}

//-------------------------------------------------------------------------------------------------//
void GLXWindow::writeContentsToFile(const String& filename)
{
	ImageCodec::ImageData* imgData = new ImageCodec::ImageData;
	imgData->width = mWidth;
	imgData->height = mHeight;
	imgData->format = PF_BYTE_RGB;

	// Allocate buffer
	uchar* pBuffer = new uchar[mWidth * mHeight * 3];

	// Read pixels
	// I love GL: it does all the locking & colour conversion for us
	glReadPixels(0,0, mWidth-1, mHeight-1, GL_RGB, GL_UNSIGNED_BYTE, pBuffer);

	// Wrap buffer in a memory stream
	DataStreamPtr stream(new MemoryDataStream(pBuffer, mWidth * mHeight * 3, false));

	// Need to flip the read data over in Y though
	Image img;
	img.loadRawData(stream, mWidth, mHeight, PF_BYTE_RGB );
	img.flipAroundX();

	MemoryDataStreamPtr streamFlipped(new MemoryDataStream(img.getData(), stream->size(), false));

	// Get codec
	size_t pos = filename.find_last_of(".");
	String extension;
	if( pos == String::npos )
		OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Unable to determine image type for '"
			+ filename + "' - invalid extension.", "SDLWindow::writeContentsToFile" );

	while( pos != filename.length() - 1 )
		extension += filename[++pos];

	// Get the codec
	Codec * pCodec = Codec::getCodec(extension);

	// Write out
	Codec::CodecDataPtr codecDataPtr(imgData);
	pCodec->codeToFile(streamFlipped, filename, codecDataPtr);

	delete [] pBuffer;
}

}

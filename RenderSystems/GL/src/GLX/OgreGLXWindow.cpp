/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
 
Copyright © 2000-2004 The OGRE Team
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

#include "OgreGLXWindow.h"
#include "OgreRoot.h"
#include "OgreRenderSystem.h"
#include "OgreImageCodec.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

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


namespace Ogre {

GLXWindow::GLXWindow(Display *display) :
		mDisplay(display),
		mWindow(0),
		mGlxContext(0),
mActive(false), mClosed(false),mFullScreen(false), mOldMode(-1) {}

GLXWindow::~GLXWindow() {
	if(mGlxContext)
		glXDestroyContext(mDisplay, mGlxContext);
	if(mWindow)
		XDestroyWindow(mDisplay, mWindow);
#ifndef NO_XRANDR

	if(mFullScreen) {
		// Restore original video mode.
		Window rootWindow = DefaultRootWindow(mDisplay);
		XRRScreenConfiguration *config;

		// Get current screen info
		config = XRRGetScreenInfo(mDisplay, rootWindow);
		if(config) {
			Rotation current_rotation;
			XRRConfigCurrentConfiguration (config, &current_rotation);
			//std::cerr << "Restore mode " << mOldMode << std::endl;
			LogManager::getSingleton().logMessage("GLXWindow::~GLXWindow -- Leaving full screen mode");
			XRRSetScreenConfig(mDisplay, config, rootWindow, mOldMode, current_rotation, CurrentTime);
			XRRFreeScreenConfigInfo(config);
		} else {
			LogManager::getSingleton().logMessage("GLXWindow::~GLXWindow -- Could not switch from full screen mode: XRRGetScreenInfo failed");
		}
	}
#endif
}

namespace {

/**
 * Loads an icon from an Ogre resource into the X Server. This currently only
 * works for 24 and 32 bit displays. The image must be findable by the Ogre
 * resource system, and of format PF_A8R8G8B8.
 *
 * @param mDisplay,rootWindow	X resources to use
 * @param name			Name of image to load
 * @param pix			Receiver for the output pixmap
 * @param mask			Receiver for the output mask (alpha bitmap)
 * @returns true on success
 */
bool LoadIcon(Display *mDisplay, Window rootWindow, const std::string &name, Pixmap *pix, Pixmap *mask) {
	Image img;
	int mWidth, mHeight;
	char *data, *bitmap;
	try {
		// Try to load image
		img.load(name);
		mWidth = img.getWidth();
		mHeight = img.getHeight();
		if(img.getFormat() != PF_A8R8G8B8)
			// Image format must be RGBA
			return 0;
	} catch(Exception &e) {
		// Could not find image; never mind
		return false;
	}

	// Allocate space for image data
	data = (char*)malloc(mWidth * mHeight * 4); // Must be allocated with malloc
	// Allocate space for transparency bitmap
	int wbits = (mWidth+7)/8;
	bitmap = (char*)malloc(wbits * mHeight);

	// Convert and copy image
	const char *imgdata = (const char*)img.getData();
	int sptr = 0, dptr = 0;
	for(int y=0; y<mHeight; y++) {
		for(int x=0; x<mWidth; x++) {
			data[dptr + 0] = 0;
			data[dptr + 1] = imgdata[sptr + 0];
			data[dptr + 2] = imgdata[sptr + 1];
			data[dptr + 3] = imgdata[sptr + 2];
			// Alpha threshold
			if(((unsigned char)imgdata[sptr + 3])<128) {
				bitmap[y*wbits+(x>>3)] &= ~(1<<(x&7));
			} else {
				bitmap[y*wbits+(x>>3)] |= 1<<(x&7);
			}
			sptr += 4;
			dptr += 4;
		}
	}

	/* put my pixmap data into the client side X image data structure */
	XImage *image = XCreateImage (mDisplay, NULL, 24, ZPixmap, 0,
	                              data,
	                              mWidth, mHeight, 8,
	                              mWidth*4);
	image->byte_order = MSBFirst; // 0RGB format

	/* tell server to start managing my pixmap */
	Pixmap retval = XCreatePixmap(mDisplay, rootWindow, mWidth,
	                              mHeight, 24);

	/* copy from client to server */
	GC context = XCreateGC (mDisplay, rootWindow, 0, NULL);
	XPutImage(mDisplay, retval, context, image, 0, 0, 0, 0,
	          mWidth, mHeight);

	/* free up the client side pixmap data area */
	XDestroyImage(image); // also cleans data
	XFreeGC(mDisplay, context);

	*pix = retval;
	*mask = XCreateBitmapFromData(mDisplay, rootWindow, bitmap, mWidth, mHeight);
	free(bitmap);
	return true;
}

struct visual_attribs
{
   /* X visual attribs */
   int id;
   int klass;
   int depth;
   int redMask, greenMask, blueMask;
   int colormapSize;
   int bitsPerRGB;

   /* GL visual attribs */
   int supportsGL;
   int transparentType;
   int transparentRedValue;
   int transparentGreenValue;
   int transparentBlueValue;
   int transparentAlphaValue;
   int transparentIndexValue;
   int bufferSize;
   int level;
   int rgba;
   int doubleBuffer;
   int stereo;
   int auxBuffers;
   int redSize, greenSize, blueSize, alphaSize;
   int depthSize;
   int stencilSize;
   int accumRedSize, accumGreenSize, accumBlueSize, accumAlphaSize;
   int numSamples, numMultisample;
   int visualCaveat;
};


static void
get_visual_attribs(Display *dpy, XVisualInfo *vInfo,
                   struct visual_attribs *attribs) {
	const char *ext = glXQueryExtensionsString(dpy, vInfo->screen);

	memset(attribs, 0, sizeof(struct visual_attribs));

	attribs->id = vInfo->visualid;
#if defined(__cplusplus) || defined(c_plusplus)

	attribs->klass = vInfo->c_class;
#else

	attribs->klass = vInfo->class;
#endif

	attribs->depth = vInfo->depth;
	attribs->redMask = vInfo->red_mask;
	attribs->greenMask = vInfo->green_mask;
	attribs->blueMask = vInfo->blue_mask;
	attribs->colormapSize = vInfo->colormap_size;
	attribs->bitsPerRGB = vInfo->bits_per_rgb;

	if (glXGetConfig(dpy, vInfo, GLX_USE_GL, &attribs->supportsGL) != 0)
		return;
	glXGetConfig(dpy, vInfo, GLX_BUFFER_SIZE, &attribs->bufferSize);
	glXGetConfig(dpy, vInfo, GLX_LEVEL, &attribs->level);
	glXGetConfig(dpy, vInfo, GLX_RGBA, &attribs->rgba);
	glXGetConfig(dpy, vInfo, GLX_DOUBLEBUFFER, &attribs->doubleBuffer);
	glXGetConfig(dpy, vInfo, GLX_STEREO, &attribs->stereo);
	glXGetConfig(dpy, vInfo, GLX_AUX_BUFFERS, &attribs->auxBuffers);
	glXGetConfig(dpy, vInfo, GLX_RED_SIZE, &attribs->redSize);
	glXGetConfig(dpy, vInfo, GLX_GREEN_SIZE, &attribs->greenSize);
	glXGetConfig(dpy, vInfo, GLX_BLUE_SIZE, &attribs->blueSize);
	glXGetConfig(dpy, vInfo, GLX_ALPHA_SIZE, &attribs->alphaSize);
	glXGetConfig(dpy, vInfo, GLX_DEPTH_SIZE, &attribs->depthSize);
	glXGetConfig(dpy, vInfo, GLX_STENCIL_SIZE, &attribs->stencilSize);
	glXGetConfig(dpy, vInfo, GLX_ACCUM_RED_SIZE, &attribs->accumRedSize);
	glXGetConfig(dpy, vInfo, GLX_ACCUM_GREEN_SIZE, &attribs->accumGreenSize);
	glXGetConfig(dpy, vInfo, GLX_ACCUM_BLUE_SIZE, &attribs->accumBlueSize);
	glXGetConfig(dpy, vInfo, GLX_ACCUM_ALPHA_SIZE, &attribs->accumAlphaSize);

	/* get transparent pixel stuff */
	glXGetConfig(dpy, vInfo,GLX_TRANSPARENT_TYPE, &attribs->transparentType);
	if (attribs->transparentType == GLX_TRANSPARENT_RGB) {
		glXGetConfig(dpy, vInfo, GLX_TRANSPARENT_RED_VALUE, &attribs->transparentRedValue);
		glXGetConfig(dpy, vInfo, GLX_TRANSPARENT_GREEN_VALUE, &attribs->transparentGreenValue);
		glXGetConfig(dpy, vInfo, GLX_TRANSPARENT_BLUE_VALUE, &attribs->transparentBlueValue);
		glXGetConfig(dpy, vInfo, GLX_TRANSPARENT_ALPHA_VALUE, &attribs->transparentAlphaValue);
	} else if (attribs->transparentType == GLX_TRANSPARENT_INDEX) {
		glXGetConfig(dpy, vInfo, GLX_TRANSPARENT_INDEX_VALUE, &attribs->transparentIndexValue);
	}

	/* multisample attribs */
#ifdef GLX_ARB_multisample
	if (ext && strstr("GLX_ARB_multisample", ext) == 0) {
		glXGetConfig(dpy, vInfo, GLX_SAMPLE_BUFFERS_ARB, &attribs->numMultisample);
		glXGetConfig(dpy, vInfo, GLX_SAMPLES_ARB, &attribs->numSamples);
	}
#endif
	else {
		attribs->numSamples = 0;
		attribs->numMultisample = 0;
	}

#if defined(GLX_EXT_visual_rating)
	if (ext && strstr(ext, "GLX_EXT_visual_rating")) {
		glXGetConfig(dpy, vInfo, GLX_VISUAL_CAVEAT_EXT, &attribs->visualCaveat);
	} else {
		attribs->visualCaveat = GLX_NONE_EXT;
	}
#else
	attribs->visualCaveat = 0;
#endif
}

/*
 * Examine all visuals to find the so-called best one.
 * We prefer deepest RGBA buffer with depth, stencil and accum
 * that has no caveats.
 * @author Brian Paul (from the glxinfo source)
 */
int find_best_visual(Display *dpy, int scrnum) {
	XVisualInfo theTemplate;
	XVisualInfo *visuals;
	int numVisuals;
	long mask;
	int i;
	struct visual_attribs bestVis;

	/* get list of all visuals on this screen */
	theTemplate.screen = scrnum;
	mask = VisualScreenMask;
	visuals = XGetVisualInfo(dpy, mask, &theTemplate, &numVisuals);
	if(numVisuals == 0 || visuals == 0) {
		/* something went wrong */
		if(visuals)
			XFree(visuals);
		return -1;
	}

	/* init bestVis with first visual info */
	get_visual_attribs(dpy, &visuals[0], &bestVis);

	/* try to find a "better" visual */
	for (i = 1; i < numVisuals; i++) {
		struct visual_attribs vis;

		get_visual_attribs(dpy, &visuals[i], &vis);

		/* always skip visuals with caveats */
		if (vis.visualCaveat != GLX_NONE_EXT)
			continue;

		/* see if this vis is better than bestVis */
		if ((!bestVis.supportsGL && vis.supportsGL) ||
		                (bestVis.visualCaveat != GLX_NONE_EXT) ||
		                (!bestVis.rgba && vis.rgba) ||
		                (!bestVis.doubleBuffer && vis.doubleBuffer) ||
		                (bestVis.redSize < vis.redSize) ||
		                (bestVis.greenSize < vis.greenSize) ||
		                (bestVis.blueSize < vis.blueSize) ||
		                (bestVis.alphaSize < vis.alphaSize) ||
		                (bestVis.depthSize < vis.depthSize) ||
		                (bestVis.stencilSize < vis.stencilSize) ||
		                (bestVis.accumRedSize < vis.accumRedSize)) {
			/* found a better visual */
			bestVis = vis;
		}
	}

	XFree(visuals);

	return bestVis.id;
}

};

void GLXWindow::create(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth,
                       bool fullScreen, int left, int top, bool depthBuffer,
                       void* miscParam, ...) {
	std::cerr << "GLXWindow::create" << std::endl;

	// We will attempt to create new window on default screen op display 0
	int screen = DefaultScreen(mDisplay);
	int depth = DisplayPlanes(mDisplay, screen);
	Window rootWindow = RootWindow(mDisplay,screen);
#ifndef NO_XRANDR
	// Attempt mode switch for fullscreen
	if(fullScreen) {
		// Use Xrandr extension to switch video modes. This is much better than
		// XVidMode as you can't scroll away from the full-screen applications.
		XRRScreenConfiguration *config;
		XRRScreenSize *sizes;
		Rotation current_rotation;
		int current_size;
		int nsizes;

		// Get current screen info
		config = XRRGetScreenInfo(mDisplay, rootWindow);
		// Get available sizes
		if(config)
			sizes = XRRConfigSizes (config, &nsizes);

		if(config && nsizes > 0) {
			// Get current size and rotation
			mOldMode = XRRConfigCurrentConfiguration (config, &current_rotation);
			// Find smallest matching mode
			int mode = -1;
			int mode_width = INT_MAX;
			int mode_height = INT_MAX;
			for(int i=0; i<nsizes; i++) {
				if(sizes[i].width >= width && sizes[i].height >= height &&
				                sizes[i].width < mode_width && sizes[i].height < mode_height) {
					mode = i;
					mode_width = sizes[i].width;
					mode_height = sizes[i].height;
				}
			}
			if(mode >= 0) {
				// Finally, set the screen configuration
				LogManager::getSingleton().logMessage("GLXWindow::create -- Entering full screen mode");
				XRRSetScreenConfig(mDisplay, config, rootWindow, mode, current_rotation, CurrentTime);
			} else {
				LogManager::getSingleton().logMessage("GLXWindow::create -- Could not switch to full screen mode: No conforming mode was found");
			}
			// Free configuration data
			XRRFreeScreenConfigInfo(config);
		} else {
			LogManager::getSingleton().logMessage("GLXWindow::create -- Could not switch to full screen mode: XRRGetScreenInfo failed");
		}
	}
#endif
	// Apply some magic algorithm to get the best visual
	int best_visual = find_best_visual(mDisplay, screen);
	LogManager::getSingleton().logMessage("GLXWindow::create -- Best visual is "+StringConverter::toString(best_visual));

	// Get information about this so-called-best visual
	XVisualInfo templ;
	int nmatch;
	templ.visualid = best_visual;
	XVisualInfo *visualInfo = XGetVisualInfo(mDisplay, VisualIDMask, &templ, &nmatch);
	if(visualInfo==0 || nmatch==0) {
		Except(999, "GLXWindow: error choosing visual", "GLXWindow::create");
	}

	XSetWindowAttributes attr;
	unsigned long mask;
	attr.background_pixel = 0;
	attr.border_pixel = 0;
	attr.colormap = XCreateColormap(mDisplay,rootWindow,visualInfo->visual,AllocNone);
	attr.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
	if(fullScreen) {
		mask = CWBackPixel | CWColormap | CWOverrideRedirect | CWSaveUnder | CWBackingStore | CWEventMask;
		attr.override_redirect = True;
		attr.backing_store = NotUseful;
		attr.save_under = False;
		// Fullscreen windows are always in the top left origin
		left = top = 0;
	} else
		mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

	// Create window on server
	mWindow = XCreateWindow(mDisplay,rootWindow,left,top,width,height,0,visualInfo->depth,InputOutput,visualInfo->visual,mask,&attr);
	if(!mWindow) {
		Except(999, "GLXWindow: XCreateWindow failed", "GLXWindow::create");
	}

	// Make sure the window is in normal state
	XWMHints *wm_hints;
	if ((wm_hints = XAllocWMHints()) != NULL) {
		wm_hints->initial_state = NormalState;
		wm_hints->input = True;
		wm_hints->flags = StateHint | InputHint;

		// Check if we can give it an icon
		if(depth == 24 || depth == 32) {
			// Woot! The right bit depth, we can load an icon
			if(LoadIcon(mDisplay, rootWindow, "GLX_icon.png", &wm_hints->icon_pixmap, &wm_hints->icon_mask))
				wm_hints->flags |= IconPixmapHint | IconMaskHint;
		}
	}

	// Set size and location hints
	XSizeHints *size_hints;
	if ((size_hints = XAllocSizeHints()) != NULL) {
		// Otherwise some window managers ignore our position request
		size_hints->flags = USPosition;
	}

	// Make text property from title
	XTextProperty titleprop;
	char *lst = (char*)name.c_str();
	XStringListToTextProperty((char **)&lst, 1, &titleprop);

	XSetWMProperties(mDisplay, mWindow, &titleprop, NULL, NULL, 0, size_hints, wm_hints, NULL);

	// We don't like memory leaks. Free the clientside storage, but not the
	// pixmaps as they're still being used by the server.
	XFree(titleprop.value);
	XFree(wm_hints);
	XFree(size_hints);

	// Acquire atom to recognize window close events
	mAtomDeleteWindow = XInternAtom(mDisplay,"WM_DELETE_WINDOW",False);
	XSetWMProtocols(mDisplay,mWindow,&mAtomDeleteWindow,1);

	// Map window unto screen and focus it.
	XMapWindow(mDisplay,mWindow);

	// Make sure the server is up to date and focus the window
	XFlush(mDisplay);

	// Finally, create a GL context
	mGlxContext = glXCreateContext(mDisplay,visualInfo,NULL,True);
	if(!mGlxContext) {
		Except(999, "glXCreateContext failed", "GLXWindow::create");
	}
	glXMakeCurrent(mDisplay,mWindow,mGlxContext);

	// Free visual info
	XFree(visualInfo);

	mName = name;
	mWidth = width;
	mHeight = height;
	mFullScreen = fullScreen;
}

void GLXWindow::destroy(void) {
	if(mGlxContext)
		glXDestroyContext(mDisplay, mGlxContext);
	if(mWindow)
		XDestroyWindow(mDisplay, mWindow);
	mWindow = 0;
	mGlxContext = 0;
	mActive = false;

	Root::getSingleton().getRenderSystem()->detachRenderTarget( this->getName() );
}

bool GLXWindow::isActive() const {
	return mActive;
}

bool GLXWindow::isClosed() const {
	return mClosed;
}

void GLXWindow::reposition(int left, int top) {
	XMoveWindow(mDisplay,mWindow,left,top);
}

void GLXWindow::resize(unsigned int width, unsigned int height) {
	XResizeWindow(mDisplay,mWindow,width,height);
}

void GLXWindow::swapBuffers(bool waitForVSync) {
	glXSwapBuffers(mDisplay,mWindow);
}

void GLXWindow::outputText(int x, int y, const String& text) {
	// Deprecated
}

void GLXWindow::processEvent(const XEvent &event) {
	// Process only events for this window
	switch(event.type) {
	case ClientMessage:
		if(event.xclient.display != mDisplay || event.xclient.window != mWindow)
			// Not for me
			break;
		if(event.xclient.format == 32 && event.xclient.data.l[0] == (long)mAtomDeleteWindow)  {
			// Window deleted -- oops, this does not work, ogre doesn't register the close
			//mClosed = true;
			//mActive = false;
			//Root::getSingleton().getRenderSystem()->detachRenderTarget( this->getName() );
		}
		break;
	case ConfigureNotify:
		if(event.xconfigure.display != mDisplay || event.xconfigure.window != mWindow)
			// Not for me
			break;
		// Check if the window size really changed
		if(mWidth == event.xconfigure.width && mHeight == event.xconfigure.height)
			break;
		mWidth = event.xconfigure.width;
		mHeight = event.xconfigure.height;

		for (ViewportList::iterator it = mViewportList.begin();
		                it != mViewportList.end(); ++it) {
			(*it).second->_updateDimensions();
		}

		break;
	case MapNotify:
		if(event.xconfigure.display != mDisplay || event.xconfigure.window != mWindow)
			// Not for me
			break;
		// Window was mapped to the screen
		mActive = true;
		break;
	case UnmapNotify:
		if(event.xconfigure.display != mDisplay || event.xconfigure.window != mWindow)
			// Not for me
			break;
		// Window was unmapped from the screen (user switched
		// to another workspace, for example)
		mActive = false;
		break;
	}
}


void GLXWindow::getCustomAttribute( const String& name, void* pData ) {
	if( name == "GLXWINDOW" ) {
		*static_cast<Window*>(pData) = mWindow;
		return;
	} else if( name == "GLXDISPLAY" ) {
		*static_cast<Display**>(pData) = mDisplay;
		return;
	}
	RenderWindow::getCustomAttribute(name, pData);
}

void GLXWindow::writeContentsToFile(const String& filename) {

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
		        "GLXWindow::writeContentsToFile" );

	while( pos != filename.length() - 1 )
		extension += filename[++pos];

	// Get the codec
	Codec * pCodec = Codec::getCodec(extension);

	// Write out
	pCodec->codeToFile(chunkFlipped, filename, &imgData);

	delete [] pBuffer;


}
}

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

#ifndef __GLXWindow_H__
#define __GLXWindow_H__

#include "OgreRenderWindow.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/xf86vmode.h>
#include <GL/glx.h>

namespace Ogre {
class GLXWindow : public RenderWindow {
private:
	Display *mDisplay;
	Window mWindow;
	Atom mAtomDeleteWindow;
	GLXContext mGlxContext;

	bool mActive;	// Window is currently on screen?
	bool mClosed;
	bool mFullScreen;
	int mOldMode;	// Mode before switching to fullscreen
public:
	// Pass X display to create this window on
	GLXWindow(Display *display);
	~GLXWindow();

	void create(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth,
	            bool fullScreen, int left, int top, bool depthBuffer, void* miscParam, ...);
	/** Overridden - see RenderWindow */
	void destroy(void);
	/** Overridden - see RenderWindow */
	bool isActive(void) const;
	/** Overridden - see RenderWindow */
	bool isClosed(void) const;
	/** Overridden - see RenderWindow */
	void reposition(int left, int top);
	/** Overridden - see RenderWindow */
	void resize(unsigned int width, unsigned int height);
	/** Overridden - see RenderWindow */
	void swapBuffers(bool waitForVSync);

	/** Overridden - see RenderTarget.
	*/
	void writeContentsToFile(const String& filename);

	/**
	 * Get custom attribute; the following attributes are valid:
	 * GLXWINDOW	The X Window associated with this
	 * GLXDISPLAY	The X Display associated with this
	 */
	void getCustomAttribute(const String& name, void* pData);

	/**
	 * Call this for every X event, so that the window stays up to date with 
	 * ConfigureNotify and Deletion events.
	 */
	virtual void processEvent(const XEvent &event);

	bool requiresTextureFlipping() const {
		return false;
	}
};
}

#endif


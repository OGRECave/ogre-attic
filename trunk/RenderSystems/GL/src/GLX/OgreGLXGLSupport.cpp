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

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include "OgreGLXGLSupport.h"

#include "OgreGLXWindow.h"

namespace Ogre {

GLXGLSupport::GLXGLSupport():
mDisplay(0) {}

GLXGLSupport::~GLXGLSupport() {}

void GLXGLSupport::addConfig(void) {
	ConfigOption optFullScreen;
	ConfigOption optVideoMode;
	ConfigOption optBitDepth;

	// FS setting possiblities
	optFullScreen.name = "Full Screen";
	optFullScreen.possibleValues.push_back("Yes");
	optFullScreen.possibleValues.push_back("No");
	optFullScreen.currentValue = "Yes";
	optFullScreen.immutable = false;

	// Video mode possiblities
	optVideoMode.name = "Video Mode";
	optVideoMode.immutable = false;

	// We could query Xrandr here, but that wouldn't work in the non-fullscreen case
	// or when that extension is disables. Anyway, this list of modes is fairly
	// complete.
	optVideoMode.possibleValues.push_back("640 x 480");
	optVideoMode.possibleValues.push_back("800 x 600");
	optVideoMode.possibleValues.push_back("1024 x 768");
	optVideoMode.possibleValues.push_back("1280 x 960");
	optVideoMode.possibleValues.push_back("1280 x 1024");
	optVideoMode.possibleValues.push_back("1600 x 1200");

	optVideoMode.currentValue = "800 x 600";

	mOptions[optFullScreen.name] = optFullScreen;
	mOptions[optVideoMode.name] = optVideoMode;

}

String GLXGLSupport::validateConfig(void) {
	return String("");
}

RenderWindow* GLXGLSupport::createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle) {
	if (autoCreateWindow) {
		ConfigOptionMap::iterator opt = mOptions.find("Full Screen");
		if (opt == mOptions.end())
			Except(999, "Can't find full screen options!", "GLXGLSupport::createWindow");
		bool fullscreen = (opt->second.currentValue == "Yes");

		opt = mOptions.find("Video Mode");
		if (opt == mOptions.end())
			Except(999, "Can't find video mode options!", "GLXGLSupport::createWindow");
		String val = opt->second.currentValue;
		String::size_type pos = val.find('x');
		if (pos == String::npos)
			Except(999, "Invalid Video Mode provided", "GLXGLSupport::createWindow");

		unsigned int w = StringConverter::parseUnsignedInt(val.substr(0, pos));
		unsigned int h = StringConverter::parseUnsignedInt(val.substr(pos + 1));

		// Make sure the window is centered
		int screen = DefaultScreen(mDisplay);
		int left = DisplayWidth(mDisplay, screen)/2 - w/2; 
		int top = DisplayHeight(mDisplay, screen)/2 - h/2; 

		return renderSystem->createRenderWindow(windowTitle, w, h, 32, fullscreen, left, top);
	} else {
		// XXX What is the else?
		return NULL;
	}
}

RenderWindow* GLXGLSupport::newWindow(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth,
                                      bool fullScreen, int left, int top, bool depthBuffer, RenderWindow* parentWindowHandle,
                                      bool vsync) {
	GLXWindow* window = new GLXWindow(mDisplay);
	window->create(name, width, height, colourDepth, fullScreen, left, top, depthBuffer,
	               parentWindowHandle);
	return window;
}

void GLXGLSupport::start() {
	LogManager::getSingleton().logMessage(
	        "******************************\n"
	        "*** Starting GLX Subsystem ***\n"
	        "******************************");
	mDisplay = XOpenDisplay(NULL);
	if(!mDisplay) {
		Except(999, "Couldn`t open X display", "GLXGLSupport::start");
	}

}

void GLXGLSupport::stop() {
	LogManager::getSingleton().logMessage(
	        "******************************\n"
	        "*** Stopping GLX Subsystem ***\n"
	        "******************************");
	if(mDisplay)
		XCloseDisplay(mDisplay);
	mDisplay = 0;
}
extern "C" {
extern void (*glXGetProcAddressARB(const GLubyte *procName))( void );
};

void* GLXGLSupport::getProcAddress(const String& procname) {
	return (void*)glXGetProcAddressARB((const GLubyte*)procname.c_str());
}

#if 0
// TODO: multiple context/window support

void GLXGLSupport::begin_context(RenderTarget *_target)
{
	// Support nested contexts, in which case.. nothing happens
    	++_context_ref;
    	if (_context_ref == 1) {
		if(_target) {
			// Begin a specific context
			OGREWidget *_ogre_widget = static_cast<GTKWindow*>(_target)->get_ogre_widget();

	        	_ogre_widget->get_gl_window()->gl_begin(_ogre_widget->get_gl_context());
		} else {
			// Begin a generic main context
			_main_window->gl_begin(_main_context);
		}
    	}
}

void GLXGLSupport::end_context()
{
    	--_context_ref;
    	if(_context_ref < 0)
        	Except(999, "Too many contexts destroyed!", "GTKGLSupport::end_context");
    	if (_context_ref == 0)
    	{
		// XX is this enough? (_main_window might not be the current window,
 		// but we can never be sure the previous rendering window 
		// even still exists)
		_main_window->gl_end();
    	}
}
#endif

};

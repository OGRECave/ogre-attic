#ifndef OGRE_GLXGLSupport_H
#define OGRE_GLXGLSupport_H

#include "OgreGLSupport.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/xf86vmode.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

namespace Ogre {

class GLXGLSupport : public GLSupport {
public:
	GLXGLSupport();
	~GLXGLSupport();

	/**
	* Add any special config values to the system.
	* Must have a "Full Screen" value that is a bool and a "Video Mode" value
	* that is a string in the form of wxh
	*/
	void addConfig(void);
	/**
	* Make sure all the extra options are valid
	*/
	String validateConfig(void);

	virtual RenderWindow* createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle);

	/**
	* Create a new specific render window
	*/
	virtual RenderWindow* newWindow(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth,
	                                bool fullScreen, int left, int top, bool depthBuffer, RenderWindow* parentWindowHandle,
	                                bool vsync);

	/**
	* Start anything special
	*/
	void start();
	/**
	* Stop anything special
	*/
	void stop();

	/**
	* Get the address of a function
	*/
	void* getProcAddress(const String& procname);
private:
	// X display
	Display *mDisplay;
}
; // class GLXGLSupport

}
; // namespace Ogre

#endif // OGRE_GLXGLSupport_H

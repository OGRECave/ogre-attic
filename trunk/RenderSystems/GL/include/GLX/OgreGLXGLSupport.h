#ifndef OGRE_GLXGLSupport_H
#define OGRE_GLXGLSupport_H

#include "OgreGLSupport.h"

#include "X11/Xlib.h"
#include <GL/gl.h>

namespace Ogre {

class _OgrePrivate GLXGLSupport : public GLSupport
{
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

	/// @copydoc GLSupport::createWindow
	RenderWindow* createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle);
	
	/// @copydoc RenderSystem::createRenderWindow
	virtual RenderWindow* newWindow(const String &name, unsigned int width, unsigned int height, 
		bool fullScreen, const NameValuePairList *miscParams = 0);

	
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

	virtual GLPBuffer *createPBuffer(PixelComponentType format, size_t width, size_t height);

	virtual void setConfigOption(const String &name, const String &value);

private:
	// X display
	Display *mDisplay;
	// X screen
	Screen *mScreen;
	// Fill the "Video Mode" option
	void FillVideoModes ();
	// Fill the "Refresh Rate" option, if supported
	void RenewRefreshRate ();
}
; // class GLXGLSupport

}
; // namespace Ogre

#endif // OGRE_GLXGLSupport_H

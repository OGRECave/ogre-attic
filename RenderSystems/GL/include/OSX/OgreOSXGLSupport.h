#ifndef OGRE_OSXGLSupport_H
#define OGRE_OSXGLSupport_H

#include "OgreGLSupport.h"

namespace Ogre
{

class OSXGLSupport : public GLSupport
{
public:
	OSXGLSupport();
	~OSXGLSupport();

	/**
	* Add any special config values to the system.
	* Must have a "Full Screen" value that is a bool and a "Video Mode" value
	* that is a string in the form of wxh
	*/
	void addConfig( void );

	/**
	* Make sure all the extra options are valid
	*/
	String validateConfig( void );

	/// @copydoc GLSupport::createWindow
	RenderWindow* createWindow( bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle );
	
	/// @copydoc RenderSystem::createRenderWindow
	virtual RenderWindow* newWindow( const String &name, unsigned int width, unsigned int height, 
		bool fullScreen, const NameValuePairList *miscParams = 0 );
	
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
	void* getProcAddress( const char *name );
	void* getProcAddress( const String& procname );

	virtual bool supportsPBuffers();
	virtual GLPBuffer* createPBuffer( PixelComponentType format, size_t width, size_t height );

}; // class OSXGLSupport

}; // namespace Ogre

#endif // OGRE_OSXGLSupport_H

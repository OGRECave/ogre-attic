#ifndef __OgreWin32GLSupport_H__
#define __OgreWin32GLSupport_H__

#include "OgreWin32Prerequisites.h"
#include "OgreGLSupport.h"
#include "OgreGLRenderSystem.h"

using std::vector;

namespace Ogre
{
    
	class Win32GLSupport : public GLSupport
	{
	public:
        Win32GLSupport();
		/**
		* Add any special config values to the system.
		* Must have a "Full Screen" value that is a bool and a "Video Mode" value
		* that is a string in the form of wxhxb
		*/
		void addConfig();

		void setConfigOption(const String &name, const String &value);

		/**
		* Make sure all the extra options are valid
		*/
		String validateConfig();

		virtual RenderWindow* createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle = "OGRE Render Window");

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

		void setExternalWindowHandle(void* hwnd){
			mExternalWindowHandle=(HWND) hwnd;
		};

		virtual void resizeReposition(void*);
		virtual RenderTexture * createRenderTexture( const String & name, unsigned int width, unsigned int height, TextureType texType,  PixelFormat format );

		/**
		 * Initialise extensions
		 */
		virtual void initialiseExtensions();
		/**
		 * Initialise support specific capabilities
		 */
		virtual void initialiseCapabilities(RenderSystemCapabilities &caps);
	private:
		// Allowed video modes
		vector<DEVMODE> mDevModes;

		void refreshConfig();

		HWND mExternalWindowHandle;
	};

}

#endif

#ifndef OGRE_GLSUPPORT_H
#define OGRE_GLSUPPORT_H

#include "OgreGLPrerequisites.h"
#include "OgreGLRenderSystem.h"

#include "OgreRenderWindow.h"
#include "OgreConfigOptionMap.h"

namespace Ogre
{
    
class GLSupport
{
public:
    GLSupport() { }
    virtual ~GLSupport() { }

    /**
    * Add any special config values to the system.
    * Must have a "Full Screen" value that is a bool and a "Video Mode" value
    * that is a string in the form of wxh
    */
    virtual void addConfig() = 0;

	virtual void setConfigOption(const String &name, const String &value);

    /**
    * Make sure all the extra options are valid
    * @return string with error message
    */
    virtual String validateConfig() = 0;

	virtual ConfigOptionMap& getConfigOptions(void);

    
	virtual RenderWindow* createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle) = 0;

	/**
    * Create a specific instance of a render window
    */
    virtual RenderWindow* newWindow(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth,
            bool fullScreen, int left, int top, bool depthBuffer, RenderWindow* parentWindowHandle,
			bool vsync) = 0;

    /**
     * Create a RenderTexture
     */
    virtual RenderTexture * createRenderTexture( const String & name, unsigned int width, unsigned int height, TextureType texType,  PixelFormat format);

    /**
    * Start anything special
    */
    virtual void start() = 0;
    /**
    * Stop anything special
    */
    virtual void stop() = 0;

    /**
    * get vendor information
    */
    const String& getGLVendor(void) const
    {
        return mVendor;
    }

    /**
    * get version information
    */
    const String& getGLVersion(void) const
    {
        return mVersion;
    }

    /**
    * compare GL version numbers
    */
    bool checkMinGLVersion(const String& v) const;

    /**
    * Check if an extension is available
    */
    virtual bool checkExtension(const String& ext) const;
    /**
    * Get the address of a function
    */
    virtual void* getProcAddress(const String& procname) = 0;

    void setExternalWindowHandle(void* hwnd) { }

    /** Intialises GL extensions, must be done AFTER the GL context has been
        established.
    */
    virtual void initialiseExtensions();

	/**	GLsupport specific capabilities (hardware render-to-texture, being one of 
	    them) are marked in caps.
	*/
	virtual void initialiseCapabilities(RenderSystemCapabilities &caps);

	virtual void resizeRepositionWindow(void * window){m_windowToResize = window;};
	virtual void resizeReposition(void*){;};  // should change to pure when it is implemented for all cases

protected:
	// Stored options
    ConfigOptionMap mOptions;

	void *m_windowToResize;

	// This contains the complete list of supported extensions
    std::set<String> extensionList;
private:
    String mVersion;
    String mVendor;

}; // class GLSupport

}; // namespace Ogre

#endif // OGRE_GLSUPPORT_H

#ifndef OGRE_GLSUPPORT_H
#define OGRE_GLSUPPORT_H

#include "OgreGLPrerequisites.h"

#include "OgreSingleton.h"
#include "OgreRenderWindow.h"
#include "OgreConfigOptionMap.h"


#include <string>

namespace Ogre
{
    
class GLSupport : public Singleton<GLSupport>
{
public:
    GLSupport() { }
    virtual ~GLSupport() 
    { 
        // XXX Is this really necessary?
        extensionList.clear();
    }

    /**
    * Add any special config values to the system.
    * Must have a "Full Screen" value that is a bool and a "Video Mode" value
    * that is a string in the form of wxh
    */
    virtual void addConfig(ConfigOptionMap& options) = 0;
    /**
    * Make sure all the extra options are valid
    * @return string with error message
    */
    virtual std::string validateConfig(ConfigOptionMap& options) = 0;

    /**
    * Create a specific instance of a render window
    */
    virtual RenderWindow* newWindow() = 0;

    /**
    * Start anything special
    */
    virtual void start() = 0;
    /**
    * Stop anything special
    */
    virtual void stop() = 0;

    /**
    * get version information
    */
    const String& getGLVersion(void)
    {
      return version;
    }

    /**
    * Check if an extension is available
    */
    virtual bool checkExtension(const std::string& ext);
    /**
    * Get the address of a function
    */
    virtual void* getProcAddress(const std::string& procname) = 0;
    /** Intialises GL extensions, must be done AFTER the GL context has been
        established.
    */
    virtual void initialiseExtensions(void);

    /** Override standard Singleton retrieval.
        @remarks
            Why do we do this? Well, it's because the Singleton
            implementation is in a .h file, which means it gets compiled
            into anybody who includes it. This is needed for the
            Singleton template to work, but we actually only want it
            compiled into the implementation of the class based on the
            Singleton, not all of them. If we don't change this, we get
            link errors when trying to use the Singleton-based class from
            an outside dll.
        @par
            This method just delegates to the template version anyway,
            but the implementation stays in this single compilation unit,
            preventing link errors.
      */
      static GLSupport& getSingleton(void);

private:

    // This contains the complete list of supported extensions
    std::set<String> extensionList;
    String version;

}; // class GLSupport

}; // namespace Ogre

#endif // OGRE_GLSUPPORT_H

#ifndef OGRE_GLSUPPORT_H
#define OGRE_GLSUPPORT_H

#include "OgreGLPrerequisites.h"

#include "OgreRenderWindow.h"
#include "OgreConfigOptionMap.h"


#include <string>

namespace Ogre
{
    
class GLSupport
{
public:
    virtual ~GLSupport() { }

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
    * Check if an extension is available
    */
    virtual bool checkExtension(const std::string& ext) = 0;
    /**
    * Get the address of a function
    */
    virtual void* getProcAddress(const std::string& procname) = 0;
    /** Intialises GL extensions, must be done AFTER the GL context has been
        established.
    */
    virtual void initialiseExtensions(void);


protected:

    // This contains the complete list of supported extensions
    std::set<String> extensionList;

}; // class GLSupport

}; // namespace Ogre

#endif // OGRE_GLSUPPORT_H

#ifndef OGRE_GLSUPPORT_H
#define OGRE_GLSUPPORT_H

#include "OgreRenderWindow.h"
#include "OgreConfigOptionMap.h"

#if OGRE_PLATFORM == PLATFORM_WIN32
#   include <windows.h>
#   include <wingdi.h>
#   include "gl.h"
#   define GL_GLEXT_PROTOTYPES
#   include "glprocs.h"
#   include <GL/glu.h>
#elif OGRE_PLATFORM == PLATFORM_LINUX
#   include <GL/gl.h>
#   include <GL/glu.h>
#elif OGRE_PLATFORM == PLATFORM_APPLE
#   include <OpenGL/gl.h>
#   define GL_EXT_texture_env_combine 1
#   include <OpenGL/glext.h>
#   include <OpenGL/glu.h>
#endif

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
}; // class GLSupport

}; // namespace Ogre

#endif // OGRE_GLSUPPORT_H

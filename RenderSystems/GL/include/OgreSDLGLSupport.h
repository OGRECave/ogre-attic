#ifndef OGRE_SDLGLSUPPORT_H
#define OGRE_SDLGLSUPPORT_H

#include "OgreSDLPrerequisites.h"
#include "OgreGLSupport.h"

#include <SDL.h>

namespace Ogre
{
    
class SDLGLSupport : public GLSupport
{
public:
    SDLGLSupport();
    ~SDLGLSupport();

    /**
    * Add any special config values to the system.
    * Must have a "Full Screen" value that is a bool and a "Video Mode" value
    * that is a string in the form of wxh
    */
    void addConfig(ConfigOptionMap& options);
    /**
    * Make sure all the extra options are valid
    */
    std::string validateConfig(ConfigOptionMap& options);

    /**
    * Create a new specific render window
    */
    RenderWindow* newWindow();

    /**
    * Start anything special
    */
    void start();
    /**
    * Stop anything special
    */
    void stop();

    /**
    * Check if an extension is available
    */
    bool checkExtension(const std::string& ext);
    /**
    * Get the address of a function
    */
    void* getProcAddress(const std::string& procname);
private:
    // Allowed video modes
    SDL_Rect** mVideoModes;


}; // class SDLGLSupport

}; // namespace Ogre

#endif // OGRE_SDLGLSUPPORT_H

#include "OgreException.h"
#include "OgreLogManager.h"

#include "OgreSDLGLSupport.h"

#include "OgreSDLWindow.h"

using namespace Ogre;

SDLGLSupport::SDLGLSupport()
{
    std::stringstream ext;
    String str;
    
    ext << glGetString(GL_EXTENSIONS);

    while(ext >> str)
    {
        extensionList.insert(str);
    }

    ext.str("");

    SDL_Init(SDL_INIT_VIDEO);

    atexit(SDL_Quit);
}

SDLGLSupport::~SDLGLSupport()
{
    // XXX Is this really necessary?
    extensionList.clear();

    SDL_Quit();
}

void SDLGLSupport::addConfig(ConfigOptionMap& options)
{
    mVideoModes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_OPENGL);
    
    if (mVideoModes == (SDL_Rect **)0)
    {
        Except(999, "Unable to load video modes",
                "SDLRenderSystem::initConfigOptions");
    }

    ConfigOption optFullScreen;
    ConfigOption optVideoMode;

    // FS setting possiblities
    optFullScreen.name = "Full Screen";
    optFullScreen.possibleValues.push_back("Yes");
    optFullScreen.possibleValues.push_back("No");
    optFullScreen.currentValue = "Yes";
    optFullScreen.immutable = false;

    // Video mode possiblities
    optVideoMode.name = "Video Mode";
    optVideoMode.immutable = false;
    for (int i = 0; mVideoModes[i]; i++)
    {
        char szBuf[16];
        snprintf(szBuf, 16, "%d x %d", mVideoModes[i]->w, mVideoModes[i]->h);
        optVideoMode.possibleValues.push_back(szBuf);
        // Make the first one default
        if (i == 0)
        {
            optVideoMode.currentValue = szBuf;
        }
    }
    
    options[optFullScreen.name] = optFullScreen;
    options[optVideoMode.name] = optVideoMode;
}

std::string SDLGLSupport::validateConfig(ConfigOptionMap& options)
{
    return std::string("");
}

RenderWindow* SDLGLSupport::newWindow()
{
    return new SDLWindow();
}

void SDLGLSupport::start()
{
    LogManager::getSingleton().logMessage(
        "******************************\n"
        "*** Starting SDL Subsystem ***\n"
        "******************************");

    SDL_Init(SDL_INIT_VIDEO);
}

void SDLGLSupport::stop()
{
    LogManager::getSingleton().logMessage(
        "******************************\n"
        "*** Stopping SDL Subsystem ***\n"
        "******************************");

    SDL_Quit();
}

bool SDLGLSupport::checkExtension(const std::string& ext)
{
    if(extensionList.find(ext) == extensionList.end())
        return false;

    return true;
}

void* SDLGLSupport::getProcAddress(const std::string& procname)
{
    // XXX IMPL
    return NULL;
}

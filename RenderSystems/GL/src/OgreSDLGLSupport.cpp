#include "OgreException.h"
#include "OgreLogManager.h"

#include "OgreSDLGLSupport.h"

#include "OgreSDLWindow.h"

using namespace Ogre;

SDLGLSupport::SDLGLSupport()
{

    SDL_Init(SDL_INIT_VIDEO);

    atexit(SDL_Quit);
}

SDLGLSupport::~SDLGLSupport()
{
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

String SDLGLSupport::validateConfig(ConfigOptionMap& options)
{
    return String("");
}

RenderWindow* SDLGLSupport::createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem)
{
	if (autoCreateWindow)
    {
        ConfigOptionMap::iterator opt = mOptions.find("Full Screen");
        if (opt == mOptions.end())
            Except(999, "Can't find full screen options!", "SDLGLSupport::createWindow");
        bool fullscreen = (opt->second.currentValue == "Yes");

        opt = mOptions.find("Video Mode");
        if (opt == mOptions.end())
            Except(999, "Can't find video mode options!", "SDLGLSupport::createWindow");
        String val = opt->second.currentValue;
        String::size_type pos = val.find('x');
        if (pos == String::npos)
            Except(999, "Invalid Video Mode provided", "SDLGLSupport::createWindow");

        int w = atoi(val.substr(0, pos).c_str());
        int h = atoi(val.substr(pos + 1).c_str());

        return renderSystem->createRenderWindow("OGRE Render Window", w, h, 32, fullscreen);
    }
    else
    {
        // XXX What is the else?
		return NULL;
    }
}

RenderWindow* SDLGLSupport::newWindow(const String& name, int width, int height, int colourDepth,
        bool fullScreen, int left, int top, bool depthBuffer, RenderWindow* parentWindowHandle,
		bool vsync)
{
	SDLWindow* window = new SDLWindow();
	window->create(name, width, height, colourDepth, fullScreen, left, top, depthBuffer,
		parentWindowHandle);
	return window;
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

void* SDLGLSupport::getProcAddress(const String& procname)
{
    // XXX IMPL
    return NULL;
}

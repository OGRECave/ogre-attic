#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include "OgreSDLGLSupport.h"

#include "OgreSDLWindow.h"

using namespace Ogre;

SDLGLSupport::SDLGLSupport()
{

    SDL_Init(SDL_INIT_VIDEO);
}

SDLGLSupport::~SDLGLSupport()
{
}

void SDLGLSupport::addConfig(void)
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
    for (size_t i = 0; mVideoModes[i]; i++)
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
    
    mOptions[optFullScreen.name] = optFullScreen;
    mOptions[optVideoMode.name] = optVideoMode;
}

String SDLGLSupport::validateConfig(void)
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

        unsigned int w = StringConverter::parseUnsignedInt(val.substr(0, pos));
        unsigned int h = StringConverter::parseUnsignedInt(val.substr(pos + 1));

        return renderSystem->createRenderWindow("OGRE Render Window", w, h, 32, fullscreen);
    }
    else
    {
        // XXX What is the else?
		return NULL;
    }
}

RenderWindow* SDLGLSupport::newWindow(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth,
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
    return SDL_GL_GetProcAddress(procname.c_str());
}

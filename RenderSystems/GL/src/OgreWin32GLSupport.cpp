#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include <algorithm>

#include "OgreWin32GLSupport.h"

#include "OgreWin32Window.h"

using namespace Ogre;

namespace Ogre {
	template<class C> void remove_duplicates(C& c)
	{
		std::sort(c.begin(), c.end());
		typename C::iterator p = std::unique(c.begin(), c.end());
		c.erase(p, c.end());
	}

	void Win32GLSupport::addConfig()
	{
		//TODO: EnumDisplayDevices http://msdn.microsoft.com/library/en-us/gdi/devcons_2303.asp
		/*vector<string> DisplayDevices;
		DISPLAY_DEVICE DisplayDevice;
		DisplayDevice.cb = sizeof(DISPLAY_DEVICE);
		DWORD i=0;
		while (EnumDisplayDevices(NULL, i++, &DisplayDevice, 0) {
			DisplayDevices.push_back(DisplayDevice.DeviceName);
		}*/
		  
		ConfigOption optFullScreen;
		ConfigOption optVideoMode;
		ConfigOption optColourDepth;
		ConfigOption optDisplayFrequency;
		ConfigOption optVSync;

		// FS setting possiblities
		optFullScreen.name = "Full Screen";
		optFullScreen.possibleValues.push_back("Yes");
		optFullScreen.possibleValues.push_back("No");
		optFullScreen.currentValue = "Yes";
		optFullScreen.immutable = false;

		// Video mode possiblities
		DEVMODE DevMode;
		DevMode.dmSize = sizeof(DEVMODE);
		optVideoMode.name = "Video Mode";
		optVideoMode.immutable = false;
		for (DWORD i = 0; EnumDisplaySettings(NULL, i, &DevMode); ++i)
		{
			if (DevMode.dmBitsPerPel < 16 || DevMode.dmPelsHeight < 480)
				continue;
			mDevModes.push_back(DevMode);
			char szBuf[16];
			snprintf(szBuf, 16, "%d x %d", DevMode.dmPelsWidth, DevMode.dmPelsHeight);
			optVideoMode.possibleValues.push_back(szBuf);
		}
		remove_duplicates(optVideoMode.possibleValues);
		optVideoMode.currentValue = optVideoMode.possibleValues.front();

		optColourDepth.name = "Colour Depth";
		optColourDepth.immutable = false;
		optColourDepth.currentValue = "";

		optDisplayFrequency.name = "Display Frequency";
		optDisplayFrequency.immutable = false;
		optDisplayFrequency.currentValue = "";

		optVSync.name = "VSync";
		optVSync.immutable = false;
		optVSync.possibleValues.push_back("No");
		optVSync.possibleValues.push_back("Yes");
		optVSync.currentValue = "No";

		mOptions[optFullScreen.name] = optFullScreen;
		mOptions[optVideoMode.name] = optVideoMode;
		mOptions[optColourDepth.name] = optColourDepth;
		mOptions[optDisplayFrequency.name] = optDisplayFrequency;
		mOptions[optVSync.name] = optVSync;

		refreshConfig();
	}

	void Win32GLSupport::refreshConfig()
	{
		ConfigOptionMap::iterator optVideoMode = mOptions.find("Video Mode");
		ConfigOptionMap::iterator moptColourDepth = mOptions.find("Colour Depth");
		ConfigOptionMap::iterator moptDisplayFrequency = mOptions.find("Display Frequency");
		if(optVideoMode == mOptions.end() || moptColourDepth == mOptions.end() || moptDisplayFrequency == mOptions.end())
			Except(999, "Can't find mOptions!", "Win32GLSupport::refreshConfig");
		ConfigOption* optColourDepth = &moptColourDepth->second;
		ConfigOption* optDisplayFrequency = &moptDisplayFrequency->second;

		String val = optVideoMode->second.currentValue;
		String::size_type pos = val.find('x');
		if (pos == String::npos)
			Except(999, "Invalid Video Mode provided", "Win32GLSupport::refreshConfig");
		int width = atoi(val.substr(0, pos).c_str());

		for(std::vector<DEVMODE>::const_iterator i = mDevModes.begin(); i != mDevModes.end(); ++i)
		{
			if (i->dmPelsWidth != width)
				continue;
			char buf[128];
			sprintf(buf, "%d", i->dmBitsPerPel);
			optColourDepth->possibleValues.push_back(buf);
			sprintf(buf, "%d", i->dmDisplayFrequency);
			optDisplayFrequency->possibleValues.push_back(buf);
		}
		remove_duplicates(optColourDepth->possibleValues);
		remove_duplicates(optDisplayFrequency->possibleValues);
		optColourDepth->currentValue = optColourDepth->possibleValues.back();
		optDisplayFrequency->currentValue = optDisplayFrequency->possibleValues.front();
	}

	void Win32GLSupport::setConfigOption(const String &name, const String &value)
	{
		ConfigOptionMap::iterator it = mOptions.find(name);

		// Update
		if(it != mOptions.end())
			it->second.currentValue = value;
		else
		{
			char msg[128];
			sprintf( msg, "Option named '%s' does not exist.", name.c_str() );
			Except( Exception::ERR_INVALIDPARAMS, msg, "Win32GLSupport::setConfigOption" );
		}

		if( name == "Video Mode" )
			refreshConfig();

		if( name == "Full Screen" )
		{
			it = mOptions.find( "Display Frequency" );
			if( value == "No" )
			{
				it->second.currentValue = "N/A";
				it->second.immutable = true;
			}
			else
			{
				it->second.currentValue = it->second.possibleValues.front();
				it->second.immutable = false;
			}
		}
	}

	String Win32GLSupport::validateConfig()
	{
		// TODO, DX9
		return String("");
	}

	RenderWindow* Win32GLSupport::createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle)
	{
		if (autoCreateWindow)
        {
            ConfigOptionMap::iterator opt = mOptions.find("Full Screen");
            if (opt == mOptions.end())
                Except(999, "Can't find full screen options!", "Win32GLSupport::createWindow");
            bool fullscreen = (opt->second.currentValue == "Yes");

            opt = mOptions.find("Video Mode");
            if (opt == mOptions.end())
                Except(999, "Can't find video mode options!", "Win32GLSupport::createWindow");
            String val = opt->second.currentValue;
            String::size_type pos = val.find('x');
            if (pos == String::npos)
                Except(999, "Invalid Video Mode provided", "Win32GLSupport::createWindow");

			unsigned int w = StringConverter::parseUnsignedInt(val.substr(0, pos));
            unsigned int h = StringConverter::parseUnsignedInt(val.substr(pos + 1));

			opt = mOptions.find("Colour Depth");
			if (opt == mOptions.end())
				Except(999, "Can't find Colour Depth options!", "Win32GLSupport::createWindow");
			unsigned int colourDepth = atoi(opt->second.currentValue);

			opt = mOptions.find("VSync");
			if (opt == mOptions.end())
				Except(999, "Can't find VSync options!", "Win32GLSupport::createWindow");
			bool vsync = (opt->second.currentValue == "Yes");
			renderSystem->setWaitForVerticalBlank(vsync);

            return renderSystem->createRenderWindow(windowTitle, w, h, colourDepth, fullscreen);
        }
        else
        {
            // XXX What is the else?
			return NULL;
        }
	}

	RenderWindow* Win32GLSupport::newWindow(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth,
            bool fullScreen, int left, int top, bool depthBuffer, RenderWindow* parentWindowHandle,
			bool vsync)
	{
		ConfigOptionMap::iterator opt = mOptions.find("Display Frequency");
		if (opt == mOptions.end())
			Except(999, "Can't find Colour Depth options!", "Win32GLSupport::newWindow");
		unsigned int displayFrequency = atoi(opt->second.currentValue);

		Win32Window* window = new Win32Window();
		window->create(name, width, height, colourDepth, fullScreen, left, top, depthBuffer,
			parentWindowHandle, vsync, displayFrequency);
		return window;
	}

	void Win32GLSupport::start()
	{
		LogManager::getSingleton().logMessage("*** Starting Win32GL Subsystem ***");
	}

	void Win32GLSupport::stop()
	{
		LogManager::getSingleton().logMessage("*** Stopping Win32GL Subsystem ***");
	}

	void* Win32GLSupport::getProcAddress(const String& procname)
	{
        return wglGetProcAddress( procname.c_str() );
	}

}

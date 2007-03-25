/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreRoot.h"

#include "OgreGLXGLSupport.h"

#include "OgreGLXWindow.h"
#include "OgreGLTexture.h"

#include "OgreGLXRenderTexture.h"

#ifndef NO_XRANDR
#include <X11/extensions/Xrandr.h>
#endif

namespace Ogre {

static String gVideoMode ("Video Mode");
static String gFullScreen ("Full Screen");
static String gRefreshRate ("Refresh Rate");
static String gYes ("Yes");
static String gNo ("No");
static String gNotApplicable ("Not applicable");

GLXGLSupport::GLXGLSupport () : mDisplay (0), mScreen (0)
{}

GLXGLSupport::~GLXGLSupport ()
{
	if (mDisplay)
		XCloseDisplay (mDisplay);
}

void GLXGLSupport::addConfig ()
{
	// We have to temporarily open the current display so that we can
	// query display parameters.
	if (!mDisplay)
	{
		mDisplay = XOpenDisplay (NULL);
		if (!mDisplay)
			OGRE_EXCEPT (Exception::ERR_RENDERINGAPI_ERROR,
						 "Couldn`t open X display", "GLXGLSupport::addConfig");
		mScreen = DefaultScreenOfDisplay (mDisplay);
	}

	FillVideoModes ();

	ConfigOption optFullScreen;
	ConfigOption optBitDepth;
	ConfigOption optFSAA;
	ConfigOption optRTTMode;

	// FS setting possiblities
	optFullScreen.name = gFullScreen;
	optFullScreen.possibleValues.push_back(gYes);
	optFullScreen.possibleValues.push_back(gNo);
	optFullScreen.currentValue = gYes;
	optFullScreen.immutable = false;

	// FSAA possibilities
	optFSAA.name = "FSAA";
	optFSAA.possibleValues.push_back("0");
	optFSAA.possibleValues.push_back("2");
	optFSAA.possibleValues.push_back("4");
	optFSAA.possibleValues.push_back("6");
	optFSAA.currentValue = "0";
	optFSAA.immutable = false;

	optRTTMode.name = "RTT Preferred Mode";
	optRTTMode.possibleValues.push_back("FBO");
	optRTTMode.possibleValues.push_back("PBuffer");
	optRTTMode.possibleValues.push_back("Copy");
	optRTTMode.currentValue = "FBO";
	optRTTMode.immutable = false;

	mOptions[optFullScreen.name] = optFullScreen;
	mOptions[optFSAA.name] = optFSAA;
	mOptions[optRTTMode.name] = optRTTMode;
}

void GLXGLSupport::setConfigOption (const String &name, const String &value)
{
	GLSupport::setConfigOption (name, value);
	if (name == gVideoMode || name == gFullScreen)
		RenewRefreshRate ();
}

String GLXGLSupport::validateConfig ()
{
	return String("");
}

RenderWindow* GLXGLSupport::createWindow (bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle)
{
	if (autoCreateWindow)
	{
		ConfigOptionMap::iterator opt = mOptions.find(gFullScreen);
		if (opt == mOptions.end())
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't find full screen options!", "GLXGLSupport::createWindow");
		bool fullscreen = (opt->second.currentValue == gYes);

		opt = mOptions.find(gVideoMode);
		if (opt == mOptions.end())
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Can't find video mode options!", "GLXGLSupport::createWindow");
		String val = opt->second.currentValue;
		String::size_type pos = val.find('x');
		if (pos == String::npos)
			OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Invalid Video Mode provided", "GLXGLSupport::createWindow");

		unsigned int w = StringConverter::parseUnsignedInt(val.substr(0, pos));
		unsigned int h = StringConverter::parseUnsignedInt(val.substr(pos + 1));

		// Parse FSAA config
		NameValuePairList winOptions;
		winOptions["title"] = windowTitle;
		opt = mOptions.find("FSAA");
		if(opt != mOptions.end())
			winOptions["FSAA"] = opt->second.currentValue;

		opt = mOptions.find(gRefreshRate);
		if (opt != mOptions.end ())
			winOptions["displayFrequency"] = opt->second.currentValue;

		return renderSystem->createRenderWindow(windowTitle, w, h, fullscreen, &winOptions);
	}
	else
	{
		// XXX What is the else?
		return NULL;
	}
}

RenderWindow* GLXGLSupport::newWindow(const String &name, unsigned int width, unsigned int height,
	bool fullScreen, const NameValuePairList *miscParams)
{
	GLXWindow *window = new GLXWindow (mDisplay);
	window->create (name, width, height, fullScreen, miscParams);
	return window;
}

void GLXGLSupport::start()
{
	LogManager::getSingleton().logMessage(
		"******************************\n"
		"*** Starting GLX Subsystem ***\n"
		"******************************");
	if (!mDisplay)
	{
		mDisplay = XOpenDisplay (NULL);
		if(!mDisplay)
			OGRE_EXCEPT (Exception::ERR_RENDERINGAPI_ERROR,
						 "Couldn`t open X display", "GLXGLSupport::start");
		mScreen = DefaultScreenOfDisplay (mDisplay);
	}
}

void GLXGLSupport::stop()
{
	LogManager::getSingleton().logMessage(
	        "******************************\n"
	        "*** Stopping GLX Subsystem ***\n"
	        "******************************");
	if(mDisplay)
		XCloseDisplay(mDisplay);
	mDisplay = 0;
}

void *GLXGLSupport::getProcAddress(const String& procname)
{
	return (void*)glXGetProcAddress((const GLubyte*)procname.c_str());
}

GLPBuffer *GLXGLSupport::createPBuffer (
    PixelComponentType format, size_t width, size_t height)
{
    return new GLXPBuffer (format, width, height);
}

static int compare_modes (const void *m1, const void *m2)
{
	const XRRScreenSize *ss1 = reinterpret_cast<const XRRScreenSize *> (m1);
	const XRRScreenSize *ss2 = reinterpret_cast<const XRRScreenSize *> (m2);
	return ss2->width * ss2->height - ss1->width * ss1->height;
}

void GLXGLSupport::FillVideoModes ()
{
	// Video mode possiblities
	ConfigOption optVideoMode;
	optVideoMode.name = gVideoMode;
	optVideoMode.immutable = false;

	char tmp [20];
#ifndef NO_XRANDR
	int dummy;
	if (XQueryExtension (mDisplay, "RANDR", &dummy, &dummy, &dummy))
	{
		XRRScreenConfiguration *config = XRRGetScreenInfo (
			mDisplay, RootWindowOfScreen (mScreen));
		if (config)
		{
			int nsizes;
			XRRScreenSize *sizes = XRRConfigSizes (config, &nsizes);

			// Sort modes based on number of pixels
			qsort (sizes, nsizes, sizeof (XRRScreenSize), compare_modes);

			int sw = WidthOfScreen (mScreen);
			int sh = HeightOfScreen (mScreen);

			int best_mode_rating = 0x7fffffff;
			for (int i = 0; i < nsizes; i++)
			{
				snprintf (tmp, sizeof (tmp), "%d x %d", sizes [i].width, sizes [i].height);
				optVideoMode.possibleValues.push_back (tmp);

				int mode_rating = (sizes [i].width - sw) * (sizes [i].width - sw) +
					(sizes [i].height - sh) * (sizes [i].height - sh);
				if (mode_rating < best_mode_rating)
				{
					optVideoMode.currentValue =
						optVideoMode.possibleValues [optVideoMode.possibleValues.size () - 1];
					best_mode_rating = mode_rating;
				}
			}

			XRRFreeScreenConfigInfo (config);
		}
	}

	if (!optVideoMode.possibleValues.empty ())
	{
		// Xrandr supported, we can switch refresh rates
		ConfigOption optRefreshRate;
		optRefreshRate.name = gRefreshRate;
		optRefreshRate.immutable = false;
		mOptions[optRefreshRate.name] = optRefreshRate;
	}
	else
#endif
	{
		// XRandr is not supported, so just use a list of predefined modes
		// Just make sure they aren't larger than desktop.
		int sw = WidthOfScreen (mScreen);
		int sh = HeightOfScreen (mScreen);

		// Check if current screen is 16:10 (wide), add widescreen mdoes too
		bool is_wide = ((sw * 100) / sh) > 140;

		static struct { short w, h; bool wide; } predefModes [] =
		{
			{  640,  400, true  },
			{  640,  480, false },
			{  720,  450, true  },
			{  800,  600, false },
			{  840,  525, true  },
			{ 1024,  768, false },
			{ 1280,  800, true  },
			{ 1280, 1024, false },
			{ 1440,  900, true  },
			{ 1680, 1050, true  },
			{ 1600, 1200, false },
		};

		int best_mode_rating = 0x7fffffff;
		for (size_t i = 0; i < sizeof (predefModes) / sizeof (predefModes [0]); i++)
		{
			if (sw >= predefModes [i].w && sh >= predefModes [i].h &&
				(is_wide || !predefModes [i].wide))
			{
				snprintf (tmp, sizeof (tmp), "%d x %d",
						  predefModes [i].w, predefModes [i].h);
				optVideoMode.possibleValues.push_back (tmp);

				int mode_rating = (predefModes [i].w - sw) * (predefModes [i].w - sw) +
					(predefModes [i].h - sh) * (predefModes [i].h - sh);
				if (mode_rating < best_mode_rating)
				{
					optVideoMode.currentValue =
						optVideoMode.possibleValues [optVideoMode.possibleValues.size () - 1];
					best_mode_rating = mode_rating;
				}
			}
		}
	}

	mOptions[optVideoMode.name] = optVideoMode;
	RenewRefreshRate ();
}

void GLXGLSupport::RenewRefreshRate ()
{
#ifndef NO_XRANDR
	ConfigOption &optRefreshRate = mOptions [gRefreshRate];

	optRefreshRate.possibleValues.clear ();
	if (optRefreshRate.currentValue == gNotApplicable)
		optRefreshRate.currentValue.clear ();

	ConfigOptionMap::iterator opt = mOptions.find (gVideoMode);
	if (opt == mOptions.end ())
		return;

	String val = opt->second.currentValue;
	String::size_type pos = val.find('x');
	if (pos == String::npos)
		OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Invalid Video Mode provided", "GLXGLSupport::createWindow");

	int sw = StringConverter::parseUnsignedInt (val.substr (0, pos));
	int sh = StringConverter::parseUnsignedInt (val.substr (pos + 1));

	opt = mOptions.find (gFullScreen);
	if (opt != mOptions.end () && opt->second.currentValue != gYes)
	{
		optRefreshRate.possibleValues.push_back (gNotApplicable);
		optRefreshRate.currentValue = optRefreshRate.possibleValues [optRefreshRate.possibleValues.size () - 1];
		return;
	}

	int dummy;
	if (XQueryExtension (mDisplay, "RANDR", &dummy, &dummy, &dummy))
	{
		XRRScreenConfiguration *config = XRRGetScreenInfo (
			mDisplay, RootWindowOfScreen (mScreen));
		if (config)
		{
			int nsizes;
			XRRScreenSize *sizes = XRRConfigSizes (config, &nsizes);

			for (int i = 0; i < nsizes; i++)
			{
				if (sizes [i].width == sw && sizes [i].height == sh)
				{
					int nrates;
					short *rates = XRRConfigRates (config, i, &nrates);
					for (int j = 0; j < nrates; j++)
						optRefreshRate.possibleValues.push_back (
							StringConverter::toString (rates [j]));

					if (optRefreshRate.currentValue.empty ())
						if (i == 0)
						{
							short cr = XRRConfigCurrentRate (config);
							optRefreshRate.currentValue = StringConverter::toString (cr);
						}
						else
							optRefreshRate.currentValue = StringConverter::toString (rates [0]);

					break;
				}
			}

			XRRFreeScreenConfigInfo (config);
		}
	}
#endif
}

}

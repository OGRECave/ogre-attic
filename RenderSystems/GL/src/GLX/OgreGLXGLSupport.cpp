/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
 
Copyright © 2000-2004 The OGRE Team
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

namespace Ogre {

GLXGLSupport::GLXGLSupport():
mDisplay(0) {}

GLXGLSupport::~GLXGLSupport() {}

void GLXGLSupport::addConfig(void) {
	ConfigOption optFullScreen;
	ConfigOption optVideoMode;
	ConfigOption optBitDepth;
    ConfigOption optFSAA;

	// FS setting possiblities
	optFullScreen.name = "Full Screen";
	optFullScreen.possibleValues.push_back("Yes");
	optFullScreen.possibleValues.push_back("No");
	optFullScreen.currentValue = "Yes";
	optFullScreen.immutable = false;

	// Video mode possiblities
	optVideoMode.name = "Video Mode";
	optVideoMode.immutable = false;

	// We could query Xrandr here, but that wouldn't work in the non-fullscreen case
	// or when that extension is disabled. Anyway, this list of modes is fairly
	// complete.
	optVideoMode.possibleValues.push_back("640 x 480");
	optVideoMode.possibleValues.push_back("800 x 600");
	optVideoMode.possibleValues.push_back("1024 x 768");
	optVideoMode.possibleValues.push_back("1280 x 960");
	optVideoMode.possibleValues.push_back("1280 x 1024");
	optVideoMode.possibleValues.push_back("1600 x 1200");

	optVideoMode.currentValue = "800 x 600";

    //FSAA possibilities
    optFSAA.name = "FSAA";
    optFSAA.possibleValues.push_back("0");
    optFSAA.possibleValues.push_back("2");
    optFSAA.possibleValues.push_back("4");
    optFSAA.possibleValues.push_back("6");
    optFSAA.currentValue = "0";
    optFSAA.immutable = false;


	mOptions[optFullScreen.name] = optFullScreen;
	mOptions[optVideoMode.name] = optVideoMode;
    mOptions[optFSAA.name] = optFSAA;
}

String GLXGLSupport::validateConfig(void) {
	return String("");
}

RenderWindow* GLXGLSupport::createWindow(bool autoCreateWindow, GLRenderSystem* renderSystem, const String& windowTitle) 
{
	if (autoCreateWindow) {
		ConfigOptionMap::iterator opt = mOptions.find("Full Screen");
		if (opt == mOptions.end())
			Except(999, "Can't find full screen options!", "GLXGLSupport::createWindow");
		bool fullscreen = (opt->second.currentValue == "Yes");

		opt = mOptions.find("Video Mode");
		if (opt == mOptions.end())
			Except(999, "Can't find video mode options!", "GLXGLSupport::createWindow");
		String val = opt->second.currentValue;
		String::size_type pos = val.find('x');
		if (pos == String::npos)
			Except(999, "Invalid Video Mode provided", "GLXGLSupport::createWindow");

		unsigned int w = StringConverter::parseUnsignedInt(val.substr(0, pos));
		unsigned int h = StringConverter::parseUnsignedInt(val.substr(pos + 1));

        // Parse FSAA config
		NameValuePairList winOptions;
		winOptions["title"] = windowTitle;
        int fsaa_x_samples = 0;
        opt = mOptions.find("FSAA");
        if(opt != mOptions.end())
        {
			winOptions["FSAA"] = opt->second.currentValue;
        }

		return renderSystem->createRenderWindow(windowTitle, w, h, fullscreen, &winOptions);
	} else {
		// XXX What is the else?
		return NULL;
	}
}

RenderWindow* GLXGLSupport::newWindow(const String &name, unsigned int width, unsigned int height, 
	bool fullScreen, const NameValuePairList *miscParams)
{
	GLXWindow* window = new GLXWindow(mDisplay);
	window->create(name, width, height, fullScreen, miscParams);
	return window;
}

void GLXGLSupport::start() {
	LogManager::getSingleton().logMessage(
	        "******************************\n"
	        "*** Starting GLX Subsystem ***\n"
	        "******************************");
	mDisplay = XOpenDisplay(NULL);
	if(!mDisplay) {
		Except(999, "Couldn`t open X display", "GLXGLSupport::start");
	}

}

void GLXGLSupport::stop() {
	LogManager::getSingleton().logMessage(
	        "******************************\n"
	        "*** Stopping GLX Subsystem ***\n"
	        "******************************");
	if(mDisplay)
		XCloseDisplay(mDisplay);
	mDisplay = 0;
}

void GLXGLSupport::initialiseCapabilities(RenderSystemCapabilities &caps) 
{
    // Broken for ATI glx currently
	if(getGLVendor() != "ATI")
    	caps.setCapability(RSC_HWRENDER_TO_TEXTURE);
	// This does work for ATI, even under Linux
	if(checkExtension("GL_SGIS_generate_mipmap"))
		caps.setCapability(RSC_AUTOMIPMAP);
}


extern "C" {
extern void (*glXGetProcAddressARB(const GLubyte *procName))( void );
};

void* GLXGLSupport::getProcAddress(const String& procname) {
	return (void*)glXGetProcAddressARB((const GLubyte*)procname.c_str());
}



RenderTexture * GLXGLSupport::createRenderTexture( const String & name, unsigned int width, unsigned int height,
		TextureType texType, PixelFormat internalFormat, 
		const NameValuePairList *miscParams )
{
	if(Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(RSC_HWRENDER_TO_TEXTURE))
		return new GLXRenderTexture(name, width, height, texType, internalFormat, miscParams);
	else
		return new GLRenderTexture(name, width, height, texType, internalFormat, miscParams);
}  

}

/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourgeforge.net/

Copyright © 2000-2003 Steven J. Streeting
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/gpl.html.
-----------------------------------------------------------------------------
*/

#include "OgreGTKGLSupport.h"
#include "OgreLogManager.h"
#include "OgreException.h"
#include "OgreStringConverter.h"

#include "GTKWindow.h"

using namespace Ogre;

GTKGLSupport::GTKGLSupport() : 
    _kit(0, NULL), _context_ref(0)
{
    Gtk::GL::init(0, NULL);
}

void GTKGLSupport::addConfig()
{
    ConfigOption optFullScreen;
    ConfigOption optVideoMode;

     // FS setting possiblities
    optFullScreen.name = "Full Screen";
    optFullScreen.possibleValues.push_back("Yes");
    optFullScreen.possibleValues.push_back("No");
    optFullScreen.currentValue = "Yes";
    optFullScreen.immutable = false;
 
    // Video mode possiblities
    // XXX Actually do this
    optVideoMode.name = "Video Mode";
    optVideoMode.immutable = false;
    optVideoMode.possibleValues.push_back("800 x 600");
    optVideoMode.currentValue = "800 x 600";

    mOptions[optFullScreen.name] = optFullScreen;
    mOptions[optVideoMode.name] = optVideoMode;
}
    
String GTKGLSupport::validateConfig()
{
    return String("");
}

RenderWindow* GTKGLSupport::createWindow(bool autoCreateWindow, 
                                         GLRenderSystem* renderSystem, 
										 const String& windowTitle)
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
 
        return renderSystem->createRenderWindow(windowTitle, w, h, 32,
fullscreen);
    }
    else
    {
        // XXX What is the else?
                return NULL;
    }
}

RenderWindow* GTKGLSupport::newWindow(const String& name, unsigned int width, 
        unsigned int height, unsigned int colourDepth, bool fullScreen, int left, int top,
        bool depthBuffer, RenderWindow* parentWindowHandle, bool vsync)
{
    GTKWindow* window = new GTKWindow();
    window->create(name, width, height, colourDepth, fullScreen, left, top,
                   depthBuffer, parentWindowHandle);

    _ogre_widget = window->get_ogre_widget();
    return window;
}

void GTKGLSupport::start()
{
    LogManager::getSingleton().logMessage(
        "******************************\n"
        "*** Starting GTK Subsystem ***\n"
        "******************************");
}
 
void GTKGLSupport::stop()
{
    LogManager::getSingleton().logMessage(
        "******************************\n"
        "*** Stopping GTK Subsystem ***\n"
        "******************************");
}

void GTKGLSupport::begin_context()
{
    ++_context_ref;
    if (_context_ref == 1)
        _ogre_widget->get_gl_window()->gl_begin(_ogre_widget->get_gl_context());
}

void GTKGLSupport::end_context()
{
    --_context_ref;
    if(_context_ref < 0)
        Except(999, "Too many contexts destroyed!", "GTKGLSupport::end_context");
    if (_context_ref == 0)
    {
        _ogre_widget->get_gl_window()->gl_end();
    }
}
 
void GTKGLSupport::initialiseExtensions(void)
{
    // XXX anythign to actually do here?
}

bool GTKGLSupport::checkMinGLVersion(const String& v) const
{
    int major, minor;
    Gdk::GL::query_version(major, minor);

    std::string::size_type pos = v.find(".");
    int cmaj = atoi(v.substr(0, pos).c_str());
    int cmin = atoi(v.substr(pos + 1).c_str());

    return ( (major >= cmaj) && (minor >= cmin) );
}

bool GTKGLSupport::checkExtension(const String& ext) const
{
    return Gdk::GL::query_gl_extension(ext.c_str());
}

void* GTKGLSupport::getProcAddress(const String& procname)
{
    return Gdk::GL::get_proc_address(procname.c_str());
}


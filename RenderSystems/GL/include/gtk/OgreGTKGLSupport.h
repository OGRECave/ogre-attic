/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://ogre.sourceforge.net/

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

#ifndef INCL_OGRE_GTKGLSUPPORT_H
#define INCL_OGRE_GTKGLSUPPORT_H

#include "OgreGLSupport.h"

#include <gtkmm/main.h>
#include <gtkglmm.h>

namespace Ogre {

class OGREWidget;

class GTKGLSupport : public GLSupport
{
public:
    GTKGLSupport();
    void addConfig();
    void setConfigOptions(const String& name, const String& value);
    String validateConfig();
    RenderWindow* createWindow(bool autoCreateWindow, 
                               GLRenderSystem* renderSystem, const String& windowTitle);
    RenderWindow* newWindow(const String& name, unsigned int width, unsigned int height, 
                            unsigned int colourDepth, bool fullScreen, int left, int top,
                            bool depthBuffer, RenderWindow* parentWindowHandle,
                            bool vsync);
    void start();
    void stop();
    void begin_context();
    void end_context();
    void initialiseExtensions(void);
    bool checkMinGLVersion(const String& v) const;
    bool checkExtension(const String& ext) const;
    void* getProcAddress(const String& procname);
private:
    Gtk::Main _kit;
    int _context_ref;
    OGREWidget* _ogre_widget;
}; // class GTKGLSupport

}; // namespace Ogre

#endif // INCL_OGRE_GTKGLSUPPORT_H

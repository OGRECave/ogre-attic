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

#include "OgreGTKWindow.h"
#include "OgreRenderSystem.h"
#include "OgreRoot.h"

using namespace Ogre;

OGREWidget::OGREWidget(bool useDepthBuffer) : Gtk::GL::DrawingArea()
{
    Glib::RefPtr<Gdk::GL::Config> glconfig;

    Gdk::GL::ConfigMode mode = Gdk::GL::MODE_RGBA | Gdk::GL::MODE_DOUBLE;
    if (useDepthBuffer)
        mode |= Gdk::GL::MODE_DEPTH;

    glconfig = Gdk::GL::Config::create(mode);
    if (glconfig.is_null())
    {
        printf("GLCONFIG BLOWUP\n");
    }

    set_gl_capability(glconfig);

    add_events(Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
}

GTKWindow::GTKWindow()
{
    kit = Gtk::Main::instance();
    Gtk::GL::init(0, NULL);
}

GTKWindow::~GTKWindow()
{
}

bool GTKWindow::pump_events()
{
    if (kit->events_pending())
    {
        kit->iteration(false);
        return true;
    }
    return false;
}

OGREWidget* GTKWindow::get_ogre_widget()
{
    return ogre;
}

void GTKWindow::create(const String& name, unsigned int width, unsigned int height, unsigned int colourDepth, 
                       bool fullScreen, int left, int top, bool depthBuffer, 
                       void* miscParam, ...)
{
    mName = name;
    set_title(mName);

    if (fullScreen)
    {
        set_decorated(false);
        fullscreen();
    }
    else
    {
        mWidth = width;
        mHeight = height;
        set_default_size(mWidth, mHeight);
        move(left, top); 
    }

    ogre = Gtk::manage(new OGREWidget(depthBuffer));
    ogre->set_size_request(width, height);
    add(*ogre);

    signal_delete_event().connect(SigC::slot(*this, &GTKWindow::on_delete_event));

    show_all();
}

void GTKWindow::destroy()
{
    Root::getSingleton().getRenderSystem()->detachRenderTarget( this->getName() );
}

bool GTKWindow::isActive() const
{
    return is_realized();
}

bool GTKWindow::isClosed() const
{
    return is_visible();
}

void GTKWindow::reposition(int left, int top)
{
    move(left, top);
}

void GTKWindow::resize(unsigned int width, unsigned int height)
{
    resize(width, height);
}

void GTKWindow::swapBuffers(bool waitForVSync)
{
    Glib::RefPtr<Gdk::GL::Window> glwindow = ogre->get_gl_window();
    glwindow->swap_buffers();
}

void GTKWindow::outputText(int x, int y, const String& text)
{
    // XXX impl me
}

void GTKWindow::writeContentsToFile(const String& filename)
{
    // XXX impl me
}

bool GTKWindow::on_delete_event(GdkEventAny* event)
{
    Root::getSingleton().getRenderSystem()->detachRenderTarget( getName() );
    return false;
}

void GTKWindow::getCustomAttribute( const String& name, void* pData )
{
	if( name == "GTKMMWINDOW" )
	{
		Gtk::Window **win = static_cast<Gtk::Window **>(pData);
		// Oh, the burdens of multiple inheritance
		*win = this;
		return;
	}
	else if( name == "GTKGLMMWIDGET" )
	{
	    	Gtk::GL::DrawingArea **widget = static_cast<Gtk::GL::DrawingArea **>(pData);
		*widget = ogre;
		return;
	}
	else if( name == "isTexture" )
	{
		bool *b = reinterpret_cast< bool * >( pData );
		*b = false;
		return;
	}
	RenderWindow::getCustomAttribute(name, pData);
}

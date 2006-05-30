/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2005 The OGRE Team
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
#include "OgreStableHeaders.h"
#include "OgreWindowEventUtilities.h"
#include "OgreRenderWindow.h"
#include "OgreLogManager.h"
#include "OgreRoot.h"

using namespace Ogre;

WindowEventUtilities::WindowEventListeners WindowEventUtilities::msListeners;
WindowEventUtilities::Windows WindowEventUtilities::msWindows;

//--------------------------------------------------------------------------------//
void WindowEventUtilities::messagePump()
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	// Windows Message Loop (NULL means check all HWNDs belonging to this context)
	MSG  msg;
	while( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	//GLX Message Pump (Loop through each specific RenderWindow we have registered)
	Windows::iterator i = msWindows.begin(), e = msWindows.end();
	for( ; i != e; ++i )
	{
		Display display;
		XEvent event;

		*i->getCustomAttribute("DISPLAY");
		while(XPending(display) > 0)
		{
			XNextEvent(display, &event);
			_GLXProc(win, display, event);
		}
	}
#endif
}

//--------------------------------------------------------------------------------//
void WindowEventUtilities::addWindowEventListener( RenderWindow* window, WindowEventListener* listener )
{
	msListeners.insert(std::make_pair(window, listener));
}

//--------------------------------------------------------------------------------//
void WindowEventUtilities::removeWindowEventListener( RenderWindow* window, WindowEventListener* listener )
{
	WindowEventListeners::iterator i = msListeners.begin(), e = msListeners.end();

	for( ; i != e; ++i )
	{
		if( i->first == window && i->second == listener )
		{
			msListeners.erase(i);
			break;
		}
	}
}

//--------------------------------------------------------------------------------//
void WindowEventUtilities::_addRenderWindow(RenderWindow* window)
{
	msWindows.push_back(window);
}

//--------------------------------------------------------------------------------//
void WindowEventUtilities::_removeRenderWindow(RenderWindow* window)
{
	Windows::iterator i = std::find(msWindows.begin(), msWindows.end(), window);
	if( i != msWindows.end() )
		msWindows.erase( i );
}

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
//--------------------------------------------------------------------------------//
LRESULT CALLBACK WindowEventUtilities::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CREATE)
	{	// Store pointer to Win32Window in user data area
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)(((LPCREATESTRUCT)lParam)->lpCreateParams));
		return 0;
	}

	// look up window instance
	// note: it is possible to get a WM_SIZE before WM_CREATE
	RenderWindow* win = (RenderWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (!win)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	//LogManager* log = LogManager::getSingletonPtr();
	//Iterator of all listeners registered to this RenderWindow
	WindowEventListeners::iterator start = msListeners.lower_bound(win),
								   end   = msListeners.upper_bound(win);

	switch( uMsg )
	{
	case WM_ACTIVATE:
	{
		bool active = (LOWORD(wParam) != WA_INACTIVE);
		win->setActive( active );
		for( ; start != end; ++start )
			(start->second)->windowFocusChange(win);
		break;
	}
	case WM_ENTERSIZEMOVE:
		//log->logMessage("WM_ENTERSIZEMOVE");
		break;
	case WM_EXITSIZEMOVE:
		//log->logMessage("WM_EXITSIZEMOVE");
		break;
	case WM_MOVE:
		//log->logMessage("WM_MOVE");
		win->windowMovedOrResized();
		for( ; start != end; ++start )
			(start->second)->windowMoved(win);
		break;
	case WM_SIZE:
		//log->logMessage("WM_SIZE");
		win->windowMovedOrResized();
		for( ; start != end; ++start )
			(start->second)->windowResized(win);
		break;
	case WM_GETMINMAXINFO:
		// Prevent the window from going smaller than some minimu size
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
		break;
	case WM_CLOSE:
		//log->logMessage("WM_CLOSE");
		win->destroy();

		for( ; start != end; ++start )
			(start->second)->windowClosed(win);
		return 0;
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
//--------------------------------------------------------------------------------//
void WindowEventUtilities::_GLXProc( RenderWindow* win, Display& disp, const XEvent &event )
{
	switch(event.type)
	{
	case ClientMessage:
		if(event.xclient.format == 32 && event.xclient.data.l[0] == (long)mAtomDeleteWindow)
		{
			//Window Closed (via X button)
			mClosed = true;
			mActive = false;

			//Root::getSingleton().getRenderSystem()->detachRenderTarget( this->getName() );
		}
		break;
	case ConfigureNotify:
		win->resized(event.xconfigure.width, event.xconfigure.height);

		break;
	case MapNotify:
		// Window was mapped to the screen
		mActive = true;
		break;
	case UnmapNotify:
		// Window was unmapped from the screen (user switched
		// to another workspace, for example)
		mActive = false;
		break;
	case VisibilityNotify:
		//Visibility status changed
		switch(event.xvisibility.state)
		{
		case VisibilityUnobscured:
			mActive = mVisible = true;
			break;
		case VisibilityPartiallyObscured:
			mActive = false;
			mVisible = true;
			break;
		case VisibilityFullyObscured:
			mActive = mVisible = false;
			break;
		}
	}
}
#endif

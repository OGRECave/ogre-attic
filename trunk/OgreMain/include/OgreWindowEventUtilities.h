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
#ifndef __OgreWindowEventUtils_H__
#define __OgreWindowEventUtils_H__

#include "OgrePrerequisites.h"
#include "OgrePlatform.h"

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

namespace Ogre
{
	/**
		@Remarks
			Callback class used to send out window events to client app
	*/
	class WindowEventListener
	{
	public:
		virtual ~WindowEventListener() {};

		/**
		@Remarks
			Window has moved position
		@param rw
			The RenderWindow which created this events
		*/
		virtual void windowMoved(RenderWindow* rw)   {}

		/**
		@Remarks
			Window has resized
		@param rw
			The RenderWindow which created this events
		*/
		virtual void windowResized(RenderWindow* rw) {}

		/**
		@Remarks
			Window has been closed
		@param rw
			The RenderWindow which created this events
		*/
		virtual void windowClosed(RenderWindow* rw)  {}

		/**
		@Remarks
			Window has lost/gained focuw
		@param rw
			The RenderWindow which created this events
		*/
		virtual void windowFocusChange(RenderWindow* rw) {}
	};

	/**
		@Remarks
			Utility class to handle Window Events/Pumping/Messages
	*/
	class _OgreExport WindowEventUtilities
	{
	public:
		static void messagePump();

		static void addWindowEventListener( RenderWindow* window, WindowEventListener* listener );
		static void removeWindowEventListener( RenderWindow* window, WindowEventListener* listener );

		static void _addRenderWindow(RenderWindow* window);
		static void _removeRenderWindow(RenderWindow* window);

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

		//These are public only so GLXProc can access them
		typedef std::multimap<RenderWindow*, WindowEventListener*> WindowEventListeners;
		static WindowEventListeners _msListeners;
	protected:
		typedef std::vector<RenderWindow*> Windows;
		static Windows msWindows;
	};
}
#endif

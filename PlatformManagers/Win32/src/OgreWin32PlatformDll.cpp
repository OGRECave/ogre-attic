/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.stevestreeting.com/ogre/

Copyright © 2000-2001 Steven J. Streeting
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
#include "OgreWin32ConfigDialog.h"
#include "OgreWin32ErrorDialog.h"
#include "OgreWin32Input.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"

namespace Ogre {

#ifdef DEBUG
    int g_iCreatedConfigDiag = 0;
    int g_iCreatedErrorDiag = 0;
    int g_iCreatedRenderWindow = 0;
    int g_iCreatedInputReader = 0;
#endif

    /// Retrieves an instance of a config dialog for this platform
    extern "C" void createPlatformConfigDialog(ConfigDialog** ppDlg)
    {
        // Must get HISTANCE
        HINSTANCE hInst = GetModuleHandle("OgrePlatform.dll");
        *ppDlg = new Win32ConfigDialog(hInst);

#ifdef DEBUG
        g_iCreatedConfigDiag++;
#endif
    }

    /// Retrieves an instance of an error dialog for this platform
    extern "C" void createPlatformErrorDialog(ErrorDialog** ppDlg)
    {
        HINSTANCE hInst = GetModuleHandle("OgrePlatform.dll");
        *ppDlg = new Win32ErrorDialog(hInst);

#ifdef DEBUG
        g_iCreatedErrorDiag++;
#endif
    }


    /// Retrieves an instance of an input reader for this platform
    extern "C" void createPlatformInputReader(InputReader** ppReader)
    {
        *ppReader = new Win32Input();

#ifdef DEBUG
        g_iCreatedInputReader++;
#endif
    }

    /// Destroys
    extern "C" void destroyPlatformConfigDialog(ConfigDialog* dlg)
    {
        delete dlg;

#ifdef DEBUG
        g_iCreatedConfigDiag--;
#endif
    }
    /// Destroys
    extern "C" void destroyPlatformErrorDialog(ErrorDialog* dlg)
    {
        delete dlg;

#ifdef DEBUG
        g_iCreatedErrorDiag--;
#endif
    }
    /// Destroys
    extern "C" void destroyPlatformRenderWindow(RenderWindow* wnd)
    {
        delete wnd;

#ifdef DEBUG
        g_iCreatedRenderWindow--;
#endif
    }
    /// Destroys
    extern "C" void destroyPlatformInputReader(InputReader* reader)
    {
        delete reader;

#ifdef DEBUG
        g_iCreatedInputReader--;
#endif
    }

#ifdef DEBUG
    BOOL WINAPI DllMain( HINSTANCE hinstDLL,  // handle to DLL module
                         DWORD fdwReason,     // reason for calling function
                         LPVOID lpvReserved   // reserved
                       )
    {
        if( fdwReason == DLL_THREAD_DETACH ) {
            if( g_iCreatedConfigDiag )
                LogManager::logMessage( "Memory Leak: Not all platform configuration dialogs were destroyed!!!", LML_CRITICAL );
            if( g_iCreatedConfigDiag )
                LogManager::logMessage( "Memory Leak: Not all platform error dialogs were destroyed!!!", LML_CRITICAL );
            if( g_iCreatedConfigDiag )
                LogManager::logMessage( "Memory Leak: Not all platform render windows were destroyed!!!", LML_CRITICAL );
            if( g_iCreatedConfigDiag )
                LogManager::logMessage( "Memory Leak: Not all platform input readers were destroyed!!!", LML_CRITICAL );
        }
    }
#endif
}
